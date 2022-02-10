// $Id$
//
//    File: DFMWPCMatchedTrack.h
// Created: Sat Jan 22 08:53:49 EST 2022
// Creator: davidl (on Linux ifarm1901.jlab.org 3.10.0-1160.11.1.el7.x86_64 x86_64)
//

#ifndef _DFMWPCMatchedTrack_
#define _DFMWPCMatchedTrack_

#include <JANA/JObject.h>
#include <JANA/JFactory.h>

#include <TRACKING/DTrackTimeBased.h>

class DFMWPCMatchedTrack:public jana::JObject{
	public:
		JOBJECT_PUBLIC(DFMWPCMatchedTrack);
        
        DFMWPCMatchedTrack(const DTrackTimeBased *tbt):tbt(tbt),FCAL_E_center(0.0),FCAL_E_3x3(0.0),FCAL_E_5x5(0.0){
            for(int i=0; i<6; i++){
                FMWPC_closest_wire[i]      = -1000;
                FMWPC_dist_closest_wire[i] = 1.0E6;
                FMWPC_Nhits_cluster[i]     = 0;
            }
        }

        const DTrackTimeBased *tbt;         
        
        double FCAL_E_center;            // Energy of FCAL hit in block where track is projected (0 if block not hit)
        double FCAL_E_3x3;               // Sum of FCAL hits in 3x3 grid surrounding block where track is projected
        double FCAL_E_5x5;               // Sum of FCAL hits in 5x5 grid surrounding block where track is projected
        
        int    FMWPC_closest_wire[6];      // The closest wire in each FMWPC plane to the projected track position
        double FMWPC_dist_closest_wire[6]; // DOCA of closest wire in units of wire spacing (NOT cm!)
        int    FMWPC_Nhits_cluster[6];     // Number of adjacent wires in cluster that FMWPC_closest_wire is in 


        // Convenience methods
        Particle_t PID(void) const { return tbt->PID(); }
        DVector3 GetPosFCAL(void) const;       // Projected track position at FCAL front face
        DVector3 GetPosFMWPC(int layer)const ; // Projected track position at specified FMWPC wire layer (1-6)
		
		// This method is used primarily for pretty printing
		// the second argument to AddString is printf style format
		void toStrings(vector<pair<string,string> > &items)const{
            AddString(items, "PID", "%s", ParticleType(PID()));
			AddString(items, "FCAL_E_center", "%3.1f", FCAL_E_center);
			AddString(items, "FCAL_E_3x3", "%3.1f", FCAL_E_3x3);
			AddString(items, "FCAL_E_5x5", "%3.1f", FCAL_E_5x5);
			AddString(items, "FMWPC_Nhits_cluster1", "%d", FMWPC_Nhits_cluster[0]);
			AddString(items, "FMWPC_Nhits_cluster2", "%d", FMWPC_Nhits_cluster[1]);
			AddString(items, "FMWPC_Nhits_cluster3", "%d", FMWPC_Nhits_cluster[2]);
			AddString(items, "FMWPC_Nhits_cluster4", "%d", FMWPC_Nhits_cluster[3]);
			AddString(items, "FMWPC_Nhits_cluster5", "%d", FMWPC_Nhits_cluster[4]);
			AddString(items, "FMWPC_Nhits_cluster6", "%d", FMWPC_Nhits_cluster[5]);
		}
		
};

#endif // _DFMWPCMatchedTrack_

