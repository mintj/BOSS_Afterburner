// * ========================= * //
// * ------- LIBRARIES ------- * //
// * ========================= * //
	#include "BranchPlotOptions.h"
	#include "CommonFunctions.h"
	#include "FrameworkSettings.h"
	#include "TRegexp.h"
	#include <sstream>
	#include <stdio.h>
	using namespace CommonFunctions;



// * =========================== * //
// * ------- CONSTRUCTORS ------ * //
// * =========================== * //


	BranchPlotOptions::BranchPlotOptions(const TString &input)
	{
		Import(input);
	}



// * ====================== * //
// * ------- SETTERS ------ * //
// * ====================== * //


	/// Set all the branch plot options based on an input line.
		/// The syntax for this input line is descripted in the class description for 
		/// @remark The `TString` argument `input` is not a reference, because it is modified in this function.
	const bool BranchPlotOptions::Import(TString input)
	{
		/// -# Check `input` argument: needs at least **three** semicolumns (`;`).
			Int_t nsemicol { input.CountChar(';') };
			if(ImportWarning(input, "has to contain at least 3 semicolumns (at least 4 argument blocks)", nsemicol < 3)) return false;
			if(ImportWarning(input, Form("contains %d branches, which is impossible to plot", nsemicol-2), nsemicol > 5)) return false;
			Ssiz_t count{0};
		/// -# Insert spaces in all occurences of `;;`, because `TString::Tokenize` will skip such an argument blocks.
			while(input.Contains(";;")) input.Insert(input.Index(";;")+1, " ");
			if(input.EndsWith(";")) input+=" ";
		/// -# Get and set the tree name.
			TString tok;
			Ssiz_t start{0};
			if(ImportWarning(input, "does not contain a tree name", !Tokenize(input, tok, start))) return false;
			SetTreeName(tok);
			++count;
		/// -# The next arguments will be branches. **If they have been loaded, set the output file name.*8
			while(count < nsemicol-1 && Tokenize(input, tok, start)) {
				if(!AddBranch(tok)) return false;
				++count;
			}
		/// -# Get cut selection options.
			if(ImportWarning(input, "does not contain a cut selection", !Tokenize(input, tok, start))) return false;
			SetCutSelection(tok);
		/// -# Get draw options.
			if(ImportWarning(input, "does not contain draw options", !Tokenize(input, tok, start))) return false;
			SetDrawOptions(tok);
		/// -# Set `fIsOK` status to `true`.
			fIsOK = true;
			return true;
	}


	/// Attempt to add a `AxisBinning` object to the `fListOfBranches` based on the `input` string.
		/// Format of the `input` string should be according to the description of the `BranchPlotOptions`
	const bool BranchPlotOptions::AddBranch(const TString &input)
	{
		auto ncommas{input.CountChar(',')};
		if(ncommas==0 || ncommas==3) {
			fListOfBranches.push_back({input.Data()});
			return true;
		}
		ImportWarning(input, Form("--> branch \"%s\" is invalid", input.Data()));
		return false;
	}


	/// Set cut selection string.
		/// Format should be according to the `selection` argument of the [TTree::Draw](https://root.cern.ch/doc/master/classTTree.html#a73450649dc6e54b5b94516c468523e45) method.
	void BranchPlotOptions::SetCutSelection(const TString &selection)
	{
		fCutSelection = selection;
	}


	/// Set draw options and logscale bits based on some `input` line.
	void BranchPlotOptions::SetDrawOptions(TString input)
	{
		/// <ol>
		/// <li> Ignore upper case.
			input.ToLower();
		/// <li> Set logscale bits if the `input` string contains a substring `"log"`. All characters that are `x`, `y`, or `z` after each `"log"` substring are read and used to set the corresponding log bits.
			auto start{input.Index("log")};
			if(start > 0) {
				TString tok;
				for(auto i : fSetLog) i = false; // reset bits
				while(input.Tokenize(tok, start, "log")) {
					for(int i=0; i<tok.Length(); ++i) {
						UChar_t index = tok[i]-'x'; // unsiged to simplify next if statement
						if(index>=3) break; // if not x, y, or z
						fSetLog[index] = true;
					}
				}
				TRegexp search{"log[xyz]*"};
				while(input.Contains(search)) {
					input.Remove(input.Index(search), input(search).Length());
				}
			}
		/// <li> Remove leading and trailing whitespace characters from what remains of `input` and use to set `fDrawOption`.
			input = input.Strip(TString::kBoth, ' ');
			input = input.Strip(TString::kBoth, '\t');
			fDrawOption = input;
		/// </ol>
	}


	/// Set name of the `TTree` and the output file name.
		/// If the input string `treename` contains a `">"` sign, everything that comes behind it determines the output file name.
	void BranchPlotOptions::SetTreeName(const TString &treename)
	{
		fTreeName       = treename;
		fOutputFileName = treename;
		if(treename.Contains(">")) {
			fTreeName.Remove(treename.First('>'));
			fOutputFileName.Remove(0, treename.First('>')+1);
			String::RemoveWhitespaces(fTreeName);
			String::RemoveWhitespaces(fOutputFileName);
		}
	}


	/// Set name of the output file name.
		/// As with the [`TPad::SaveAs`](https://root.cern.ch/doc/master/classTPad.html#ac53122e6b3fe76f52c379178482236dd) method, the file extension in the `filename` string determines the output format.
	void BranchPlotOptions::SetOutputFileName(const TString &filename)
	{
		fOutputFileName = filename;
	}



