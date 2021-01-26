// $Id$
//
//    File: JEventProcessor_cdc_scan.cc
// Created: Sat Dec  6 21:34:19 EST 2014
// Creator: njarvis (on Linux maria 2.6.32-431.20.3.el6.x86_64 x86_64)
//

// Use -PCDC_SCAN:SHORT_MODE=0 to include window raw data, otherwise short mode is assumed


#include <stdint.h>
#include <vector>


#include "JEventProcessor_cdc_scan.h"
#include <JANA/JApplication.h>

using namespace std;
using namespace jana;


#include "CDC/DCDCDigiHit.h"
#include "DAQ/Df125WindowRawData.h"     
#include "DAQ/Df125CDCPulse.h"
#include "DAQ/Df125TriggerTime.h"
#include "TRIGGER/DTrigger.h"


#include <TTree.h>
#include <TBranch.h>



static TTree *t = NULL;
static TTree *p = NULL;
static TTree *w = NULL;
static TTree *tt = NULL;


extern "C"{
void InitPlugin(JApplication *app){
	InitJANAPlugin(app);
	app->AddProcessor(new JEventProcessor_cdc_scan());
}
} // "C"


//------------------
// JEventProcessor_cdc_scan (Constructor)
//------------------
JEventProcessor_cdc_scan::JEventProcessor_cdc_scan()
{

}

//------------------
// ~JEventProcessor_cdc_scan (Destructor)
//------------------
JEventProcessor_cdc_scan::~JEventProcessor_cdc_scan()
{

}

//------------------
// init
//------------------
jerror_t JEventProcessor_cdc_scan::init(void)
{
	// This is called once at program startup. If you are creating
	// and filling histograms in this plugin, you should lock the
	// ROOT mutex like this:
	//

  const uint32_t NSAMPLES = 200;

  SHORT_MODE = 1;    // suppresses use of window raw data and warnings if not found

  if (gPARMS) {
    gPARMS->SetDefaultParameter("CDC_SCAN:SHORT_MODE",SHORT_MODE,"Set to 0 to include window raw data");
  }

  japp->RootWriteLock();



  if (SHORT_MODE)  cout << "\n cdc_scan: suppressing WRD info\n\n";

  t = new TTree("T","Event stats");

  ULong64_t t_eventnum;
  t->Branch("eventnum",&t_eventnum,"eventnum/l");

  uint32_t nd;
  t->Branch("Digihitcount",&nd,"nd/i");

  uint32_t ncp;
  t->Branch("CDCPulsecount",&ncp,"ncp/i");

  uint32_t nfp;
  t->Branch("FDCPulsecount",&nfp,"nfp/i");

  uint32_t nw;
  t->Branch("WindowRawDatacount",&nw,"nw/i");

  uint32_t nu;
  t->Branch("Unpairedobjectcount",&nu,"nu/i");



  p = new TTree("P","CDC pulse data");


  ULong64_t p_eventnum;
  p->Branch("eventnum",&p_eventnum,"eventnum/l");

  uint32_t p_rocid;
  p->Branch("rocid",&p_rocid,"rocid/i");

  uint32_t p_slot;
  p->Branch("slot",&p_slot,"slot/i");

  uint32_t p_channel;
  p->Branch("channel",&p_channel,"channel/i");

  uint32_t p_itrigger;
  p->Branch("itrigger",&p_itrigger,"itrigger/i");

  uint32_t word1;
  p->Branch("word1",&word1,"word1/i");

  uint32_t word2; 
  p->Branch("word2",&word2,"word2/i");

  uint32_t time;   
  p->Branch("time",&time,"time/i");    

  uint32_t q;
  p->Branch("q",&q,"q/i");    

  uint32_t pedestal;  
  p->Branch("pedestal",&pedestal,"pedestal/i");    

  uint32_t integral;
  p->Branch("integral",&integral,"integral/i");    

  uint32_t amp;
  p->Branch("amp",&amp,"amp/i");    

  uint32_t overflows; 
  p->Branch("overflows",&overflows,"overflows/i");    

  bool emulated; 
  p->Branch("emulated",&emulated,"emulated/O");    

  bool paired;
  p->Branch("paired",&paired,"paired/O");

  bool cdc;
  p->Branch("cdcdata",&cdc,"cdc/O");



  w = new TTree("W","CDC window raw data");

  ULong64_t w_eventnum;
  w->Branch("eventnum",&w_eventnum,"eventnum/l");

  uint32_t w_rocid;
  w->Branch("rocid",&w_rocid,"rocid/i");

  uint32_t w_slot;
  w->Branch("slot",&w_slot,"slot/i");

  uint32_t w_channel;
  w->Branch("channel",&w_channel,"channel/i");

  uint32_t w_itrigger;
  w->Branch("itrigger",&w_itrigger,"itrigger/i");

  uint32_t ns;
  w->Branch("nsamples",&ns,"nsamples/i");

  uint16_t adc[NSAMPLES];
  w->Branch("adc",&adc,Form("adc[%i]/s",NSAMPLES));         
 
  w->Branch("paired",&paired,"paired/O");
  

  tt = new TTree("TT","Trigger time");

  ULong64_t tt_eventnum;
  tt->Branch("eventnum",&tt_eventnum,"eventnum/l");

  uint32_t tt_rocid;
  tt->Branch("rocid",&tt_rocid,"rocid/i");

  uint32_t tt_slot;
  tt->Branch("slot",&tt_slot,"slot/i");

  uint32_t tt_itrigger;
  tt->Branch("itrigger",&tt_itrigger,"itrigger/i");

  uint64_t tt_time;
  tt->Branch("time",&tt_time,"time/l");

  japp->RootUnLock();


  return NOERROR;
}

