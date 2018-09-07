// $Id$
//
//    File: JEventProcessor_CDC_dedx.cc
// Created: Thu Aug 16 16:35:42 EDT 2018
// Creator: njarvis (on Linux egbert 2.6.32-696.23.1.el6.x86_64 x86_64)
//

#include "JEventProcessor_CDC_dedx.h"
using namespace jana;


// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
#include <JANA/JFactory.h>
extern "C"{
void InitPlugin(JApplication *app){
	InitJANAPlugin(app);
	app->AddProcessor(new JEventProcessor_CDC_dedx());
}
} // "C"


//------------------
// JEventProcessor_CDC_dedx (Constructor)
//------------------
JEventProcessor_CDC_dedx::JEventProcessor_CDC_dedx()
{

}

//------------------
// ~JEventProcessor_CDC_dedx (Destructor)
//------------------
JEventProcessor_CDC_dedx::~JEventProcessor_CDC_dedx()
{

}

//------------------
// init
//------------------
jerror_t JEventProcessor_CDC_dedx::init(void)
{
	// This is called once at program startup. 

  TDirectory *main = gDirectory;
  gDirectory->mkdir("CDC_dedx")->cd();

  dedx_p = new TH2D("dedx_p","CDC dE/dx vs p, 4+ hits used;p (GeV/c);dE/dx (keV/cm)",250,0,10,400,0,25);

  dedx_p_pos = new TH2D("dedx_p_pos","CDC dE/dx vs p, q+, 4+ hits used;p (GeV/c);dE/dx (keV/cm)",250,0,10,400,0,25);

  dedx_p_neg = new TH2D("dedx_p_neg","CDC dE/dx vs p, q-, 4+ hits used;p (GeV/c);dE/dx (keV/cm)",250,0,10,400,0,25);


  intdedx_p = new TH2D("intdedx_p","CDC dE/dx (from integral) vs p, 4+ hits used;p (GeV/c);dE/dx (keV/cm)",250,0,10,400,0,25);

  intdedx_p_pos = new TH2D("intdedx_p_pos","CDC dE/dx (from integral) vs p, q+, 4+ hits used;p (GeV/c);dE/dx (keV/cm)",250,0,10,400,0,25);

  intdedx_p_neg = new TH2D("intdedx_p_neg","CDC dE/dx (from integral) vs p, q-, 4+ hits used;p (GeV/c);dE/dx (keV/cm)",250,0,10,400,0,25);



  main->cd();

	return NOERROR;
}

//------------------
// brun
//------------------
jerror_t JEventProcessor_CDC_dedx::brun(JEventLoop *eventLoop, int32_t runnumber)
{
	// This is called whenever the run number changes
	return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t JEventProcessor_CDC_dedx::evnt(JEventLoop *loop, uint64_t eventnumber)
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
	//  ... fill historgrams or trees ...
	// japp->RootFillUnLock(this);

  // select events with physics events, i.e., not LED and other front panel triggers
  const DTrigger* locTrigger = NULL; 
  loop->GetSingle(locTrigger); 
  if(locTrigger->Get_L1FrontPanelTriggerBits() != 0) return NOERROR;


  const DVertex* locVertex  = NULL;
  loop->GetSingle(locVertex);
  double vertexz = locVertex->dSpacetimeVertex.Z();
  if ((vertexz < 52.0) || (vertexz > 78.0)) return NOERROR;


  vector<const DTrackTimeBased*> tracks;
  loop->Get(tracks);
  if (tracks.size() ==0) return NOERROR;


  for (uint32_t i=0; i<tracks.size(); i++) {

    int nhits = (int)tracks[i]->dNumHitsUsedFordEdx_CDC; 
    if (nhits < 4) continue;

    double charge = tracks[i]->charge();
    DVector3 mom = tracks[i]->momentum();
    double p = mom.Mag();

    double dedx = 1.0e6*tracks[i]->ddEdx_CDC_amp;

    if (dedx > 0) {

      japp->RootFillLock(this);

      dedx_p->Fill(p,dedx);
    
      if (charge > 0) {
        dedx_p_pos->Fill(p,dedx);
      } else {
        dedx_p_neg->Fill(p,dedx);
      } 

      japp->RootFillUnLock(this);

    }

    // repeat for dedx from integral

    dedx = 1.0e6*tracks[i]->ddEdx_CDC;

    if (dedx > 0) {

      japp->RootFillLock(this);

      intdedx_p->Fill(p,dedx);
    
      if (charge > 0) {
        intdedx_p_pos->Fill(p,dedx);
      } else {
        intdedx_p_neg->Fill(p,dedx);
      } 

      japp->RootFillUnLock(this);

    }



  }

	return NOERROR;
}

//------------------
// erun
//------------------
jerror_t JEventProcessor_CDC_dedx::erun(void)
{
	// This is called whenever the run number changes, before it is
	// changed to give you a chance to clean up before processing
	// events from the next run number.
	return NOERROR;
}

//------------------
// fini
//------------------
jerror_t JEventProcessor_CDC_dedx::fini(void)
{
	// Called before program exit after event processing is finished.
	return NOERROR;
}

