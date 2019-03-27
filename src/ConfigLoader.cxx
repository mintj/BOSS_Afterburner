// * ========================= * //
// * ------- LIBRARIES ------- * //
// * ========================= * //
	#include "ConfigLoader.h"
	#include "CommonFunctions.h"
	#include "FrameworkSettings.h"
	#include "TRegexp.h"
	#include <stdio.h>
	using namespace CommonFunctions;



// * ============================ * //
// * ------- READ HANDLERS ------ * //
// * ============================ * //


	/// Attempt to get a corresponding parameter @b identifier from a line.
	ConfigParBase* ConfigLoader::ExtractParameter(TString input)
	{
		/// -# If `input` contains an equal sign, remove everything after it. (You are also allowed to place the equal sign on the next input.)
		if(input.Contains("=")) input.Resize(input.First('='));
		/// -# Remove leading and trailing spaces and tabs and double quotation marks (`"`).
		String::RemoveWhitespaces(input);
		input.Strip(TString::kTrailing, '\"');
		/// @return Parameter name as deduced from input `line`
		return ConfigParBase::GetParameter(input);
	}


	/// Import parameter an arbitrary number of argument values from a line.
		/// The `input` line has to be formatted, that is, it may not contain the initial opening or final closing bracket.
	void ConfigLoader::ImportValues(ConfigParBase *par, TString input)
	{
		if(!par) return;
		String::RemoveWhitespaces(input);
		if(input.BeginsWith("{")) input.Remove(0, 1);
		if(input.EndsWith("}")) input.Chop();
		String::RemoveWhitespaces(input);
		if(!input.Length()) return;
		do {
			TString value(input);
			if(input.BeginsWith("\"")) {
				input.Remove(0, 1);
				value.Remove(0, 1);
				if(input.Contains((TRegexp)"\"[ \t]*\"")) input = "";
				else input = input(input.First('\"')+1, input.Length());
				value.Resize(value.First('\"'));
				input.Strip(TString::kBoth, ',');
				String::RemoveWhitespaces(input);
				value.Strip(TString::kBoth, '\"');
			} else {
				if(input.Contains(",")) {
					value.Resize(value.First(','));
					input = input(input.First(',')+1, input.Length());
				} else {
					input = "";
				}
			}
			AddValue(par, value);
			String::RemoveWhitespaces(input);
		} while(!input.EqualTo(""));
	}


	/// Format a string by removing all leading and trailing whitespace characters and double quotation marks (`"`).
	void ConfigLoader::AddValue(ConfigParBase *par, TString &val)
	{
		if(!par) return;
		String::RemoveWhitespaces(val);
		val.Strip(TString::kBoth, '\"');
		if(val.EqualTo("")) return;
		par->AddValue(val);
	}


	/// Load a configuration for analysis from a <i>BOSS Afterburner</i> configuration file.
		/// @todo Rewrite entire read file procedure in terms of `TString`.
	size_t ConfigLoader::LoadConfiguration(const TString &filename)
	{
		/// <ol>
		/// <li> Create file stream (`std::ifstream`) of config `txt` file.
			FILE *file = std::fopen(filename.Data(), "r");
			if(!file) {
				TerminalIO::PrintWarning(Form("Could not load configuration file \"%s\"", filename.Data()));
				return 0;
			}
		/// <li> Loop over lines.
			TString line;
			while(line.Gets(file, true)) {
std::cout << line << std::endl;
				/// <ul>
				/// <li> Remove leading spaces and tabs.
					String::RemoveWhitespaces(line);
					line.Strip(TString::kBoth, ';');
				/// <li> Skip line if it is a comment or if it is empty.
					if(line.EqualTo("")) continue;
					if(String::IsComment(line)) continue;
				/// <li> Now, attempt to extract the paramater identifier.
					ConfigParBase *par = nullptr;
					TString parname{line};
					TString parval {line};
				/// <li> If the `line` does **not** contain brackets opening (`{`) or closing (``}) bracket, ends in an equal sign **and** does contain an equal sign, **there is only one value for this parameter**. This is a simple case and we can move to the next line.
						if(line.Contains("=") && !line.Contains("{") && !line.Contains("}") && !line.EndsWith("=")) {
							// * Attempt to get parameter
							parname.Resize(parname.First('='));
							String::RemoveWhitespaces(parname);
							par = ConfigParBase::GetCleanParameter(parname);
							if(!par) {
std::cout << "Cannot find parameter \"" << parname << "\"" << std::endl;
								continue;
							}
							// * Add the value to the parameter
							parval = parval(parval.First('=')+1, -1);
							TString parval_str{parval.Data()};
							AddValue(par, parval_str);
							continue;
						}
				/// <li> If this is not the case, the parameter contains **multiple values**:
					/// <ol>
					/// <li> If the line contains `=`, `{`, and `}`, all parameter values are defined on this line and we can round up without reading the next line.
						if(line.Contains("=") && line.Contains("{") && line.Contains("}")) {
							// * Attempt to get parameter
							parname.Resize(parname.First('='));
							String::RemoveWhitespaces(parname);
							par = ConfigParBase::GetCleanParameter(parname);
							if(!par) continue;
							// * Add the value to the parameter
							parval = parval(parval.First('{')+1, -1);
							TString parval_str{parval.Data()};
							ImportValues(par, parval_str);
							continue;
						}
					/// <li> If the line does not contain an equal sign (`=`), it means the line only contains the parameter name and that we should continue reading the next lines.
						if(!line.Contains("=")) {
							par = ConfigParBase::GetCleanParameter(parname);
							if(!par) continue;
					/// <li> Else, get the parameter, read the first values on the line and continue to the next.
						} else {
							parname.Resize(parname.First('='));
							String::RemoveWhitespaces(parname);
							par = ConfigParBase::GetCleanParameter(parname);
							if(!par) continue;
							parval = parval(parval.First('=')+1, -1);
							ImportValues(par, parval);
						}
					/// </ol>
				/// <li> Import the remaining values by continuing over the next lines until the first closing bracket (`}`) is encountered.
					while(line.Gets(file, true)) {
std::cout << "sub :" << line << std::endl;
						/// <ol>
						// /// <li> Remove weird characters like EOF.
						// 	if(line.back()<' ') line.pop_back();
						/// <li> Skip line if it is a comment or if it is empty.
							String::RemoveWhitespaces(line);
							if(line.EqualTo("")) continue;
							if(String::IsComment(line)) continue;
						/// <li> `RemoveWhitespaces` in line line and remove opening equal sign (`=`).
							String::RemoveWhitespaces(line); if(line.BeginsWith("=")) line.Remove(0, 1);
							String::RemoveWhitespaces(line); if(line.BeginsWith("{")) line.Remove(0, 1);
							String::RemoveWhitespaces(line);
						/// <li> If this line does not contain a closing bracket, simply import all arguments on this line and go to the next.
							if(!line.Contains("}")) ImportValues(par, line);
						/// <li> If not, we should import the last arguments and then `break` the `while` loop
							else {
								line.Resize(line.Last('}'));
								ImportValues(par, line);
								break;
							}
						/// </ol>
					}
				/// </ul>
			}
		/// <li> Set all parameter values from the read strings
			auto mapping = ConfigParBase::GetMapOfParameters();
			for(auto &it : *mapping) {
				it.second->ConvertStringsToValue();
				it.second->ConvertValueToStrings();
			}
		/// <li> Print success message with the configuration title and all parameters if required by `fPrint`.
			TerminalIO::PrintSuccess(Form(
				"Successfully loaded %d parameters from config file:\n  \"%s\"\n",
				ConfigParBase::GetNParameters(), filename.Data()));
			if(fPrint) {
				ConfigParBase::PrintAll();
				std::cout << std::endl;
			}
		/// </ol>
		/// @return Number of valid loaded arguments
			return ConfigParBase::GetNParameters();
	}