#ifndef BOSS_Afterburner_AxisBinning_H
#define BOSS_Afterburner_AxisBinning_H

// * ========================= * //
// * ------- LIBRARIES ------- * //
// * ========================= * //

	#include "CommonFunctions.h"
	#include "TString.h"
	#include <utility>



// * ================================= * //
// * ------- CLASS DEFINITIONS ------- * //
// * ================================= * //
/// @addtogroup BOSS_Afterburner_objects
/// @{


	/// Container object for the binning for *one* axis.
	/// Only contains the number of bins, the bin width and the plot range (from to).
	/// @author   Remco de Boer 雷穆克 (r.e.deboer@students.uu.nl or remco.de.boer@ihep.ac.cn)
	/// @date     March 23rd, 2019
	class AxisBinning
	{
	public:
		AxisBinning() : fIsOK(false), fNBins(0), fBinWidth(0.) {}
		AxisBinning(const TString &name, const int &nbins, const double &from, const double &to);
		AxisBinning(const TString &name, const double &width, const double &from, const double &to);
		AxisBinning(const TString &input);
		void SetName(const TString &name) { fAxisName = name; }
		void Set(const int &nbins, const double &from, const double &to);
		void Set(const double &width, const double &from, const double &to);
		void Set(const TString &binning);
		void SetBinWidth(const double &width);
		void SetNBins(const int &nbins);
		void SetRange(const double &from, const double &to);
		void SetFrom(const double &from);
		void SetTo(const double &to);
		const bool IsOK() const { return fIsOK; }
		const int& NBins() const { if(fIsOK) return fNBins; else return 0; }
		const double& From() const { if(fIsOK) return fRange.first; else return 0.; }
		const double& To() const { if(fIsOK) return fRange.second; else return 0.; }
		const double& BinWidth() const { if(fIsOK) return fBinWidth; else return 0.; }
		const double ComputeRange() const { return ComputeRange(fRange.first, fRange.second); }
		const TString& AxisName() const { return fAxisName; }
		void Print() const;

	private:
		TString fAxisName;
		bool fIsOK; ///< Switch that keeps track of whether arguments have been set correctly.
		int fNBins; ///< The number of bins on the axis.
		double fBinWidth; ///< The number of bins on the axis.CheckBinWidth
		std::pair<double, double> fRange; ///< Plot range from-to.
		static constexpr int gMaxNBins = 1e4;
		void Set(const int &nbins, const double &width, const double &from, const double &to);
		const double ComputeRange(const double &from, const double &to) const;
		const bool CheckNBins(const int &nbins) const;
		const bool CheckBinWidth(const double &width, const double & from, const double &to) const;
		const bool CheckRange(const double &from, const double &to) const;
		const double ComputeBinWidth(const int &nbins, const double &from, const double &to) const;
		const int ComputeNBins(const double &width, const double &from, const double &to) const;
	};



/// @}


#endif