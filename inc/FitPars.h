#ifndef BOSS_Afterburner_FitPars_H
#define BOSS_Afterburner_FitPars_H

// * ========================= * //
// * ------- LIBRARIES ------- * //
// * ========================= * //

	#include <list>
	#include <string>



// * ================================ * //
// * ------- CLASS DEFINITION ------- * //
// * ================================ * //
/// @addtogroup BOSS_Afterburner_objects
/// @{


	/// Base container for fit parameters.
	/// This object only contains the most essential parameters. You should define the actual parameters in the derived classes (such as `FitPars_DoubleGauss)`.
	///
	/// A `FitPars` object is added to 
	/// @author   Remco de Boer 雷穆克 (r.e.deboer@students.uu.nl or remco.de.boer@ihep.ac.cn)
	/// @date     Mrach 28th, 2019
	class FitPars
	{
	public:
		FitPars(const std::list<std::string> &input);
		void Import(const std::list<std::string> &input);
		virtual void Import_impl(const std::list<std::string> &input) = 0;

		/// @name Enumerate for fit types
			///@{
			enum ESignalType {
				kSingleGauss,
				kDoubleGauss,
				kBW,
				kBWconvSingleG,
				kBWconvDoubleG
			}; ///< Enumerate for the different types of fit types for the signal.
			enum EBkgType {
				kPolynomial,
				kArgus,
				kRooChebychev
			}; ///< Enumerate for the different types of background types.
			///@}
	private:
	};


/// @}
#endif