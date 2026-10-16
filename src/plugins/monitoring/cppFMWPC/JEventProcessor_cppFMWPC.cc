// $Id$
//
//    File: JEventProcessor_cppFMWPC.cc
// Created: Sat Mar 13 08:01:05 EST 2021
// Creator: zihlmann (on Linux ifarm1901.jlab.org 3.10.0-1062.4.1.el7.x86_64 x86_64)
//

#include "JEventProcessor_cppFMWPC.h"
#include <JANA/JEventProcessor.h>


// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
#include <JANA/JFactory.h>
extern "C"{
  void InitPlugin(JApplication *app){
    InitJANAPlugin(app);
    app->Add(new JEventProcessor_cppFMWPC());
  }
} // "C"


//------------------
// JEventProcessor_cppFMWPC (Constructor)
//------------------
JEventProcessor_cppFMWPC::JEventProcessor_cppFMWPC()
{
	SetTypeName("JEventProcessor_cppFMWPC");
}

//------------------
// ~JEventProcessor_cppFMWPC (Destructor)
//------------------
JEventProcessor_cppFMWPC::~JEventProcessor_cppFMWPC()
{
  
}

//------------------
// init
//------------------
void JEventProcessor_cppFMWPC::Init()
{
  // This is called once at program startup. 

  TDirectory *top = gDirectory;
  top->cd();

  gDirectory->mkdir("FMWPC")->cd();

  nFMWPCchambers = 6;
  char hnam[128], htit[128];
  for ( int k=0; k<nFMWPCchambers ;k++){
    sprintf(hnam,"FMWPCwiresT%d",k);
    sprintf(htit,"Chamber %d FMWPC hit time vs. wire number",k);
    FMWPCwiresT[k] = new TH2D(hnam, htit,  145, 0., 145., 100, 0., 500.);

    sprintf(hnam,"FMWPCwiresE%d",k);
    sprintf(htit,"Chamber %d FMWPC hit q vs. wire number",k);
    FMWPCwiresQ[k] = new TH2D(hnam, htit,  145, 0., 145., 500, 0., 100.);
  }


  for (int k=0; k<24; k++){
    sprintf(hnam,"FDCwiresT%d",k);
    sprintf(htit,"Plane %d FDC hit time vs. wire number",k);
    FDCwiresT[k] = new TH2D(hnam, htit,  100, 0., 100., 100, 0., 500.);
  }

  // Correlations between planes

  h2_V1_vs_H2 = new TH2D("h2_V1_vs_H2",";H2 wire;V1 wire",145,0,145,145,0,145);
  h2_V3_vs_H4 = new TH2D("h2_V3_vs_H4",";H4 wire;V3 wire",145,0,145,145,0,145);
  h2_V5_vs_H4 = new TH2D("h2_V5_vs_H4",";H4 wire;V5 wire",145,0,145,145,0,145);
  h2_V6_vs_H4 = new TH2D("h2_V6_vs_H4",";H4 wire;V6 wire",145,0,145,145,0,145);
  h2_V3_vs_V1 = new TH2D("h2_V3_vs_V1",";V1 wire;V3 wire",145,0,145,145,0,145);
  h2_H2_vs_H4 = new TH2D("h2_H2_vs_H4",";H4 wire;H2 wire",145,0,145,145,0,145);
  h2_V3_vs_V5 = new TH2D("h2_V3_vs_V5",";V5 wire;V3 wire",145,0,145,145,0,145);
  h2_V5_vs_V6 = new TH2D("h2_V5_vs_V6",";V6 wire;V5 wire",145,0,145,145,0,145);
  
  top->cd();
}

//------------------
// brun
//------------------
void JEventProcessor_cppFMWPC::BeginRun(const std::shared_ptr<const JEvent> &event)
{
  // This is called whenever the run number changes
}

