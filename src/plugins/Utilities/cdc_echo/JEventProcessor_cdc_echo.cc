// $Id$
//
//    File: JEventProcessor_cdc_echo.cc
// Created: Sat Dec  6 21:34:19 EST 2014
// Creator: njarvis (on Linux maria 2.6.32-431.20.3.el6.x86_64 x86_64)

// track down small pulses 3-5 samples after large pulses

// Naomi Jarvis May 2024

#include <stdint.h>
#include <vector>
#include <TH1.h>

#include "JEventProcessor_cdc_echo.h"
#include <JANA/JApplication.h>

using namespace std;
using namespace jana;


#include "CDC/DCDCHit.h"
#include "CDC/DCDCDigiHit.h"

#include "DAQ/Df125WindowRawData.h"     
#include "DAQ/Df125CDCPulse.h"
#include "DAQ/Df125Config.h"
#include "TRIGGER/DTrigger.h"

#include <TRACKING/DTrackTimeBased.h>
#include <TRACKING/DTrackFitter.h>
#include "CDC/DCDCTrackHit.h"


#include <TTree.h>
#include <TBranch.h>



static TTree *T = NULL;
static TTree *TT = NULL;


extern "C"{
void InitPlugin(JApplication *app){
	InitJANAPlugin(app);
	app->AddProcessor(new JEventProcessor_cdc_echo());
}
} // "C"


//------------------
// JEventProcessor_cdc_echo (Constructor)
//------------------
JEventProcessor_cdc_echo::JEventProcessor_cdc_echo()
{

}

//------------------
// ~JEventProcessor_cdc_echo (Destructor)
//------------------
JEventProcessor_cdc_echo::~JEventProcessor_cdc_echo()
{

}

//------------------
// init
//------------------
jerror_t JEventProcessor_cdc_echo::init(void)
{
	// This is called once at program startup. If you are creating
	// and filling histograms in this plugin, you should lock the
	// ROOT mutex like this:
	//


  //  ECHO_ORIGIN = 4088;
  // gPARMS->SetDefaultParameter("CDC:ECHO_ORIGIN", ECHO_ORIGIN, "Min height (adc units 0-4095) for primary pulses considered in the search for afterpulses. Set to 4088 for the saturated value 511");

  ECHO_A = 500;
  gPARMS->SetDefaultParameter("CDC:ECHO_A", ECHO_A,
                              "Max height (adc units 0-4095) for afterpulses");

  ECHO_DT = 15;
  gPARMS->SetDefaultParameter("CDC:ECHO_DT", ECHO_DT,
                              "End of time range (number of samples) to search for afterpulses");


   // increment counts histo    0: tracks  1:all pulses  2:hits  3:hits on tracks  4: echo pulses  5: echo hits  6:  echoes on tracks  7: saturated pulses  8:saturated hits  9:saturated hits on tracks
  
  counts =  new TH1I("counts","0: tracks  1:pulses  2:hits  3:hits on tracks  4: echo pulses  5: echo hits  6: echoes on tracks  7: sat pulses  8:sat hits  9:sat hits on tracks",10,0,10);


  
  const uint32_t NSAMPLES = 200;

  ULong64_t eventnumber;  
  
  uint32_t rocid, slot, preamp, channel;
  uint32_t time, q, pedestal, amp, integral, overflows;

  uint16_t integral8;
  
  uint16_t ns;
  uint16_t adc[NSAMPLES];

  uint16_t ntrackhits,ntrackhits_sat,ntrackhits_echo;
  double FOM;

  japp->RootWriteLock();
  
  TT = new TTree("TT","Track data");

  TT->Branch("eventnum",&eventnumber,"eventnum/l");
  TT->Branch("ntrackhits",&ntrackhits,"ntrackhits/s");
  TT->Branch("nsat",&ntrackhits_sat,"nsat/s");
  TT->Branch("necho",&ntrackhits_echo,"necho/s");
  TT->Branch("FOM",&FOM,"FOM/D");

  
  T = new TTree("T","Echo Pulse data");

  T->Branch("eventnum",&eventnumber,"eventnum/l");
  T->Branch("rocid",&rocid,"rocid/i");
  T->Branch("slot",&slot,"slot/i");
  T->Branch("preamp",&preamp,"preamp/i");
  T->Branch("channel",&channel,"channel/i");

  T->Branch("time",&time,"time/i");    
  T->Branch("q",&q,"q/i");    
  T->Branch("pedestal",&pedestal,"pedestal/i");    
  T->Branch("amp",&amp,"amp/i");  
  T->Branch("integral",&integral,"integral/i");    
  T->Branch("overflows",&overflows,"overflows/i");    

  T->Branch("integral8",&integral8,"integral8/s");

  uint16_t timeovert;
  T->Branch("timeovert",&timeovert,"timeovert/s");
  
  uint32_t parentamp, parentint, trough, iovera;
  uint16_t dt, dc, nechoes, numsat, hitpresent, hitontrack;
  T->Branch("parentamp",&parentamp,"parentamp/i");
  T->Branch("parentint",&parentint,"parentint/i");
  T->Branch("nsat",&numsat,"nsat/s");      
  T->Branch("t",&trough,"t/i");    
  T->Branch("nechoes",&nechoes,"nechoes/s");    
  T->Branch("dt",&dt,"dt/s");    
  T->Branch("dc",&dc,"dc/s");    
  T->Branch("iovera",&iovera,"iovera/i");   
  T->Branch("hitpresent",&hitpresent,"hitpresent/s");
  T->Branch("hitontrack",&hitontrack,"hitontrack/s");      

  T->Branch("nsamples",&ns,"nsamples/s");
  
  T->Branch("adc",&adc,Form("adc[%i]/s",NSAMPLES));         
  
  japp->RootUnLock();


  return NOERROR;
}

