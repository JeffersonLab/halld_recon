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

