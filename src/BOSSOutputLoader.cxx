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
	#include <iomanip>
	#include <iostream>
	#include <map>
	#include <sstream>
	#include <utility>
	using namespace CommonFunctions;



// * =========================================== * //
// * ------- CONSTRUCTORS AND DESTRUCTORS ------ * //
// * =========================================== * //


	/// Constructor that opens a `TFile` and unordered_maps its contents.
	BOSSOutputLoader::BOSSOutputLoader(const TString &path, bool print_branches, bool print_averages) :
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
				TSystemDirectory dir(fDirectoryPath.Data(), fDirectoryPath.Data());
			/// <li> Abort if directory is empty.
				TList *files = dir.GetListOfFiles();
				if(!files) {
					std::cout << "ERROR: Directory \"" << fDirectoryPath << "\" does not exist" << std::endl;
					return;
				}
			/// <li> Loop over list of files in directory
				TIter fileIter(files);
				TObject *obj = nullptr;
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
			LoadChains(print_branches, print_averages);
	}



// * ======================= * //
// * ------- GETTERS ------- * //
// * ======================= * //


	ChainLoader& BOSSOutputLoader::GetChainLoader(const TString &chainName)
	{
		return TerminalIO::GetFromMap(fChains, chainName.Data(), fDirectoryPath.Data());
	}


	/// Get the number of events in one of the `TChain`s.
		/// @param treeName Name of the `TChain` that you are looking for.
		/// @return Tree object that contains the trees. Returns a `nullptr` if this `TChain` does not exist.
	Long64_t BOSSOutputLoader::GetEntries(const char* treeName)
	{
		GetChain(treeName).GetEntries();
	}


	/// Return the largest number of events in all of the `TChain`s in the loaded ROOT file.
	Long64_t BOSSOutputLoader::GetLargestEntries() const
	{
		Long64_t largest = 0;
		for(auto it = fChains.begin(); it != fChains.end(); ++it)
			if(it->second.GetEntries() > largest)
				largest = it->second.GetEntries();
		return largest;
	}


	/// Look for a tree in the files and get its `TChain`.
		/// @param treeName Name of the `TChain` that you are looking for.
		/// @return TChain& Tree object that contains the trees. Returns a `nullptr` if this `TChain` does not exist.
	TChain& BOSSOutputLoader::GetChain(const TString &treeName)
	{
		return GetChainLoader(treeName).GetChain();
	}



