
#include "DEventWriterREST.h"

#include <DANA/DEvent.h>
#include <JANA/Calibrations/JCalibration.h>
#include <TRACKING/DTrackFitter.h>

int& DEventWriterREST::Get_NumEventWriterThreads(void) const
{
	// must be read/used entirely in "RESTWriter" lock
	static int locNumEventWriterThreads = 0;
	return locNumEventWriterThreads;
}

map<string, pair<ofstream*, hddm_r::ostream*> >& DEventWriterREST::Get_RESTOutputFilePointers(void) const
{
	// must be read/used entirely in "RESTWriter" lock
	// cannot do individual file locks, because the map itself can be modified
	static map<string, pair<ofstream*, hddm_r::ostream*> > locRESTOutputFilePointers;
	return locRESTOutputFilePointers;
}

DEventWriterREST::DEventWriterREST(const std::shared_ptr<const JEvent>& locEventLoop, string locOutputFileBaseName) : dOutputFileBaseName(locOutputFileBaseName)
{
	auto app = locEventLoop->GetJApplication();
	lockService = app->GetService<JLockService>();

	lockService->WriteLock("RESTWriter");
	{
		++Get_NumEventWriterThreads();
	}
	lockService->Unlock("RESTWriter");
	
	REST_WRITE_TRACK_EXIT_PARAMS=true;
	app->SetDefaultParameter("REST:WRITE_TRACK_EXIT_PARAMS", REST_WRITE_TRACK_EXIT_PARAMS,"Add track parameters at exit to tracking volume");

	HDDM_USE_COMPRESSION = true;
	string locCompressionString = "Turn on/off compression of the output HDDM stream. Set to \"0\" to turn off (it's on by default)";
	app->SetDefaultParameter("HDDM:USE_COMPRESSION", HDDM_USE_COMPRESSION, locCompressionString);

	HDDM_USE_INTEGRITY_CHECKS = true;
	string locIntegrityString = "Turn on/off automatic integrity checking on the output HDDM stream. Set to \"0\" to turn off (it's on by default)";
	app->SetDefaultParameter("HDDM:USE_INTEGRITY_CHECKS", HDDM_USE_INTEGRITY_CHECKS, locIntegrityString);

	HDDM_DATA_VERSION_STRING = "";
	if(app->GetJParameterManager()->Exists("REST:DATAVERSIONSTRING"))
		app->GetParameter("REST:DATAVERSIONSTRING", HDDM_DATA_VERSION_STRING);
	else
		app->SetDefaultParameter("REST:DATAVERSIONSTRING", HDDM_DATA_VERSION_STRING, "");

	REST_WRITE_DIRC_HITS = true;
	app->SetDefaultParameter("REST:WRITE_DIRC_HITS", REST_WRITE_DIRC_HITS);

	REST_WRITE_FMWPC_HITS = true;
	gPARMS->SetDefaultParameter("REST:WRITE_FMWPC_HITS", REST_WRITE_FMWPC_HITS);

	REST_WRITE_CCAL_SHOWERS = true;
	app->SetDefaultParameter("REST:WRITE_CCAL_SHOWERS", REST_WRITE_CCAL_SHOWERS);

	ADD_FCAL_DATA_FOR_CPP=false;
	gPARMS->SetDefaultParameter("PID:ADD_FCAL_DATA_FOR_CPP",ADD_FCAL_DATA_FOR_CPP);


    CCDB_CONTEXT_STRING = "";
    // if we can get the calibration context from the DANA interface, then save this as well
	JCalibration *jcalib = DEvent::GetJCalibration(locEventLoop);
	if (jcalib) {
		CCDB_CONTEXT_STRING = jcalib->GetContext();
	}
}

