#ifndef BOSS_Afterburner_ConfigParameter_H
#define BOSS_Afterburner_ConfigParameter_H


// * ========================= * //
// * ------- LIBRARIES ------- * //
// * ========================= * //


	#include "AxisBinning.h"
	#include "BranchPlotOptions.h"
	#include "CommonFunctions.h"
	#include "ConfigParBase.h"
	#include "ReconstructedParticle.h"
	#include "TString.h"
	#include <iomanip>
	#include <iostream>
	#include <list>
	#include <sstream>
	#include <string>



// * ================================ * //
// * ------- CLASS DEFINITION ------- * //
// * ================================ * //
/// @addtogroup BOSS_Afterburner_objects
/// @{


	/// This `template` class is used to specify how to handle the values loaded to the `ConfigParBase` base class.
		/// You should define in this derived class how to go from the loaded string values to data. This has to defined for each `TYPE`. In principle, the `TYPE` is just a `typename`, that is, for instance a `double` or `int`. But you can also overload the `ConvertStringsToValue` function and specify in that overload how to go from the strings in `fReadStrings` to the data of your object.
	/// @author   Remco de Boer 雷穆克 (r.e.deboer@students.uu.nl or remco.de.boer@ihep.ac.cn)
	/// @date     March 15th, 2019
	template<class TYPE>
	class ConfigParameter : public ConfigParBase {
	public:
		ConfigParameter(const TString &identifier);
		ConfigParameter(const TString &identifier, const TYPE &default_value);
		TYPE& Get() { return fValue; }
    TYPE* operator->() { return &fValue; }
		TYPE& operator*() { return fValue; }
		void operator=(const TYPE &val) { fValue = val; ConvertValueToStrings(); }
		const bool operator==(const TString &data) const { return  GetIdentifier().EqualTo(data); }
		const bool operator!=(const TString &data) const { return !GetIdentifier().EqualTo(data); }
		operator TYPE() { return fValue; }
		void PrintValue() const;
	private:
		const bool ConvertStringsToValue_impl_str();
		virtual const bool ConvertStringsToValue_impl();
		virtual const bool ConvertValueToStrings_impl();
		const bool HasSingleString() const;
		TYPE fValue; /// The object that contains the data read from the `fReadStrings` `list`.
	};



/// @}
// * =============================== * //
// * ------- SPECIALISATIONS ------- * //
// * =============================== * //
// ! This list needs to be expended if you are implementing new template specialisations. Please keep sorted in alphabetical order.


	template class ConfigParameter<AxisBinning>;
	template class ConfigParameter<bool>;
	template class ConfigParameter<std::list<BranchPlotOptions> >;
	template class ConfigParameter<std::list<std::pair<ReconstructedParticle, BranchPlotOptions> > >;
	template class ConfigParameter<TString>;



// * ============================ * //
// * ------- CONSTRUCTORS ------- * //
// * ============================ * //


	template<class TYPE> inline
	ConfigParameter<TYPE>::ConfigParameter(const TString &identifier) :
		ConfigParBase(identifier) {}


	template<class TYPE> inline
	ConfigParameter<TYPE>::ConfigParameter(const TString &identifier, const TYPE &default_value) :
		ConfigParBase(identifier), fValue(default_value)
	{
		if(ConvertValueToStrings()) fValueIsSet = true;
	}



