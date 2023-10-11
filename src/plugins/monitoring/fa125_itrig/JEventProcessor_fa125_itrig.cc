// $Id$
//
//    File: JEventProcessor_fa125_itrig.cc
// Created: Sat Dec  6 21:34:19 EST 2014
// Creator: njarvis (on Linux maria 2.6.32-431.20.3.el6.x86_64 x86_64)
//

// Looks for differences between Df125triggertime's itrigger and the eventnumber,
// which could indicate the fadc getting out of sync.
// Also compares the trigger time of each fadc to that of the others.

// Increments a 2D histogram(roc,slot) each time a difference is found.
// For normal runs the histogram looks like noise; bad runs look as if there are hot channels, with larger z scale.

// Use -Pfa125_itrig:MAKE_TREE=1 to produce tree output


#include "JEventProcessor_fa125_itrig.h"

using namespace std;

#include <JANA/JFactoryT.h>

#include <stdint.h>
#include <vector>
#include <bitset>

#include "DAQ/Df125TriggerTime.h"
#include "DAQ/DCODAEventInfo.h"

#include <TDirectory.h>
#include <TH1.h>
#include <TH2.h>
#include <TTree.h>
#include <TBranch.h>
#include <TMath.h>
#include <TBits.h>

static TH2I *hdiffs = NULL;
static TH1I *hevents = NULL;
static TTree *tree = NULL;



extern "C"{
void InitPlugin(JApplication *app){
	InitJANAPlugin(app);
	app->Add(new JEventProcessor_fa125_itrig());
}
} // "C"