bool DEventWriterREST::Write_RESTEvent(const std::shared_ptr<const JEvent>& locEventLoop, string locOutputFileNameSubString) const
{
	std::vector<const DMCReaction*> reactions;
	locEventLoop->Get(reactions);

	std::vector<const DRFTime*> rftimes;
	locEventLoop->Get(rftimes);

	std::vector<const DBeamPhoton*> locBeamPhotons;
	locEventLoop->Get(locBeamPhotons);

	std::vector<const DBeamPhoton*> locBeamPhotons_TAGGEDMCGEN;
	locEventLoop->Get(locBeamPhotons_TAGGEDMCGEN, "TAGGEDMCGEN");

	std::vector<const DFCALShower*> fcalshowers;
	locEventLoop->Get(fcalshowers);

	std::vector<const DBCALShower*> bcalshowers;
	locEventLoop->Get(bcalshowers);

	std::vector<const DCCALShower*> ccalshowers;
	if(REST_WRITE_CCAL_SHOWERS) {
	    locEventLoop->Get(ccalshowers);
	}
  
	std::vector<const DTOFPoint*> tofpoints;
	locEventLoop->Get(tofpoints);

	std::vector<const DCTOFPoint*> ctofpoints;
	locEventLoop->Get(ctofpoints);

	std::vector<const DSCHit*> starthits;
	locEventLoop->Get(starthits);

	std::vector<const DTrackTimeBased*> tracks;
	locEventLoop->Get(tracks);

	std::vector<const DDetectorMatches*> locDetectorMatches;
	locEventLoop->Get(locDetectorMatches);

	std::vector<const DDIRCPmtHit*> locDIRCPmtHits;
	locEventLoop->Get(locDIRCPmtHits);

	std::vector<const DFMWPCHit*> fmwpchits;
	if(REST_WRITE_FMWPC_HITS) {
	    locEventLoop->Get(fmwpchits);
	}

	std::vector<const DEventHitStatistics*> hitStats;
	locEventLoop->Get(hitStats);

	std::vector<const DTrigger*> locTriggers;
	locEventLoop->Get(locTriggers);

	//Check to see if there are any objects to write out.  If so, don't write out an empty event
	bool locOutputDataPresentFlag = false;
	if((!reactions.empty()) || (!locBeamPhotons.empty()) || (!tracks.empty()))
		locOutputDataPresentFlag = true;
	else if((!fcalshowers.empty()) || (!bcalshowers.empty()) || (!tofpoints.empty()) || (!starthits.empty()))
		locOutputDataPresentFlag = true;
	//don't need to check detector matches: no matches if none of the above objects
	if(!locOutputDataPresentFlag)
		return true; //had correct response to data

	string locOutputFileName = Get_OutputFileName(locOutputFileNameSubString);

	hddm_r::HDDM locRecord;
	hddm_r::ReconstructedPhysicsEventList res = locRecord.addReconstructedPhysicsEvents(1);

	// load the run and event numbers
	res().setRunNo(locEventLoop->GetRunNumber());
	//The REST type for this is int64_t, whereas the event type is uint64_t
	//This copy is lazy: the last bit is lost.  However, we should never need the last bit.
	res().setEventNo(locEventLoop->GetEventNumber());

	// push any DMCReaction objects to the output record
	for (size_t i=0; i < reactions.size(); i++)
	{
		hddm_r::ReactionList rea = res().addReactions(1);
		rea().setType(reactions[i]->type);
		rea().setWeight(reactions[i]->weight);
		rea().setEbeam(reactions[i]->beam.energy());
		rea().setTargetType(reactions[i]->target.PID());

		if(i != 0)
			break;

		std::vector<const DMCThrown*> throwns;
		locEventLoop->Get(throwns);
		hddm_r::VertexList ver = rea().getVertices();
		DLorentzVector locPreviousX4(-9.9E9, -9.9E9, -9.9E9, -9.9E9);
		for(size_t it=0; it < throwns.size(); ++it)
		{
			DLorentzVector locThrownX4(throwns[it]->position(), throwns[it]->time());
			if((locThrownX4.T() != locPreviousX4.T()) || (locThrownX4.Vect() != locPreviousX4.Vect()))
			{
				//new vertex
				ver = rea().addVertices(1);
				hddm_r::OriginList ori = ver().addOrigins(1);
				ori().setT(locThrownX4.T());
				ori().setVx(locThrownX4.X());
				ori().setVy(locThrownX4.Y());
				ori().setVz(locThrownX4.Z());
				locPreviousX4 = locThrownX4;
			}

			hddm_r::ProductList pro = ver().addProducts(1);
			pro().setId(throwns[it]->myid);
			pro().setParentId(throwns[it]->parentid);
			int pdgtype = throwns[it]->pdgtype;
			if (pdgtype == 0)
				pdgtype = PDGtype((Particle_t)throwns[it]->type);
			pro().setPdgtype(pdgtype);
			hddm_r::MomentumList mom = pro().addMomenta(1);
			mom().setE(throwns[it]->energy());
			mom().setPx(throwns[it]->px());
			mom().setPy(throwns[it]->py());
			mom().setPz(throwns[it]->pz());
		}
	}

	// push any DRFTime objects to the output record
	for (size_t i=0; i < rftimes.size(); i++)
	{
		hddm_r::RFtimeList rf = res().addRFtimes(1);
		rf().setTsync(rftimes[i]->dTime);
	}

	// push any DBeamPhoton objects to the output record
	for(size_t loc_i = 0; loc_i < locBeamPhotons.size(); ++loc_i)
	{
		if(locBeamPhotons[loc_i]->dSystem == SYS_TAGM)
		{
			hddm_r::TagmBeamPhotonList locTagmBeamPhotonList = res().addTagmBeamPhotons(1);
			locTagmBeamPhotonList().setT(locBeamPhotons[loc_i]->time());
			locTagmBeamPhotonList().setE(locBeamPhotons[loc_i]->energy());
			hddm_r::TagmChannelList locTagmChannelList = locTagmBeamPhotonList().addTagmChannels(1);
			locTagmChannelList().setColumn(locBeamPhotons[loc_i]->dCounter);
		}
		else if(locBeamPhotons[loc_i]->dSystem == SYS_TAGH)
		{
			hddm_r::TaghBeamPhotonList locTaghBeamPhotonList = res().addTaghBeamPhotons(1);
			locTaghBeamPhotonList().setT(locBeamPhotons[loc_i]->time());
			locTaghBeamPhotonList().setE(locBeamPhotons[loc_i]->energy());
			hddm_r::TaghChannelList locTaghChannelList = locTaghBeamPhotonList().addTaghChannels(1);
			locTaghChannelList().setCounter(locBeamPhotons[loc_i]->dCounter);
		}
	}
	for(size_t loc_i = 0; loc_i < locBeamPhotons_TAGGEDMCGEN.size(); ++loc_i)
	{
		if(locBeamPhotons_TAGGEDMCGEN[loc_i]->dSystem == SYS_TAGM)
		{
			hddm_r::TagmBeamPhotonList locTagmBeamPhotonList = res().addTagmBeamPhotons(1);
			locTagmBeamPhotonList().setJtag("TAGGEDMCGEN");
			locTagmBeamPhotonList().setT(locBeamPhotons_TAGGEDMCGEN[loc_i]->time());
			locTagmBeamPhotonList().setE(locBeamPhotons_TAGGEDMCGEN[loc_i]->energy());
			hddm_r::TagmChannelList locTagmChannelList = locTagmBeamPhotonList().addTagmChannels(1);
			locTagmChannelList().setColumn(locBeamPhotons_TAGGEDMCGEN[loc_i]->dCounter);
		}
		else if(locBeamPhotons_TAGGEDMCGEN[loc_i]->dSystem == SYS_TAGH)
		{
			hddm_r::TaghBeamPhotonList locTaghBeamPhotonList = res().addTaghBeamPhotons(1);
			locTaghBeamPhotonList().setJtag("TAGGEDMCGEN");
			locTaghBeamPhotonList().setT(locBeamPhotons_TAGGEDMCGEN[loc_i]->time());
			locTaghBeamPhotonList().setE(locBeamPhotons_TAGGEDMCGEN[loc_i]->energy());
			hddm_r::TaghChannelList locTaghChannelList = locTaghBeamPhotonList().addTaghChannels(1);
			locTaghChannelList().setCounter(locBeamPhotons_TAGGEDMCGEN[loc_i]->dCounter);
		}
	}

	// push any DFCALShower objects to the output record
	for (size_t i=0; i < fcalshowers.size(); i++)
	{
		hddm_r::FcalShowerList fcal = res().addFcalShowers(1);
		DVector3 pos = fcalshowers[i]->getPosition();
		fcal().setX(pos(0));
		fcal().setY(pos(1));
		fcal().setZ(pos(2));
		fcal().setT(fcalshowers[i]->getTime());
		fcal().setE(fcalshowers[i]->getEnergy());
		fcal().setXerr(fcalshowers[i]->xErr());
		fcal().setYerr(fcalshowers[i]->yErr());
		fcal().setZerr(fcalshowers[i]->zErr());
		fcal().setTerr(fcalshowers[i]->tErr());
		fcal().setEerr(fcalshowers[i]->EErr());
		fcal().setXycorr(fcalshowers[i]->XYcorr());
		fcal().setXzcorr(fcalshowers[i]->XZcorr());
		fcal().setYzcorr(fcalshowers[i]->YZcorr());
		fcal().setEzcorr(fcalshowers[i]->EZcorr());
		fcal().setTzcorr(fcalshowers[i]->ZTcorr());

		// further correlations (an extension of REST format so code is different.)
		hddm_r::FcalCorrelationsList locFcalCorrelationsList = fcal().addFcalCorrelationses(1);
		locFcalCorrelationsList().setEtcorr(fcalshowers[i]->ETcorr());
		locFcalCorrelationsList().setExcorr(fcalshowers[i]->EXcorr());
		locFcalCorrelationsList().setEycorr(fcalshowers[i]->EYcorr());
		locFcalCorrelationsList().setTxcorr(fcalshowers[i]->XTcorr());
		locFcalCorrelationsList().setTycorr(fcalshowers[i]->YTcorr());

        // add in classification based on MVA		
        //hddm_r::FcalShowerClassificationList locFcalShowerClassificationList = fcal().addFcalShowerClassifications(1);
        //        locFcalShowerClassificationList().setClassifierOuput(fcalshowers[i]->getClassifierOutput());

        // add in shower properties used for MVA algorithm, etc.
        
        hddm_r::FcalShowerPropertiesList locFcalShowerPropertiesList = fcal().addFcalShowerPropertiesList(1);
        locFcalShowerPropertiesList().setDocaTrack(fcalshowers[i]->getDocaTrack());
        locFcalShowerPropertiesList().setTimeTrack(fcalshowers[i]->getTimeTrack());
        locFcalShowerPropertiesList().setSumU(fcalshowers[i]->getSumU());
        locFcalShowerPropertiesList().setSumV(fcalshowers[i]->getSumV());
        locFcalShowerPropertiesList().setE1E9(fcalshowers[i]->getE1E9());
        locFcalShowerPropertiesList().setE9E25(fcalshowers[i]->getE9E25());
        hddm_r::FcalShowerNBlocksList locFcalShowerNBlocksList = fcal().addFcalShowerNBlockses(1);
	locFcalShowerNBlocksList().setNumBlocks(fcalshowers[i]->getNumBlocks());

    }
            

	// push any DBCALShower objects to the output record
	for (size_t i=0; i < bcalshowers.size(); i++)
	{
		hddm_r::BcalShowerList bcal = res().addBcalShowers(1);
		DVector3 pos(bcalshowers[i]->x,bcalshowers[i]->y,bcalshowers[i]->z);
		bcal().setX(bcalshowers[i]->x);
		bcal().setY(bcalshowers[i]->y);
		bcal().setZ(bcalshowers[i]->z);
		bcal().setT(bcalshowers[i]->t);
		bcal().setE(bcalshowers[i]->E);
		bcal().setXerr(bcalshowers[i]->xErr());
		bcal().setYerr(bcalshowers[i]->yErr());
		bcal().setZerr(bcalshowers[i]->zErr());
		bcal().setTerr(bcalshowers[i]->tErr());
		bcal().setEerr(bcalshowers[i]->EErr());
		bcal().setXycorr(bcalshowers[i]->XYcorr());
		bcal().setXzcorr(bcalshowers[i]->XZcorr());
		bcal().setYzcorr(bcalshowers[i]->YZcorr());
		bcal().setEzcorr(bcalshowers[i]->EZcorr());
		bcal().setTzcorr(bcalshowers[i]->ZTcorr());

		// further correlations (an extension of REST format so code is different.)
		hddm_r::BcalCorrelationsList locBcalCorrelationsList = bcal().addBcalCorrelationses(1);
		locBcalCorrelationsList().setEtcorr(bcalshowers[i]->ETcorr());
		locBcalCorrelationsList().setExcorr(bcalshowers[i]->EXcorr());
		locBcalCorrelationsList().setEycorr(bcalshowers[i]->EYcorr());
		locBcalCorrelationsList().setTxcorr(bcalshowers[i]->XTcorr());
		locBcalCorrelationsList().setTycorr(bcalshowers[i]->YTcorr());

		hddm_r::PreshowerList locPreShowerList = bcal().addPreshowers(1);
		locPreShowerList().setPreshowerE(bcalshowers[i]->E_preshower);

		hddm_r::WidthList locWidthList = bcal().addWidths(1);
		locWidthList().setSigLong(bcalshowers[i]->sigLong);
		locWidthList().setSigTrans(bcalshowers[i]->sigTrans);
		locWidthList().setSigTheta(bcalshowers[i]->sigTheta);

		//N_cell
		hddm_r::BcalClusterList bcalcluster = bcal().addBcalClusters(1);
		bcalcluster().setNcell(bcalshowers[i]->N_cell);

		hddm_r::BcalLayersList bcallayerdata = bcal().addBcalLayerses(1);
		bcallayerdata().setE_L2(bcalshowers[i]->E_L2);
		bcallayerdata().setE_L3(bcalshowers[i]->E_L3);
		bcallayerdata().setE_L4(bcalshowers[i]->E_L4);
		bcallayerdata().setRmsTime(bcalshowers[i]->rmsTime);
	}

    // push any DCCALShower objects to the output record                                                                                         
    for (size_t i=0; i < ccalshowers.size(); i++)                                                                                                
    {                                                                                                                                            
        hddm_r::CcalShowerList ccal = res().addCcalShowers(1);                                                                               
        ccal().setX(ccalshowers[i]->x);                                                                                                      
        ccal().setY(ccalshowers[i]->y);                                                                                                      
        ccal().setZ(ccalshowers[i]->z);                                                                                                      
        ccal().setT(ccalshowers[i]->time);                                                                                                   
        ccal().setE(ccalshowers[i]->E);                                                                                                      
        ccal().setEmax(ccalshowers[i]->Emax);
        ccal().setTerr(ccalshowers[i]->sigma_t);                                                                                             
        ccal().setEerr(ccalshowers[i]->sigma_E);                                                                                             
        ccal().setChi2(ccalshowers[i]->chi2);
        ccal().setX1(ccalshowers[i]->x1);
        ccal().setY1(ccalshowers[i]->y1);
                                                                                                     
        ccal().setType(ccalshowers[i]->type);                                                                                                
        ccal().setDime(ccalshowers[i]->dime);                                                                                             
        ccal().setId(ccalshowers[i]->id);                                                                                                
        ccal().setIdmax(ccalshowers[i]->idmax);                                                                                                
    }

    // push any DCTOFPoint objects to the output record
	for (size_t i=0; i < ctofpoints.size(); i++)
	{
		hddm_r::CtofPointList ctof = res().addCtofPoints(1);
		ctof().setBar(ctofpoints[i]->bar);
		ctof().setX(ctofpoints[i]->pos(0));
		ctof().setY(ctofpoints[i]->pos(1));
		ctof().setZ(ctofpoints[i]->pos(2));
		ctof().setT(ctofpoints[i]->t);
		ctof().setDE(ctofpoints[i]->dE);
	}

	// push any DTOFPoint objects to the output record
	for (size_t i=0; i < tofpoints.size(); i++)
	{
		hddm_r::TofPointList tof = res().addTofPoints(1);
		tof().setX(tofpoints[i]->pos(0));
		tof().setY(tofpoints[i]->pos(1));
		tof().setZ(tofpoints[i]->pos(2));
		tof().setT(tofpoints[i]->t);
		tof().setDE(tofpoints[i]->dE);

		//Status //Assume compiler optimizes multiplication
		hddm_r::TofStatusList tofstatus = tof().addTofStatuses(1);
		int locStatus = tofpoints[i]->dHorizontalBar + 45*tofpoints[i]->dVerticalBar;
		locStatus += 45*45*tofpoints[i]->dHorizontalBarStatus + 45*45*4*tofpoints[i]->dVerticalBarStatus;
		tofstatus().setStatus(locStatus);
		// Energy deposition for each plane
		hddm_r::TofEnergyDepositionList tofEnergyDeposition = tof().addTofEnergyDepositions(1);
		tofEnergyDeposition().setDE1(tofpoints[i]->dE1);
		tofEnergyDeposition().setDE2(tofpoints[i]->dE2);
	}

	// push any DSCHit objects to the output record
	for (size_t i=0; i < starthits.size(); i++)
	{
		hddm_r::StartHitList hit = res().addStartHits(1);
		hit().setSector(starthits[i]->sector);
		hit().setT(starthits[i]->t);
		hit().setDE(starthits[i]->dE);
	}

	if(REST_WRITE_DIRC_HITS) {
		// push any DDIRCPmtHit objects to the output record
		for (size_t i=0; i < locDIRCPmtHits.size(); i++)
		{
			hddm_r::DircHitList hit = res().addDircHits(1);
			hit().setCh(locDIRCPmtHits[i]->ch);
			hit().setT(locDIRCPmtHits[i]->t);
			hit().setTot(locDIRCPmtHits[i]->tot);
		}
	}

	if(REST_WRITE_FMWPC_HITS) {
	  // push any DFMWPCHit objects to the output record
	  for (size_t i=0; i < fmwpchits.size(); i++)
	    {
	      hddm_r::FmwpcHitList hit = res().addFmwpcHits(1);
	      hit().setLayer(fmwpchits[i]->layer);
	      hit().setWire(fmwpchits[i]->wire);
	      hit().setQ(fmwpchits[i]->q);
	      hit().setAmp(fmwpchits[i]->amp);
	      hit().setT(fmwpchits[i]->t);
	      hit().setQf(fmwpchits[i]->QF);
	      hit().setPed(fmwpchits[i]->ped);
	    }
	}

	// push any DTrackTimeBased objects to the output record
	for (size_t i=0; i < tracks.size(); ++i)
	{
	

		hddm_r::ChargedTrackList tra = res().addChargedTracks(1);
		tra().setCandidateId(tracks[i]->candidateid);
		tra().setPtype(tracks[i]->PID());

		hddm_r::TrackFitList fit = tra().addTrackFits(1);
		fit().setNdof(tracks[i]->Ndof);
		fit().setChisq(tracks[i]->chisq);
		fit().setX0(tracks[i]->x());
		fit().setY0(tracks[i]->y());
		fit().setZ0(tracks[i]->z());
		fit().setPx(tracks[i]->px());
		fit().setPy(tracks[i]->py());
		fit().setPz(tracks[i]->pz());
		fit().setT0(tracks[i]->time());
		fit().setT0err(0.0);
		fit().setT0det(SYS_CDC);

		const TMatrixFSym& errors = *(tracks[i]->TrackingErrorMatrix().get());
		fit().setE11(errors(0,0));
		fit().setE12(errors(0,1));
		fit().setE13(errors(0,2));
		fit().setE14(errors(0,3));
		fit().setE15(errors(0,4));
		fit().setE22(errors(1,1));
		fit().setE23(errors(1,2));
		fit().setE24(errors(1,3));
		fit().setE25(errors(1,4));
		fit().setE33(errors(2,2));
		fit().setE34(errors(2,3));
		fit().setE35(errors(2,4));
		fit().setE44(errors(3,3));
		fit().setE45(errors(3,4));
		fit().setE55(errors(4,4));	

		hddm_r::TrackFlagsList myflags = tra().addTrackFlagses(1);
		myflags().setFlags(tracks[i]->flags);

		hddm_r::HitlayersList locHitLayers = tra().addHitlayerses(1);
		locHitLayers().setCDCrings(tracks[i]->dCDCRings);
		locHitLayers().setFDCplanes(tracks[i]->dFDCPlanes);

		vector<const DCDCTrackHit*> locCDCHits;
		tracks[i]->Get(locCDCHits);
		vector<const DFDCPseudo*> locFDCHits;
		tracks[i]->Get(locFDCHits);

		hddm_r::ExpectedhitsList locExpectedHits = tra().addExpectedhitses(1);
		//locExpectedHits().setMeasuredCDChits(locCDCHits.size());
		//locExpectedHits().setMeasuredFDChits(locFDCHits.size());
		locExpectedHits().setMeasuredCDChits(tracks[i]->measured_cdc_hits_on_track);
		locExpectedHits().setMeasuredFDChits(tracks[i]->measured_fdc_hits_on_track);
		//locExpectedHits().setMeasuredCDChits(tracks[i]->cdc_hit_usage.total_hits);
		//locExpectedHits().setMeasuredFDChits(tracks[i]->fdc_hit_usage.total_hits);
		locExpectedHits().setExpectedCDChits(tracks[i]->potential_cdc_hits_on_track);
		locExpectedHits().setExpectedFDChits(tracks[i]->potential_fdc_hits_on_track);

		hddm_r::McmatchList locMCMatches = tra().addMcmatchs(1);
		locMCMatches().setIthrown(tracks[i]->dMCThrownMatchMyID);
		locMCMatches().setNumhitsmatch(tracks[i]->dNumHitsMatchedToThrown);

		if (tracks[i]->dNumHitsUsedFordEdx_FDC + tracks[i]->dNumHitsUsedFordEdx_CDC > 0)
		{
			hddm_r::DEdxDCList elo = tra().addDEdxDCs(1);
			elo().setNsampleFDC(tracks[i]->dNumHitsUsedFordEdx_FDC);
			elo().setNsampleCDC(tracks[i]->dNumHitsUsedFordEdx_CDC);
			elo().setDxFDC(tracks[i]->ddx_FDC);
			elo().setDxCDC(tracks[i]->ddx_CDC);
			elo().setDEdxFDC(tracks[i]->ddEdx_FDC);
			elo().setDEdxCDC(tracks[i]->ddEdx_CDC);
			hddm_r::CDCAmpdEdxList elo2 = elo().addCDCAmpdEdxs(1);
			elo2().setDxCDCAmp(tracks[i]->ddx_CDC_amp);
			elo2().setDEdxCDCAmp(tracks[i]->ddEdx_CDC_amp);
            for (int it=0; it < (int)tracks[i]->ddx_CDC_trunc.size() &&
                           it < (int)tracks[i]->ddx_CDC_amp_trunc.size(); ++it)
            {
               hddm_r::CDCdEdxTruncList elo3 = elo().addCDCdEdxTruncs(1);
               elo3().setNtrunc(it);
               elo3().setDx(tracks[i]->ddx_CDC_trunc[it]);
               elo3().setDEdx(tracks[i]->ddEdx_CDC_trunc[it]);
               elo3().setDxAmp(tracks[i]->ddx_CDC_amp_trunc[it]);
               elo3().setDEdxAmp(tracks[i]->ddEdx_CDC_amp_trunc[it]);
            }
            for (int it=0; it < (int)tracks[i]->ddx_FDC_trunc.size() &&
                           it < (int)tracks[i]->ddx_FDC_amp_trunc.size(); ++it)
            {
               hddm_r::FDCdEdxTruncList elo3 = elo().addFDCdEdxTruncs(1);
               elo3().setNtrunc(it);
               elo3().setDx(tracks[i]->ddx_FDC_trunc[it]);
               elo3().setDEdx(tracks[i]->ddEdx_FDC_trunc[it]);
               elo3().setDxAmp(tracks[i]->ddx_FDC_amp_trunc[it]);
               elo3().setDEdxAmp(tracks[i]->ddEdx_FDC_amp_trunc[it]);
            }
		}
		if (REST_WRITE_TRACK_EXIT_PARAMS){
		  if (tracks[i]->extrapolations.find(SYS_NULL) != tracks[i]->extrapolations.end()) {
		    vector<DTrackFitter::Extrapolation_t>extraps=tracks[i]->extrapolations.at(SYS_NULL);
		    if (extraps.size()>0){
		      hddm_r::ExitParamsList locExitParams = tra().addExitParamses(1);
		      DVector3 pos=extraps[0].position;
		      DVector3 mom=extraps[0].momentum;
		      locExitParams().setX1(pos.X());
		      locExitParams().setY1(pos.Y());
		      locExitParams().setZ1(pos.Z()); 
		      locExitParams().setPx1(mom.X());
		      locExitParams().setPy1(mom.Y());
		      locExitParams().setPz1(mom.Z());
		      locExitParams().setT1(extraps[0].t);
		    }
		  }
		}
		
	}

	// push any DEventHitStatistics objects to the output record
	if (hitStats.size() > 0)
	{
		hddm_r::HitStatisticsList stats = res().addHitStatisticses(1);
		hddm_r::StartCountersList starts = stats().addStartCounterses(1);
		starts().setCount(hitStats[0]->start_counters);
		hddm_r::CdcStrawsList straws = stats().addCdcStrawses(1);
		straws().setCount(hitStats[0]->cdc_straws);
		hddm_r::FdcPseudosList pseudos = stats().addFdcPseudoses(1);
		pseudos().setCount(hitStats[0]->fdc_pseudos);
		hddm_r::BcalCellsList cells = stats().addBcalCellses(1);
		cells().setCount(hitStats[0]->bcal_cells);
		hddm_r::FcalBlocksList blocks = stats().addFcalBlockses(1);
		blocks().setCount(hitStats[0]->fcal_blocks);
		hddm_r::CcalBlocksList bloccs = stats().addCcalBlockses(1);
		bloccs().setCount(hitStats[0]->ccal_blocks);
		hddm_r::DircPMTsList pmts = stats().addDircPMTses(1);
		pmts().setCount(hitStats[0]->dirc_PMTs);
	}

	// push any DTrigger objects to the output record
	for (size_t i=0; i < locTriggers.size(); ++i)
	{
		hddm_r::TriggerList trigger = res().addTriggers(1);
		trigger().setL1_trig_bits(Convert_UnsignedIntToSigned(locTriggers[i]->Get_L1TriggerBits()));
		trigger().setL1_fp_trig_bits(Convert_UnsignedIntToSigned(locTriggers[i]->Get_L1FrontPanelTriggerBits()));
		
		// trigger energy sums
		hddm_r::TriggerEnergySumsList triggerEnergySum = trigger().addTriggerEnergySumses(1);
		triggerEnergySum().setBCALEnergySum(locTriggers[i]->Get_GTP_BCALEnergy());
		triggerEnergySum().setFCALEnergySum(locTriggers[i]->Get_GTP_FCALEnergy());

	}

	// push any DDetectorMatches objects to the output record
	for(size_t loc_i = 0; loc_i < locDetectorMatches.size(); ++loc_i)
	{
		hddm_r::DetectorMatchesList matches = res().addDetectorMatcheses(1);
		for(size_t loc_j = 0; loc_j < tracks.size(); ++loc_j)
		{
			vector<shared_ptr<const DBCALShowerMatchParams>> locBCALShowerMatchParamsVector;
			locDetectorMatches[loc_i]->Get_BCALMatchParams(tracks[loc_j], locBCALShowerMatchParamsVector);
			for(size_t loc_k = 0; loc_k < locBCALShowerMatchParamsVector.size(); ++loc_k)
			{
				hddm_r::BcalMatchParamsList bcalList = matches().addBcalMatchParamses(1);
				bcalList().setTrack(loc_j);

				const DBCALShower* locBCALShower = locBCALShowerMatchParamsVector[loc_k]->dBCALShower;
				size_t locBCALindex = 0;
				for(; locBCALindex < bcalshowers.size(); ++locBCALindex)
				{
					if(bcalshowers[locBCALindex] == locBCALShower)
						break;
				}
				bcalList().setShower(locBCALindex);

				bcalList().setDeltaphi(locBCALShowerMatchParamsVector[loc_k]->dDeltaPhiToShower);
				bcalList().setDeltaz(locBCALShowerMatchParamsVector[loc_k]->dDeltaZToShower);
				bcalList().setDx(locBCALShowerMatchParamsVector[loc_k]->dx);
				bcalList().setPathlength(locBCALShowerMatchParamsVector[loc_k]->dPathLength);
				bcalList().setTflight(locBCALShowerMatchParamsVector[loc_k]->dFlightTime);
				bcalList().setTflightvar(locBCALShowerMatchParamsVector[loc_k]->dFlightTimeVariance);
			}

			vector<shared_ptr<const DFCALShowerMatchParams>> locFCALShowerMatchParamsVector;
			locDetectorMatches[loc_i]->Get_FCALMatchParams(tracks[loc_j], locFCALShowerMatchParamsVector);
			for (size_t loc_k = 0; loc_k < locFCALShowerMatchParamsVector.size(); ++loc_k)
			{
				hddm_r::FcalMatchParamsList fcalList = matches().addFcalMatchParamses(1);
				fcalList().setTrack(loc_j);

				const DFCALShower* locFCALShower = locFCALShowerMatchParamsVector[loc_k]->dFCALShower;
				size_t locFCALindex = 0;
				for(; locFCALindex < fcalshowers.size(); ++locFCALindex)
				{
					if(fcalshowers[locFCALindex] == locFCALShower)
						break;
				}
				fcalList().setShower(locFCALindex);

				fcalList().setDoca(locFCALShowerMatchParamsVector[loc_k]->dDOCAToShower);
				fcalList().setDx(locFCALShowerMatchParamsVector[loc_k]->dx);
				fcalList().setPathlength(locFCALShowerMatchParamsVector[loc_k]->dPathLength);
				fcalList().setTflight(locFCALShowerMatchParamsVector[loc_k]->dFlightTime);
				fcalList().setTflightvar(locFCALShowerMatchParamsVector[loc_k]->dFlightTimeVariance);
				// Additional energy information
				if (ADD_FCAL_DATA_FOR_CPP){
				  // Sanity check for this additional info
				  double myE5x5=locFCALShowerMatchParamsVector[loc_k]->dE5x5;
				  double myE3x3=locFCALShowerMatchParamsVector[loc_k]->dE3x3;
				  double myEcenter=locFCALShowerMatchParamsVector[loc_k]->dEcenter;
				  if (myEcenter>0. || myE3x3>0. || myE5x5>0.){ 
				    hddm_r::FcalEnergyParamsList fcalEnergyParamsList = fcalList().addFcalEnergyParamses(1);
				    fcalEnergyParamsList().setEcenter(myEcenter);
				    fcalEnergyParamsList().setE3x3(myE3x3);
				    fcalEnergyParamsList().setE5x5(myE5x5);
				  }
				}
			}

			vector<shared_ptr<const DFCALSingleHitMatchParams>> locFCALSingleHitMatchParamsVector;
			locDetectorMatches[loc_i]->Get_FCALSingleHitMatchParams(tracks[loc_j], locFCALSingleHitMatchParamsVector);
			for (size_t loc_k = 0; loc_k < locFCALSingleHitMatchParamsVector.size(); ++loc_k)
			{
				hddm_r::FcalSingleHitMatchParamsList fcalSingleHitList = matches().addFcalSingleHitMatchParamses(1);
				fcalSingleHitList().setTrack(loc_j);

				fcalSingleHitList().setEhit(locFCALSingleHitMatchParamsVector[loc_k]->dEHit);
				fcalSingleHitList().setThit(locFCALSingleHitMatchParamsVector[loc_k]->dTHit);
				fcalSingleHitList().setDoca(locFCALSingleHitMatchParamsVector[loc_k]->dDOCAToHit);
				fcalSingleHitList().setDx(locFCALSingleHitMatchParamsVector[loc_k]->dx);
				fcalSingleHitList().setPathlength(locFCALSingleHitMatchParamsVector[loc_k]->dPathLength);
				fcalSingleHitList().setTflight(locFCALSingleHitMatchParamsVector[loc_k]->dFlightTime);
				fcalSingleHitList().setTflightvar(locFCALSingleHitMatchParamsVector[loc_k]->dFlightTimeVariance);
			}

			vector<shared_ptr<const DTOFHitMatchParams>> locTOFHitMatchParamsVector;
			locDetectorMatches[loc_i]->Get_TOFMatchParams(tracks[loc_j], locTOFHitMatchParamsVector);
			for(size_t loc_k = 0; loc_k < locTOFHitMatchParamsVector.size(); ++loc_k)
			{
				hddm_r::TofMatchParamsList tofList = matches().addTofMatchParamses(1);
				tofList().setTrack(loc_j);

				size_t locTOFindex = 0;
				for(; locTOFindex < tofpoints.size(); ++locTOFindex)
				{
					if(tofpoints[locTOFindex] == locTOFHitMatchParamsVector[loc_k]->dTOFPoint)
						break;
				}
				tofList().setHit(locTOFindex);

				tofList().setThit(locTOFHitMatchParamsVector[loc_k]->dHitTime);
				tofList().setThitvar(locTOFHitMatchParamsVector[loc_k]->dHitTimeVariance);
				tofList().setEhit(locTOFHitMatchParamsVector[loc_k]->dHitEnergy);

				tofList().setDEdx(locTOFHitMatchParamsVector[loc_k]->dEdx);
				tofList().setPathlength(locTOFHitMatchParamsVector[loc_k]->dPathLength);
				tofList().setTflight(locTOFHitMatchParamsVector[loc_k]->dFlightTime);
				tofList().setTflightvar(locTOFHitMatchParamsVector[loc_k]->dFlightTimeVariance);

				tofList().setDeltax(locTOFHitMatchParamsVector[loc_k]->dDeltaXToHit);
				tofList().setDeltay(locTOFHitMatchParamsVector[loc_k]->dDeltaYToHit);
				// dEdx for each plane
				hddm_r::TofDedxList tofDedx = tofList().addTofDedxs(1);
				tofDedx().setDEdx1(locTOFHitMatchParamsVector[loc_k]->dEdx1);
				tofDedx().setDEdx2(locTOFHitMatchParamsVector[loc_k]->dEdx2);

			}

			vector<shared_ptr<const DSCHitMatchParams>> locSCHitMatchParamsVector;
			locDetectorMatches[loc_i]->Get_SCMatchParams(tracks[loc_j], locSCHitMatchParamsVector);
			for(size_t loc_k = 0; loc_k < locSCHitMatchParamsVector.size(); ++loc_k)
			{
				hddm_r::ScMatchParamsList scList = matches().addScMatchParamses(1);
				scList().setTrack(loc_j);

				size_t locSCindex = 0;
				for(; locSCindex < starthits.size(); ++locSCindex)
				{
					if(starthits[locSCindex] == locSCHitMatchParamsVector[loc_k]->dSCHit)
						break;
				}
				scList().setHit(locSCindex);

				scList().setDEdx(locSCHitMatchParamsVector[loc_k]->dEdx);
				scList().setDeltaphi(locSCHitMatchParamsVector[loc_k]->dDeltaPhiToHit);
				scList().setEhit(locSCHitMatchParamsVector[loc_k]->dHitEnergy);
				scList().setPathlength(locSCHitMatchParamsVector[loc_k]->dPathLength);
				scList().setTflight(locSCHitMatchParamsVector[loc_k]->dFlightTime);
				scList().setTflightvar(locSCHitMatchParamsVector[loc_k]->dFlightTimeVariance);
				scList().setThit(locSCHitMatchParamsVector[loc_k]->dHitTime);
				scList().setThitvar(locSCHitMatchParamsVector[loc_k]->dHitTimeVariance);
			}

			
			shared_ptr<const DDIRCMatchParams> locDIRCMatchParams;
			map<shared_ptr<const DDIRCMatchParams>, vector<const DDIRCPmtHit*> > locDIRCTrackMatchParamsMap;
			DDetectorMatches *locDetectorMatch = (DDetectorMatches*)locDetectorMatches[loc_i];
                        locDetectorMatch->Get_DIRCMatchParams(tracks[loc_j], locDIRCMatchParams);
			locDetectorMatch->Get_DIRCTrackMatchParamsMap(locDIRCTrackMatchParamsMap);

			if(locDIRCMatchParams) {
				hddm_r::DircMatchParamsList dircList = matches().addDircMatchParamses(1);
        	                dircList().setTrack(loc_j);

				vector<const DDIRCPmtHit*> locDIRCHitTrackMatch = (vector<const DDIRCPmtHit*>)locDIRCTrackMatchParamsMap[locDIRCMatchParams];
				for(size_t loc_k = 0; loc_k < locDIRCPmtHits.size(); ++loc_k) {
					const DDIRCPmtHit* locDIRCPmtHit = (DDIRCPmtHit*)locDIRCPmtHits[loc_k];
					if(find(locDIRCHitTrackMatch.begin(), locDIRCHitTrackMatch.end(), locDIRCPmtHit) != locDIRCHitTrackMatch.end()) {
						hddm_r::DircMatchHitList dircHitList = matches().addDircMatchHits(1);
						dircHitList().setTrack(loc_j);
						dircHitList().setHit(loc_k);
					}
				}

				vector<DTrackFitter::Extrapolation_t> extrapolations=tracks[loc_j]->extrapolations.at(SYS_DIRC);
				DVector3 locProjPos = locDIRCMatchParams->dExtrapolatedPos;
				DVector3 locProjMom = locDIRCMatchParams->dExtrapolatedMom;
				double locFlightTime = locDIRCMatchParams->dExtrapolatedTime;
				dircList().setX(locProjPos.X());
				dircList().setY(locProjPos.Y());
				dircList().setZ(locProjPos.Z());
				dircList().setT(locFlightTime);
				dircList().setPx(locProjMom.X());
				dircList().setPy(locProjMom.Y());
				dircList().setPz(locProjMom.Z());
				dircList().setExpectthetac(locDIRCMatchParams->dExpectedThetaC);
				dircList().setThetac(locDIRCMatchParams->dThetaC);
				dircList().setDeltat(locDIRCMatchParams->dDeltaT);
				dircList().setLele(locDIRCMatchParams->dLikelihoodElectron);
				dircList().setLpi(locDIRCMatchParams->dLikelihoodPion);
				dircList().setLk(locDIRCMatchParams->dLikelihoodKaon);
				dircList().setLp(locDIRCMatchParams->dLikelihoodProton);
				dircList().setNphotons(locDIRCMatchParams->dNPhotons);
			}

			double locFlightTimePCorrelation = 0.0;
			if(locDetectorMatches[loc_i]->Get_FlightTimePCorrelation(tracks[loc_j], SYS_BCAL, locFlightTimePCorrelation))
			{
				hddm_r::TflightPCorrelationList correlationList = matches().addTflightPCorrelations(1);
				correlationList().setTrack(loc_j);
				correlationList().setSystem(SYS_BCAL);
				correlationList().setCorrelation(locFlightTimePCorrelation);
			}
			if(locDetectorMatches[loc_i]->Get_FlightTimePCorrelation(tracks[loc_j], SYS_FCAL, locFlightTimePCorrelation))
			{
				hddm_r::TflightPCorrelationList correlationList = matches().addTflightPCorrelations(1);
				correlationList().setTrack(loc_j);
				correlationList().setSystem(SYS_FCAL);
				correlationList().setCorrelation(locFlightTimePCorrelation);
			}
			if(locDetectorMatches[loc_i]->Get_FlightTimePCorrelation(tracks[loc_j], SYS_TOF, locFlightTimePCorrelation))
			{
				hddm_r::TflightPCorrelationList correlationList = matches().addTflightPCorrelations(1);
				correlationList().setTrack(loc_j);
				correlationList().setSystem(SYS_TOF);
				correlationList().setCorrelation(locFlightTimePCorrelation);
			}
			if(locDetectorMatches[loc_i]->Get_FlightTimePCorrelation(tracks[loc_j], SYS_START, locFlightTimePCorrelation))
			{
				hddm_r::TflightPCorrelationList correlationList = matches().addTflightPCorrelations(1);
				correlationList().setTrack(loc_j);
				correlationList().setSystem(SYS_START);
				correlationList().setCorrelation(locFlightTimePCorrelation);
			}

			//---------  The following are for CPP -------------//
			vector<shared_ptr<const DFMWPCMatchParams>> locFMWPCMatchParamsVector;
			locDetectorMatches[loc_i]->Get_FMWPCMatchParams(tracks[loc_j], locFMWPCMatchParamsVector);
			for(size_t loc_k = 0; loc_k < locFMWPCMatchParamsVector.size(); ++loc_k){
			  hddm_r::FmwpcMatchParamsList fmwpcList = matches().addFmwpcMatchParamses(1);
			  fmwpcList().setTrack(loc_j);
			  vector<int>locLayers=locFMWPCMatchParamsVector[loc_k]->dLayers;
			  vector<int>locNhits=locFMWPCMatchParamsVector[loc_k]->dNhits;
			  vector<int>locDists=locFMWPCMatchParamsVector[loc_k]->dDists;
			  vector<int>locClosestWires=locFMWPCMatchParamsVector[loc_k]->dClosestWires;
			  for (size_t loc_m=0;loc_m<locLayers.size();loc_m++){
			    hddm_r::FmwpcDataList fmwpcDataList = fmwpcList().addFmwpcDatas(1);
			    fmwpcDataList().setLayer(locLayers[loc_m]);
			    fmwpcDataList().setNhits(locNhits[loc_m]);
			    fmwpcDataList().setDist(locDists[loc_m]);
			    fmwpcDataList().setClosestwire(locClosestWires[loc_m]);
			  }
			}

			vector<shared_ptr<const DCTOFHitMatchParams>> locCTOFHitMatchParamsVector;
			locDetectorMatches[loc_i]->Get_CTOFMatchParams(tracks[loc_j], locCTOFHitMatchParamsVector);
			for(size_t loc_k = 0; loc_k < locCTOFHitMatchParamsVector.size(); ++loc_k)
			{
				hddm_r::CtofMatchParamsList ctofList = matches().addCtofMatchParamses(1);
				ctofList().setTrack(loc_j);

				size_t locCTOFindex = 0;
				for(; locCTOFindex < ctofpoints.size(); ++locCTOFindex)
				{
					if(ctofpoints[locCTOFindex] == locCTOFHitMatchParamsVector[loc_k]->dCTOFPoint)
						break;
				}
				ctofList().setHit(locCTOFindex);
				ctofList().setDEdx(locCTOFHitMatchParamsVector[loc_k]->dEdx);
				ctofList().setTflight(locCTOFHitMatchParamsVector[loc_k]->dFlightTime);

				ctofList().setDeltax(locCTOFHitMatchParamsVector[loc_k]->dDeltaXToHit);
				ctofList().setDeltay(locCTOFHitMatchParamsVector[loc_k]->dDeltaYToHit);
			}
		}

		for(size_t loc_j = 0; loc_j < bcalshowers.size(); ++loc_j)
		{
			double locDeltaPhi = 0.0, locDeltaZ = 0.0;
			if(!locDetectorMatches[loc_i]->Get_DistanceToNearestTrack(bcalshowers[loc_j], locDeltaPhi, locDeltaZ))
				continue;

			hddm_r::BcalDOCAtoTrackList bcalDocaList = matches().addBcalDOCAtoTracks(1);
			bcalDocaList().setShower(loc_j);
			bcalDocaList().setDeltaphi(locDeltaPhi);
			bcalDocaList().setDeltaz(locDeltaZ);
		}

		for(size_t loc_j = 0; loc_j < fcalshowers.size(); ++loc_j)
		{
			double locDistance = 0.0;
			if(!locDetectorMatches[loc_i]->Get_DistanceToNearestTrack(fcalshowers[loc_j], locDistance))
				continue;

			hddm_r::FcalDOCAtoTrackList fcalDocaList = matches().addFcalDOCAtoTracks(1);
			fcalDocaList().setShower(loc_j);
			fcalDocaList().setDoca(locDistance);
		}
	}

	// write the resulting record to the output stream
	bool locWriteStatus = Write_RESTEvent(locOutputFileName, locRecord);
	locRecord.clear();
	return locWriteStatus;
}