// * ===================================== * //
// * ------- ConvertValueToStrings ------- * //
// * ===================================== * //


	/// Default `ConvertValueToStrings` template function: works only for `type`s like `double`s, because this function relies on `std::stringstream`.
	template<typename TYPE> inline
	const bool ConfigParameter<TYPE>::ConvertValueToStrings_impl()
	{
		std::stringstream ss;
		ss << fValue;
		AddValue(ss.str());
		return true;
	}


	/// `ConvertValueToStrings` implementation handler for `AxisBinning`.
	template<> inline
	const bool ConfigParameter<AxisBinning>::ConvertValueToStrings_impl()
	{
		std::stringstream ss;
		if(!fValue.IsOK()) {
			std::cout << "WARNING: " << fValue.NBins() << ", " << fValue.From() << ", " << fValue.To() << std::endl;
			return false;
		}
		ss << fValue.NBins() << ", " << fValue.From() << ", " << fValue.To();
		AddValue(ss.str().c_str());
		return true;
	}


	/// `ConvertValueToStrings` implementation handler for `bool`.
	template<> inline
	const bool ConfigParameter<bool>::ConvertValueToStrings_impl()
	{
		if(fValue) AddValue("true");
		else       AddValue("false");
		return true;
	}


	/// `ConvertValueToStrings` implementation handler for a list of `BranchPlotOptions`.
		/// This parameter can for instance be used to [`TTree::Draw`](https://root.cern.ch/doc/master/classTTree.html#a8a2b55624f48451d7ab0fc3c70bfe8d7) a selection of branches (or of a combination of branches).
	template<> inline
	const bool ConfigParameter<std::list<BranchPlotOptions> >::ConvertValueToStrings_impl()
	{
		for(auto &it : fValue) {
			std::stringstream ss;
			ss << "{\"";
			ss << it.VarExp() << "\", \"";
			ss << it.CutSelection() << "\", \"";
			ss << it.DrawOption() << "\"}";
			AddValue(ss.str());
		}
		return true;
	}


	/// `ConvertValueToStrings` implementation handler for list of pairs of a `ReconstructedParticle` object (which characterise a signal that is to be fit) with a corresponding `BranchPlotOptions` object (which characterise the histogram that is to be fit).
	template<> inline
	const bool ConfigParameter<std::list<std::pair<ReconstructedParticle, BranchPlotOptions> > >::ConvertValueToStrings_impl()
	{
		for(auto &it : fValue) {
			AddValue((TString)Form("%d", it.first.GetPDGCode()));
			AddValue((TString)it.first.GetDaughterLabel());
			AddValue((TString)it.second.BuildOriginalString().Data());
		}
		return true;
	}



// * ===================================== * //
// * ------- ConvertStringsToValue ------- * //
// * ===================================== * //


	/// Default implementation handler for `ConvertStringsToValue` (only works for internal C++ types).
	template<typename TYPE> inline
	const bool ConfigParameter<TYPE>::ConvertStringsToValue_impl()
	{
		if(!HasSingleString()) return false;
		std::istringstream ss(fReadStrings.front());
		ss >> fValue;
		return true;
	}


	/// `ConvertStringsToValue` handler for an object that can be contructed from a `TString`.
	template<class TYPE> inline
	const bool ConfigParameter<TYPE>::ConvertStringsToValue_impl_str()
	{
		if(!HasSingleString()) return false;
		fValue = fReadStrings.front();
		return true;
	}


	/// `ConvertStringsToValue` handler for `AxisBinning`s (has a `string` constructor).
	template<> inline
	const bool ConfigParameter<AxisBinning>::ConvertStringsToValue_impl()
	{
		return ConvertStringsToValue_impl_str();
	}


	/// `ConvertStringsToValue` handler for `bool`eans.
	template<> inline
	const bool ConfigParameter<bool>::ConvertStringsToValue_impl()
	{
		if(!HasSingleString()) return false;
		if(
			fReadStrings.front().EqualTo("false") ||
			fReadStrings.front()[0] == '0') fValue = false;
		else fValue = true;
		return true;
	}


	/// `ConvertStringsToValue` handler for `bool`eans.
	template<> inline
	const bool ConfigParameter<std::list<BranchPlotOptions> >::ConvertStringsToValue_impl()
	{
		for(auto& it : fReadStrings) {
			fValue.push_back({it});
		}
		return true;
	}


	/// `ConvertStringsToValue` implementation handler for list of pairs of a `ReconstructedParticle` object (which characterise a signal that is to be fit) with a corresponding `BranchPlotOptions` object (which characterise the histogram that is to be fit).
	template<> inline
	const bool ConfigParameter<std::list<std::pair<ReconstructedParticle, BranchPlotOptions> > >::ConvertStringsToValue_impl()
	{
std::cout << "Till ConvertStringsToValue_impl" << std::endl;
		/// <ol>
		/// <li> **Abort** if the number of read strings for this parameter is not a multiple of 3. Each batch of 3 read values is used to construct a pair of a `ReconstructedParticle` with a `BranchPlotOptions`.
			if(fReadStrings.size()%3 != 0) {
				CommonFunctions::TerminalIO::PrintWarning(Form(
					"Converting parameter \"%s\" failed because it needs exactly 3 read values", GetIdentifier().Data()));
				return false;
			}
			for(auto it = fReadStrings.begin(); it != fReadStrings.end(); ++it) {
		/// <li> Read value 1: the PDG code or name of the decaying particle.
			TString pdgName{it->Data()};
			auto pdgCode{pdgName.Atoi()};
		/// <li> Read value 2: the daughters of the decaying particle.
			++it;
			TString daughters{it->Data()};
		/// <li> Read value 3: the input string for constructing the `BranchPlotOptions` object.
			++it;
			TString options{it->Data()};
		/// <li> Insert a pair (using constructor brackets) into the `fValue` list.
			if(pdgCode) fValue.push_back(std::make_pair(ReconstructedParticle{pdgCode,        daughters.Data()}, BranchPlotOptions{options}));
			else        fValue.push_back(std::make_pair(ReconstructedParticle{pdgName.Data(), daughters.Data()}, BranchPlotOptions{options}));
			}
		/// </ol>
		return true;
	}


	/// `ConvertStringsToValue` handler for `string`s.
	template<> inline
	const bool ConfigParameter<TString>::ConvertStringsToValue_impl()
	{
		return ConvertStringsToValue_impl_str();
	}



