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

#include <FCAL/DFCALHit.h>
#include <FCAL/DFCALGeometry.h>
#include <FMWPC/DFMWPCHit.h>
#include <FMWPC/DFMWPCCluster.h>
#include <FMWPC/DCTOFPoint.h>
#include <PID/DChargedTrack.h>
#include <DANA/DEvent.h>


//------------------
// Init
//------------------
void DFMWPCMatchedTrack_factory::Init()
{

    // These need to be promoted to either JANA config. parameters
    // or calib constants.
    MIN_DELTA_T_FCAL_PROJECTION  = 100.0; // min. time between track projection and FCAL hit to consider them matched
    MIN_DELTA_T_FMWPC_PROJECTION = 100.0; // min. time between track projection and FMWPC hit to consider them matched
    FMWPC_WIRE_SPACING           = 1.016; // distance between wires in FMWPC in cm
}

//------------------
// brun
//------------------
void DFMWPCMatchedTrack_factory::BeginRun(const std::shared_ptr<const JEvent> &event)
{
    // Get pointer to DGeometry object
    dgeom  = DEvent::GetDGeometry(event);

    // Get x and y offsets for each chamber.
    if (!dgeom->GetFMWPCXY_vec(xvec, yvec)){
        xvec = {0.0,0.0,0.0,0.0,0.0,0.0};
        yvec = {0.0,0.0,0.0,0.0,0.0,0.0};
    }

    // Get the FMWPC wire spacing in cm (should be 1.016)
    dgeom->GetFMWPCWireSpacing( FMWPC_WIRE_SPACING );

    // Get the FMWPC wire orientation (should be vertical, horizontal, ...)
    dgeom->GetFMWPCWireOrientation( fmwpc_wire_orientation );
}

//------------------
// Process
//------------------
void DFMWPCMatchedTrack_factory::Process(const std::shared_ptr<const JEvent> &event)
{
    vector<const DTrackTimeBased*> tbts;
    vector<const DFCALHit*>        fcalhits;
    vector<const DFMWPCHit*>       fmwpchits;
    vector<const DFMWPCCluster*>   fmwpcclusters;
    vector<const DCTOFPoint*>      ctofpoints;
    vector<const DChargedTrack*>   chargedtracks;
    const DFCALGeometry*           fcalgeom = nullptr;
    event->Get(tbts);
    event->Get(fcalhits);
    event->Get(fmwpchits);
    event->Get(fmwpcclusters);
    event->Get(chargedtracks);
    event->Get(ctofpoints);
    event->GetSingle( fcalgeom );

    // Make sure we found a DFCALGeometry object
    if( ! fcalgeom ){
        _DBG_ << " Missing DFCALGeometry!!" << endl;
        return; // NOERROR; // should this be a fatal error?
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

        // Find the DChargedTrack object that has this as one of its hypotheses
        for( auto ct : chargedtracks ){
            for( auto hypoth : ct->dChargedTrackHypotheses ){
                if( hypoth->Get_TrackTimeBased() == tbt ){
                    // Found the DChargedTrack that this DTrackTimeBased belongs to.
                    // Add matched objects from that as associated objects to this
                    // DFMWPCMatchedTrack
                    fmpwc_mt->AddAssociatedObject( ct );
                    fmpwc_mt->AddAssociatedObject( hypoth );
                    auto fcalmatch = hypoth->Get_FCALShowerMatchParams();
                    if( fcalmatch ){
                        fmpwc_mt->AddAssociatedObject( fcalmatch->dFCALShower );
                    }
                    break;
                }
            }
        }

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

                // Convert into local coordinates so we can work with wire numbers
                auto orientation = fmwpc_wire_orientation[layer-1];
                auto xoffset = xvec[layer-1];
                auto yoffset = yvec[layer-1];
                auto xpos = xoffset + proj.position.x();
                auto ypos = yoffset + proj.position.y();
                double s = orientation==DGeometry::kFMWPC_WIRE_ORIENTATION_VERTICAL ? xpos:ypos;
                int wire_trk_proj = round(71.5 + s/FMWPC_WIRE_SPACING) + 1; // 1-144

                // If the projection is outside of the wire range then bail now
                if( (wire_trk_proj<1) || (wire_trk_proj>144) ) continue;

                // Loop over DFMWPCClusters and find closest match to this projection
                int min_dist = 1000000;
                const DFMWPCCluster* closest_fmwpc_cluster= nullptr;
                for(auto fmwpccluster : fmwpcclusters){
                    if( fmwpccluster->layer != layer ) continue;

                    int dist;
                    if( wire_trk_proj >= fmwpccluster->first_wire ){
                        dist = wire_trk_proj - fmwpccluster->last_wire; // distance beyond last wire (will be negative if inside cluster)
                        if( dist < 0 ) dist = 0; // force dist to 0 if inside cluster
                    }else{
                        dist = fmwpccluster->first_wire - wire_trk_proj; // distance before first wire
                    }

                    if( dist < min_dist ){
                        min_dist = dist;
                        closest_fmwpc_cluster = fmwpccluster;
                    }
                }

                // If a DFMWPCCluster was found, add it as an associated object
                // and fill in relevant fields of matched track
                if( closest_fmwpc_cluster ){
                    fmpwc_mt->AddAssociatedObject( closest_fmwpc_cluster );
                    fmpwc_mt->FMWPC_dist_closest_wire[layer-1] = min_dist;
                    fmpwc_mt->FMWPC_Nhits_cluster[layer-1] = closest_fmwpc_cluster->Nhits;
                    if( wire_trk_proj < closest_fmwpc_cluster->first_wire ) {
                        fmpwc_mt->FMWPC_closest_wire[layer - 1] = closest_fmwpc_cluster->first_wire;
                    }else if(wire_trk_proj > closest_fmwpc_cluster->last_wire){
                        fmpwc_mt->FMWPC_closest_wire[layer - 1] = closest_fmwpc_cluster->last_wire;
                    }else{
                        fmpwc_mt->FMWPC_closest_wire[layer - 1] = wire_trk_proj;
                    }
                }
            }

            // Match to CTOF


        }catch(...){
            // We get here if there are no extrapolations to the FMWPC
        }

        // Publish DFMWPCMatchedTrack by pushing onto _data
        Insert(fmpwc_mt);
    }
}

//------------------
// EndRun
//------------------
void DFMWPCMatchedTrack_factory::EndRun()
{
}

//------------------
// Finish
//------------------
void DFMWPCMatchedTrack_factory::Finish()
{
}