// * =========================== * //
// * ------- INFORMATION ------- * //
// * =========================== * //


	/// Check if file has chain with name `chainName`.
	const bool BOSSOutputLoader::HasChain(const TString &chainName) const
	{
		if(fChains.find(chainName.Data()) == fChains.end()) return false;
		return true;
	}


	/// Check if file is loaded.
	bool BOSSOutputLoader::IsZombie()
	{
		return !fChains.size();
	}


	/// Print information about all trees in the `TFile`.
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


	/// Print information about a certain tree.
	/// This function loops over the `std::unordered_map` of file names and over the `std::unordered_map` of trees and prints its name and number of events. For each tree, information about its branches is also printed.
		/// @param nameOfTree
		/// @param option
	void BOSSOutputLoader::Print(const char* nameOfTree, Option_t *option)
	{
		TChain *chain = &GetChain(nameOfTree);
		chain->Print(option);
		std::cout << "------------------------------------" << std::endl;
		std::cout << "Total number of events in TTree \"" << chain->GetName() << "\": " << std::scientific << chain->GetEntries() << std::endl;
		std::cout << std::endl;
	}


	/// Method that prints the number of entries of each `TChain` in the ROOT file. This allows you to get an idea of the cut flow of the original macro. (Of course, have a look at your original source code `.cxx` to see what these numbers mean.)
	void BOSSOutputLoader::PrintCutFlow()
	{
		if(!fChains.size()) return;
		/// -# Write number of entries per `TChain`.
			auto width = ceil(log10(GetLargestEntries()));
			width += 2;
			width += width/3;
			auto list = CreateOrderedMap();
			std::cout << std::endl << "CUT FLOW FOR " << fChains.size() << " TREES" << std::endl;
			for(auto &it : list) {
				TString chainName { it->GetChain().GetName() };
				if(chainName.EqualTo("_cutvalues")) continue; /// Do not print `"_cutvalues"` branch
				std::cout
					<< std::right << std::setw(width) << CommonFunctions::Print::CommaFormattedString(it->GetEntries()) << "  "
					<< std::left  << "\"" << it->GetChain().GetName() << "\"" << std::endl;
			}
			std::cout << std::endl;
		/// -# If there is a `TChain` called `"_cutvalues"`, print these values and names as well using `PrintCuts`.
			PrintCuts();
	}


	/// Function that allows you to print the cut definitions if they have been stored in the output ROOT file.
	void BOSSOutputLoader::PrintCuts()
	{
		/// <ol>
		/// <li> @b Abort if there is no `TChain` `"_cutvalues"` or if it is empty.
			auto key = fChains.find("_cutvalues");
			if(key == fChains.end()) return;
			if(!key->second.GetEntries()) return;
			ChainLoader *chainLoader { &(key->second) };
			TChain *chain { &chainLoader->GetChain() };
		/// <li> Create an inventory of branches in the form of a `std::map`. The keys are the branch names, the values are `double` `vector`s of size 3 that will contain the loaded values. We use a *sorted* `map` so that the keys are sorted automatically. At the same time, the maximum number of characters in all of the cut names is also determined.
			std::map<std::string, std::vector<double> > cuts;
			int w_name = 0;
			TIter next(chain->GetListOfBranches());
			TObject *obj = nullptr;
			while(obj = next()) {
				const TString branchName { obj->GetName() };
				if(branchName.EqualTo("index")) continue;
				cuts[branchName.Data()].resize(3);
				if(branchName.Length() > w_name) w_name = branchName.Length();
			}
		/// <li> @b CASE1. If `"_cutvalues"` contains a branch called `"index"`, assume that the `TTree` contains double arrays.
			if(TerminalIO::MapHasKey(chainLoader->Get<int>(), "index")) for(auto &cut : cuts) {
			/// <ol>
			/// <li> `SetBranchAddress` for the array of this branch. @b Abort if `SetBranchAddress` does not return `0`.
				double values[3];
				if(chain->SetBranchAddress(cut.first.c_str(), &values)) {
					std::cout << "ERROR: failed to load \"" << cut.first << "\" in branch \"_cutvalues\"" << std::endl;
					std::cout << "  -->> SetBranchAddress returned " << chain->SetBranchAddress(cut.first.c_str(), &values) << std::endl;
					return;
				}
			/// <li> Entry `0`: this is information written from the `CutObject` to the first file of the `TChain` loaded here. We use it to load the `min` and `max` value (array entries 0 and 1). @see TrackSelector::AddAndWriteCuts
				chain->GetEntry(0);
				cut.second[0] = values[0]; // min
				cut.second[1] = values[1]; // max
				cut.second[2] = 0; // count
			/// <li> Get the total count value by adding up each 3rd entry of the array.
				for(int i=0; i<chainLoader->GetEntries(); ++i) {
					chainLoader->GetEntry(i);
					cut.second[2] += values[2];
				}
			/// <li> Drop address allocated for this branch.
				chain->ResetBranchAddress(chain->GetBranch(cut.first.c_str()));
			/// </ol>
		/// <li> If `TChain` `"_cutvalues"` does not contain a branch called `"index"`, it is probably created by an older version of the `TrackSelector` and we need to use one of the following methods.
			} else {
			/// <ol>
			/// <li> @b Abort if `"_cutvalues"` contains 2 entry. This means something went wrong while writing this `NTuple`.
				if(chainLoader->GetEntries()==2) {
					std::cout << "ERROR: TChain \"_cutvalues\" contains 2 entries. Not sure how to work with that..." << std::endl;
					return;
				}
			/// <li> @b CASE2. If `"_cutvalues"` contains only 1 entry, @b only print a list of cut paramters and their values:
				if(chainLoader->GetEntries()==1) {
				/// <ul>
				/// <li> For each branch, add up the entries
					for(auto &cut : cuts) {
						cut.second[0] = 0;
						for(int i=0; i<chainLoader->GetEntries(); ++i) {
							chainLoader->GetEntry(i);
							cut.second[0] += chainLoader->Get<double>(cut.first);
						}
					}
				/// <li> Entry 0 of the `"_cutvalues"` tree is considered to be that value.
					std::cout << "CUT PARAMTERS" << std::endl;
					for(auto it : cuts)
						std::cout << "  " << std::setw(w_name) << std::right << it.first << ": " << it.second[0] << std::endl;
					std::cout << std::endl;
					return;
				/// </ul>
			/// <li> @b CASE3. If `"_cutvalues"` contains at least 3 entries, consider entry 0 to be the `min` value, entry 1 to be the `max`, and entry 2 to be `count` (the number of events or tracks that passed the cut).
				} else {
				/// <ol>
				/// <li> Get values from entry 0 (`min`).
					chainLoader->GetChain().GetEntry(0);
					for(auto it : chainLoader->Get<double>())
						cuts[it.first].push_back(it.second);
				/// <li> Get values from entry 1 (`max`).
					chainLoader->GetChain().GetEntry(1);
					for(auto it : chainLoader->Get<double>())
						cuts[it.first].push_back(it.second);
				/// <li> Get values from entry 2 (`count`). In case of an `hadd`ed file, add up the values of entry 2, 5, 8, etc.
					chainLoader->GetChain().GetEntry(2);
					for(auto it : chainLoader->Get<double>()) {
						int ncounts{0};
						for(int i = 2; i < chainLoader->GetEntries(); i += 3) {
							chainLoader->GetChain().GetEntry(i);
							ncounts += it.second;
						}
						cuts[it.first].push_back(ncounts);
					}
				/// </ol>
				}
			/// </ol>
			}
		/// <li> Print loaded values as a table: one row per parameters.
			/// <ol>
			/// <li> Get width of columns (`w_name` has to be redone because of the table headers).
				const TString h_name  { "CUT NAME" };
				const TString h_min   { "MIN" };
				const TString h_max   { "MAX" };
				const TString h_count { "COUNT" };
				w_name = h_name.Length();
				int w_min   = h_min  .Length();
				int w_max   = h_max  .Length();
				int w_count = h_count.Length();
				for(auto &cut : cuts) {
					// * Name *
					if(cut.first.size() > w_name) w_name = cut.first.size();
					// * Minimum value *
					if(cut.second[0] > -DBL_MAX) {
						std::ostringstream os;
						os << cut.second[0];
						if(os.str().size() > w_min) w_min = os.str().size();
					}
					// * Maximum value *
					if(cut.second[1] < DBL_MAX) {
						std::ostringstream os;
						os << cut.second[1];
						if(os.str().size() > w_max) w_max = os.str().size();
					}
					// * Count value *
					std::ostringstream os;
					os << (unsigned long long)cut.second[2];
					if(os.str().size() > w_count) w_count = os.str().size();
				}
			/// <li> Print table header with four columns.
				std::cout << std::endl << "  "
					<< std::setw(w_name)  << std::left << h_name << " | "
					<< std::setw(w_min)   << std::left << h_min << " | "
					<< std::setw(w_max)   << std::left << h_max << " | "
					<< std::setw(w_count) << std::left << h_count
					<< std::endl;
			/// <li> Print horizontal line beneath it.
				std::cout << "  " << std::setfill('-') << std::setw(w_name+w_min+w_max+w_count+9) << "" << std::endl;
				std::cout << std::setfill(' ');
			/// <li> Create a list of cut names ordered by counter (using `std::list::sort` and a lambda).
				std::list<std::pair<TString, unsigned long long> > cutNames;
				for(auto &cut : cuts) cutNames.push_back(std::make_pair(cut.first, cut.second[2]));
				cutNames.sort([](std::pair<TString, unsigned long long> const & a, std::pair<TString, unsigned long long> const & b)
				{
					return a.second != b.second ?
						a.second > b.second :
						a.first < b.first;
				});
			/// <li> Print the table:
				for(auto &cutName : cutNames) {
					/// <ol>
					/// <li> Print a horizontal line just before counter `"N_events"`.
						if(cutName.first.EqualTo("N_events")) {
							std::cout << "  " << std::setfill('-') << std::setw(w_name+w_min+w_max+w_count+9) << "" << std::endl;
							std::cout << std::setfill(' ');
						}
					/// <li> Column 1: **cut name**.
						std::cout << "  " << std::setw(w_name) << std::left << cutName.first << " | ";
					/// <li> Column 2: @b minimum, if available.
						std::cout << std::setw(w_min) << std::right;
						if(cuts[cutName.first.Data()][0] > -DBL_MAX) std::cout << cuts[cutName.first.Data()][0];
						else std::cout << "";
						std::cout << " | ";
					/// <li> Column 3: @b maximum, if available.
						std::cout << std::setw(w_max) << std::right;
						if(cuts[cutName.first.Data()][1] < DBL_MAX) std::cout << cuts[cutName.first.Data()][1];
						else std::cout << "";
						std::cout << " | ";
					/// <li> Column 4: @b counter, if available.
						std::cout << std::setw(w_count) << std::right << cutName.second << std::endl;
					/// </ol>
				}
				std::cout << std::endl;
			/// </ol>
		/// <li> Reset branches to avoid a crash at the end of your executable.
			chain->ResetBranchAddresses();
		/// </ol>
	}