//------------------
// brun
//------------------
jerror_t JEventProcessor_cdc_echo::brun(JEventLoop *eventLoop, int32_t runnumber)
{
	// This is called whenever the run number changes
	return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t JEventProcessor_cdc_echo::evnt(JEventLoop *loop, uint64_t eventnumber)
{
  
    // Only look at physics triggers
    
    const DTrigger* locTrigger = NULL; 
    loop->GetSingle(locTrigger); 
    if(locTrigger->Get_L1FrontPanelTriggerBits() != 0)
      return NOERROR;
    if (!locTrigger->Get_IsPhysicsEvent()){ // do not look at PS triggers
      return NOERROR;
    }
     
    vector <const Df125CDCPulse*> cdcpulses;
    loop->Get(cdcpulses);
    uint32_t nc = (uint32_t)cdcpulses.size();
  
    if (nc==0) return NOERROR;  // no CDC pulses
  
    
    vector <const DCDCHit*> cdchits;
    loop->Get(cdchits);
    uint32_t nh = (uint32_t)cdchits.size();
     
    const Df125Config* config = NULL;
    cdcpulses[0]->GetSingle(config);
  
    const uint16_t ABIT=config->ABIT;
    const uint16_t PBIT=config->PBIT; 
    const uint16_t NSAMPLES = config->NW;   // 200 for CDC;   // 100 for FDC    
 
    const uint16_t MIN_ORIG_AMP = 4088; // threshold for original pulses  

    // look for saturated pulses
        
    uint32_t rocid,slot,channel,preamp,time,q,pedestal,amp,integral,overflows;
    uint32_t rought, iovera;
    uint16_t nhit, dt, dc, ns;
    uint16_t timeovert;

    uint16_t adc[NSAMPLES] = {0}; 
  
    uint16_t nsat[4][15][3] = {0};   // number of saturated hits in each preamp
    uint32_t tsat[4][15][3][24] = {0};   // rough time of each saturated hit
    uint16_t csat[4][15][3][24] = {0};   // saturated chan num
    uint32_t asat[4][15][3][24] = {0};   // saturated chan amp
    uint32_t isat[4][15][3][24] = {0};   // saturated chan integral
    uint16_t esat[4][15][3][24] = {0};   // number of echoes found for each saturated chan num
  
    // record these so that i can check to see if the pulses made it into cdchits later on
    // cdchits also has some cuts on time window
    
    uint16_t parent[4][15][72] = {0};   // set to the index number of the parent saturated pulse
    uint32_t parentamp[4][15][72] = {0};   // set to the max amp of the parent saturated pulse
    uint32_t parentint[4][15][72] = {0};   // set to the integral of the parent saturated pulse    
    
      // sort(x[0][0], x[0][0]+4) will sort the first 4 values in x[0][0][]
  
    for (uint32_t i=0; i<nc; i++) {
  
        const Df125CDCPulse *cp = cdcpulses[i];
        rocid = cp->rocid;
        slot = cp->slot;
        channel = cp->channel;
        overflows = cp->overflow_count;
        preamp = (uint32_t)(channel/24);
        rought = (uint32_t)(cp->le_time/10);
        
        //      printf("rocid %i slot %i preamp %i chan %i rough_t %i amp %i overflows %i\n",rocid,slot,preamp,channel,rought, cp->first_max_amp, overflows);
  
        if (overflows == 0) continue;
        if (cp->first_max_amp<<ABIT < MIN_ORIG_AMP) continue;     
  
        uint16_t loc=nsat[rocid-25][slot-3][preamp];
  
        tsat[rocid-25][slot-3][preamp][loc] = rought; 
        csat[rocid-25][slot-3][preamp][loc] = (uint16_t)channel; 
        asat[rocid-25][slot-3][preamp][loc] = cp->first_max_amp;
        isat[rocid-25][slot-3][preamp][loc] = cp->integral;       
  
        nsat[rocid-25][slot-3][preamp]++;

	parent[rocid-25][slot-3][channel] = nsat[rocid-25][slot-3][preamp];    //n+1;   first parent has value 1, to avoid confusion w 0=no parent
  
    }
  
    
    // so now i have, for each preamp, nsat times in array tsat, not ordered in time

    for (uint32_t i=0; i<nc; i++) {
      
      const Df125CDCPulse *cp = cdcpulses[i];
      rocid = cp->rocid;
      slot = cp->slot;
      channel = cp->channel;
      preamp = (uint32_t)(channel/24);
      rought = (uint32_t)(cp->le_time/10);

      uint32_t net_amp = (cp->first_max_amp<<ABIT) - (cp->pedestal<<PBIT) ;
      
      uint16_t x=nsat[rocid-25][slot-3][preamp];           // number of sat hits for this preamp
       
      if (net_amp <= ECHO_A ) {         // check for echoes
	
        // assume there are many saturated pulses
	// assign the echo pulse to the saturated pulse that precedes it and is closest in time to it 

	uint16_t prevdt = 200; // impossibly big, so the first dt found will be smaller

        for (uint16_t n=0; n<x; n++) {	  

	  if (rought < tsat[rocid-25][slot-3][preamp][n]) continue; // too early
	  
  	  dt = (uint16_t)(rought - tsat[rocid-25][slot-3][preamp][n]);

          bool found = 0;  

          if ((dt>=2) && (dt<=ECHO_DT) && (dt < prevdt) ) found=1;
	  
	  if (found) {
	    prevdt = dt;
            parent[rocid-25][slot-3][channel] = n+1;   
            parentamp[rocid-25][slot-3][channel] = asat[rocid-25][slot-3][preamp][n];   
            parentint[rocid-25][slot-3][channel] = isat[rocid-25][slot-3][preamp][n];  
            esat[rocid-25][slot-3][preamp][n]++;   

           //printf("likely echo rocid %i slot %i preamp %i chan %i rough_t %i dt %i net amp %i\n",rocid,slot,preamp,channel,rought,dt, (cp->first_max_amp<<ABIT) - (cp->pedestal<<PBIT) );
	  }
        }
      }
    }

    // The echo identification has finished.  The code below finds more info to add to the tree & histo.
    
    
    // look to see if the problem pulses are present in cdchits too.  if we are not pruning, they will be.

    uint16_t cdchitpresent[4][15][72] = {0};
   
    for (uint32_t i=0; i<nh; i++) {
      
      const DCDCHit *hit = cdchits[i];

      vector <const Df125CDCPulse*> pulse;
      hit->Get(pulse);       
      
      const Df125CDCPulse *cp = pulse[0];

      rocid = cp->rocid;
      slot = cp->slot;
      channel = cp->channel;
      cdchitpresent[rocid-25][slot-3][channel] = 1;
      
    }
   
    
    // find out if the hits are on tracks or not.

    uint16_t ontrack[4][15][72] = {0};

    vector<const DTrackTimeBased*> tracks;
    loop->Get(tracks);

   
    for (uint32_t i=0; i<(uint32_t)tracks.size(); i++) {  
      
      const DTrackTimeBased *track = tracks[i];

      vector<const DCDCTrackHit*> locCDCHits;
      track->Get(locCDCHits);

      if (locCDCHits.size() == 0) continue;

      // increment counts histo    0: tracks  1:all pulses  2:hits  3:hits on tracks  4: echo pulses  5: echo hits  6:  echoes on tracks  7: saturated pulses  8:saturated hits  9:saturated hits on tracks

      japp->RootWriteLock();    
      counts->Fill(0);
      japp->RootUnLock();

      
      vector<DTrackFitter::pull_t> pulls = track->pulls;

      uint16_t ntrackhits = (uint16_t)pulls.size();
      uint16_t ntrackhits_sat = 0;
      uint16_t ntrackhits_echo = 0;      
      double FOM = track->FOM;
      
      for (uint32_t j=0; j<(uint32_t)pulls.size(); j++) {

        if (pulls[j].cdc_hit == NULL) continue;

        const DCDCHit *hit = NULL;
        pulls[j].cdc_hit->GetSingle(hit);

        const DCDCDigiHit *digihit = NULL;
	hit->GetSingle(digihit);

        const Df125CDCPulse *cp = NULL;		
        digihit->GetSingle(cp);

	rocid = cp->rocid;
        slot = cp->slot;
        channel = cp->channel;
        ontrack[rocid-25][slot-3][channel] = 1;

        if (cp->first_max_amp == 511) ntrackhits_sat++;
	if (parentamp[rocid-25][slot-3][channel] >0) ntrackhits_echo++;
      }
	
      japp->RootWriteLock();    

      TT->SetBranchAddress("eventnum",&eventnumber);
      TT->SetBranchAddress("ntrackhits",&ntrackhits);
      TT->SetBranchAddress("nsat",&ntrackhits_sat);
      TT->SetBranchAddress("necho",&ntrackhits_echo);
      TT->SetBranchAddress("FOM",&FOM);      

      TT->Fill();

      japp->RootUnLock();          

    }    


    

    // write arrays into the output tree

    
    for (uint32_t i=0; i<nc; i++) {
      
      const Df125CDCPulse *cp = cdcpulses[i];
      rocid = cp->rocid;
      slot = cp->slot;
      channel = cp->channel;
      preamp = (uint32_t)(channel/24);
      rought = (uint32_t)(cp->le_time/10);
      amp = cp->first_max_amp;
      
      nhit = parent[rocid-25][slot-3][channel] - 1;
      
      //      printf("rocid %i slot %i preamp %i chan %i rough_t %i dt %i amp %i parent %i echoes %i\n",rocid,slot,preamp,channel,rought,dt, cp->first_max_amp, parent[rocid-25][slot-3][channel], esat[rocid-25][slot-3][preamp][nhit]);


      // increment counts histo    0: tracks  1:all pulses  2:hits  3:hits on tracks  4: echo pulses  5: echo hits  6:  echoes on tracks  7: saturated pulses  8:saturated hits  9:saturated hits on tracks
      
      japp->RootWriteLock();

      counts->Fill(1);
      
      if ( cdchitpresent[rocid-25][slot-3][channel] >0 ) counts->Fill(2);       
      if ( ontrack[rocid-25][slot-3][channel] >0 ) counts->Fill(3);

      /*	      
      if ( parentamp[rocid-25][slot-3][channel] >0 ) {  // probable echoes
	counts->Fill(4);
        if ( cdchitpresent[rocid-25][slot-3][channel] >0 ) counts->Fill(5);      
        if ( ontrack[rocid-25][slot-3][channel] >0 ) counts->Fill(6);
	}*/

      if ( amp ==511 ) { // saturated pulses
	counts->Fill(7);
        if ( cdchitpresent[rocid-25][slot-3][channel] >0 ) counts->Fill(8);      
        if ( ontrack[rocid-25][slot-3][channel] >0 ) counts->Fill(9);
      }
           
      japp->RootUnLock();
      

      if (parent[rocid-25][slot-3][channel] == 0 ) continue; // not a parent or an echo

      dt = rought - tsat[rocid-25][slot-3][preamp][nhit];

      uint16_t nechoes = 0;
      nechoes = esat[rocid-25][slot-3][preamp][nhit];
      if (nechoes==0) continue; // saturated but no echoes 

      int temp = (int)channel - (int)csat[rocid-25][slot-3][preamp][nhit]; 
      if (temp<0) temp = -temp;
      dc = (uint16_t)(temp);

      uint16_t hitpresent = cdchitpresent[rocid-25][slot-3][channel];
      uint16_t hitontrack = ontrack[rocid-25][slot-3][channel];      

      uint32_t ptamp = parentamp[rocid-25][slot-3][channel];
      uint32_t ptint = parentint[rocid-25][slot-3][channel];      

      uint16_t numsat = nsat[rocid-25][slot-3][preamp];

      uint16_t integral8=0;  // integral over 8 samples, calculated later

    
      time = cp->le_time;
      pedestal = cp->pedestal;
      integral = cp->integral;
      q = cp->time_quality_bit;
      overflows = cp->overflow_count;


      iovera = (uint32_t)(integral/amp);
 
      const Df125WindowRawData *wrd;
      cp->GetSingle(wrd);

      timeovert = 0;
      
      if (wrd) {
        ns = (uint16_t)wrd->samples.size();

        uint16_t nsave = (ns<=NSAMPLES) ? ns : NSAMPLES ;  // save the first NSAMPLES values of the array
      
        for (uint16_t j=0; j<nsave; j++) adc[j] = wrd->samples[j];
        for (uint16_t j=nsave; j<NSAMPLES; j++) adc[j]=0;   
	
        for (uint16_t j=(uint16_t)rought; j<(uint16_t)rought+8 ; j++) {
  	  if (j==NSAMPLES) break;
    	  integral8 += adc[j];
        }
	
        uint16_t threshold = pedestal + 100;
	bool overthreshold = false;
	
        for (uint16_t j=(uint16_t)rought; j<(uint16_t)rought+20 ; j++) {
  	  if (j==NSAMPLES) break;
          if (adc[j]>threshold) overthreshold = true;
	  if (adc[j]>threshold && overthreshold) timeovert++;
	  if (adc[j]<threshold && overthreshold) break;
        }

      }
      

      ULong64_t eventnum = (ULong64_t)eventnumber;
      
      japp->RootWriteLock();    

      if ( parentamp[rocid-25][slot-3][channel] >0 && timeovert < 8 ) {  // probable echoes
	counts->Fill(4);
        if ( cdchitpresent[rocid-25][slot-3][channel] >0 ) counts->Fill(5);      
        if ( ontrack[rocid-25][slot-3][channel] >0 ) counts->Fill(6);
      }


	
      T->SetBranchAddress("eventnum",&eventnum);
      
      T->SetBranchAddress("rocid",&rocid);
      T->SetBranchAddress("slot",&slot);
      T->SetBranchAddress("channel",&channel);
      T->SetBranchAddress("preamp",&preamp);
  
      T->SetBranchAddress("time",&time);
      T->SetBranchAddress("q",&q);
      T->SetBranchAddress("pedestal",&pedestal);
      T->SetBranchAddress("amp",&amp);
      T->SetBranchAddress("integral",&integral);
      T->SetBranchAddress("overflows",&overflows);
      T->SetBranchAddress("integral8",&integral8);
      T->SetBranchAddress("timeovert",&timeovert);

      T->SetBranchAddress("t",&rought);
      //      T->SetBranchAddress("nhit",&nhit);    
      T->SetBranchAddress("nechoes",&nechoes);    
      T->SetBranchAddress("parentamp",&ptamp);
      T->SetBranchAddress("parentint",&ptint);      
      T->SetBranchAddress("nsat",&numsat);
      
      T->SetBranchAddress("dt",&dt);    
      T->SetBranchAddress("dc",&dc);    
      
      T->SetBranchAddress("iovera",&iovera);

      T->SetBranchAddress("hitpresent",&hitpresent);
      T->SetBranchAddress("hitontrack",&hitontrack);      

      T->SetBranchAddress("nsamples",&ns);
      T->SetBranchAddress("adc",&adc);
      
      T->Fill();
    
      japp->RootUnLock();    

    }
  
    return NOERROR;

}

//------------------
// erun
//------------------
jerror_t JEventProcessor_cdc_echo::erun(void)
{
	// This is called whenever the run number changes);
	// changed to give you a chance to clean up before processing
	// events from the next run number.
	return NOERROR;
}

//------------------
// fini
//------------------
jerror_t JEventProcessor_cdc_echo::fini(void)
{
	// Called before program exit after event processing is finished.
	return NOERROR;
}

