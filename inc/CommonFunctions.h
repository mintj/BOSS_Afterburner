#ifndef BOSS_Afterburner_CommonFunctions_H
#define BOSS_Afterburner_CommonFunctions_H


// * ========================= * //
// * ------- LIBRARIES ------- * //
// * ========================= * //

	#include "FrameworkSettings.h"
	#include "Particle.h"
	#include "ReconstructedParticle.h"
	#include "RooDataHist.h"
	#include "RooRealVar.h"
	#include "TCanvas.h"
	#include "TH1.h"
	#include "TH1D.h"
	#include "TH1F.h"
	#include "TH2D.h"
	#include "TString.h"
	#include "TTree.h"
	#include "TVirtualPad.h"
	#include <string>
	#include <unordered_map>



// * ============================ * //
// * ------- DECLARATIONS ------- * //
// * ============================ * //
/// @addtogroup BOSS_Afterburner_functions
/// @{


	/// A namespace that is used to bundle functions that can be used by different analysis scripts.
	/// The idea of sharing these functions is to standardise output. For instance, if a pdf of a histogram is saved through a function defined here, the pdf output can be centrally 'beautified' in this header file.
	/// @author  Remco de Boer 雷穆克 (r.e.deboer@students.uu.nl or remco.de.boer@ihep.ac.cn)
	/// @date    November 20th, 2018
	namespace CommonFunctions
	{
		/// Namespace containing functions related to drawing and saving plots.
		namespace Draw
		{
			template<class... ARGS>
			void DrawAndSaveRecursion(Option_t* opt, ARGS&&... args);
				///< Start recursion for `DrawAndSaveRecursion`.
			template<class TYPE, class... ARGS>
			void DrawAndSaveRecursion(Option_t* opt, TYPE first, ARGS... args);
			template<> inline
			void DrawAndSaveRecursion(Option_t* opt) {};
				///< End recursion for `DrawAndSaveRecursion`. @todo Impossible to compile in `make` framework?

			template<class ...ARGS>
			void DrawAndSave(const char* saveas, Option_t* opt, const char* logScale, ARGS... args);
			void DrawAndSave(TH1 &hist, const char* saveas, Option_t* opt, TString logScale="");
			void DrawAndSave(TH1D &hist, const char* saveas, TString logScale="");
			void DrawAndSave(TH2D &hist, const char* saveas, TString logScale="");
			void DrawAndSave(TTree* tree, const char* varexp, const char* selection, Option_t* opt, TString logScale="");
			void DrawDifference(TH1 *histToDraw, TH1 *histToSubtract, Option_t* opt="E1", const char* setLog="");
			void SaveCanvas(const char *saveas, TVirtualPad *pad=gPad, TString logScale="");
			void SetLogScale(TString logScale="", TVirtualPad *pad=gPad);
		}
		/// Namespace containing functions related to file naming and file I/O.
		namespace File
		{
			const char* GetOutputFilename(const char* filenameWithoutExt);
			const char* SetOutputFilename(const char* filenameWithoutExt);
		}
		/// Namespace containing functions related to performing fits on histograms (in particular related to `RooFit`).
		namespace Fit
		{
			RooDataHist CreateRooFitInvMassDistr(TH1F *hist, const RooRealVar &var, const ReconstructedParticle& particle);
			RooRealVar CreateRooFitInvMassVar(const ReconstructedParticle& particle);
			void FitBWGaussianConvolution(TH1F *hist, const ReconstructedParticle& particle, const UChar_t numPolynomials = 0, TString logScale="");
			void FitBWDoubleGaussianConvolution(TH1F *hist, const ReconstructedParticle& particle, const UChar_t numPolynomials = 0, TString logScale="");
			void FitBreitWigner(TH1F *hist, const ReconstructedParticle& particle, const UChar_t numPolynomials = 0);
			void FitDoubleGaussian(TH1F *hist, const ReconstructedParticle& particle, const UChar_t numPolynomials = 0, TString logScale="");
		}
		/// Namespace containing functions related to generating and modifying histograms.
		namespace Hist
		{
			TH1D CreateInvariantMassHistogram(const ReconstructedParticle& particle, const int nBins = 200);
			void SetAxisTitles(TH1* hist, const char* xAxis, const char* yAxis=nullptr, const char* zAxis=nullptr, bool update=true);
		}
		/// Namespace containing functions related to looping over objects, for instance over the entries in a `TTree`. Functions here either use a `Lambda` as argument or are defined recursively. This is why support for `C++11` is required for the <i>BOSS Afterburner</i>.
		namespace Loop
		{
			template<typename FUNCTOR, typename ...Rest>
			void LoopTree(TTree* tree, FUNCTOR&& lambda, Rest&&... args);
		}
		/// Namespace containing functions related to terminal output.
		namespace Print
		{
			template<typename TYPE>
			const TString CommaFormattedString(TYPE number);
		}
		/// Namespace containing functions for `string` manipulation.
		namespace String
		{
			void RemoveWhitespaces(TString &line);
			const bool IsComment(TString line);
		}
		/// Namespace containing functions related to debugging and making macro's handle errors.
		namespace TerminalIO
		{
			void ColourPrint(const char code, const TString &message, const TString &quantifier="");
			void PrintFatalError(const TString &message);
			void PrintSuccess(const TString &message);
			void PrintWarning(const TString &message);
			bool IsEmptyPtr(void* ptr);
			template<class TYPE> inline
			TYPE& GetFromMap(std::unordered_map<std::string, TYPE> &map, const TString &key, const TString &mapName="");
			template<class TYPE> inline
			bool MapHasKey(std::unordered_map<std::string, TYPE> &map, const TString &key);
		}
	}



