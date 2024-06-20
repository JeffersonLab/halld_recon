// $Id$
//
//    File: JEventProcessor_cdc_echo.cc
// Created: Sat Dec  6 21:34:19 EST 2014
// Creator: njarvis (on Linux maria 2.6.32-431.20.3.el6.x86_64 x86_64)

// track down small pulses 3-5 samples after large pulses

// Naomi Jarvis May 2024

#include <stdint.h>
#include <vector>


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



#include <TTree.h>
#include <TBranch.h>



static TTree *T = NULL;


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


  /*  ECHO_ORIGIN = 4088;
  gPARMS->SetDefaultParameter("CDC:ECHO_ORIGIN", ECHO_ORIGIN,
                              "Min height (adc units 0-4095) for primary pulses considered in the search for afterpulses. Set to 4088 for the saturated value 511");

  ECHO_MAX_A = 500;
  gPARMS->SetDefaultParameter("CDC:ECHO_MAX_A", ECHO_MAX_A,
                              "Max height (adc units 0-4095) for afterpulses");

  ECHO_MAX_T = 7;
  gPARMS->SetDefaultParameter("CDC:ECHO_MAX_T", ECHO_MAX_T,
                              "End of time range (number of samples) to search for afterpulses");

  */

  const uint32_t NSAMPLES = 200;





  ULong64_t eventnum;  
  
  uint32_t rocid, slot, preamp, channel;
  uint32_t time, q, pedestal, amp, integral, overflows;
  
  uint32_t ns;
  uint16_t adc[NSAMPLES];

  japp->RootWriteLock();

  T = new TTree("T","Echo Pulse data");

  T->Branch("eventnum",&eventnum,"eventnum/l");

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

  T->Branch("nsamples",&ns,"nsamples/i");
  
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

  vector <const DCDCHit*> cdchits;
  loop->Get(cdchits);
  uint32_t nh = (uint32_t)cdchits.size();


  if (nc==0) return NOERROR;  // no DC hits

  const Df125Config* config = NULL;
  cdcpulses[0]->GetSingle(config);

  const uint16_t ABIT=config->ABIT;
  const uint16_t PBIT=config->PBIT;

  ULong64_t eventnum = (ULong64_t)eventnumber;

  const uint32_t NSAMPLES = 200;   // 100 for FDC


  const uint16_t MAX_ECHO_AMP = 500; //on scale 0-4095.  not an echo if larger. 

  const uint16_t MIN_ORIG_AMP = 4088; // threshold for original pulses

  const uint16_t MAX_DT = 15;

  // look for echo pulses

  
  if (nc) {

    uint32_t rocid,slot,channel,preamp,time,q,pedestal,amp,integral,overflows;
    uint32_t rought, iovera;
    uint16_t nhit, dt, dc, ns;

    uint16_t adc[NSAMPLES] = {0}; 

    uint16_t nsat[4][15][3] = {0};   // number of saturated hits in each preamp
    uint32_t tsat[4][15][3][24] = {0};   // rough time of each saturated hit
    uint16_t csat[4][15][3][24] = {0};   // saturated chan num
    uint32_t asat[4][15][3][24] = {0};   // saturated chan amp
    uint32_t isat[4][15][3][24] = {0};   // saturated chan integral
    uint16_t esat[4][15][3][24] = {0};   // number of echoes found for each saturated chan num


    // sort(x[0][0], x[0][0]+4) will sort the first 4 values in x[0][0][]

    for (uint32_t i=0; i<nc; i++) {

      const Df125CDCPulse *cp = cdcpulses[i];
      rocid = cp->rocid;
      slot = cp->slot;
      channel = cp->channel;
      overflows = cp->overflow_count;
      preamp = uint16_t(channel/24);
      rought = uint32_t(cp->le_time/10);
      
      //      printf("rocid %i slot %i preamp %i chan %i rough_t %i amp %i overflows %i\n",rocid,slot,preamp,channel,rought, cp->first_max_amp, overflows);

      if (overflows == 0) continue;
      if (cp->first_max_amp<<ABIT < MIN_ORIG_AMP) continue;     

      uint16_t loc=nsat[rocid-25][slot-3][preamp];

      tsat[rocid-25][slot-3][preamp][loc] = rought; 
      csat[rocid-25][slot-3][preamp][loc] = channel; 
      asat[rocid-25][slot-3][preamp][loc] = cp->first_max_amp;
      isat[rocid-25][slot-3][preamp][loc] = cp->integral;       

      nsat[rocid-25][slot-3][preamp]++;

    }

    
    // so now i have, for each preamp, nsat times in array tsat, not ordered in time

    //record these so that i can check cdchits later on
    
    uint16_t parent[4][15][72] = {0};   // set to the index number of the parent saturated pulse
    uint32_t parentamp[4][15][72] = {0};   // set to the max amp of the parent saturated pulse
    uint32_t parentint[4][15][72] = {0};   // set to the integral of the parent saturated pulse    

    for (uint32_t i=0; i<nc; i++) {
      
      const Df125CDCPulse *cp = cdcpulses[i];
      rocid = cp->rocid;
      slot = cp->slot;
      channel = cp->channel;
      preamp = uint16_t(channel/24);
      rought = uint32_t(cp->le_time/10);


      uint32_t amp = cp->first_max_amp<<ABIT;

      uint16_t x=nsat[rocid-25][slot-3][preamp];           // number of sat hits for this preamp
 
      // found is set true and parent array filled if this is a problem pulse

      if (amp >= MIN_ORIG_AMP && cp->overflow_count>0) {   //check sat hits
        bool found = 0;  
        for (uint32_t n=0; n<x; n++) {
          if (rought < tsat[rocid-25][slot-3][preamp][n]) continue; // too early
  	  dt = rought - tsat[rocid-25][slot-3][preamp][n]; 
          if (dt==0) found=1;
          if (found) parent[rocid-25][slot-3][channel] = n+1;   // set to itself
          //if (found) printf("likely parent rocid %i slot %i preamp %i chan %i rough_t %i dt %i amp %i\n",rocid,slot,preamp,channel,rought,dt, amp);
          if (found) break;
        }

      } else {   // check echoes
        bool found = 0;  
        uint32_t net_amp = (cp->first_max_amp<<ABIT) - (cp->pedestal<<PBIT) ;

        // assume there are many saturated pulses
	// assign the echo pulse to the saturated pulse that precedes it and is closest in time to it 

	uint16_t prevdt = 200; // impossibly big, so the first dt found will be smaller
	
        for (uint16_t n=0; n<x; n++) {
          if (rought < tsat[rocid-25][slot-3][preamp][n]) continue; // too early
	  
  	  dt = (uint16_t)(rought - tsat[rocid-25][slot-3][preamp][n]);
  
          if ((dt>=2) && (dt<=MAX_DT) && (dt < prevdt) && (net_amp <= MAX_ECHO_AMP)) found=1;

	  if (found) {
	    prevdt = dt;
            parent[rocid-25][slot-3][channel] = n+1;   //
            parentamp[rocid-25][slot-3][channel] = asat[rocid-25][slot-3][preamp][n];   //;   //
            parentint[rocid-25][slot-3][channel] = isat[rocid-25][slot-3][preamp][n];   //;   //	    
            esat[rocid-25][slot-3][preamp][n]++;   //
	  }
          //if (found) printf("likely echo rocid %i slot %i preamp %i chan %i rough_t %i dt %i net amp %i\n",rocid,slot,preamp,channel,rought,dt, (cp->first_max_amp<<ABIT) - (cp->pedestal<<PBIT) );

        }

      }   // if found is set, save pulse info to the tree


    }


    // look to see if the problem pulses are present in cdchits too.  if we are not pruning, they will be.

    uint16_t cdchitpresent[4][15][72] = {0};

    //cout << nh << " hits " << endl;
    
    for (uint32_t i=0; i<nh; i++) {
      
      const DCDCHit *hit = cdchits[i];
      //      const Df125CDCPulse *cp = NULL;
      //hit->GetSingle(cp);
      vector <const Df125CDCPulse*> pulse;
      hit->Get(pulse);
      if (pulse.size()==0) cout << "no pulse\n";

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
    if (tracks.size() ==0) return NOERROR;
   
    for (uint32_t i=0; i<(uint32_t)tracks.size(); i++) {  // not finished!
      
      const DTrackTimeBased *track = tracks[i];

      vector<DTrackFitter::pull_t> pulls = track->pulls;

      for (uint32_t j=0; j<pulls.size(); j++) {

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
	
      }
	
    }    



    
    // also want to record non-echo hits
    // could make histos as func of straw number for initial hits, final hits, hits on tracks


    

    // write arrays into the output tree

    
    for (uint32_t i=0; i<nc; i++) {
      
      const Df125CDCPulse *cp = cdcpulses[i];
      rocid = cp->rocid;
      slot = cp->slot;
      channel = cp->channel;
      preamp = uint16_t(channel/24);
      rought = uint32_t(cp->le_time/10);

      nhit = parent[rocid-25][slot-3][channel] - 1;

      //      printf("rocid %i slot %i preamp %i chan %i rough_t %i dt %i amp %i parent %i echoes %i\n",rocid,slot,preamp,channel,rought,dt, cp->first_max_amp, parent[rocid-25][slot-3][channel], esat[rocid-25][slot-3][preamp][nhit]);

      if (!parent[rocid-25][slot-3][channel]) continue;



      dt = rought - tsat[rocid-25][slot-3][preamp][nhit];

      uint16_t nechoes = 0;
      nechoes = esat[rocid-25][slot-3][preamp][nhit];
      if (nechoes==0) continue; // saturated but no echoes 

      int temp = channel - csat[rocid-25][slot-3][preamp][nhit]; 
      if (temp<0) temp = -temp;
      dc = uint16_t(temp);

      uint16_t hitpresent = cdchitpresent[rocid-25][slot-3][channel];
      uint16_t hitontrack = ontrack[rocid-25][slot-3][channel];      

      uint32_t ptamp = parentamp[rocid-25][slot-3][channel];
      uint32_t ptint = parentint[rocid-25][slot-3][channel];      

      uint16_t numsat = nsat[rocid-25][slot-3][preamp];
      
      japp->RootWriteLock();    
  
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
    
      time = cp->le_time;
      pedestal = cp->pedestal;
      integral = cp->integral;
      q = cp->time_quality_bit;
      overflows = cp->overflow_count;
      amp = cp->first_max_amp;

      iovera = (uint32_t)(integral/amp);
 
      const Df125WindowRawData *wrd;
      cp->GetSingle(wrd);

      if (wrd) {
        ns = (uint32_t)wrd->samples.size();

        uint32_t nsave = (ns<=NSAMPLES) ? ns : NSAMPLES ;  // save the first NSAMPLES values of the array
      
        for (uint j=0; j<nsave; j++) adc[j] = wrd->samples[j];
        for (uint j=nsave; j<NSAMPLES; j++) adc[j]=0;   
      }	  

      T->Fill();

    
      japp->RootUnLock();    
    }
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

