#include <stdint.h>
#include <vector>

#include "JEventProcessor_TRD_hists.h"
#include <JANA/JApplication.h>

using namespace std;

#include <DANA/DEvent.h>

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

static TH1I *trd_num_events;
static TH1I *num_tracks;

static TH2I *hTRDPoint_TrackX, *hTRDPoint_TrackY, *hTRDPoint_DeltaXY;
static TH2I *hTRDPoint_DeltaX_T;
static TH1I *hTRDPoint_Time;

static TH2I *hTRDPoint_TrackX_EPCut, *hTRDPoint_TrackY_EPCut, *hTRDPoint_DeltaXY_EPCut;
static TH2I *hTRDPoint_DeltaX_T_EPCut;
static TH1I *hTRDPoint_Time_EPCut;

static TH1I *hTRDMatchedTrack_EP;


//----------------------------------------------------------------------------------


// Routine used to create our JEventProcessor
extern "C"{
    void InitPlugin(JApplication *app){
        InitJANAPlugin(app);
        app->Add(new JEventProcessor_TRD_hists());
    }
}


//----------------------------------------------------------------------------------


JEventProcessor_TRD_hists::JEventProcessor_TRD_hists() {
	SetTypeName("JEventProcessor_TRD_hists");
}


//----------------------------------------------------------------------------------


JEventProcessor_TRD_hists::~JEventProcessor_TRD_hists() {
}


//----------------------------------------------------------------------------------

void JEventProcessor_TRD_hists::Init() {

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


    // GEM TRD
    hTRDPoint_TrackX = new TH2I("TRDPoint_TrackX","; GEM TRD X; Extrapolated track X",200,-55,55,200,-55,55);
    hTRDPoint_TrackY = new TH2I("TRDPoint_TrackY","; GEM TRD Y; Extrapolated track Y",200,-85,-65,200,-85,-65);
    hTRDPoint_DeltaXY = new TH2I("TRDPoint_DeltaXY","; #Delta X (cm); #Delta Y (cm)",100,-5,5,100,-5,5);
    hTRDPoint_DeltaX_T = new TH2I("TRDPoint_DeltaX_T","; #Delta X; hit time",1000,-20,20,1000,0,1000);
	hTRDPoint_Time = new TH1I("TRDPoint_Time","; hit time",1000,0,1000);

    hTRDPoint_TrackX_EPCut = new TH2I("TRDPoint_TrackX_EPCut","; GEM TRD X; Extrapolated track X",200,-55,55,200,-55,55);
    hTRDPoint_TrackY_EPCut = new TH2I("TRDPoint_TrackY_EPCut","; GEM TRD Y; Extrapolated track Y",200,-85,-65,200,-85,-65);
    hTRDPoint_DeltaXY_EPCut = new TH2I("TRDPoint_DeltaXY_EPCut","; #Delta X (cm); #Delta Y (cm)",100,-5,5,100,-5,5);
    hTRDPoint_DeltaX_T_EPCut = new TH2I("TRDPoint_DeltaX_T_EPCut","; #Delta X; hit time",1000,-20,20,1000,0,1000);
	hTRDPoint_Time_EPCut = new TH1I("TRDPoint_Time_EPCut","; hit time",1000,0,1000);

    // back to main dir
    mainDir->cd();
}

//----------------------------------------------------------------------------------


void JEventProcessor_TRD_hists::BeginRun(const std::shared_ptr<const JEvent>& event) {
    // This is called whenever the run number changes

    //auto runnumber = event->GetRunNumber();

    const DGeometry *geom = DEvent::GetDGeometry(event);

    vector<double> z_trd;
    geom->GetTRDZ(z_trd);

    const DMagneticFieldMap *bfield = DEvent::GetBfield(event);
    dIsNoFieldFlag = ((dynamic_cast<const DMagneticFieldMapNoField*>(bfield)) != NULL);
}


//----------------------------------------------------------------------------------


void JEventProcessor_TRD_hists::Process(const std::shared_ptr<const JEvent>& event) {
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
        event->GetSingle(trig_words);
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
        return;
    }
