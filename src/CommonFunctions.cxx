// * ========================= * //
// * ------- LIBRARIES ------- * //
// * ========================= * //
	#include "CommonFunctions.h"
	#include "RooAddPdf.h"
	#include "RooBreitWigner.h"
	#include "RooChebychev.h"
	#include "RooCmdArg.h"
	#include "RooFFTConvPdf.h"
	#include "RooFitResult.h"
	#include "RooGaussian.h"
	#include "RooLinkedList.h"
	#include "RooPlot.h"
	#include "RooPolynomial.h"
	#include "TRegexp.h"
	#include "TSystem.h"
	#include <stdio.h>
	#include <utility> // for shared pointers



// * ================================== * //
// * ------- SUB-NAMESPACE DRAW ------- * //
// * ================================== * //


	/// Auxiliary function that is used by the more specific `DrawAndSave` functions for `TH1D` and `TH2D`.
		/// @param hist Histogram that you want to draw and save.
		/// @param saveas Filename that the output file name should have. See `CommonFunctions::CommonFunctions::File::SetOutputFilename` for more information.
		/// @param opt Draw options.
		/// @param logScale If this argument contains an `'x'`, the \f$x\f$-scale will be set to log scale (same for `'y'` and `'z'`).
	void CommonFunctions::Draw::DrawAndSave(TH1 &hist, const char* saveas, Option_t* opt, TString logScale)
	{
		hist.Draw(opt);
		SaveCanvas(saveas, gPad, logScale);
	}


	/// Draw and save a 1D distribution (output folder is determined from `FrameworkSettings.h`).
		/// @param hist One-dimenational histogram that you would like to draw and save.
		/// @param saveas Filename that the output file name should have. See `CommonFunctions::CommonFunctions::File::SetOutputFilename` for more information.
		/// @param logScale If this argument contains an `'x'`, the \f$x\f$-scale will be set to log scale (same for `'y'` and `'z'`).
	void CommonFunctions::Draw::DrawAndSave(TH1D &hist, const char* saveas, TString logScale)
	{
		DrawAndSave(hist, saveas, "ep", logScale);
	}


	/// Draw and save a 2D distribution (output folder is determined from `FrameworkSettings.h`).
		/// @param hist Two-dimenational histogram that you would like to draw and save.
		/// @param saveas Filename that the output file name should have. See `CommonFunctions::CommonFunctions::File::SetOutputFilename` for more information.
		/// @param logScale If this argument contains an `'x'`, the \f$x\f$-scale will be set to log scale (same for `'y'` and `'z'`).
	void CommonFunctions::Draw::DrawAndSave(TH2D &hist, const char* saveas, TString logScale)
	{
		DrawAndSave(hist, saveas, "colz", logScale);
	}


	/// Function that allows you to create and save a quick sketch of a `TTree` branch.
		/// @param tree The `TTree` of which you want to draw and save a branch.
		/// @param varexp The branches that you want to plot should be in this parameter. See https://root.cern.ch/doc/master/classTTree.html#a8a2b55624f48451d7ab0fc3c70bfe8d7 for the syntax.
		/// @param selection Cuts on the branch. See https://root.cern.ch/doc/master/classTTree.html#a8a2b55624f48451d7ab0fc3c70bfe8d7 for the syntax.
		/// @param opt Draw options.
		/// @param logScale If this argument contains an `'x'`, the \f$x\f$-scale will be set to log scale (same for `'y'` and `'z'`).
	void CommonFunctions::Draw::DrawAndSave(TTree* tree, const char* varexp, const char* selection, Option_t* opt, TString logScale)
	{
		tree->Draw(varexp, selection, opt);
		SaveCanvas(Form("%s_%s", tree->GetName(), varexp), gPad, logScale);
	}


	/// Helper function that allows you to save a `TPad` or `TCanvas`.
		/// @param pad Pointer to the `TPad` that you want to save. **Can also be a `TCanvas`.**
		/// @param saveas Filename that the output file name should have. See `CommonFunctions::CommonFunctions::File::SetOutputFilename` for more information.
		/// @param logScale If this argument contains an `'x'`, the \f$x\f$-scale will be set to log scale (same for `'y'` and `'z'`).
	void CommonFunctions::Draw::SaveCanvas(const char *saveas, TVirtualPad *pad, TString logScale)
	{
		if(CommonFunctions::TerminalIO::IsEmptyPtr(pad)) return;
		SetLogScale(logScale, pad);
		pad->SaveAs(CommonFunctions::File::SetOutputFilename(saveas));
	}


	/// *TEMPORARY* function that serves as a fix for the bug that causes the wrong best pair to be stored.
		/// @todo Consider removing `CommonFunctions::Draw::DrawDifference`.
	void CommonFunctions::Draw::DrawDifference(TH1 *histToDraw, TH1 *histToSubtract, Option_t* opt, const char* setLog)
	{
		if(!gPad) return;
		gPad->Clear();
		histToDraw->Scale(-1.);
		histToDraw->Add(histToSubtract);
		histToSubtract->SetLineColor(kWhite);
		// histToSubtract->SetMarkerColor(kWhite);
		TString option(opt);
		if(dynamic_cast<TH1F*>(histToDraw)) {
			histToSubtract->Draw(option.Data());
			option += "";
		}
		histToDraw->Draw(option.Data());
		CommonFunctions::Draw::SaveCanvas(Form("%s", histToDraw->GetName()), gPad, setLog);
	}


	/// Helper function that allows you to set the log scale of a `TPad` or `TCanvas`.
		/// @param logScale Which axes do you want to set? Just mention e.g. `"xy"` if you want to have the x and y axes in logaritmic scale. This opt is not capital sensitive.
		/// @param pad Pointer to the `TPad` of which you want to set the log scale. **Can also be a `TCanvas`.**
	void CommonFunctions::Draw::SetLogScale(TString logScale, TVirtualPad *pad)
	{
		// * Check arguments * //
		if(logScale.EqualTo("")) return;
		if(CommonFunctions::TerminalIO::IsEmptyPtr(pad)) return;
		logScale.ToLower();
		// * Set log scale * //
		pad->SetLogx(0); pad->SetLogy(0); pad->SetLogz(0);
		if(logScale.Contains("x")) pad->SetLogx();
		if(logScale.Contains("y")) pad->SetLogy();
		if(logScale.Contains("z")) pad->SetLogz();
	}



