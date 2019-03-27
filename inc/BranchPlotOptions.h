#ifndef BOSS_Afterburner_BranchPlotOptions_H
#define BOSS_Afterburner_BranchPlotOptions_H

// * ========================= * //
// * ------- LIBRARIES ------- * //
// * ========================= * //
	#include "CommonFunctions.h"
	#include "AxisBinning.h"
	#include "TString.h"
	#include <vector>



// * ================================= * //
// * ------- CLASS DEFINITIONS ------- * //
// * ================================= * //
/// @addtogroup BOSS_Afterburner_objects
/// @{


	/// This object is used as a container of information you need to plot a branch or a collection of branches using the `TTree::Draw` method.
		/// The syntax for plotting a certain branch are determined through the `Import` method. The input should one line and should look as follows:
		/// ```
		/// 	"<tree name>; <list branches to plot here>; <cut selection>; <draw options>"
		/// ```
		/// Not the three semicolumns (`;`): you need at least these **four arguments** in the input line, seperated by a semicolumn. The `<tree name>` is necessary. In `<list branches to plot here>`, you need to give a least one branch to plot. The `<cut selection>` should follow the syntax of the `selection` argument in the [`TTree::Draw`](https://root.cern.ch/doc/master/classTTree.html#a73450649dc6e54b5b94516c468523e45) method, but can be left empty. The draw options should follow the [`THistPainter`](https://root.cern.ch/doc/master/classTHistPainter.html#HP01) plot option syntax, but can also be left empty.
		///
		/// As for the list of branches to plot, separate them with a semicolumn. One branch should look like *either* one of these:
		/// ```
		/// 	"<branch name>"
		/// 	"<branch name>, <number of bins>, <plot from>, <plot to>"
		/// 	"<branch name>, <bin width>, <plot from>, <plot to>"
		/// 	"<branch name>, <binning as one of the above two>, <cut selection>"
		/// 	"<branch name>, <cut selection>"
		/// ```
		/// Some examples of what this could look like:
		/// ```
		/// 	"treeName; branch1; branch2;sqrt(branch2) > 6.2e-3;"
		/// 	"treeName; branch1, 100, 1.2, 3.5;; E1"
		/// 	"treeName; branch1; branch2; colz; branch1 > 1.5 && branch1 < 2.8 && sqrt(branch2) > 6.2e-3; lego"
		/// ```
	/// @author   Remco de Boer 雷穆克 (r.e.deboer@students.uu.nl or remco.de.boer@ihep.ac.cn)
	/// @date     March 24th, 2018
	class BranchPlotOptions
	{
	public:
		BranchPlotOptions(const TString &input);


		/// @name Information
			///@{
			const bool IsOK() const;
			const bool IsWrite() const { return fOutputFileName.Length(); };
			void Print() const;
			///@}


		/// @name Setters
			///@{
			const bool Import(TString input);
			const bool AddBranch(const TString &input);
			void SetCutSelection(const TString &selection);
			void SetDrawOptions(TString input);
			void SetTreeName(const TString &treename);
			void SetOutputFileName(const TString &filename);
			///@}


		/// @name Getters
			///@{
			const size_t GetNBranches() const { return fListOfBranches.size(); }
			///@}

		/// @name TTree::Draw argument builders
			///@{
			const char* OutputFileName() const { return fOutputFileName.Data(); }
			const char* TreeName() const { return fTreeName.Data(); }
			const char* VarExp() const;
			const char* CutSelection() const { return fCutSelection.Data(); }
			const char* DrawOption() const { return fDrawOption.Data(); }
			const bool LogX() const { return fSetLog[0]; }
			const bool LogY() const { return fSetLog[1]; }
			const bool LogZ() const { return fSetLog[2]; }
			const bool HasLogScale() const;
			const TString BuildOriginalString() const;
			const TString BuildHistName() const;
			const TString LogXYZ() const;
			///@}

	private:
		TString fTreeName;
		TString fCutSelection;
		TString fDrawOption;
		TString fOutputFileName;
		std::vector<bool> fSetLog{false, false, false};
		std::vector<AxisBinning> fListOfBranches;
		bool fIsOK{false};

		/// @name Private helper functions
			///@{
			const bool ImportWarning(const TString &input, const TString &message, const bool notOK=true) const;
			const bool Tokenize(const TString &input, TString &tok, Ssiz_t &start, const char *delim=";") const;
			///@}

	};


/// @}
#endif