*/

    vector<const DTRDPoint*> points;
    event->Get(points);

    vector<const DTrackWireBased*> straight_tracks;
    vector<const DChargedTrack*> tracks;
    if (dIsNoFieldFlag)
	    event->Get(straight_tracks, "StraightLine");
    else
	    event->Get(tracks);

    num_tracks->Fill(0.,(double)straight_tracks.size());
    num_tracks->Fill(1.,(double)tracks.size());

    // FILL HISTOGRAMS
    // Since we are filling histograms local to this plugin, it will not interfere with other ROOT operations: can use plugin-wide ROOT fill lock
	DEvent::GetLockService(event)->RootFillLock(this); //ACQUIRE ROOT FILL LOCK

    ///////////////////////////
    // TRD points //
    ///////////////////////////

    if (points.size() > 0) trd_num_events->Fill(1);

    // check if have good extrapolated track with TRD wire point
    bool goodTrack = false;

    /////////////////////////////
    // Straight track analysis //
    /////////////////////////////

    for (const auto& straight_track : straight_tracks) {
	    vector<DTrackFitter::Extrapolation_t>extrapolations=straight_track->extrapolations.at(SYS_TRD);
	    //cout<<"found straight track with "<<extrapolations.size()<<" extrapolations"<<endl;

	    for (const auto& extrapolation : extrapolations) {
	
		    // correlate  TRD with extrapolated tracks
		    for (const auto& point : points) {

				hTRDPoint_TrackX->Fill(point->x, extrapolation.position.X());
				hTRDPoint_TrackY->Fill(point->y, extrapolation.position.Y());
				
				double locDeltaX = point->x - extrapolation.position.X();
				double locDeltaY = point->y - extrapolation.position.Y();
				hTRDPoint_DeltaXY->Fill(locDeltaX, locDeltaY);
				hTRDPoint_DeltaX_T->Fill(locDeltaX, point->time);

				if(fabs(locDeltaX) < 5. && fabs(locDeltaY) < 5.) {
					hTRDPoint_Time->Fill(point->time);
					goodTrack = true;
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
    	
	    //int locCharge = locChargedTrackHypothesis->charge();
    	//Particle_t locPID = locChargedTrackHypothesis->PID();
		double locP = locTrackTimeBased->momentum().Mag();
		//double locTheta = locTrackTimeBased->momentum().Theta()*180.0/TMath::Pi();
    	
    	auto locFCALShowerMatchParams = locChargedTrackHypothesis->Get_FCALShowerMatchParams();
    	auto locTOFHitMatchParams = locChargedTrackHypothesis->Get_TOFHitMatchParams();

	    //int charge = locChargedTrackHypothesis->charge();
	    vector<DTrackFitter::Extrapolation_t>extrapolations=locTrackTimeBased->extrapolations.at(SYS_TRD);
	    //cout<<"found straight track with "<<extrapolations.size()<<" extrapolations"<<endl;

	    for (const auto& extrapolation : extrapolations) {
	
		    // correlate  TRD with extrapolated tracks
		    for (const auto& point : points) {

				hTRDPoint_TrackX->Fill(point->x, extrapolation.position.X());
				hTRDPoint_TrackY->Fill(point->y, extrapolation.position.Y());
				
				double locDeltaX = point->x - extrapolation.position.X();
				double locDeltaY = point->y - extrapolation.position.Y();
				hTRDPoint_DeltaXY->Fill(locDeltaX, locDeltaY);
				hTRDPoint_DeltaX_T->Fill(locDeltaX, point->time);

				if(fabs(locDeltaX) < 5. && fabs(locDeltaY) < 5.) {
					hTRDPoint_Time->Fill(point->time);
					goodTrack = true;
				}
				
				// select electrons
				if(locFCALShowerMatchParams != NULL) {
					const DFCALShower* locFCALShower = locFCALShowerMatchParams->dFCALShower;
					double locEOverP = locFCALShower->getEnergy()/locP;

					hTRDMatchedTrack_EP->Fill(locEOverP);		
					
					if(locEOverP > 0.9 && locEOverP < 1.1) {
						hTRDPoint_TrackX_EPCut->Fill(point->x, extrapolation.position.X());
						hTRDPoint_TrackY_EPCut->Fill(point->y, extrapolation.position.Y());
						
						hTRDPoint_DeltaXY_EPCut->Fill(locDeltaX, locDeltaY);
						hTRDPoint_DeltaX_T_EPCut->Fill(locDeltaX, point->time);

						if(fabs(locDeltaX) < 5. && fabs(locDeltaY) < 5.) {
							hTRDPoint_Time_EPCut->Fill(point->time);
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
//     }

	DEvent::GetLockService(event)->RootFillUnLock(this); //RELEASE ROOT FILL LOCK
}
//----------------------------------------------------------------------------------


void JEventProcessor_TRD_hists::EndRun() {
    // This is called whenever the run number changes, before it is
    // changed to give you a chance to clean up before processing
    // events from the next run number.
}


//----------------------------------------------------------------------------------


void JEventProcessor_TRD_hists::Finish() {
    // Called before program exit after event processing is finished.
}

//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