// * ================================== * //
// * ------- SUB-NAMESPACE FILE ------- * //
// * ================================== * //


	/// This function creates a default output file name according to the `FrameworkSettings`. The file name consists of the plots output director, with the extension as subfolder and then the filename you want.
		/// @param filenameWithoutExt Output file name to which `gPlotOutputDir` in `FrameworkSettings.h` will be prepended and **`gExtension` will be appended**.
	/// @return Full path of the resulting output file name.
	const char* CommonFunctions::File::GetOutputFilename(const char* filenameWithoutExt)
	{
		return Form("%s/%s/%s.%s", Settings::File::gPlotOutputDir, Settings::File::gExtension, filenameWithoutExt, Settings::File::gExtension);
	}


	/// This function creates a file name from the settings in `FrameworkSettings.h` and from the file name (without extension) that you feed it.
	/// You don't have to worry about creating the output directory, as it is created automatically from the slashes `/` in the resulting file name
		/// @param filenameWithoutExt Output file name to which `gPlotOutputDir` in `FrameworkSettings.h` will be prepended and `gExtension` will be appended.
	/// @return Full path of the resulting output file name.
	const char* CommonFunctions::File::SetOutputFilename(const char* filenameWithoutExt)
	{
		// * Create full path * //
		const char* fullpath = GetOutputFilename(filenameWithoutExt);
		// * Create dirname * //
		TString dirname(fullpath);
		Int_t pos = dirname.Last('/');
		dirname.Resize(pos);
		gSystem->mkdir(dirname.Data(), true); // true: recursively create directory
		// * Get basename * //
		TString basename(fullpath);
		basename.Remove(0, pos+1);
		// * Create file name and return * //
		return Form("%s/%s", dirname.Data(), basename.Data());
	}



