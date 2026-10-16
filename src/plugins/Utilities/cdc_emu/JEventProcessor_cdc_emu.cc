// $Id$
//
//    File: JEventProcessor_NSJ_cdc_m.cc
// Created: Thu Apr 21 10:38:27 EDT 2016
// Creator: njarvis (on Linux egbert 2.6.32-573.3.1.el6.x86_64 x86_64)
//

// read in thresholds from RunLog configuration files, filenames are hardcoded except for the run number.   TH and TL are hardcoded

// CDC:DIFFS_ONLY only write out events with differing fa125 output & emulation if this =1

// writes out reported and emulated fa125 quantities to tree_m.root
// the events appear out of order if run multithreaded

// uses fa125fns.h NOT new emulation in Df125EmulatorAlgorithm_v2
// (these should produce identical results)


#include "JEventProcessor_cdc_emu.h"



//static TTree *cdctree = NULL;

static Int_t roc25h[19][72];
static Int_t roc26h[19][72];
static Int_t roc27h[19][72];
static Int_t roc28h[19][72];



extern "C"{
void InitPlugin(JApplication *app){
	InitJANAPlugin(app);
	app->Add(new JEventProcessor_cdc_emu());
}
} // "C"

//define static local variable //declared in header file
thread_local DTreeFillData JEventProcessor_cdc_emu::dTreeFillData;

//------------------
// JEventProcessor_cdc_emu (Constructor)
//------------------
JEventProcessor_cdc_emu::JEventProcessor_cdc_emu()
{
	SetTypeName("JEventProcessor_cdc_emu");
}

//------------------
// ~JEventProcessor_cdc_emu (Destructor)
//------------------
JEventProcessor_cdc_emu::~JEventProcessor_cdc_emu()
{

}

//------------------
// Init
//------------------
void JEventProcessor_cdc_emu::Init()
{
  auto app = GetApplication();

  
  DIFFS_ONLY = 0;
  gPARMS->SetDefaultParameter("CDC:DIFFS_ONLY",DIFFS_ONLY,"Record (0) all events or (1) only those events where difference are found");


  //TTREE INTERFACE
  //MUST DELETE WHEN FINISHED: OR ELSE DATA WON'T BE SAVED!!!
  dTreeInterface = DTreeInterface::Create_DTreeInterface("CDC", "tree_m.root");

  //TTREE BRANCHES
  DTreeBranchRegister locTreeBranchRegister;

  locTreeBranchRegister.Register_Single<ULong64_t>("eventnum"); 
  locTreeBranchRegister.Register_Single<UInt_t>("rocid"); //uint32_t
  locTreeBranchRegister.Register_Single<UInt_t>("slot");
  locTreeBranchRegister.Register_Single<UInt_t>("channel"); 
  locTreeBranchRegister.Register_Single<UInt_t>("nsamples_pedestal"); 
  locTreeBranchRegister.Register_Single<UInt_t>("nsamples_integral"); 

  locTreeBranchRegister.Register_Single<Int_t>("ring");
  locTreeBranchRegister.Register_Single<Int_t>("straw");
  locTreeBranchRegister.Register_Single<Int_t>("n");
  locTreeBranchRegister.Register_Single<UInt_t>("time"); 
  locTreeBranchRegister.Register_Single<UInt_t>("q");
  locTreeBranchRegister.Register_Single<UInt_t>("pedestal"); 
  locTreeBranchRegister.Register_Single<UInt_t>("integral");
  locTreeBranchRegister.Register_Single<UInt_t>("amp");
  locTreeBranchRegister.Register_Single<UInt_t>("overflows"); 

  locTreeBranchRegister.Register_Single<Bool_t>("emulated"); 
  locTreeBranchRegister.Register_Single<Bool_t>("m_hitfound");

  locTreeBranchRegister.Register_Single<UInt_t>("m_hitsample");
  locTreeBranchRegister.Register_Single<UInt_t>("m_time");
  locTreeBranchRegister.Register_Single<UInt_t>("m_q");
  locTreeBranchRegister.Register_Single<UInt_t>("m_pedestal"); 
  locTreeBranchRegister.Register_Single<UInt_t>("m_integral");
  locTreeBranchRegister.Register_Single<UInt_t>("m_amp");
  locTreeBranchRegister.Register_Single<UInt_t>("m_overflows"); 

  locTreeBranchRegister.Register_Single<UInt_t>("m_initped");
  locTreeBranchRegister.Register_Single<UInt_t>("m_net_integral");

  locTreeBranchRegister.Register_Single<UInt_t>("s_pedestal");
  locTreeBranchRegister.Register_Single<UInt_t>("s_integral");
  locTreeBranchRegister.Register_Single<UInt_t>("s_amp");

  locTreeBranchRegister.Register_Single<UInt_t>("m_H");
  locTreeBranchRegister.Register_Single<UInt_t>("m_TH");
  locTreeBranchRegister.Register_Single<UInt_t>("m_TL");

  locTreeBranchRegister.Register_Single<UInt_t>("diffs");

  locTreeBranchRegister.Register_Single<Int_t>("d_time");
  locTreeBranchRegister.Register_Single<Int_t>("d_q");
  locTreeBranchRegister.Register_Single<Int_t>("d_pedestal");
  locTreeBranchRegister.Register_Single<Int_t>("d_integral");
  locTreeBranchRegister.Register_Single<Int_t>("d_amp");
  locTreeBranchRegister.Register_Single<Int_t>("d_overflows");

  locTreeBranchRegister.Register_Single<UInt_t>("NSAMPLES");
  locTreeBranchRegister.Register_FundamentalArray<UInt_t>("adc", "NSAMPLES");

  /*
	//SEARCH
	locTreeBranchRegister.Register_Single<UChar_t>("NumMatchingSCHits");
	locTreeBranchRegister.Register_FundamentalArray<Float_t>("TrackHitDeltaT", "NumMatchingSCHits"); //is signed: SC - Track

  */


  //REGISTER BRANCHES
  dTreeInterface->Create_Branches(locTreeBranchRegister);



}

