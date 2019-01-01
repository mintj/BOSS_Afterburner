// * ========================= * //
// * ------- LIBRARIES ------- * //
// * ========================= * //

	#include "CLHEP/Geometry/Point3D.h"
	#include "CLHEP/Vector/LorentzVector.h"
	#include "CLHEP/Vector/ThreeVector.h"
	#include "CLHEP/Vector/TwoVector.h"
	#include "D0phi_KpiKK/D0phi_KpiKK.h"
	#include "VertexFit/KalmanKinematicFit.h"
	#include "VertexFit/VertexFit.h"
	#include <string>
	#include <utility>

	#ifndef ENABLE_BACKWARDS_COMPATIBILITY
		typedef HepGeom::Point3D<double> HepPoint3D;
	#endif

	using CLHEP::Hep2Vector;
	using CLHEP::Hep3Vector;
	using CLHEP::HepLorentzVector;
	using namespace TSGlobals;



// * =========================== * //
// * ------- CONSTRUCTOR ------- * //
// * =========================== * //

	/**
	 * @brief Constructor for the `D0phi_KpiKK` algorithm.
	 * @details Here, you should declare properties: give them a name, assign a parameter (data member of `D0phi_KpiKK`), and if required a documentation string. Note that you should define the paramters themselves in the header (D0phi_KpiKK/D0phi_KpiKK.h) and that you should assign the values in `share/D0phi_KpiKK.txt`.
	 */
	D0phi_KpiKK::D0phi_KpiKK(const std::string &name, ISvcLocator* pSvcLocator) :
		TrackSelector(name, pSvcLocator)
	{
		fLog << MSG::DEBUG << "===>> D0phi_KpiKK::D0phi_KpiKK() <<===" << endmsg;

		/// * The `"write_<treename>"` properties determine whether or not the corresponding `TTree`/`NTuple` will be filled. Default values are set in the constructor as well.
		declareProperty("write_fit4c_all",      fWrite_fit4c_all      = false);
		declareProperty("write_fit4c_best",     fWrite_fit4c_best     = false);
		declareProperty("write_fit4c_best_D0",  fWrite_fit4c_best_D0  = false);
		declareProperty("write_fit4c_best_phi", fWrite_fit4c_best_phi = false);

	}