// * ================================= * //
// * ------- SUB-NAMESPACE FIT ------- * //
// * ================================= * //


	/// Create a `RooDataHist` specifically for resonstructing a certain particle (`ReconstructedParticle`).
	RooDataHist CommonFunctions::Fit::CreateRooFitInvMassDistr(TH1F *hist, const RooRealVar &var, const ReconstructedParticle& particle) {
		RooDataHist distr(
			Form("%scandidate_RooDataHist", particle.GetName()),
			hist->GetTitle(), var, RooFit::Import(*hist));
		return distr;
	}


	/// Create a `RooRealVar` specifically for resonstructing a certain particle (`ReconstructedParticle`).
	RooRealVar CommonFunctions::Fit::CreateRooFitInvMassVar(const ReconstructedParticle& particle)
	{
		RooRealVar var(
			Form("#it{M}_{%s}", particle.GetDaughterLabel()),
			Form("#it{M}_{%s} (GeV/#it{c}^{2})", particle.GetDaughterLabel()),
			particle.PlotFrom(),
			particle.PlotUntil()
		);
		return var;
	}


	/// Fit the sum of two Gaussian functions on a invariant mass distrubution. The mean of the two Gaussian is in both cases taken to be the mass of the particle to be reconstructed.
	/// For a pure particle signal, that is, without backround @b and without a physical particle width, the width of the two Gaussians characterises the resolution of the detector.
	/// See https://root.cern.ch/roofit-20-minutes for an instructive tutorial.
		/// @param hist Invariant mass histogram that you would like to fit.
		/// @param particle Hypothesis particle: which particle are you reconstructing? All analysis parameters, such as estimates for Gaussian widths, are contained within this object.
		/// @param numPolynomials The degree of the polynomial that describes the background.
		/// @param logScale If this argument contains an `'x'`, the \f$x\f$-scale will be set to log scale (same for `'y'` and `'z'`).
	void CommonFunctions::Fit::FitBWGaussianConvolution(TH1F *hist, const ReconstructedParticle& particle, const UChar_t numPolynomials, TString logScale)
	{

		// * Create RooFit variable and data distribution * //
			RooRealVar fRooRealVar = CreateRooFitInvMassVar(particle);
			RooDataHist fRooDataHist = CreateRooFitInvMassDistr(hist, fRooRealVar, particle);

		// * Create double Gaussian function * //
			RooRealVar m0("GaussianMeanZero", "GaussianMeanZero", 0.);
			RooRealVar fSigma("#sigma_{gauss}", Form("%s Gaussian width", particle.GetNameLaTeX()),
					particle.GetSingleGaussianWidth(),
					Settings::Fit::gSigmaScaleFactorLow * particle.GetSingleGaussianWidth(),
					Settings::Fit::gSigmaScaleFactorUp  * particle.GetSingleGaussianWidth());
			RooGaussian fGaussian("gauss",
				Form("Gaussian PDF for #it{M}_{%s} distribution", particle.GetDaughterLabel()),
				fRooRealVar, m0, fSigma);

		// * Create Breit-Wigner/Cauchy distribution * //
			RooRealVar mean(
				Form("m_{%s}", particle.GetNameLaTeX()),
				Form("%s mass", particle.GetNameLaTeX()),
				particle.GetMass(), particle.GetLowerMass(), particle.GetUpperMass());
			RooRealVar width("#sigma_{BW}", Form("%s BW width", particle.GetNameLaTeX()),
				particle.GetBWConvolutedWidth(), 0., 10.*particle.GetBWConvolutedWidth());
			RooBreitWigner bw("breitwigner",
				Form("Breit-Wigner PDF for #it{M}_{%s} distribution", particle.GetDaughterLabel()),
				fRooRealVar, mean, width);

		// * Convolute * //
			RooFFTConvPdf signal("convolution", "convolution", fRooRealVar, bw, fGaussian);
			RooRealVar n("N_{gaus1}", "N_{gaus1}", 1e2, 0., 1e6);
			RooArgList fComponents(signal);
			RooArgList fNContributions(n);

		// * Add polynomial background if required * //
			RooArgList fBckParameters;
			for(UChar_t i = 0; i <= numPolynomials; ++i) {
				auto p = new RooRealVar(Form("p%u", i), Form("p%u", i), 0., -1e6, 1e6);
				fBckParameters.add(*p);
			}
			RooChebychev fPolBackground("polBkg",
				Form("Polynomial-%u background", numPolynomials),
				fRooRealVar, fBckParameters);
			RooRealVar fSigToBckRatio(
				Form("N_{pol%u}", numPolynomials),
				Form("N_{pol%u}", numPolynomials),
				0., 0., 1e5);
			if(numPolynomials) {
				fComponents.add(fPolBackground);
				fNContributions.add(fSigToBckRatio);
			}

		// * Add the components and fit * //
			RooAddPdf fFullShape("full_shape", "Double gaussian + background", fComponents, fNContributions);
			fFullShape.fitTo(
				fRooDataHist,
				RooFit::Range(particle.FitFrom(), particle.FitUntil()));

		// * Plot results and save * //
			RooPlot *frame = fRooRealVar.frame(); // create a frame to draw
			frame->SetAxisRange(particle.PlotFrom(), particle.PlotUntil());
			fRooDataHist.plotOn(frame, // draw distribution
				RooFit::LineWidth(2), RooFit::LineColor(kBlue+2), RooFit::LineWidth(1),
				RooFit::MarkerColor(kBlue+2), RooFit::MarkerSize(.5));
			fFullShape.plotOn(frame, RooFit::LineWidth(2), RooFit::LineColor(kBlack));
			if(numPolynomials) {
				fFullShape.plotOn(frame, RooFit::Components(signal), // draw signal
					RooFit::LineWidth(1), RooFit::LineColor(kRed-4));
				fFullShape.plotOn(frame, RooFit::Components(fPolBackground), // draw background
					RooFit::LineStyle(kDashed), RooFit::LineWidth(1), RooFit::LineColor(kGray));
			}
			fFullShape.paramOn(frame, RooFit::Layout(.56, .98, .92));

		// * Write fitted histograms * //
			TString pname = particle.GetName();
			pname.ReplaceAll("/", ""); // in case of for instance "J/psi"
			CommonFunctions::Draw::DrawAndSave(Form("ConvBWSingleGauss_%s", pname.Data()), "", logScale, frame);
			delete frame;

	}


	/// Fit the sum of two Gaussian functions on a invariant mass distrubution. The mean of the two Gaussian is in both cases taken to be the mass of the particle to be reconstructed.
	/// For a pure particle signal, that is, without backround @b and without a physical particle width, the width of the two Gaussians characterises the resolution of the detector.
	/// See https://root.cern.ch/roofit-20-minutes for an instructive tutorial.
		/// @param hist Invariant mass histogram that you would like to fit
		/// @param particle Hypothesis particle: which particle are you reconstructing? All analysis parameters, such as estimates for Gaussian widths, are contained within this object.
		/// @param numPolynomials The degree of the polynomial that describes the background.
		/// @param logScale If this argument contains an `'x'`, the \f$x\f$-scale will be set to log scale (same for `'y'` and `'z'`).
	void CommonFunctions::Fit::FitBWDoubleGaussianConvolution(TH1F *hist, const ReconstructedParticle& particle, const UChar_t numPolynomials, TString logScale)
	{

		// * Create RooFit variable and data distribution * //
			RooRealVar fRooRealVar = CreateRooFitInvMassVar(particle);
			RooDataHist fRooDataHist = CreateRooFitInvMassDistr(hist, fRooRealVar, particle);

		// * Create double Gaussian function * //
			RooRealVar m0("GaussianMeanZero", "GaussianMeanZero", 0.);
			RooRealVar fSigma1("#sigma_{1}", Form("%s width 1", particle.GetNameLaTeX()),
				particle.GetGaussianSmallWidth()); //! width is fixed
			RooRealVar fSigma2("#sigma_{2}", Form("%s width 2", particle.GetNameLaTeX()),
				particle.GetGaussianWideWidth()); //! width is fixed
			RooGaussian fGaussian1("gauss1",
				Form("Gaussian PDF 1 for #it{M}_{%s} distribution", particle.GetDaughterLabel()),
				fRooRealVar, m0, fSigma1);
			RooGaussian fGaussian2("gauss2",
				Form("Gaussian PDF 2 for #it{M}_{%s} distribution", particle.GetDaughterLabel()),
				fRooRealVar, m0, fSigma2);

		// * Add the Gaussian components * //
			RooRealVar fNGauss1("N_{gaus1}", "N_{gaus1}", 1e2, 0., 1e6);
			RooRealVar fNGauss2("N_{gaus2}", "N_{gaus2}", 1e4, 0., 1e6);
			RooRealVar ratio("N_{gaus1} / N_{gaus2}", "Ratio between the two Gaussian pdfs", .8, 0., 1.);
			RooAddPdf doublegauss("double_gaussian", "Double gaussian",
				RooArgList(fGaussian1, fGaussian2), RooArgList(ratio));

		// * Create Breit-Wigner/Cauchy distribution * //
			RooRealVar mean(
				Form("m_{%s}", particle.GetNameLaTeX()),
				Form("%s mass", particle.GetNameLaTeX()),
				particle.GetMass(), particle.GetLowerMass(), particle.GetUpperMass());
			RooRealVar width("#sigma_{BW}", Form("%s BW width", particle.GetNameLaTeX()),
				particle.GetBWConvolutedWidth(), 0., 10.*particle.GetBWConvolutedWidth());
			RooBreitWigner bw("breitwigner",
				Form("Breit-Wigner PDF for #it{M}_{%s} distribution", particle.GetDaughterLabel()),
				fRooRealVar, mean, width);

		// * Convolute * //
			RooFFTConvPdf signal("convolution", "convolution", fRooRealVar, bw, doublegauss);
			RooRealVar n("N_{gaus1}", "N_{gaus1}", 1e2, 0., 1e6);
			RooArgList fComponents(signal);
			RooArgList fNContributions(n);

		// * Add polynomial background if required * //
			RooArgList fBckParameters;
			for(UChar_t i = 0; i <= numPolynomials; ++i) {
				auto p = new RooRealVar(Form("p%u", i), Form("p%u", i), 0., -1e6, 1e6);
				fBckParameters.add(*p);
			}
			RooChebychev fPolBackground("polBkg",
				Form("Polynomial-%u background", numPolynomials),
				fRooRealVar, fBckParameters);
			RooRealVar fSigToBckRatio(
				Form("N_{pol%u}", numPolynomials),
				Form("N_{pol%u}", numPolynomials),
				0., 0., 1e5);
			if(numPolynomials) {
				fComponents.add(fPolBackground);
				fNContributions.add(fSigToBckRatio);
			}

		// * Add the components and fit * //
			RooAddPdf fFullShape("full_shape", "Double gaussian + background", fComponents, fNContributions);
			fFullShape.fitTo(
				fRooDataHist,
				RooFit::Range(particle.FitFrom(), particle.FitUntil()));

		// * Plot results and save * //
			RooPlot *frame = fRooRealVar.frame(); // create a frame to draw
			frame->SetAxisRange(particle.PlotFrom(), particle.PlotUntil());
			fRooDataHist.plotOn(frame, // draw distribution
				RooFit::LineWidth(2), RooFit::LineColor(kBlue+2), RooFit::LineWidth(1),
				RooFit::MarkerColor(kBlue+2), RooFit::MarkerSize(.5));
			fFullShape.plotOn(frame, RooFit::LineWidth(2), RooFit::LineColor(kBlack));
			if(numPolynomials) {
				fFullShape.plotOn(frame, RooFit::Components(signal), // draw signal
					RooFit::LineWidth(1), RooFit::LineColor(kRed-4));
				fFullShape.plotOn(frame, RooFit::Components(fPolBackground), // draw background
					RooFit::LineStyle(kDashed), RooFit::LineWidth(1), RooFit::LineColor(kGray));
			}
			fFullShape.paramOn(frame, RooFit::Layout(.56, .98, .92));

		// * Write fitted histograms * //
			TString pname = particle.GetName();
			pname.ReplaceAll("/", ""); // in case of for instance "J/psi"
			CommonFunctions::Draw::DrawAndSave(Form("ConvBWDoubleGauss_%s", pname.Data()), "", logScale, frame);
			delete frame;

	}


	/// Fit the sum of two Gaussian functions on a invariant mass distrubution. The mean of the two Gaussian is in both cases taken to be the mass of the particle to be reconstructed.
	/// For a pure particle signal, that is, without backround @b and without a physical particle width, the width of the two Gaussians characterises the resolution of the detector.
	/// See https://root.cern.ch/roofit-20-minutes for an instructive tutorial.
		/// @param hist Invariant mass histogram that you would like to fit
		/// @param particle Hypothesis particle: which particle are you reconstructing? All analysis parameters, such as estimates for Gaussian widths, are contained within this object.
		/// @param numPolynomials The degree of the polynomial that describes the background.
	void CommonFunctions::Fit::FitBreitWigner(TH1F *hist, const ReconstructedParticle& particle, const UChar_t numPolynomials)
	{

		// * Create RooFit variable and data distribution * //
			RooRealVar fRooRealVar = CreateRooFitInvMassVar(particle);
			RooDataHist fRooDataHist = CreateRooFitInvMassDistr(hist, fRooRealVar, particle);

		// * Create Breit-Wigner function and fit * //
			RooRealVar mean(
				Form("m_{%s}", particle.GetNameLaTeX()),
				Form("%s mass", particle.GetNameLaTeX()),
				particle.GetMass(), particle.GetLowerMass(), particle.GetUpperMass());
			RooRealVar width("width", Form("%s width", particle.GetNameLaTeX()),
				particle.GetBWPureWidth(), 0., 100.*particle.GetBWPureWidth());
			RooGaussian signal("breitwigner",
				Form("Breit-Wigner PDF for #it{M}_{%s} distribution", particle.GetDaughterLabel()),
				fRooRealVar, mean, width);
			RooRealVar n("N_{BW}", "N_{BW}", 1e2, 0., 1e6);
			RooArgList fComponents(signal);
			RooArgList fNContributions(n);

		// * Add polynomial background if required * //
			RooArgList fBckParameters;
			for(UChar_t i = 0; i <= numPolynomials; ++i) {
				auto p = new RooRealVar(Form("p%u", i), Form("p%u", i), 0., -1e6, 1e6);
				fBckParameters.add(*p);
			}
			RooChebychev fPolBackground("polBkg",
				Form("Polynomial-%u background", numPolynomials),
				fRooRealVar, fBckParameters);
			RooRealVar fSigToBckRatio(
				Form("N_{pol%u}", numPolynomials),
				Form("N_{pol%u}", numPolynomials),
				0., 0., 1e5);
			if(numPolynomials) {
				fComponents.add(fPolBackground);
				fNContributions.add(fSigToBckRatio);
			}

		// * Add the components and fit * //
			RooAddPdf fFullShape("full_shape", "Breit-Wigner + background", fComponents, fNContributions);
			fFullShape.fitTo(
				fRooDataHist,
				RooFit::Range(particle.FitFrom(), particle.FitUntil()));

		// * Plot results and save * //
			RooPlot *frame = fRooRealVar.frame(); // create a frame to draw
			frame->SetAxisRange(particle.PlotFrom(), particle.PlotUntil());
			fRooDataHist.plotOn(frame, // draw distribution
				RooFit::LineWidth(2), RooFit::LineColor(kBlue+2), RooFit::LineWidth(1),
				RooFit::MarkerColor(kBlue+2), RooFit::MarkerSize(.5));
			fFullShape.plotOn(frame, RooFit::LineWidth(2), RooFit::LineColor(kBlack));
			if(numPolynomials) {
				fFullShape.plotOn(frame, RooFit::Components(signal), // draw Breit-Wigner
					RooFit::LineWidth(1), RooFit::LineColor(kRed-4));
				fFullShape.plotOn(frame, RooFit::Components(fPolBackground), // draw background
					RooFit::LineStyle(kDashed), RooFit::LineWidth(1), RooFit::LineColor(kGray));
			}
			fFullShape.paramOn(frame, RooFit::Layout(.56, .98, .92));

		// * Write fitted histograms * //
			TCanvas c;
			c.SetBatch();
			frame->Draw();
			CommonFunctions::Draw::SaveCanvas(Form("BreitWigner_%s", particle.GetName()), &c);

	}


	/// Fit the sum of two Gaussian functions on a invariant mass distrubution. The mean of the two Gaussian is in both cases taken to be the mass of the particle to be reconstructed.
	/// For a pure particle signal, that is, without backround @b and without a physical particle width, the width of the two Gaussians characterises the resolution of the detector.
	/// See https://root.cern.ch/roofit-20-minutes for an instructive tutorial.
		/// @param hist Invariant mass histogram that you would like to fit
		/// @param particle Hypothesis particle: which particle are you reconstructing? All analysis parameters, such as estimates for Gaussian widths, are contained within this object.
		/// @param numPolynomials The degree of the polynomial that describes the background.
		/// @param logScale If this argument contains an `'x'`, the \f$x\f$-scale will be set to log scale (same for `'y'` and `'z'`).
	void CommonFunctions::Fit::FitDoubleGaussian(TH1F *hist, const ReconstructedParticle& particle, const UChar_t numPolynomials, TString logScale)
	{
		if(CommonFunctions::TerminalIO::IsEmptyPtr(hist)) return;
		// * DATA MEMBERS * //
			// * FitObject
				std::unique_ptr<RooDataHist> fRooDataHist;
				std::unique_ptr<RooRealVar>  fRooRealVar;
			// * FitObjectDoubleGaussian
				RooArgList fBckParameters;
				RooArgList fComponents;
				RooArgList fNContributions;
				std::unique_ptr<RooAddPdf>     fFullShape;
				std::unique_ptr<RooGaussian>   fGaussian1;
				std::unique_ptr<RooGaussian>   fGaussian2;
				std::unique_ptr<RooChebychev> fPolBackground;
				std::unique_ptr<RooRealVar>    fMean;
				std::unique_ptr<RooRealVar>    fNGauss1;
				std::unique_ptr<RooRealVar>    fNGauss2;
				std::unique_ptr<RooRealVar>    fSigToBckRatio;
				std::unique_ptr<RooRealVar>    fSigma1;
				std::unique_ptr<RooRealVar>    fSigma2;

		// * METHODS * //
		if(true) { // local namespace for beginning of class design
			// * Create RooFit variable and data distribution * //
				fRooRealVar  = std_fix::make_unique<RooRealVar>(CreateRooFitInvMassVar(particle));
				fRooDataHist = std_fix::make_unique<RooDataHist>(CreateRooFitInvMassDistr(hist, *fRooRealVar, particle));

			// * Create Gaussian functions * //
				fMean = std_fix::make_unique<RooRealVar>(
					Form("m_{%s}", particle.GetNameLaTeX()),
					Form("%s mass", particle.GetNameLaTeX()),
					particle.GetMass(), particle.GetLowerMass(), particle.GetUpperMass());
				fSigma1 = std_fix::make_unique<RooRealVar>("#sigma_{1}",
					Form("%s width 1", particle.GetNameLaTeX()),
					particle.GetGaussianSmallWidth(),
					Settings::Fit::gSigmaScaleFactorLow * particle.GetGaussianSmallWidth(),
					Settings::Fit::gSigmaScaleFactorUp  * particle.GetGaussianSmallWidth());
				fSigma2 = std_fix::make_unique<RooRealVar>("#sigma_{2}",
					Form("%s width 2", particle.GetNameLaTeX()),
					particle.GetGaussianWideWidth(),
					Settings::Fit::gSigmaScaleFactorLow * particle.GetGaussianWideWidth(),
					Settings::Fit::gSigmaScaleFactorUp  * particle.GetGaussianWideWidth());
				fGaussian1 = std_fix::make_unique<RooGaussian>("gauss1",
					Form("Gaussian PDF 1 for #it{M}_{%s} distribution", particle.GetDaughterLabel()),
					*fRooRealVar, *fMean, *fSigma1);
				fGaussian2 = std_fix::make_unique<RooGaussian>("gauss2",
					Form("Gaussian PDF 2 for #it{M}_{%s} distribution", particle.GetDaughterLabel()),
					*fRooRealVar, *fMean, *fSigma2);
				fNGauss1 = std_fix::make_unique<RooRealVar>("N_{gaus1}", "N_{gaus1}", 1e2, 0., 1e6);
				fNGauss2 = std_fix::make_unique<RooRealVar>("N_{gaus2}", "N_{gaus2}", 1e4, 0., 1e6);
				fComponents.add(*fGaussian1);
				fComponents.add(*fGaussian2);
				fNContributions.add(*fNGauss1);
				fNContributions.add(*fNGauss2);

			// * Add polynomial background if required * //
				for(UChar_t i = 0; i <= numPolynomials; ++i) {
					fBckParameters.addClone(RooRealVar(Form("p%u", i), Form("p%u", i), 0., -1e6, 1e6));
				}
				fPolBackground = std_fix::make_unique<RooChebychev>("polBkg",
					Form("Polynomial-%u background", numPolynomials),
					*fRooRealVar, fBckParameters);
				fSigToBckRatio = std_fix::make_unique<RooRealVar>(
					Form("N_{pol%u}", numPolynomials),
					Form("N_{pol%u}", numPolynomials),
					0., 0., 1e5);
				if(numPolynomials) {
					fComponents.add(*fPolBackground);
					fNContributions.add(*fSigToBckRatio);
				}

			// * Add the components and fit * //
				fFullShape = std_fix::make_unique<RooAddPdf>("full_shape", "Double gaussian + background", fComponents, fNContributions);
				fFullShape->fitTo(
					*fRooDataHist,
					RooFit::Range(particle.FitFrom(), particle.FitUntil()));

			// * Plot results and save * //
				auto frame = fRooRealVar->frame(); // create a frame to draw
				frame->SetAxisRange(particle.PlotFrom(), particle.PlotUntil());
				fRooDataHist->plotOn(frame, // draw distribution
					RooFit::LineWidth(2), RooFit::LineColor(kBlue+2), RooFit::LineWidth(1),
					RooFit::MarkerColor(kBlue+2), RooFit::MarkerSize(.5));
				fFullShape->plotOn(frame, RooFit::LineWidth(2), RooFit::LineColor(kBlack)); // draw sig+bck fit
				fFullShape->plotOn(frame, RooFit::Components(*fComponents.at(0)), // draw gauss 1
					RooFit::LineWidth(1), RooFit::LineColor(kRed-4));
				fFullShape->plotOn(frame, RooFit::Components(*fComponents.at(1)), // draw gauss 2
					RooFit::LineWidth(1), RooFit::LineColor(kBlue-4));
				if(numPolynomials) {
					fFullShape->plotOn(frame, RooFit::Components(*fPolBackground), // draw background
						RooFit::LineStyle(kDashed), RooFit::LineWidth(1), RooFit::LineColor(kGray));
				}
				fFullShape->paramOn(frame, RooFit::Layout(.56, .98, .92));

			// * Write fitted histograms * //
				TString pname = particle.GetName();
				pname.ReplaceAll("/", ""); // in case of for instance "J/psi"
				CommonFunctions::Draw::DrawAndSave(Form("DoubleGauss_%s", pname.Data()), "", logScale, frame);
				delete frame;
		}

	}