//------------------
// BeginRun
//------------------
void JEventProcessor_cdc_emu::BeginRun(const std::shared_ptr<const JEvent>& event)
{
	// This is called whenever the run number changes


  //read in thresholds file

  int i;

  Char_t line[200];
  Int_t slotnum=0;
  Int_t junk;
  Float_t fjunk;
  Char_t filename[500];

  Int_t *harray;

  for (int iroc=1; iroc<5; iroc++) {


    sprintf(filename,"roccdc%i_fadc125.cnf",iroc);

    printf("Looking for config parameter file %s\n",filename);

    FILE *config = fopen(filename,"r");
    
    if (!config) printf("Could not open config file for cdcroc%i\n",iroc);
    if (!config) break;



    if (iroc==1) harray = &roc25h[0][0];
    if (iroc==2) harray = &roc26h[0][0];
    if (iroc==3) harray = &roc27h[0][0];
    if (iroc==4) harray = &roc28h[0][0];

 

    fscanf(config,"\n");    //new line
    fscanf(config,"################\n"); 
    fscanf(config,"CRATE    %s\n",line); 
    //printf("%s \n",line);  // print roc name
    fscanf(config,"################\n"); 

    fscanf(config,"\n\n"); //2 blank lines

    while (!feof(config)) {

      fscanf(config,"%s\n",line);
      if (!strcmp(line, "################\n")) break;
 
      //   fscanf(config,"FADC125_SLOTS   %i\n",&slotnum);
      fscanf(config,"%s   %i\n",line,&slotnum);
      fscanf(config,"################\n"); 

      //printf("%s slot %i\n",line,slotnum);

      fscanf(config,"\n\n"); //2 empty lines


      //fscanf(config,"FADC125_DAC_CH_00_17   ");
      fscanf(config,"%s   ",line);
      //printf("DAC: %s\n",line);
      if (!strcmp(line,"FADC125_DAC_CH_00_17   ")) printf("mismatch\n");
      if (!strcmp(line,"FADC125_DAC_CH_00_17   ")) break;

      for (i=0; i<18; i++) fscanf(config,"  %i",&junk);
      fscanf(config,"\n"); 

      fscanf(config,"FADC125_DAC_CH_18_35   ");
      for (i=0; i<18; i++) fscanf(config,"  %i",&junk);
      fscanf(config,"\n"); 

      fscanf(config,"FADC125_DAC_CH_36_53   ");
      for (i=0; i<18; i++) fscanf(config,"  %i",&junk);
      fscanf(config,"\n"); 

      fscanf(config,"FADC125_DAC_CH_54_71   ");
      for (i=0; i<18; i++) fscanf(config,"  %i",&junk);
      fscanf(config,"\n"); 

      fscanf(config,"\n\n"); //2 empty lines


      //fscanf(config,"FADC125_THR_CH_00_17   ");
      fscanf(config,"%s   ",line);
      //printf("THR: %s\n",line);
      if (!strcmp(line,"FADC125_THR_CH_00_17   ")) printf("THR mismatch\n");
      // fscanf(config,"FADC125_THR_CH_00_17   ");

      for (i=0; i<18; i++) fscanf(config,"  %i",harray+72*slotnum+i);
	//      for (i=0; i<18; i++) fscanf(config,"  %i",&harray[slotnum][i]);
      fscanf(config,"\n"); 
 
 
      fscanf(config,"FADC125_THR_CH_18_35   ");
      for (i=18; i<36; i++) fscanf(config,"  %i",harray+72*slotnum+i);
      //      for (i=18; i<36; i++) fscanf(config,"  %i",&harray[slotnum][i]);
      fscanf(config,"\n"); 

      fscanf(config,"FADC125_THR_CH_36_53   ");
      for (i=36; i<54; i++) fscanf(config,"  %i",harray+72*slotnum+i);
      //      for (i=36; i<54; i++) fscanf(config,"  %i",&harray[slotnum][i]);
      fscanf(config,"\n"); 

      fscanf(config,"FADC125_THR_CH_54_71   ");
      for (i=54; i<72; i++) fscanf(config,"  %i",harray+72*slotnum+i);
      //      for (i=54; i<72; i++) fscanf(config,"  %i",&harray[slotnum][i]);
      fscanf(config,"\n"); 


      fscanf(config,"\n\n"); //2 empty lines


      //fscanf(config,"FADC125_BL_CH_00_17   ");
      fscanf(config,"%s   ",line);
      //printf("BL: %s\n",line);
      if (!strcmp(line,"FADC125_BL_CH_00_17   ")) printf("BL mismatch\n");
      if (!strcmp(line,"FADC125_BL_CH_00_17   ")) break;


      // fscanf(config,"FADC125_BL_CH_00_17   ");
      for (i=0; i<18; i++) fscanf(config,"  %f",&fjunk);
      fscanf(config,"\n"); 

      fscanf(config,"FADC125_BL_CH_18_35   ");
      for (i=0; i<18; i++) fscanf(config,"  %f",&fjunk);
      fscanf(config,"\n"); 

      fscanf(config,"FADC125_BL_CH_36_53   ");
      for (i=0; i<18; i++) fscanf(config,"  %f",&fjunk);
      fscanf(config,"\n"); 

      fscanf(config,"FADC125_BL_CH_54_71   ");
      for (i=0; i<18; i++) fscanf(config,"  %f",&fjunk);
      fscanf(config,"\n"); 

      fscanf(config,"\n\n"); //2 empty lines


      //fscanf(config,"FADC125_SIG_CH_00_17   ");
      fscanf(config,"%s   ",line);
      //printf("SIG: %s\n",line);
      if (!strcmp(line,"FADC125_SIG_CH_00_17   ")) printf("SIG mismatch\n");
      if (!strcmp(line,"FADC125_SIG_CH_00_17   ")) break;

      // fscanf(config,"FADC125_SIG_CH_00_17   ");
      for (i=0; i<18; i++) fscanf(config,"  %f",&fjunk);
      fscanf(config,"\n"); 

      fscanf(config,"FADC125_SIG_CH_18_35   ");
      for (i=0; i<18; i++) fscanf(config,"  %f",&fjunk);
      fscanf(config,"\n"); 

      fscanf(config,"FADC125_SIG_CH_36_53   ");
      for (i=0; i<18; i++) fscanf(config,"  %f",&fjunk);
      fscanf(config,"\n"); 

      fscanf(config,"FADC125_SIG_CH_54_71   ");
      for (i=0; i<18; i++) fscanf(config,"  %f",&fjunk);
      fscanf(config,"\n"); 

      fscanf(config,"\n"); //1 empty line


      //fscanf(config,"FADC125_CH_ENB   %*s  %*s  %*s\n");
      fscanf(config,"%s   %*s  %*s  %*s\n",line);
      //printf("ENB: %s\n",line);
      fscanf(config,"\n"); //1 empty line
      //fscanf(config,"FADC125_CH_DIS   %*s  %*s  %*s\n");
      fscanf(config,"%s   %*s  %*s  %*s\n",line);
      //printf("DIS: %s\n",line);
      fscanf(config,"\n"); //1 empty line

      fscanf(config,"FADC125_TH  %i\n",&junk);
      //         printf("runconfig TH %i\n",junk);
      fscanf(config,"FADC125_TL  %i\n",&junk);
      //         printf("runconfig TL %i\n",junk);

      fscanf(config,"\n"); //1 empty line
    }
 // end of one roc.

    fclose(config);



   
    /*
    printf("\nroc %i\n",iroc);

    for (slotnum=3; slotnum<19; slotnum++) {
      if (slotnum==11||slotnum==12) continue;
      for (i=0; i<72; i++) {
        if (i>4 && i<70) continue;
        printf("hit thres slot %i ch %i %i\n",slotnum,i,*(harray+72*slotnum+i));
      }
    }
    */

    for (slotnum=3; slotnum<18; slotnum++) {

      if (slotnum==11 || slotnum==12) continue;
      if (iroc==2 && slotnum==17) continue;
      if (iroc==3 && slotnum==17) continue;

      for (i=0; i<72; i++) {
        if (*(harray+72*slotnum+i)==0) printf("hit thres not set for roccdc %i (rocid %i) slot %i ch %i\n",iroc,iroc+24,slotnum,i);
      }
    }


   
  }






	return;
}

