/// @addtogroup BOSS_Afterburner_scripts
/// @{
// * ======================================== * //
// * ------- LIBRARIES AND NAMESPACES ------- * //
// * ======================================== * //

	#include "ConfigLoader_Plot.h"
	#include "BOSSOutputLoader.h"
	#include "CommonFunctions.h"
	#include "TStyle.h"
	#include <fstream>
	#include <iostream>

	using namespace CommonFunctions;
	using namespace CommonFunctions::Draw;
	using namespace CommonFunctions::Fit;
	using namespace RooFit;
	using namespace Settings;
	using namespace std;



// * ============================= * //
// * ------- MAIN FUNCTION ------- * //
// * ============================= * //


	/// Simple script that allows to quickly investigate the contents of a ROOT file that has been produced with BOSS.
		/// @author   Remco de Boer 雷穆克 (r.e.deboer@students.uu.nl or remco.de.boer@ihep.ac.cn)
		/// @date     November 5th, 2018
	void AnalyseBOSSOutput(const char* configuration_file="configs/debug.config")
	{
		/// -# Attempt to load `configuration_file`.
			ConfigLoader_Plot config(configuration_file, false);

		/// -# Attempt to load input ROOT file as a `BOSSOutputLoader` object).
			BOSSOutputLoader file(config.fInputFilename, config.fPrintBranches, config.fPrintAverages);
			if(file.IsZombie()) {
				TerminalIO::PrintFatalError(Form("Failed to load ROOT file\n  \"%s\"", config.fInputFilename->Data()));
				return;
			}

		/// -# Print cut flow.
			file.PrintCutFlow();

		/// -# Set global plotting style based on loaded configuration settings.
			if(!config.fPlotstats) gStyle->SetOptStat(0);
			
		/// -# Plot branches without fits
			TString logY; if(config.fLogY) logY += "y";
			TString logZ; if(config.fLogZ) logZ += "z";
			for(auto &options : *config.fListOfbranches) file.Draw(options);
			if(config.fPureplot) {
				if(config.fDraw_mult) {
					for(auto tree = file.GetChains().begin(); tree != file.GetChains().end(); ++tree) {
						TString name(tree->second.GetChain().GetName());
						if(name.BeginsWith("mult")) tree->second.DrawAndSaveAllMultiplicityBranches(logY, "");
					}
				} if(config.fDraw_mctruth) {
					if(config.fSetranges) {
						file.Draw("mctruth", "E", 150, 0., 3.5,  "E1", logY);
						file.Draw("mctruth", "p", 150, 0., 1.25, "E1", logY);
					} else {
						file.Draw("mctruth", "E", "E1", logY);
						file.Draw("mctruth", "p", "E1", logY);
					}
				} if(config.fDraw_vertex) {
					if(config.fSetranges) {
						file.Draw("vertex", "vy0:vx0", "colz", logZ);
						file.Draw("vertex", "vz0:vx0", "colz", logZ);
						file.Draw("vertex", "vz0:vy0", "colz", logZ);
					} else {
						file.Draw("vertex", "vx0", "vy0", 60, -.154, -.146,  40,  .08,   .122, "colz", logZ);
						file.Draw("vertex", "vx0", "vz0", 60, -.4,    .5,    60,  .08,   .122, "colz", logZ);
						file.Draw("vertex", "vy0", "vz0", 60, -.4,    .5,    40, -.154, -.146, "colz", logZ);
					}
				} if(config.fDraw_tof) {
					if(config.fSetranges) {
						file.Draw("ToFIB", "p", "tof", 120, 2., 15., 80, 0., 1.5, "colz", logZ);
						file.Draw("ToFOB", "p", "tof", 120, 2., 15., 80, 0., 1.5, "colz", logZ);
					} else {
						file.Draw("ToFIB", "tof:p", "colz", logZ);
						file.Draw("ToFOB", "tof:p", "colz", logZ);
					}
				} if(config.fDraw_fit) {
					if(config.fSetranges) {
						/// -# Draw main invariant mass distributions
							file.Draw("fit4c_all",  "mphi", "mD0", 240, .7, 2., 160, .9, 2.1, "colz", logZ);
							file.Draw("fit4c_all",  "mD0",   400,  .7,    2.,      "E1", logY);
							file.Draw("fit4c_all",  "mJpsi", 400, 3.0967, 3.09685, "E1", logY);
							file.Draw("fit4c_all",  "mphi",  400,  .97,   1.7,     "E1", logY);
							file.Draw("fit4c_best", "mphi", "mD0", 240, .7, 2., 160, .9, 2.1, "colz", logZ);
							file.Draw("fit4c_best", "mD0",   400,  .7,    2.,      "E1", logY);
							file.Draw("fit4c_best", "mJpsi", 400, 3.0967, 3.09685, "E1", logY);
							file.Draw("fit4c_best", "mphi",  400,  .97,   1.7,     "E1", logY);
							// file.Draw("fit_mc",     "mphi", "mD0", 240, .7, 2., 160, .9, 2.1, "colz", logZ);
							// file.Draw("fit_mc",     "mD0",   400,  .7,    2.,      "E1", logY);
							// file.Draw("fit_mc",     "mJpsi", 400, 3.0967, 3.09685, "E1", logY);
							// file.Draw("fit_mc",     "mphi",  400,  .97,   1.7,     "E1", logY);
						/// -# Draw invariant mass distributions with cuts applied on the other candidate
							DrawAndSave(&file["fit4c_all"].GetChain(), "mphi", "mD0>1.5",  "E1", logY);
							DrawAndSave(&file["fit4c_all"].GetChain(), "mD0",  "mphi<1.1", "E1", logY);
							DrawAndSave(&file["fit4c_all"].GetChain(), "mD0:mphi",  "mD0>1.5&&mphi<1.1", "colz", logZ);
						/// -# Draw 3-momentum distributions
							file.Draw("fit4c_all",  "pD0",  400, .4, 1.15, "E1", logY);
							file.Draw("fit4c_all",  "pphi", 400, .4, 1.15, "E1", logY);
							file.Draw("fit4c_best", "pD0",  400, .4, 1.15, "E1", logY);
							file.Draw("fit4c_best", "pphi", 400, .4, 1.15, "E1", logY);
						/// -# Draw invariant mass versus 3-momentum
							file.Draw("fit4c_all",  "mD0",  "pD0",  240, .4, 1.15, 160, .97, 1.7, "colz", logZ);
							file.Draw("fit4c_all",  "mphi", "pphi", 240, .4, 1.15, 160,  .7,  2., "colz", logZ);
							file.Draw("fit4c_best", "mD0",  "pD0",  240, .4, 1.15, 160, .97, 1.7, "colz", logZ);
							file.Draw("fit4c_best", "mphi", "pphi", 240, .4, 1.15, 160,  .7,  2., "colz", logZ);
							file.Draw("fit4c_all",  "mD0:pD0",   "colz", logZ);
							file.Draw("fit4c_all",  "mphi:pphi", "colz", logZ);
							file.Draw("fit4c_best", "mD0:pD0",   "colz", logZ);
							file.Draw("fit4c_best", "mphi:pphi", "colz", logZ);
					} else {
						/// -# Draw main invariant mass distributions
							file.Draw("fit4c_all",  "mD0",   "E1", logY);
							file.Draw("fit4c_all",  "mJpsi", "E1", logY);
							file.Draw("fit4c_all",  "mphi",  "E1", logY);
							file.Draw("fit4c_best", "mD0",   "E1", logY);
							file.Draw("fit4c_best", "mJpsi", "E1", logY);
							file.Draw("fit4c_best", "mphi",  "E1", logY);
						/// -# Draw invariant mass distributions with cuts applied on the other candidate
							DrawAndSave(&file["fit4c_all"].GetChain(), "mphi", "mD0>1.5",  "E1", logY);
							DrawAndSave(&file["fit4c_all"].GetChain(), "mD0",  "mphi<1.1", "E1", logY);
							DrawAndSave(&file["fit4c_all"].GetChain(), "mD0:mphi",  "mD0>1.5&&mphi<1.1", "colz", logZ);
						/// -# Draw 3-momentum distributions
							file.Draw("fit4c_all",  "pD0",  "E1", logY);
							file.Draw("fit4c_all",  "pphi", "E1", logY);
							file.Draw("fit4c_best", "pD0",  "E1", logY);
							file.Draw("fit4c_best", "pphi", "E1", logY);
						/// -# Draw invariant mass versus 3-momentum
							file.Draw("fit4c_all",  "mD0:pD0",   "colz", logZ);
							file.Draw("fit4c_all",  "mphi:pphi", "colz", logZ);
							file.Draw("fit4c_best", "mD0:pD0",   "colz", logZ);
							file.Draw("fit4c_best", "mphi:pphi", "colz", logZ);
					}
				}
			}

		/// -# Perform and plot fits
			if(config.fFitplots) {
			// * Particles to reconstruct
				ReconstructedParticle D0  (421, "K^{-}#pi^{+}");
				ReconstructedParticle phi (333, "K^{+}K^{-}");
				ReconstructedParticle Jpsi(443, "#D^{0}#phi");

			// * Create invariant mass histogram
				TH1F* hist_D0  = file["fit4c"].GetInvariantMassHistogram("mD0",   D0,   500);
				TH1F* hist_phi = file["fit4c"].GetInvariantMassHistogram("mphi",  phi,  500);
				// TH1F* hist_Jpsi = file["fit4c"].GetInvariantMassHistogram("mJpsi", Jpsi, 2000); //! useless plot

			// * Fit double gaussian
				if(config.fDo_gauss) {
					FitDoubleGaussian(hist_D0,   D0,   0, logY);
					FitDoubleGaussian(hist_phi,  phi,  2, logY);
					// FitDoubleGaussian(hist_Jpsi, Jpsi, 0, gLogY.data()); //! useless plot
				}

			// * Fit Breit-Wigner convoluted with singe Gaussian
				if(config.fDo_conv_s) {
					FitBWGaussianConvolution(hist_D0,   D0,   0, logY);
					FitBWGaussianConvolution(hist_phi,  phi,  2, logY);
					// FitBWGaussianConvolution(hist_Jpsi, Jpsi, 2, logY); //! useless plot
				}

			// * Fit Breit-Wigner convoluted with double Gaussian
				if(config.fDo_conv_s) {
					FitBWDoubleGaussianConvolution(hist_D0,   D0,   0, logY);
					FitBWDoubleGaussianConvolution(hist_phi,  phi,  2, logY);
					// FitBWDoubleGaussianConvolution(hist_Jpsi, Jpsi, 2, logY); //! useless plot
				}

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