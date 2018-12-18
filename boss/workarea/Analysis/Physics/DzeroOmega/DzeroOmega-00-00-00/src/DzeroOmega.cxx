// * ========================= * //
// * ------- LIBRARIES ------- * //
// * ========================= * //
	#include "CLHEP/Geometry/Point3D.h"
	#include "CLHEP/Vector/LorentzVector.h"
	#include "CLHEP/Vector/ThreeVector.h"
	#include "CLHEP/Vector/TwoVector.h"
	#include "DstEvent/TofHitStatus.h"
	#include "DzeroOmega/DzeroOmega.h"
	#include "EventModel/Event.h"
	#include "EventModel/EventHeader.h"
	#include "EventModel/EventModel.h"
	#include "EvtRecEvent/EvtRecEvent.h"
	#include "EvtRecEvent/EvtRecTrack.h"
	#include "GaudiKernel/AlgFactory.h"
	#include "GaudiKernel/Bootstrap.h"
	#include "GaudiKernel/IDataProviderSvc.h"
	#include "GaudiKernel/IHistogramSvc.h"
	#include "GaudiKernel/INTupleSvc.h"
	#include "GaudiKernel/ISvcLocator.h"
	#include "GaudiKernel/ISvcLocator.h"
	#include "GaudiKernel/MsgStream.h"
	#include "GaudiKernel/NTuple.h"
	#include "GaudiKernel/PropertyMgr.h"
	#include "GaudiKernel/SmartDataPtr.h"
	#include "NTupleMap/NTupleMap.h"
	#include "ParticleID/ParticleID.h"
	#include "TMath.h"
	#include "TString.h"
	#include "VertexFit/Helix.h"
	#include "VertexFit/IVertexDbSvc.h"
	#include "VertexFit/KalmanKinematicFit.h"
	#include "VertexFit/VertexFit.h"
	#include <vector>
	// #include "VertexFit/KinematicFit.h"
	#ifndef ENABLE_BACKWARDS_COMPATIBILITY
		typedef HepGeom::Point3D<double> HepPoint3D;
	#endif
	using CLHEP::Hep2Vector;
	using CLHEP::Hep3Vector;
	using CLHEP::HepLorentzVector;



// * ==================================== * //
// * ------- GLOBALS AND TYPEDEFS ------- * //
// * ==================================== * //

	// * Constants * //
		const double mpi0  = 0.13957; // mass of pi0
		const double mrho0 = 0.770;   // mass of rho0
		const double xmass[5] = {
			0.000511, // electron
			0.105658, // muon
			0.139570, // charged pion
			0.493677, // charged kaon
			0.938272  // proton
		};
		const double velc_cm = 29.9792458; // tof_path unit in cm.
		const double velc_mm = 299.792458; // tof path unit in mm
		const double Ecms = 3.097;// center-of-mass energy


	// * Typedefs * //
		typedef std::vector<int> Vint;
		typedef std::vector<HepLorentzVector> Vp4;


	// * Counters for generating cut flow * //
		int Ncut0; // counter for all events (no cuts)
		int Ncut1; // vertex cuts
		int Ncut2; // number of photons < 2
		int Ncut3; // pass PID
		int Ncut4; // pass fit4c
		int Ncut5; // pass fit5c
		int Ncut6; // "geff" branch



// * =========================== * //
// * ------- CONSTRUCTOR ------- * //
// * =========================== * //
/**
 * @brief Constructor for the `DzeroOmega` algorithm.
 * @details Here, you should declare properties: give them a name, assign a parameter (data member of `DzeroOmega`), and if required a documentation string. Note that you should define the paramters themselves in the header (DzeroOmega/DzeroOmega.h) and that you should assign the values in `share/jopOptions_DzeroOmega.txt`.
 */
// Algorithms should inherit from Gaudi's `Algorithm` class. See https://dayabay.bnl.gov/dox/GaudiKernel/html/classAlgorithm.html.
DzeroOmega::DzeroOmega(const std::string& name, ISvcLocator* pSvcLocator) :
	Algorithm(name, pSvcLocator) {

	// * Define r0, z0 cut for charged tracks *
	declareProperty("Vr0cut",    fVr0cut);
	declareProperty("Vz0cut",    fVz0cut);
	declareProperty("Vrvz0cut",  fRvz0cut);
	declareProperty("Vrvxy0cut", fRvxy0cut);

	// * Define energy, dphi, dthe cuts for fake gamma's *
	declareProperty("EnergyThreshold", fEnergyThreshold);
	declareProperty("GammaPhiCut",     fGammaPhiCut);
	declareProperty("GammaThetaCut",   fGammaThetaCut);
	declareProperty("GammaAngleCut",   fGammaAngleCut);

	// * Define invariant mass window cut *
	declareProperty("dM_rho0", fDeltaMrho0 = .150);

	// * Whether to test the success of the 4- and 5-constraint fits *
	declareProperty("MaxChiSq", fMaxChiSq); // chisq for both fits should be less

}



// * ========================== * //
// * ------- INITIALIZE ------- * //
// * ========================== * //
/**
 * @brief   (Inherited) `initialize` step of `Algorithm`. This function is called only once in the beginning.
 * @details Define and load NTuples here.
 */
