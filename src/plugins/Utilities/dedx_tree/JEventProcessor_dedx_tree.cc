// $Id$
//
//    File: JEventProcessor_dedx_tree.cc
// Created: Wed Aug 28 14:48:25 EDT 2019
// Creator: njarvis (on Linux albert.phys.cmu.edu 3.10.0-693.5.2.el7.x86_64 x86_64)
//

#include "JEventProcessor_dedx_tree.h"
using namespace jana;


// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
#include <JANA/JFactory.h>

extern "C"{
void InitPlugin(JApplication *app){
	InitJANAPlugin(app);
	app->AddProcessor(new JEventProcessor_dedx_tree());
}
} // "C"

//define static local variable //declared in header file
thread_local DTreeFillData JEventProcessor_dedx_tree::dTreeFillData;

//------------------
// JEventProcessor_dedx_tree (Constructor)
//------------------
JEventProcessor_dedx_tree::JEventProcessor_dedx_tree()
{

}

//------------------
// ~JEventProcessor_dedx_tree (Destructor)
//------------------
JEventProcessor_dedx_tree::~JEventProcessor_dedx_tree()
{

}

//------------------
// init
//------------------
jerror_t JEventProcessor_dedx_tree::init(void)
{
	// This is called once at program startup. 

  
    //TTREE INTERFACE
    //MUST DELETE WHEN FINISHED: OR ELSE DATA WON'T BE SAVED!!!
    dTreeInterface = DTreeInterface::Create_DTreeInterface("dedx_tree", "dedx_tree.root");

    //TTREE BRANCHES
    DTreeBranchRegister locTreeBranchRegister;

    locTreeBranchRegister.Register_Single<ULong64_t>("eventnumber"); 
    locTreeBranchRegister.Register_Single<UInt_t>("ctracks"); 

    locTreeBranchRegister.Register_Single<UInt_t>("ndedxhits"); 

    locTreeBranchRegister.Register_Single<Double_t>("x"); 
    locTreeBranchRegister.Register_Single<Double_t>("y"); 
    locTreeBranchRegister.Register_Single<Double_t>("z"); 
    locTreeBranchRegister.Register_Single<Double_t>("r");
 
    locTreeBranchRegister.Register_Single<Double_t>("phi");   
    locTreeBranchRegister.Register_Single<Double_t>("theta"); 

    locTreeBranchRegister.Register_Single<Int_t>("charge"); 
    locTreeBranchRegister.Register_Single<Double_t>("p"); 
    locTreeBranchRegister.Register_Single<Double_t>("dedx"); 
    locTreeBranchRegister.Register_Single<Double_t>("dedx_int"); 
    locTreeBranchRegister.Register_Single<Double_t>("dedx_corr");
    locTreeBranchRegister.Register_Single<Double_t>("dedx_int_corr");

    //REGISTER BRANCHES
    dTreeInterface->Create_Branches(locTreeBranchRegister);

    return NOERROR;
}

//------------------
// brun
//------------------
jerror_t JEventProcessor_dedx_tree::brun(JEventLoop *eventLoop, int32_t runnumber)
{
	// This is called whenever the run number changes
	return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t JEventProcessor_dedx_tree::evnt(JEventLoop *loop, uint64_t eventnumber)
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
  double z = locVertex->dSpacetimeVertex.Z();
  if ((z < 45.0) || (z > 85.0)) return NOERROR;

  double x = locVertex->dSpacetimeVertex.X();
  double y = locVertex->dSpacetimeVertex.Y();
  double r = sqrt (x*x + y*y);


  vector<const DChargedTrack*> ctracks;
  loop->Get(ctracks);

  if ((int)ctracks.size() ==0) return NOERROR;

  dTreeFillData.Fill_Single<ULong64_t>("eventnumber",(ULong64_t)eventnumber);

  dTreeFillData.Fill_Single<UInt_t>("ctracks",(UInt_t)ctracks.size()); 


  for (uint32_t i=0; i<ctracks.size(); i++) {

    const DChargedTrackHypothesis *hyp=ctracks[i]->Get_BestFOM();
    
    if (hyp != NULL) {  

      const DTrackTimeBased *track = hyp->Get_TrackTimeBased();

      int nhits = (int)track->dNumHitsUsedFordEdx_CDC; 

      if (nhits==0) continue;

      dTreeFillData.Fill_Single<UInt_t>("ndedxhits",(UInt_t)nhits); 

      double charge = track->charge();
      DVector3 mom = track->momentum();
      double p = mom.Mag();

      double theta_degrees = mom.Theta() * 180.0/3.14159;
      double phi_degrees = mom.Phi() * 180.0/3.14159;

      double dedx = 1.0e6*track->ddEdx_CDC_amp;
      double dedxfromintegral = 1.0e6*track->ddEdx_CDC;
      double dedx_corr = 1.0e6*hyp->Get_dEdx_CDC_amp();
      double dedxfromintegral_corr = 1.0e6*hyp->Get_dEdx_CDC_int();

      dTreeFillData.Fill_Single<Double_t>("x",x); 
      dTreeFillData.Fill_Single<Double_t>("y",y); 
      dTreeFillData.Fill_Single<Double_t>("z",z); 
      dTreeFillData.Fill_Single<Double_t>("r",r); 
      dTreeFillData.Fill_Single<Double_t>("phi",phi_degrees);   
      dTreeFillData.Fill_Single<Double_t>("theta",theta_degrees); 

      dTreeFillData.Fill_Single<Int_t>("charge",(Int_t)charge); 
      dTreeFillData.Fill_Single<Double_t>("p",p); 
      dTreeFillData.Fill_Single<Double_t>("dedx",dedx); 
      dTreeFillData.Fill_Single<Double_t>("dedx_int",dedxfromintegral); 
      dTreeFillData.Fill_Single<Double_t>("dedx_corr",dedx_corr);
      dTreeFillData.Fill_Single<Double_t>("dedx_int_corr",dedxfromintegral_corr);

       //FILL TTREE
       dTreeInterface->Fill(dTreeFillData);
    }

  }


	return NOERROR;
}

//------------------
// erun
//------------------
jerror_t JEventProcessor_dedx_tree::erun(void)
{
	// This is called whenever the run number changes, before it is
	// changed to give you a chance to clean up before processing
	// events from the next run number.
	return NOERROR;
}

//------------------
// fini
//------------------
jerror_t JEventProcessor_dedx_tree::fini(void)
{
	// Called before program exit after event processing is finished.

        delete dTreeInterface; //saves trees to file, closes file

	return NOERROR;
}