// * ========================== * //
// * ------- PrintValue ------- * //
// * ========================== * //


	/// By default, `fReadStrings` are printed in a vector style as an indication of the value value.
	template<class TYPE> inline
	void ConfigParameter<TYPE>::PrintValue() const
	{
		if(!fReadStrings.size()) return;
		if(fReadStrings.size() == 1)
			std::cout << fReadStrings.front();
		else {
			std::cout << "{" << std::endl;
			auto val = fReadStrings.begin();
			while(val != fReadStrings.end()) {
				std::cout << *val;
				++val;
				if(val != fReadStrings.end()) std::cout << "," << std::endl;
			}
			std::cout << " }";
		}
	}


	/// In case of a `AxisBinning` `fValue`, encapsulate its value in double quotations marks (`"`).
	template<> inline
	void ConfigParameter<AxisBinning>::PrintValue() const
	{
		std::cout << fValue.NBins() << " bins, range " << fValue.From() << " to " << fValue.To();
	}


	/// In case of a `bool` `fValue`, either `true` or `false` should be printed.
	template<> inline
	void ConfigParameter<bool>::PrintValue() const
	{
		if(fValue) std::cout << "true";
		else       std::cout << "false";
	}


	/// In case of a `AxisBinning` `fValue`, encapsulate its value in double quotations marks (`"`).
	template<> inline
	void ConfigParameter<std::list<BranchPlotOptions> >::PrintValue() const
	{
		std::cout << std::endl << std::endl;
		for(auto& it : fValue) it.Print();
	}


	/// In case of a `string` `fValue`, encapsulate its value in double quotations marks (`"`).
	template<> inline
	void ConfigParameter<TString>::PrintValue() const
	{
		std::cout << "\"" << fValue << "\"";
	}



// * ================================ * //
// * ------- HELPER FUNCTIONS ------- * //
// * ================================ * //


	/// Check if `fReadStrings` contains more than one value and print warning if so.
	/// This should not be the case in case `fValue` is of type `double`, `bool`, etc.
	template<typename TYPE> inline
	const bool ConfigParameter<TYPE>::HasSingleString() const
	{
		if(fReadStrings.size() > 1) {
			CommonFunctions::TerminalIO::PrintWarning(Form("Parameter \"%s\" is single value, but has multiple read values\n  --> Maybe define another template specialisation of ConvertStringsToValue?", GetIdentifier().Data()));
			return false;
		}
		return true;
	}



#endif