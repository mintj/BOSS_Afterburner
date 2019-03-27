// * ========================= * //
// * ------- LIBRARIES ------- * //
// * ========================= * //
	#include "AxisBinning.h"
	#include "CommonFunctions.h"
	#include "TList.h"
	#include <sstream>
	#include <cmath>
	using namespace CommonFunctions;



// * =========================== * //
// * ------- CONSTRUCTORS ------ * //
// * =========================== * //

	/// Construct the binnings from **number of bins**.
	AxisBinning::AxisBinning(const TString &name, const int &nbins, const double &from, const double &to) :
		fAxisName(name), fIsOK(false), fNBins(0), fBinWidth(0.), fRange(0., 0.)
	{
		Set(nbins, from, to);
	}


	/// Construct the binnings from **bin width**.
	AxisBinning::AxisBinning(const TString &name, const double &width, const double &from, const double &to) :
		fAxisName(name), fIsOK(false), fNBins(0), fBinWidth(0.), fRange(0., 0.)
	{
		Set(width, from, to);
	}


	/// Construct the binnings from a string.
	/// This constructor facilitates the `ConfigParameter::ConvertStringsToValue_impl_str` method.
	AxisBinning::AxisBinning(const TString &input) :
		fIsOK(false), fNBins(0), fBinWidth(0.), fRange(0., 0.)
	{
		/// -# Check input line for the number of commas (`,`).
			TString line { input };
			auto ncommas { line.CountChar(',') };
		/// -# If there are no commas in the `input` line, set only the axis name and abort. This means the `input` line is interpreted as the axis name only.
			if(!ncommas) {
				fAxisName = input;
				return;
			}
		/// -# **Abort** if there the number of commas is not three (you need a name, a bin width / number of bins and a from-to range)
			if(ncommas!=3 && ncommas!=0) {
				TerminalIO::PrintWarning(Form("AxisBinning construction from string \"%s\" has to contain 3 commas", input.Data()));
				return;
			}
		/// -# Get name from block before first comma.
			Ssiz_t start {0};
			TString tok;
			line.Tokenize(tok, start, ",");
			fAxisName = tok.Data();
		/// -# Use everything after the first comma to set the binning.
			line.Remove(0, start+1);
			Set(line.Data());
	}



// * ====================== * //
// * ------- SETTERS ------ * //
// * ====================== * //


	/// Private helper function to set all values at once.
	/// @warning Since `fBinWidth` and `fNBins` should be related
	void AxisBinning::Set(const int &nbins, const double &width, const double &from, const double &to)
	{
		if(!CheckBinWidth(width, from, to)) {
			TerminalIO::PrintWarning(Form("Problem with bin width %g: %d bins (from %g to %g)", width, nbins, from, to));
			return;
		}
		if(!CheckNBins(nbins)) {
			TerminalIO::PrintWarning(Form("Problem with %d bins: bin width %g (from %g to %g)", nbins, width, from, to));
			return;
		}
		fBinWidth = width;
		fNBins = nbins;
		fRange = { from, to };
		fIsOK = true;
	}


	/// Set the binnings from **number of bins**.
	void AxisBinning::Set(const int &nbins, const double &from, const double &to)
	{
		Set(nbins, ComputeBinWidth(nbins, from, to), from, to);
	}


	/// Set the binnings from **bin width**.
	void AxisBinning::Set(const double &width, const double &from, const double &to)
	{
		Set(ComputeNBins(width, from, to), width, from, to);
	}


	/// Intelligently set binnins from a string.
	void AxisBinning::Set(const TString &binning)
	{
		/// -# **Abort** if the string does not contain two commas.
			if(binning.CompareTo("")) return;
			int ncommas{binning.CountChar(',')};
			if(ncommas!=2) {
				TerminalIO::PrintWarning(Form("Binning string \"%s\" has to contain 2 commas", binning.Data()));
				return;
			}
		/// -# Set local variables to load from the string.
			int    nbins = 0;
			double width = 0.;
			double from  = 0.;
			double to    = 0.;
		/// -# Import the three arguments from the string.
			TString line(binning.Data());
			TString tok;
			UChar_t sw = 0;
			Ssiz_t start{0};
			while(line.Tokenize(tok, start, ",")) {
				switch(sw) {
					case 0 :
						if(tok.Contains(".") || tok.Contains("e") || tok.Contains("E")) width = tok.Atof();
						else nbins = tok.Atoi();
						break;
					case 1 : from = tok.Atof(); break;
					case 2 : to   = tok.Atof(); break;
					default :
						TerminalIO::PrintFatalError("AxisBinning::Set(const TString&)");
						std::terminate();
				}
				++sw;
			}
		/// -# Import the three arguments from the string
			if(nbins) Set(nbins, from, to); // if first argument is int
			else      Set(width, from, to); // if first argument is double
	}


	void AxisBinning::SetBinWidth(const double &width)
	{
		Set(ComputeNBins(width, fRange.first, fRange.second), width, fRange.first, fRange.second);
	}


	void AxisBinning::SetNBins(const int &nbins)
	{
		Set(nbins, ComputeBinWidth(nbins, fRange.first, fRange.second), fRange.first, fRange.second);
	}


	void AxisBinning::SetRange(const double &from, const double &to)
	{
		Set(fNBins, fBinWidth, from, to);
	}


	void AxisBinning::SetFrom(const double &from)
	{
		SetRange(from, fRange.second);
	}


	void AxisBinning::SetTo(const double &to)
	{
		SetRange(fRange.first, to);
	}



// * ====================== * //
// * ------- HELPERS ------ * //
// * ====================== * //


	const double AxisBinning::ComputeRange(const double &from, const double &to) const
	{
		return to - from;
	}


	const bool AxisBinning::CheckNBins(const int &nbins) const
	{
		return nbins < gMaxNBins && nbins > 0;
	}


	const bool AxisBinning::CheckBinWidth(const double &width, const double & from, const double &to) const
	{
		return width > 0 && ComputeNBins(width, from, to) < gMaxNBins;
	}


	const bool AxisBinning::CheckRange(const double &from, const double &to) const
	{
		return from < to;
	}


	const double AxisBinning::ComputeBinWidth(const int &nbins, const double &from, const double &to) const
	{
		return ComputeRange(from, to) / nbins;
	}


	const int AxisBinning::ComputeNBins(const double &width, const double &from, const double &to) const
	{
		return std::ceil(ComputeRange(from, to) / width);
	}





// * ========================== * //
// * ------- INFORMATION ------ * //
// * ========================== * //


	void AxisBinning::Print() const
	{
		std::cout << "    \"" << AxisName() << "\"";
		if(IsOK()) std::cout << ": " << NBins() << " bins, from " << From() << " to " << To() << " (width " << BinWidth() << ")";
		std::cout << std::endl;
	}