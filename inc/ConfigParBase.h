#ifndef BOSS_Afterburner_ConfigParBase_H
#define BOSS_Afterburner_ConfigParBase_H


// * ========================= * //
// * ------- LIBRARIES ------- * //
// * ========================= * //

	#include "TString.h"
	#include <list>
	#include <string>
	#include <unordered_map>



// * ================================ * //
// * ------- CLASS DEFINITION ------- * //
// * ================================ * //
/// @addtogroup BOSS_Afterburner_objects
/// @{


	/// This is the base class for objects that hold parameters from a config file.
		/// This base class is necessary to allow one to loop over all different types (manifestations of `template`s) for `ConfigParameter`.
	/// @author   Remco de Boer 雷穆克 (r.e.deboer@students.uu.nl or remco.de.boer@ihep.ac.cn)
	/// @date     March 15th, 2019
	class ConfigParBase {
	public:
		ConfigParBase(const TString &identifier);
		~ConfigParBase();

		static void PrintAll();
		static ConfigParBase* GetParameter(const TString &identifier);
		static ConfigParBase* GetCleanParameter(const TString &identifier);
		static const size_t GetNParameters() { return fInstances.size(); }
		static const std::unordered_map<std::string, ConfigParBase*>* GetMapOfParameters() { return &fInstances; }

		const TString &GetIdentifier() const { return fIdentifier; }

		void AddValue(TString value);
		void ResetIfHasValue() { if(fValueIsSet) ClearValues(); }
		const size_t GetNReadValues() const { return fReadStrings.size(); }
		const std::list<TString>* GetListOfValues() { return &fReadStrings; }
		const bool ConvertStringsToValue();
		const bool ConvertValueToStrings();
		virtual void PrintValue() const = 0;
		const bool ValueIsSet() const { return fValueIsSet; }


	protected:
		virtual const bool ConvertStringsToValue_impl() = 0;
		virtual const bool ConvertValueToStrings_impl() = 0;
		std::list<TString> fReadStrings; ///< Loaded values in string format. You can specify in derived classes how to use these values.
		bool fValueIsSet; /// Switch that is used to prevent from double adding values to the `fReadStrings` `list`.


	private:
		void ClearValues() { fReadStrings.clear(); fValueIsSet = false; }
		const TString fIdentifier; ///< Unique identifier of the paramter. If this identifier is found in the configuration file you loaded with the `ConfigLoader`, its corresponding values will be added to `fReadStrings`. @warning The executable will `terminate` if the identifier already exists in the mapping of parameters `fInstances`.
		static std::unordered_map<std::string, ConfigParBase*> fInstances;
	};



/// @}
#endif