//------------------
// brun
//------------------
jerror_t JEventProcessor_cdc_scan::brun(JEventLoop *eventLoop, int32_t runnumber)
{
	// This is called whenever the run number changes
	return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t JEventProcessor_cdc_scan::evnt(JEventLoop *loop, uint64_t eventnumber)
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



  // Only look at physics triggers

  
  const DTrigger* locTrigger = NULL; 
  loop->GetSingle(locTrigger); 
  if(locTrigger->Get_L1FrontPanelTriggerBits() != 0)
    return NOERROR;
  if (!locTrigger->Get_IsPhysicsEvent()){ // do not look at PS triggers
    return NOERROR;
  }


  //   cout << "Event " << eventnumber << endl;


  // get raw data for cdc
  vector<const DCDCDigiHit*> digihits;
  loop->Get(digihits);

  vector<const Df125WindowRawData*> wrdvector;
  loop->Get(wrdvector);

  vector<const Df125TriggerTime*> ttvector;
  loop->Get(ttvector);

  uint32_t nd = (uint32_t)digihits.size();
  uint32_t nw = (uint32_t)wrdvector.size();
  uint32_t ntt = (uint32_t)ttvector.size();

  
  if (ntt > 0) { //   Df125TriggerTime 

    japp->RootWriteLock(); //ACQUIRE ROOT LOCK!!


    ULong64_t tt_eventnum;
    tt->SetBranchAddress("eventnum",&tt_eventnum);

    uint32_t tt_rocid;
    tt->SetBranchAddress("rocid",&tt_rocid);

    uint32_t tt_slot;
    tt->SetBranchAddress("slot",&tt_slot);

    uint32_t tt_itrigger;
    tt->SetBranchAddress("itrigger",&tt_itrigger);

    ULong64_t tt_time;
    tt->SetBranchAddress("time",&tt_time);

    tt_eventnum = eventnumber;

    for (uint32_t i=0; i<ntt; i++) {

      const Df125TriggerTime *thistt = ttvector[i];

      if (thistt->rocid < 25 || thistt->rocid > 28) continue;   // skip FDC

      //      cout << thistt->rocid << " " << thistt->slot << " " << thistt->itrigger << " " << thistt->time << endl;

      tt_rocid = thistt->rocid;
      tt_slot = thistt->slot;
      tt_itrigger = (uint32_t)thistt->itrigger;
      tt_time = thistt->time;

      tt->Fill();
    }
    japp->RootUnLock();

  }


  uint32_t nhits = nd;  
  if (nw>nd) nhits = nw;

  //  if (!nhits) printf("No hits in event %li\n",eventnumber);

  if (nhits) {

    japp->RootWriteLock(); //ACQUIRE ROOT LOCK!!

    const uint32_t NSAMPLES = 200;
    const uint32_t NSAMPLESFDC = 100;


    t->SetBranchAddress("Digihitcount",&nd);
    t->SetBranchAddress("WindowRawDatacount",&nw);


    ULong64_t t_eventnum;
    t->SetBranchAddress("eventnum",&t_eventnum);

    uint32_t ncp;
    t->SetBranchAddress("CDCPulsecount",&ncp);

    uint32_t nfp;
    t->SetBranchAddress("FDCPulsecount",&nfp);

    uint32_t nu;
    t->SetBranchAddress("Unpairedobjectcount",&nu);



    ULong64_t p_eventnum;
    p->SetBranchAddress("eventnum",&p_eventnum);

    uint32_t p_rocid;
    p->SetBranchAddress("rocid",&p_rocid);

    uint32_t p_slot;
    p->SetBranchAddress("slot",&p_slot);

    uint32_t p_channel;
    p->SetBranchAddress("channel",&p_channel);

    uint32_t p_itrigger;
    p->SetBranchAddress("itrigger",&p_itrigger);

    uint32_t word1;
    p->SetBranchAddress("word1",&word1);

    uint32_t word2; 
    p->SetBranchAddress("word2",&word2);

    uint32_t time;   
    p->SetBranchAddress("time",&time);

    uint32_t q;
    p->SetBranchAddress("q",&q);

    uint32_t pedestal;  
    p->SetBranchAddress("pedestal",&pedestal);

    uint32_t integral;
    p->SetBranchAddress("integral",&integral);

    uint32_t amp;
    p->SetBranchAddress("amp",&amp);

    uint32_t overflows; 
    p->SetBranchAddress("overflows",&overflows);

    bool emulated; 
    p->SetBranchAddress("emulated",&emulated);

    bool paired;
    p->SetBranchAddress("paired",&paired);

    bool cdc;
    p->SetBranchAddress("cdcdata",&cdc);




    ULong64_t w_eventnum;
    w->SetBranchAddress("eventnum",&w_eventnum);

    uint32_t w_rocid;
    w->SetBranchAddress("rocid",&w_rocid);

    uint32_t w_slot;
    w->SetBranchAddress("slot",&w_slot);

    uint32_t w_channel;
    w->SetBranchAddress("channel",&w_channel);

    uint32_t w_itrigger;
    w->SetBranchAddress("itrigger",&w_itrigger);

    uint32_t ns;
    w->SetBranchAddress("nsamples",&ns);

    uint16_t adc[NSAMPLES];       //    vector<uint16_t> samples;
    w->SetBranchAddress("adc",&adc);
 
    w->SetBranchAddress("paired",&paired);



    uint32_t i,j;


    ncp = 0;
    nfp = 0;
    nu = 0;


    t_eventnum = eventnumber;

    //    printf("%i hits \n",(int)nhits); //temp


    // look through digihits, find CP and look for associated WRD----------------------

    for (i=0; i<nd; i++) {

      p_eventnum = 0;
      w_eventnum = 0;


      p_rocid = 0; 
      p_slot = 0; 
      p_channel = 0;
      p_itrigger = 0;

      time = 0;
      q = 0;
      overflows = 0;  
      pedestal = 0;
      amp = 0;
      integral = 0; 

      word1 = 0;
      word2 = 0;

      emulated = 0;
      paired = 0;


      w_rocid = 0; 
      w_slot = 0; 
      w_channel = 0;
      w_itrigger = 0;

  
      for (j=0; j<NSAMPLES; j++) {
        adc[j] = 0; 
      }
  

      const DCDCDigiHit *digihit = NULL;
      if (i < nd) digihit = digihits[i];  

      const Df125CDCPulse *cp = NULL;
      if (digihit) digihit->GetSingle(cp);


      const Df125WindowRawData *wrd = NULL;

      if (!SHORT_MODE) {
        if (cp) cp->GetSingle(wrd);
        //if (wrd) printf("Found cp->GetSingle(wrd)\n");
        if (cp && !wrd) printf("Did not find cp->GetSingle(wrd)\n");
      }

      if (cp) ncp++;


      if (!cp) printf("\nMissing CDCPulse in event %li \n\n",eventnumber);
      if (!SHORT_MODE) {
        if (!wrd) printf("\nMissing WRD in event %li \n\n",eventnumber);
      }

      if (!digihit) continue;



      uint32_t nexpect = NSAMPLES;  //set to CDC 


      if (cp) {

        cdc = 1;

        p_eventnum = eventnumber;
        p_rocid = cp->rocid;
        p_slot = cp->slot;
        p_channel = cp->channel;
        p_itrigger = cp->itrigger;

        word1 = cp->word1;
        word2 = cp->word2;
        time = cp->le_time;
        pedestal = cp->pedestal;
        integral = cp->integral;
        q = cp->time_quality_bit;
        overflows = cp->overflow_count;
        amp = cp->first_max_amp;

        emulated = cp->emulated;
      }


      if (wrd) {

        w_eventnum = eventnumber;
        w_rocid = wrd->rocid;
        w_slot = wrd->slot;
        w_channel = wrd->channel;
        w_itrigger = wrd->itrigger;

        ns = (uint32_t)wrd->samples.size();
  
        for (j=0; j<ns; j++) {
          adc[j] = wrd->samples[j];
        }

        if (w_rocid<24 || w_rocid>28) nexpect = NSAMPLESFDC;
        if (!w_rocid) nexpect = 0;

        if (ns != nexpect) {
          printf("\nFound %u WRD samples for event %llu roc %u slot %u channel %u \n\n",ns,w_eventnum,w_rocid,w_slot,w_channel);
        }


        if (w_eventnum == p_eventnum) {

          paired = 1;

          if (p_rocid != w_rocid) paired = 0;
          if (p_slot != w_slot) paired = 0;
          if (p_channel != w_channel) paired = 0;
          if (p_itrigger != w_itrigger) paired = 0;

        } 

        if (!paired) nu++;
      
      } //wrd

      //if (!paired) printf("  Mismatch for p_eventnum %i CDCPulse roc %i slot %i ch %i trig %i with WRD roc %i slot %i ch %i trig %i\n",p_eventnum,rocid,slot,channel,itrigger,wrd->rocid,wrd->slot,wrd->channel,wrd->itrigger);

      if (!SHORT_MODE) w->Fill();
      p->Fill();

    }  //nhits

    //    cout << "Event " << eventnumber;  
    //    printf(" PulseData: %u CDCPulse: %u FDCPulse: %u WindowData: %u Unpaired data: %u\n",nd,ncp,nfp,nw,nu);





    // look through wrd ----------------------------



    for (i=0; i<nw; i++) {

      const Df125WindowRawData *wrd = wrdvector[i];
      if (wrd->rocid>28) nfp++; 
      if (wrd->rocid>28) continue; //skip fdc
      //printf("wrd->rocid %i\n",wrd->rocid);

      const Df125CDCPulse *cp = NULL;
      wrd->GetSingle(cp);

      if (cp) continue; //should have found this one already via digihits

      //if (cp) printf("Found wrd->GetSingle(cp)\n");
      if (!cp) printf("Did not find wrd->GetSingle(cp)\n");

      if (!cp) printf("\nMissing CDCPulse in eventnum %li \n\n",eventnumber);
      if (!wrd) printf("\nMissing WRD in eventnum %li \n\n",eventnumber);


  
      for (j=0; j<NSAMPLES; j++) {
        adc[j] = 0; 
      }
  
      uint32_t nexpect = NSAMPLES;  //set to CDC 


        w_eventnum = eventnumber;
        w_rocid = wrd->rocid;
        w_slot = wrd->slot;
        w_channel = wrd->channel;
        w_itrigger = wrd->itrigger;

        ns = (uint32_t)wrd->samples.size();
  
        for (j=0; j<ns; j++) {
          adc[j] = wrd->samples[j];
        }

        if (w_rocid<24 || w_rocid>28) nexpect = NSAMPLESFDC;
        if (!w_rocid) nexpect = 0;

        if (ns != nexpect) {
          printf("\nFound %u WRD samples for event %llu roc %u slot %u channel %u \n\n",ns,w_eventnum,w_rocid,w_slot,w_channel);
        }


      w->Fill();
 
    }  //nhits

    //    cout << "Event " << eventnumber;  
    //    printf(" PulseData: %u CDCPulse: %u FDCPulse: %u WindowData: %u Unpaired data: %u\n",nd,ncp,nfp,nw,nu);





    if (nd||nw)  t->Fill();

    //    if (nd||nw) printf("filling tree T\n");


    japp->RootUnLock();


  }  // if (nhits)




  return NOERROR;

}

//------------------
// erun
//------------------
jerror_t JEventProcessor_cdc_scan::erun(void)
{
	// This is called whenever the run number changes);
	// changed to give you a chance to clean up before processing
	// events from the next run number.
	return NOERROR;
}

//------------------
// fini
//------------------
jerror_t JEventProcessor_cdc_scan::fini(void)
{
	// Called before program exit after event processing is finished.
	return NOERROR;
}