string DEventWriterREST::Get_OutputFileName(string locOutputFileNameSubString) const
{
	string locOutputFileName = dOutputFileBaseName;
	if (locOutputFileNameSubString != "")
		locOutputFileName += string("_") + locOutputFileNameSubString;
	return (locOutputFileName + string(".hddm"));
}

bool DEventWriterREST::Write_RESTEvent(string locOutputFileName, hddm_r::HDDM& locRecord) const
{
	lockService->WriteLock("RESTWriter");
	{
		//check to see if the REST file is open
		if(Get_RESTOutputFilePointers().find(locOutputFileName) != Get_RESTOutputFilePointers().end())
		{
			//open: get pointer, write event
			hddm_r::ostream* locOutputRESTFileStream = Get_RESTOutputFilePointers()[locOutputFileName].second;
			lockService->Unlock("RESTWriter");
			*(locOutputRESTFileStream) << locRecord;
			return true;
		}

		//not open: open it
		pair<ofstream*, hddm_r::ostream*> locRESTFilePointers(NULL, NULL);
		locRESTFilePointers.first = new ofstream(locOutputFileName.c_str());
		if(!locRESTFilePointers.first->is_open())
		{
			//failed to open
			delete locRESTFilePointers.first;
			lockService->Unlock("RESTWriter");
			return false;
		}
		locRESTFilePointers.second = new hddm_r::ostream(*locRESTFilePointers.first);

		// enable on-the-fly bzip2 compression on output stream
		if(HDDM_USE_COMPRESSION)
		{
			jout << " Enabling bz2 compression of output HDDM file stream" << std::endl;
			locRESTFilePointers.second->setCompression(hddm_r::k_bz2_compression);
		}
		else
			jout << " HDDM compression disabled" << std::endl;

		// enable a CRC data integrity check at the end of each event record
		if(HDDM_USE_INTEGRITY_CHECKS)
		{
			jout << " Enabling CRC data integrity check in output HDDM file stream" << std::endl;
			locRESTFilePointers.second->setIntegrityChecks(hddm_r::k_crc32_integrity);
		}
		else
			jout << " HDDM integrity checks disabled" << std::endl;

		// write a comment record at the head of the file
		hddm_r::HDDM locCommentRecord;
		hddm_r::ReconstructedPhysicsEventList res = locCommentRecord.addReconstructedPhysicsEvents(1);
		hddm_r::CommentList comment = res().addComments();
		comment().setText("This is a REST event stream...");
        // write out any metadata if it's been set
        if(HDDM_DATA_VERSION_STRING != "") {
            hddm_r::DataVersionStringList dataVersionString = res().addDataVersionStrings();
            dataVersionString().setText(HDDM_DATA_VERSION_STRING);
        }
        if(CCDB_CONTEXT_STRING != "") {
            hddm_r::CcdbContextList ccdbContextString = res().addCcdbContexts();
            ccdbContextString().setText(CCDB_CONTEXT_STRING);
        }
		*(locRESTFilePointers.second) << locCommentRecord;
		locCommentRecord.clear();

		//write the event
		*(locRESTFilePointers.second) << locRecord;

		//store the stream pointers
		Get_RESTOutputFilePointers()[locOutputFileName] = locRESTFilePointers;
	}
	lockService->Unlock("RESTWriter");

	return true;
}