StatusCode DzeroOmega::initialize() {

	MsgStream log(msgSvc(), name());
	log << MSG::INFO << "In initialize():" << endmsg;

	// * Book NTuple: Multiplicities * //
		NTuplePtr nt = BookNTuple("mult");
		if(!nt) return StatusCode::FAILURE;
		nt->addItem("Ntotal",   fNtotal);
		nt->addItem("Nneutral", fNneutral);
		nt->addItem("Ncharge",  fNcharge);
		nt->addItem("Nmdc",     fNmdc);
		nt->addItem("Ngood",    fNgood);

	// * Book NTuple: Vertex position * //
		nt = BookNTuple("vxyz");
		if(!nt) return StatusCode::FAILURE;
		nt->addItem("vx0",    fVx0);
		nt->addItem("vy0",    fVy0);
		nt->addItem("vz0",    fVz0);
		nt->addItem("vr0",    fVr0);
		nt->addItem("rvxy0",  fRvxy0);
		nt->addItem("rvz0",   fRvz0);
		nt->addItem("rvphi0", fRvphi0);

	// * Book NTuple: Photon kinematics * //
		nt = BookNTuple("photon");
		if(!nt) return StatusCode::FAILURE;
		nt->addItem("dthe", fDeltaTheta);
		nt->addItem("dphi", fDeltaPhi);
		nt->addItem("dang", fDeltaAngle);
		nt->addItem("eraw", fEraw);

	// * Book NTuple: Energy branch * //
		nt = BookNTuple("etot");
		if(!nt) return StatusCode::FAILURE;
		nt->addItem("m2gg", fMtoGG);
		nt->addItem("etot", fEtot);

	// * Book NTuple: Neutral pion fit branch * //
		nt = BookNTuple("fit4c");
		if(!nt) return StatusCode::FAILURE;
		nt->addItem("mpi0", fMpi0);
		nt->addItem("chi2", fChi1);

	// * Book NTuple: Rho fit branch * //
		nt = BookNTuple("fit6c");
		if(!nt) return StatusCode::FAILURE;
		nt->addItem("chi2",  fChi2);
		nt->addItem("mrho0", fMrho0);
		nt->addItem("mrhop", fMrhop);
		nt->addItem("mrhom", fMrhom);

	// * Book NTuple: Photon detection efficiences * //
		nt = BookNTuple("geff");
		if(!nt) return StatusCode::FAILURE;
		nt->addItem("fcos", fFcos);
		nt->addItem("elow", fElow);

	// * Book NTuple: Import dE/dx PID branch * //
		nt = BookNTuple("dedx");
		if(!nt) return StatusCode::FAILURE;
		nt->addItem("ptrk",   fPtrack);
		nt->addItem("chie",   fChi2e);
		nt->addItem("chimu",  fChi2mu);
		nt->addItem("chipi",  fChi2pi);
		nt->addItem("chik",   fChi2k);
		nt->addItem("chip",   fChi2p);
		nt->addItem("probPH", fProbPH);
		nt->addItem("normPH", fNormPH);
		nt->addItem("ghit",   fGhit);
		nt->addItem("thit",   fThit);

	// * Book NTuple: ToF endcap branch * //
		nt = BookNTuple("tofe");
		if(!nt) return StatusCode::FAILURE;
		nt->addItem("ptrk", fPtotTofEC);
		nt->addItem("path", fPathTofEC);
		nt->addItem("tof",  fTofEC);
		nt->addItem("cntr", fCntrTofEC);
		nt->addItem("ph",   fPhTofEC);
		nt->addItem("rhit", fRhitTofEC);
		nt->addItem("qual", fQualTofEC);
		nt->addItem("te",   fElectronTofEC);
		nt->addItem("tmu",  fMuonTofEC);
		nt->addItem("tpi",  fProtoniTofEC);
		nt->addItem("tk",   fKaonTofEC);
		nt->addItem("tp",   fProtonTofEC);

	// * Book NTuple: TToF inner barrel branch * //
		nt = BookNTuple("tof1");
		if(!nt) return StatusCode::FAILURE;
		nt->addItem("ptrk", fPtotTofIB);
		nt->addItem("path", fPathTofIB);
		nt->addItem("tof",  fTofIB);
		nt->addItem("cntr", fCntrTofIB);
		nt->addItem("ph",   fPhTofIB);
		nt->addItem("zhit", fZhitTofIB);
		nt->addItem("qual", fQualTofIB);
		nt->addItem("te",   fElectronTofIB);
		nt->addItem("tmu",  fMuonTofIB);
		nt->addItem("tpi",  fProtoniTofIB);
		nt->addItem("tk",   fKaonTofIB);
		nt->addItem("tp",   fProtonTofIB);

	// * Book NTuple: check ToF outer barrel branch * //
		nt = BookNTuple("tof1");
		if(!nt) return StatusCode::FAILURE;
		nt->addItem("ptrk", fPtotTofOB);
		nt->addItem("path", fPathTofOB);
		nt->addItem("tof",  fTofOB);
		nt->addItem("cntr", fCntrTofOB);
		nt->addItem("ph",   fPhTofOB);
		nt->addItem("zhit", fZhitTofOB);
		nt->addItem("qual", fQualTofOB);
		nt->addItem("te",   fElectronTofOB);
		nt->addItem("tmu",  fMuonTofOB);
		nt->addItem("tpi",  fProtoniTofOB);
		nt->addItem("tk",   fKaonTofOB);
		nt->addItem("tp",   fProtonTofOB);

	// * Book NTuple: Track PID information * //
		nt = BookNTuple("pid");
		if(!nt) return StatusCode::FAILURE;
		nt->addItem("ptrk", fPtrackPID);
		nt->addItem("cost", fCostPID);
		nt->addItem("dedx", fDedxPID);
		nt->addItem("tof1", fTof1PID);
		nt->addItem("tof2", fTof2PID);
		nt->addItem("prob", fProbPID);

	log << MSG::INFO << "Successfully returned from initialize()" << endmsg;
	return StatusCode::SUCCESS;
}



