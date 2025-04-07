// $Id$
//
//    File: DTRDHit_factory.cc
//

#include <iostream>
#include <iomanip>
using namespace std;

#include "DANA/DEvent.h"
#include <DAQ/Df125Config.h>
#include <DAQ/Df125FDCPulse.h>
#include "TRD/DTRDHit_factory.h"

static bool DTRDHit_cmp(const DTRDHit* a, const DTRDHit* b) {
  if (a->plane==b->plane){
    return a->t < b->t;
  }
  return a->plane < b->plane;
}



//------------------
// Init
//------------------
void DTRDHit_factory::Init()
{
	// Initialize calibration tables
	//vector<double> new_t0s(TRD_MAX_CHANNELS);	
	//time_offsets.push_back(new_t0s); time_offsets.push_back(new_t0s);

	auto app = GetApplication();

	IS_XY_TIME_DIFF_CUT = false;
	app->SetDefaultParameter("TRD:IS_XY_TIME_DIFF_CUT", IS_XY_TIME_DIFF_CUT, 
			      "Apply time difference cut between X and Y hits (default: false)");

	XY_TIME_DIFF = 40.;
	app->SetDefaultParameter("TRD:XY_TIME_DIFF", XY_TIME_DIFF, 
			      "Time difference between hits in X and Y planes to be considered a coincidence (default: 40.)");
	
	HIT_SIZE_MAX = 100000.;
    app->SetDefaultParameter("TRD:HIT_SIZE_MAX", HIT_SIZE_MAX, 
                  "Maximum number of hits on a strip in either X or Y in an event, for noise cleaning (default: 100000.)");
	
	CDIFF_MIN = 3;
    app->SetDefaultParameter("TRD:CDIFF_MIN", CDIFF_MIN, 
                  "Maximum circular difference between phases for X or Y hits (default: 3)");
	
	FREQ_COUNT = 10;
    app->SetDefaultParameter("TRD:FREQ_COUNT", FREQ_COUNT,
                  "Maximum counts of a frequency occurring for a hit in either X or Y (default: 10)");
	
	// Setting this flag makes it so that JANA does not delete the objects in _data.
	// This factory will manage this memory.
	SetFactoryFlag(NOT_OBJECT_OWNER);  // TODO: Make sure we don't need PERSISTENT as well
  	
  	return;
}

//------------------
// BeginRun
//------------------
void DTRDHit_factory::BeginRun(const std::shared_ptr<const JEvent>& event)
{

    return;
}

//------------------
// Process
//------------------
void DTRDHit_factory::Process(const std::shared_ptr<const JEvent>& event)
{
    vector<const DTRDHit*> hits;
    event->Get(hits, "Calib");
    
    if(!IS_XY_TIME_DIFF_CUT) {
    	for(auto &hit : hits)
    		Insert(const_cast<DTRDHit*>(hit));
    	return;
    }

	vector<const DTRDHit*> hits_plane[2]; // one for each plane
	vector<const DTRDHit*> hits_pamp[2][10]; // one for each pre-amp board
    
	// Sort hits by layer number and by time
	sort(hits.begin(),hits.end(),DTRDHit_cmp);
	
	// Sift through all hits and select out X and Y hits.
	for (vector<const DTRDHit*>::iterator i = hits.begin(); i != hits.end(); ++i) {
		// sort hits
		int stripPlane = (*i)->plane-1;
		if( (stripPlane<0) || (stripPlane>=2) ) { // only two planes
			static int Nwarn = 0;
			if( Nwarn<10 ){
				jerr << " stripPlane is outside of array bounds!! stripPlane="<< stripPlane << std::endl;
				if( ++Nwarn==10 )jerr << " LAST WARNING!" << std::endl;
			}
			continue;
		}
		hits_plane[stripPlane].push_back(*i);
	}
	
	int nfreq=17; //Noise band frequency
	
	// loops to check the time coincidence of hits in the two planes and add them to the _data vector
	if (IS_XY_TIME_DIFF_CUT) {
		float phase[2][10][20]={0};
		for (unsigned int i=0; i < hits_plane[0].size(); i++) {
			for (unsigned int j=0; j < hits_plane[1].size(); j++) {
				if (abs(hits_plane[0][i]->t - hits_plane[1][j]->t) < XY_TIME_DIFF) {
					if (hits_plane[0].size()>HIT_SIZE_MAX) continue;
					//Determine which pre-amp the hit is in
					int pamp=(hits_plane[0][i]->strip)/120; //120 strips per pre-amp
					hits_pamp[0][pamp].push_back(hits_plane[0][i]);
					int ph=((int)hits_plane[0][i]->t/8)%nfreq; //signal phase
					phase[0][pamp][ph]+=1;
					//Insert(const_cast<DTRDHit*>(hits_plane[0][i]));
					break;
				}
			}
		}
		
		for (unsigned int i=0; i < hits_plane[1].size(); i++) {
			for (unsigned int j=0; j < hits_plane[0].size(); j++) {
				if (abs(hits_plane[1][i]->t - hits_plane[0][j]->t) < XY_TIME_DIFF) {
					if (hits_plane[1].size()>HIT_SIZE_MAX) continue;
					//Determine which pre-amp the hit is in
                    int pamp=(hits_plane[1][i]->strip)/120; //120 strips per pre-amp
                    hits_pamp[1][pamp].push_back(hits_plane[1][i]);
                    int ph=((int)hits_plane[1][i]->t/8)%nfreq; //signal phase
                    phase[1][pamp][ph]+=1;
					//Insert(const_cast<DTRDHit*>(hits_plane[1][i]));
					break;
				}
			}
		}
		
		int fmax[2][10]={-1}; //freq max
		int amax[2][10]={0}; //amplitude max
		
		for (int iplane=0; iplane<2; iplane++) {
			for (int ipamp=0; ipamp<10; ipamp++) {
				for (int i=0; i<nfreq; i++) {
					int fp=phase[iplane][ipamp][i]; //frequency of the phase...?
					if (fp>amax[iplane][ipamp]) {
						amax[iplane][ipamp]=fp;
						fmax[iplane][ipamp]=i;
					}
				}
			}
		}
		
		for (int iplane=0; iplane<2; iplane++) {
            for (int ipamp=0; ipamp<10; ipamp++) {
				for (unsigned int i=0; i<hits_pamp[iplane][ipamp].size(); i++) {
					int ph=((int)hits_pamp[iplane][ipamp][i]->t/8)%nfreq; //signal phase
					int diff=abs(ph-fmax[iplane][ipamp]);
					int cdiff=std::min(diff,nfreq-diff); //circular difference
					if (amax[iplane][ipamp]<FREQ_COUNT || cdiff>CDIFF_MIN) {
						Insert(const_cast<DTRDHit*>(hits_pamp[iplane][ipamp][i]));
					}
				}
            }
        }
		
	} //End IS_XY_TIME_DIFF_CUT condition
		    
    return;
}

//------------------
// EndRun
//------------------
void DTRDHit_factory::EndRun()
{
}

//------------------
// Finish
//------------------
void DTRDHit_factory::Finish()
{
}