// * ================================== * //
// * ------- SUB-NAMESPACE HIST ------- * //
// * ================================== * //

	/// Create a histogram object especially for invariant mass analysis.
		/// @param particle Axis titles, histogram title, and histogram ranges are determined from this object.
		/// @param nBins Number of bins on the \f$x\f$-axis.
	/// @return A new, <i>empty</i> `TH1D` histogram, ready to be filled from a branch from a `TTree`.
	TH1D CommonFunctions::Hist::CreateInvariantMassHistogram(const ReconstructedParticle& particle, const int nBins)
	{
		return TH1D(
			Form("hist_%s", particle.GetName()),
			Form("Invariant mass for %s candidate;#it{M}_{%s} (GeV/#it{c}^{2});counts",
				particle.GetNameLaTeX(), particle.GetDaughterLabel()),
			nBins, particle.PlotFrom(), particle.PlotUntil());
	}

	/// Set the axis titles of a histogram
		/// @param hist Pointer to the histogram of which you want to change the axis titles.
		/// @param xAxis Title of the \f$x\f$-axis.
		/// @param yAxis Title of the \f$y\f$-axis. Can be left empty.
		/// @param zAxis Title of the \f$z\f$-axis. Can be left empty.
		/// @param update Whether or not to update the `gPad`. Not that this only makes sense if this particular histogram has been recently drawn.
	void CommonFunctions::Hist::SetAxisTitles(TH1* hist, const char* xAxis, const char* yAxis, const char* zAxis, bool update)
	{
		if(CommonFunctions::TerminalIO::IsEmptyPtr(hist)) return;
		if(xAxis) hist->GetXaxis()->SetTitle(xAxis);
		if(yAxis) hist->GetYaxis()->SetTitle(yAxis);
		if(zAxis) hist->GetZaxis()->SetTitle(zAxis);
		if(update && gPad) gPad->Update();
	}



