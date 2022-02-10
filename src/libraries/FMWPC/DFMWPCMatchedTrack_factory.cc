// $Id$
//
//    File: DFMWPCMatchedTrack_factory.cc
// Created: Sat Jan 22 08:53:49 EST 2022
// Creator: davidl (on Linux ifarm1901.jlab.org 3.10.0-1160.11.1.el7.x86_64 x86_64)
//


#include <iostream>
#include <iomanip>
using namespace std;

#include "DFMWPCMatchedTrack_factory.h"
using namespace jana;

#include <FCAL/DFCALHit.h>
#include <FCAL/DFCALGeometry.h>
#include <FMWPC/DFMWPCHit.h>


//------------------
// init
//------------------
jerror_t DFMWPCMatchedTrack_factory::init(void)
{

    // These need to be promoted to either JANA config. parameters
    // or calib constants.
    MIN_DELTA_T_FCAL_PROJECTION  = 100.0; // min. time between track projection and FCAL hit to consider them matched
    MIN_DELTA_T_FMWPC_PROJECTION = 100.0; // min. time between track projection and FMWPC hit to consider them matched
    FMWPC_WIRE_SPACING           = 1.016; // distance between wires in FMWPC in cm

	return NOERROR;
}

//------------------
// brun
//------------------
jerror_t DFMWPCMatchedTrack_factory::brun(jana::JEventLoop *eventLoop, int32_t runnumber)
{
	return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t DFMWPCMatchedTrack_factory::evnt(JEventLoop *loop, uint64_t eventnumber)
{
    vector<const DTrackTimeBased*> tbts;
    vector<const DFCALHit*>        fcalhits;
    vector<const DFMWPCHit*>       fmwpchits;
    const DFCALGeometry*           fcalgeom = nullptr;
    loop->Get(tbts);
    loop->Get(fcalhits);
    loop->Get(fmwpchits);
    loop->GetSingle( fcalgeom );

    // Make sure we found a DFCALGeometry object
    if( ! fcalgeom ){
        _DBG_ << " Missing DFCALGeometry!!" << endl;
        return NOERROR; // should this be a fatal error?
    }    

    // Loop over time-based tracks and make a DFMWPCMatchedTrack
    // for any that have a mass hypothesis of interest.
    for( auto tbt : tbts ){
        
        // Only interested in e and pi tracks
        // (we don't usually fit to muon mass hypotheses but
        // include it just in case someone does)
        bool is_pid_of_interest = false;
        switch( tbt->PID() ){
            case Positron:
            case Electron:
            case MuonPlus:
            case MuonMinus:
            case PiPlus:
            case PiMinus:
                is_pid_of_interest = true;
                break;
            default:
                // this statement is redundant since we initialize it to false.
                // It does prevent compiler complaints and makes the intent
                // of the author clearer.
                is_pid_of_interest = false; 
        }
        
        // Bail early if we're not making a DFMWPCMatchedTrack for this tbt
        if( !is_pid_of_interest ) continue;

        // Make a DFMWPCMatchedTrack
        auto fmpwc_mt = new DFMWPCMatchedTrack(tbt);
        fmpwc_mt->AddAssociatedObject( tbt ); // don't really need this as associated object, but allows janaview to click through to it for easier debugging

        // Match to FCAL
        DVector3 pos;
        DVector3 mom;
        double t;
        if( tbt->GetProjection(SYS_FCAL, pos, &mom, &t) ){
            // Found projection to FCAL
            auto row = fcalgeom->row( (float)pos.y() );
            auto col = fcalgeom->column( (float)pos.x() );

            // Loop over FCAL hits and gather info for
            // ones in time with the track projection.
            for( auto fcalhit : fcalhits ){
                auto delta_t = fabs( fcalhit->t - t);
                if( delta_t > MIN_DELTA_T_FCAL_PROJECTION ) continue; // not matched in time. ignore this hit
                
                auto delta_row = abs( fcalhit->row - row );
                auto delta_col = abs( fcalhit->column - col );
                if( (delta_row<=2) && (delta_col<=2) ){
                    fmpwc_mt->FCAL_E_5x5 += fcalhit->E;
                    fmpwc_mt->AddAssociatedObject( fcalhit );
                    if( (delta_row<=1) && (delta_col<=1) ){
                        fmpwc_mt->FCAL_E_3x3 += fcalhit->E;
                        if( (delta_row==0) && (delta_col==0) ){
                            fmpwc_mt->FCAL_E_center = fcalhit->E;
                        }
                    }
                }
            }
        }

        // Match to FMWPC layers
        try{
            // Get all extrapolations and loop over layers
            auto fmwpc_projections=tbt->extrapolations.at(SYS_FMWPC);
            for( int layer=1; layer<=(int)fmwpc_projections.size(); layer++){
                auto proj = fmwpc_projections[layer-1];

//                if( layer==1) _DBG_<<" proj.position.x()="<<proj.position.x()<<" proj.position.y()="<<proj.position.y()<<endl;

                // TODO: Fix this code so the geometry numbers come from
                // a central service like DGeometry. Some of this is really
                // just guesses too so it needs to be carefully reviewed.
                int wire_trk_proj = 0;
                switch( layer ){
                    case 1:
                    case 3:
                    case 5:
                        wire_trk_proj = round(71.5 + proj.position.x()/FMWPC_WIRE_SPACING) + 1; // 1-144
                        break;
                    case 2:
                    case 4:
                    case 6:
                        wire_trk_proj = round(71.5 + proj.position.y()/FMWPC_WIRE_SPACING) + 1; // 1-144
                        break;
                }
                
                // Check if projection is within bound of this layer
//                _DBG_<<" layer=" << layer << "  wire_trk_proj="<<wire_trk_proj<<"  proj.position.z()="<<proj.position.z()<<endl;
                if( (wire_trk_proj>=1) && (wire_trk_proj<=144) ) {

                    // Loop over FMWPC hits that are in-time with the track projection.
                    // Fill a 144 length array to know which wires fired. This will
                    // allow us to identify the wire with a hit that is closest 
                    // to the wire that the track projected closest to. (clear?)
                    // For the value of the array use a pointer to the actual DFMWPCHit
                    // object. This makes it easier to add the ones we use as associated
                    // objects later.
                    vector<const DFMWPCHit *> wires_hit(144, nullptr);
                    int min_delta_wire = 1E6;
                    for (auto hit: fmwpchits) {
                        if (hit->layer != layer) continue;
//                        _DBG_<<"  hit->t="<<hit->t<<" proj.t="<<proj.t<<" delta_t="<<fabs(hit->t - proj.t)<<" MIN_DELTA_T_FMWPC_PROJECTION="<<MIN_DELTA_T_FMWPC_PROJECTION<<endl;
                        if (fabs(hit->t - proj.t) > MIN_DELTA_T_FMWPC_PROJECTION) continue;
                        uint32_t idx = hit->wire - 1;
                        if( idx < 144) {
                            wires_hit[idx] = hit;
                        }else{
                            _DBG_ << " FMWPC wire index out of range! " << hit->wire << " should be between 1 and 144 inclusive!" << std::endl;
                        }

                        // keep track of which hit wire is closest to track projection
                        int delta_wire = abs(hit->wire - wire_trk_proj);
                        if (delta_wire < min_delta_wire) {
                            min_delta_wire = delta_wire;
//                            _DBG_<<"  fmpwc_mt->FMWPC_closest_wire[layer="<<layer<<" - 1]=" << hit->wire << endl;
                            fmpwc_mt->FMWPC_closest_wire[layer - 1] = hit->wire;
                            fmpwc_mt->FMWPC_dist_closest_wire[layer - 1] = min_delta_wire;
                        }
                    }

                    // Count the number of consecutive wires hit including 
                    // the one closest to the projection. First count those
                    // with smaller wire numbers and then those with larger
                    // wire numbers.
//                    _DBG_<<"  -- fmpwc_mt->FMWPC_closest_wire[layer="<<layer<<" - 1] = " << fmpwc_mt->FMWPC_closest_wire[layer - 1] <<endl;
                    if ((fmpwc_mt->FMWPC_closest_wire[layer - 1] >= 1) && (fmpwc_mt->FMWPC_closest_wire[layer - 1] <= 144)) {
                        for (int i = fmpwc_mt->FMWPC_closest_wire[layer - 1];
                             i >= 1; i--) { // before and including closest wire
//                            _DBG_<<"    --- i="<<i<<" wires_hit[i-1]=" <<wires_hit[i-1]<<endl;
                            if (wires_hit[i-1] == nullptr) break;
                            fmpwc_mt->FMWPC_Nhits_cluster[layer - 1]++;
                            fmpwc_mt->AddAssociatedObject(wires_hit[i-1]);
                        }
                        for (int i = fmpwc_mt->FMWPC_closest_wire[layer - 1] + 1;
                             i <= 144; i++) { // after, but not including closest wire
//                            _DBG_<<"    --- i="<<i<<" wires_hit[i-1]=" <<wires_hit[i-1]<<endl;
                            if (wires_hit[i-1] == nullptr) break;
                            fmpwc_mt->FMWPC_Nhits_cluster[layer - 1]++;
                            fmpwc_mt->AddAssociatedObject(wires_hit[i-1]);
                        }
                    }
                }
            }
        
        }catch(...){
            // We get here if there are no extrapolations to the FMWPC
        }

        // Publish DFMWPCMatchedTrack by pushing onto _data
        _data.push_back(fmpwc_mt);
    }

	return NOERROR;
}

//------------------
// erun
//------------------
jerror_t DFMWPCMatchedTrack_factory::erun(void)
{
	return NOERROR;
}

//------------------
// fini
//------------------
jerror_t DFMWPCMatchedTrack_factory::fini(void)
{
	return NOERROR;
}

