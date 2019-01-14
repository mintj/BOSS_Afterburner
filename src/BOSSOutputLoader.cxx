// * ========================= * //
// * ------- LIBRARIES ------- * //
// * ========================= * //
	#include "BOSSOutputLoader.h"
	#include "CommonFunctions.h"
	#include "FrameworkSettings.h"
	#include "TFile.h"
	#include "TKey.h"
	#include "TList.h"
	#include "TObject.h"
	#include "TSystemDirectory.h"
	#include "TTree.h"
	#include <iostream>
	#include <map>



// * =========================================== * //
// * ------- CONSTRUCTORS AND DESTRUCTORS ------ * //
// * =========================================== * //


	/**
	 * @brief Constructor that opens a `TFile` and unordered_maps its contents.
	 */
	BOSSOutputLoader::BOSSOutputLoader(const char* path, bool print) :
		fDirectoryPath(path)
	{
		/// -# Make an inventory of ROOT files in input directory and add filenames to the `fFileNames` list.
		/// -# If `path` ends in `".root"`, treat as <i>single ROOT file</i> and attempt to load it.
		if(fDirectoryPath.EndsWith(".root")) {
			/// <ul>
			/// <li> Abort if `path` cannot be loaded as ROOT file.
				TFile file(fDirectoryPath.Data());
				if(file.IsZombie()) return;
			/// <li> Extract directory and file name from input `path`.
				TString fname(fDirectoryPath);
				auto pos = fDirectoryPath.Last('/');
				fDirectoryPath.Resize(pos);
				fname.Remove(0, pos+1);
			/// <li> Add resulting file name to `fFileNames` and continue loading as `TChain`.
				fFileNames.push_back(fname);
			/// </ul>
		}
		/// -# Else, treat `path` as <i>path to directory</i> and make an inventory of ROOT files in this directory.
		else {
			/// <ul>
			/// <li> Remove trailing slash `/` from `fDirectoryPath`
				if(fDirectoryPath.EndsWith("/")) fDirectoryPath.Chop();
			/// <li> Open file directory using `TSystemDirectory`.
				TSystemDirectory dir(path, path);
			/// <li> Abort if directory is empty.
				TList *files = dir.GetListOfFiles();
				if(!files) {
					std::cout << "ERROR: Directory \"" << fDirectoryPath << "\" does not exist" << std::endl;
					return;
				}
			/// <li> Loop over list of files in directory
				TIter fileIter(files);
				TObject *obj;
				while ((obj = fileIter())) {
					auto systemFile = dynamic_cast<TSystemFile*>(obj);
					if(!systemFile) continue;
					TString filename { systemFile->GetName() };
					if(!filename.EndsWith(".root")) continue;
					TString fname(systemFile->GetName());
					/// and add file name if file exists and is no zombie.
					TFile file((fDirectoryPath+"/"+fname).Data());
					if(!file.IsZombie()) fFileNames.push_back(fname);
				}
			/// </ul>
		}
		/// -# Execute `LoadChains` now that the `fFileNames` list has been loaded.
			LoadChains(print);
	}



// * ======================= * //
// * ------- GETTERS ------- * //
// * ======================= * //

	/**
	 * @brief Get the number of events in one of the `TChain`s.
	 * @param treeName Name of the `TChain` that you are looking for.
	 * @return Tree object that contains the trees. Returns a `nullptr` if this `TChain` does not exist.
	 */
	Long64_t BOSSOutputLoader::GetEntries(const char* treeName)
	{
		FindTree(treeName).GetEntries();
	}

	/**
	 * @brief Return the largest number of events in all of the `TChain`s in the loaded ROOT file.
	 */
	Long64_t BOSSOutputLoader::GetLargestEntries() const
	{
		Long64_t largest = 0;
		for(auto it = fChains.begin(); it != fChains.end(); ++it)
			if(it->second.GetEntries() > largest)
				largest = it->second.GetEntries();
		return largest;
	}

	/**
	 * @brief Look for a tree in the files and get its `TChain`.
	 * @param treeName Name of the `TChain` that you are looking for.
	 * @return TChain& Tree object that contains the trees. Returns a `nullptr` if this `TChain` does not exist.
	 */
	TChain& BOSSOutputLoader::FindTree(const char* treeName)
	{
		return fChains.at(treeName).GetChain();
	}



