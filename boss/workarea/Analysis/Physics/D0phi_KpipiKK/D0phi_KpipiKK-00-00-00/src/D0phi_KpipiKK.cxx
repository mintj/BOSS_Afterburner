// * ========================= * //
// * ------- LIBRARIES ------- * //
// * ========================= * //

	#include "CLHEP/Geometry/Point3D.h"
	#include "CLHEP/Vector/LorentzVector.h"
	#include "CLHEP/Vector/ThreeVector.h"
	#include "CLHEP/Vector/TwoVector.h"
	#include "D0phi_KpipiKK/D0phi_KpipiKK.h"
	#include "VertexFit/KalmanKinematicFit.h"
	#include "VertexFit/VertexFit.h"
	#include <float.h> // for DBL_MAX
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


	/// Constructor for the `D0phi_KpipiKK` algorithm, derived from `TrackSelector`.
	/// Here, you should declare properties: give them a name, assign a parameter (data member of `D0phi_KpipiKK`), and if required a documentation string. Note that you should define the paramters themselves in the header (D0phi_KpipiKK/D0phi_KpipiKK.h) and that you should assign the values in `share/D0phi_KpipiKK.txt`.
	D0phi_KpipiKK::D0phi_KpipiKK(const std::string &name, ISvcLocator* pSvcLocator) :
		/// * Construct base algorithm `TrackSelector`.
			TrackSelector(name, pSvcLocator),
		/// * Construct `NTuple::Tuple` containers used in derived classes.
			fNTuple_dedx_K    ("dedx_K",     "dE/dx of the kaons"),
			fNTuple_dedx_pi   ("dedx_pi",    "dE/dx of the pions"),
			fNTuple_fit5c_all ("fit5c_all",  "5-constraint fit information (CMS 4-momentum)"),
			fNTuple_fit5c_best("fit5c_best", "5-constraint fit information of the invariant masses closest to the reconstructed particles"),
			fNTuple_fit_mc    ("fit_mc",     "Fake fit information according to MC truth"),
			fNTuple_photon    ("photon",     "Kinematics of selected photons"),
		/// * Construct counters (in essence a `CutObject` without cuts).
			fCutFlow_NChargedOK   ("N_charged_OK",    "Number of events that had exactly 4 charged tracks"),
			fCutFlow_NFitOK       ("N_Fit_OK",        "Number of combinations where where the kinematic fit worked"),
			fCutFlow_NPIDnumberOK ("N_PID_OK",        "Number of events that had exactly 2 K-, 1 K+ and 1 pi+ PID tracks"),
			fCutFlow_NetChargeOK  ("N_netcharge_OK",  "Number of events where the total charge detected in the detectors was 0"),
			fCutFlow_mD0_mphi     ("N_mD0_mphi",      "Number of events that passed the wide cut on both invariant masses"),
			fCutFlow_mD0_mphi_3sig("N_mD0_mphi_3sig", "Number of events that passed the 3sigma cut on both invariant masses"),
		/// * Construct `CutObject`s. The `"cut_<parameter>_min/max"` properties determine cuts on certain parameters.
			fCut_mphi     ("mphi"),
			fCut_mD0      ("mD0"),
			fCut_mphi_3sig("mphi_3sig"),
			fCut_mD0_3sig ("mD0_3sig"),
		/// * Construct additional `CutObject`s that are specific for the `rhopi_pipig` package.
			fCut_GammaAngle("gamma_angle"),
			fCut_GammaPhi  ("gamma_phi"),
			fCut_GammaTheta("gamma_theta")
	{ PrintFunctionName("D0phi_KpipiKK", __func__); PostConstructor();
		fCreateChargedCollection = true;
		fCreateNeutralCollection = true;
	}



