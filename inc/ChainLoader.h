#ifndef BOSS_Afterburner_ChainLoader_H
#define BOSS_Afterburner_ChainLoader_H

// * ========================= * //
// * ------- LIBRARIES ------- * //
// * ========================= * //

	#include "BranchPlotOptions.h"
	#include "CommonFunctions.h"
	#include "ReconstructedParticle.h"
	#include "TChain.h"
	#include "TH1F.h"
	#include "TH2F.h"
	#include "TObject.h"
	#include "TString.h"
	#include "TTree.h"
	#include <string>
	#include <unordered_map>



// * ================================ * //
// * ------- CLASS DEFINITION ------- * //
// * ================================ * //
/// @addtogroup BOSS_Afterburner_objects
/// @{


	/// A container class for a `TChain` that allows easy access to its branches. Addresses are set automatically upon construction.
	/// @author   Remco de Boer 雷穆克 (r.e.deboer@students.uu.nl or remco.de.boer@ihep.ac.cn)
	/// @date     December 21st, 2018
	class ChainLoader
	{
	public:
		/// @name Constructors
			///@{
			ChainLoader() {}
			ChainLoader(TTree* tree) : fChain(tree->GetName(), tree->GetTitle()) {}
			///@}

		/// @name Getters
			///@{
			TChain&  GetChain() { return fChain; }
			Long64_t GetEntries() const { return fChain.GetEntries(); }
			Int_t GetEntry(Long64_t entry=0, Int_t getall=0) { return fChain.GetEntry(entry, getall); }
			///@}

		/// @name Getters for branches, with type
			///@{
			template<typename TYPE> inline
			std::unordered_map<std::string, TYPE>& Get(); ///< Get mapping of branches for a certaine `typename` (specified through `template` specialisation).
			template<typename TYPE> inline
			TYPE& Get(const TString &branchName);
			///@}

		/// @name Setters
			///@{
			Int_t Add(const char* filename) { return fChain.Add(filename); }
			void BookAddresses(bool print_branches=false, bool print_averages=false);
			///@}

		/// @name Information functions
			///@{
			TH1F* GetInvariantMassHistogram(const BranchPlotOptions &branch, const ReconstructedParticle& particle);
			TH1F* GetInvariantMassHistogram(const char* branchName, const ReconstructedParticle& particle, const int nBins=100, Option_t *option="", const TString &logScale="");
			TH2F* Draw(const char* branchX, const char* branchY, const Int_t nBinx, const double x1, const double x2, const Int_t nBiny, const double y1, const double y2, Option_t *option="", const bool save=true, const TString &logScale="", const char* cut="");
			double ComputeMean(TChain* tree, const char* branchName);
			TH1F* Draw(const char* branchX, const Int_t nBinx, const double x1, const double x2, Option_t *option="", const bool save=true, const TString &logScale="", const char* cut="");
			void Draw(const char* branchNames, const char* cut="", Option_t *option="", const bool save=true, const TString &logScale="");
			TH1* Draw(const BranchPlotOptions &options);
			void DrawAndSaveAllBranches(Option_t *option="", const TString &logScale="");
			void DrawAndSaveAllMultiplicityBranches(const TString &logScale="", Option_t *option="E1");
			///@}

	private:
		/// @name Data members
			///@{
			TChain fChain; ///< `TChain` object to which the `TFile`s are added.
			///@}

		/// @name Data members: Maps for branch addresses
			///@{
			std::unordered_map<std::string, Char_t>    fMap_B; ///< Map of addresses for Char_t (8 bit signed integer).
			std::unordered_map<std::string, UChar_t>   fMap_b; ///< Map of addresses for UChar_t (8 bit unsigned integer).
			std::unordered_map<std::string, Short_t>   fMap_S; ///< Map of addresses for Short_t (16 bit signed integer).
			std::unordered_map<std::string, UShort_t>  fMap_s; ///< Map of addresses for UShort_t (16 bit unsigned integer).
			std::unordered_map<std::string, Int_t>     fMap_I; ///< Map of addresses for Int_t (32 bit signed integer).
			std::unordered_map<std::string, UInt_t>    fMap_i; ///< Map of addresses for UInt_t (32 bit unsigned integer).
			std::unordered_map<std::string, Float_t>   fMap_F; ///< Map of addresses for Float_t (32 bit floating point).
			std::unordered_map<std::string, Double_t>  fMap_D; ///< Map of addresses for Double_t (64 bit floating point).
			std::unordered_map<std::string, Long64_t>  fMap_L; ///< Map of addresses for Long64_t (64 bit signed integer).
			std::unordered_map<std::string, ULong64_t> fMap_l; ///< Map of addresses for ULong64_t (64 bit unsigned integer).
			std::unordered_map<std::string, Bool_t>    fMap_O; ///< Map of addresses for Bool_t (boolean).
			///@}

		/// @name Helper functions
			///@{
			template<typename TYPE> inline
			TYPE& GetHelper(std::unordered_map<std::string, TYPE> &map, const TString &name);
			template<typename TYPE> inline
			void SetAddress(TObject* obj, std::unordered_map<std::string, TYPE> &map);
			template<typename TYPE> inline
			void SetAddressSafe(TObject* obj, std::unordered_map<std::string, TYPE> &map);
			///@}

	};



