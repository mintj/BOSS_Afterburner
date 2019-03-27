#ifndef BOSS_Afterburner_ConfigLoader_PrintContent_H
#define BOSS_Afterburner_ConfigLoader_PrintContent_H


// * ========================= * //
// * ------- LIBRARIES ------- * //
// * ========================= * //

	#include "CommonFunctions.h"
	#include "ConfigLoader.h"
	#include "ConfigParameter.h"
	#include "TString.h"



// * ================================= * //
// * ------- CLASS DEFINITIONS ------- * //
// * ================================= * //
/// @addtogroup BOSS_Afterburner_objects
/// @{


	/// Derived class of `ConfigLoader` that contains parameters used for the `InvestigateContent` algorithm.
	/// @author   Remco de Boer 雷穆克 (r.e.deboer@students.uu.nl or remco.de.boer@ihep.ac.cn)
	/// @date     March 26th, 2018
	class ConfigLoader_PrintContent : public ConfigLoader
	{
	public:
		ConfigLoader_PrintContent(const TString &path, const bool print=true) : ConfigLoader(path, print) { LoadConfiguration(path); }
		/// @name I/O strings
			///@{
			ConfigParameter<TString> fInputFilename{"Input file or directory", "/mnt/d/IHEP/root/D0phi_KpiKK/Jpsi2009+2012_dst"};
			///@}


		/// @name Do switches
			///@{
			ConfigParameter<bool> fPrintBranches{"Print branches", true}; ///< Whether or not to print all branch names.
			ConfigParameter<bool> fPrintCutFlow{"Print cut flow", false}; ///< Whether or not to print the cut flow branch (if available).
			///@}


	};
/// @}
#endif