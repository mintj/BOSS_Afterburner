#ifndef BOSS_Afterburner_ConfigLoader_InvMass_H
#define BOSS_Afterburner_ConfigLoader_InvMass_H


// * ========================= * //
// * ------- LIBRARIES ------- * //
// * ========================= * //

	#include "CommonFunctions.h"
	#include "ConfigLoader.h"
	#include "ConfigParameter.h"
	#include "TString.h"
	#include <list>



// * ================================= * //
// * ------- CLASS DEFINITIONS ------- * //
// * ================================= * //
/// @addtogroup BOSS_Afterburner_objects
/// @{


	/// Derived class of `ConfigLoader` that contains parameters used for a simple plotting algorithm.
	/// @author   Remco de Boer 雷穆克 (r.e.deboer@students.uu.nl or remco.de.boer@ihep.ac.cn)
	/// @date     March 26th, 2018
	class ConfigLoader_InvMass : public ConfigLoader
	{
	public:
		ConfigLoader_InvMass(const TString &path, const bool print=true) : ConfigLoader(path, print) { LoadConfiguration(path); }
		/// @name I/O strings
			///@{
			ConfigParameter<TString> Filename_excl{"Exclusive MC file"};
			ConfigParameter<TString> Filename_incl{"Inclusive MC file"};
			ConfigParameter<TString> Filename_data{"Measurements file"};
			ConfigParameter<std::list<BranchPlotOptions> > ListOfbranches_excl{"Plot these inclusive MC branches"};
			ConfigParameter<std::list<BranchPlotOptions> > ListOfbranches_incl{"Plot these exclusive MC branches"};
			ConfigParameter<std::list<BranchPlotOptions> > ListOfbranches_data{"Plot these measurement branches"};
			///@}


		/// @name Do switches
			///@{
			ConfigParameter<bool> Do_PrintCutFlow{"Print cut flow", true}; ///< Whether or not to print an indication of the cut flow for the three files.
			///@}


		/// @name Fit parameters
			///@{
			ConfigParameter<std::list<std::pair<ReconstructedParticle, BranchPlotOptions> > > ExclFits {"Signals to fit"};
			///@}


		/// @name Global plot style options
			///@{
			ConfigParameter<bool> Do_PlotStatsLegend{"Plot legend with statistics", false}; ///< Whether or not to draw the legend in the upper right corner with histogram statistics.
			///@}


	};
/// @}
#endif