/// @}
// * =============================== * //
// * ------- PRIVATE METHODS ------- * //
// * =============================== * //


	/// Set a memory address for one of the branches of `fChain`.
	/// @tparam TYPE The `typename` of the address. It is determined by the `map` you feed it.
	/// @param obj The object from which you want to load the address. This is usually a `TBranch` object.
	/// @param map The type of address you a want to assign (`double`, `int`, etc) is determined by the `map` you feed it. This map should be one of the `fMap_*` data members of this class.
	template<typename TYPE> inline
	void ChainLoader::SetAddress(TObject* obj, std::unordered_map<std::string, TYPE> &map)
	{
		fChain.SetBranchAddress(obj->GetName(), &map[obj->GetName()]);
	}


	/// Set a memory address for one of the branches of `fChain`. Only do this if `fChain` exists.
	/// @tparam TYPE The `typename` of the address. It is determined by the `map` you feed it.
	/// @param obj The object from which you want to load the address. This is usually a `TBranch` object.
	/// @param map The type of address you a want to assign (`double`, `int`, etc) is determined by the `map` you feed it. This map should be one of the `fMap_*` data members of this class.
	template<typename TYPE> inline
	void ChainLoader::SetAddressSafe(TObject* obj, std::unordered_map<std::string, TYPE> &map)
	{
		if(fChain.GetNbranches()) SetAddress(obj, map);
	}



// * ========================= * //
// * ------- TEMPLATES ------- * //
// * ========================= * //


	template<> inline std::unordered_map<std::string, Char_t>&    ChainLoader::Get<Char_t>   () { return fMap_B; }
	template<> inline std::unordered_map<std::string, UChar_t>&   ChainLoader::Get<UChar_t>  () { return fMap_b; }
	template<> inline std::unordered_map<std::string, Short_t>&   ChainLoader::Get<Short_t>  () { return fMap_S; }
	template<> inline std::unordered_map<std::string, UShort_t>&  ChainLoader::Get<UShort_t> () { return fMap_s; }
	template<> inline std::unordered_map<std::string, Int_t>&     ChainLoader::Get<Int_t>    () { return fMap_I; }
	template<> inline std::unordered_map<std::string, UInt_t>&    ChainLoader::Get<UInt_t>   () { return fMap_i; }
	template<> inline std::unordered_map<std::string, Float_t>&   ChainLoader::Get<Float_t>  () { return fMap_F; }
	template<> inline std::unordered_map<std::string, Double_t>&  ChainLoader::Get<Double_t> () { return fMap_D; }
	template<> inline std::unordered_map<std::string, Long64_t>&  ChainLoader::Get<Long64_t> () { return fMap_L; }
	template<> inline std::unordered_map<std::string, ULong64_t>& ChainLoader::Get<ULong64_t>() { return fMap_l; }
	template<> inline std::unordered_map<std::string, Bool_t>&    ChainLoader::Get<Bool_t>   () { return fMap_O; }


	/// Get branch of this TChain with `branchName`.
	/// You will have to know and specify the `typename` of the branch using e.g. `Get<double>` in the case of a `double`.
	template<typename TYPE> inline
	TYPE& ChainLoader::Get(const TString &branchName)
	{
		return CommonFunctions::TerminalIO::GetFromMap(Get<TYPE>(), branchName, fChain.GetName());
	}



#endif