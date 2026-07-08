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

	vector<const DEventRFBunch*> rf;
    event->Get(rf);

    if(rf.size() == 0) return;

    double locRFTime = rf[0]->dTime;
    if(isnan(locRFTime)) return;

	vector<const DTOFPoint*> locTOFPoints;
	event->Get(locTOFPoints);

	vector<const DChargedTrack*> locChargedTracks;
    event->Get(locChargedTracks);
	if (locChargedTracks.size()!=2) return;

	vector<const DFCALShower*> locFCALShowers;
    event->Get(locFCALShowers);
    vector<const DFCALHit*> locFCALHits;
    event->Get(locFCALHits);

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
    }

	if(track_doca_check){
		double track_doca = 0.; DVector3 posp,  posn;
    	dAnalysisUtilities->Calc_DOCA(piplus, piminus, posp, posn, track_doca);
    	//Quasi-vertex constraint
    	if(track_doca>1.5) track_doca_check = false;
	}

	vector<const DFMWPCHit*> fmwpchits;
    event->Get(fmwpchits);

	vector<const DCTOFPoint*> ctofpoints;
	event->Get(ctofpoints);

	const double mpic = 0.13957;

	double track_plus_energy = calculateTrackEnergy(piplus->momentum(),mpic);
	double track_minus_energy = calculateTrackEnergy(piminus->momentum(),mpic);

	for(unsigned int ib = 0; ib < beamphotons.size();ib++){
		double beamE=beamphotons[ib]->energy();
        double dt_rf=beamphotons[ib]->time()-locRFTime;
        double weight=1.;
        bool got_beam_photon = false;
        
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
		
		if(fcal_shower_piplus == 0){
			if(DCPPSelect::MatchToFCALHit_CPP(locFCALHits,fcal_matched_hits_piplus,e9e25_hit_piplus,doca_hit_piplus,e1e9_hit_piplus,fcal_piplus_pos,sumu_hit_piplus,sumv_hit_piplus)) fcal_hit_piplus += 1;
		}
		
		vector<const DFCALHit*> fcal_matched_hits_piminus;
		double e9e25_hit_piminus = 0,doca_hit_piminus = 0,e1e9_hit_piminus=0.,sumu_hit_piminus=0.,sumv_hit_piminus=0.;
		
		if(fcal_shower_piminus == 0){
			if(DCPPSelect::MatchToFCALHit_CPP(locFCALHits,fcal_matched_hits_piminus,e9e25_hit_piminus,doca_hit_piminus,e1e9_hit_piminus,fcal_piminus_pos,sumu_hit_piminus,sumv_hit_piminus)) fcal_hit_piminus += 1;
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
			fcaldoca8 = doca_hit_piplus;
			fcale9e258 = e9e25_hit_piplus;
			fcale1e98 = e1e9_hit_piplus;
			fcalblocksn8 = fcal_matched_hits_piplus.size();
			sumu8 = sumu_hit_piplus;
			sumv8 = sumv_hit_piplus;
			fcaltime8 = fcal_matched_hits_piplus[0]->t;
			nFH_p += 1;
		}
		
		
		if(!fcal_matched_hits_piminus.empty() && fcal_matched_showers_piminus.empty()){
			fcale9 = fcal_matched_hits_piminus[0]->E;
			fcalep9 = fcal_matched_hits_piminus[0]->E/piminus_pmag;
			fcaldoca9 = doca_hit_piminus;
			fcale9e259 = e9e25_hit_piminus;
			fcale1e99 = e1e9_hit_piminus;
			fcalblocksn9 = fcal_matched_hits_piminus.size();
			sumu9 = sumu_hit_piminus;
			sumv9 = sumv_hit_piminus;
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
		
		myCPPSelect->piplus3mom = piplus->momentum();
		myCPPSelect->piminus3mom = piminus->momentum();

		myCPPSelect->piplus3mom_kf = pip_p4_kinfit.Vect();
		myCPPSelect->piminus3mom_kf = pim_p4_kinfit.Vect();

		myCPPSelect->kinfit_chisq = pippim_chisq;
		myCPPSelect->kinfit_ndf = pippim_ndf;

		myCPPSelect->IS_TrackDOCAGood = track_doca_check;
		
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

/*
ProjectionResults DCPPSelect_factory::SwimTracksToAllDetectors(const DTrackTimeBased* locTimeTrack, Particle_t particle_id, const DMagneticFieldMap *bfield,double fcalfrontfaceZ,double m_TOFfront){

	const double mwpcz[6] = {811.62, 827.79, 849.04,890.65,921.84,941.12};

	ProjectionResults result;

	DReferenceTrajectory rt(bfield);
    rt.SetMass(ParticleMass(particle_id));
    rt.q = ParticleCharge(particle_id);
    rt.SetZmaxTrackingBoundary(1000.);
    rt.Swim(locTimeTrack->position(),locTimeTrack->momentum(),rt.q);

	DVector3 norm(0.0,0.0,1.0);
	DVector3 tof_face_pos(0.0,0.0,m_TOFfront);
	DVector3 fcal_face_pos(0.0,0.0,fcalfrontfaceZ);
	DVector3 fmwpc1_zpos(0.0,0.0,mwpcz[0]);
	DVector3 fmwpc2_zpos(0.0,0.0,mwpcz[1]);
	DVector3 fmwpc3_zpos(0.0,0.0,mwpcz[2]);
	DVector3 fmwpc4_zpos(0.0,0.0,mwpcz[3]);
	DVector3 fmwpc5_zpos(0.0,0.0,mwpcz[4]);
	DVector3 fmwpc6_zpos(0.0,0.0,mwpcz[5]);

	DVector3 tof_proj_pos, tof_proj_mom;
	if(rt.GetIntersectionWithPlane(tof_face_pos,norm,tof_proj_pos,tof_proj_mom) != NOERROR) return result;

	DVector3 fcal_proj_pos, fcal_proj_mom;
	if(rt.GetIntersectionWithPlane(fcal_face_pos,norm,fcal_proj_pos,fcal_proj_mom) != NOERROR) return result;

	DVector3 mwpc_proj_pos_chamber1, mwpc_proj_mom_chamber1;
	if(rt.GetIntersectionWithPlane(fmwpc1_zpos,norm,mwpc_proj_pos_chamber1,mwpc_proj_mom_chamber1) != NOERROR) return result;

	DVector3 mwpc_proj_pos_chamber2, mwpc_proj_mom_chamber2;
	if(rt.GetIntersectionWithPlane(fmwpc2_zpos,norm,mwpc_proj_pos_chamber2,mwpc_proj_mom_chamber2) != NOERROR) return result;

	DVector3 mwpc_proj_pos_chamber3, mwpc_proj_mom_chamber3;
	if(rt.GetIntersectionWithPlane(fmwpc3_zpos,norm,mwpc_proj_pos_chamber3,mwpc_proj_mom_chamber3) != NOERROR) return result;

	DVector3 mwpc_proj_pos_chamber4, mwpc_proj_mom_chamber4;
	if(rt.GetIntersectionWithPlane(fmwpc4_zpos,norm,mwpc_proj_pos_chamber4,mwpc_proj_mom_chamber4) != NOERROR) return result;

	DVector3 mwpc_proj_pos_chamber5, mwpc_proj_mom_chamber5;
	if(rt.GetIntersectionWithPlane(fmwpc5_zpos,norm,mwpc_proj_pos_chamber5,mwpc_proj_mom_chamber5) != NOERROR) return result;

	DVector3 mwpc_proj_pos_chamber6, mwpc_proj_mom_chamber6;
	if(rt.GetIntersectionWithPlane(fmwpc6_zpos,norm,mwpc_proj_pos_chamber6,mwpc_proj_mom_chamber6) != NOERROR) return result;

	result.tof_projection = tof_proj_pos;
	result.fcal_projection = fcal_proj_pos;

	result.mwpc_projections[0] = mwpc_proj_pos_chamber1;
	result.mwpc_projections[1] = mwpc_proj_pos_chamber2;
	result.mwpc_projections[2] = mwpc_proj_pos_chamber3;
	result.mwpc_projections[3] = mwpc_proj_pos_chamber4;
	result.mwpc_projections[4] = mwpc_proj_pos_chamber5;
	result.mwpc_projections[5] = mwpc_proj_pos_chamber6;

	result.projection_success = true;

	return result;

}
	*/

/*
bool DCPPSelect_factory::MatchToTOF_CPP_GEOM(const vector<const DTOFPoint*>& tof_points, DVector3 tof_proj_pos){
  double x1tof, y1tof;
  x1tof = tof_proj_pos.x();
  y1tof = tof_proj_pos.y();

  int tofxF = 0, tofyF = 0;
  for(unsigned int itof = 0;itof < tof_points.size();itof++){
	  const DTOFPoint *tp = tof_points[itof];
    double tofx = tp->pos.x();
    double tofy = tp->pos.y();
	  bool xDef = tp->Is_XPositionWellDefined();
	  bool yDef = tp->Is_YPositionWellDefined();
    if(xDef != 1 || yDef != 1) continue;
    double xdif = tofx - x1tof;
    double ydif = tofy - y1tof;

    if(xdif < -6.03 || xdif > 8.409){
      continue;
    }else{
      tofxF += 1;
	  }
    if(ydif < -5.73 || ydif > 5.08){
      continue;
    }else{
      tofyF += 1;
    }
  } 
  if(tofxF == 0 || tofyF == 0) return false;
  return true;
}
*/
/*
bool DCPPSelect_factory::MatchToFCALShower_CPP(const vector<const DFCALShower*>& fcal_showers, vector<const DFCALShower*>& fcal_matched_showers, DVector3 fcal_proj_pos, DVector3 fcal_proj_mom){
  double x1fcal = 0.0, y1fcal = 0.0;

  x1fcal = fcal_proj_pos.x();
  y1fcal = fcal_proj_pos.y();
  const DFCALShower* best_fcal_shower = nullptr;
  double best_doca = 1e9;
  for(unsigned int ifs = 0; ifs<fcal_showers.size();++ifs){
    const DFCALShower *fs1 = fcal_showers[ifs];
    double xfs1 = fs1->getPosition().X();
    double yfs1 = fs1->getPosition().Y();

    double calculated_doca = hypot((xfs1-x1fcal-0.5),(yfs1-y1fcal));
    if(calculated_doca < 5.0){
      if(calculated_doca < best_doca){
        best_doca = calculated_doca;
        best_fcal_shower = fs1;
      }
    }
  }
  if(!best_fcal_shower) return false;
  
  fcal_matched_showers.push_back(best_fcal_shower);
  return true;
  
}
*/
/*
bool DCPPSelect_factory::MatchToFCALHit_CPP(const vector<const DFCALHit*>& fcal_hits, vector<const DFCALHit*>& fcal_matched_hits, double& e9e25, double& doca, double& e1e9,DVector3 fcal_proj_pos, DVector3 fcal_proj_mom,double& sumUSh, double& sumVSh){
  double x1fcal = 0.0, y1fcal = 0.0;
  //int fcal_hit_match_count = 0;
  //double E1cut = 2.9;
  double E9cut = 4.5, E25cut = 8.5;
  
  
  x1fcal = fcal_proj_pos.x();
  y1fcal = fcal_proj_pos.y();

  //Helper Vectors
  //vector<const DFCALHit*> fcal_hits_list;
  //vector<double> e1e9_list;
  //vector<double> e9e25_list;
  //vector<double> doca_list;
  //vector<double> eoverp_list;

  const DFCALHit* best_hit = nullptr;
  double best_doca = 1e9;
  double best_e1e9 = 0.0;
  double best_e9e25 = 0.0;
  double best_sumu = 0.0;
  double best_sumv = 0.0;
  
  for(unsigned int ifh = 0; ifh<fcal_hits.size(); ++ifh){
    const DFCALHit *fcal_hit = fcal_hits[ifh];
    double fcalx = fcal_hit->x;
    double fcaly = fcal_hit->y;
    double fcale = fcal_hit->E;
    double dx = fcalx - x1fcal;
    double dy = fcaly - y1fcal;
    double fcalr = sqrt(dx*dx+dy*dy);
    if(fcalr < 5.0){
      double E1 = fcale,E9 = 0, E25 = 0;
      DVector3 trackVec(x1fcal,y1fcal,0);
      DVector3 hitVec(fcalx,fcaly,0);
      DVector3 u =(hitVec - trackVec).Unit();
      DVector3 z(0,0,1);
      DVector3 v = u.Cross(z);
      DVector3 hitLoc(0,0,0);
      sumUSh = 0;
      sumVSh = 0;
      E1 = fcale;
      double sumE = 0;
      double sumu_cand =0.,sumv_cand=0.;
      for(unsigned int jfh = 0; jfh < fcal_hits.size(); jfh++){
        const DFCALHit *fcal_hit1 = fcal_hits[jfh];
        hitLoc.SetX(fcal_hit1->x-hitVec.X());
        hitLoc.SetY(fcal_hit1->y-hitVec.Y());
        sumu_cand += (fcal_hit1->E) * pow( u.Dot( hitLoc ), 2 );
        sumv_cand += (fcal_hit1->E) * pow( v.Dot( hitLoc ), 2 );
        sumE += fcal_hit1->E;
        //if( abs(fcal_hit->x - fcal_hit1->x)<=E1cut && abs(fcal_hit->y - fcal_hit1->y)<=E1cut ) E1 = fcal_hit1->E;
        if( fabs(fcal_hit->x - fcal_hit1->x)<=E9cut && fabs(fcal_hit->y - fcal_hit1->y)<=E9cut ) E9 += fcal_hit1->E;
        if( fabs(fcal_hit->x - fcal_hit1->x)<=E25cut && fabs(fcal_hit->y - fcal_hit1->y)<=E25cut ) E25 += fcal_hit1->E;
      }
      if(sumE == 0) continue;
      sumu_cand /= sumE;
      sumv_cand /= sumE;

      double e1e9_cand = E1/E9;
      double e9e25_cand = E9/E25;

      if(fcalr < best_doca){
        best_doca = fcalr;
        best_hit = fcal_hit;
        best_e1e9 = e1e9_cand;
        best_e9e25 = e9e25_cand;
        best_sumu = sumu_cand;
        best_sumv = sumv_cand;
      }
    }
  }

  if(!best_hit) return false;

  fcal_matched_hits.push_back(best_hit);
  e9e25 = best_e9e25;
  doca = best_doca;
  e1e9 = best_e1e9;
  sumUSh = best_sumu;
  sumVSh = best_sumv;

  return true;
}
  */

/*
double DCPPSelect_factory::mwpc_sigma(int ic, double p) { // ic = chamber # from 0 to 5

	const double fmwpc_spar[6][4] = {	\
		3.8515, 1.6761, 0.98888, 0.041894, \
		3.9221, 1.7107, 1.18600, 0.057278, \
		4.0039, 1.6271, 1.53250, 0.088580, \
		13.070, 6.6978, 2.04570, 0.140260, \
		2.2558, .14294, -1., 0., \
		2.2548, .42019, 1.3483, 0.0022198 };
	
	if(p<0.2) p = 0.2;
	if(p>10.) p = 10.;
	double s  = exp( fmwpc_spar[ic][0] - fmwpc_spar[ic][1]*p );
	if(fmwpc_spar[ic][2]>0.)
			s += exp( fmwpc_spar[ic][2] - fmwpc_spar[ic][3]*p );
	return s;
}
*/

/*
bool DCPPSelect_factory::CheckTrackinMWPCFiducial_CPP(DVector3 mwpc_proj_pos){
	int track_in_mwpc = 0;
	double x1fmwpc=0.0, y1fmwpc=0.0;
	
	x1fmwpc = mwpc_proj_pos.x();
	y1fmwpc = mwpc_proj_pos.y();

	if((x1fmwpc < 5.1 && x1fmwpc > -5.1) && (y1fmwpc < 5.1 && y1fmwpc > -5.1)){
		return false;
	}else if(x1fmwpc > 73.15 || x1fmwpc < -73.15 || y1fmwpc < -73.15 || y1fmwpc > 73.15){
		return false;
	}else{
		track_in_mwpc += 1;
	}
	return track_in_mwpc > 0;
}
*/
/*
void DCPPSelect_factory::RemoveFartherDuplicateHits(std::map<int,double>& plus_map,std::map<int,double>& minus_map){
    for (auto it_plus = plus_map.begin(); it_plus != plus_map.end(); ) {
        auto it_minus = minus_map.find(it_plus->first);
        if (it_minus == minus_map.end()) {
            ++it_plus;
            continue;
        }
        if (it_plus->second > it_minus->second)
            it_plus = plus_map.erase(it_plus);
        else {
            minus_map.erase(it_minus);
            ++it_plus;
        }
    }
}
*/
/*
bool DCPPSelect_factory::ComputeMWPCWireResiduals(const vector<const DFMWPCHit*> locFMWPCHits, std::map<MWPCKey,DVector3> mwpc_projections,double track1_energy,double track2_energy,map<MWPCKey,int>& mwpc_multis,bool& piplus_track_chamber6,bool& piminus_track_chamber6){
	
	piplus_track_chamber6 = CheckTrackinMWPCFiducial_CPP(mwpc_projections.at(MWPCKey::fmwpc6_proj_plus));
	piminus_track_chamber6 = CheckTrackinMWPCFiducial_CPP(mwpc_projections.at(MWPCKey::fmwpc6_proj_minus));

	std::map<int,double> fmwpc1_plus_wire_diff, fmwpc2_plus_wire_diff, fmwpc3_plus_wire_diff,fmwpc4_plus_wire_diff, fmwpc5_plus_wire_diff,fmwpc6_plus_wire_diff;

	const double fmwpc_al[6] = {  0.25,  -0.22,   0.23,  -0.27,   0.17,  -0.06};

	int fmwpc1m8 = 0,fmwpc2m8 = 0,fmwpc3m8 = 0,fmwpc4m8 = 0,fmwpc5m8 = 0,fmwpc6m8 = 0;
	int pfinalChamber = -1;
	for(int jlayer = 0;jlayer<6;jlayer++){
		double s=-1;
		for(unsigned j = 0; j < locFMWPCHits.size();j++){
			const DFMWPCHit *hit1 = locFMWPCHits[j];
			int ilayer = hit1->layer-1;
			if(ilayer!=jlayer) continue;
			int iwire = hit1->wire;
			double coord = (iwire -72.5)*2.54*0.4;
			double track_coord = NAN;
			if(ilayer == 0) track_coord = mwpc_projections.at(MWPCKey::fmwpc1_proj_plus).X();
			if(ilayer == 1) track_coord = mwpc_projections.at(MWPCKey::fmwpc2_proj_plus).Y();
			if(ilayer == 2) track_coord = mwpc_projections.at(MWPCKey::fmwpc3_proj_plus).X();
			if(ilayer == 3) track_coord = mwpc_projections.at(MWPCKey::fmwpc4_proj_plus).Y();
			if(ilayer == 4) track_coord = mwpc_projections.at(MWPCKey::fmwpc5_proj_plus).X();
			if(ilayer == 5) track_coord = mwpc_projections.at(MWPCKey::fmwpc6_proj_plus).Y();
			if(std::isnan(track_coord)){
				continue;
			}
			s = mwpc_sigma(jlayer,track1_energy);
			double diff = coord - track_coord - fmwpc_al[ilayer];
			if(fabs(diff)>s*5.)continue;
			if(pfinalChamber < ilayer) pfinalChamber = ilayer;
			if(ilayer == 0){
				fmwpc1m8 += 1;
				fmwpc1_plus_wire_diff.emplace(iwire,diff);
			}
			if(ilayer == 1){
				fmwpc2m8 += 1;
				fmwpc2_plus_wire_diff.emplace(iwire,diff);
			}
			if(ilayer == 2){
				fmwpc3m8 += 1;
				fmwpc3_plus_wire_diff.emplace(iwire,diff);
			}
			if(ilayer == 3){
				fmwpc4m8 += 1;
				fmwpc4_plus_wire_diff.emplace(iwire,diff);
			}
			if(ilayer == 4){
				fmwpc5m8 += 1;
				fmwpc5_plus_wire_diff.emplace(iwire,diff);
			}
			if(ilayer == 5){
				fmwpc6m8 += 1;
				fmwpc6_plus_wire_diff.emplace(iwire,diff);
			}
		}
	}
	int pfinalChamber1to5 = -1;
	if(fmwpc1m8 > 0) pfinalChamber1to5 = 0;
	if(fmwpc2m8 > 0 && fmwpc1m8 > 0) pfinalChamber1to5 = 1;
	if(fmwpc3m8 > 0 && fmwpc2m8 > 0 && fmwpc1m8 > 0) pfinalChamber1to5 = 2;
	if(fmwpc4m8 > 0 && fmwpc3m8 > 0 && fmwpc2m8 > 0 && fmwpc1m8 > 0) pfinalChamber1to5 = 3;
	if(fmwpc5m8 > 0 && fmwpc4m8 > 0 && fmwpc3m8 > 0 && fmwpc2m8 > 0 && fmwpc1m8 > 0) pfinalChamber1to5 = 4;


	pfinalChamber = -1;
	if(fmwpc1m8 > 0) pfinalChamber = 0;
	if(fmwpc2m8 > 0 && fmwpc1m8 > 0) pfinalChamber = 1;
	if(fmwpc3m8 > 0 && fmwpc2m8 > 0 && fmwpc1m8 > 0) pfinalChamber = 2;
	if(fmwpc4m8 > 0 && fmwpc3m8 > 0 && fmwpc2m8 > 0 && fmwpc1m8 > 0) pfinalChamber = 3;
	if(fmwpc5m8 > 0 && fmwpc4m8 > 0 && fmwpc3m8 > 0 && fmwpc2m8 > 0 && fmwpc1m8 > 0) pfinalChamber = 4;

	std::map<int,double> fmwpc1_minus_wire_diff,fmwpc2_minus_wire_diff,fmwpc3_minus_wire_diff,fmwpc4_minus_wire_diff,
		fmwpc5_minus_wire_diff,fmwpc6_minus_wire_diff;
	int fmwpc1m9 = 0,fmwpc2m9 = 0,fmwpc3m9 = 0,fmwpc4m9 = 0,fmwpc5m9 = 0,fmwpc6m9 = 0;
	int nfinalChamber = -1;
	for(int jlayer=0;jlayer < 6;jlayer++){
		double s = -1.;
		for(unsigned j = 0; j < locFMWPCHits.size();j++){
			const DFMWPCHit *hit1 = locFMWPCHits[j];
			int ilayer = hit1->layer-1;
			if(ilayer!=jlayer) continue;
			int iwire = hit1->wire;
			double coord = (iwire -72.5)*2.54*0.4;
			double track_coord = NAN;
			if(ilayer == 0) track_coord = mwpc_projections.at(MWPCKey::fmwpc1_proj_minus).X();
			if(ilayer == 1) track_coord = mwpc_projections.at(MWPCKey::fmwpc2_proj_minus).Y();
			if(ilayer == 2) track_coord = mwpc_projections.at(MWPCKey::fmwpc3_proj_minus).X();
			if(ilayer == 3) track_coord = mwpc_projections.at(MWPCKey::fmwpc4_proj_minus).Y();
			if(ilayer == 4) track_coord = mwpc_projections.at(MWPCKey::fmwpc5_proj_minus).X();
			if(ilayer == 5) track_coord = mwpc_projections.at(MWPCKey::fmwpc6_proj_minus).Y();
			if(std::isnan(track_coord)){
				continue;
			}
			s = mwpc_sigma(jlayer,track2_energy);
			double diff = coord - track_coord - fmwpc_al[ilayer];
			if(fabs(diff)>s*5.)continue;
			if(nfinalChamber < ilayer) nfinalChamber = ilayer;
			if(ilayer == 0){ 
				fmwpc1m9 += 1;
				fmwpc1_minus_wire_diff.emplace(iwire,diff);
			}
			if(ilayer == 1){
				fmwpc2m9 += 1;
				fmwpc2_minus_wire_diff.emplace(iwire,diff);
			}
			if(ilayer == 2){
				fmwpc3m9 += 1;
				fmwpc3_minus_wire_diff.emplace(iwire,diff);
			}
			if(ilayer == 3){
				fmwpc4m9 += 1;
				fmwpc4_minus_wire_diff.emplace(iwire,diff);
			}
			if(ilayer == 4){
				fmwpc5m9 += 1;
				fmwpc5_minus_wire_diff.emplace(iwire,diff);
			}
			if(ilayer == 5){
				fmwpc6m9 += 1;
				fmwpc6_minus_wire_diff.emplace(iwire,diff);
			}
		}
	}
	int nfinalChamber1to5 = -1;
	if(fmwpc1m9 > 0) nfinalChamber1to5 = 0;
	if(fmwpc2m9 > 0 && fmwpc1m9 > 0) nfinalChamber1to5 = 1;
	if(fmwpc3m9 > 0 && fmwpc2m9 > 0 && fmwpc1m9 > 0) nfinalChamber1to5 = 2;
	if(fmwpc4m9 > 0 && fmwpc3m9 > 0 && fmwpc2m9 > 0 && fmwpc1m9 > 0) nfinalChamber1to5 = 3;
	if(fmwpc5m9 > 0 && fmwpc4m9 > 0 && fmwpc3m9 > 0 && fmwpc2m9 > 0 && fmwpc1m9 > 0) nfinalChamber1to5 = 4;

	nfinalChamber = -1;
	if(fmwpc1m9 > 0) nfinalChamber = 0;
	if(fmwpc2m9 > 0 && fmwpc1m9 > 0) nfinalChamber = 1;
	if(fmwpc3m9 > 0 && fmwpc2m9 > 0 && fmwpc1m9 > 0) nfinalChamber = 2;
	if(fmwpc4m9 > 0 && fmwpc3m9 > 0 && fmwpc2m9 > 0 && fmwpc1m9 > 0) nfinalChamber = 3;
	if(fmwpc5m9 > 0 && fmwpc4m9 > 0 && fmwpc3m9 > 0 && fmwpc2m9 > 0 && fmwpc1m9 > 0) nfinalChamber = 4;
	if(fmwpc6m9 > 0 && fmwpc5m9 > 0 && fmwpc4m9 > 0 && fmwpc3m9 > 0 && fmwpc2m9 > 0 && fmwpc1m9 > 0) nfinalChamber = 5;

	int tfinalChamber = (pfinalChamber >= nfinalChamber) ? pfinalChamber : nfinalChamber;
	int tfinalChamber1to5 = (pfinalChamber1to5 >= nfinalChamber1to5) ? pfinalChamber1to5 : nfinalChamber1to5;

	RemoveFartherDuplicateHits(fmwpc1_plus_wire_diff,fmwpc1_minus_wire_diff);
	RemoveFartherDuplicateHits(fmwpc2_plus_wire_diff,fmwpc2_minus_wire_diff);
	RemoveFartherDuplicateHits(fmwpc3_plus_wire_diff,fmwpc3_minus_wire_diff);
	RemoveFartherDuplicateHits(fmwpc4_plus_wire_diff,fmwpc4_minus_wire_diff);
	RemoveFartherDuplicateHits(fmwpc5_plus_wire_diff,fmwpc5_minus_wire_diff);
	RemoveFartherDuplicateHits(fmwpc6_plus_wire_diff,fmwpc6_minus_wire_diff);

	int fmwpc1_plus_count = static_cast<int>(fmwpc1_plus_wire_diff.size());
	int fmwpc1_minus_count = static_cast<int>(fmwpc1_minus_wire_diff.size());
	int fmwpc2_plus_count = static_cast<int>(fmwpc2_plus_wire_diff.size());
	int fmwpc2_minus_count = static_cast<int>(fmwpc2_minus_wire_diff.size());
	int fmwpc3_plus_count = static_cast<int>(fmwpc3_plus_wire_diff.size());
	int fmwpc3_minus_count = static_cast<int>(fmwpc3_minus_wire_diff.size());
	int fmwpc4_plus_count = static_cast<int>(fmwpc4_plus_wire_diff.size());
	int fmwpc4_minus_count = static_cast<int>(fmwpc4_minus_wire_diff.size());
	int fmwpc5_plus_count = static_cast<int>(fmwpc5_plus_wire_diff.size());
	int fmwpc5_minus_count = static_cast<int>(fmwpc5_minus_wire_diff.size());
	int fmwpc6_plus_count = static_cast<int>(fmwpc6_plus_wire_diff.size());
	int fmwpc6_minus_count = static_cast<int>(fmwpc6_minus_wire_diff.size());

	mwpc_multis.emplace(MWPCKey::fmwpc1_proj_plus,fmwpc1_plus_count);
	mwpc_multis.emplace(MWPCKey::fmwpc2_proj_plus,fmwpc2_plus_count);
	mwpc_multis.emplace(MWPCKey::fmwpc3_proj_plus,fmwpc3_plus_count);
	mwpc_multis.emplace(MWPCKey::fmwpc4_proj_plus,fmwpc4_plus_count);
	mwpc_multis.emplace(MWPCKey::fmwpc5_proj_plus,fmwpc5_plus_count);
	mwpc_multis.emplace(MWPCKey::fmwpc6_proj_plus,fmwpc6_plus_count);
	mwpc_multis.emplace(MWPCKey::fmwpc1_proj_minus,fmwpc1_minus_count);
	mwpc_multis.emplace(MWPCKey::fmwpc2_proj_minus,fmwpc2_minus_count);
	mwpc_multis.emplace(MWPCKey::fmwpc3_proj_minus,fmwpc3_minus_count);
	mwpc_multis.emplace(MWPCKey::fmwpc4_proj_minus,fmwpc4_minus_count);
	mwpc_multis.emplace(MWPCKey::fmwpc5_proj_minus,fmwpc5_minus_count);
	mwpc_multis.emplace(MWPCKey::fmwpc6_proj_minus,fmwpc6_minus_count);

	return true;
}
*/

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