// * ==================== * //
// * ------- DRAW ------- * //
// * ==================== * //


	/// Draw the distributions of all branches available in the ROOT file.
		/// @param treeName Name of the `TChain` that you are looking for.
		/// @param branchX Branch that you want to plot. You may use a formula.
		/// @param nBinx Number of bins to use on the \f$x\f$-axis.
		/// @param x1 Lower limit on the \f$x\f$-axis.
		/// @param x2 Upper limit on the \f$x\f$-axis..
		/// @param option Draw options.
		/// @param logScale If this argument contains an `'x'`, the \f$x\f$-scale will be set to log scale (same for `'y'` and `'z'`).
		/// @param cut Fill in a cut according to the syntax described <a href="https://root.cern.ch/doc/master/classTTree.html#a73450649dc6e54b5b94516c468523e45">here</a>.
	TH1F* BOSSOutputLoader::Draw(const char* treeName, const char* branchX, const Int_t nBinx, const double x1, const double x2, Option_t *option, const TString &logScale, const char* cut)
	{
		return GetChainLoader(treeName).Draw(branchX, nBinx, x1, x2, option, true, logScale, cut);
	}


	/// Draw the distributions of all branches available in the ROOT file.
		/// @param treeName Name of the `TChain` that you are looking for.
		/// @param branchX Branch that you want to plot on the \f$x\f$-axis. You may use a formula.
		/// @param branchY Branch that you want to plot on the \f$y\f$-axis. You may use a formula.
		/// @param nBinx Number of bins to use on the \f$x\f$-axis.
		/// @param x1 Lower limit on the \f$x\f$-axis.
		/// @param x2 Upper limit on the \f$x\f$-axis.
		/// @param nBiny Number of bins to use on the \f$y\f$-axis.
		/// @param y1 Lower limit on the \f$y\f$-axis.
		/// @param y2 Upper limit on the \f$y\f$-axis.
		/// @param option Draw options.
		/// @param logScale If this argument contains an `'x'`, the \f$x\f$-scale will be set to log scale (same for `'y'` and `'z'`).
		/// @param cut Fill in a cut according to the syntax described <a href="https://root.cern.ch/doc/master/classTTree.html#a73450649dc6e54b5b94516c468523e45">here</a>.
	TH2F* BOSSOutputLoader::Draw(const char* treeName, const char* branchX, const char* branchY, const Int_t nBinx, const double x1, const double x2, const Int_t nBiny, const double y1, const double y2, Option_t *option, const TString &logScale, const char* cut)
	{
		return GetChainLoader(treeName).Draw(branchX, branchY, nBinx, x1, x2, nBiny, y1, y2, option, true, logScale, cut);
	}


	/// Draw the distributions of all branches available in the ROOT file.
		/// @param treeName Name of the `TChain` that you are looking for.
		/// @param branchNames Name of the branch names that you want to plot. See https://root.cern.ch/doc/master/classTChain.html#a8a2b55624f48451d7ab0fc3c70bfe8d7 for the syntax.
		/// @param cut Fill in a cut according to the syntax described <a href="https://root.cern.ch/doc/master/classTTree.html#a73450649dc6e54b5b94516c468523e45">here</a>.
		/// @param option Draw options.
		/// @param logScale If this argument contains an `'x'`, the \f$x\f$-scale will be set to log scale (same for `'y'` and `'z'`).
	void BOSSOutputLoader::Draw(const char* treeName, const char* branchNames, const char* cut, Option_t *option, const TString &logScale)
	{
		GetChainLoader(treeName).Draw(branchNames, cut, option, true, logScale);
	}


	/// Draw the distributions of all branches available in the ROOT file.
	void BOSSOutputLoader::Draw(const BranchPlotOptions &options)
	{
		if(!options.IsOK()) return;
		if(!HasChain(options.TreeName())) {
			TerminalIO::PrintWarning(Form("Cannot draw non-existing branch \"%s\" in file \"%s\"", options.TreeName(), fDirectoryPath.Data()));
			return;
		}
		GetChainLoader(options.TreeName()).Draw(options);
	}


	/// Draw the distributions of all branches available in the ROOT file.
		/// @param treeName Name of the `TChain` that you are looking for.
		/// @param option Draw options.
		/// @param logScale If this argument contains an `'x'`, the \f$x\f$-scale will be set to log scale (same for `'y'` and `'z'`).
	void BOSSOutputLoader::DrawAndSaveAllBranches(const char* treeName, Option_t *option, const TString &logScale)
	{
		GetChainLoader(treeName).DrawAndSaveAllBranches(option, logScale);
	}


	/// Draw the distributions of all branches available in the ROOT file.
		/// @param option Draw options.
	void BOSSOutputLoader::QuickDrawAndSaveAll(Option_t *option)
	{
		for(auto it = fChains.begin(); it != fChains.end(); ++it) it->second.DrawAndSaveAllBranches(option);
	}



