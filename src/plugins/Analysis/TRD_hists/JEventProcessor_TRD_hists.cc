#include <stdint.h>
#include <vector>

#include "JEventProcessor_TRD_hists.h"
#include <JANA/JApplication.h>

using namespace std;
using namespace jana;

#include <DANA/DApplication.h>

#include <TRIGGER/DL1Trigger.h>
#include <TRD/DTRDDigiHit.h>
#include <TRD/DTRDHit.h>
#include <TRD/DTRDStripCluster.h>
#include <TRD/DTRDPoint.h>
#include <PID/DChargedTrack.h>

#include <TDirectory.h>
#include <TH1.h>
#include <TH2.h>
#include <TProfile.h>

// root hist pointers
// fix constants for now
static const int NTRDplanes = 2;
static const int NTRD_xstrips = 720;
static const int NTRD_ystrips = 432;

static TH1I *trd_num_events;
static TH1I *num_tracks;

static TH2I *hTRDPoint_TrackX, *hTRDPoint_TrackY;
static TH2I *hTRDPoint_DeltaXY, *hTRDPoint_DeltaXY_Pos, *hTRDPoint_DeltaXY_Neg;
static TH1I *hTRDPoint_Time, *hTRDMatchedTrack_EP;

//----------------------------------------------------------------------------------


// Routine used to create our JEventProcessor
extern "C"{
    void InitPlugin(JApplication *app){
        InitJANAPlugin(app);
        app->AddProcessor(new JEventProcessor_TRD_hists());
    }
}


//----------------------------------------------------------------------------------


JEventProcessor_TRD_hists::JEventProcessor_TRD_hists() {
}


//----------------------------------------------------------------------------------


JEventProcessor_TRD_hists::~JEventProcessor_TRD_hists() {
}


//----------------------------------------------------------------------------------

jerror_t JEventProcessor_TRD_hists::init(void) {

    // create root folder for TRD and cd to it, store main dir
    TDirectory *mainDir = gDirectory;
    TDirectory *trdDir = gDirectory->mkdir("TRD_hists");
    trdDir->cd();
    // book hists
    trd_num_events = new TH1I("trd_hists_num_events","TRD number of events",1,0.5,1.5);
    num_tracks = new TH1I("num_tracks", "; # of straight (0) and time based (1) tracks",2,-0.5,1.5);

    trdDir->cd();

    // Straight track histograms...
    gDirectory->mkdir("StraightTracks")->cd();

    hTRDPoint_TrackX = new TH2I("WireTRDPoint_TrackX","; Wire TRD X (cm); Extrapolated track X (cm)",200,-55,55,200,-55,55);
    hTRDPoint_TrackY = new TH2I("WireTRDPoint_TrackY","; Wire TRD Y (cm); Extrapolated track Y (cm)",200,-85,-65,200,-85,-65);
    hTRDPoint_DeltaXY = new TH2I("WireTRDPoint_DeltaXY","; #Delta X (cm); #Delta Y (cm)",100,-5,5,100,-5,5);
    hTRDPoint_DeltaXY_Pos = new TH2I("WireTRDPoint_DeltaXY_Pos","; #Delta X (cm); #Delta Y (cm)",100,-5,5,100,-5,5);
    hTRDPoint_DeltaXY_Neg = new TH2I("WireTRDPoint_DeltaXY_Neg","; #Delta X (cm); #Delta Y (cm)",100,-5,5,100,-5,5);
    hTRDPoint_Time = new TH1I("TRDPoint_Time","; hit time",1000,0,1000);
    hTRDMatchedTrack_EP = new TH1I("TRDMatchedTrack_EP", "; E/p",150,0.,1.5);

    // back to main dir
    mainDir->cd();

    return NOERROR;
}

//----------------------------------------------------------------------------------