// * ========================== * //
// * ------- INFORMATION ------ * //
// * ========================== * //


	const bool BranchPlotOptions::HasLogScale() const
	{
		bool setlog{false};
		for(auto it : fSetLog) setlog |= it;
		return setlog;
	}


	void BranchPlotOptions::Print() const
	{
		if(!IsOK()) {
			std::cout << "Empty BranchPlotOptions";
			if(fTreeName.Length()) std::cout << " (" << fTreeName << ")";
			std::cout << std::endl;
		} else {
			std::cout << "BranchPlotOptions for \"" << fTreeName << "\"" << std::endl;
			std::cout << "  output = \"" << OutputFileName() << "\"" << std::endl;
			std::cout << "  varexp = \"" << VarExp() << "\"" << std::endl;
			std::cout << "  cutsel = \"" << fCutSelection << "\"" << std::endl;
			std::cout << "  option = \"" << fDrawOption << "\"" << std::endl;
			if(HasLogScale()) {
				std::cout << "  setlog = " << LogXYZ() << std::endl;
			}
			std::cout << "  Branches:" << std::endl;
			for(auto &branch : fListOfBranches) branch.Print();
		}
	}


	const bool BranchPlotOptions::IsOK() const
	{
		return fTreeName.Length() && fListOfBranches.size();
	}


	const TString BranchPlotOptions::LogXYZ() const
	{
		TString logxyz;
		for(int i=0; i<3; ++i) if(fSetLog[i]) logxyz += Form("%c", i+'x');
		return logxyz;
	}