// * =============================== * //
// * ------- ALGORITHM STEPS ------- * //
// * =============================== * //

	/**
	 * @brief   (Inherited) `initialize` step of `Algorithm`. This function is called only once in the beginning.
	 * @details Define and load NTuples here.
	 */
	StatusCode D0phi_KpiKK::initialize_rest()
	{
		fLog << MSG::INFO << "===>> D0phi_KpiKK::initialize_rest() <<===" << endmsg;

		/// <ol type="A">
		/// <li> `"mult_select"`: Multiplicities of selected particles
			/// <ol>
			if(fWrite_mult_select) {
				fMap_mult_select["NKaonPos"]; /// <li> `"NKaonPos"`: Number of \f$K^+\f$.
				fMap_mult_select["NKaonNeg"]; /// <li> `"NKaonNeg"`: Number of \f$K^-\f$.
				fMap_mult_select["NPionPos"]; /// <li> `"NPionPos"`: Number of \f$\pi^-\f$.
				AddItemsToNTuples("mult_select", fMap_mult_select, "Multipliciies of selected particles");
			}
			/// </ol>

		/// <li> `"dedx_K"` and `"dedx_pi"`: energy loss \f$dE/dx\f$ PID branch. See `TrackSelector::BookNtupleItemsDedx` for more info.
			if(fWrite_dedx) {
				BookNtupleItemsDedx("dedx_K",  fMap_dedx_K);
				BookNtupleItemsDedx("dedx_pi", fMap_dedx_pi);
			}

		/// <li> `"fit4c_*"`: results of the Kalman kinematic fit results. See `TrackSelector::BookNtupleItemsFit` for more info.
			if(fWrite_fit4c_all)      BookNtupleItemsFit("fit4c_all",      fMap_fit4c_all,      "4-constraint fit information (CMS 4-momentum)");
			if(fWrite_fit4c_best)     BookNtupleItemsFit("fit4c_best",     fMap_fit4c_best,     "4-constraint fit information of the invariant masses closest to the reconstructed particles");
			if(fWrite_fit4c_best_D0)  BookNtupleItemsFit("fit4c_best_D0",  fMap_fit4c_best_D0,  "4-constraint fit information of the invariant masses closest to #m_{D^{0}}");
			if(fWrite_fit4c_best_phi) BookNtupleItemsFit("fit4c_best_phi", fMap_fit4c_best_phi, "4-constraint fit information of the invariant masses closest to #m_{\phi}");

		/// </ol>
		fLog << MSG::INFO << "Successfully returned from initialize()" << endmsg;
		return StatusCode::SUCCESS;
	}


	/**
	 * @brief Inherited `execute` method of the `Algorithm`. This function is called *for each event*.
	 */
	StatusCode D0phi_KpiKK::execute_rest()
	{
		fLog << MSG::DEBUG << "===>> D0phi_KpiKK::execute_rest() <<===" << endmsg;

		/// <ol type="A">
		/// <li> Create selection charged tracks

			// * Print log and set counters *
				fLog << MSG::DEBUG << "Starting particle selection:" << endmsg;
				fPIDInstance = ParticleID::instance();

			// * Clear vectors of selected particles *
				fKaonNeg.clear();
				fKaonPos.clear();
				fPionPos.clear();

			// * Loop over charged tracks *
			for(fTrackIterator = fGoodChargedTracks.begin(); fTrackIterator != fGoodChargedTracks.end(); ++fTrackIterator) {
				/// <ol>
				/// <li> Initialise PID and skip if it fails:
					/// <ul>
					if(!InitializePID(
						fPIDInstance->methodProbability(),
							/// <li> use <b>probability method</b>
						fPIDInstance->useDedx() |
						fPIDInstance->useTof1() |
						fPIDInstance->useTof2() |
						fPIDInstance->useTofE(),
							/// <li> use \f$dE/dx\f$ and the three ToF detectors
						fPIDInstance->onlyPion() |
						fPIDInstance->onlyKaon(),
							/// <li> identify only pions and kaons
						4.0
							/// <li> use \f$\chi^2 > 4.0\f$
					)) continue;
					/// </ul>

				/// <li> <b>Write</b> Particle Identification information of all tracks
					if(fWrite_PID) WritePIDInformation();

				/// <li> Identify type of charged particle and add to related vector: (this package: kaon and pion).
					RecMdcKalTrack* mdcKalTrk = (*fTrackIterator)->mdcKalTrack(); /// `RecMdcKalTrack` (Kalman) is used, but this can be substituted by `RecMdcTrack`.
					if(fPIDInstance->probPion() > fPIDInstance->probKaon()) { /// The particle identification first decides whether the track is more likely to have come from a pion or from a kaon.
						if(fPIDInstance->probPion() < fCut_PIDProb_min) continue; /// A cut is then applied on whether the probability to be a pion (or kaon) is at least `fCut_PIDProb_min` (see eventual settings in `D0phi_KpiKK.txt`).
						if(fPIDInstance->pdf(RecMdcKalTrack::pion) < fPIDInstance->pdf(RecMdcKalTrack::kaon)) continue; /// If, according to the likelihood method, the particle is still more likely to be a kaon than a pion, the track is rejected.
						RecMdcKalTrack::setPidType(RecMdcKalTrack::pion); /// Finally, the particle ID of the `RecMdcKalTrack` object is set to pion
						if(mdcKalTrk->charge() > 0) fPionPos.push_back(*fTrackIterator); /// and the (positive) pion is added to the vector of pions.
					}
					else {
						if(fPIDInstance->probKaon() < fCut_PIDProb_min) continue;
						if(fPIDInstance->pdf(RecMdcKalTrack::kaon) < fPIDInstance->pdf(RecMdcKalTrack::pion)) continue;
						RecMdcKalTrack::setPidType(RecMdcKalTrack::kaon);
						if(mdcKalTrk->charge() < 0) fKaonNeg.push_back(*fTrackIterator);
						else                        fKaonPos.push_back(*fTrackIterator);
					}

				/// </ol>
			}

			// * Finish good photon selection *
			fLog << MSG::DEBUG
				<< "N_{K^-}  = "  << fKaonNeg.size() << ", "
				<< "N_{K^+}  = "  << fKaonPos.size() << ", "
				<< "N_{\pi^+} = " << fPionPos.size() << endmsg;


		/// <li> <b>Write</b> the multiplicities of the selected particles.
			if(fWrite_mult_select) {
				fMap_mult_select.at("NKaonNeg") = fKaonNeg.size();
				fMap_mult_select.at("NKaonPos") = fKaonPos.size();
				fMap_mult_select.at("NPionPos") = fPionPos.size();
				fNTupleMap.at("mult_select")->write();
			}


		/// <li> Apply a strict cut on the number of particles: <i>only 2 negative kaons, 1 positive kaon, and 1 positive pion</i>
			if(fKaonNeg.size() != 2) return StatusCode::SUCCESS;
			if(fKaonPos.size() != 1) return StatusCode::SUCCESS;
			if(fPionPos.size() != 1) return StatusCode::SUCCESS;


		/// <li> <b>Write</b> \f$dE/dx\f$ PID information (`"dedx"` branch)
			if(fWrite_dedx) {
				WriteDedxInfoForVector(fKaonNeg, "dedx_K",  fMap_dedx_K);
				WriteDedxInfoForVector(fKaonPos, "dedx_K",  fMap_dedx_K);
				WriteDedxInfoForVector(fPionPos, "dedx_pi", fMap_dedx_pi);
			}


		/// <li> Perform Kalman 4-constraint Kalman kinematic fit for all combinations and decide the combinations that results in the 'best' result. The 'best' result is defined as the combination that has the smallest value of:
			/// \f[
			/// 	\left(\frac{m_{K^-\pi^+}-m_{D^0} }{m_{D^0} }\right) \cdot
			/// 	\left(\frac{m_{K^-K^+}  -m_{\phi}}{m_{\phi}}\right).
			/// \f]
			/// See `D0phi_KpiKK::MeasureForBestFit*` for the definition of this measure.
			if(fWrite_fit4c_all || fWrite_fit4c_best || fWrite_fit4c_best_D0 || fWrite_fit4c_best_phi) {
				// * Reset best fit parameters * //
				KKFitResult_D0phi_KpiKK bestKalmanFit;
				KKFitResult_D0phi_KpiKK bestKalmanFit_D0;
				KKFitResult_D0phi_KpiKK bestKalmanFit_phi;
				bestKalmanFit.fBestCompareValue     = 1e9;
				bestKalmanFit.fBestCompareValue_D0  = 1e9;
				bestKalmanFit.fBestCompareValue_phi = 1e9;
				// * Loop over all combinations of K-, K+, and pi+ * //
				for(fKaonNeg1Iter = fKaonNeg.begin(); fKaonNeg1Iter != fKaonNeg.end(); ++fKaonNeg1Iter)
				for(fKaonNeg2Iter = fKaonNeg.begin(); fKaonNeg2Iter != fKaonNeg.end(); ++fKaonNeg2Iter)
				for(fKaonPosIter  = fKaonPos.begin(); fKaonPosIter  != fKaonPos.end(); ++fKaonPosIter)
				for(fPionPosIter  = fPionPos.begin(); fPionPosIter  != fPionPos.end(); ++fPionPosIter)
				{
					if(fKaonNeg1Iter == fKaonNeg2Iter) continue;
					// * Get Kalman tracks reconstructed by the MDC
						RecMdcKalTrack* kalTrkKm1 = (*fKaonNeg1Iter)->mdcKalTrack();
						RecMdcKalTrack* kalTrkKm2 = (*fKaonNeg2Iter)->mdcKalTrack();
						RecMdcKalTrack* kalTrkKp  = (*fKaonPosIter) ->mdcKalTrack();
						RecMdcKalTrack* kalTrkpip = (*fPionPosIter) ->mdcKalTrack();

					// * Get W-tracks
						WTrackParameter wvKmTrk1(gM_K,  kalTrkKm1->getZHelix(), kalTrkKm1->getZError());
						WTrackParameter wvKmTrk2(gM_K,  kalTrkKm2->getZHelix(), kalTrkKm2->getZError());
						WTrackParameter wvKpTrk (gM_K,  kalTrkKp ->getZHelix(), kalTrkKp ->getZError());
						WTrackParameter wvpipTrk(gM_pi, kalTrkpip->getZHelix(), kalTrkpip->getZError());

					// * Test vertex fit
						HepPoint3D vx(0., 0., 0.);
						HepSymMatrix Evx(3, 0);
						double bx = 1E+6;
						double by = 1E+6;
						double bz = 1E+6;
						Evx[0][0] = bx*bx;
						Evx[1][1] = by*by;
						Evx[2][2] = bz*bz;

						VertexParameter vxpar;
						vxpar.setVx(vx);
						vxpar.setEvx(Evx);

						VertexFit* vtxfit = VertexFit::instance();
						vtxfit->init();
						vtxfit->AddTrack(0, wvKmTrk1);
						vtxfit->AddTrack(1, wvKmTrk2);
						vtxfit->AddTrack(2, wvKpTrk);
						vtxfit->AddTrack(3, wvpipTrk);
						vtxfit->AddVertex(0, vxpar, 0, 1);
						if(!vtxfit->Fit(0)) continue;
						vtxfit->Swim(0);

					// * Get Kalman kinematic fit for this combination and store if better than previous one
						KalmanKinematicFit *kkmfit = KalmanKinematicFit::instance();
						kkmfit->init();
						kkmfit->AddTrack(0, vtxfit->wtrk(0)); // K- (1st occurrence)
						kkmfit->AddTrack(1, vtxfit->wtrk(1)); // K- (2nd occurrence)
						kkmfit->AddTrack(2, vtxfit->wtrk(2)); // K+
						kkmfit->AddTrack(3, vtxfit->wtrk(3)); // pi+
						kkmfit->AddFourMomentum(0, gEcmsVec); // 4 constraints: CMS energy and 3-momentum
						if(kkmfit->Fit()) {
							/// -# Apply max. \f$\chi^2\f$ cut (determined by `fCut_PIDChiSq_max`).
							if(kkmfit->chisq() > fCut_PIDChiSq_max) continue;
							/// -# <b>Write</b> results of the Kalman kinematic fit (all combinations, `"fit4c_all"`).
							KKFitResult_D0phi_KpiKK fitresult(kkmfit);
							if(fWrite_fit4c_all) WriteFitResults(fitresult, fMap_fit4c_all, "fit4c_all");
							/// -# Decide if this fit is better than the previous
							if(fitresult.IsBetter())     bestKalmanFit     = fitresult;
							if(fitresult.IsBetter_D0())  bestKalmanFit_D0  = fitresult;
							if(fitresult.IsBetter_phi()) bestKalmanFit_phi = fitresult;
						}
				} // end of loop over particle combinations
		/// <li> <b>Write</b> results of the Kalman kitematic fit <i>of the best combination</i> (`"fit4c_best_*"` branches)
				if(fWrite_fit4c_best)     WriteFitResults(bestKalmanFit,     fMap_fit4c_best,     "fit4c_best");
				if(fWrite_fit4c_best_D0)  WriteFitResults(bestKalmanFit_D0,  fMap_fit4c_best_D0,  "fit4c_best_D0");
				if(fWrite_fit4c_best_phi) WriteFitResults(bestKalmanFit_phi, fMap_fit4c_best_phi, "fit4c_best_phi");

			} // end of fWrite_fit4c_*


		/// </ol>
		return StatusCode::SUCCESS;
	}


	/**
	 * @brief Currently does nothing. Cut flow could be printed in this step.
	 * @todo Add log output to `finalize` step.
	 */
	StatusCode D0phi_KpiKK::finalize_rest()
	{
		fLog << MSG::INFO << "===>> D0phi_KpiKK::finalize_rest() <<===" << endmsg;

		return StatusCode::SUCCESS;
	}



