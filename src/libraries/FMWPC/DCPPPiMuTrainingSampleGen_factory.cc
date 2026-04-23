// $Id$
//
//    File: DCPPPiMuTrainingSampleGen_factory.cc
// Created: Wed Apr  1 03:58:36 PM EDT 2026
// Creator: alfab (on Linux ifarm2402.jlab.org 5.14.0-611.38.1.el9_7.x86_64 x86_64)
//


#include <iostream>
#include <iomanip>

#include "DCPPPiMuTrainingSampleGen_factory.h"

/*
extern "C"{
void InitPlugin(JApplication *app){
	InitJANAPlugin(app);
	app->Add(new JFactoryGeneratorT<DCPPPiMuTrainingSampleGen_factory>());
}
} // "C"
 */

//------------------
// Init
//------------------
void DCPPPiMuTrainingSampleGen_factory::Init()
{
	//auto app = GetApplication();
    
    /// Acquire any parameters
    // app->GetParameter("parameter_name", m_destination);
    
    /// Set any factory flags
    // SetFactoryFlag(JFactory_Flags_t::NOT_OBJECT_OWNER);
}

//------------------
// BeginRun
//------------------
void DCPPPiMuTrainingSampleGen_factory::BeginRun(const std::shared_ptr<const JEvent>& event)
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
void DCPPPiMuTrainingSampleGen_factory::Process(const std::shared_ptr<const JEvent>& event)
{

	// Code to generate factory data goes here. Add it like:
	//
	// DCPPPiMuTrainingSampleGen *myDCPPPiMuTrainingSampleGen = new DCPPPiMuTrainingSampleGen;
	// myDCPPPiMuTrainingSampleGen->x = x;
	// myDCPPPiMuTrainingSampleGen->y = y;
	// ...
	// Insert(myDCPPPiMuTrainingSampleGen);
	// For whole collection
	// Use Set like this
	// std::vector<fac1*> results;
    // results.push_back(new fac1(...));
    // Set(results);
	// Note that the objects you create here will be deleted later
	// by the system and the mData vector will be cleared automatically.


	vector<const DBeamPhoton*>beamphotons;
    event->Get(beamphotons);
    if (beamphotons.size()==0) return;

    vector<const DChargedTrack*>tracks;
    event->Get(tracks);
    if (tracks.size()!=2) return;

    vector<const DEventRFBunch*> rf;
    event->Get(rf);
    if(rf.size() == 0) return;

	double locRFTime = rf[0]->dTime;
    if(isnan(locRFTime)) return;

	const DAnalysisUtilities *dAnalysisUtilities;
    event->GetSingle(dAnalysisUtilities);
    if(dAnalysisUtilities==NULL) {
        cerr << "unable to get DAnalysisUtilities pointer" << endl;
        exit(0);
    }

	double q1=tracks[0]->Get_Charge(),q2=tracks[1]->Get_Charge();
    if (q1*q2>0) return;

    unsigned int ip=(q1>q2)?0:1;
    unsigned int in=(q1>q2)?1:0;

	const DChargedTrackHypothesis *PiPhyp=tracks[ip]->Get_Hypothesis(PiPlus);
    if (PiPhyp==NULL) return;
    const DTrackTimeBased *piplus=PiPhyp->Get_TrackTimeBased();

    const DChargedTrackHypothesis *PiMhyp=tracks[in]->Get_Hypothesis(PiMinus);
    if (PiMhyp==NULL) return;
    const DTrackTimeBased *piminus=PiMhyp->Get_TrackTimeBased();

    double pip_chi2 = piplus->chisq;
    double pim_chi2 = piminus->chisq;

    int pi_n_dofp = piplus->Ndof;
    int pi_n_dofn = piminus->Ndof;

    DReferenceTrajectory rt_piplus(bfield);
    rt_piplus.SetMass(ParticleMass(PiPlus));
    rt_piplus.q = ParticleCharge(PiPlus);

    rt_piplus.SetZmaxTrackingBoundary(1000.);

    rt_piplus.Swim(piplus->position(),piplus->momentum(),rt_piplus.q);
    
    DReferenceTrajectory rt_piminus(bfield);
    rt_piminus.SetMass(ParticleMass(PiMinus));
    rt_piminus.q = ParticleCharge(PiMinus);

    rt_piminus.SetZmaxTrackingBoundary(1000.);
    rt_piminus.Swim(piminus->position(),piminus->momentum(),rt_piminus.q);

	/*
	if(piplus->momentum().Mag() > 10.0 || piplus->momentum().Mag() < 1.0) return;
    if(piminus->momentum().Mag() > 10.0 || piminus->momentum().Mag() < 1.0) return;

	if(piplus->momentum().Mag() > 10.0 || piplus->momentum().Mag() < 1.0) return;
    if(piminus->momentum().Mag() > 10.0 || piminus->momentum().Mag() < 1.0) return;
	*/

	//double track_doca = 0.; DVector3 posp,  posn;
    //dAnalysisUtilities->Calc_DOCA(piplus, piminus, posp, posn, track_doca);

	vector<const DFCALShower*> fcalshowers;
    event->Get(fcalshowers);
    vector<const DFCALHit*> fcalhits;
    event->Get(fcalhits);
    vector<const DFMWPCHit*> fmwpchits;
    event->Get(fmwpchits);
    vector<const DTOFPoint*> tofpoints;
    event->Get(tofpoints);

    for (unsigned int i=0;i<beamphotons.size();i++){
        double beamE=beamphotons[i]->energy();
        double beamT=beamphotons[i]->time();
        double dt_rf=beamphotons[i]->time()-locRFTime;
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


        if(rt_piplus.GetIntersectionWithPlane(origin,norm,tof_piplus_pos,tof_piplus_mom) != NOERROR) return;
        if(rt_piminus.GetIntersectionWithPlane(origin,norm,tof_piminus_pos,tof_piminus_mom) != NOERROR) return;
        int tof_match_plus = 0, tof_match_minus = 0;
        if(MatchToTOF_CPP_GEOM(tofpoints,tof_piplus_pos)) tof_match_plus +=1;
        if(MatchToTOF_CPP_GEOM(tofpoints,tof_piminus_pos)) tof_match_minus +=1; 


        DVector3 fcal_face_pos(0.0,0.0,fcalfrontfaceZ);
        DVector3 fcal_piplus_pos,fcal_piminus_pos,fcal_piplus_mom,fcal_piminus_mom;

        double track_time_fcal_piplus, track_time_fcal_piminus;

        if(rt_piplus.GetIntersectionWithPlane(fcal_face_pos,norm,fcal_piplus_pos,fcal_piplus_mom,NULL,&track_time_fcal_piplus) != NOERROR) return;
        if(rt_piminus.GetIntersectionWithPlane(fcal_face_pos,norm,fcal_piminus_pos,fcal_piminus_mom,NULL,&track_time_fcal_piminus) != NOERROR) return;

        int fcal_shower_piplus = 0, fcal_shower_piminus = 0, fcal_hit_piplus = 0, fcal_hit_piminus = 0;

        vector<const DFCALShower*> fcal_matched_showers_piplus;
        if(MatchToFCALShower_CPP(fcalshowers, fcal_matched_showers_piplus,fcal_piplus_pos,fcal_piplus_mom)) fcal_shower_piplus += 1;
        
        vector<const DFCALShower*> fcal_matched_showers_piminus;
        if(MatchToFCALShower_CPP(fcalshowers, fcal_matched_showers_piminus,fcal_piminus_pos,fcal_piminus_mom)) fcal_shower_piminus += 1;
            
        vector<const DFCALHit*> fcal_matched_hits_piplus;
        double e9e25_hit_piplus = 0.,doca_hit_piplus = 0.,e1e9_hit_piplus=0.,sumu_hit_piplus=0.,sumv_hit_piplus=0.;
        
        if(fcal_shower_piplus == 0){
            if(MatchToFCALHit_CPP(fcalhits,fcal_matched_hits_piplus,e9e25_hit_piplus,doca_hit_piplus,e1e9_hit_piplus,fcal_piplus_pos,fcal_piplus_mom,sumu_hit_piplus,sumv_hit_piplus)) fcal_hit_piplus += 1;
        }
        
        vector<const DFCALHit*> fcal_matched_hits_piminus;
        double e9e25_hit_piminus = 0,doca_hit_piminus = 0,e1e9_hit_piminus=0.,sumu_hit_piminus=0.,sumv_hit_piminus=0.;
        
        if(fcal_shower_piminus == 0){
            if(MatchToFCALHit_CPP(fcalhits,fcal_matched_hits_piminus,e9e25_hit_piminus,doca_hit_piminus,e1e9_hit_piminus,fcal_piminus_pos,fcal_piminus_mom,sumu_hit_piminus,sumv_hit_piminus)) fcal_hit_piminus += 1;
        }

        DVector3 mwpc_plus_pos, mwpc_plus_mom, mwpc_minus_pos, mwpc_minus_mom;
        
        DVector3 fmwpc1_zpos(0.0,0.0,mwpcz[0]);
        //std::cout << "mwpc1 origin:x,y,z= " << origin.X() << " " << origin.Y() << " " << origin.Z() << std::endl;
        DVector3 mwpc_pos_chamber1_piplus, mwpc_mom_chamber1_piplus, mwpc_pos_chamber1_piminus,mwpc_mom_chamber1_piminus;
        if(rt_piplus.GetIntersectionWithPlane(fmwpc1_zpos,norm,mwpc_pos_chamber1_piplus,mwpc_mom_chamber1_piplus) != NOERROR) return;
        if(rt_piminus.GetIntersectionWithPlane(fmwpc1_zpos,norm,mwpc_pos_chamber1_piminus,mwpc_mom_chamber1_piminus) != NOERROR) return;
        DVector3 fmwpc2_zpos(0.0,0.0,mwpcz[1]);
        //std::cout << "mwpc2 origin:x,y,z= " << origin.X() << " " << origin.Y() << " " << origin.Z() << std::endl;
        DVector3 mwpc_pos_chamber2_piplus, mwpc_mom_chamber2_piplus, mwpc_pos_chamber2_piminus,mwpc_mom_chamber2_piminus;
        if(rt_piplus.GetIntersectionWithPlane(fmwpc2_zpos,norm,mwpc_pos_chamber2_piplus,mwpc_mom_chamber2_piplus) != NOERROR) return;
        if(rt_piminus.GetIntersectionWithPlane(fmwpc2_zpos,norm,mwpc_pos_chamber2_piminus,mwpc_mom_chamber2_piminus) != NOERROR) return;
        DVector3 fmwpc3_zpos(0.0,0.0,mwpcz[2]);
        //std::cout << "mwpc3 origin:x,y,z= " << origin.X() << " " << origin.Y() << " " << origin.Z() << std::endl;
        DVector3 mwpc_pos_chamber3_piplus, mwpc_mom_chamber3_piplus, mwpc_pos_chamber3_piminus,mwpc_mom_chamber3_piminus;
        if(rt_piplus.GetIntersectionWithPlane(fmwpc3_zpos,norm,mwpc_pos_chamber3_piplus,mwpc_mom_chamber3_piplus) != NOERROR) return;
        if(rt_piminus.GetIntersectionWithPlane(fmwpc3_zpos,norm,mwpc_pos_chamber3_piminus,mwpc_mom_chamber3_piminus) != NOERROR) return;
        DVector3 fmwpc4_zpos(0.0,0.0,mwpcz[3]);
        //std::cout << "mwpc4 origin:x,y,z= " << origin.X() << " " << origin.Y() << " " << origin.Z() << std::endl;
        DVector3 mwpc_pos_chamber4_piplus, mwpc_mom_chamber4_piplus, mwpc_pos_chamber4_piminus,mwpc_mom_chamber4_piminus;
        if(rt_piplus.GetIntersectionWithPlane(fmwpc4_zpos,norm,mwpc_pos_chamber4_piplus,mwpc_mom_chamber4_piplus) != NOERROR) return;
        if(rt_piminus.GetIntersectionWithPlane(fmwpc4_zpos,norm,mwpc_pos_chamber4_piminus,mwpc_mom_chamber4_piminus) != NOERROR) return;
        DVector3 fmwpc5_zpos(0.0,0.0,mwpcz[4]);
        //std::cout << "mwpc5 origin:x,y,z= " << origin.X() << " " << origin.Y() << " " << origin.Z() << std::endl;
        DVector3 mwpc_pos_chamber5_piplus, mwpc_mom_chamber5_piplus, mwpc_pos_chamber5_piminus,mwpc_mom_chamber5_piminus;
        if(rt_piplus.GetIntersectionWithPlane(fmwpc5_zpos,norm,mwpc_pos_chamber5_piplus,mwpc_mom_chamber5_piplus) != NOERROR) return;
        if(rt_piminus.GetIntersectionWithPlane(fmwpc5_zpos,norm,mwpc_pos_chamber5_piminus,mwpc_mom_chamber5_piminus) != NOERROR) return;
        DVector3 fmwpc6_zpos(0.0,0.0,mwpcz[5]);
        //std::cout << "mwpc6 origin:x,y,z= " << origin.X() << " " << origin.Y() << " " << origin.Z() << std::endl;
        DVector3 mwpc_pos_chamber6_piplus, mwpc_mom_chamber6_piplus, mwpc_pos_chamber6_piminus,mwpc_mom_chamber6_piminus;
        if(rt_piplus.GetIntersectionWithPlane(fmwpc6_zpos,norm,mwpc_pos_chamber6_piplus,mwpc_mom_chamber6_piplus) != NOERROR) return;
        if(rt_piminus.GetIntersectionWithPlane(fmwpc6_zpos,norm,mwpc_pos_chamber6_piminus,mwpc_mom_chamber6_piminus) != NOERROR) return;

        int mwpc_plus = 0, mwpc_minus = 0;

        if(CheckTrackinMWPCFiducial_CPP(mwpc_pos_chamber6_piplus)) mwpc_plus += 1;
        if(CheckTrackinMWPCFiducial_CPP(mwpc_pos_chamber6_piminus)) mwpc_minus += 1;

        DKinFitUtils_GlueX *dKinFitUtils = new DKinFitUtils_GlueX(event);
        DKinFitter *dKinFitter = new DKinFitter(dKinFitUtils);   
        dKinFitUtils->Reset_NewEvent();
        dKinFitter->Reset_NewEvent();
        dKinFitter->Reset_NewFit();
        DoKinematicFit(beamphotons[i],piminus,piplus,dKinFitUtils,dKinFitter,dAnalysisUtilities);
    
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


        double piplus_pmag = (piplus->momentum()).Mag();
        double piminus_pmag = (piminus->momentum()).Mag();
        
        double fcale8=0.,fcale9=0.,fcalep8=0.,fcalep9=0.,fcaldoca8=999.,fcaldoca9=999.,fcale9e258=0.,fcale9e259=0.,fcale1e98=0,fcale1e99=0,
        fcalblocksn8=0,fcalblocksn9=0;
        //double sumu8=0,sumu9=0,sumv8=0,sumv9=0,fcaltime8=0,fcaltime9=0;

        int nFS_p = 0,nFS_n = 0,nFH_p = 0,nFH_n = 0;
        if(!fcal_matched_showers_piplus.empty() && fcal_matched_hits_piplus.empty()){
            fcale8 = fcal_matched_showers_piplus[0]->getEnergy();
            fcalep8 = fcal_matched_showers_piplus[0]->getEnergy()/piplus_pmag;
            fcaldoca8 = fcal_matched_showers_piplus[0]->getDocaTrack();
            fcale9e258 = fcal_matched_showers_piplus[0]->getE9E25();
            fcale1e98 = fcal_matched_showers_piplus[0]->getE1E9();
            fcalblocksn8 = fcal_matched_showers_piplus[0]->getNumBlocks();
            //sumu8 = fcal_matched_showers_piplus[0]->getSumU();
            //sumv8 = fcal_matched_showers_piplus[0]->getSumV();
            //fcaltime8 = fcal_matched_showers_piplus[0]->getTime();
            nFS_p += 1;
        }
        
        if(!fcal_matched_showers_piminus.empty() && fcal_matched_hits_piminus.empty()){
            fcale9 = fcal_matched_showers_piminus[0]->getEnergy();
            fcalep9 = fcal_matched_showers_piminus[0]->getEnergy()/piminus_pmag;
            fcaldoca9 = fcal_matched_showers_piminus[0]->getDocaTrack();
            fcale9e259 = fcal_matched_showers_piminus[0]->getE9E25();
            fcale1e99 = fcal_matched_showers_piminus[0]->getE1E9();
            fcalblocksn9 = fcal_matched_showers_piminus[0]->getNumBlocks();
            //sumu9 = fcal_matched_showers_piminus[0]->getSumU();
            //sumv9 = fcal_matched_showers_piminus[0]->getSumV();
            //fcaltime9 = fcal_matched_showers_piminus[0]->getTime();
            nFS_n += 1;
        }
        
        //std::cout << "shower check" << endl;
        if(!fcal_matched_hits_piplus.empty() && fcal_matched_showers_piplus.empty()){
            fcale8 = fcal_matched_hits_piplus[0]->E;
            fcalep8 = fcal_matched_hits_piplus[0]->E/piplus_pmag;
            fcaldoca8 = doca_hit_piplus;
            fcale9e258 = e9e25_hit_piplus;
            fcale1e98 = e1e9_hit_piplus;
            fcalblocksn8 = fcal_matched_hits_piplus.size();
            //sumu8 = sumu_hit_piplus;
            //sumv8 = sumv_hit_piplus;
            //fcaltime8 = fcal_matched_hits_piplus[0]->t;
            nFH_p += 1;
        }
        
        
        if(!fcal_matched_hits_piminus.empty() && fcal_matched_showers_piminus.empty()){
            fcale9 = fcal_matched_hits_piminus[0]->E;
            fcalep9 = fcal_matched_hits_piminus[0]->E/piminus_pmag;
            fcaldoca9 = doca_hit_piminus;
            fcale9e259 = e9e25_hit_piminus;
            fcale1e99 = e1e9_hit_piminus;
            fcalblocksn9 = fcal_matched_hits_piminus.size();
            //sumu9 = sumu_hit_piplus;
            //sumv9 = sumv_hit_piplus;
            //fcaltime9 = fcal_matched_hits_piminus[0]->t;
            nFH_n += 1;
        }

        DVector3 piplus_3mom = piplus->momentum();
        DVector3 piminus_3mom = piminus->momentum();

        double track1_energy = calculateTrackEnergy(piplus_3mom,mpic);
        double track2_energy = calculateTrackEnergy(piminus_3mom,mpic);

        std::map<int,double> fmwpc1_plus_wire_diff, fmwpc2_plus_wire_diff, fmwpc3_plus_wire_diff,fmwpc4_plus_wire_diff,
            fmwpc5_plus_wire_diff,fmwpc6_plus_wire_diff;

        int fmwpc1m8 = 0,fmwpc2m8 = 0,fmwpc3m8 = 0,fmwpc4m8 = 0,fmwpc5m8 = 0,fmwpc6m8 = 0;
        int pfinalChamber = -1;
        for(int jlayer = 0;jlayer<6;jlayer++){
            double s=-1;
            for(unsigned j = 0; j < fmwpchits.size();j++){
                const DFMWPCHit *hit1 = fmwpchits[j];
                int ilayer = hit1->layer-1;
                if(ilayer!=jlayer) continue;
                int iwire = hit1->wire;
                double coord = (iwire -72.5)*2.54*0.4;
                double track_coord = NAN;
                if(ilayer == 0) track_coord = mwpc_pos_chamber1_piplus.X();
                if(ilayer == 1) track_coord = mwpc_pos_chamber2_piplus.Y();
                if(ilayer == 2) track_coord = mwpc_pos_chamber3_piplus.X();
                if(ilayer == 3) track_coord = mwpc_pos_chamber4_piplus.Y();
                if(ilayer == 4) track_coord = mwpc_pos_chamber5_piplus.X();
                if(ilayer == 5) track_coord = mwpc_pos_chamber6_piplus.Y();
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
            for(unsigned j = 0; j < fmwpchits.size();j++){
                const DFMWPCHit *hit1 = fmwpchits[j];
                int ilayer = hit1->layer-1;
                if(ilayer!=jlayer) continue;
                int iwire = hit1->wire;
                double coord = (iwire -72.5)*2.54*0.4;
                double track_coord = NAN;
                if(ilayer == 0) track_coord = mwpc_pos_chamber1_piminus.X();
                if(ilayer == 1) track_coord = mwpc_pos_chamber2_piminus.Y();
                if(ilayer == 2) track_coord = mwpc_pos_chamber3_piminus.X();
                if(ilayer == 3) track_coord = mwpc_pos_chamber4_piminus.Y();
                if(ilayer == 4) track_coord = mwpc_pos_chamber5_piminus.X();
                if(ilayer == 5) track_coord = mwpc_pos_chamber6_piminus.Y();
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

        size_t fmwpc1_plus_count = fmwpc1_plus_wire_diff.size();
        size_t fmwpc1_minus_count = fmwpc1_minus_wire_diff.size();
        size_t fmwpc2_plus_count = fmwpc2_plus_wire_diff.size();
        size_t fmwpc2_minus_count = fmwpc2_minus_wire_diff.size();
        size_t fmwpc3_plus_count = fmwpc3_plus_wire_diff.size();
        size_t fmwpc3_minus_count = fmwpc3_minus_wire_diff.size();
        size_t fmwpc4_plus_count = fmwpc4_plus_wire_diff.size();
        size_t fmwpc4_minus_count = fmwpc4_minus_wire_diff.size();
        size_t fmwpc5_plus_count = fmwpc5_plus_wire_diff.size();
        size_t fmwpc5_minus_count = fmwpc5_minus_wire_diff.size();
        size_t fmwpc6_plus_count = fmwpc6_plus_wire_diff.size();
        size_t fmwpc6_minus_count = fmwpc6_minus_wire_diff.size();

        TLorentzVector pip_p4(piplus->momentum(),mpic);
        TLorentzVector pim_p4(piminus->momentum(),mpic);
        DCPPPiMuTrainingSampleGen *mySampleGen = new DCPPPiMuTrainingSampleGen;
        mySampleGen->beam_energy = beamE;
        mySampleGen->beam_time = beamT;
        mySampleGen->weight = weight;
        mySampleGen->chisq_piplus=pip_chi2;
        mySampleGen->chisq_piminus=pim_chi2;
        mySampleGen->nDof_piplus=pi_n_dofp;
        mySampleGen->nDof_piminus=pi_n_dofn;
        mySampleGen->piplus_p4=pip_p4;
        mySampleGen->piminus_p4=pim_p4;
        mySampleGen->TOF_match_to_plus_track=tof_match_plus;
        mySampleGen->TOF_match_to_minus_track=tof_match_minus;
        mySampleGen->chisq_kinfit=pippim_chisq;
        mySampleGen->ndof_kinfit=pippim_ndf;
        mySampleGen->piplus_kinfit_p4=pip_p4_kinfit;
        mySampleGen->piminus_kinfit_p4=pim_p4_kinfit;
        mySampleGen->fcal_energy_piplus=fcale8;
        mySampleGen->fcal_energy_piminus=fcale9;
        mySampleGen->fcal_eoverp_piplus=fcalep8;
        mySampleGen->fcal_eoverp_piminus=fcalep9;
        mySampleGen->fcal_doca_piplus=fcaldoca8;
        mySampleGen->fcal_doca_piminus=fcaldoca9;
        mySampleGen->fcal_e1e9_piplus=fcale1e98;
        mySampleGen->fcal_e1e9_piminus=fcale1e99;
        mySampleGen->fcal_e9e25_piplus=fcale9e258;
        mySampleGen->fcal_e9e25_piminus=fcale9e259;
        mySampleGen->fcal_nblocks_piplus=fcalblocksn8;
        mySampleGen->fcal_nblocks_piminus=fcalblocksn9;
        mySampleGen->fcal_showers_count_piplus=nFS_p;
        mySampleGen->fcal_showers_count_piminus=nFS_n;
        mySampleGen->fcal_hit_count_piplus=nFH_p;
        mySampleGen->fcal_hit_count_piminus=nFS_n;
        mySampleGen->fmwpc1n_piplus=fmwpc1_plus_count;
        mySampleGen->fmwpc1n_piminus=fmwpc1_minus_count;
        mySampleGen->fmwpc2n_piplus=fmwpc2_plus_count;
        mySampleGen->fmwpc2n_piminus=fmwpc2_minus_count;
        mySampleGen->fmwpc3n_piplus=fmwpc3_plus_count;
        mySampleGen->fmwpc3n_piminus=fmwpc3_minus_count;
        mySampleGen->fmwpc4n_piplus=fmwpc4_plus_count;
        mySampleGen->fmwpc4n_piminus=fmwpc4_minus_count;
        mySampleGen->fmwpc5n_piplus=fmwpc5_plus_count;
        mySampleGen->fmwpc5n_piminus=fmwpc5_minus_count;
        mySampleGen->fmwpc6n_piplus=fmwpc6_plus_count;
        mySampleGen->fmwpc6n_piminus=fmwpc6_minus_count;
        mySampleGen->final_chamber_hit=tfinalChamber;
        mySampleGen->final_chamber_hit_1to5=tfinalChamber1to5;
        mySampleGen->hit_in_chamber6_plus=mwpc_plus;
        mySampleGen->hit_in_chamber6_minus=mwpc_minus;

        Insert(mySampleGen);
        delete dKinFitter;
        delete dKinFitUtils;
    }
}

//------------------
// EndRun
//------------------
void DCPPPiMuTrainingSampleGen_factory::EndRun()
{
}

//------------------
// Finish
//------------------
void DCPPPiMuTrainingSampleGen_factory::Finish()
{
}

double DCPPPiMuTrainingSampleGen_factory::calculateTrackEnergy(DVector3 ptrack_mom, double pmass){
  double px = ptrack_mom.X();
  double py = ptrack_mom.Y();
  double pz = ptrack_mom.Z();

  double energy = sqrt(px*px+py*py+pz*pz + pmass*pmass);

  return energy;
}

bool DCPPPiMuTrainingSampleGen_factory::MatchToTOF_CPP_GEOM(const vector<const DTOFPoint*>& tof_points, DVector3 tof_proj_pos){
  
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

bool DCPPPiMuTrainingSampleGen_factory::MatchToFCALShower_CPP(const vector<const DFCALShower*>& fcal_showers, vector<const DFCALShower*>& fcal_matched_showers, DVector3 fcal_proj_pos, DVector3 fcal_proj_mom){
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

bool DCPPPiMuTrainingSampleGen_factory::MatchToFCALHit_CPP(const vector<const DFCALHit*>& fcal_hits, vector<const DFCALHit*>& fcal_matched_hits, double& e9e25, double& doca, double& e1e9,DVector3 fcal_proj_pos, DVector3 fcal_proj_mom,double& sumUSh, double& sumVSh){
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

bool DCPPPiMuTrainingSampleGen_factory::CheckTrackinMWPCFiducial_CPP(DVector3 mwpc_proj_pos){
  int track_in_mwpc = 0;
  double x1fmwpc=0.0, y1fmwpc=0.0;
  
  x1fmwpc = mwpc_proj_pos.x();
  y1fmwpc = mwpc_proj_pos.y();

  //  std::cout << "x1fmwpc= " << x1fmwpc << endl;
  //  std::cout << "y1fmwpc= " << y1fmwpc << endl;

  //  std::cout << "x1fmwpc= "<< x1fmwpc << endl;
  //std::cout << "y1fmwpc= " << y1fmwpc << endl;
  if((x1fmwpc < 5.1 && x1fmwpc > -5.1) && (y1fmwpc < 5.1 && y1fmwpc > -5.1)){
    return false;
  }else if(x1fmwpc > 73.15 || x1fmwpc < -73.15 || y1fmwpc < -73.15 || y1fmwpc > 73.15){
    return false;
  }else{
    track_in_mwpc += 1;
  }
  return track_in_mwpc > 0;
}

void DCPPPiMuTrainingSampleGen_factory::RemoveFartherDuplicateHits(std::map<int,double>& plus_map,std::map<int,double>& minus_map)
{
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

void DCPPPiMuTrainingSampleGen_factory::DoKinematicFit(const DBeamPhoton *beamphoton, const DTrackTimeBased *negative, const DTrackTimeBased *positive, DKinFitUtils_GlueX *dKinFitUtils,DKinFitter *dKinFitter,const DAnalysisUtilities *dAnalysisUtilities) const {
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