// * ==================================== * //
// * ------- SUB-NAMESPACE STRING ------- * //
// * ==================================== * //


	/// Remove all leading and trailing spaces and tabs characters from a string.
	void CommonFunctions::String::RemoveWhitespaces(TString &input)
	{
		while(input.EndsWith(" ")   || input.EndsWith("\t"))   input.Chop();
		while(input.BeginsWith(" ") || input.BeginsWith("\t")) input.Remove(0, 1);
	}


	/// Check if a string starts with a comment identifier (such as `//` in the case of `C++`).
	const bool CommonFunctions::String::IsComment(TString input)
	{
		RemoveWhitespaces(input);
		if(input.BeginsWith("//")) return true; // C++ line comment
		if(input.BeginsWith("/*")) return true; // C++ begin block comment
		if(input.BeginsWith("*"))  return true; // C++ continue/end block comment
		if(input.BeginsWith("#"))  return true; // bash comment
		if(input.BeginsWith("%%")) return true; // LaTeX comment
		return false;
	}



// * ========================================== * //
// * ------- SUB-NAMESPACE TERMINAL I/O ------- * //
// * ========================================== * //


	/// Print a message in a certain colour.
		/// @param code An [ANSI escape colour code](https://en.wikipedia.org/wiki/ANSI_escape_code). For instance, `31` would be red.
		/// @param message The message you want to print.
		/// @param qualifier If added, this will print `"<qualifier>: " in bold in front of the message.
	void CommonFunctions::TerminalIO::ColourPrint(const char code, const TString &message, const TString &qualifier)
	{
		if(qualifier.Length()) printf("\033[1;%dm%s: \033[0m", code, qualifier.Data());
		printf("\033[%dm%s\033[0m\n", code, message.Data());
	}


	/// Print a **fatal error** message (that is, one that terminates) in red.
	/// @warning Calling this function will terminate the entire programme.
	void CommonFunctions::TerminalIO::PrintFatalError(const TString &message)
	{
		ColourPrint(31, message, "FATAL ERROR");
		std::terminate();
	}


	/// Print a **success** message in green.
	void CommonFunctions::TerminalIO::PrintSuccess(const TString &message)
	{
		ColourPrint(32, message);
	}


	/// Print a **warning** message in yellow.
	void CommonFunctions::TerminalIO::PrintWarning(const TString &message)
	{
		ColourPrint(33, message, "WARNING");
	}



	/// Auxiliary function that you can used in other functions to check if a pointer that you fed it is a nullpointer.
	/// It is useful to use this function if you want to print output when you are incidentally using an empty pointer.
	bool CommonFunctions::TerminalIO::IsEmptyPtr(void* ptr)
	{
		if(!ptr) {
			std::cout << "ERROR: fed a nullpointer" << std::endl;
			return true;
		}
		return false;
	}