// * =========================== * //
// * ------- INFORMATION ------- * //
// * =========================== * //

	/**
	 * @brief Draw the distributions of all branches available in the ROOT file.
	 * 
	 * @param treeName Name of the `TChain` that you are looking for.
	 * @param branchX Branch that you want to plot. You may use a formula.
	 * @param nBinx Number of bins to use on the \f$x\f$-axis.
	 * @param x1 Lower limit on the \f$x\f$-axis.
	 * @param x2 Upper limit on the \f$x\f$-axis..
	 * @param opt Draw options.
	 * @param logScale If this argument contains an `'x'`, the \f$x\f$-scale will be set to log scale (same for `'y'` and `'z'`).
	 */
	TH1F* BOSSOutputLoader::DrawBranches(const char* treeName, const char* branchX, const Int_t nBinx, const double x1, const double x2, Option_t* opt, const TString &logScale)
	{
		return fChains.at(treeName).DrawBranches(branchX, nBinx, x1, x2, true, opt, logScale);
	}


	/**
	 * @brief Draw the distributions of all branches available in the ROOT file.
	 * 
	 * @param treeName Name of the `TChain` that you are looking for.
	 * @param branchX Branch that you want to plot on the \f$x\f$-axis. You may use a formula.
	 * @param branchY Branch that you want to plot on the \f$y\f$-axis. You may use a formula.
	 * @param nBinx Number of bins to use on the \f$x\f$-axis.
	 * @param x1 Lower limit on the \f$x\f$-axis.
	 * @param x2 Upper limit on the \f$x\f$-axis.
	 * @param nBiny Number of bins to use on the \f$y\f$-axis.
	 * @param y1 Lower limit on the \f$y\f$-axis.
	 * @param y2 Upper limit on the \f$y\f$-axis.
	 * @param opt Draw options.
	 * @param logScale If this argument contains an `'x'`, the \f$x\f$-scale will be set to log scale (same for `'y'` and `'z'`).
	 */
	TH2F* BOSSOutputLoader::DrawBranches(const char* treeName, const char* branchX, const char* branchY, const Int_t nBinx, const double x1, const double x2, const Int_t nBiny, const double y1, const double y2, Option_t* opt, const TString &logScale)
	{
		return fChains.at(treeName).DrawBranches(branchX, branchY, nBinx, x1, x2, nBiny, y1, y2, true, opt, logScale);
	}


	/**
	 * @brief Check if file is loaded.
	 */
	bool BOSSOutputLoader::IsZombie()
	{
		return !fChains.size();
	}


	/**
	 * @brief Draw the distributions of all branches available in the ROOT file.
	 * 
	 * @param treeName Name of the `TChain` that you are looking for.
	 * @param opt Draw options.
	 * @param logScale If this argument contains an `'x'`, the \f$x\f$-scale will be set to log scale (same for `'y'` and `'z'`).
	 */
	void BOSSOutputLoader::DrawAndSaveAllBranches(const char* treeName, Option_t* opt, const TString &logScale)
	{
		fChains.at(treeName).DrawAndSaveAllBranches(opt, logScale);
	}


	/**
	 * @brief Draw the distributions of all branches available in the ROOT file.
	 * 
	 * @param treeName Name of the `TChain` that you are looking for.
	 * @param branchNames Name of the branch names that you want to plot. See https://root.cern.ch/doc/master/classTChain.html#a8a2b55624f48451d7ab0fc3c70bfe8d7 for the syntax.
	 * @param opt Draw options.
	 * @param logScale If this argument contains an `'x'`, the \f$x\f$-scale will be set to log scale (same for `'y'` and `'z'`).
	 */
	void BOSSOutputLoader::DrawBranches(const char* treeName, const char* branchNames, Option_t* opt, const TString &logScale)
	{
		fChains.at(treeName).DrawBranches(branchNames, true, opt, logScale);
	}

	/**
	 * @brief Print information about all trees in the `TFile`.
	 */
	void BOSSOutputLoader::Print()
	{
		// * Check if class has been constructed *
		if(IsZombie()) {
			std::cout << std::endl << "File not loaded" << std::endl;
			return;
		}
		// * If so, print all 
		std::cout << std::endl << "Input path: \"" << fDirectoryPath.Data();
		if(fFileNames.size() == 1) std::cout << "/" << fFileNames.front();
		std::cout << "\"" << std::endl;
		if(fChains.size()) {
			std::cout << "  contains " << fChains.size() << " TChains:" << std::endl;
			std::unordered_map<std::string, ChainLoader>::iterator it = fChains.begin();
			for(; it != fChains.end(); ++it) {
				TChain *chain = &it->second.GetChain();
				std::cout << "    \"" <<
					chain->GetName() << "\"\t\"" <<
					chain->GetTitle() << "\"\t" <<
					chain->GetEntries() << " entries" << std::endl;
			}
		} else {
			std::cout << "  does not contain any TChains" << std::endl;
		}
		std::cout << std::endl;
		// * Print contents of input file
		TFile file(fFileNames.front().Data());
		if(file.IsZombie()) return;
		std::cout << "  Output of TFile::ls():" << std::endl;
		std::cout << "  ---------------------" << std::endl;
		file.ls();
		std::cout << std::endl;
	}


	/**
	 * @brief Print information about a certain tree.
	 * @details This function loops over the `std::unordered_map` of file names and over the `std::unordered_map` of trees and prints its name and number of events. For each tree, information about its branches is also printed.
	 * @param nameOfTree
	 * @param opt
	 */
	void BOSSOutputLoader::Print(const char* nameOfTree, Option_t *opt)
	{
		TChain *chain = &FindTree(nameOfTree);
		chain->Print(opt);
		std::cout << "------------------------------------" << std::endl;
		std::cout << "Total number of events in TTree \"" << chain->GetName() << "\": " << std::scientific << chain->GetEntries() << std::endl;
		std::cout << std::endl;
	}


	/**
	 * @brief Method that prints the number of entries of each `TChain` in the ROOT file. This allows you to get an idea of the cut flow of the original macro. (Of course, have a look at your original source code `.cxx` to see what these numbers mean.)
	 */
	void BOSSOutputLoader::PrintCutFlow()
	{
		if(!fChains.size()) return;
		/// -# Write number of entries per `TChain`.
			auto width = ceil(log10(GetLargestEntries()));
			width += 2;
			width += width/3;
			auto list = CreateOrderedMap();
			std::cout << std::endl << "CUT FLOW FOR " << fChains.size() << " TREES" << std::endl;
			for(auto it : list) {
				std::cout
					<< std::right << std::setw(width) << CommonFunctions::Print::CommaFormattedString(it->GetEntries()) << "  "
					<< std::left  << "\"" << it->GetChain().GetName() << "\"" << std::endl;
			}
			std::cout << std::endl;
		/// -# If there is a `TChain` called `"_cutvalues"`, print these values and names as well.
			auto key = fChains.find("_cutvalues");
			if(key == fChains.end()) return;
			if(!key->second.GetEntries()) return;
			key->second.GetChain().GetEntry(0); // get first entry of `TChain`
			std::cout << "CUT PARAMTERS" << std::endl;
			std::map<std::string, Double_t> tempMap; // temp *sorted* `map`
			int length = 0;
			for(auto it : key->second.Get_D()) {
				tempMap[it.first] = it.second;
				if(it.first.length() > length) length = it.first.length();
			}
			length += 4;
			for(auto it : tempMap) {
				std::cout
					<< std::setw(length) << std::right << it.first << ": "
					<< it.second << std::endl;
			}
			std::cout << std::endl;
	}
	void BOSSOutputLoader::PrintCuts()
	{
		/// <ol>
		/// <li> Search for a `TChain` called `"_cutvalues"` and return if it doesn't exist or if it is empty.
			auto key = fChains.find("_cutvalues");
			if(key == fChains.end()) return;
			if(!key->second.GetEntries()) return;
		/// <li> Use entry 0 to get the names of the cut parameters. We use a *sorted* `map` so that the keys are sorted automatically. At the same time, the maximum number of characters in all of the cut names is also determined.
			key->second.GetChain().GetEntry(0);
			size_t i = 0, tot = key->second.Get_D().size();
			std::vector<std::string> names (tot);
			std::vector<double>      entry0(tot);
			int length = 0;
			for(auto it : key->second.Get_D()) {
				names [i] = it.first;
				entry0[i] = it.second;
				if(names[i].length() > length) length = names[i].length();
				++i;
			}
			length += 4; // some indent
		/// <li> If `"_cutvalues"` contains only 1 entry, <b>only</b> print a list of cut paramters and their values. Entry 0 of the `"_cutvalues"` tree is considered to be that value. (This is for backward compatibility with output of the older versions of `TrackSelector`.)
			if(key->second.GetEntries()==1) {
				std::cout << "CUT PARAMTERS" << std::endl;
				for(i=0; i<tot; ++i) {
					std::cout << std::setw(length) << std::right << names[i] << ": " << entry0[i] << std::endl;
				}
				std::cout << std::endl;
				return;
			}
		/// <li> If `"_cutvalues"` contains 3 entries, consider entry 0 to be the `min` value, entry 1 to be the `max`, and entry 2 to be `count` (the number of events or tracks that passed the cut).
			/// <ol>
			/// <li> Declare remaining two vectors for `max` and `count`.
			std::vector<double> entry1(tot);
			std::vector<int>    entry2(tot);
			/// <li> Get values from entry 1 (`max`).
			key->second.GetChain().GetEntry(1);
			i = 0;
			for(auto it : key->second.Get_D()) {
				entry1[i] = it.second;
				++i;
			}
			/// <li> Get values from entry 2 (`count`).
			key->second.GetChain().GetEntry(2);
			i = 0;
			for(auto it : key->second.Get_D()) {
				entry2[i] = it.second;
				++i;
			}
			/// </ol>
		/// <li> And print loaded values as a table: one row per parameters.
			/// <ol>
			/// <li> Print table header with four columns.
			std::cout << std::endl << "  "
				<< std::setw(length) << std::left  << "CUT NAME" << " | "
				<< std::setw(10)     << std::right << "MIN" << " | "
				<< std::setw(10)     << std::right << "MAX" << " | "
				<< std::setw(10)     << std::right << "COUNT"
				<< std::endl;
			for(i=0; i<tot; ++i) {
				/// <li> Column 1: <b>cut name</b>.
				std::cout << std::setw(length) << std::left << names[i] << " | ";
				/// <li> Column 2: <b>minimum</b>, if available.
				std::cout << std::setw(10) << std::right;
				if(entry0[i] > -DBL_MAX) std::cout << entry0[i];
				else std::cout << "";
				std::cout << " | ";
				/// <li> Column 3: <b>maximum</b>, if available.
				std::cout << std::setw(10) << std::right;
				if(entry1[i] < DBL_MAX) std::cout << entry1[i];
				else std::cout << "";
				std::cout << " | ";
				/// <li> Column 4: <b>counter</b>, if available.
				std::cout << std::setw(10) << std::right << entry2[i] << std::endl;
				/// </ol>
			}
			std::cout << std::endl;
		/// </ol>
	}


	/**
	 * @brief Draw the distributions of all branches available in the ROOT file.
	 * @param opt Draw options.
	 */
	void BOSSOutputLoader::QuickDrawAndSaveAll(Option_t* opt)
	{
		for(auto it = fChains.begin(); it != fChains.end(); ++it) it->second.DrawAndSaveAllBranches(opt);
	}