// * =============================== * //
// * ------- PRIVATE METHODS ------- * //
// * =============================== * //


	/**
	 * @brief Encapsulation of the procedure to write results of the Kalman kinematic fit (no matter how many constrains).
	 */
	void D0phi_KpiKK::WriteFitResults(KKFitResult_D0phi_KpiKK &fitresult, std::map<std::string, NTuple::Item<double> > &map, const char *tupleName)
	{
		if(!fitresult.HasFit()) {
			fLog << MSG::DEBUG << "KalmanKinematicFit for \"" << tupleName << "\" is empty" << endmsg;
			return;
		}
		fLog << MSG::DEBUG << "Writing fit results \"" << tupleName << "\"" << endmsg;
		map.at("mD0")   = fitresult.fM_D0;
		map.at("mphi")  = fitresult.fM_phi;
		map.at("mJpsi") = fitresult.fM_Jpsi;
		map.at("chisq") = fitresult.fChiSquared;
		fNTupleMap.at(tupleName)->write();
	}


	/**
	 * @brief This function encapsulates the `addItem` procedure for the fit branches.
	 */ 
	void D0phi_KpiKK::BookNtupleItemsFit(const char* tupleName, std::map<std::string, NTuple::Item<double> > &map, const char* tupleTitle)
	{
		/// <ol>
		map["mD0"];   /// <li> `"mD0"`:   Invariant mass for \f$K^- \pi^+\f$ (\f$D^0\f$).
		map["mphi"];  /// <li> `"mphi"`:  Invariant mass for \f$K^+ K^+  \f$ (\f$\phi\f$).
		map["mJpsi"]; /// <li> `"mJpsi"`: Invariant mass for \f$D^0 \phi \f$ (\f$J/\psi\f$).
		map["chisq"]; /// <li> `"chisq"`: \f$\chi^2\f$ of the Kalman kinematic fit.
		AddItemsToNTuples(tupleName, map, tupleTitle);
		/// </ol>
	}