DEventWriterREST::~DEventWriterREST(void)
{
	lockService->WriteLock("RESTWriter");
	{
		--Get_NumEventWriterThreads();
		if(Get_NumEventWriterThreads() > 0)
		{
			lockService->Unlock("RESTWriter");
			return; //not the last thread writing to REST files
		}

		//last thread writing to REST files: close all files and free all memory
		map<string, pair<ofstream*, hddm_r::ostream*> >::iterator locIterator;
		for(locIterator = Get_RESTOutputFilePointers().begin(); locIterator != Get_RESTOutputFilePointers().end(); ++locIterator)
		{
			string locOutputFileName = locIterator->first;
			if (locIterator->second.second != NULL)
				delete locIterator->second.second;
			if (locIterator->second.first != NULL)
				delete locIterator->second.first;
			std::cout << "Closed REST file " << locOutputFileName << std::endl;
		}
		Get_RESTOutputFilePointers().clear();
	}
	lockService->Unlock("RESTWriter");
}

int32_t DEventWriterREST::Convert_UnsignedIntToSigned(uint32_t locUnsignedInt) const
{
	//Convert uint32_t to int32_t
	//Scheme:
		//If bit 32 is zero, then the int32_t is the same as the uint32_t: Positive or zero
		//If bit 32 is one, and at least one other bit is 1, then the int32_t is -1 * uint32_t (after stripping the top bit)
		//If bit 32 is one, and all other bits are zero, then the int32_t is the minimum int: -(2^31)
	if((locUnsignedInt & 0x80000000) == 0)
		return int32_t(locUnsignedInt); //bit 32 is zero: positive or zero

	//bit 32 is 1. see if there is another bit set
	int32_t locTopBitStripped = int32_t(locUnsignedInt & uint32_t(0x7FFFFFFF)); //strip the top bit
	if(locTopBitStripped == 0)
		return numeric_limits<int32_t>::min(); //no other bit is set: minimum int
	return -1*locTopBitStripped; //return the negative
}