/// @}
// * ================================== * //
// * ------- SUB-NAMESPACE DRAW ------- * //
// * ================================== * //


	/// The `DrawAndSaveRecursion` functions are necessary for `DrawAndSave`, which is a <i>variadic</i> template function.
	/// @tparam TYPE The type of objects that you want to draw. @todo Should ideally be `TObject`s.
	/// @tparam ARGS The type of the rest of the objects that you want to draw. The type is actually inferred from `TYPE`.
	/// @param opt Draw options.
	/// @param first The first object that you want to plot.
	/// @param args The objects that you want to plot.
	template<class TYPE, class... ARGS> inline
	void CommonFunctions::Draw::DrawAndSaveRecursion(Option_t* opt, TYPE first, ARGS... args)
	{
		/// Some useful information for defining template functions in the `.cxx` file:
		/// - https://isocpp.org/wiki/faq/templates#template-specialization-speed
		/// - https://isocpp.org/wiki/faq/templates#separate-template-fn-defn-from-decl
		/// - https://isocpp.org/wiki/faq/templates#separate-template-fn-defn-from-decl-export-keyword
		auto obj = dynamic_cast<TObject*>(first);
		if(obj) obj->Draw(opt);
		DrawAndSaveRecursion(opt, args...); // continue recursion
	}


	/// Function that allows you to draw and save any set of `TObject`s.
	/// @tparam ARGS The type of objects that you want to draw. @todo Should ideally be `TObject`s.
	/// @param saveas Filename that the output file name should have. See `CommonFunctions::CommonFunctions::File::SetOutputFilename` for more information.
	/// @param opt Draw options.
	/// @param logScale If this argument contains an `'x'`, the \f$x\f$-scale will be set to log scale (same for `'y'` and `'z'`).
	/// @param args The objects that you want to plot.
	template<class ...ARGS> inline
	void CommonFunctions::Draw::DrawAndSave(const char* saveas, Option_t* opt, const char* logScale, ARGS... args)
	{
		// * Create canvas in batch mode * //
		TCanvas c;
		c.SetBatch();
		// * Draw objects * //
		DrawAndSaveRecursion(opt, args...);
		// * Save canvas * //
		SaveCanvas(saveas, &c, logScale);
	}



// * =================================== * //
// * ------- SUB-NAMESPACE ERROR ------- * //
// * =================================== * //


	/// Construction to `try` and `catch` an exception that comes from `std::map::at`.
	template<class TYPE> inline
	TYPE& CommonFunctions::TerminalIO::GetFromMap(std::unordered_map<std::string, TYPE> &map, const TString &key, const TString &mapName)
	{
		try {
			return map.at(key.Data());
		} catch(std::exception) {
			TString message = Form("Could not load key \"%s\"", key.Data());
			if(mapName.EqualTo("")) message += Form(" from map \"%s\"", mapName.Data());
			message += Form("\n   --->> check file %s", __BASE_FILE__);
			message += Form("\n   --->> check line %s:%d", __FILE__, __LINE__);
			PrintFatalError(message);
		}
	}


	/// Returns `true` if `map` has a certain `key`, and returns `false` if not`.
	/// This function makes use of `std::map::find`.
	template<class TYPE> inline
	bool CommonFunctions::TerminalIO::MapHasKey(std::unordered_map<std::string, TYPE> &map, const TString &key)
	{
		return !(map.find(key.Data()) == map.end());
	}



// * ================================== * //
// * ------- SUB-NAMESPACE LOOP ------- * //
// * ================================== * //


	/// Generalisation of the procedure when looping over a `TTree`.
	template<typename FUNCTOR, typename ...Rest> inline
	void CommonFunctions::Loop::LoopTree(TTree* tree, FUNCTOR&& lambda, Rest&&... args)
	{
		if(CommonFunctions::TerminalIO::IsEmptyPtr(tree)) return;
		auto nEntries = tree->GetEntries();
		std::cout << "Looping over " << nEntries << " events in the \"" << tree->GetName() << "\" tree" << std::flush;
		for(auto i = 0; i < nEntries; ++i) {
			tree->GetEntry(i);
			lambda(args...);
		}
		std::cout << "\rSuccesfully looped over " << nEntries << " events in the \"" << tree->GetName() << "\" tree" << std::endl;
	}



// * =================================== * //
// * ------- SUB-NAMESPACE PRINT ------- * //
// * =================================== * //


	/// Create a `TString` from a number that has a number between each 
	template<typename TYPE> inline
	const TString CommonFunctions::Print::CommaFormattedString(TYPE number)
	{
		TString output(Form("%d", number));
		int insertPosition = output.Length()-3;
		while (insertPosition > 0) {
			output.Insert(insertPosition, ",");
			insertPosition -= 3;
		}
		return output;
	}



#endif