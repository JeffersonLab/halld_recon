// $Id$
//
//    File: JEventProcessor_fa125_itrig.cc
// Created: Sat Dec  6 21:34:19 EST 2014
// Creator: njarvis (on Linux maria 2.6.32-431.20.3.el6.x86_64 x86_64)
//

// read in Df125triggertime



//#include <iostream>

#include "JEventProcessor_fa125_itrig.h"

using namespace std;
using namespace jana;

#include <JANA/JApplication.h>
#include <JANA/JFactory.h>

#include <stdint.h>
#include <vector>

//#include "CDC/DCDCDigiHit.h"          //need to include this in order to use WRD.
//#include "DAQ/Df125WindowRawData.h"     

#include "DAQ/Df125TriggerTime.h"

#include "DAQ/DCODAEventInfo.h"

#include <TTree.h>
#include <TBranch.h>


static TTree *tree = NULL;




extern "C"{
void InitPlugin(JApplication *app){
	InitJANAPlugin(app);
	app->AddProcessor(new JEventProcessor_fa125_itrig());
}
} // "C"


//------------------
// JEventProcessor_fa125_itrig (Constructor)
//------------------
JEventProcessor_fa125_itrig::JEventProcessor_fa125_itrig()
{

}

//------------------
// ~JEventProcessor_fa125_itrig (Destructor)
//------------------
JEventProcessor_fa125_itrig::~JEventProcessor_fa125_itrig()
{

}

//------------------
// init
//------------------
jerror_t JEventProcessor_fa125_itrig::init(void)
{
	// This is called once at program startup. If you are creating
	// and filling histograms in this plugin, you should lock the
	// ROOT mutex like this:
	//

  japp->RootWriteLock();
  
  tree = new TTree("T","Df125 trigger times");

  ULong64_t eventnum;
  tree->Branch("eventnum",&eventnum,"eventnum/l");

  ULong64_t timestamp;
  tree->Branch("timestamp",&timestamp,"timestamp/l");

  ULong64_t ttime;
  tree->Branch("ttime",&ttime,"ttime/l");

  uint32_t rocid;
  tree->Branch("rocid",&rocid,"rocid/i");

  uint32_t slot;
  tree->Branch("slot",&slot,"slot/i");

  uint32_t itrigger;
  tree->Branch("itrigger",&itrigger,"itrigger/i");

  int tdiff;
  tree->Branch("tdiff",&tdiff,"tdiff/I");

  japp->RootUnLock();


  return NOERROR;
}

//------------------
// brun
//------------------
jerror_t JEventProcessor_fa125_itrig::brun(JEventLoop *eventLoop, int32_t runnumber)
{
	// This is called whenever the run number changes
	return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t JEventProcessor_fa125_itrig::evnt(JEventLoop *loop, uint64_t eventnumber)
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
	// japp->RootWriteLock();
	//  ... fill histograms or trees ...
	// japp->RootUnLock();


  //  printf("\nEvent %i\n\n",(int)eventnumber);

  vector<const DCODAEventInfo*> info;

  ULong64_t timestamp = 0;

  loop->Get(info);

  if (info.size() != 0) {
    //    printf("found eventinfo \n");
    //cout << "info size " << info.size() << endl;
    timestamp = (ULong64_t)info[0]->avg_timestamp;
  }


  
  vector<const Df125TriggerTime*> ttvector;
  loop->Get(ttvector); 

  uint32_t nd = (uint32_t)ttvector.size();

  if (nd) {
        

    ULong64_t eventnum = (ULong64_t)eventnumber;
    tree->SetBranchAddress("eventnum",&eventnum);

    //ULong64_t timestamp; //av timestamp for event
    tree->SetBranchAddress("timestamp",&timestamp);

    ULong64_t ttime; //trigger time
    tree->SetBranchAddress("ttime",&ttime);

    uint32_t rocid;
    tree->SetBranchAddress("rocid",&rocid);

    uint32_t slot;
    tree->SetBranchAddress("slot",&slot);

    uint32_t itrigger;
    tree->SetBranchAddress("itrigger",&itrigger);

    int tdiff;
    tree->SetBranchAddress("tdiff",&tdiff);


    const Df125TriggerTime *tt = NULL;

    //    cout  << "eventnum " << eventnum << " " << nd << " tt objects" << endl;

    for (uint32_t i=0; i<nd; i++) {

      tt = ttvector[i];  
 
      if (!tt) continue;

      //      if (tt->rocid < 25) continue;
      //      if (tt->rocid > 28) continue;

      ttime = (ULong64_t)tt->time;

      rocid = tt->rocid;
      slot = tt->slot;
      itrigger = tt->itrigger;

      tdiff =  (eventnum & 0xFFFF) - itrigger;

      japp->RootWriteLock(); //ACQUIRE ROOT LOCK!!

      tree->Fill();

      japp->RootUnLock();

    }  // for each Df125TriggerTime

  

  }  // if (nd)  


  return NOERROR;

}

//------------------
// erun
//------------------
jerror_t JEventProcessor_fa125_itrig::erun(void)
{
	// This is called whenever the run number changes, before it is
	// changed to give you a chance to clean up before processing
	// events from the next run number.
	return NOERROR;
}

//------------------
// fini
//------------------
jerror_t JEventProcessor_fa125_itrig::fini(void)
{
	// Called before program exit after event processing is finished.
	return NOERROR;
}

