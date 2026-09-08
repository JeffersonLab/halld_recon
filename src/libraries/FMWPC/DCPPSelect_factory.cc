// $Id$
//
//    File: DCPPSelect_factory.cc
// Created: Sun May 17 01:31:58 PM EDT 2026
// Creator: alfab (on Linux ifarm2402.jlab.org 5.14.0-611.49.2.el9_7.x86_64 x86_64)
//

/// For more information on the syntax changes between JANA1 and JANA2, visit: https://jeffersonlab.github.io/JANA2/#/jana1to2/jana1-to-jana2

#include <iostream>
#include <iomanip>

#include "DCPPSelect_factory.h"

// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
#include <JANA/JFactoryGenerator.h>
#include <JANA/JEvent.h>

extern "C"{
void InitPlugin(JApplication *app){
	InitJANAPlugin(app);
	app->Add(new JFactoryGeneratorT<DCPPSelect_factory>());
}
} // "C"

//------------------
// Init
//------------------
void DCPPSelect_factory::Init()
{
	auto app = GetApplication();
    
    /// Acquire any parameters
    // app->GetParameter("parameter_name", m_destination);
    
    /// Set any factory flags
    // SetFactoryFlag(JFactory_Flags_t::NOT_OBJECT_OWNER);
	
}

//------------------
// BeginRun
//------------------
void DCPPSelect_factory::BeginRun(const std::shared_ptr<const JEvent>& event)
{
	auto runnumber = event->GetRunNumber();
    auto app = event->GetJApplication();
    
    auto geo_manager = app->GetService<DGeometryManager>();
    bfield = geo_manager->GetBfield(runnumber);
    
    auto geom = geo_manager->GetDGeometry(runnumber);
    geom->GetFCALZ(fcalfrontfaceZ);

    geom->GetFCALPosition(m_FCALdX,m_FCALdY,m_FCALfront);

    vector<double>  tof_face;
    geom->Get("//section/composition/posXYZ[@volume='ForwardTOF']/@X_Y_Z",tof_face);
    m_TOFdX = tof_face[0]; m_TOFdY = tof_face[1]; m_TOFfront = tof_face[2];

	
}