//------------------
// JEventProcessor_fa125_itrig (Constructor)
//------------------
JEventProcessor_fa125_itrig::JEventProcessor_fa125_itrig()
{
	SetTypeName("JEventProcessor_fa125_itrig");
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
void JEventProcessor_fa125_itrig::Init()
{
	// This is called once at program startup. If you are creating
	// and filling histograms in this plugin, you should lock the
	// ROOT mutex like this:
	//

  MAKE_TREE = 0;
  auto app = GetApplication();
  lockService = app->GetService<JLockService>();
  app->SetDefaultParameter("fa125_itrig:MAKE_TREE", MAKE_TREE, "Make a ROOT tree file");

  lockService->RootWriteLock();

  TDirectory *main = gDirectory;
  gDirectory->mkdir("fa125_itrig")->cd();

  hdiffs = new TH2I("errcount","Count of fa125 itrigger time errors; roc ; slot", 15, 1, 16, 17, 3, 20);

  hevents = new TH1I("num_events","Number of events", 1, 0.0, 1.0);

  for (int i=0; i<70; i++) rocmap[i] = 0;  // rocmap[rocid] = bin number for roc rocid in histogram

  int xlabels[70] = {0};
  int nbins;

  for (int i=25; i<29; i++) {
    int x = i-24;           // CDC, bins 1 to 4
    rocmap[i] = x;
    xlabels[x] = i;         // histo label
  }

  for (int i=52; i<54; i++) {
    int x = i-46;           // FDC, bins 6-7
    rocmap[i] = x;
    xlabels[x] = i;         // histo label
  }

  for (int i=55; i<63; i++) {
    int x = i-47;           // FDC, bins 8-15
    rocmap[i] = x;
    xlabels[x] = i;         // histo label
    nbins = x;
  }

  // ROCs 51,54,63 and 64 are TDCs.

  for (int i=1;i<=nbins;i++) {
    if (xlabels[i]>0) {
         hdiffs->GetXaxis()->SetBinLabel(i,Form("%i",xlabels[i]));
    } else {
         hdiffs->GetXaxis()->SetBinLabel(i," ");
    }
  }

  if (MAKE_TREE) {
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

    int itrigdiff;
    tree->Branch("itrigdiff",&itrigdiff,"itrigdiff/I");

  }

  main->cd();

}

//------------------
// BeginRun
//------------------
void JEventProcessor_fa125_itrig::BeginRun(const std::shared_ptr<const JEvent> &event)
{
	// This is called whenever the run number changes
}

//------------------
// Process
//------------------
void JEventProcessor_fa125_itrig::Process(const std::shared_ptr<const JEvent> &event)
{
  // This is called for every event.
  auto eventnumber = event->GetEventNumber();

  // Event count used by RootSpy->RSAI so it knows how many events have been seen.
  hevents->Fill(0.5);

  ULong64_t timestamp = 0;

  if (MAKE_TREE) {
    vector<const DCODAEventInfo*> info;
  
    event->Get(info);
  
    if (info.size() != 0) {
      timestamp = (ULong64_t)info[0]->avg_timestamp;
    }
  }

  
  vector<const Df125TriggerTime*> ttvector;
  event->Get(ttvector);

  int nd = (int)ttvector.size();

  if (nd) {

    ULong64_t eventnum = (ULong64_t)eventnumber;
    ULong64_t ttime; //trigger time
    uint32_t rocid, slot, itrigger;
    int tdiff, itrigdiff;
        
    if (MAKE_TREE) {

      tree->SetBranchAddress("eventnum",&eventnum);
      tree->SetBranchAddress("timestamp",&timestamp);
      tree->SetBranchAddress("ttime",&ttime);
      tree->SetBranchAddress("rocid",&rocid);
      tree->SetBranchAddress("slot",&slot);
      tree->SetBranchAddress("itrigger",&itrigger);
      tree->SetBranchAddress("tdiff",&tdiff);
      tree->SetBranchAddress("itrigdiff",&itrigdiff);
    }
  
    const Df125TriggerTime *tt = NULL;

    // assume several trigger times are reported, find out which is the most popular 

    ULong64_t ttimes[255] = {0};  // 15 rocs*17slots=255

    // load times into array 
    for (int i=0; i<nd; i++) {

      tt = ttvector[i];  
 
      if (!tt) continue;

      ttimes[i] = (ULong64_t)tt->time;

    }  // for each Df125TriggerTime


    int order[255] = {0};

    TMath::Sort(nd,ttimes,order,1);  // sort the array, largest first

    // count the unique ttimes
    ULong64_t unique_times[255] = {0}; 
    int count[255] = {0};
    int nunique = 0;

    for (int i=0; i<nd; i++) {
      if (ttimes[order[i]] == unique_times[nunique-1]) { 
        count[nunique-1]++;
      } else {
        unique_times[nunique] = ttimes[order[i]];
        count[nunique]++;
	nunique++;
      }
    }

    // find the most popular time.  If the top 2 are equally popular, just take the first.
    TMath::Sort(nunique,count,order,1);

    ULong64_t most_popular_time = unique_times[order[0]];

    for (int i=0; i<nd; i++) {

      tt = ttvector[i];  
 
      if (!tt) continue;

      ttime = (ULong64_t)tt->time;

      rocid = tt->rocid;
      slot = tt->slot;
      itrigger = tt->itrigger;

      itrigdiff =  (eventnum & 0xFFFF) - itrigger;

      tdiff = most_popular_time - ttime;

      // count the bits differing between ttime and most popular time
      // ttime often differs by 1 between L and R halves of the crate, this is not important. 
      // single bit differences in more significant bits happen infrequently when other output is ok

      ULong64_t posdiff;

      // Compare right-shifted ttime and most_popular_time to ignore LSB.

      if (ttime > most_popular_time) {
        posdiff = (ttime>>1) - (most_popular_time>>1);
      } else {
        posdiff = (most_popular_time>>1) - (ttime>>1);
      }

      UInt_t diffcount_ttime = 0;

      while (posdiff>0) {
        if (posdiff&1) diffcount_ttime++;
        posdiff = posdiff>>1;
      }

      // count the bits differing between itrigger and lowest 16 bits of eventnumber
      // single bit differences are fairly common
      
      if ((eventnum & 0xFFFF) - itrigger > 0) {
        posdiff = (ULong64_t)((eventnum & 0xFFFF) - itrigger);
      } else {
        posdiff = (ULong64_t)(itrigger - (eventnum & 0xFFFF));
      }

      UInt_t diffcount_itrig = 0;

      while (posdiff>0) {
        if (posdiff&1) diffcount_itrig++;
        posdiff = posdiff>>1;
      }

      lockService->RootWriteLock(); //ACQUIRE ROOT LOCK!!

      // increment monitoring histo when trigger time and itrigger are both off by more than 1 bit

      if ( diffcount_ttime>1 && diffcount_itrig>1 ) hdiffs->Fill(rocmap[rocid],slot,1);  

      if (MAKE_TREE) tree->Fill();

      lockService->RootUnLock();

    }  // for each Df125TriggerTime


  }  // if (nd)  

}

//------------------
// EndRun
//------------------
void JEventProcessor_fa125_itrig::EndRun()
{
	// This is called whenever the run number changes, before it is
	// changed to give you a chance to clean up before processing
	// events from the next run number.
}

//------------------
// Finish
//------------------
void JEventProcessor_fa125_itrig::Finish()
{
	// Called before program exit after event processing is finished.
}

