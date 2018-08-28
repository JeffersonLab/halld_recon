// $Id$
//
//    File: JEventProcessor_CDC_amp.cc
// Created: Tue Sep  6 10:13:02 EDT 2016
// Creator: njarvis (on Linux egbert 2.6.32-642.3.1.el6.x86_64 x86_64)
//

#include "JEventProcessor_CDC_amp.h"



// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
#include <JANA/JFactory.h>
extern "C"{
void InitPlugin(JApplication *app){
	InitJANAPlugin(app);
	app->AddProcessor(new JEventProcessor_CDC_amp());
}
} // "C"


//------------------
// JEventProcessor_CDC_amp (Constructor)
//------------------
JEventProcessor_CDC_amp::JEventProcessor_CDC_amp()
{

}

//------------------
// ~JEventProcessor_CDC_amp (Destructor)
//------------------
JEventProcessor_CDC_amp::~JEventProcessor_CDC_amp()
{

}

//------------------
// init
//------------------
jerror_t JEventProcessor_CDC_amp::init(void)
{
	// This is called once at program startup. 

  TDirectory *main = gDirectory;
  gDirectory->mkdir("CDC_amp")->cd();

  time = new TH1I("time","CDC time (hits on tracks); raw time",200,0,2000);

  a = new TH1I("a","CDC amplitude (hits on tracks);amplitude - pedestal",4096,0,4096);

  an = new TH2I("an","CDC amplitude vs n (hits on tracks); n; amplitude - pedestal",3522,0,3522,4096,0,4096);

  atime = new TH2D("atime","CDC amplitude vs time (hits on tracks); raw time; amplitude - pedestal",200,0,2000,4096,0,4096);

  atheta = new TH2D("atheta","CDC amplitude vs theta (hits on tracks); theta; amplitude - pedestal",180,0,180,4096,0,4096);


  a30 = new TH1I("a30","CDC amplitude (tracks, theta 28-32 deg, z 52-78cm);amplitude - pedestal",4096,0,4096);

  a30_100ns = new TH1I("a30_100ns","CDC amplitude, hits with drift time < 100ns (tracks, theta 28-32 deg, z 52-78cm);amplitude - pedestal",4096,0,4096);

  an30_100ns = new TH2I("an30_100ns","CDC amplitude vs n, hits with drift time < 100ns (tracks, theta 28-32 deg, z 52-78cm); n; amplitude - pedestal",3522,0,3522,4096,0,4096);

  atime30 = new TH2D("atime30","CDC amplitude vs time (tracks, theta 28-32 deg, z 52-78cm); raw time; amplitude - pedestal",200,0,2000,4096,0,4096);

  adoca30 = new TH2D("adoca30","CDC amplitude vs DOCA (tracks, theta 28-32 deg, z 52-78cm); DOCA (cm); amplitude - pedestal",200,0,1.0,4096,0,4096);



  a45 = new TH1I("a45","CDC amplitude (tracks, theta 43-47 deg, z 52-78cm);amplitude - pedestal",4096,0,4096);

  a45_100ns = new TH1I("a45_100ns","CDC amplitude, hits with drift time < 100ns (tracks, theta 43-47 deg, z 52-78cm); amplitude - pedestal",4096,0,4096);

  an45_100ns = new TH2I("an45_100ns","CDC amplitude vs n, hits with drift time < 100ns (tracks, theta 43-47 deg, z 52-78cm); n; amplitude - pedestal",3522,0,3522,4096,0,4096);

  atime45 = new TH2D("atime45","CDC amplitude vs time (tracks, theta 43-47 deg, z 52-78cm); raw time; amplitude - pedestal",200,0,2000,4096,0,4096);

  adoca45 = new TH2D("adoca45","CDC amplitude vs DOCA (tracks, theta 43-47 deg, z 52-78cm); DOCA (cm); amplitude - pedestal",200,0,1.0,4096,0,4096);






  a90 = new TH1I("a90","CDC amplitude (tracks, theta 85-95 deg, z 52-78cm);amplitude - pedestal",4096,0,4096);

  a90_100ns = new TH1I("a90_100ns","CDC amplitude, hits with drift time < 100ns (tracks, theta 85-95 deg, z 52-78cm);amplitude - pedestal",4096,0,4096);

  an90_100ns = new TH2I("an90_100ns","CDC amplitude vs n, hits with drift time < 100ns (tracks, theta 85-95 deg, z 52-78cm); n; amplitude - pedestal",3522,0,3522,4096,0,4096);

  atime90 = new TH2D("atime90","CDC amplitude vs time (tracks, theta 85-95 deg, z 52-78cm); raw time; amplitude - pedestal",200,0,2000,4096,0,4096);

  adoca90 = new TH2D("adoca90","CDC amplitude vs DOCA (tracks, theta 85-95 deg, z 52-78cm); DOCA (cm); amplitude - pedestal",200,0,1.0,4096,0,4096);

  time90 = new TH1I("time90","CDC time (tracks, theta 85-95 deg, z 52-78cm); raw time",200,0,2000);

  an90 = new TH2I("an90","CDC amplitude vs n (tracks, theta 85-95 deg, z 52-78cm); n; amplitude - pedestal",3522,0,3522,4096,0,4096);

  xt90 = new TH2D("xt90","CDC DOCA vs time (tracks, theta 85-95 deg, z 52-78cm); time (ns); DOCA (cm)",400,0,1200,400,0,1.0);


  main->cd();

  return NOERROR;
}

