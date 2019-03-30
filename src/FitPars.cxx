// * ========================= * //
// * ------- LIBRARIES ------- * //
// * ========================= * //

	#include "FitPars.h"
	#include "CommonFunctions.h"
	#include <iostream>
	using namespace CommonFunctions;



// * ======================= * //
// * ------- SETTERS ------- * //
// * ======================= * //


	/// @return TParticlePDG* Pointer to the `TParticlePDG` in the `TDatabasePDG` (located in `FrameworkSettings.h`). Is a `nullptr` if particle was not properly loaded.
	void FitPars::Import(const std::list<std::string> &input)
	{
		for(auto str &: input) {
			if(str.find('=') == std::string::npos) continue;
			std::string parname{};
			std::string parval {};
			Import_impl(str);
		}
	}