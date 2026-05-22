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

	vector<const DChargedTrack*>locChargedTracks;
    event->Get(locChargedTracks);
	if (locChargedTracks.size()!=2) return;

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

	vector<const DFMWPCHit*> fmwpchits;
    event->Get(fmwpchits);

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


	std::map<MWPCProjKey,DVector3> mwpc_projection_map;

	DVector3 norm(0.0,0.0,1.0);
	DVector3 fmwpc1_zpos(0.0,0.0,mwpcz[0]);
	DVector3 mwpc_pos_chamber1_piplus, mwpc_mom_chamber1_piplus, mwpc_pos_chamber1_piminus,mwpc_mom_chamber1_piminus;
	if(rt_piplus.GetIntersectionWithPlane(fmwpc1_zpos,norm,mwpc_pos_chamber1_piplus,mwpc_mom_chamber1_piplus) != NOERROR) return;
	if(rt_piminus.GetIntersectionWithPlane(fmwpc1_zpos,norm,mwpc_pos_chamber1_piminus,mwpc_mom_chamber1_piminus) != NOERROR) return;
	mwpc_projection_map.emplace(MWPCProjKey::fmwpc1_proj_plus,mwpc_pos_chamber1_piplus);
	mwpc_projection_map.emplace(MWPCProjKey::fmwpc1_proj_minus,mwpc_pos_chamber1_piminus);

	DVector3 fmwpc2_zpos(0.0,0.0,mwpcz[1]);
	DVector3 mwpc_pos_chamber2_piplus, mwpc_mom_chamber2_piplus, mwpc_pos_chamber2_piminus,mwpc_mom_chamber2_piminus;
	if(rt_piplus.GetIntersectionWithPlane(fmwpc2_zpos,norm,mwpc_pos_chamber2_piplus,mwpc_mom_chamber2_piplus) != NOERROR) return;
	if(rt_piminus.GetIntersectionWithPlane(fmwpc2_zpos,norm,mwpc_pos_chamber2_piminus,mwpc_mom_chamber2_piminus) != NOERROR) return;
	mwpc_projection_map.emplace(MWPCProjKey::fmwpc2_proj_plus,mwpc_pos_chamber2_piplus);
	mwpc_projection_map.emplace(MWPCProjKey::fmwpc2_proj_minus,mwpc_pos_chamber2_piminus);

	DVector3 fmwpc3_zpos(0.0,0.0,mwpcz[2]);
	DVector3 mwpc_pos_chamber3_piplus, mwpc_mom_chamber3_piplus, mwpc_pos_chamber3_piminus,mwpc_mom_chamber3_piminus;
	if(rt_piplus.GetIntersectionWithPlane(fmwpc3_zpos,norm,mwpc_pos_chamber3_piplus,mwpc_mom_chamber3_piplus) != NOERROR) return;
	if(rt_piminus.GetIntersectionWithPlane(fmwpc3_zpos,norm,mwpc_pos_chamber3_piminus,mwpc_mom_chamber3_piminus) != NOERROR) return;
	mwpc_projection_map.emplace(MWPCProjKey::fmwpc3_proj_plus,mwpc_pos_chamber3_piplus);
	mwpc_projection_map.emplace(MWPCProjKey::fmwpc3_proj_minus,mwpc_pos_chamber3_piminus);

	DVector3 fmwpc4_zpos(0.0,0.0,mwpcz[3]);
	DVector3 mwpc_pos_chamber4_piplus, mwpc_mom_chamber4_piplus, mwpc_pos_chamber4_piminus,mwpc_mom_chamber4_piminus;
	if(rt_piplus.GetIntersectionWithPlane(fmwpc4_zpos,norm,mwpc_pos_chamber4_piplus,mwpc_mom_chamber4_piplus) != NOERROR) return;
	if(rt_piminus.GetIntersectionWithPlane(fmwpc4_zpos,norm,mwpc_pos_chamber4_piminus,mwpc_mom_chamber4_piminus) != NOERROR) return;
	mwpc_projection_map.emplace(MWPCProjKey::fmwpc4_proj_plus,mwpc_pos_chamber4_piplus);
	mwpc_projection_map.emplace(MWPCProjKey::fmwpc4_proj_minus,mwpc_pos_chamber4_piminus);

	DVector3 fmwpc5_zpos(0.0,0.0,mwpcz[4]);
	DVector3 mwpc_pos_chamber5_piplus, mwpc_mom_chamber5_piplus, mwpc_pos_chamber5_piminus,mwpc_mom_chamber5_piminus;
	if(rt_piplus.GetIntersectionWithPlane(fmwpc5_zpos,norm,mwpc_pos_chamber5_piplus,mwpc_mom_chamber5_piplus) != NOERROR) return;
	if(rt_piminus.GetIntersectionWithPlane(fmwpc5_zpos,norm,mwpc_pos_chamber5_piminus,mwpc_mom_chamber5_piminus) != NOERROR) return;
	mwpc_projection_map.emplace(MWPCProjKey::fmwpc5_proj_plus,mwpc_pos_chamber5_piplus);
	mwpc_projection_map.emplace(MWPCProjKey::fmwpc5_proj_minus,mwpc_pos_chamber5_piminus);

	DVector3 fmwpc6_zpos(0.0,0.0,mwpcz[5]);
	DVector3 mwpc_pos_chamber6_piplus, mwpc_mom_chamber6_piplus, mwpc_pos_chamber6_piminus,mwpc_mom_chamber6_piminus;
	if(rt_piplus.GetIntersectionWithPlane(fmwpc6_zpos,norm,mwpc_pos_chamber6_piplus,mwpc_mom_chamber6_piplus) != NOERROR) return;
	if(rt_piminus.GetIntersectionWithPlane(fmwpc6_zpos,norm,mwpc_pos_chamber6_piminus,mwpc_mom_chamber6_piminus) != NOERROR) return;
	mwpc_projection_map.emplace(MWPCProjKey::fmwpc6_proj_plus,mwpc_pos_chamber6_piplus);
	mwpc_projection_map.emplace(MWPCProjKey::fmwpc6_proj_minus,mwpc_pos_chamber6_piminus);

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

double DCPPSelect_factory::mwpc_sigma(int ic, double p) { // ic = chamber # from 0 to 5
	if(p<0.2) p = 0.2;
	if(p>10.) p = 10.;
	double s  = exp( fmwpc_spar[ic][0] - fmwpc_spar[ic][1]*p );
	if(fmwpc_spar[ic][2]>0.)
			s += exp( fmwpc_spar[ic][2] - fmwpc_spar[ic][3]*p );
	return s;
}


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

void DCPPSelect_factory::ComputeMWPCWireResiduals(const DTrackTimeBased* locTrackTimeBased,const vector<const DFMWPCHit*> locFMWPCHits, std::map<MWPCProjKey,DVector3> mwpc_projections){
	std::map<int,double> fmwpc1_plus_wire_diff, fmwpc2_plus_wire_diff, fmwpc3_plus_wire_diff,fmwpc4_plus_wire_diff, fmwpc5_plus_wire_diff,fmwpc6_plus_wire_diff;

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
}