//------------------
// Process
//------------------
void DCPPSelect_factory::Process(const std::shared_ptr<const JEvent>& event)
{

	// Code to generate factory data goes here. Add it like:
	//
	// DCPPSelect *myDCPPSelect = new DCPPSelect;
	// myDCPPSelect->x = x;
	// myDCPPSelect->y = y;
	// ...
	// Insert(myDCPPSelect);
	// For whole collection
	// Use Set like this
	// std::vector<fac1*> results;
    // results.push_back(new fac1(...));
    // Set(results);
	// Note that the objects you create here will be deleted later
	// by the system and the mData vector will be cleared automatically.

	const double mwpcz[6] = {811.62, 827.79, 849.04,890.65,921.84,941.12};

	vector<const DBeamPhoton*> beamphotons;
    event->Get(beamphotons);

	vector<const DChargedTrack*> locChargedTracks;
    event->Get(locChargedTracks);
	if (locChargedTracks.size()!=2) return;

	vector<const DEventRFBunch*> rf;
    event->Get(rf);
    if(rf.size() == 0) return;

    double locRFTime = rf[0]->dTime;
    if(isnan(locRFTime)) return;

	vector<const DTOFPoint*> locTOFPoints;
	event->Get(locTOFPoints);

	vector<const DFCALShower*> locFCALShowers;
    event->Get(locFCALShowers);
    vector<const DFCALHit*> locFCALHits;
    event->Get(locFCALHits);

	vector<const DBCALShower*> locBCALShowers;
	event->Get(locBCALShowers);

	vector<const DBCALHit*> locBCALHits;
	event->Get(locBCALHits);

	double q1=locChargedTracks[0]->Get_Charge(),q2=locChargedTracks[1]->Get_Charge();
    if (q1*q2>0) return;

    unsigned int ip=(q1>q2)?0:1;
    unsigned int in=(q1>q2)?1:0;

	const DChargedTrackHypothesis *PiPhyp=locChargedTracks[ip]->Get_Hypothesis(PiPlus);
    if (PiPhyp==NULL) return;
    const DTrackTimeBased *piplus=PiPhyp->Get_TrackTimeBased();

    const DChargedTrackHypothesis *PiMhyp=locChargedTracks[in]->Get_Hypothesis(PiMinus);
    if (PiMhyp==NULL) return;
    const DTrackTimeBased *piminus=PiMhyp->Get_TrackTimeBased();

	bool track_doca_check = true;
	const DAnalysisUtilities *dAnalysisUtilities;
    event->GetSingle(dAnalysisUtilities);
    if(dAnalysisUtilities==NULL) {
        cerr << "unable to get DAnalysisUtilities pointer" << endl;
        track_doca_check = false;
		//return;
    }

	if(track_doca_check){
		double track_doca = 0.; DVector3 posp,  posn;
    	dAnalysisUtilities->Calc_DOCA(piplus, piminus, posp, posn, track_doca);
    	//Quasi-vertex constraint
    	if(track_doca>1.5) track_doca_check = false;
		const DVector3 vtx = 0.5*(posp+posn);
		if(fabs(vtx.Z()) > 15.) return;
	}

	vector<const DFMWPCHit*> fmwpchits;
    event->Get(fmwpchits);

	vector<const DCTOFPoint*> ctofpoints;
	event->Get(ctofpoints);

	const double mpic = 0.13957;

	double track_plus_energy = calculateTrackEnergy(piplus->momentum(),mpic);
	double track_minus_energy = calculateTrackEnergy(piminus->momentum(),mpic);

	double track_plus_time = piplus->time();
	double track_minus_time = piminus->time();

	for(unsigned int ib = 0; ib < beamphotons.size();ib++){
		double beamE=beamphotons[ib]->energy();
        double dt_rf=beamphotons[ib]->time()-locRFTime;
        double weight=1.;
        bool got_beam_photon = false;
		bool tagger_type;
		if(beamphotons[ib]->dSystem == SYS_TAGH){
			tagger_type = 0; 
		}
		if(beamphotons[ib]->dSystem == SYS_TAGM){
			tagger_type = 1; 
		}
        if(fabs(dt_rf)>6.012 && fabs(dt_rf)<18.036){
            weight = -1./6.;
            got_beam_photon=true;
        }
        if(fabs(dt_rf)<2.004){
            got_beam_photon=true;
        }
        if(got_beam_photon == false) continue;


		DVector3 norm(0.0,0.0,1.0);
		DVector3 origin(0.0,0.0,m_TOFfront);
			
		DVector3 tof_piplus_pos,tof_piminus_pos,tof_piplus_mom,tof_piminus_mom;

		ProjectionResults piplusProj = DCPPSelect::SwimTracksToAllDetectors(piplus,PiPlus,bfield,fcalfrontfaceZ,m_TOFfront);
		ProjectionResults piminusProj = DCPPSelect::SwimTracksToAllDetectors(piminus,PiMinus,bfield,fcalfrontfaceZ,m_TOFfront);

		if(!piplusProj.projection_success || !piminusProj.projection_success) return;

		tof_piplus_pos = piplusProj.tof_projection;
		tof_piminus_pos = piminusProj.tof_projection;
		
		int tof_match_plus = 0, tof_match_minus = 0;
		if(DCPPSelect::MatchToTOF_CPP_GEOM(locTOFPoints,tof_piplus_pos)) tof_match_plus +=1;
		if(DCPPSelect::MatchToTOF_CPP_GEOM(locTOFPoints,tof_piminus_pos)) tof_match_minus +=1; 


		//std::cout << "tof_match_plus= " << tof_match_plus << std::endl; 
		//std::cout << "tof_match_minus= " << tof_match_minus << std::endl;
		DVector3 fcal_face_pos(0.0,0.0,fcalfrontfaceZ);
		DVector3 fcal_piplus_pos,fcal_piminus_pos,fcal_piplus_mom,fcal_piminus_mom;

		double track_time_fcal_piplus, track_time_fcal_piminus;

		fcal_piplus_pos = piplusProj.fcal_projection;
		fcal_piminus_pos = piminusProj.fcal_projection;

		int fcal_shower_piplus = 0, fcal_shower_piminus = 0, fcal_hit_piplus = 0, fcal_hit_piminus = 0;

		vector<const DFCALShower*> fcal_matched_showers_piplus;
		if(DCPPSelect::MatchToFCALShower_CPP(locFCALShowers, fcal_matched_showers_piplus,fcal_piplus_pos)) fcal_shower_piplus += 1;
		
		vector<const DFCALShower*> fcal_matched_showers_piminus;
		if(DCPPSelect::MatchToFCALShower_CPP(locFCALShowers, fcal_matched_showers_piminus,fcal_piminus_pos)) fcal_shower_piminus += 1;
			
		vector<const DFCALHit*> fcal_matched_hits_piplus;
		double e9e25_hit_piplus = 0.,doca_hit_piplus = 0.,e1e9_hit_piplus=0.,sumu_hit_piplus=0.,sumv_hit_piplus=0.;
		FCALSingleHitVals piplusFCALSingleVals;
		FCALSingleHitVals piminusFCALSingleVals;
		if(fcal_shower_piplus == 0){
			if(DCPPSelect::MatchToFCALHit_CPP(locFCALHits,fcal_piplus_pos,fcal_matched_hits_piplus,piplusFCALSingleVals)) fcal_hit_piplus += 1;
			//if(DCPPSelect::MatchToFCALHit_CPP(locFCALHits,fcal_matched_hits_piplus,e9e25_hit_piplus,doca_hit_piplus,e1e9_hit_piplus,fcal_piplus_pos,sumu_hit_piplus,sumv_hit_piplus)) fcal_hit_piplus += 1;
		}
		
		vector<const DFCALHit*> fcal_matched_hits_piminus;
		double e9e25_hit_piminus = 0.,doca_hit_piminus = 0.,e1e9_hit_piminus=0.,sumu_hit_piminus=0.,sumv_hit_piminus=0.;
		
		if(fcal_shower_piminus == 0){
			if(DCPPSelect::MatchToFCALHit_CPP(locFCALHits,fcal_piminus_pos,fcal_matched_hits_piminus,piminusFCALSingleVals)) fcal_hit_piminus += 1;
			//if(DCPPSelect::MatchToFCALHit_CPP(locFCALHits,fcal_matched_hits_piminus,e9e25_hit_piminus,doca_hit_piminus,e1e9_hit_piminus,fcal_piminus_pos,sumu_hit_piminus,sumv_hit_piminus)) fcal_hit_piminus += 1;
		}


		double piplus_pmag = (piplus->momentum()).Mag();
		double piminus_pmag = (piminus->momentum()).Mag();
		
		double fcale8=0.,fcale9=0.,fcalep8=0.,fcalep9=0.,fcaldoca8=999.,fcaldoca9=999.,fcale9e258=0.,fcale9e259=0.,fcale1e98=0,fcale1e99=0,
		fcalblocksn8=0,fcalblocksn9=0;
		
		double sumu8=0,sumu9=0,sumv8=0,sumv9=0,fcaltime8=0,fcaltime9=0;

		int nFS_p = 0,nFS_n = 0,nFH_p = 0,nFH_n = 0;
		if(!fcal_matched_showers_piplus.empty() && fcal_matched_hits_piplus.empty()){
			fcale8 = fcal_matched_showers_piplus[0]->getEnergy();
			fcalep8 = fcal_matched_showers_piplus[0]->getEnergy()/piplus_pmag;
			fcaldoca8 = fcal_matched_showers_piplus[0]->getDocaTrack();
			fcale9e258 = fcal_matched_showers_piplus[0]->getE9E25();
			fcale1e98 = fcal_matched_showers_piplus[0]->getE1E9();
			fcalblocksn8 = fcal_matched_showers_piplus[0]->getNumBlocks();
			sumu8 = fcal_matched_showers_piplus[0]->getSumU();
			sumv8 = fcal_matched_showers_piplus[0]->getSumV();
			fcaltime8 = fcal_matched_showers_piplus[0]->getTime();
			nFS_p += 1;
		}
		
		if(!fcal_matched_showers_piminus.empty() && fcal_matched_hits_piminus.empty()){
			fcale9 = fcal_matched_showers_piminus[0]->getEnergy();
			fcalep9 = fcal_matched_showers_piminus[0]->getEnergy()/piminus_pmag;
			fcaldoca9 = fcal_matched_showers_piminus[0]->getDocaTrack();
			fcale9e259 = fcal_matched_showers_piminus[0]->getE9E25();
			fcale1e99 = fcal_matched_showers_piminus[0]->getE1E9();
			fcalblocksn9 = fcal_matched_showers_piminus[0]->getNumBlocks();
			sumu9 = fcal_matched_showers_piminus[0]->getSumU();
			sumv9 = fcal_matched_showers_piminus[0]->getSumV();
			fcaltime9 = fcal_matched_showers_piminus[0]->getTime();
			nFS_n += 1;
		}
		
		if(!fcal_matched_hits_piplus.empty() && fcal_matched_showers_piplus.empty()){
			fcale8 = fcal_matched_hits_piplus[0]->E;
			fcalep8 = fcal_matched_hits_piplus[0]->E/piplus_pmag;
			fcaldoca8 = piplusFCALSingleVals.fcalSingleDOCA;//doca_hit_piplus;
			fcale9e258 = piplusFCALSingleVals.fcalSingleE9E25;//e9e25_hit_piplus;
			fcale1e98 = piplusFCALSingleVals.fcalSingleE1E9; //e1e9_hit_piplus;
			fcalblocksn8 = fcal_matched_hits_piplus.size();
			sumu8 = piplusFCALSingleVals.fcalSingleSumU; //sumu_hit_piplus;
			sumv8 = piplusFCALSingleVals.fcalSingleSumV;//sumv_hit_piplus;
			fcaltime8 = fcal_matched_hits_piplus[0]->t;
			nFH_p += 1;
		}
		
		
		if(!fcal_matched_hits_piminus.empty() && fcal_matched_showers_piminus.empty()){
			fcale9 = fcal_matched_hits_piminus[0]->E;
			fcalep9 = fcal_matched_hits_piminus[0]->E/piminus_pmag;
			fcaldoca9 = piminusFCALSingleVals.fcalSingleDOCA;//doca_hit_piminus;
			fcale9e259 = piminusFCALSingleVals.fcalSingleE9E25;//e9e25_hit_piminus;
			fcale1e99 = piminusFCALSingleVals.fcalSingleE1E9;//e1e9_hit_piminus;
			fcalblocksn9 = fcal_matched_hits_piminus.size();
			sumu9 = piminusFCALSingleVals.fcalSingleSumU;//sumu_hit_piminus;
			sumv9 = piminusFCALSingleVals.fcalSingleSumV;
			fcaltime9 = fcal_matched_hits_piminus[0]->t;
			nFH_n += 1;
		}

		
		std::map<MWPCKey,DVector3> mwpc_projection_map;
		DVector3 mwpc_pos_chamber1_piplus, mwpc_pos_chamber1_piminus;
		mwpc_projection_map.emplace(MWPCKey::fmwpc1_proj_plus,piplusProj.mwpc_projections[0]);
		mwpc_projection_map.emplace(MWPCKey::fmwpc1_proj_minus,piminusProj.mwpc_projections[0]);

		DVector3 mwpc_pos_chamber2_piplus, mwpc_pos_chamber2_piminus;
		mwpc_projection_map.emplace(MWPCKey::fmwpc2_proj_plus,piplusProj.mwpc_projections[1]);
		mwpc_projection_map.emplace(MWPCKey::fmwpc2_proj_minus,piminusProj.mwpc_projections[1]);

		DVector3 mwpc_pos_chamber3_piplus, mwpc_pos_chamber3_piminus;
		mwpc_projection_map.emplace(MWPCKey::fmwpc3_proj_plus,piplusProj.mwpc_projections[2]);
		mwpc_projection_map.emplace(MWPCKey::fmwpc3_proj_minus,piminusProj.mwpc_projections[2]);

		DVector3 mwpc_pos_chamber4_piplus, mwpc_pos_chamber4_piminus;
		mwpc_projection_map.emplace(MWPCKey::fmwpc4_proj_plus,piplusProj.mwpc_projections[3]);
		mwpc_projection_map.emplace(MWPCKey::fmwpc4_proj_minus,piminusProj.mwpc_projections[3]);

		DVector3 mwpc_pos_chamber5_piplus, mwpc_pos_chamber5_piminus;
		mwpc_projection_map.emplace(MWPCKey::fmwpc5_proj_plus,piplusProj.mwpc_projections[4]);
		mwpc_projection_map.emplace(MWPCKey::fmwpc5_proj_minus,piminusProj.mwpc_projections[4]);

		DVector3 mwpc_pos_chamber6_piplus, mwpc_pos_chamber6_piminus;
		mwpc_projection_map.emplace(MWPCKey::fmwpc6_proj_plus,piplusProj.mwpc_projections[5]);
		mwpc_projection_map.emplace(MWPCKey::fmwpc6_proj_minus,piminusProj.mwpc_projections[5]);

		map<MWPCKey,int> mwpc_multiplicity_map;
		bool piplus_track_chamber6 = false, piminus_track_chamber6 = false;
		if(!DCPPSelect::ComputeMWPCWireResiduals(fmwpchits,mwpc_projection_map,track_plus_energy,track_minus_energy,mwpc_multiplicity_map,piplus_track_chamber6,piminus_track_chamber6)) return;

		//CTOF Matching
		vector<const DCTOFPoint*> ctof_matched_hits;
		std::vector<int> allowedPaddles = {1,2,3,4};
		DCPPSelect::MatchToCTOFHit_CPP(ctofpoints,ctof_matched_hits,piplusProj.ctof_projection,allowedPaddles);

		int paddle_counts[4] = {0,0,0,0};
		for(unsigned int ictof = 0; ictof < ctof_matched_hits.size(); ictof++){
			
			const DCTOFPoint *hit1 = ctof_matched_hits[ictof];
			switch (hit1->bar)
			{
				case 1:
					paddle_counts[0]++;
					break;
				case 2:
					paddle_counts[1]++;
					break;
				case 3:
					paddle_counts[2]++;
					break;
				case 4:
					paddle_counts[3]++;
					break;
				default:
					break;
			}
		}

		DKinFitUtils_GlueX *dKinFitUtils = new DKinFitUtils_GlueX(event);
        DKinFitter *dKinFitter = new DKinFitter(dKinFitUtils);   
        dKinFitUtils->Reset_NewEvent();
        dKinFitter->Reset_NewEvent();
        dKinFitter->Reset_NewFit();
        DoKinematicFit(beamphotons[ib],piminus,piplus,dKinFitUtils,dKinFitter,dAnalysisUtilities);
    
        double pippim_chisq=dKinFitter->Get_ChiSq();
        double pippim_ndf=dKinFitter->Get_NDF();

        TLorentzVector pip_p4_kinfit, pim_p4_kinfit;

        set<shared_ptr<DKinFitParticle>>myParticles=dKinFitter->Get_KinFitParticles();
        set<shared_ptr<DKinFitParticle>>::iterator locParticleIterator=myParticles.begin();
        for(; locParticleIterator != myParticles.end(); ++locParticleIterator){
            if ((*locParticleIterator)->Get_KinFitParticleType()==d_DetectedParticle){
                if ((*locParticleIterator)->Get_PID()==211){
                    pip_p4_kinfit=(*locParticleIterator)->Get_P4();
                }else{
                    pim_p4_kinfit=(*locParticleIterator)->Get_P4();
                }
            }
        }

		DCPPSelect *myCPPSelect = new DCPPSelect;

		myCPPSelect->beam_weight = weight;
		myCPPSelect->beam_energy = beamE;
		myCPPSelect->tagger_id = tagger_type;
		
		myCPPSelect->piplus3pos = piplus->position();
		myCPPSelect->piminus3pos = piminus->position();
		myCPPSelect->piplus3mom = piplus->momentum();
		myCPPSelect->piminus3mom = piminus->momentum();
		myCPPSelect->piplusTime = track_plus_time;
		myCPPSelect->piminusTime = track_minus_time;

		myCPPSelect->piplus3mom_kf = pip_p4_kinfit.Vect();
		myCPPSelect->piminus3mom_kf = pim_p4_kinfit.Vect();

		myCPPSelect->kinfit_chisq = pippim_chisq;
		myCPPSelect->kinfit_ndf = pippim_ndf;

		myCPPSelect->IS_TrackDOCAGood = track_doca_check;

		myCPPSelect->BCALShowersExist = (locBCALShowers.size() > 0);
		myCPPSelect->BCALHitsExist = (locBCALHits.size() > 0);

		myCPPSelect->PlusTrack_EnergyInBCAL = (PiPhyp->Get_BCALShowerMatchParams() != nullptr);
		myCPPSelect->MinusTrack_EnergyInBCAL = (PiMhyp->Get_BCALShowerMatchParams() != nullptr);
		
		myCPPSelect->IS_PlusTrackInTOF = tof_match_plus;
		myCPPSelect->IS_MinusTrackInTOF = tof_match_minus;

		myCPPSelect->IS_Chamber1ExtrapReal_plus = piplusProj.mwpc_flags[0];
		myCPPSelect->IS_Chamber2ExtrapReal_plus = piplusProj.mwpc_flags[1];
		myCPPSelect->IS_Chamber3ExtrapReal_plus = piplusProj.mwpc_flags[2];
		myCPPSelect->IS_Chamber4ExtrapReal_plus = piplusProj.mwpc_flags[3];
		myCPPSelect->IS_Chamber5ExtrapReal_plus = piplusProj.mwpc_flags[4];
		myCPPSelect->IS_Chamber6ExtrapReal_plus = piplusProj.mwpc_flags[5];

		myCPPSelect->IS_Chamber1ExtrapReal_minus = piminusProj.mwpc_flags[0];
		myCPPSelect->IS_Chamber2ExtrapReal_minus = piminusProj.mwpc_flags[1];
		myCPPSelect->IS_Chamber3ExtrapReal_minus = piminusProj.mwpc_flags[2];
		myCPPSelect->IS_Chamber4ExtrapReal_minus = piminusProj.mwpc_flags[3];
		myCPPSelect->IS_Chamber5ExtrapReal_minus = piminusProj.mwpc_flags[4];
		myCPPSelect->IS_Chamber6ExtrapReal_minus = piminusProj.mwpc_flags[5];

		myCPPSelect->IS_PlusTrackINChamber6 = piplus_track_chamber6;
		myCPPSelect->IS_MinusTrackINChamber6 = piminus_track_chamber6;

		myCPPSelect->fmwpc1n_piplus = mwpc_multiplicity_map.at(MWPCKey::fmwpc1_proj_plus);
		myCPPSelect->fmwpc2n_piplus = mwpc_multiplicity_map.at(MWPCKey::fmwpc2_proj_plus);
		myCPPSelect->fmwpc3n_piplus = mwpc_multiplicity_map.at(MWPCKey::fmwpc3_proj_plus);
		myCPPSelect->fmwpc4n_piplus = mwpc_multiplicity_map.at(MWPCKey::fmwpc4_proj_plus);
		myCPPSelect->fmwpc5n_piplus = mwpc_multiplicity_map.at(MWPCKey::fmwpc5_proj_plus);
		myCPPSelect->fmwpc6n_piplus = mwpc_multiplicity_map.at(MWPCKey::fmwpc6_proj_plus);
		myCPPSelect->fmwpc1n_piminus = mwpc_multiplicity_map.at(MWPCKey::fmwpc1_proj_minus);
		myCPPSelect->fmwpc2n_piminus = mwpc_multiplicity_map.at(MWPCKey::fmwpc2_proj_minus);
		myCPPSelect->fmwpc3n_piminus = mwpc_multiplicity_map.at(MWPCKey::fmwpc3_proj_minus);
		myCPPSelect->fmwpc4n_piminus = mwpc_multiplicity_map.at(MWPCKey::fmwpc4_proj_minus);
		myCPPSelect->fmwpc5n_piminus = mwpc_multiplicity_map.at(MWPCKey::fmwpc5_proj_minus);
		myCPPSelect->fmwpc6n_piminus = mwpc_multiplicity_map.at(MWPCKey::fmwpc6_proj_minus);

		myCPPSelect->fcal_energy_piplus   = fcale8;
		myCPPSelect->fcal_energy_piminus  = fcale9;
		myCPPSelect->fcal_eoverp_piplus   = fcalep8;
		myCPPSelect->fcal_eoverp_piminus  = fcalep9;
		myCPPSelect->fcal_doca_piplus     = fcaldoca8;
		myCPPSelect->fcal_doca_piminus    = fcaldoca9;
		myCPPSelect->fcal_e1e9_piplus     = fcale1e98;
		myCPPSelect->fcal_e1e9_piminus    = fcale1e99;
		myCPPSelect->fcal_e9e25_piplus    = fcale9e258;
		myCPPSelect->fcal_e9e25_piminus   = fcale9e259;
		myCPPSelect->fcal_nblocks_piplus  = fcalblocksn8;
		myCPPSelect->fcal_nblocks_piminus = fcalblocksn9;

		myCPPSelect->fcal_showers_count_piplus = fcal_matched_showers_piplus.size();
		myCPPSelect->fcal_showers_count_piminus = fcal_matched_showers_piminus.size();

		myCPPSelect->fcal_hit_count_piplus = fcal_matched_hits_piplus.size();
		myCPPSelect->fcal_hit_count_piminus = fcal_matched_hits_piminus.size();
		int only_fcal_hits = 0;
		int only_fcal_showers = 0;
		int pshower_mhit = 0;
		int mshower_phit = 0;

		if((locFCALHits.size() - fcal_matched_hits_piplus.size() - fcal_matched_hits_piminus.size()) == 0) only_fcal_hits = 1;
		if((locFCALShowers.size() - fcal_matched_showers_piplus.size() - fcal_matched_showers_piminus.size()) == 0) only_fcal_hits = 1;
		if(fcal_matched_showers_piminus.size() && fcal_matched_hits_piplus.size()) mshower_phit = 1;
		if(fcal_matched_showers_piplus.size() && fcal_matched_hits_piminus.size()) pshower_mhit = 1;

		myCPPSelect->fcal_hits_only = only_fcal_hits;
		myCPPSelect->fcal_showers_only = only_fcal_showers;
		myCPPSelect->fcal_shower_minus_plus_hit = mshower_phit;
		myCPPSelect->fcal_shower_plus_minus_hit = pshower_mhit;

		myCPPSelect->ctof_bar1 = paddle_counts[0];
		myCPPSelect->ctof_bar2 = paddle_counts[1];
		myCPPSelect->ctof_bar3 = paddle_counts[2];
		myCPPSelect->ctof_bar4 = paddle_counts[3];


		Insert(myCPPSelect);

		delete dKinFitter;
		delete dKinFitUtils;
	}

}