// * ========================= * //
// * -------- EXECUTE -------- * //
// * ========================= * //
/**
 * @brief Inherited `execute` method of the `Algorithm`. This function is called *for each event*.
 */
StatusCode DzeroOmega::execute() {

	MsgStream log(msgSvc(), name());
	log << MSG::INFO << "In execute():" << endreq;

	// * Load next event from DST file * //
	// NOTE: Ncut0 -- no cut yet

		// * Load DST file info *
		SmartDataPtr<Event::EventHeader> eventHeader(eventSvc(), "/Event/EventHeader");
		int runNo = eventHeader->runNumber();
		int evtNo = eventHeader->eventNumber();
		log << MSG::DEBUG << "run, evtnum = "
		    << runNo << " , "
		    << evtNo << endreq;
		cout << "event number: " << evtNo << endl;
		Ncut0++; // counter for all events

		// * Load event information and track collection *
			/*
			http://bes3.to.infn.it/Boss/7.0.2/html/namespaceEventModel_1_1EvtRec.html (namespace)
			http://bes3.to.infn.it/Boss/7.0.2/html/classEvtRecEvent.html (class)
			http://bes3.to.infn.it/Boss/7.0.2/html/EvtRecTrack_8h.html (typedef EvtRecTrackCol)
			*/
		SmartDataPtr<EvtRecEvent>    evtRecEvent (eventSvc(), EventModel::EvtRec::EvtRecEvent);
		SmartDataPtr<EvtRecTrackCol> evtRecTrkCol(eventSvc(), EventModel::EvtRec::EvtRecTrackCol);

		// * Log number of events *
		log << MSG::DEBUG << "Ncharged, Nneutral, Ntotal = "
		    << evtRecEvent->totalCharged() << ", "
		    << evtRecEvent->totalNeutral() << ", "
		    << evtRecEvent->totalTracks() << endreq;


	// * Set vertex origin * //
		Hep3Vector xorigin(0,0,0);
		IVertexDbSvc* vtxsvc;
		Gaudi::svcLocator()->service("VertexDbSvc", vtxsvc);
		if(vtxsvc->isVertexValid()){
			double* dbv = vtxsvc->PrimaryVertex();
			double* vv = vtxsvc->SigmaPrimaryVertex();
			// HepVector dbv = fReader.PrimaryVertex(runNo);
			// HepVector vv  = fReader.SigmaPrimaryVertex(runNo);
			xorigin.setX(dbv[0]);
			xorigin.setY(dbv[1]);
			xorigin.setZ(dbv[2]);
		}


	// * LOOP OVER CHARGED TRACKS: select 'good' charged tracks * //
	// NOTE: Ncut1 -- vertex cuts
		// The first part of the set of reconstructed tracks are the charged tracks
		fNmdc = 0;
			//!< Number of charged tracks as identified by the MDC.
		Vint iGood;
			//!< vector of integers that give the position of tracks in the `evtRecEvent` marked good.
		for(int i = 0; i < evtRecEvent->totalCharged(); ++i) {

			// * Get track: beginning of all tracks + event number
			EvtRecTrackIterator itTrk = evtRecTrkCol->begin() + i; 
			if(!(*itTrk)->isMdcTrackValid()) continue;

			// * Get track info from Main Drift Chamber
			RecMdcTrack *mdcTrk = (*itTrk)->mdcTrack();
			double pch  = mdcTrk->p();
			double x0   = mdcTrk->x();
			double y0   = mdcTrk->y();
			double z0   = mdcTrk->z();
			double phi0 = mdcTrk->helix(1);

			// * Get vertex origin
			double Rxy  =
				(x0 - xorigin.x()) * cos(phi0) +
				(y0 - xorigin.y()) * sin(phi0);

			// * Get radii of vertex
			HepVector a = mdcTrk->helix();
			HepSymMatrix Ea = mdcTrk->err();
			HepPoint3D point0(0., 0., 0.); // the initial point for MDC recosntruction
			HepPoint3D IP(xorigin[0], xorigin[1], xorigin[2]);
			VFHelix helixip(point0, a, Ea);
			helixip.pivot(IP);
			HepVector vecipa = helixip.a();
			double Rvxy0  = fabs(vecipa[0]); // nearest distance to IP in xy plane
			double Rvz0   = vecipa[3];       // nearest distance to IP in z direction
			double Rvphi0 = vecipa[1];

			// * WRITE primary vertex position info ("vxyz" branch) *
			fVx0    = x0;     // primary x-vertex as determined by MDC
			fVy0    = y0;     // primary y-vertex as determined by MDC
			fVz0    = z0;     // primary z-vertex as determined by MDC
			fVr0    = Rxy;    // distance from origin in xy-plane
			fRvxy0  = Rvxy0;  // nearest distance to IP in xy plane
			fRvz0   = Rvz0;   // nearest distance to IP in z direction
			fRvphi0 = Rvphi0; // angle in the xy-plane (?)
			fNTupleMap["vxyz"]->write();

			// * Apply vertex cuts *
			if(fabs(z0)    >= fVz0cut)   continue;
			if(fabs(Rxy)   >= fVr0cut)   continue;
			if(fabs(Rvz0)  >= fRvz0cut)  continue;
			if(fabs(Rvxy0) >= fRvxy0cut) continue;

			// * Add charged track to vector *
			iGood.push_back(i);
			fNmdc += mdcTrk->charge();

		}

	// * Write multiplicities ("mult" branch) * //
		log << MSG::DEBUG << "ngood, totcharge = " << iGood.size() << " , " << fNmdc << endreq;
		if((iGood.size() != 2) || (fNmdc!=0)) return StatusCode::SUCCESS;
		fNtotal   = evtRecEvent->totalTracks();
		fNneutral = evtRecEvent->totalNeutral();
		fNcharge  = evtRecEvent->totalCharged();
		fNgood    = iGood.size();
		fNTupleMap["mult"]->write(); // "mult" branch
		Ncut1++; // vertex cuts


	// * LOOP OVER NEUTRAL TRACKS: select photons * //
	// NOTE: Ncut2 -- number of good photons has to be 2 at least
		// The second part of the set of reconstructed events consists of the neutral tracks, that is, the photons detected by the EMC (by clustering EMC crystal energies). Each neutral track is paired with each charged track and if their angle is smaller than a certain value (here, 200), the photon track is stored as 'good photon' (added to `iGam`).
		Vint iGam;
		for(int i = evtRecEvent->totalCharged(); i < evtRecEvent->totalTracks(); ++i) {

			// * Get track
			EvtRecTrackIterator itTrk = evtRecTrkCol->begin() + i;
			if(!(*itTrk)->isEmcShowerValid()) continue;
			RecEmcShower *emcTrk = (*itTrk)->emcShower();
			Hep3Vector emcpos(emcTrk->x(), emcTrk->y(), emcTrk->z());

			// * Find the theta, phi, and angle difference with nearest charged track
			double dthe = 200.; // start value for difference in theta
			double dphi = 200.; // start value for difference in phi
			double dang = 200.; // start value for difference in angle (?)
			for(int j = 0; j < evtRecEvent->totalCharged(); j++) {
				EvtRecTrackIterator jtTrk = evtRecTrkCol->begin() + j;
				if(!(*jtTrk)->isExtTrackValid()) continue;
				RecExtTrack *extTrk = (*jtTrk)->extTrack();
				if(extTrk->emcVolumeNumber() == -1) continue;
				Hep3Vector extpos = extTrk->emcPosition();
				// double ctht = extpos.cosTheta(emcpos);
				double angd = extpos.angle(emcpos);
				double thed = extpos.theta() - emcpos.theta();
				double phid = extpos.deltaPhi(emcpos);
				thed = fmod(thed + CLHEP::twopi + CLHEP::twopi + pi, CLHEP::twopi) - CLHEP::pi;
				phid = fmod(phid + CLHEP::twopi + CLHEP::twopi + pi, CLHEP::twopi) - CLHEP::pi;
				if(angd < dang){
					dang = angd;
					dthe = thed;
					dphi = phid;
				}
			}

			// * Apply angle cut
			if(dang>=200) continue;
			double eraw = emcTrk->energy();
			dthe = dthe * 180 / (CLHEP::pi);
			dphi = dphi * 180 / (CLHEP::pi);
			dang = dang * 180 / (CLHEP::pi);

			// * WRITE photon info ("photon" branch)
			fDeltaTheta = dthe; // theta difference with nearest charged track (degrees)
			fDeltaPhi = dphi; // phi difference with nearest charged track (degrees)
			fDeltaAngle = dang; // angle difference with nearest charged track
			fEraw = eraw; // energy of the photon
			fNTupleMap["photon"]->write();

			// * Apply photon cuts
			if(eraw < fEnergyThreshold) continue;
			if((fabs(dthe) < fGammaThetaCut) && (fabs(dphi) < fGammaPhiCut) ) continue;
			if(fabs(dang) < fGammaAngleCut) continue;

			// * Add photon track to vector
			iGam.push_back(i);

		}

		// * Finish Good Photon Selection *
		int nGam = iGam.size();
		log << MSG::DEBUG << "Number of good photons: " << nGam << " , " << evtRecEvent->totalNeutral() << endreq;
		if(nGam<2) {
			return StatusCode::SUCCESS;
		}
		Ncut2++; // number of photons < 2


	// * Check charged track dEdx PID information * //
		for(int i = 0; i < iGood.size(); ++i) {

			// * Get track *
			EvtRecTrackIterator  itTrk = evtRecTrkCol->begin() + iGood[i];
			if(!(*itTrk)->isMdcTrackValid()) continue;
			if(!(*itTrk)->isMdcDedxValid())continue;
			RecMdcTrack* mdcTrk = (*itTrk)->mdcTrack();
			RecMdcDedx* dedxTrk = (*itTrk)->mdcDedx();

			// * WRITE energy loss PID info ("dedx" branch) *
			fPtrack   = mdcTrk->p();             // momentum of the track
			fChi2e   = dedxTrk->chiE();         // chi2 in case of electron
			fChi2mu  = dedxTrk->chiMu();        // chi2 in case of muon
			fChi2pi  = dedxTrk->chiPi();        // chi2 in case of pion
			fChi2k   = dedxTrk->chiK();         // chi2 in case of kaon
			fChi2p   = dedxTrk->chiP();         // chi2 in case of proton
			fProbPH = dedxTrk->probPH();       // most probable pulse height from truncated mean
			fNormPH = dedxTrk->normPH();       // normalized pulse height
			fGhit   = dedxTrk->numGoodHits();  // number of good hits
			fThit   = dedxTrk->numTotalHits(); // total number of hits
			fNTupleMap["dedx"]->write();
		}


	// * Check charged track ToF PID information * //
		for(int i = 0; i < iGood.size(); ++i) {
			EvtRecTrackIterator  itTrk = evtRecTrkCol->begin() + iGood[i];
			if(!(*itTrk)->isMdcTrackValid()) continue;
			if(!(*itTrk)->isTofTrackValid()) continue;

			RecMdcTrack * mdcTrk = (*itTrk)->mdcTrack();
			SmartRefVector<RecTofTrack> tofTrkCol = (*itTrk)->tofTrack();

			double ptrk = mdcTrk->p();

			SmartRefVector<RecTofTrack>::iterator iter_tof = tofTrkCol.begin();
			for(;iter_tof != tofTrkCol.end(); ++iter_tof) {
				TofHitStatus hitStatus;
				hitStatus.setStatus((*iter_tof)->status());
				if(!hitStatus.is_counter()) continue;

				// * If end cap ToF detector: *
				if(!hitStatus.is_barrel()) { // is not barrel
					if(hitStatus.layer() != 0) continue; // abort if not end cap

					// * Get ToF info *
					double path = (*iter_tof)->path();       // distance of flight
					double tof  = (*iter_tof)->tof();        // time of flight
					double ph   = (*iter_tof)->ph();         // ToF pulse height
					double rhit = (*iter_tof)->zrhit();      // Track extrapolate Z or R Hit position
					double qual = 0.+(*iter_tof)->quality(); // data quality of reconstruction
					double cntr = 0.+(*iter_tof)->tofID();   // ToF counter ID

					// * Get ToF for each particle hypothesis *
					double texp[5];
					for(int j = 0; j < 5; j++) {
						double gb = ptrk/xmass[j]; // v = p/m (non-relativistic velocity)
						double beta = gb/sqrt(1+gb*gb);
						texp[j] = 10 * path /beta/velc_mm; // hypothesis ToF
					}

					// * WRITE ToF end cap info ("tofe" branch) *
					fPtotTofEC = ptrk;          // momentum of the track as reconstructed by MDC
					fPathTofEC = path;          // path length
					fTofEC  = tof;           // time of flight
					fCntrTofEC = cntr;          // ToF counter ID
					fPhTofEC   = ph;            // ToF pulse height
					fRhitTofEC = rhit;          // track extrapolate Z or R Hit position
					fQualTofEC = qual;          // data quality of reconstruction
					fElectronTofEC   = tof - texp[0]; // difference with ToF in electron hypothesis
					fMuonTofEC  = tof - texp[1]; // difference with ToF in muon hypothesis
					fProtoniTofEC  = tof - texp[2]; // difference with ToF in charged pion hypothesis
					fKaonTofEC   = tof - texp[3]; // difference with ToF in charged kaon hypothesis
					fProtonTofEC   = tof - texp[4]; // difference with ToF in proton hypothesis
					fNTupleMap["tofe"]->write();
				}

				// * If barrel ToF detector: *
				else {

					// * Inner barrel ToF detector
					if(hitStatus.layer() == 1) {
						double path = (*iter_tof)->path();       // distance of flight
						double tof  = (*iter_tof)->tof();        // time of flight
						double ph   = (*iter_tof)->ph();         // ToF pulse height
						double rhit = (*iter_tof)->zrhit();      // Track extrapolate Z or R Hit position
						double qual = 0.+(*iter_tof)->quality(); // data quality of reconstruction
						double cntr = 0.+(*iter_tof)->tofID();   // ToF counter ID
						double texp[5];
						for(int j = 0; j < 5; j++) {
							double gb = ptrk/xmass[j]; // v = p/m (non-relativistic velocity)
							double beta = gb/sqrt(1+gb*gb);
							texp[j] = 10 * path /beta/velc_mm; // hypothesis ToF
						}

						// * WRITE ToF inner barrel info ("tof1" branch) *
						fPtotTofIB = ptrk;          // momentum of the track as reconstructed by MDC
						fPathTofIB = path;          // path length
						fTofIB  = tof;           // time of flight
						fCntrTofIB = cntr;          // ToF counter ID
						fPhTofIB   = ph;            // ToF pulse height
						fZhitTofIB = rhit;          // track extrapolate Z or R Hit position
						fQualTofIB = qual;          // data quality of reconstruction
						fElectronTofIB   = tof - texp[0]; // difference with ToF in electron hypothesis
						fMuonTofIB  = tof - texp[1]; // difference with ToF in muon hypothesis
						fProtoniTofIB  = tof - texp[2]; // difference with ToF in charged pion hypothesis
						fKaonTofIB   = tof - texp[3]; // difference with ToF in charged kaon hypothesis
						fProtonTofIB   = tof - texp[4]; // difference with ToF in proton hypothesis
						TupleTofIB->write(); // "tof1" branch
					}

					// * Outer barrel ToF detector
					if(hitStatus.layer() == 2) {
						double path = (*iter_tof)->path();       // distance of flight
						double tof  = (*iter_tof)->tof();        // ToF pulse height
						double ph   = (*iter_tof)->ph();         // ToF pulse height
						double rhit = (*iter_tof)->zrhit();      // track extrapolate Z or R Hit position
						double qual = 0.+(*iter_tof)->quality(); // data quality of reconstruction
						double cntr = 0.+(*iter_tof)->tofID();   // ToF counter ID
						double texp[5];
						for(int j = 0; j < 5; j++) {
							double gb = ptrk/xmass[j]; // v = p/m (non-relativistic velocity)
							double beta = gb/sqrt(1+gb*gb);
							texp[j] = 10 * path/beta/velc_mm; // hypothesis ToF
						}

						// * WRITE ToF outer barrel info ("tof2" branch) *
						fPtotTofOB = ptrk;          // momentum of the track as reconstructed by MDC
						fPathTofOB = path;          // path length
						fTofOB  = tof;           // time of flight
						fCntrTofOB = cntr;          // ToF counter ID
						fPhTofOB   = ph;            // ToF pulse height
						fZhitTofOB = rhit;          // track extrapolate Z or R Hit position
						fQualTofOB = qual;          // data quality of reconstruction
						fElectronTofOB   = tof - texp[0]; // difference with ToF in electron hypothesis
						fMuonTofOB  = tof - texp[1]; // difference with ToF in muon hypothesis
						fProtoniTofOB  = tof - texp[2]; // difference with ToF in charged pion hypothesis
						fKaonTofOB   = tof - texp[3]; // difference with ToF in charged kaon hypothesis
						fProtonTofOB   = tof - texp[4]; // difference with ToF in proton hypothesis
						TupleTofOB->write(); // "tof2" branch
					}

				}
			}
		} // loop all charged track


	// * Get 4-momentum for each photon * //
		Vp4 pGam;
		for(int i = 0; i < nGam; ++i) {
			EvtRecTrackIterator itTrk = evtRecTrkCol->begin() + iGam[i];
			RecEmcShower* emcTrk = (*itTrk)->emcShower();
			double eraw = emcTrk->energy();
			double phi = emcTrk->phi();
			double the = emcTrk->theta();
			HepLorentzVector ptrk;
			ptrk.setPx(eraw*sin(the)*cos(phi));
			ptrk.setPy(eraw*sin(the)*sin(phi));
			ptrk.setPz(eraw*cos(the));
			ptrk.setE(eraw);
			// ptrk = ptrk.boost(-0.011, 0, 0); // boost to cms
			pGam.push_back(ptrk);
		}
		// cout << "before pid" << endl;


	// * Get 4-momentum for each charged track * //
		Vint ipip, ipim; // vector of number of good tracks
		Vp4  ppip, ppim; // vector of momenta
		ParticleID *pid = ParticleID::instance();
		for(int i = 0; i < iGood.size(); ++i) {
			EvtRecTrackIterator itTrk = evtRecTrkCol->begin() + iGood[i];
			// if(pid) delete pid;
			pid->init();
			pid->setMethod(pid->methodProbability());
			// pid->setMethod(pid->methodLikelihood()); // for Likelihood Method

			pid->setChiMinCut(4);
			pid->setRecTrack(*itTrk);
			pid->usePidSys(pid->useDedx() | pid->useTof1() | pid->useTof2() | pid->useTofE()); // use PID sub-system
			pid->identify(pid->onlyPion() | pid->onlyKaon()); // seperater Pion/Kaon
			// pid->identify(pid->onlyPion());
			// pid->identify(pid->onlyKaon());
			pid->calculate();
			if(!(pid->IsPidInfoValid())) continue;
			RecMdcTrack* mdcTrk = (*itTrk)->mdcTrack();

			// * WRITE particle identification info ("pid" branch) *
			fPtrackPID = mdcTrk->p();          // momentum of the track
			fCostPID = cos(mdcTrk->theta()); // theta angle of the track
			fDedxPID = pid->chiDedx(2);      // Chi squared of the dedx of the track
			fTof1PID = pid->chiTof1(2);      // Chi squared of the inner barrel ToF of the track
			fTof2PID = pid->chiTof2(2);      // Chi squared of the outer barrel ToF of the track
			fProbPID = pid->probPion();      // probability that it is a pion
			fNTupleMap["pid"]->write();

			if(pid->probPion() < 0.001 || (pid->probPion() < pid->probKaon())) continue;
			if(pid->probPion() < 0.001) continue;
			if(pid->pdf(2)<pid->pdf(3)) continue; // for Likelihood Method (0=electron 1=muon 2=pion 3=kaon 4=proton)

			RecMdcKalTrack* mdcKalTrk = (*itTrk)->mdcKalTrack(); // After ParticleID, use RecMdcKalTrack substitute RecMdcTrack
			RecMdcKalTrack::setPidType(RecMdcKalTrack::pion); // PID can set to electron, muon, pion, kaon and proton;The default setting is pion

			if(mdcKalTrk->charge() >0) {
				ipip.push_back(iGood[i]);
				HepLorentzVector ptrk;
				ptrk.setPx(mdcKalTrk->px());
				ptrk.setPy(mdcKalTrk->py());
				ptrk.setPz(mdcKalTrk->pz());
				double p3 = ptrk.mag();
				ptrk.setE(sqrt(p3*p3+mpi0*mpi0));
				// ptrk = ptrk.boost(-0.011,0,0); // boost to cms
				ppip.push_back(ptrk);
			} else {
				ipim.push_back(iGood[i]);
				HepLorentzVector ptrk;
				ptrk.setPx(mdcKalTrk->px());
				ptrk.setPy(mdcKalTrk->py());
				ptrk.setPz(mdcKalTrk->pz());
				double p3 = ptrk.mag();
				ptrk.setE(sqrt(p3*p3+mpi0*mpi0));
				// ptrk = ptrk.boost(-0.011,0,0); // boost to cms
				ppim.push_back(ptrk);
			}
		}


	// NOTE: Ncut3 -- ipip.size() * ipim.size() cannot be 1
		int npip = ipip.size();
		int npim = ipim.size();
		if(npip*npim != 1) return SUCCESS;
		Ncut3++; // ipip.size() * ipim.size() cannot be 1


	// * Loop over each gamma pair and store total energy * //
		HepLorentzVector pTot;
		for(int i = 0; i < nGam-1; ++i) {
			for(int j = i+1; j < nGam; j++) {
				HepLorentzVector p2g = pGam[i] + pGam[j];
				pTot = ppip[0] + ppim[0];
				pTot += p2g;

				// * WRITE total energy and pi^0 candidate inv. mass ("etot" branch) *
				fMtoGG = p2g.m();  // invariant mass of the two gammas
				fEtot = pTot.e(); // total energy of pi^+, pi^ and the two gammas
				fNTupleMap["etot"]->write();
			}
		}

		RecMdcKalTrack *pipTrk = (*(evtRecTrkCol->begin()+ipip[0]))->mdcKalTrack();
		RecMdcKalTrack *pimTrk = (*(evtRecTrkCol->begin()+ipim[0]))->mdcKalTrack();

		WTrackParameter wvpipTrk, wvpimTrk;
		wvpipTrk = WTrackParameter(mpi0, pipTrk->getZHelix(), pipTrk->getZError());
		wvpimTrk = WTrackParameter(mpi0, pimTrk->getZHelix(), pimTrk->getZError());


	// * Test vertex fit * //
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
		vtxfit->AddTrack(0, wvpipTrk);
		vtxfit->AddTrack(1, wvpimTrk);
		vtxfit->AddVertex(0, vxpar,0, 1);
		if(!vtxfit->Fit(0)) return SUCCESS;
		vtxfit->Swim(0);

		WTrackParameter wpip = vtxfit->wtrk(0);
		WTrackParameter wpim = vtxfit->wtrk(1);

		// KinematicFit * kmfit = KinematicFit::instance();
		KalmanKinematicFit * kkmfit = KalmanKinematicFit::instance();


	// * Apply Kalman 4-constrain kinematic fit * //
		HepLorentzVector ecms(0.034, 0, 0, Ecms);
		double chisq = 9999.;
		int ig1 = -1;
		int ig2 = -1;
		// * Run over all gamma pairs and find the pair with the best chi2
		for(int i = 0; i < nGam-1; ++i) {
			RecEmcShower *g1Trk = (*(evtRecTrkCol->begin()+iGam[i]))->emcShower();
			for(int j = i+1; j < nGam; j++) {
				RecEmcShower *g2Trk = (*(evtRecTrkCol->begin()+iGam[j]))->emcShower();
				kkmfit->init();
				kkmfit->AddTrack(0, wpip);       // 1c: pos. pion
				kkmfit->AddTrack(1, wpim);       // 2c: neg. pion
				kkmfit->AddTrack(2, 0.0, g1Trk); // 3c: first gamma track
				kkmfit->AddTrack(3, 0.0, g2Trk); // 4c: seconnd gamma track
				kkmfit->AddFourMomentum(0, ecms);
				if(kkmfit->Fit()) {
					double chi2 = kkmfit->chisq();
					if(chi2 < chisq) {
						chisq = chi2;
						ig1 = iGam[i];
						ig2 = iGam[j];
					}
				}
			}
		}


	// NOTE: Ncut4 -- fit4c passed and ChiSq less than fMaxChiSq
		if(chisq < fMaxChiSq) {
			RecEmcShower *g1Trk = (*(evtRecTrkCol->begin()+ig1))->emcShower();
			RecEmcShower *g2Trk = (*(evtRecTrkCol->begin()+ig2))->emcShower();
			kkmfit->init();
			kkmfit->AddTrack(0, wpip);      // 1c: pos. pion
			kkmfit->AddTrack(1, wpim);      // 2c: neg. pion
			kkmfit->AddTrack(2, 0., g1Trk); // 3c: first gamma track
			kkmfit->AddTrack(3, 0., g2Trk); // 4c: second gamma track
			kkmfit->AddFourMomentum(0, ecms);
			if(kkmfit->Fit()) {
				HepLorentzVector ppi0 = kkmfit->pfit(2) + kkmfit->pfit(3);

				// * WRITE pi^0 information from EMCal ("fit4c" branch) *
				fMpi0 = ppi0.m();        // invariant pi0 mass according to Kalman kinematic fit
				fChi1 = kkmfit->chisq(); // chi square of the Kalman kinematic fit
				fNTupleMap["fit4c"]->write();
				Ncut4++; // ChiSq has to be less than 200 and fit4c has to be passed
			}
		}


	// * Apply Kalman kinematic fit * //
	// NOTE: Ncut5 -- Kalman kinematic fit 5c is successful
	// NOTE: Ncut6 -- J/psi -> rho0 pi0 (cut on invariant mass window)
		HepLorentzVector ecms(0.034, 0, 0, Ecms);
		double chisq = 9999.;
		int ig1 = -1;
		int ig2 = -1;
		// * Find the best combination over all possible pi+ pi- gamma gamma pair
		for(int i = 0; i < nGam-1; ++i) {
			RecEmcShower *g1Trk = (*(evtRecTrkCol->begin()+iGam[i]))->emcShower();
			for(int j = i+1; j < nGam; j++) {
				RecEmcShower *g2Trk = (*(evtRecTrkCol->begin()+iGam[j]))->emcShower();
				kkmfit->init();
				kkmfit->AddTrack(0, wpip);            // 1c: pos. pion
				kkmfit->AddTrack(1, wpim);            // 2c: neg. pion
				kkmfit->AddTrack(2, 0., g1Trk);       // 3c: first gamma track
				kkmfit->AddTrack(3, 0., g2Trk);       // 4c: second gamma track
				kkmfit->AddResonance(0, 0.135, 2, 3); // 5c: pi0 resonance
				kkmfit->AddFourMomentum(1, ecms);
				if(!kkmfit->Fit(0)) continue;
				if(!kkmfit->Fit(1)) continue;
				if(kkmfit->Fit()) {
					double chi2 = kkmfit->chisq();
					if(chi2 < chisq) {
						chisq = chi2;
						ig1 = iGam[i];
						ig2 = iGam[j];
					}
				}
			}
		}

		log << MSG::INFO << " chisq = " << chisq << endreq;

		if(chisq < fMaxChiSq) {
			RecEmcShower* g1Trk = (*(evtRecTrkCol->begin()+ig1))->emcShower();
			RecEmcShower* g2Trk = (*(evtRecTrkCol->begin()+ig2))->emcShower();
			kkmfit->init();
			kkmfit->AddTrack(0, wpip);            // 1c: pi^+ track
			kkmfit->AddTrack(1, wpim);            // 2c: pi^- track
			kkmfit->AddTrack(2, 0., g1Trk);       // 3c: first gamma track
			kkmfit->AddTrack(3, 0., g2Trk);       // 4c: second gamma track
			kkmfit->AddResonance(0, 0.135, 2, 3); // 5c: pi0 resonance
			kkmfit->AddFourMomentum(1, ecms);

			// * Kalman kinematic fit6c * //
			if(kkmfit->Fit()){
				HepLorentzVector ppi0  = kkmfit->pfit(2) + kkmfit->pfit(3); // inv. mass Gamma Gamma (pi^0)
				HepLorentzVector prho0 = kkmfit->pfit(0) + kkmfit->pfit(1); // inv. mass pi^+ pi^- (rho^0)
				HepLorentzVector prhop = ppi0 + kkmfit->pfit(0);            // inv. mass pi^0 pi^+ (rho^+)
				HepLorentzVector prhom = ppi0 + kkmfit->pfit(1);            // inv. mass pi^0 pi^- (rho^-)
				double eg1 = (kkmfit->pfit(2)).e();
				double eg2 = (kkmfit->pfit(3)).e();
				double fcos = abs(eg1-eg2)/ppi0.rho();

				// * WRITE inv. mass from EMCal info ("fit6c" branch) *
				fChi2 = kkmfit->chisq(); // chi squared of the Kalman kinematic fit
				fMrho0 = prho0.m();      // inv. mass pi^+ pi^- (rho^0)
				fMrhop = prhop.m();      // inv. mass pi^0 pi^+ (rho^+)
				fMrhom = prhom.m();      // inv. mass pi^0 pi^- (rho^-)
				fNTupleMap["fit6c"]->write();
				Ncut5++; // Kalman kinematic fit 5c is successful


				// * Measure the photon detection efficiences via J/psi -> rho0 pi0 * //
				if(fabs(prho0.m() - mrho0) < fDeltaMrho0) {
					if(fabs(fcos) < 0.99) {
						// * WRITE photon detection efficiency info ("geff" branch) *
						fFcos = (eg1-eg2) / ppi0.rho();  // E/p ratio for pi^0 candidate
						fElow = (eg1 < eg2) ? eg1 : eg2; // lowest energy of the two gammas
						fNTupleMap["geff"]->write();
						Ncut6++; // 0.620 < mass of rho0 < 0.920
					}
				} // rho0 cut
			}
		}


	return StatusCode::SUCCESS;
}