// * =============================== * //
// * ------- PRIVATE METHODS ------- * //
// * =============================== * //


	/**
	 * @brief Create a `list` that is ordered by the number of entries in the `TChain`.
	 */
	std::list<ChainLoader*> BOSSOutputLoader::CreateOrderedMap()
	{
		// * Create list of pointers to `ChainLoader` * //
		std::list<ChainLoader*> outputList;
		for(auto it = fChains.begin(); it != fChains.end(); ++it) {
			TString name(it->second.GetChain().GetName());
			if(!name.EqualTo("_cutvalues")) outputList.push_back(&(it->second));
		}
		// * Sort resulting list based on number of entries * //
		outputList.sort([](ChainLoader* const & a, ChainLoader* const & b)
		{
			std::string nameA(a->GetChain().GetName());
			std::string nameB(b->GetChain().GetName());
			return a->GetEntries() != b->GetEntries() ?
				a->GetEntries() > b->GetEntries() :
				nameA < nameB;
		});
		return outputList;
	}



	/**
	 * @brief Create a list of `TChain`s based on the files in the directory you are loading.
	 * @brief If the ROOT `TFile` could be successfully loaded, this method will generate a `std::unordered_map` of pointers to all `TChain`s in the main folder of the ROOT file. The first file in the directory is used to make an inventory of all `TTree`s within the file.
	 */
	void BOSSOutputLoader::LoadChains(bool print)
	{
		/// -# Check whether there are files in the `fFileNames` list.
			if(!fFileNames.size()) {
				std::cout << "ERROR: List of file names is empty!" << std::endl;
				std::cout << "--> There were no valid ROOT files in directory \"" << fDirectoryPath << "\"" << std::endl;
			}
		/// -# Load first ROOT file `fFileNames` list.
			TString fileToLoad(fDirectoryPath+"/"+fFileNames.front());
			TFile file(fileToLoad.Data());
			if(file.IsZombie()) {
				std::cout << "ERROR: File \"" << fileToLoad << "\" is zombie" << std::endl;
			}
		/// -# Go through keys in the file and add create `ChainLoader` if the key is a `TTree`.
			TIter next(file.GetListOfKeys());
			TObject *obj  = nullptr;
			TKey    *key  = nullptr;
			TTree   *tree = nullptr;
			while((obj = next())) {
				/// <ul>
				/// <li> Attempt to cast loaded `TObject` to `TKey`.
					if(!(key = dynamic_cast<TKey*>(obj))) continue;
				/// <li> Read `TKey` and attempt to cast to `TTree`.
					if(!(tree = dynamic_cast<TTree*>(key->ReadObj()))) continue;
				/// <li> Create `ChainLoader` object (with underlying `TChain`) from the loaded `TTree`.
					fChains.emplace(tree->GetName(), tree);
					auto simptreePtr = &fChains.at(tree->GetName());
				/// <li> Loop over file names and at them to the `ChainLoader` object.
					for(auto filename : fFileNames) simptreePtr->Add(fDirectoryPath+"/"+filename);
				/// <li> Book branches using `ChainLoader::BookAddresses`.
					simptreePtr->BookAddresses(print);
				/// </ul>
			}
		/// -# Print terminal output
			std::cout << "Created a map of " << fChains.size() << " TChains from ";
			if(fFileNames.size() > 1) std::cout << fFileNames.size() << " files in ";
			std::cout << "input path \"" << fDirectoryPath.Data();
			if(fFileNames.size() == 1) std::cout << "/" << fFileNames.front(); // in case of one file
			std::cout << "\"" << std::endl;
	}