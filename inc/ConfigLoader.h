#ifndef BOSS_Afterburner_ConfigLoader_H
#define BOSS_Afterburner_ConfigLoader_H


// * ========================= * //
// * ------- LIBRARIES ------- * //
// * ========================= * //

	#include "AxisBinning.h"
	#include "CommonFunctions.h"
	#include "ConfigParameter.h"
	#include "TString.h"
	#include <list>



// * ================================= * //
// * ------- CLASS DEFINITIONS ------- * //
// * ================================= * //
/// @addtogroup BOSS_Afterburner_objects
/// @{


	/// Base container object for a <i>BOSS Afterburner</i> analysis configuration file.
		/// This base object defines the abstract loading procedure of a config file. You have to define a derived class (for example `ConfigLoader_Plot`) to specify which parameters your config file contains. The parameters are `ConfigParameter` objects, so you have to define in that object how you load the string values in loaded from the config file to the value types of those `ConfigParameter` objects. Construct such a derived object with a path to the configuration text file for the analysis you want to perform, and all settings will be automatically loaded from this file. The syntax for this file is of course determined by this base object.
		///
		/// The configuration (`.config`) files of the *BOSS Afterburner* final selection framework can be used to load settings for your analysis during runtime from a configuration text file. This allows you to quickly modify plots without having to recompile your analysis script: you just change some of the parameters in your config file. The types of parameters (their identifiers and value types) are defined in the `ConfigLoader` class.
		///
		/// The `ConfigParameter` classes (base `ConfigParBase` and derived classes such as `ConfigParameter`) represent these certain types of parameters. The `ConfigParBase` class defines the most abstract form of a configuration parameter. It just has an @b identifier (or parameter name) and a @b value or set of values. Both are strings, as they are the raw data read from the data file. A line in your configuration file can for instance look like this:
		/// ```
		/// 	Plot raw data = true
		/// ```
		/// Here, the line `"Plot raw data"` is the parameter @b identifier, while `"true"` is the @b value. The line will only be read if there is an `ConfigParBase` object (or derived object with the name `"Plot raw data"`.) The value is in `string` format, but in this case you will want the value to be a boolean. For this you should define the parameter in the `ConfigLoader` in terms of the `ConfigParameter` class.
		///
		/// Another possibility would be to have the lines:
		/// ```
		/// 	Draw branches = {
		/// 		do something; this is OK;
		/// 			mctruth, E, 150, 0.,
		/// 		   3.5, "E1"
		/// 		plo, 1, ;
		///			; empty:zero}
		/// 		
		/// ```
		/// Note this is a lazily formatted vector with weird indentations. What `ConfigParBase` does in this case is, is that it will read these lines as an arrey. In this case, what will be read to the `list` of read values (`fReadStrings`) for the parameter with name `"Draw branches"` is:
		/// ```
		/// 	do something
		/// 	this is OK
		/// 	mctruth, E, 150, 0.,3.5, "E1"
		/// 	plo, 1,
		/// 	empty:zero
		/// ```
		/// The contents of this example are not that useful, but you get the idea:
		/// 	-# a parameter will contain more than one value if its identifier is followed by an opening curly brace (`{`)
		/// 	-# its values are separated by a semicolumn (`;`).
		/// What you want to do with these read values is up to use to define in a derived class.
	/// @author   Remco de Boer 雷穆克 (r.e.deboer@students.uu.nl or remco.de.boer@ihep.ac.cn)
	/// @date     January 24th, 2018
	class ConfigLoader
	{
	public:
		ConfigLoader(const TString &path, const bool print=true) :
			fConfigPath(path), fPrint("Print loaded configuration parameters", print) {}

	protected:
		size_t LoadConfiguration(const TString &path);
		ConfigParBase* ExtractParameter(TString line);
		void ImportValues(ConfigParBase *par, TString line);
		void AddValue(ConfigParBase *par, TString &line);

	private:
		const TString fConfigPath;
		ConfigParameter<bool> fPrint; ///< Parameter that decides whether or not to call `ConfigParBase::PrintAll` after all parmeters have been loaded.
	};



/// @}
#endif