//------------------
// brun
//------------------
jerror_t JEventProcessor_CDC_amp::brun(JEventLoop *eventLoop, int32_t runnumber)
{
	// This is called whenever the run number changes

        if (runnumber<40000) ASCALE = 8;    // default for ASCALE before run 40,000 to be used if Df125config is not present

        if (runnumber>41497) ASCALE = 8;    // default for ASCALE before run 40,000 to be used if Df125config is not present

	return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t JEventProcessor_CDC_amp::evnt(JEventLoop *loop, uint64_t eventnumber)
{
	// This is called for every event. Use of common resources like writing
	// to a file or filling a histogram should be mutex protected. Using
	// loop->Get(...) to get reconstructed objects (and thereby activating the
	// reconstruction algorithm) should be done outside of any mutex lock
	// since multiple threads may call this method at the same time.
	// Here's an example:
	//
	// vector<const MyDataClass*> mydataclasses;
	// loop->Get(mydataclasses);
	//
	// japp->RootFillLock(this);
	//  ... fill histograms or trees ...
	// japp->RootFillUnLock(this);

  int ring, straw, n;   // ring number, straw number within ring, straw number overall (1 to 3522)

  uint32_t amp,ped,t;     // dcdcdigihits raw quantities: time, pedestal, amplitude, quality factor, overflow count

  //scaling factors will be overridden by Df125Config if presqnt
  //  uint16_t ASCALE = 8;   //amplitude
  //  uint16_t PSCALE = 1;   //ped

  //add extra 0 at front to use offset[1] for ring 1  //used to calculate straw number n 
  int straw_offset[29] = {0,0,42,84,138,192,258,324,404,484,577,670,776,882,1005,1128,1263,1398,1544,1690,1848,2006,2176,2346,2528,2710,2907,3104,3313};


  // select events with physics events, i.e., not LED and other front panel triggers
  const DTrigger* locTrigger = NULL; 
  loop->GetSingle(locTrigger); 
  if(locTrigger->Get_L1FrontPanelTriggerBits() != 0) 
    return NOERROR;

  // use only events with track vertex in the target region
  const DVertex* locVertex  = NULL;
  loop->GetSingle(locVertex);
  double vertexz = locVertex->dSpacetimeVertex.Z();
  if (vertexz < 52 || vertexz > 78) return NOERROR;


  // // test whether this is simulated or real data (skip digihits for sim data)
  // int SIMULATION;
  // vector<const DMCThrown*> MCThrowns;
  // loop->Get(MCThrowns);
  // if (MCThrowns.empty()) SIMULATION = 0;
  // if (!MCThrowns.empty()) SIMULATION = 1;


  const DCDCHit *hit = NULL;
  const DCDCDigiHit *digihit = NULL;
  const Df125CDCPulse *cp = NULL;

  int netamp = 0;
  float scaledped;

  int used[3523] = {0};

  //--------tracks---------------------------
  

  vector<const DTrackTimeBased*> tracks;

  loop->Get(tracks);


  for (uint32_t i=0; i<tracks.size(); i++) {

    DVector3 mom = tracks[i]->momentum();
    double theta = mom.Theta();
    theta = 180.0*theta/3.14159;

    vector<DTrackFitter::pull_t> pulls = tracks[i]->pulls;

    for (uint32_t j=0; j<pulls.size(); j++) {

      if (pulls[j].cdc_hit == NULL) continue;

      hit = NULL;
      pulls[j].cdc_hit->GetSingle(hit);

      double doca = pulls[j].d;
      double tdrift = pulls[j].tdrift;

      netamp = 0;  

      //   if (!SIMULATION) {

        digihit = NULL;
        hit->GetSingle(digihit);
        if (!digihit) continue;

        cp = NULL; 
        digihit->GetSingle(cp);
        if (!cp) continue; //no CDCPulseData (happens occasionally)

        if (cp->time_quality_bit) continue;
        if (cp->overflow_count) continue;

        amp = cp->first_max_amp;
        ped = cp->pedestal;
        t = cp->le_time;

        scaledped = ped*(float)PSCALE/(float)ASCALE;

        netamp = (int)amp - (int)scaledped;

	//  }

      if (netamp ==0) continue;

      ring     = (int)hit->ring;
      straw    = (int)hit->straw;

      n = straw_offset[ring] + straw;


      japp->RootFillLock(this); //ACQUIRE ROOT LOCK!!

      if (!used[n]) {

        used[n] = 1;

        a->Fill(netamp);
        an->Fill(n,netamp);
        atheta->Fill(theta,netamp);
        atime->Fill((int)t,netamp);
        time->Fill((int)t);

        if ((theta>85) && (theta<95)) {

            a90->Fill(netamp);
            atime90->Fill((int)t,netamp);
            adoca90->Fill(doca,netamp);

	    if (hit->t < 100.0) a90_100ns->Fill(netamp);
	    if (hit->t < 100.0) an90_100ns->Fill(n,netamp);

            an90->Fill(n,netamp);
            time90->Fill((int)t);
  	    xt90->Fill(tdrift,doca);

        } else if ((theta > 28.05) && (theta < 32)) {

            a30->Fill(netamp);
            atime30->Fill((int)t,netamp);
            adoca30->Fill(doca,netamp);

	    if (hit->t < 100.0) a30_100ns->Fill(netamp);
	    if (hit->t < 100.0) an30_100ns->Fill(n,netamp);

        } else if ((theta > 43.07) && (theta < 47)) {

            a45->Fill(netamp);
            atime45->Fill((int)t,netamp);
            adoca45->Fill(doca,netamp);

	    if (hit->t < 100.0) a45_100ns->Fill(netamp);
	    if (hit->t < 100.0) an45_100ns->Fill(n,netamp);


       }

      } //if !used


      japp->RootFillUnLock(this); 

    }
  }

	return NOERROR;
}

//------------------
// erun
//------------------
jerror_t JEventProcessor_CDC_amp::erun(void)
{
	// This is called whenever the run number changes, before it is
	// changed to give you a chance to clean up before processing
	// events from the next run number.
	return NOERROR;
}

//------------------
// fini
//------------------
jerror_t JEventProcessor_CDC_amp::fini(void)
{
	// Called before program exit after event processing is finished.


	return NOERROR;
}
