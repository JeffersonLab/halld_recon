// $Id$
//
//    File: JEventProcessor_cppFMWPC_ana.cc
// Created: Sat Mar 13 08:01:05 EST 2021
// Creator: zihlmann (on Linux ifarm1901.jlab.org 3.10.0-1062.4.1.el7.x86_64 x86_64)
//

#include <JANA/JEventProcessor.h>
#include <JANA/JEventLoop.h>
// using namespace jana;
using namespace std;


// Routine used to create our JEventProcessor
#include "JEventProcessor_cppFMWPC_ana.h"
#include <JANA/JApplication.h>
#include <JANA/JFactory.h>
#include "TRACKING/DMCThrown.h"
extern "C"{
  void InitPlugin(JApplication *app){
    InitJANAPlugin(app);
    app->AddProcessor(new JEventProcessor_cppFMWPC_ana());
  }
} // "C"


//------------------
// JEventProcessor_cppFMWPC_ana (Constructor)
//------------------
JEventProcessor_cppFMWPC_ana::JEventProcessor_cppFMWPC_ana()
{
  
}

//------------------
// ~JEventProcessor_cppFMWPC_ana (Destructor)
//------------------
JEventProcessor_cppFMWPC_ana::~JEventProcessor_cppFMWPC_ana()
{
  
}

//------------------
// init
//------------------
jerror_t JEventProcessor_cppFMWPC_ana::init(void)
{
  // This is called once at program startup. 

  TDirectory *top = gDirectory;
  top->cd();

  gDirectory->mkdir("FMWPC_ana")->cd();

  nFMWPCchambers = 6;
  char hnam[128], htit[128];
  for ( int k=0; k<nFMWPCchambers ;k++){
    sprintf(hnam,"FMWPCwiresT%d",k+1);
    sprintf(htit,"Chamber %d FMWPC hit time vs. wire number",k+1);
    FMWPCwiresT[k] = new TH2D(hnam, htit,  145, 0., 145., 100, 0., 500.);

    sprintf(hnam,"FMWPCwiresQ%d",k+1);
    sprintf(htit,"Chamber %d FMWPC hit q vs. wire number",k+1);
    FMWPCwiresQ[k] = new TH2D(hnam, htit,  145, 0., 145., 500, 0., 100.);

    sprintf(hnam,"h2_pmuon_vs_mult%d",k+1);
    sprintf(htit,"Plane %d;Multipliticity;Pmuon",k+1);
    h2_pmuon_vs_mult[k] = new TH2D(hnam, htit,  10, 0, 10, 40, 0, 8);
  }


  for (int k=0; k<24; k++){
    sprintf(hnam,"FDCwiresT%d",k+1);
    sprintf(htit,"Plane %d FDC hit time vs. wire number",k+1);
    FDCwiresT[k] = new TH2D(hnam, htit,  100, 0., 100., 100, 0., 500.);
  }

  // Correlations between planes

  h2_V1_vs_H2 = new TH2D("h2_V1_vs_H2",";H2 wire;V1 wire",145,0,145,145,0,145);
  h2_V3_vs_H4 = new TH2D("h2_V3_vs_H4",";H4 wire;V3 wire",145,0,145,145,0,145);
  h2_V5_vs_H6 = new TH2D("h2_V5_vs_H6",";H6 wire;V5 wire",145,0,145,145,0,145);
  h2_V1_vs_V3 = new TH2D("h2_V1_vs_V3",";V3 wire;V1 wire",145,0,145,145,0,145);
  h2_H2_vs_H4 = new TH2D("h2_H2_vs_H4",";H4 wire;H2 wire",145,0,145,145,0,145);
  h2_V3_vs_V5 = new TH2D("h2_V3_vs_V5",";V5 wire;V3 wire",145,0,145,145,0,145);
  h2_H4_vs_H6 = new TH2D("h2_H4_vs_H6",";H6 wire;H4 wire",145,0,145,145,0,145);

  h2_pmuon_vs_MWPC = new TH2D("h2_pmuon_vs_MWPC",";MWPC plane; Muon momentum (GeV)",7,0,7,40,0,8);
  
  top->cd();


  return NOERROR;
}

