#ifndef BOSS_Afterburner_FitPars_DoubleGaussian_H
#define BOSS_Afterburner_FitPars_DoubleGaussian_H

// * ========================= * //
// * ------- LIBRARIES ------- * //
// * ========================= * //

	#include "FitPars.h"



// * ================================ * //
// * ------- CLASS DEFINITION ------- * //
// * ================================ * //
/// @addtogroup BOSS_Afterburner_objects
/// @{


	/// Derived container that contains the parameters of a double Gaussian fit.
	/// This object only contains the most essential parameters. You should define the actual parameters in the derived classes (such as `FitPars_DoubleGaussian_DoubleGauss)`.
	///
	/// A `FitPars_DoubleGaussian` object is added to 
	/// @author   Remco de Boer 雷穆克 (r.e.deboer@students.uu.nl or remco.de.boer@ihep.ac.cn)
	/// @date     Mrach 28th, 2019
	class FitPars_DoubleGaussian : public FitPars
	{
	public:
		FitPars_DoubleGaussian(const std::string &input) : FitPars(input) {};
		virtual void Import_impl(const std::string &input);
	private:
	};


/// @}
#endif