// * ========================== * //
// * -------- FINALIZE -------- * //
// * ========================== * //
/**
 * @brief Inherited `finalize` method of `Algorithm`. This function is only called once, after running over all events.
 * @details Prints the flow chart to the terminal, so make sure you save this output!
 */
StatusCode DzeroOmega::finalize() {

	MsgStream log(msgSvc(), name());
	log << MSG::INFO << "in finalize()" << endmsg;

	// * Print flow chart * //
		cout << "Resulting FLOW CHART:" << endl;
		cout << "  (0) Total number of events: " << Ncut0 << endl;
		cout << "  (1) Vertex cut              " << Ncut1 << endl;
		cout << "  (2) nGam>=2:                " << Ncut2 << endl;
		cout << "  (3) Pass PID:               " << Ncut3 << endl;
		cout << "  (4) Pass 4C Kalman fit:     " << Ncut4 << endl;
		cout << "  (5) Pass 5C Kalman fit:     " << Ncut5 << endl;
		cout << "  (6) J/psi -> rho0 pi0:      " << Ncut6 << endl;
		cout << endl ;

	log << MSG::INFO << "Successfully returned from finalize()" << endmsg;
	return StatusCode::SUCCESS;
}



// * ================================= * //
// * -------- PRIVATE METHODS -------- * //
// * ================================= * //
/**
 * @brief   Helper function that allows you to create pair of a string with a `NTuplePtr`.
 * @details This function first attempts to see if there is already an `NTuple` in the output file. If not, it will book an `NTuple` of 
 *
 * @param tupleName This will not only be the name of your `NTuple`, but also the name of the `TTree` in the output ROOT file when you use `NTuple::write()`. The name used here is also used as identifier in `NTuplePtr` map `fNTupleMap`. In other words, you can get any of the `NTuplePtr`s in this map by using `fNTupleMap[<tuple name>]`. If there is no `NTuple` of this name, it will return a `nullptr`.
 */
NTuplePtr NTupleMap::BookNTuple(const char* tupleName) {
	const char* bookName = Form("FILE1/%s", tupleName);
	NTuplePtr nt(ntupleSvc(), bookName); // attempt to get from file
	if(!nt) { // if not available in file, book a new one
		nt = ntupleSvc()->book(bookName, CLID_ColumnWiseTuple, "ks N-Tuple example");
		// if(!nt) log << MSG::ERROR << "    Cannot book N-tuple:" << long(fNTupleMap[tupleName]) << endmsg;
	}
	fNTupleMap[tupleName] = nt;
	return nt;
}