//------------------
// brun
//------------------
jerror_t JEventProcessor_cppFMWPC_ana::brun(JEventLoop *eventLoop, int32_t runnumber)
{
  // This is called whenever the run number changes
  return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t JEventProcessor_cppFMWPC_ana::evnt(JEventLoop *loop, uint64_t eventnumber)
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
  


  vector <const DMCThrown*> mcthrowns;
  Double_t pmuon=0;
  loop->Get(mcthrowns);
  unsigned int kmax= mcthrowns.size() <=1? mcthrowns.size(): 1 ;    // assumes that the original particle is first in list
  for (unsigned int k=0; k<kmax;k++){
    //cout << endl << " cppFMWPC testing output " << endl;
    const DMCThrown *mcthrown = mcthrowns[k];
    // cout << " k=" << k << " mcthrowns.size()=" << mcthrowns.size() << " mcthrown->charge()= " << mcthrown->charge() << " mcthrown->momentum().Mag()=" << mcthrown->momentum().Mag() << " mcthrown->z()=" <<  mcthrown->z() << endl << endl;
    if (mcthrown->charge() !=0 && mcthrown->z()<2 && mcthrown->time()<5 && mcthrown->parentid==0) {
      // cout << " k=" << k << " mcthrown->momentum().Mag()= " <<  mcthrown->momentum().Mag() << endl;
      pmuon = mcthrown->momentum().Mag();
    }
    else {
      cout << " *** JEventProcessor Loop: charge=" << mcthrown->charge() << " size=" << mcthrowns.size() << " pmuon=" << pmuon << " mcthrown->z()=" << mcthrown->z() << " mcthrown->time()=" << mcthrown->time() << " mcthrown->parentid=" << mcthrown->parentid << endl;
    }
  }

  // if  (mcthrowns.size() > 1) return NOERROR;   // eliminate muon decays

  // define variables to accumulate multiplicities per event
  Int_t mwpc_mult[nFMWPCchambers];

  for (int k=0; k<nFMWPCchambers; k++) {
    mwpc_mult[k] = 0;     // initialize each event
  }

 

  vector < const DFMWPCHit*> fmwpcHits;
  loop->Get(fmwpcHits);

  // cout << " pmuon=" << pmuon << " (int)fmwpcHits.size()=" << (int)fmwpcHits.size() << endl;

  for (int k=0; k<(int)fmwpcHits.size(); k++) {
    
    const DFMWPCHit *hit1 = fmwpcHits[k];
    //std::cout<<hit->layer<<" / "<<hit->wire<<" / "<<hit->t<<std::endl;
    FMWPCwiresT[hit1->layer-1]->Fill((double)hit1->wire, (double)hit1->t);
    FMWPCwiresQ[hit1->layer-1]->Fill((double)hit1->wire, (double)hit1->q);

    mwpc_mult[hit1->layer-1] += 1;
    // cout << " k=" << k << " hit1->layer-1=" << hit1->layer-1 << " mwpc_mult[k]=" << mwpc_mult[hit1->layer-1] << endl;
  }



  for (int k=0; k<nFMWPCchambers; k++) {
    h2_pmuon_vs_mult[k]->Fill((double)mwpc_mult[k],pmuon);
    // cout << " Fill loop k=" << k << " mult=" << (double)mwpc_mult[k] << " pmuon=" << pmuon << endl;
  }


  // V1 vs H2

  Int_t jlayer1=0;
  Int_t jlayer2=1;
  for (int k=0; k<(int)fmwpcHits.size(); k++) {
    
    const DFMWPCHit *hit1 = fmwpcHits[k];
    h2_pmuon_vs_MWPC->Fill(hit1->layer,pmuon);
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

  // V5 vs H6

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
	h2_V5_vs_H6->Fill((double)hit2->wire, (double)hit1->wire);
	}
      }
    }
  }


  // V1 vs V3

  jlayer1=0;
  jlayer2=2;
  for (int k=0; k<(int)fmwpcHits.size(); k++) {
    
    const DFMWPCHit *hit1 = fmwpcHits[k];
    //std::cout<<hit->layer<<" / "<<hit->wire<<" / "<<hit->t<<std::endl;
    if (hit1->layer-1 == jlayer1) {
      for (int j=0; j<(int)fmwpcHits.size(); j++) {
    
	const DFMWPCHit *hit2 = fmwpcHits[j];
	//std::cout<<hit->layer<<" / "<<hit->wire<<" / "<<hit->t<<std::endl;

	if (hit2->layer-1 == jlayer2) {
	h2_V1_vs_V3->Fill((double)hit2->wire, (double)hit1->wire);
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

  // H4 vs H6

  jlayer1=3;
  jlayer2=5;
  for (int k=0; k<(int)fmwpcHits.size(); k++) {
    
    const DFMWPCHit *hit1 = fmwpcHits[k];
    //std::cout<<hit->layer<<" / "<<hit->wire<<" / "<<hit->t<<std::endl;
    if (hit1->layer-1 == jlayer1) {
      for (int j=0; j<(int)fmwpcHits.size(); j++) {
    
	const DFMWPCHit *hit2 = fmwpcHits[j];
	//std::cout<<hit->layer<<" / "<<hit->wire<<" / "<<hit->t<<std::endl;

	if (hit2->layer-1 == jlayer2) {
	h2_H4_vs_H6->Fill((double)hit2->wire, (double)hit1->wire);
	}
      }
    }
  }


  vector <const DFDCHit*> fdcHits;
  loop->Get(fdcHits);
  
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

  


  return NOERROR;
}

//------------------
// erun
//------------------
jerror_t JEventProcessor_cppFMWPC_ana::erun(void)
{
	// This is called whenever the run number changes, before it is
	// changed to give you a chance to clean up before processing
	// events from the next run number.
	return NOERROR;
}

//------------------
// fini
//------------------
jerror_t JEventProcessor_cppFMWPC_ana::fini(void)
{
  // Called before program exit after event processing is finished.

  return NOERROR;
}