//------------------
// evnt
//------------------
void JEventProcessor_cppFMWPC::Process(const std::shared_ptr<const JEvent> &event)
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
  

  vector < const DFMWPCHit*> fmwpcHits;
  event->Get(fmwpcHits);

  for (int k=0; k<(int)fmwpcHits.size(); k++) {
    
    const DFMWPCHit *hit1 = fmwpcHits[k];
    //std::cout<<hit->layer<<" / "<<hit->wire<<" / "<<hit->t<<std::endl;
    FMWPCwiresT[hit1->layer-1]->Fill((double)hit1->wire, (double)hit1->t);
    FMWPCwiresQ[hit1->layer-1]->Fill((double)hit1->wire, (double)hit1->q);
  }


  // V1 vs H2

  Int_t jlayer1=0;
  Int_t jlayer2=1;
  for (int k=0; k<(int)fmwpcHits.size(); k++) {
    
    const DFMWPCHit *hit1 = fmwpcHits[k];
    //std::cout<<hit->layer<<" / "<<hit->wire<<" / "<<hit->t<<std::endl;
    if (hit1->layer-1 == jlayer1) {
      for (int j=0; j<(int)fmwpcHits.size(); j++) {
    
	const DFMWPCHit *hit2 = fmwpcHits[j];
	//std::cout<<hit->layer<<" / "<<hit->wire<<" / "<<hit->t<<std::endl;

	if (hit2->layer-1 == jlayer2) {
	h2_V1_vs_H2->Fill((double)hit2->wire, (double)hit1->wire);
	}
      }
    }
  }

  // V3 vs H4

  jlayer1=2;
  jlayer2=3;
  for (int k=0; k<(int)fmwpcHits.size(); k++) {
    
    const DFMWPCHit *hit1 = fmwpcHits[k];
    //std::cout<<hit->layer<<" / "<<hit->wire<<" / "<<hit->t<<std::endl;
    if (hit1->layer-1 == jlayer1) {
      for (int j=0; j<(int)fmwpcHits.size(); j++) {
    
	const DFMWPCHit *hit2 = fmwpcHits[j];
	//std::cout<<hit->layer<<" / "<<hit->wire<<" / "<<hit->t<<std::endl;

	if (hit2->layer-1 == jlayer2) {
	h2_V3_vs_H4->Fill((double)hit2->wire, (double)hit1->wire);
	}
      }
    }
  }

  // V5 vs H4

  jlayer1=4;
  jlayer2=3;
  for (int k=0; k<(int)fmwpcHits.size(); k++) {
    
    const DFMWPCHit *hit1 = fmwpcHits[k];
    //std::cout<<hit->layer<<" / "<<hit->wire<<" / "<<hit->t<<std::endl;
    if (hit1->layer-1 == jlayer1) {
      for (int j=0; j<(int)fmwpcHits.size(); j++) {
    
	const DFMWPCHit *hit2 = fmwpcHits[j];
	//std::cout<<hit->layer<<" / "<<hit->wire<<" / "<<hit->t<<std::endl;

	if (hit2->layer-1 == jlayer2) {
	h2_V5_vs_H4->Fill((double)hit2->wire, (double)hit1->wire);
	}
      }
    }
  }


  // V6 vs H4

  jlayer1=5;
  jlayer2=3;
  for (int k=0; k<(int)fmwpcHits.size(); k++) {
    
    const DFMWPCHit *hit1 = fmwpcHits[k];
    //std::cout<<hit->layer<<" / "<<hit->wire<<" / "<<hit->t<<std::endl;
    if (hit1->layer-1 == jlayer1) {
      for (int j=0; j<(int)fmwpcHits.size(); j++) {
    
	const DFMWPCHit *hit2 = fmwpcHits[j];
	//std::cout<<hit->layer<<" / "<<hit->wire<<" / "<<hit->t<<std::endl;

	if (hit2->layer-1 == jlayer2) {
	h2_V6_vs_H4->Fill((double)hit2->wire, (double)hit1->wire);
	}
      }
    }
  }

  // V3 vs V1

  jlayer1=2;
  jlayer2=0;
  for (int k=0; k<(int)fmwpcHits.size(); k++) {
    
    const DFMWPCHit *hit1 = fmwpcHits[k];
    //std::cout<<hit->layer<<" / "<<hit->wire<<" / "<<hit->t<<std::endl;
    if (hit1->layer-1 == jlayer1) {
      for (int j=0; j<(int)fmwpcHits.size(); j++) {
    
	const DFMWPCHit *hit2 = fmwpcHits[j];
	//std::cout<<hit->layer<<" / "<<hit->wire<<" / "<<hit->t<<std::endl;

	if (hit2->layer-1 == jlayer2) {
	h2_V3_vs_V1->Fill((double)hit2->wire, (double)hit1->wire);
	}
      }
    }
  }

  // V3 vs V5

  jlayer1=2;
  jlayer2=4;
  for (int k=0; k<(int)fmwpcHits.size(); k++) {
    
    const DFMWPCHit *hit1 = fmwpcHits[k];
    //std::cout<<hit->layer<<" / "<<hit->wire<<" / "<<hit->t<<std::endl;
    if (hit1->layer-1 == jlayer1) {
      for (int j=0; j<(int)fmwpcHits.size(); j++) {
    
	const DFMWPCHit *hit2 = fmwpcHits[j];
	//std::cout<<hit->layer<<" / "<<hit->wire<<" / "<<hit->t<<std::endl;

	if (hit2->layer-1 == jlayer2) {
	h2_V3_vs_V5->Fill((double)hit2->wire, (double)hit1->wire);
	}
      }
    }
  }

  // H2 vs H4

  jlayer1=1;
  jlayer2=3;
  for (int k=0; k<(int)fmwpcHits.size(); k++) {
    
    const DFMWPCHit *hit1 = fmwpcHits[k];
    //std::cout<<hit->layer<<" / "<<hit->wire<<" / "<<hit->t<<std::endl;
    if (hit1->layer-1 == jlayer1) {
      for (int j=0; j<(int)fmwpcHits.size(); j++) {
    
	const DFMWPCHit *hit2 = fmwpcHits[j];
	//std::cout<<hit->layer<<" / "<<hit->wire<<" / "<<hit->t<<std::endl;

	if (hit2->layer-1 == jlayer2) {
	h2_H2_vs_H4->Fill((double)hit2->wire, (double)hit1->wire);
	}
      }
    }
  }

  // V5 vs V6

  jlayer1=4;
  jlayer2=5;
  for (int k=0; k<(int)fmwpcHits.size(); k++) {
    
    const DFMWPCHit *hit1 = fmwpcHits[k];
    //std::cout<<hit->layer<<" / "<<hit->wire<<" / "<<hit->t<<std::endl;
    if (hit1->layer-1 == jlayer1) {
      for (int j=0; j<(int)fmwpcHits.size(); j++) {
    
	const DFMWPCHit *hit2 = fmwpcHits[j];
	//std::cout<<hit->layer<<" / "<<hit->wire<<" / "<<hit->t<<std::endl;

	if (hit2->layer-1 == jlayer2) {
	h2_V5_vs_V6->Fill((double)hit2->wire, (double)hit1->wire);
	}
      }
    }
  }




  vector <const DFDCHit*> fdcHits;
  event->Get(fdcHits);
  
  for (int k=0; k<(int)fdcHits.size(); k++){
    const DFDCHit *h = fdcHits[k];
    if (h->plane != 2){
      // this is not a wire
      continue;
    }
    int l = h->gLayer - 1 ;
    int w = h->element;
    FDCwiresT[l]->Fill((double)w, h->t);
  }
}

//------------------
// erun
//------------------
void JEventProcessor_cppFMWPC::EndRun()
{
	// This is called whenever the run number changes, before it is
	// changed to give you a chance to clean up before processing
	// events from the next run number.
}

//------------------
// fini
//------------------
void JEventProcessor_cppFMWPC::Finish()
{
  // Called before program exit after event processing is finished.
}

