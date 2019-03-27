/// @addtogroup BOSS_Afterburner_scripts
/// @{
// * ======================================== * //
// * ------- LIBRARIES AND NAMESPACES ------- * //
// * ======================================== * //

	#include "ConfigLoader_InvMass.h"
	#include "BOSSOutputLoader.h"
	#include "CommonFunctions.h"
	#include "TStyle.h"
	#include <iostream>
	#include <list>
	#include <vector>

	using namespace CommonFunctions;
	using namespace CommonFunctions::Draw;
	using namespace CommonFunctions::Fit;
	using namespace RooFit;
	using namespace Settings;
	using namespace std;



// * ============================= * //
// * ------- MAIN FUNCTION ------- * //
// * ============================= * //


	/// This script allows one to go through the full procedure of analysing a peak in an invariant mass spectrum plot.
		/// Currently, the script is designed based on the \f$J/\psi \to D^0\phi \to K^-\pi+K^-\K+\f$ analysis.
		/// @author   Remco de Boer 雷穆克 (r.e.deboer@students.uu.nl or remco.de.boer@ihep.ac.cn)
		/// @date     March 26th, 2018
	void AnalyseBOSSOutput(const char* configuration_file="configs/debug.config")
	{
		/// -# Attempt to load `configuration_file`.
std::cout << "OK1" << std::endl;
			ConfigLoader_InvMass config(configuration_file, false);
std::cout << "OK2" << std::endl;

		/// -# Attempt to load three directories of ROOT files as a `BOSSOutputLoader` objects: a set of exlusive MC files, a set of inclusive MC files, and a set of data files (the BESIII measurements).
			BOSSOutputLoader excl(config.Filename_excl, false, false);
			if(excl.IsZombie()) {
				TerminalIO::PrintFatalError(Form("Failed to load directory/file\n  \"%s\"", config.Filename_excl->Data()));
				return;
			}
			BOSSOutputLoader incl(config.Filename_incl, false, false);
			if(incl.IsZombie()) {
				TerminalIO::PrintFatalError(Form("Failed to load directory/file\n  \"%s\"", config.Filename_incl->Data()));
				return;
			}
			BOSSOutputLoader data(config.Filename_data, false, false);
			if(data.IsZombie()) {
				TerminalIO::PrintFatalError(Form("Failed to load directory/file\n  \"%s\"", config.Filename_data->Data()));
				return;
			}

		/// -# Print three cut flows.

			std::cout << std::endl << "Cut flow for EXCLUSIVE Monte Carlo data set" << std::endl;
			excl.PrintCutFlow();
			std::cout << std::endl << "Cut flow for INCLUSIVE Monte Carlo data set" << std::endl;
			incl.PrintCutFlow();
			std::cout << std::endl << "Cut flow for MEASUREMENT data set" << std::endl;
			data.PrintCutFlow();

		/// -# Set global plotting style based on loaded configuration settings.
			if(!config.Do_PlotStatsLegend) gStyle->SetOptStat(0);
			
		/// -# Plot branches without fits
			for(auto &options : *config.ListOfbranches_excl) excl.Draw(options);
			for(auto &options : *config.ListOfbranches_incl) incl.Draw(options);
			for(auto &options : *config.ListOfbranches_data) data.Draw(options);

		/// -# Perform fit over exclusive Monte Carlo data sets.
			// * Particles to reconstruct
				std::vector<ReconstructedParticle*> particles;
				for(auto &fit : *config.ExclFits) particles.push_back(&fit.first);

			// * Create invariant mass histogram
				std::vector<TH1F*> histograms;
				for(auto &fit : *config.ExclFits) {
					histograms.push_back(excl[fit.second.TreeName()].GetInvariantMassHistogram(fit.second, fit.first));
				}

			// * Fit double gaussian
				int i = 0;
				for(auto &fit : *config.ExclFits) {
					/// @todo Trouble with the number of polynomials... Somehow add to a fit object (extension of `BranchPlotOptions`).
					FitDoubleGaussian(histograms[i], *particles[i], 2, fit.second.LogXYZ());
					++i;
				}

	}


	/// Main function that is called when executing the executable compiled using e.g. `g++`.
	int main(int argc, char *argv[])
	{
		if(argc > 2) {
			cout << "FATAL ERROR: Cannot run this macro with more than one argument" << endl;
			return 1;
		}
		if(argc==1) AnalyseBOSSOutput();
		else        AnalyseBOSSOutput(argv[1]);
		return 0;
	}


/// @}