jerror_t JEventProcessor_TRD_hists::brun(JEventLoop *eventLoop, int32_t runnumber) {
    // This is called whenever the run number changes

    DApplication* dapp = dynamic_cast<DApplication*>(eventLoop->GetJApplication());
    const DGeometry *geom = dapp->GetDGeometry(runnumber);
    vector<double> z_trd;
    geom->GetTRDZ(z_trd);

    const DMagneticFieldMap *bfield = dapp->GetBfield(runnumber);
    dIsNoFieldFlag = ((dynamic_cast<const DMagneticFieldMapNoField*>(bfield)) != NULL);

    return NOERROR;
}


//----------------------------------------------------------------------------------


jerror_t JEventProcessor_TRD_hists::evnt(JEventLoop *eventLoop, uint64_t eventnumber) {
    // This is called for every event. Use of common resources like writing
    // to a file or filling a histogram should be mutex protected. Using
    // loop-Get(...) to get reconstructed objects (and thereby activating the
    // reconstruction algorithm) should be done outside of any mutex lock
    // since multiple threads may call this method at the same time.

/*
    // Get trigger words and filter on PS trigger (if it exists?)
    const DL1Trigger *trig_words = nullptr;
    uint32_t trig_mask, fp_trig_mask;
    try {
        eventLoop->GetSingle(trig_words);
    } catch(...) {};
    if (trig_words != nullptr) {
        trig_mask = trig_words->trig_mask;
        fp_trig_mask = trig_words->fp_trig_mask;
    }
    else {
        trig_mask = 0;
        fp_trig_mask = 0;
    }
    int trig_bits = fp_trig_mask > 0 ? 10 + fp_trig_mask:trig_mask;
    // Select PS-triggered events
    if (trig_bits != 8) {
        return NOERROR;
    }
*/

     vector<const DTRDDigiHit*> digihits;
     eventLoop->Get(digihits);
//     vector<const DTRDHit*> hits;
//     eventLoop->Get(hits);
//     vector<const DTRDStripCluster*> clusters;
//     eventLoop->Get(clusters);
    vector<const DTRDPoint*> points;
    eventLoop->Get(points);

    vector<const DTrackWireBased*> straight_tracks;
    vector<const DChargedTrack*> tracks;
    if (dIsNoFieldFlag)
	    eventLoop->Get(straight_tracks, "StraightLine");
    else
	    eventLoop->Get(tracks);

    num_tracks->Fill(0.,(double)straight_tracks.size());
    num_tracks->Fill(1.,(double)tracks.size());

    // FILL HISTOGRAMS
    // Since we are filling histograms local to this plugin, it will not interfere with other ROOT operations: can use plugin-wide ROOT fill lock
    japp->RootFillLock(this); //ACQUIRE ROOT FILL LOCK

    ///////////////////////////
    // TRD DigiHits and Hits //
    ///////////////////////////

    if (digihits.size() > 0) trd_num_events->Fill(1);

    // check if have good extrapolated track with TRD wire point
    bool goodTrack = false;

    /////////////////////////////
    // Straight track analysis //
    /////////////////////////////

    for (const auto& straight_track : straight_tracks) {
	    vector<DTrackFitter::Extrapolation_t>extrapolations=straight_track->extrapolations.at(SYS_TRD);
	    //cout<<"found straight track with "<<extrapolations.size()<<" extrapolations"<<endl;

	    for (const auto& extrapolation : extrapolations) {
	
		    // correlate TRD with extrapolated tracks
		    for (const auto& point : points) {

			    if(point->detector == 0 && fabs(extrapolation.position.Z() - 548.8) < 5.) {  // update this based on real geometry

				    hTRDPoint_TrackX->Fill(point->x, extrapolation.position.X());
				    hTRDPoint_TrackY->Fill(point->y, extrapolation.position.Y());
				    
				    double locDeltaX = point->x - extrapolation.position.X();
				    double locDeltaY = point->y - extrapolation.position.Y();
				    hTRDPoint_DeltaXY->Fill(locDeltaX, locDeltaY);

				    if(fabs(locDeltaX) < 5. && fabs(locDeltaY) < 5.) {   // update matching
					    hTRDPoint_Time->Fill(point->time);
					    goodTrack = true;
				    }
			    }
		    }
		}

    }

    /////////////////////////////
    // Field on track analysis //
    /////////////////////////////

    for (const auto& track : tracks) {
    	const DChargedTrackHypothesis* locChargedTrackHypothesis = track->Get_BestTrackingFOM();
    	// make cuts based on locChargedTrackHypothesis->PID() ?
    	auto locTrackTimeBased = locChargedTrackHypothesis->Get_TrackTimeBased();
    	
	    int locCharge = locChargedTrackHypothesis->charge();
    	Particle_t locPID = locChargedTrackHypothesis->PID();
		double locP = locTrackTimeBased->momentum().Mag();
		double locTheta = locTrackTimeBased->momentum().Theta()*180.0/TMath::Pi();
    	
    	auto locFCALShowerMatchParams = locChargedTrackHypothesis->Get_FCALShowerMatchParams();
    	auto locTOFHitMatchParams = locChargedTrackHypothesis->Get_TOFHitMatchParams();

	    vector<DTrackFitter::Extrapolation_t>extrapolations=locTrackTimeBased->extrapolations.at(SYS_TRD);
	    //cout<<"found straight track with "<<extrapolations.size()<<" extrapolations"<<endl;

	    for (const auto& extrapolation : extrapolations) {
	
		    // correlate TRD points with extrapolated tracks
		    // this might change to TRD track segments?
		    for (const auto& point : points) {

			    if(point->detector == 0 && fabs(extrapolation.position.Z() - 548.8) < 5.) { // update this based on real geometry

				    hTRDPoint_TrackX->Fill(point->x, extrapolation.position.X());
				    hTRDPoint_TrackY->Fill(point->y, extrapolation.position.Y());
				    
				    double locDeltaX = point->x - extrapolation.position.X();
				    double locDeltaY = point->y - extrapolation.position.Y();
				    hTRDPoint_DeltaXY->Fill(locDeltaX, locDeltaY);
				    if(locCharge > 0) hTRDPoint_DeltaXY_Pos->Fill(locDeltaX, locDeltaY);
				    else hTRDPoint_DeltaXY_Neg->Fill(locDeltaX, locDeltaY);

				    if(fabs(locDeltaX) < 5. && fabs(locDeltaY) < 5.) {   // update matching
					    hTRDPoint_Time->Fill(point->time);
					    goodTrack = true;
					    
					    if(locFCALShowerMatchParams != NULL) {
							const DFCALShower* locFCALShower = locFCALShowerMatchParams->dFCALShower;
							double locEOverP = locFCALShower->getEnergy()/locP;

							hTRDMatchedTrack_EP->Fill(locEOverP);		    
					    }
			
				    }
			    }
		    }

	    }
    }

    ///////////////////////////
    // Plots for good tracks //
    ///////////////////////////

//     if(goodTrack) {
// 	    for (const auto& hit : hits) {
// 		    if(hit->plane != 0 && hit->plane != 4) continue; // only Wire TRD
// 		    int wire = hit->strip;
// 
// 		    // GEM TRD hits
// 		    for (const auto& gemtrd_hit : hits) {
// 			    if(gemtrd_hit->plane != 2 && gemtrd_hit->plane != 6) continue; 
// 			    double locDeltaT = gemtrd_hit->t - hit->t;
// 			    
// 			    hWire_GEMTRDX_DeltaT->Fill(locDeltaT, gemtrd_hit->pulse_height);
// 			   
// 			    //if(fabs(locDeltaT) < 20.)
// 			    hWire_GEMTRDXstrip->Fill(wire, gemtrd_hit->strip);
// 		    }
// 	    }
//     }
// 
    japp->RootFillUnLock(this); //RELEASE ROOT FILL LOCK

    return NOERROR;
}
//----------------------------------------------------------------------------------


jerror_t JEventProcessor_TRD_hists::erun(void) {
    // This is called whenever the run number changes, before it is
    // changed to give you a chance to clean up before processing
    // events from the next run number.
    return NOERROR;
}


//----------------------------------------------------------------------------------


jerror_t JEventProcessor_TRD_hists::fini(void) {
    // Called before program exit after event processing is finished.
    return NOERROR;
}

//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