// * =============================== * //
// * ------- ALGORITHM STEPS ------- * //
// * =============================== * //


	/// (Inherited) `initialize` step of `Algorithm`.
	/// This function is called only once, when the algorithm is initialised.
	/// @remark Define and load `NTuple`s here. Other `NTuple`s have already been defined in the `TrackSelector::initilize` step prior to this this method.
	StatusCode D0phi_KpipiKK::initialize_rest()
	{ PrintFunctionName("D0phi_KpipiKK", __func__);
		/// <ol type="A">
		/// <li> `"mult_select"`: Multiplicities of selected particles
			/// <ol>
			fNTuple_mult_sel.AddItem<int>("NPhotons"); /// <li> `"NPhotons"`: Number of \f$\gamma\f$.
			fNTuple_mult_sel.AddItem<int>("NKaonPos"); /// <li> `"NKaonPos"`: Number of \f$K^+\f$.
			fNTuple_mult_sel.AddItem<int>("NKaonNeg"); /// <li> `"NKaonNeg"`: Number of \f$K^-\f$.
			fNTuple_mult_sel.AddItem<int>("NPionPos"); /// <li> `"NPionPos"`: Number of \f$\pi^-\f$.
			/// </ol>

		/// <li> `"dedx_K"` and `"dedx_pi"`: energy loss \f$dE/dx\f$ PID branch. See `TrackSelector::AddNTupleItems_Dedx` for more info.
			AddNTupleItems_Dedx(fNTuple_dedx_K);
			AddNTupleItems_Dedx(fNTuple_dedx_pi);

		/// <li> `"fit5c_*"`: results of the Kalman kinematic fit results. See `TrackSelector::AddNTupleItems_Fit` for more info.
			AddNTupleItems_Fit(fNTuple_fit5c_all);
			AddNTupleItems_Fit(fNTuple_fit5c_best);
			AddNTupleItems_Fit(fNTuple_fit_mc);

		/// <li> `"photon"`: information of the selected photons
			/// <ol>
			fNTuple_photon.AddItem<double>("E");  /// <li> `"E"`:  Energy of the photon.
			fNTuple_photon.AddItem<double>("px"); /// <li> `"px"`: \f$x\f$ component of the 5-momentum of the photon (computed from the detected angles).
			fNTuple_photon.AddItem<double>("py"); /// <li> `"py"`: \f$y\f$ component of the 5-momentum of the photon (computed from the detected angles).
			fNTuple_photon.AddItem<double>("pz"); /// <li> `"pz"`: \f$z\f$ component of the 5-momentum of the photon (computed from the detected angles).
			fNTuple_photon.AddItem<double>("smallest_phi");   /// <li> `"phi"`:   Smallest \f$\phi\f$ angle between the photon and the nearest charged pion.
			fNTuple_photon.AddItem<double>("smallest_theta"); /// <li> `"theta"`: Smallest \f$\theta\f$ angle between the photon and the nearest charged pion.
			fNTuple_photon.AddItem<double>("smallest_angle"); /// <li> `"angle"`: Smallest angle between the photon and the nearest charged pion.
			/// </ol>

		/// <li> `"topology"`: Add @b extra mass branches to the MC truth branch for the `topoana` package
			/// <ol>
			fNTuple_topology.AddItem<double>("chi2"); /// <li> `"chi2"`: \f$\chi^2\f$ of the Kinematic kalman fit.
			fNTuple_topology.AddItem<double>("mpi0"); /// <li> `"mpi0"`: Invariant mass of the \f$\pi^0\f$ candidate in \f$D^0 \to K^-\pi^+\pi^0\f$.
			fNTuple_topology.AddItem<double>("mD0");  /// <li> `"mD0"`:  Invariant mass of the \f$D^0\f$ candidate in the \f$J/\psi \to D^0\phi\f$.
			fNTuple_topology.AddItem<double>("mphi"); /// <li> `"mphi"`: Invariant mass of the \f$\phi\f$ candidate in the \f$J/\psi \to D^0\phi\f$.
			fNTuple_topology.AddItem<double>("pD0");  /// <li> `"pD0"`:  Momentum of the \f$D^0\f$ candidate in the \f$J/\psi \to D^0\phi\f$.
			fNTuple_topology.AddItem<double>("pphi"); /// <li> `"pphi"`: Momentum of the \f$\phi\f$ candidate in the \f$J/\psi \to D^0\phi\f$.
			/// </ol>

		/// </ol>
		return StatusCode::SUCCESS;
	}


	/// Inherited `execute` method of the `Algorithm`.
	/// This function is called *for each event*.
	StatusCode D0phi_KpipiKK::execute_rest()
	{ PrintFunctionName("D0phi_KpipiKK", __func__);
		/// <ol type="A">
		/// <li> **Charged track cut**: Apply a strict cut on the number of particles. Only **4 charged tracks in total**.
			if(fGoodChargedTracks.size() != 4) return StatusCode::SUCCESS;
			++fCutFlow_NChargedOK;


		/// <li> **Net charge cut**: Apply a strict cut on the total charge detected in the detectors. If this charge is not \f$0\f$, this means some charged tracks have not been detected.
			if(fNetChargeMDC) return StatusCode::SUCCESS;
			++fCutFlow_NetChargeOK;


		/// <li> Create selection of **charged** tracks.
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
						/// <li> use <b>probability method</b>
						fPIDInstance->methodProbability(),
						/// <li> use \f$dE/dx\f$ and the three ToF detectors. Since data reconstructed with BOSS 7.0.4, `ParticleID::useTofCorr()` should be used for ToF instead of e.g. `useTof1`.
						fPIDInstance->useDedx() |
						fPIDInstance->useTof1() |
						fPIDInstance->useTof2() |
						fPIDInstance->useTofE(),
						/// <li> identify only pions and kaons
						fPIDInstance->onlyPion() |
						fPIDInstance->onlyKaon(),
						/// <li> use \f$\chi^2 > 4.0\f$
						4.0
					)) continue;
					/// </ul>

				/// <li> @b Write Particle Identification information of all tracks
					WritePIDInformation();

				/// <li> Identify type of charged particle and add to related vector: (this package: kaon and pion).
					fTrackKal = (*fTrackIterator)->mdcKalTrack();
					if(fPIDInstance->probPion() > fPIDInstance->probKaon()) { /// The particle identification first decides whether the track is more likely to have come from a pion or from a kaon.
						// if(fPIDInstance->pdf(RecMdcKalTrack::pion) < fPIDInstance->pdf(RecMdcKalTrack::kaon)) continue; /// If, according to the likelihood method, the particle is still more likely to be a kaon than a pion, the track is rejected.
						if(fCut_PIDProb.FailsMin(fPIDInstance->probPion())) continue; /// A cut is then applied on whether the probability to be a pion (or kaon) is at least `fCut_PIDProb_min` (see eventual settings in `D0phi_KpipiKK.txt`).
						RecMdcKalTrack::setPidType(RecMdcKalTrack::pion); /// Finally, the particle ID of the `RecMdcKalTrack` object is set to pion
						if(fTrackKal->charge()>0) fPionPos.push_back(*fTrackIterator); /// and the (positive) pion is added to the vector of pions.
					}
					else {
						// if(fPIDInstance->pdf(RecMdcKalTrack::kaon) < fPIDInstance->pdf(RecMdcKalTrack::pion)) continue;
						if(fCut_PIDProb.FailsMin(fPIDInstance->probKaon())) continue;
						RecMdcKalTrack::setPidType(RecMdcKalTrack::kaon);
						if(fTrackKal->charge()<0) fKaonNeg.push_back(*fTrackIterator);
						else                      fKaonPos.push_back(*fTrackIterator);
					}

				/// </ol>
			}


		/// <li> Create selection **neutral** tracks (photons)
			fPhotons.clear();
			for(fTrackIterator = fGoodNeutralTracks.begin(); fTrackIterator != fGoodNeutralTracks.end(); ++fTrackIterator) {

				/// <ol>
				/// <li> Get EM calorimeter info.
					fTrackEMC = (*fTrackIterator)->emcShower();
					Hep3Vector emcpos = fTrackEMC->position();

				/// <li> Find angle differences with nearest charged pion.
					double smallestTheta = DBL_MAX; // start value for difference in theta
					double smallestPhi   = DBL_MAX; // start value for difference in phi
					double smallestAngle = DBL_MAX; // start value for difference in angle (?)
					// Note: `fPionPosIter` is just used as a dummy iterator and has nothing to do with pi+
					for(fPionPosIter = fGoodChargedTracks.begin(); fPionPosIter != fGoodChargedTracks.end(); ++fPionPosIter) {
						/// * Get the extension object from MDC to EMC.
						if(!(*fPionPosIter)->isExtTrackValid()) continue;
						fTrackExt = (*fPionPosIter)->extTrack();
						if(fTrackExt->emcVolumeNumber() == -1) continue;
						Hep3Vector extpos(fTrackExt->emcPosition());

						/// * Get angles in @b radians.
						// double cosTheta = extpos.cosTheta(emcpos);
						double angle  = extpos.angle(emcpos);
						double dTheta = extpos.theta() - emcpos.theta();
						double dPhi   = extpos.deltaPhi(emcpos);
						dTheta = fmod(dTheta + CLHEP::twopi + CLHEP::twopi + pi, CLHEP::twopi) - CLHEP::pi;
						dPhi   = fmod(dPhi   + CLHEP::twopi + CLHEP::twopi + pi, CLHEP::twopi) - CLHEP::pi;
						if(angle < smallestAngle){
							smallestAngle = angle;
							smallestTheta = dTheta;
							smallestPhi   = dPhi;
						}
					}

				/// <li> Convert angles from radians to degrees.
					smallestTheta *= (180. / (CLHEP::pi));
					smallestPhi   *= (180. / (CLHEP::pi));
					smallestAngle *= (180. / (CLHEP::pi));

				/// <li> @b Write photon info (`"photon"` branch).
					if(fNTuple_photon.DoWrite()) {
						double eraw  = fTrackEMC->energy();
						double phi   = fTrackEMC->phi();
						double theta = fTrackEMC->theta();
						HepLorentzVector four_mom(
							eraw * sin(theta) * cos(phi),
							eraw * sin(theta) * sin(phi),
							eraw * cos(theta),
							eraw);
						fNTuple_photon.GetItem<double>("E")  = four_mom.e();
						fNTuple_photon.GetItem<double>("px") = four_mom.px();
						fNTuple_photon.GetItem<double>("py") = four_mom.py();
						fNTuple_photon.GetItem<double>("pz") = four_mom.pz();
						fNTuple_photon.GetItem<double>("smallest_phi")   = smallestTheta;
						fNTuple_photon.GetItem<double>("smallest_theta") = smallestPhi;
						fNTuple_photon.GetItem<double>("smallest_angle") = smallestAngle;
						fNTuple_photon.Write();
					}

				/// <li> Apply angle cut on the photons: you do not want to photons to be too close to any charged track to avoid noise from EMC showers that came from a charged track.
					if(
						fCut_GammaTheta.FailsCut(fabs(smallestTheta)) &&
						fCut_GammaPhi  .FailsCut(fabs(smallestPhi))) continue;
					if(fCut_GammaAngle.FailsCut(fabs(smallestAngle))) continue;

				/// <li> Add photon track to vector for gammas
					fPhotons.push_back(*fTrackIterator);

				/// </ol>
			}


		/// <li> **Write** the multiplicities of the selected particles.
			fLog << MSG::DEBUG
				<< "N_{K^-} = "   << fKaonNeg.size() << ", "
				<< "N_{K^+} = "   << fKaonPos.size() << ", "
				<< "N_{\pi^+} = " << fPionPos.size() << endmsg;
			if(fNTuple_mult_sel.DoWrite()) {
				fNTuple_mult_sel.GetItem<int>("NKaonNeg") = fKaonNeg.size();
				fNTuple_mult_sel.GetItem<int>("NKaonPos") = fKaonPos.size();
				fNTuple_mult_sel.GetItem<int>("NPionPos") = fPionPos.size();
				fNTuple_mult_sel.GetItem<int>("NPhoton")  = fPhotons.size();
				fNTuple_mult_sel.Write();
			}


		/// <li> **PID cut**: apply a strict cut on the number of the selected particles. Only continue if:
			/// <ol>
			if(fKaonNeg.size() != 2) return StatusCode::SUCCESS; /// <li> 2 negative kaons
			if(fKaonPos.size() != 1) return StatusCode::SUCCESS; /// <li> 1 positive kaon
			if(fPionPos.size() != 1) return StatusCode::SUCCESS; /// <li> 1 positive pion
			if(fPhotons.size()  < 2) return StatusCode::SUCCESS; /// <li> at least 2 photons (\f$\gamma\f$'s)
			/// </ol>
			++fCutFlow_NPIDnumberOK;
			fLog << MSG::INFO << "PID selection passed for (run, event) = ("
				<< fEventHeader->runNumber() << ", "
				<< fEventHeader->eventNumber() << ")" << endmsg;

		/// <li> **Write** \f$dE/dx\f$ PID information (`"dedx_*"` branchs)
			WriteDedxInfoForVector(fKaonNeg, fNTuple_dedx_K);
			WriteDedxInfoForVector(fKaonPos, fNTuple_dedx_K);
			WriteDedxInfoForVector(fPionPos, fNTuple_dedx_pi);


		/// <li> Loop over MC truth of final decay products.
			for(fMcPhoton1Iter  = fMcPhotons.begin(); fMcPhoton1Iter  != fMcPhotons.end(); ++fMcPhoton1Iter)
			for(fMcPhoton2Iter  = fMcPhotons.begin(); fMcPhoton2Iter  != fMcPhotons.end(); ++fMcPhoton2Iter)
			for(fMcKaonNeg1Iter = fMcKaonNeg.begin(); fMcKaonNeg1Iter != fMcKaonNeg.end(); ++fMcKaonNeg1Iter)
			for(fMcKaonNeg2Iter = fMcKaonNeg.begin(); fMcKaonNeg2Iter != fMcKaonNeg.end(); ++fMcKaonNeg2Iter)
			for(fMcKaonPosIter  = fMcKaonPos.begin(); fMcKaonPosIter  != fMcKaonPos.end(); ++fMcKaonPosIter)
			for(fMcPionPosIter  = fMcPionPos.begin(); fMcPionPosIter  != fMcPionPos.end(); ++fMcPionPosIter)
			{
				/// <ol>
				/// <li> Only continue if the two kaons are different.
					if(fMcPhoton1Iter  == fMcPhoton2Iter)  continue;
					if(fMcKaonNeg1Iter == fMcKaonNeg2Iter) continue;
				/// <li> Check topology: only consider that combination which comes from \f$J/\psi \rightarrow D^0\phi \rightarrow K^-\pi^+ K^-K^+\f$.
					if(!IsDecay(*fMcKaonNeg1Iter, 421, -321)) continue; // D0  --> K-
					if(!IsDecay(*fMcPionPosIter,  421,  211)) continue; // D0  --> pi+
					if(!IsDecay(*fMcKaonNeg2Iter, 333, -321)) continue; // phi --> K-
					if(!IsDecay(*fMcKaonPosIter,  333,  321)) continue; // phi --> K+
				/// <li> Write 'fake' fit results, that is, momenta of the particles reconstructed from MC truth.
					KKFitResult_D0phi_KpipiKK fitresult(
						*fMcKaonNeg1Iter,
						*fMcKaonNeg2Iter,
						*fMcKaonPosIter,
						*fMcPionPosIter,
						*fMcPhoton1Iter,
						*fMcPhoton2Iter
					);
					WriteFitResults(&fitresult, fNTuple_fit_mc);
				/// </ol>
			}


		/// <li> Perform Kalman **5-constraint** Kalman kinematic fit for all combinations and decide the combinations that results in the 'best' result. The 'best' result is defined as the combination that has the smallest value of: \f$m_{K^-K^+}-m_{\phi}\f$ (that is the combination for which the invariant mass of the \f$K^-\pi^+\f$ is closest to \f$\phi\f$). See `D0phi_KpipiKK::MeasureForBestFit` for the definition of this measure. @todo Decide whether 5-constraints is indeed suitable.
			/// <ol>
			if(fNTuple_fit5c_all.DoWrite() || fNTuple_fit5c_best.DoWrite()) {
				/// <li> Reset best fit parameters (see `KKFitResult_D0phi_KpipiKK`).
					KKFitResult_D0phi_KpipiKK bestKalmanFit;
					bestKalmanFit.ResetBestCompareValue();
				/// <li> Loop over all combinations of \f$\gamma\f$, \f$K^-\f$, \f$K^+\f$, and \f$\pi^+\f$.
					/// <ol>
					bool printfit{true};
					int count = 0;
					for(fPhoton1Iter  = fPhotons.begin(); fPhoton1Iter  != fPhotons.end(); ++fPhoton1Iter)
					for(fPhoton2Iter  = fPhotons.begin(); fPhoton2Iter  != fPhotons.end(); ++fPhoton2Iter)
					for(fKaonNeg1Iter = fKaonNeg.begin(); fKaonNeg1Iter != fKaonNeg.end(); ++fKaonNeg1Iter)
					for(fKaonNeg2Iter = fKaonNeg.begin(); fKaonNeg2Iter != fKaonNeg.end(); ++fKaonNeg2Iter)
					for(fKaonPosIter  = fKaonPos.begin(); fKaonPosIter  != fKaonPos.end(); ++fKaonPosIter)
					for(fPionPosIter  = fPionPos.begin(); fPionPosIter  != fPionPos.end(); ++fPionPosIter)
					{
						/// <li> Only continue if we are not dealing with the same kaon and/or photon.
							if(fPhoton1Iter == fPhoton2Iter) continue;
							if(fKaonNeg1Iter == fKaonNeg2Iter) continue;
							fLog << MSG::INFO << "  fitting combination " << count << "..." << endmsg;

						/// <li> Get Kalman tracks reconstructed by the MDC.
							RecMdcKalTrack* kalTrkKm1 = (*fKaonNeg1Iter)->mdcKalTrack();
							RecMdcKalTrack* kalTrkKm2 = (*fKaonNeg2Iter)->mdcKalTrack();
							RecMdcKalTrack* kalTrkKp  = (*fKaonPosIter) ->mdcKalTrack();
							RecMdcKalTrack* kalTrkpip = (*fPionPosIter) ->mdcKalTrack();

						/// <li> Get W-tracks.
							WTrackParameter wvKmTrk1(gM_K,  kalTrkKm1->getZHelix(), kalTrkKm1->getZError());
							WTrackParameter wvKmTrk2(gM_K,  kalTrkKm2->getZHelix(), kalTrkKm2->getZError());
							WTrackParameter wvKpTrk (gM_K,  kalTrkKp ->getZHelix(), kalTrkKp ->getZError());
							WTrackParameter wvpipTrk(gM_pi, kalTrkpip->getZHelix(), kalTrkpip->getZError());

						/// <li> Initiate vertex fit.
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

						/// <li> Test vertex fit.
							VertexFit* vtxfit = VertexFit::instance();
							vtxfit->init();
							vtxfit->AddTrack(0, wvKmTrk1);
							vtxfit->AddTrack(1, wvKmTrk2);
							vtxfit->AddTrack(2, wvKpTrk);
							vtxfit->AddTrack(3, wvpipTrk);
							vtxfit->AddVertex(0, vxpar, 0, 1);
							if(!vtxfit->Fit(0)) {
								fLog << MSG::WARNING << "vertex fit failed" << endmsg;
								continue;
							}
							vtxfit->Swim(0);

						/// <li> Get Kalman kinematic fit for this combination and store if better than previous one.
							KalmanKinematicFit *kkmfit = KalmanKinematicFit::instance();
							kkmfit->init();
							kkmfit->AddTrack(0, vtxfit->wtrk(0)); // K- (1st occurrence)
							kkmfit->AddTrack(1, vtxfit->wtrk(1)); // K- (2nd occurrence)
							kkmfit->AddTrack(2, vtxfit->wtrk(2)); // K+
							kkmfit->AddTrack(3, vtxfit->wtrk(3)); // pi+
							kkmfit->AddTrack(4, 0., (*fPhoton1Iter)->emcShower()); // gamma (1st occurrence)
							kkmfit->AddTrack(5, 0., (*fPhoton2Iter)->emcShower()); // gamma (2nd occurence)
							kkmfit->AddFourMomentum(0, gEcmsVec); // 4 constraints: CMS energy and 3-momentum
							if(kkmfit->Fit()) {
								/// <ol>
								/// <li> Apply max. \f$\chi^2\f$ cut (determined by `fCut_PIDChiSq_max`).
								if(fCut_PIDChiSq.FailsMax(kkmfit->chisq())) continue;
								if(printfit) {
									fLog << MSG::INFO << "  SUCCESS: chisq cut passed with chisq =" << kkmfit->chisq() << endmsg;
									printfit = false;
								}
								/// <li> Construct fit result object for this combintation.
								KKFitResult_D0phi_KpipiKK fitresult(kkmfit);
								/// <li> @b Write results of the Kalman kinematic fit (all combinations, `"fit5c_all"`).
								WriteFitResults(&fitresult, fNTuple_fit5c_all);
								/// <li> Decide if this fit is better than the previous.
								if(fitresult.IsBetter()) bestKalmanFit = fitresult;
								/// </ol>
							} else if(printfit) {
								fLog << MSG::INFO << "  fit failed: chisq = " << kkmfit->chisq() << endmsg;
								printfit = false;
							}
						++count;
					}
					/// </ol>
				/// <li> **Write** results of the Kalman kitematic fit <i>of the best combination</i> (`"fit5c_best"` branches).
					WriteFitResults(&bestKalmanFit, fNTuple_fit5c_best);

				/// <li> If there is a fit result, **write** the MC truth topology for this event. Also increment `fCutFlow_NFitOK` counter if fit worked.
					if(bestKalmanFit.HasResults()) {
							std::cout << "  Result Kalman fit for (run, event) = "
								<< fEventHeader->runNumber() << ", "
								<< fEventHeader->eventNumber() << "):" << std::endl
								<< "    chi2   = " << bestKalmanFit.fChiSquared << std::endl
								<< "    m_D0   = " << bestKalmanFit.fM_D0       << std::endl
								<< "    m_phi  = " << bestKalmanFit.fM_phi      << std::endl
								<< "    p_D0   = " << bestKalmanFit.fP_D0       << std::endl
								<< "    p_phi  = " << bestKalmanFit.fP_phi      << std::endl;
							++fCutFlow_NFitOK;
							CreateMCTruthCollection();
							fNTuple_topology.GetItem<double>("chi2") = bestKalmanFit.fChiSquared;
							fNTuple_topology.GetItem<double>("mD0")  = bestKalmanFit.fM_D0;
							fNTuple_topology.GetItem<double>("mphi") = bestKalmanFit.fM_phi;
							fNTuple_topology.GetItem<double>("pD0")  = bestKalmanFit.fP_D0;
							fNTuple_topology.GetItem<double>("pphi") = bestKalmanFit.fP_phi;
							WriteMCTruthForTopoAna(fNTuple_topology);

						/// Set counters for mass cut flows
							bool mD0_3sig_ok  = !fCut_mD0_3sig .FailsCut(bestKalmanFit.fM_D0);
							bool mD0_ok       = !fCut_mD0      .FailsCut(bestKalmanFit.fM_D0);
							bool mphi_3sig_ok = !fCut_mphi_3sig.FailsCut(bestKalmanFit.fM_phi);
							bool mphi_ok      = !fCut_mphi     .FailsCut(bestKalmanFit.fM_phi);
							if(mphi_ok      && mD0_ok     ) ++fCutFlow_mD0_mphi;
							if(mphi_3sig_ok && mD0_3sig_ok) ++fCutFlow_mD0_mphi_3sig;
					}
			}
			/// </ol>


		/// </ol>
		return StatusCode::SUCCESS;
	}


	/// Currently does nothing.
	/// See `TrackSelector::finalize` for what else is done when finalising.
	StatusCode D0phi_KpipiKK::finalize_rest()
	{ PrintFunctionName("D0phi_KpipiKK", __func__);
		return StatusCode::SUCCESS;
	}