//------------------
// EndRun
//------------------
void DCPPSelect_factory::EndRun()
{
}

//------------------
// Finish
//------------------
void DCPPSelect_factory::Finish()
{
}

double DCPPSelect_factory::calculateTrackEnergy(DVector3 ptrack_mom, double pmass){
	double energy = sqrt(ptrack_mom.Mag2() + pmass*pmass);
	return energy;
}

void DCPPSelect_factory::DoKinematicFit(const DBeamPhoton *beamphoton, const DTrackTimeBased *negative, const DTrackTimeBased *positive, DKinFitUtils_GlueX *dKinFitUtils,DKinFitter *dKinFitter,const DAnalysisUtilities *dAnalysisUtilities) const {
  set<shared_ptr<DKinFitParticle>> InitialParticles, FinalParticles;
  
  Particle_t m_target = Particle_t::Pb208;

  shared_ptr<DKinFitParticle>myBeam=dKinFitUtils->Make_BeamParticle(beamphoton);
  shared_ptr<DKinFitParticle>myTarget=dKinFitUtils->Make_TargetParticle(m_target);
  
  InitialParticles.insert(myBeam);  
  InitialParticles.insert(myTarget);

  shared_ptr<DKinFitParticle>myNegativeParticle=dKinFitUtils->Make_DetectedParticle(negative);
  FinalParticles.insert(myNegativeParticle);
  shared_ptr<DKinFitParticle>myPositiveParticle=dKinFitUtils->Make_DetectedParticle(positive);
  FinalParticles.insert(myPositiveParticle);
  shared_ptr<DKinFitParticle>myRecoil=dKinFitUtils->Make_MissingParticle(m_target);
  FinalParticles.insert(myRecoil);    
  
  // make energy-momentum constraint
  shared_ptr<DKinFitConstraint_P4>locP4Constraint = dKinFitUtils->Make_P4Constraint(InitialParticles, FinalParticles);
  dKinFitter->Add_Constraint(locP4Constraint);

  // Make vertex constraint
  DVector3 vertex;
  dAnalysisUtilities->Calc_DOCAVertex(myPositiveParticle.get(), myNegativeParticle.get(),vertex);
  set<shared_ptr<DKinFitParticle>> ConstrainedParticles;
  set<shared_ptr<DKinFitParticle>> NonConstrainedParticles;
  ConstrainedParticles.insert(myNegativeParticle);
  ConstrainedParticles.insert(myPositiveParticle);
  NonConstrainedParticles.insert(myBeam);  
  NonConstrainedParticles.insert(myTarget);
  NonConstrainedParticles.insert(myRecoil);
  shared_ptr<DKinFitConstraint_Vertex> locVertexConstraint = dKinFitUtils->Make_VertexConstraint(ConstrainedParticles,NonConstrainedParticles,vertex);
  dKinFitter->Add_Constraint(locVertexConstraint);
  
  // PERFORM THE KINEMATIC FIT
  dKinFitter->Fit_Reaction();
}



