// $Id$
//
//    File: JEventProcessor_scanf250.cc
// Created: Tue Oct  9 21:26:13 EDT 2018
// Creator: njarvis (on Linux egbert 2.6.32-696.23.1.el6.x86_64 x86_64)
//

#include "JEventProcessor_scanf250.h"

#include <vector>

#include "DAQ/Df250WindowRawData.h"     

#include <TTree.h>
#include <TBranch.h>



// Routine used to create our JEventProcessor
extern "C"{
void InitPlugin(JApplication *app){
	InitJANAPlugin(app);
	app->Add(new JEventProcessor_scanf250());
}
} // "C"

//define static local variable //declared in header file
thread_local DTreeFillData JEventProcessor_scanf250::dTreeFillData;

//------------------
// JEventProcessor_scanf250 (Constructor)
//------------------
JEventProcessor_scanf250::JEventProcessor_scanf250()
{
	SetTypeName("JEventProcessor_scanf250");
}

//------------------
// ~JEventProcessor_scanf250 (Destructor)
//------------------
JEventProcessor_scanf250::~JEventProcessor_scanf250()
{
}

//------------------
// Init
//------------------
void JEventProcessor_scanf250::Init()
{
	// This is called once at program startup. 

  //TTREE INTERFACE
  //MUST DELETE WHEN FINISHED: OR ELSE DATA WON'T BE SAVED!!!
  dTreeInterface = DTreeInterface::Create_DTreeInterface("T", "tree_scanf250.root");

  //TTREE BRANCHES
  DTreeBranchRegister locTreeBranchRegister;



  locTreeBranchRegister.Register_Single<ULong64_t>("eventnum");
  locTreeBranchRegister.Register_Single<UInt_t>("rocid");
  locTreeBranchRegister.Register_Single<UInt_t>("slot");
  locTreeBranchRegister.Register_Single<UInt_t>("channel");
  locTreeBranchRegister.Register_Single<UInt_t>("itrigger");

  locTreeBranchRegister.Register_Single<UInt_t>("NSAMPLES");
  locTreeBranchRegister.Register_FundamentalArray<UInt_t>("adc", "NSAMPLES");


 //REGISTER BRANCHES
  dTreeInterface->Create_Branches(locTreeBranchRegister);
}

//------------------
// BeginRun
//------------------
void JEventProcessor_scanf250::BeginRun(const std::shared_ptr<const JEvent>& event)
{
	// This is called whenever the run number changes
}

//------------------
// Process
//------------------
void JEventProcessor_scanf250::Process(const std::shared_ptr<const JEvent>& event)
{
	// This is called for every event. Use of common resources like writing
	// to a file or filling a histogram should be mutex protected. Using
	// event->Get(...) to get reconstructed objects (and thereby activating the
	// reconstruction algorithm) should be done outside of any mutex lock
	// since multiple threads may call this method at the same time.
	// Here's an example:
	//
	// vector<const MyDataClass*> mydataclasses;
	// event->Get(mydataclasses);
	//
	// lockService->RootWriteLock();
	//  ... fill historgrams or trees ...
	// lockService->RootUnLock();


  auto eventnumber = event->GetEventNumber();

  vector<const Df250WindowRawData*> wrdvector;
  event->Get(wrdvector);


  uint32_t nw = (uint32_t)wrdvector.size();

  if (nw) {

    const uint32_t NSAMPLES = 100;

    uint16_t adc[NSAMPLES];       //    vector<uint16_t> samples;
    for (uint i=0; i<NSAMPLES; i++) adc[i]=0;


    for (int i=0; i<(int)nw; i++) {

      const Df250WindowRawData *wrd = wrdvector[i];

      if (wrd) {

        int ns = (int)wrd->samples.size();
        int rocid = (int)wrd->rocid;

        // add a continue  if the rocid is not wanted
  
        if ((rocid<11 || rocid > 22)) continue;

        for (int j=0; j<ns; j++) {
          adc[j] = wrd->samples[j];
        }

        dTreeFillData.Fill_Single<ULong64_t>("eventnum",eventnumber);
        dTreeFillData.Fill_Single<UInt_t>("rocid",wrd->rocid);
	dTreeFillData.Fill_Single<UInt_t>("slot",wrd->slot);
	dTreeFillData.Fill_Single<UInt_t>("channel",wrd->channel);
	dTreeFillData.Fill_Single<UInt_t>("itrigger",wrd->itrigger);



	size_t index = 0;
        for (int j=0; j<(int)NSAMPLES; j++) {
          dTreeFillData.Fill_Array<UInt_t>("adc",adc[j],index);
          index++;
	}

	//FILL ARRAY SIZE
	dTreeFillData.Fill_Single<UInt_t>("NSAMPLES",index);


	//FILL ARRAY SIZE
	dTreeInterface->Fill(dTreeFillData);

      }
 
    }



  }   // if (nw)

}

//------------------
// EndRun
//------------------
void JEventProcessor_scanf250::EndRun()
{
	// This is called whenever the run number changes, before it is
	// changed to give you a chance to clean up before processing
	// events from the next run number.

  delete dTreeInterface;
}

//------------------
// Finish
//------------------
void JEventProcessor_scanf250::Finish()
{
	// Called before program exit after event processing is finished.
}

