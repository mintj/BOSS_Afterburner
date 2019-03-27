#ifndef BOSS_Afterburner_ConfigLoader_Plot_H
#define BOSS_Afterburner_ConfigLoader_Plot_H


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
	class ConfigLoader_Plot : public ConfigLoader
	{
	public:
		ConfigLoader_Plot(const TString &path, const bool print=true) : ConfigLoader(path, print) { LoadConfiguration(path); }
		/// @name I/O strings
			///@{
			ConfigParameter<TString> fInputFilename{"Input file or directory", "NOFILE"};
			ConfigParameter<std::list<BranchPlotOptions> > fListOfbranches{"Print these branches"};
			///@}


		/// @name Do switches
			///@{
			ConfigParameter<bool> fDo_conv_d{"Do double convolution", false}; ///< Whether or not to produce perform a Breit-Wigner convoluted with a <i>double</i> Gaussian.
			ConfigParameter<bool> fDo_conv_s{"Do single convolution", false}; ///< Whether or not to produce perform a Breit-Wigner convoluted with a <i>single</i> Gaussian.
			ConfigParameter<bool> fDo_gauss {"Do Gaussian",           true};  ///< Whether or not to produce perform a double Gaussian fit.
			///@}


		/// @name Draw switches
			///@{
			ConfigParameter<bool> fDraw_fit    {"Draw fit branches",  true};  ///< Whether or not to draw the `"fit"` branches.
			ConfigParameter<bool> fDraw_mctruth{"Draw mctruth",       false}; ///< Whether or not to draw the MC truth parameters.
			ConfigParameter<bool> fDraw_mult   {"Draw multiplicites", false}; ///< Whether or not to draw the multiplicity branches.
			ConfigParameter<bool> fDraw_tof    {"Draw ToF",           false}; ///< Whether or not to draw the `"tof*"` branches.
			ConfigParameter<bool> fDraw_vertex {"Draw vertex",        false}; ///< Whether or not to draw the `"vertex"` branch.
			///@}


		/// @name Plot style options
			///@{
			ConfigParameter<bool> fFitplots     {"Perform fits",    false}; ///< Whether or not to produce invariant mass fits.
			ConfigParameter<bool> fLogY         {"Use y log scale", false}; ///< Whether to draw the \f$y\f$ axis of the `TH1F` in log scale.
			ConfigParameter<bool> fLogZ         {"Use z log scale", false}; ///< Whether to draw the \f$z\f$ axis of the `TH2F` in log scale.
			ConfigParameter<bool> fPlotstats    {"Plot statistics", false}; ///< Whether or not to draw the legend in the upper right corner with histogram statistics.
			ConfigParameter<bool> fPrintAverages{"Print averages",  false}; ///< Whether or not to print the averages for all branches. This could be useful when testing whether the branches have been filled correctly. Note that this could increase run time significantly in case of a large data set!
			ConfigParameter<bool> fPrintBranches{"Print branches",  false}; ///< Whether or not to print all branch names.
			ConfigParameter<bool> fPureplot     {"Plot raw data",   true};  ///< Whether or not to plot histograms of branches <i>without fit</i>.
			ConfigParameter<bool> fSetranges    {"Set plot ranges", true};  ///< Whether or not to precisely set histogram ranges.
			///@}


	};
/// @}
#endif