// * =============================== * //
// * ------- PRIVATE METHODS ------- * //
// * =============================== * //


	/// Create a `list` that is ordered by the number of entries in the `TChain`.
	std::list<ChainLoader*> BOSSOutputLoader::CreateOrderedMap()
	{
		// * Create list of pointers to `ChainLoader` * //
		std::list<ChainLoader*> outputList;
		for(auto it = fChains.begin(); it != fChains.end(); ++it) {
			TString name(it->second.GetChain().GetName());
			// if(!name.EqualTo("_cutvalues"))
				outputList.push_back(&(it->second));
		}
		// * Sort resulting list based on number of entries * //
		outputList.sort([](ChainLoader* const & a, ChainLoader* const & b)
		{
			TString nameA(a->GetChain().GetName());
			TString nameB(b->GetChain().GetName());
			return a->GetEntries() != b->GetEntries() ?
				a->GetEntries() > b->GetEntries() :
				nameA < nameB;
		});
		return outputList;
	}


	/// Create a list of `TChain`s based on the files in the directory you are loading.
	/// If the ROOT `TFile` could be successfully loaded, this method will generate a `std::unordered_map` of pointers to all `TChain`s in the main folder of the ROOT file. The first file in the directory is used to make an inventory of all `TTree`s within the file.
	void BOSSOutputLoader::LoadChains(bool print_branches, bool print_averages)
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
					auto chain = &GetChainLoader(tree->GetName());
				/// <li> Loop over file names and at them to the `ChainLoader` object.
					for(auto filename : fFileNames) chain->Add(fDirectoryPath+"/"+filename);
				/// <li> Book branches using `ChainLoader::BookAddresses`.
					chain->BookAddresses(print_branches, print_averages);
				/// </ul>
			}
		/// -# Print terminal output
			std::cout << "Created a map of " << fChains.size() << " TChains from ";
			if(fFileNames.size() > 1) std::cout << fFileNames.size() << " files in ";
			std::cout << "input path \"" << fDirectoryPath.Data();
			if(fFileNames.size() == 1) std::cout << "/" << fFileNames.front(); // in case of one file
			std::cout << "\"" << std::endl;
	}