//------------------
// Process
//------------------
void JEventProcessor_cdc_emu::Process(const std::shared_ptr<const JEvent>& event)
{

  // Only look at physics triggers
  
  const DTrigger* locTrigger = NULL; 
  loop->GetSingle(locTrigger); 
  if(locTrigger->Get_L1FrontPanelTriggerBits() != 0)
    return NOERROR;
  if (!locTrigger->Get_IsPhysicsEvent()){ // do not look at PS triggers
    return NOERROR;
  }

  
  // vector<const DCODAEventInfo*> info;

  // ULong64_t timestamp = 0;
  // uint64_t avg_timestamp = 0;

  // event->Get(info);

  // if (info.size() != 0) {
  //   //    printf("found eventinfo \n");
  //   //cout << "info size " << info.size() << endl;
  //   timestamp = (ULong64_t)info[0]->avg_timestamp;
  // }



  // get raw data for cdc
  vector<const DCDCDigiHit*> digihits;
  event->Get(digihits);
  uint32_t nd = (uint32_t)digihits.size();

  vector<const Df125WindowRawData*> wrdvector;
  event->Get(wrdvector);
  uint32_t nw = (uint32_t)wrdvector.size();

  uint32_t nhits = nd;  
  if (nw>nd) nhits = nw;


  if (nhits) {


    //    const Int_t WRITE_DIFFS_ONLY = 1; // only write out events where emulation and fadc results differ

    const Int_t NSAMPLES = 200;


    const int straw_offset[29] = {0,0,42,84,138,192,258,324,404,484,577,670,776,882,1005,1128,1263,1398,1544,1690,1848,2006,2176,2346,2528,2710,2907,3104,3313};

    ULong64_t eventnum;
    uint32_t rocid;
    uint32_t slot;
    uint32_t channel;
    uint32_t nsamples_pedestal; 
    uint32_t nsamples_integral;

    Int_t ring;
    Int_t straw;
    Int_t n;
    uint32_t time;   
    uint32_t q;
    uint32_t pedestal;  
    uint32_t integral;
    uint32_t amp;
    uint32_t overflows; 

    bool emulated; 

    uint32_t m_hitsample;
    uint32_t m_time;
    uint32_t m_q;
    uint32_t m_pedestal; 
    uint32_t m_integral;
    uint32_t m_amp;
    uint32_t m_overflows; 

    uint32_t m_initped;
    uint32_t m_net_integral;

    uint32_t s_pedestal;
    uint32_t s_integral;
    uint32_t s_amp;

    uint32_t m_H;
    uint32_t m_TH;
    uint32_t m_TL;

    uint32_t diffs;

    Int_t d_time;
    Int_t d_q;
    Int_t d_pedestal;
    Int_t d_integral;
    Int_t d_amp;
    Int_t d_overflows;


    Int_t adc[NSAMPLES];


    //different types for the fa125 algos, annoyingly
    Int_t tmp_time,tmp_q,tmp_overflows,tmp_pedestal,tmp_amp;
    Long_t tmp_integral;

    //  Int_t ndec;

    Int_t HIT_THRES = 0;  //now defined later
    //    const Int_t HIT_THRES = 120;   //110 for run 3923, 115 for run 4623, 120 for 4701+
    const Int_t HIGH_THRESHOLD = 60;
    const Int_t LOW_THRESHOLD = 10;

    const Int_t NU = 20;  //number of samples sent to time algo
    const Int_t PED = 5;  //sample to be used as pedestal for timing is in place 5
    const Int_t PG = 4;
    const Int_t XTHR_SAMPLE = PED + PG; 

    const Int_t NPED = 16;
    const Int_t NPED2 = 16;
  
    const Int_t IBIT = 4;
    const Int_t ABIT = 3;
    const Int_t PBIT = 0;
  
    const Int_t WINDOW_START = NPED; //first sample after ped
    const Int_t WINDOW_END = 179;  // last sample in buffer is NW-1, so WE is NW-1-NU
    const Int_t INT_END = 300;

    const uint32_t OMAX = 7; // field max for overflows
    const uint32_t PMAX = 255; //field max for pedestal
    const uint32_t AMAX = 511; //field max for max amp
    const uint32_t IMAX = 16383; //field max for integral

  
    Int_t adc_subset[NU]; 

    Int_t hitfound=0; //hit found or not (1=found,0=not)
    Int_t hitsample=0;  // if hit found, sample number of threshold crossing
    Int_t timesample=0;
  
    Int_t i,j;

    eventnum = (ULong64_t) event->GetEventNumber();

    uint32_t id = 0;

    uint32_t w_rocid, w_slot, w_channel;

    bool paired;

    const DCDCHit *hit = NULL;
    const DCDCDigiHit *digihit = NULL;
    const Df125CDCPulse *cp = NULL;
    //const Df125Config *cf = NULL;
    const Df125WindowRawData *wrd = NULL;

    for (id=0; id<nd; id++) {

      digihit = digihits[id];  

      digihit->GetSingle(cp);

      digihit->GetSingle(hit);

      if (!cp) printf("\nno cp - event %lu digihit %i\n\n",(long unsigned int)eventnum,(int)id);
      if (!cp) continue;

      // cp->GetSingle(cf);

      // if (!cf) printf("\nno cf - event %lu digihit %i\n\n",(long unsigned int)eventnum,(int)id);
      // if (!cf) continue;

      // m_H = (uint32_t)cf->H;
      // //printf ("config H is %i file H is %i \n",m_H,);


      cp->GetSingle(wrd);

      if (!wrd) printf("\nno wrd - event %lu roc %i slot %i chan %i \n\n",(long unsigned int)eventnum,cp->rocid,cp->slot,cp->channel);
      if (!wrd) continue;


      rocid = cp->rocid;
      slot = cp->slot;
      channel = cp->channel;
 
      w_rocid = wrd->rocid;
      w_slot = wrd->slot;
      w_channel = wrd->channel;
 


      paired = 1;

      if (rocid != w_rocid) paired = 0;
      if (slot != w_slot) paired = 0;
      if (channel != w_channel) paired = 0;
      //if (itrigger != w_itrigger) paired = 0;
      //if (itrigger != tt->itrigger) paired = 0;


      if (!paired) {
        cout << "Event " << eventnum << endl;
        cout << "cdcpulse roc " << rocid << " slot " << slot << " chan " << channel << endl;
        cout << "wrd      roc " << w_rocid << " slot " << w_slot << " chan " << w_channel << endl;
        //printf("cdcpulse roc %i slot %i chan %i trig %i \n",rocid,slot,channel,itrigger);
        //printf("wrd      roc %i slot %i chan %i trig %i \n",w_rocid,w_slot,w_channel,w_itrigger);
      }
         
      //      if (!paired) continue; 


      ring = digihit->ring;
      straw = digihit->straw;

      n = straw_offset[ring] + straw;

      time = digihit->pulse_time;
      integral = digihit->pulse_integral;
      pedestal = digihit->pedestal;

      q = cp->time_quality_bit;
      overflows = cp->overflow_count;
      amp = cp->first_max_amp;


      nsamples_pedestal = cp->nsamples_pedestal;
      nsamples_integral = cp->nsamples_integral;

      if (hit) {

        cout << "hit found " << endl;

        printf("roc %i slot %i chan %i time %i ped %i integral %i q %f \n",rocid,slot,channel,time,pedestal,integral,hit->q);

      //   cout << 

      //   cout << "CP nsamples_pedestal " << cp->nsamples_pedestal << endl;
      //   cout << "hit nsamples_pedestal " << hit->nsamples_pedestal << endl;
      }


      emulated = cp->emulated;


      for (j=0; j<(Int_t)wrd->samples.size(); j++) {
        adc[j] = (Int_t)wrd->samples[j];
        if (adc[j] > 4095) adc[j] = 4095;
      }

      for (j=(Int_t)wrd->samples.size(); j<NSAMPLES; j++) {
        adc[j] = 0; 
      }

        // //check for repeated words (2 samples)  //**** CHANGED THIS!
        // m_norpt = 0;
        // for (j=3; j<(Int_t)wrd->samples.size(); j+=2) {
        //   if ((adc[j]==adc[j-2]) && (adc[j-1]==adc[j-3])) m_norpt++;
        // }
	//        if (!m_norpt) printf("repeated WRD words eventnum %llu roc %i slot %i chan %i trig %i \n",eventnum,w_rocid,w_slot,w_channel,w_itrigger);

      m_hitsample = 0;

      m_time = 0;
      m_q = 0;
      m_overflows = 0;
      m_pedestal = 0;
      m_amp = 0;
      m_integral = 0;
      m_initped = 0;
      m_net_integral = 0;

      diffs = 0;
      d_time = 0;
      d_q = 0;
      d_overflows = 0;
      d_pedestal = 0;
      d_amp = 0;
      d_integral = 0;

      tmp_time = 0;
      tmp_q = 0;
      tmp_overflows = 0;
      tmp_pedestal = 0;
      tmp_amp = 0;
      tmp_integral = 0;

      hitfound = 0;
      hitsample = 0; 
  

      for (j=0; j<NU; j++) {
        adc_subset[j] = 0; 
      }


      if (rocid==25) {
        HIT_THRES = roc25h[slot][channel];
      } else if (rocid==26) {
        HIT_THRES = roc26h[slot][channel];
      } else if (rocid==27) {
        HIT_THRES = roc27h[slot][channel];
      } else if (rocid==28) {
        HIT_THRES = roc28h[slot][channel];
      }

      m_H = HIT_THRES; //just for output tree
      m_TH = HIGH_THRESHOLD;
      m_TL = LOW_THRESHOLD;

      //      printf("rocid %i slot %i channel %i config H %i Beni's file H %i\n",rocid,slot,channel,m_H,HIT_THRES);
    

      m_initped = 0;
      for (j=WINDOW_START-NPED; j<WINDOW_START; j++) {
        m_initped += (uint32_t)adc[j];
      }

      m_initped = (uint32_t)(m_initped/(uint32_t)NPED);

      //  const Int_t WINDOW_START = NPED; //first sample after ped
      // look for hit using mean pedestal of NPED samples before trigger 
      // cdc_hit first tests adc[window_start+pg]

      //      cdc_hit(hitfound, hitsample, tmp_pedestal, adc, WINDOW_START, WINDOW_END, HIT_THRES, NPED, NPED2, PG);





//alt code


      tmp_pedestal=0;  //pedestal
      Int_t threshold=0;


      // calc pedestal as mean of NPED samples before trigger
      for (i=0; i<NPED; i++) {
        tmp_pedestal += adc[WINDOW_START-NPED+i];
      }

      tmp_pedestal = ( NPED==0 ? 0:(tmp_pedestal/NPED) );   // Integer div is ok as fpga will do 2 rightshifts

      threshold = tmp_pedestal + HIT_THRES;

      // look for threshold crossing
      i = WINDOW_START - 1 + PG;
      hitfound = 0;

 
      //now look for adc val rising above threshold

      while ((hitfound==0) && (i<WINDOW_END-1)) {

        i++;

        if (adc[i] >= threshold) {
          if (adc[i+1] >= threshold) {
            hitfound = 1;
            hitsample = i;
          }
        }
      }

      if (hitfound == 1) {

        //calculate new pedestal ending just before the hit

        tmp_pedestal = 0;

        for (i=0; i<NPED2; i++) {
          tmp_pedestal += adc[hitsample-PG-i];
        }

        tmp_pedestal = ( NPED2==0 ? 0:(tmp_pedestal/NPED2) );
      }

//end alt code

      m_pedestal = (uint32_t)tmp_pedestal;


      if (hitfound==1) {

          m_hitsample = (uint32_t)hitsample;

          for (j=0; j<NU; j++) {
            adc_subset[j] = adc[hitsample+j-XTHR_SAMPLE];
          }

    	  //eg hitsample=20
          //subset[0] = adc[20-9=11]
          //subset[8] = adc[19]

	  

	  cdc_time(tmp_time, tmp_q, adc_subset, NU, PG, HIGH_THRESHOLD, LOW_THRESHOLD);

          timesample = hitsample-XTHR_SAMPLE + (Int_t)(0.1*tmp_time);  //sample number containing leading edge sample

  	  cdc_integral(tmp_integral, tmp_overflows, timesample, adc, WINDOW_END, INT_END);
  	  tmp_amp = (Int_t)adc[timesample];
          int pktime=0;
  	  cdc_max(tmp_amp, pktime, timesample, adc, WINDOW_END);


          m_time = (uint32_t)10*(uint32_t)(hitsample-XTHR_SAMPLE) + (uint32_t)tmp_time;   // integer number * 0.1 samples

	// //	printf("time %i em_time %i timesample %i \n",time,tmp_time,timesample);
	// //	printf("unscaled integ %i em_integ %i diff %i adc[WINDOW_END] %i \n",u_integral,tmp_integral,tmp_integral-u_integral,adc[WINDOW_END]);


          m_q = (uint32_t)tmp_q;
          m_integral = (uint32_t)tmp_integral;
          m_overflows = (uint32_t)tmp_overflows;
          m_amp = (uint32_t)tmp_amp;

          if (m_integral > m_pedestal*(WINDOW_END-timesample) ) m_net_integral = m_integral - m_pedestal*(WINDOW_END-timesample);


  	  s_pedestal = m_pedestal>>PBIT;
          if (s_pedestal > PMAX) s_pedestal = PMAX;

          s_amp = m_amp>>ABIT;
          if (s_amp > AMAX) s_amp = AMAX;

          s_integral = m_integral>>IBIT;
          if (s_integral > IMAX) s_integral = IMAX;

          d_time = (Int_t)m_time - (Int_t)time;

          if ((m_q > 0) && (q == 0) ) d_q = 1;  //because m_q can be 0-9
          if ((m_q == 0) && (q > 0) ) d_q = 1;  //because m_q can be 0-9

          d_overflows = (Int_t)m_overflows - (Int_t)overflows;
          if ((overflows==OMAX) && (m_overflows > overflows)) d_overflows = 0;

          d_pedestal = (Int_t)s_pedestal - (Int_t)pedestal;

          d_amp = (Int_t)s_amp - (Int_t)amp;

          d_integral = (Int_t)s_integral - (Int_t)integral;


          if (d_time || d_q || d_overflows || d_pedestal || d_amp || d_integral) diffs = 1;


      } //if hitfound


     
      if ((DIFFS_ONLY && diffs) || !DIFFS_ONLY) {

        dTreeFillData.Fill_Single<ULong64_t>("eventnum",eventnum);
        dTreeFillData.Fill_Single<UInt_t>("rocid",rocid);
	dTreeFillData.Fill_Single<UInt_t>("slot",slot);
	dTreeFillData.Fill_Single<UInt_t>("channel",channel);
	dTreeFillData.Fill_Single<UInt_t>("nsamples_pedestal",nsamples_pedestal); 
        dTreeFillData.Fill_Single<UInt_t>("nsamples_integral",nsamples_integral);

	dTreeFillData.Fill_Single<Int_t>("ring",ring);
	dTreeFillData.Fill_Single<Int_t>("straw",straw);
	dTreeFillData.Fill_Single<Int_t>("n",n);
	dTreeFillData.Fill_Single<UInt_t>("time",time);
        dTreeFillData.Fill_Single<UInt_t>("q",q);
	dTreeFillData.Fill_Single<UInt_t>("pedestal",pedestal); 
        dTreeFillData.Fill_Single<UInt_t>("integral",integral);
	dTreeFillData.Fill_Single<UInt_t>("amp",amp);
	dTreeFillData.Fill_Single<UInt_t>("overflows",overflows); 

        dTreeFillData.Fill_Single<Bool_t>("emulated",emulated);
        dTreeFillData.Fill_Single<Bool_t>("m_hitfound",hitfound);

	dTreeFillData.Fill_Single<UInt_t>("m_hitsample",m_hitsample);
	dTreeFillData.Fill_Single<UInt_t>("m_time",m_time);
	dTreeFillData.Fill_Single<UInt_t>("m_q",m_q);
	dTreeFillData.Fill_Single<UInt_t>("m_pedestal",m_pedestal); 
	dTreeFillData.Fill_Single<UInt_t>("m_integral",m_integral);
	dTreeFillData.Fill_Single<UInt_t>("m_amp",m_amp);
	dTreeFillData.Fill_Single<UInt_t>("m_overflows",m_overflows); 

	dTreeFillData.Fill_Single<UInt_t>("m_initped",m_initped);
	dTreeFillData.Fill_Single<UInt_t>("m_net_integral",m_net_integral);

	dTreeFillData.Fill_Single<UInt_t>("s_pedestal",s_pedestal);
	dTreeFillData.Fill_Single<UInt_t>("s_integral",s_integral);
	dTreeFillData.Fill_Single<UInt_t>("s_amp",s_amp);

	dTreeFillData.Fill_Single<UInt_t>("m_H",m_H);
	dTreeFillData.Fill_Single<UInt_t>("m_TH",m_TH);
	dTreeFillData.Fill_Single<UInt_t>("m_TL",m_TL);

	dTreeFillData.Fill_Single<UInt_t>("diffs",diffs);

	dTreeFillData.Fill_Single<Int_t>("d_time",d_time);
	dTreeFillData.Fill_Single<Int_t>("d_q",d_q);
	dTreeFillData.Fill_Single<Int_t>("d_pedestal",d_pedestal);
	dTreeFillData.Fill_Single<Int_t>("d_integral",d_integral);
	dTreeFillData.Fill_Single<Int_t>("d_amp",d_amp);
	dTreeFillData.Fill_Single<Int_t>("d_overflows",d_overflows);

	size_t index = 0;
        for (j=0; j<NSAMPLES; j++) {
          dTreeFillData.Fill_Array<UInt_t>("adc",adc[j],index);
          index++;
	}

	//FILL ARRAY SIZE
	dTreeFillData.Fill_Single<UInt_t>("NSAMPLES",index);

	/*
        vector <Int_t>samples;

        for (j=0; j<3; j++) samples.push_back(adc[j]);

	//FILL ARRAYS
	size_t locArrayIndex = 0;

	for(size_t loc_i = 0; loc_i < samples.size(); ++loc_i)
		{
		  Float_t temp = (Float_t)samples[loc_i];

			dTreeFillData.Fill_Array<Float_t>("TrackHitDeltaT", temp, locArrayIndex); //is signed: SC - Track

			++locArrayIndex;

		}
     

		//FILL ARRAY SIZE
		dTreeFillData.Fill_Single<UChar_t>("NumMatchingSCHits", locArrayIndex);

	*/






	//FILL TTREE
	dTreeInterface->Fill(dTreeFillData);


    
      }



    }  // for each hit (equiv)

  }  // if (nhits)

}

//------------------
// EndRun
//------------------
void JEventProcessor_cdc_emu::EndRun()
{
	// This is called whenever the run number changes, before it is
	// changed to give you a chance to clean up before processing
	// events from the next run number.
}

//------------------
// Finish
//------------------
void JEventProcessor_cdc_emu::Finish()
{
	// Called before program exit after event processing is finished.

        delete dTreeInterface; //saves trees to file, closes file

}