// * ================================ * //
// * ------- ARGUMENT BUILDERS ------ * //
// * ================================ * //


	/// Build a `varexp` that is required by the [`TTree::Draw`](https://root.cern.ch/doc/master/classTTree.html#a73450649dc6e54b5b94516c468523e45) method from the contents of `f`.
		/// @return The returned string is a `nullptr` if building a `varexp` string failed.
		/// @todo Should be moved to `ChainLoader` so that one has access to the corresponding `TTree`.
	const char* BranchPlotOptions::VarExp() const
	{
		/// -# Return `nullptr` if `fTreeName` of `fListOfBranches` is empty.
			if(fTreeName.EqualTo("")) return nullptr;
			if(!fListOfBranches.size()) return nullptr;
		/// -# Build string of branch names.
			TString varexp;
			bool setbinning { true };
			for(auto &branch : fListOfBranches) {
				varexp += branch.AxisName();
				varexp += ":";
				setbinning &= branch.IsOK();
			}
			varexp.Chop();
		/// -# Append output histogram name for ROOT. This name has to be unique to avoid overwriting histograms in memory, hence the terribly long and messy name. The name is also used in the output file name.
			varexp += BuildHistName();
		/// -# Append binning **if all branches have bin definitions** (all have `AxisBinning::IsOK` is `true`).
			if(setbinning) {
				varexp += "(";
				for(auto &branch : fListOfBranches) {
					varexp += Form("%d,%g,%g,", branch.NBins(), branch.From(), branch.To());
				}
				varexp.Chop();
				varexp += ")";
			}
		return Form("%s", varexp.Data());
	}


	/// Build the original line from which this `BranchPlotOptions` object was created.
		/// See `Import` to get an idea of the syntax of that line.
	const TString BranchPlotOptions::BuildOriginalString() const
	{
		/// -# Return a `nullptr` if this `BranchPlotOptions` object does not have branches or a tree name.
			if(fTreeName.EqualTo(""))     return nullptr;
			if(!fListOfBranches.size()) return nullptr;
		/// -# The line starts the name of the `TTree`.
			TString line{fTreeName};
		/// -# If the `fOutputFileName` is different than the `fTreeName`, append `" > "fOutputFileName`.
			if(!fTreeName.EqualTo(fOutputFileName)) line += Form(" > %s", fOutputFileName.Data());
			line += "; ";
		/// -# Now add the branches.
			for(auto &branch : fListOfBranches) {
				line += branch.AxisName();
				if(branch.IsOK()) line += Form(", %d, %g, %g", branch.NBins(), branch.From(), branch.To());
				line += "; ";
			}
		/// -# Add the cut selection.
			line += Form("%s; ", fCutSelection.Data());
		/// -# Add the draw options and a line for the log bits if available.
			line += fCutSelection;
			if(HasLogScale()) line += Form(" log%s", LogXYZ().Data());
		return line;
	}


	/// Build the a unique name for an output histogram.
		/// The histogram has to be unique to avoid overwriting objects in the `gDirectory` of ROOT, see [TDirectory::Get](https://root.cern.ch/doc/master/classTDirectory.html#a411338189331c0ec67b2c56822b7773a).
	const TString BranchPlotOptions::BuildHistName() const
	{
		/// -# Return a `nullptr` if this `BranchPlotOptions` object does not have branches or a tree name.
			if(fTreeName.EqualTo(""))     return nullptr;
			if(!fListOfBranches.size()) return nullptr;
		/// -# The histogram name starts the name of the `TTree`.
			TString line{Form("hist_%s_", fTreeName.Data())};
		/// -# This is followed by the branches, which also get identifiers for the plot range.
			for(auto &branch : fListOfBranches) {
				line += branch.AxisName();
				if(branch.IsOK()) line += Form("-%d-%g-%g", branch.NBins(), branch.From(), branch.To());
				line += "_";
			}
			line.Chop();
		return line;
	}



// * ====================== * //
// * ------- HELPERS ------ * //
// * ====================== * //


	/// Helper function that prints a warning message if `notOK` is set to `true`.
		/// @return	Returns `false` if no warning message is printed, that is, if there where no problems (`notOK == false`).
	const bool BranchPlotOptions::ImportWarning(const TString &input, const TString &message, const bool notOK) const
	{
		if(!notOK) return false;
		TerminalIO::PrintWarning(Form("BranchPlotOptions input line:\n  \"%s\"\n%s", input.Data(), message.Data()));
		return true;
	}


	/// Helper function that gets the next argument block up till a semicolumn (`;`), strips that argument block of whitespaces and writes it to `tok`.
	const bool BranchPlotOptions::Tokenize(const TString &input, TString &tok, Ssiz_t &start, const char *delim) const
	{
		if(!input.Tokenize(tok, start, delim)) {
			return false;
		}
		tok = tok.Strip(TString::kBoth, ' ');
		tok = tok.Strip(TString::kBoth, '\t');
		return true;
	}