// * ============================== * //
// * ------- NTUPLE METHODS ------- * //
// * ============================== * //


	/// This function encapsulates the `addItem` procedure for the fit branches.
	void D0phi_KpipiKK::AddNTupleItems_Fit(NTupleContainer &tuple)
	{
		if(!tuple.DoWrite()) return;
		tuple.AddItem<double>("mD0");   /// * `"mD0"`:   Invariant mass for \f$K^- \pi^+\f$ (\f$D^0\f$).
		tuple.AddItem<double>("mphi");  /// * `"mphi"`:  Invariant mass for \f$K^+ K^+  \f$ (\f$\phi\f$).
		tuple.AddItem<double>("mJpsi"); /// * `"mJpsi"`: Invariant mass for \f$D^0 \phi \f$ (\f$J/\psi\f$).
		tuple.AddItem<double>("pD0");   /// * `"pD0"`:   3-momentum mass for the combination \f$K^- \pi^+\f$ (\f$D^0\f$ candidate).
		tuple.AddItem<double>("pphi");  /// * `"pphi"`:  3-momentum mass for the combination \f$K^+ K^+  \f$ (\f$\phi\f$ candidate).
		tuple.AddItem<double>("chisq"); /// * `"chisq"`: \f$\chi^2\f$ of the Kalman kinematic fit.
	}


	/// Specification of `TrackSelector::CreateMCTruthSelection`.
	/// Create selection of MC truth particles by looping over the collection of MC particles created by `TrackSelector::CreateMCTruthCollection()`.
	void D0phi_KpipiKK::CreateMCTruthSelection()
	{
		/// -# @b Abort if input file is not from a Monte Carlo simulation (that is, if the run number is not negative).
			if(fEventHeader->runNumber()>=0) return;
		/// -# @b Abort if `"write_fit_mc"`, has been set to `false`.
			if(!fNTuple_fit_mc.DoWrite()) return;
		/// -# Clear MC truth particle selections.
			fMcKaonNeg.clear();
			fMcKaonPos.clear();
			fMcPionPos.clear();
		/// -# Loop over `fMcParticles` collection of MC truth particles and fill the selections.
			std::vector<Event::McParticle*>::iterator it;
			for(it = fMcParticles.begin(); it != fMcParticles.end(); ++it) {
				switch((*it)->particleProperty()) {
					case -321 : fMcKaonNeg.push_back(*it); break;
					case  321 : fMcKaonPos.push_back(*it); break;
					case  211 : fMcPionPos.push_back(*it); break;
					default : fLog << MSG::DEBUG << "No switch case defined for McParticle " << (*it)->particleProperty() << endmsg;
				}
			}
	}


	/// Specification of what should be written to the fit `NTuple`.
	/// This function is called in `TrackSelector::WriteFitResults`.
	void D0phi_KpipiKK::SetFitNTuple(KKFitResult *fitresults, NTupleContainer &tuple)
	{
		/// -# Convert to the derived object of `KKFitResult` designed for this package. @remark This cast is required and cannot be solved using virtual methods, because of the specific structure of each analysis.
			KKFitResult_D0phi_KpipiKK* fit = dynamic_cast<KKFitResult_D0phi_KpipiKK*>(fitresults);

		/// -# @warning Terminate if cast failed.
			if(!fit) {
				std::cout << "FATAL ERROR: Dynamic cast failed" << std::endl;
				std::terminate();
			}

		/// -# Set the `NTuple::Item`s.
			tuple.GetItem<double>("mpi0")  = fit->fM_pi0;
			tuple.GetItem<double>("mD0")   = fit->fM_D0;
			tuple.GetItem<double>("mJpsi") = fit->fM_Jpsi;
			tuple.GetItem<double>("mphi")  = fit->fM_phi;
			tuple.GetItem<double>("pD0")   = fit->fP_D0;
			tuple.GetItem<double>("pphi")  = fit->fP_phi;
			tuple.GetItem<double>("chisq") = fit->fChiSquared;
	}