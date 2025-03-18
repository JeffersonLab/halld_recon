// $Id$
//
//    File: JEventProcessor_cdc_scan.cc
// Created: Sat Dec  6 21:34:19 EST 2014
// Creator: njarvis (on Linux maria 2.6.32-431.20.3.el6.x86_64 x86_64)


// Use -PEVIO:F125_EMULATION_MODE=2 -PCDC_SCAN:EMU=1 (default 1) to emulate firmware using window raw data
// Use -PCDC_SCAN:FDC=0 (default 1) to skip the FDC data

// To emulate the firmware using window raw data, use -PEVIO:F125_EMULATION_MODE=2
// Mode 0 switches the emulation off.  This is the default.
// Mode 1 replaces firmware values with emulated values always
// Mode 2 replaces firmware values with emulated values only if the firmware values are absent

// This plugin adds the emulated values to its output tree without replacing the firmware values.
// These are useful if run with mode=2 as above.
// Use the emulation switches to apply different configuration settings, eg -PEMULATION125:CDC_PBIT=1
// This would replace the values supplied in Df125BORConfig
// eg -PEVIO:F125_EMULATION_MODE=2 -PCDC_SCAN:EMU=1 -PEMULATION125:CDC_H=110 -PEMULATION125:CDC_TH=60 -PEMULATION125:CDC_TL=10

// Naomi Jarvis 12 Nov 2022

#include <stdint.h>
#include <vector>


#include "JEventProcessor_cdc_scan.h"
//#include <JANA/JApplication.h>


using namespace std;


#include "CDC/DCDCDigiHit.h"
#include "DAQ/Df125EmulatorAlgorithm_v2.h"
#include "DAQ/Df125WindowRawData.h"     
#include "DAQ/Df125CDCPulse.h"
#include "DAQ/Df125FDCPulse.h"
#include "DAQ/Df125TriggerTime.h"
#include "DAQ/Df125BORConfig.h"
#include "DAQ/bor_roc.h"
#include "TRIGGER/DTrigger.h"

#include <TTree.h>
#include <TBranch.h>


static TTree *t = NULL;
static TTree *p = NULL;
static TTree *tt = NULL;


extern "C"{
void InitPlugin(JApplication *app){
	InitJANAPlugin(app);
	app->Add(new JEventProcessor_cdc_scan());
}
} // "C"


//------------------
// JEventProcessor_cdc_scan (Constructor)
//------------------
JEventProcessor_cdc_scan::JEventProcessor_cdc_scan()
{
    SetTypeName("JEventProcessor_cdc_scan");
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
void JEventProcessor_cdc_scan::Init()
{
	// This is called once at program startup. If you are creating
	// and filling histograms in this plugin, you should lock the
	// ROOT mutex like this:
	//

  auto app = GetApplication();
  lockService = app->GetService<JLockService>();

  const uint32_t NSAMPLES = 200;

  EMU = 1;    // set to 0 to skip emulation from window raw data

  if (app) {
    app->SetDefaultParameter("CDC_SCAN:EMU",EMU,"Set to 0 to skip emulation from window raw data");
  }

  FDC = 1;    // set to 0 to skip FDC data

  if (app) {
    app->SetDefaultParameter("CDC_SCAN:FDC",FDC,"Set to 0 to skip FDC data");
  }






  if (!EMU)  cout << "\n cdc_scan: skipping emulation\n\n";
  if (!FDC)  cout << "\n cdc_scan: skipping FDC data\n\n";  


  lockService->RootWriteLock();
  
  t = new TTree("T","Event stats");

  ULong64_t t_eventnum;
  t->Branch("eventnum",&t_eventnum,"eventnum/l");

  uint32_t ncp, nfp, nt;
  t->Branch("CDCPulsecount",&ncp,"ncp/i");
  t->Branch("FDCPulsecount",&nfp,"nfp/i");
  t->Branch("TrigTimecount",&nt,"nt/i");


  p = new TTree("P","Pulse data");

  ULong64_t p_eventnum;  
  p->Branch("eventnum",&p_eventnum,"eventnum/l");

  uint32_t p_rocid, p_slot, p_channel, p_itrigger, word1, word2;
  p->Branch("rocid",&p_rocid,"rocid/i");
  p->Branch("slot",&p_slot,"slot/i");
  p->Branch("channel",&p_channel,"channel/i");
  p->Branch("itrigger",&p_itrigger,"itrigger/i");
  p->Branch("word1",&word1,"word1/i");
  p->Branch("word2",&word2,"word2/i");

  uint32_t npk, time, q, pedestal, amp, integral, overflows, pktime;
  p->Branch("npk",&npk,"npk/i");    
  p->Branch("time",&time,"time/i");    
  p->Branch("q",&q,"q/i");    
  p->Branch("pedestal",&pedestal,"pedestal/i");    
  p->Branch("amp",&amp,"amp/i");  
  p->Branch("integral",&integral,"integral/i");    
  p->Branch("overflows",&overflows,"overflows/i");    
  p->Branch("pktime",&pktime,"pktime/i");
  
  bool emulated, cdc;
  p->Branch("emulated",&emulated,"emulated/O");     // set to 1 if the pulse data object contents are emulated
  p->Branch("cdcdata",&cdc,"cdc/O");

  uint32_t ns;
  p->Branch("nsamples",&ns,"nsamples/i");
  
  uint16_t adc[NSAMPLES];
  p->Branch("adc",&adc,Form("adc[%i]/s",NSAMPLES));         

  if (EMU) {

    uint32_t m_time, m_q, m_pedestal, m_integral, m_amp, m_overflows, m_pktime;
    p->Branch("m_time", &m_time, "m_time/i");
    p->Branch("m_q", &m_q, "m_q/i");
    p->Branch("m_pedestal", &m_pedestal, "m_pedestal/i");
    p->Branch("m_integral", &m_integral, "m_integral/i");
    p->Branch("m_amp", &m_amp, "m_amp/i");
    p->Branch("m_overflows", &m_overflows, "m_overflows/i");
    p->Branch("m_pktime", &m_pktime, "m_pktime/i");
  
    int d_time, d_q, d_pedestal, d_integral, d_amp, d_overflows, d_pktime;
    p->Branch("d_time", &d_time, "d_time/I");
    p->Branch("d_q", &d_q, "d_q/I");
    p->Branch("d_pedestal", &d_pedestal, "d_pedestal/I");
    p->Branch("d_integral", &d_integral, "d_integral/I");
    p->Branch("d_amp", &d_amp, "d_amp/I");
    p->Branch("d_overflows", &d_overflows, "d_overflows/I");
    p->Branch("d_pktime", &d_pktime, "d_pktime/I");
  
    bool diffs;
    p->Branch("diffs",&diffs,"diffs/O");
  }
  
  //BORConfig values
  uint32_t board_id,version,proc_version,proc_blocklevel,temp;
  uint32_t cPL,cNW,cIE,cPG,cP1,cP2,cIBIT,cABIT,cPBIT,cH,cTH,cTL;
  p->Branch("board_id",&board_id,"board_id/i");
  p->Branch("version",&version,"version/i");
  p->Branch("proc_version",&proc_version,"proc_version/i");
  p->Branch("temp",&temp,"temp/i");
  p->Branch("proc_blocklevel",&proc_blocklevel,"proc_blocklevel/i");
  p->Branch("PL",&cPL,"PL/i");
  p->Branch("NW",&cNW,"NW/i");
  p->Branch("IE",&cIE,"IE/i");
  p->Branch("PG",&cPG,"PG/i");
  p->Branch("P1",&cP1,"P1/i");
  p->Branch("P2",&cP2,"P2/i");
  p->Branch("IBIT",&cIBIT,"IBIT/i");
  p->Branch("ABIT",&cABIT,"ABIT/i");
  p->Branch("PBIT",&cPBIT,"PBIT/i");
  p->Branch("H",&cH,"H/i");
  p->Branch("TH",&cTH,"TH/i");
  p->Branch("TL",&cTL,"TL/i");


  tt = new TTree("TT","Trigger time");

  ULong64_t tt_eventnum;
  tt->Branch("eventnum",&tt_eventnum,"eventnum/l");

  uint32_t tt_rocid, tt_slot, tt_itrigger;
  tt->Branch("rocid",&tt_rocid,"rocid/i");
  tt->Branch("slot",&tt_slot,"slot/i");
  tt->Branch("itrigger",&tt_itrigger,"itrigger/i");

  uint64_t tt_time;
  tt->Branch("time",&tt_time,"time/l");

  lockService->RootUnLock();

}

//------------------
// BeginRun
//------------------
void JEventProcessor_cdc_scan::BeginRun(const std::shared_ptr<const JEvent> &event)
{
	// This is called whenever the run number changes
}

//------------------
// Process
//------------------
void JEventProcessor_cdc_scan::Process(const std::shared_ptr<const JEvent> &event)
{


  /* 0x1058 FE nw register defintions */
  #define FA125_FE_NW_MASK          0x000003FF
  
  /* 0x105C FE pl register defintions */
  #define FA125_FE_PL_MASK           0x0000FFFF
  
  /* 0xN0A0 FE ped_sf definitions */
  #define FA125_FE_PED_SF_NP_MASK       0x000000FF
  #define FA125_FE_PED_SF_NP2_MASK      0x0000FF00
  #define FA125_FE_PED_SF_IBIT_MASK     0x00070000
  #define FA125_FE_PED_SF_ABIT_MASK     0x00380000
  #define FA125_FE_PED_SF_PBIT_MASK     0x01C00000
  #define FA125_FE_PED_SF_PBIT_SIGN     (1<<25)
  #define FA125_FE_PED_SF_CALC_MASK     0x1C000000
  
  
  /* 0xN0B0 FE integration_end definitions */
  #define FA125_FE_IE_INTEGRATION_END_MASK  0x00000FFF
  #define FA125_FE_IE_PEDESTAL_GAP_MASK     0x000FF000
  
  /* 0xN070 - 0xN084 threshold register defintions */
  #define FA125_FE_THRESHOLD_MASK          0x00000FFF
  
  /* 0xN0A4 FE timing_thres_lo definitions */
  #define FA125_FE_TIMING_THRES_LO_MASK(x) (0xFF<<(8+((x%2)*16)))
  
  /* 0xN0B4 FE timing_thres_hi definitions */
  #define FA125_FE_TIMING_THRES_HI_MASK(x) (0x1FF<<((x%3)*9))
  
  /*
  // Only look at physics triggers
  /*
  const DTrigger* locTrigger = NULL; 
  event->GetSingle(locTrigger);
  if(locTrigger->Get_L1FrontPanelTriggerBits() != 0)
    return;
  if (!locTrigger->Get_IsPhysicsEvent()){ // do not look at PS triggers
    return;
  }
<<<<<<< HEAD
  */   
  vector <const Df125CDCPulse*> cdcpulses;
  loop->Get(cdcpulses);
=======
  */
  
  auto cdcpulses = event->Get<Df125CDCPulse>();  
>>>>>>> 2cd7c9f89ed8f70ac624f5fd1c88173fd872127b
  uint32_t nc = (uint32_t)cdcpulses.size();

  auto fdcpulses = event->Get<Df125FDCPulse>();
  uint32_t nf = (uint32_t)fdcpulses.size();

  auto ttvector = event->Get<Df125TriggerTime>();
  uint32_t ntt = (uint32_t)ttvector.size();

  
  if (nc+nf==0) return;  // no DC hits


  ULong64_t eventnum = (ULong64_t)event->GetEventNumber();

  
  lockService->RootWriteLock(); //ACQUIRE ROOT LOCK!!

  t->SetBranchAddress("eventnum",&eventnum);
  t->SetBranchAddress("CDCPulsecount",&nc);
  t->SetBranchAddress("FDCPulsecount",&nf);
  t->SetBranchAddress("TrigTimecount",&ntt);
    
  t->Fill();  

  lockService->RootUnLock();
  
 
  
 
  
  if (ntt > 0) { //   Df125TriggerTime 

    lockService->RootWriteLock(); //ACQUIRE ROOT LOCK!!    

    tt->SetBranchAddress("eventnum",&eventnum);

    uint32_t tt_rocid, tt_slot, tt_itrigger;
    tt->SetBranchAddress("rocid",&tt_rocid);
    tt->SetBranchAddress("slot",&tt_slot);
    tt->SetBranchAddress("itrigger",&tt_itrigger);

    ULong64_t tt_time;
    tt->SetBranchAddress("time",&tt_time);

    for (uint32_t i=0; i<ntt; i++) {

      const Df125TriggerTime *thistt = ttvector[i];

      if (!FDC && (thistt->rocid < 25 || thistt->rocid > 28)) continue;   // skip FDC

      //      cout << thistt->rocid << " " << thistt->slot << " " << thistt->itrigger << " " << thistt->time << endl;

      tt_rocid = thistt->rocid;
      tt_slot = thistt->slot;
      tt_itrigger = (uint32_t)thistt->itrigger;
      tt_time = thistt->time;

      tt->Fill();
    }

    lockService->RootUnLock();

  }

  
  
  const uint32_t NSAMPLES = 200;   // 100 for FDC

  if (nc || (nf&&FDC)) {  // branches are almost the same for CDC & FDC - only amp differs

    lockService->RootWriteLock(); //ACQUIRE ROOT LOCK!!
    
    p->SetBranchAddress("eventnum",&eventnum);


    uint32_t p_rocid, p_slot, p_channel, p_itrigger, word1, word2; 
    p->SetBranchAddress("rocid",&p_rocid);
    p->SetBranchAddress("slot",&p_slot);
    p->SetBranchAddress("channel",&p_channel);
    p->SetBranchAddress("itrigger",&p_itrigger);
    p->SetBranchAddress("word1",&word1);
    p->SetBranchAddress("word2",&word2);

    uint32_t npk, time, q, pedestal, amp, integral, overflows, pktime;
    p->SetBranchAddress("npk",&npk);
    p->SetBranchAddress("time",&time);    
    p->SetBranchAddress("q",&q);
    p->SetBranchAddress("pedestal",&pedestal);
    p->SetBranchAddress("amp",&amp);
    p->SetBranchAddress("integral",&integral);
    p->SetBranchAddress("overflows",&overflows);
    p->SetBranchAddress("pktime",&pktime);    

    bool emulated, cdc; 
    p->SetBranchAddress("emulated",&emulated);
    p->SetBranchAddress("cdcdata",&cdc);

    uint32_t ns;
    p->SetBranchAddress("nsamples",&ns);

    uint16_t adc[NSAMPLES] = {0};       //    vector<uint16_t> samples;
    p->SetBranchAddress("adc",&adc);

    
    uint32_t m_npk, m_time=0, m_q=0, m_pedestal=0, m_integral=0, m_amp=0, m_overflows=0, m_pktime=0;
    int d_npk = 0, d_time=0, d_q=0, d_pedestal=0, d_integral=0, d_amp=0, d_overflows=0, d_pktime=0;
    bool diffs=0;
    
    if (EMU) {    
      p->SetBranchAddress("m_npk",&m_npk);
      p->SetBranchAddress("m_time",&m_time);
      p->SetBranchAddress("m_q",&m_q);
      p->SetBranchAddress("m_overflows",&m_overflows);
      p->SetBranchAddress("m_pedestal",&m_pedestal);
      p->SetBranchAddress("m_integral",&m_integral);
      p->SetBranchAddress("m_amp",&m_amp);
      p->SetBranchAddress("m_pktime",&m_pktime);

      p->SetBranchAddress("d_npk",&d_npk);
      p->SetBranchAddress("d_time", &d_time);
      p->SetBranchAddress("d_q", &d_q);
      p->SetBranchAddress("d_pedestal", &d_pedestal);
      p->SetBranchAddress("d_integral", &d_integral);
      p->SetBranchAddress("d_amp", &d_amp);
      p->SetBranchAddress("d_overflows", &d_overflows);
      p->SetBranchAddress("d_pktime", &d_pktime);
    
      p->SetBranchAddress("diffs",&diffs);  
    }

    //BORConfig values
    uint32_t board_id=0, version=0, proc_version=0, proc_blocklevel=0, temp=0;
    uint32_t cPL=0, cNW=0, cIE=0, cPG=0, cP1=0, cP2=0, cIBIT=0, cABIT=0, cPBIT=0, cH=0, cTH=0, cTL=0;
    p->SetBranchAddress("board_id", &board_id);
    p->SetBranchAddress("version",&version);
    p->SetBranchAddress("proc_version",&proc_version);
    p->SetBranchAddress("temp",&temp);
    p->SetBranchAddress("proc_blocklevel",&proc_blocklevel);
    p->SetBranchAddress("PL",&cPL);
    p->SetBranchAddress("NW",&cNW);
    p->SetBranchAddress("IE",&cIE);
    p->SetBranchAddress("PG",&cPG);
    p->SetBranchAddress("P1",&cP1);
    p->SetBranchAddress("P2",&cP2);
    p->SetBranchAddress("IBIT",&cIBIT);
    p->SetBranchAddress("ABIT",&cABIT);
    p->SetBranchAddress("PBIT",&cPBIT);
    p->SetBranchAddress("H",&cH);
    p->SetBranchAddress("TH",&cTH);
    p->SetBranchAddress("TL",&cTL);


    Df125EmulatorAlgorithm_v2 *em = new Df125EmulatorAlgorithm_v2();    
    
    if (nc) {
       
      cdc = 1;
      ns = 0;   // could be different for cdc & fdc
      
      for (uint32_t i=0; i<nc; i++) {

        const Df125CDCPulse *cp = cdcpulses[i];
  
        p_rocid = cp->rocid;
        p_slot = cp->slot;
        p_channel = cp->channel;
        p_itrigger = cp->itrigger;
  
        word1 = cp->word1;
        word2 = cp->word2;
	npk = cp->NPK;
        time = cp->le_time;
        pedestal = cp->pedestal;
        integral = cp->integral;
        q = cp->time_quality_bit;
        overflows = cp->overflow_count;
        amp = cp->first_max_amp;
        pktime = 0;
	
        emulated = cp->emulated;

	auto wrd = cp->GetSingle<Df125WindowRawData>();
	
        if (wrd) {
          ns = (uint32_t)wrd->samples.size();

          uint32_t nsave = (ns<=NSAMPLES) ? ns : NSAMPLES ;  // save the first NSAMPLES values of the array
      
          for (uint j=0; j<nsave; j++) adc[j] = wrd->samples[j];
          for (uint j=nsave; j<NSAMPLES; j++) adc[j]=0;   
        }	  
	
        if (EMU && wrd) { 
	
 	  Df125CDCPulse *emu = new Df125CDCPulse();

          em->EmulateFirmware(wrd, emu, NULL);

	  m_npk = 0;
          m_time = emu->le_time_emulated;
          m_q = emu->time_quality_bit_emulated;
          m_overflows = emu->overflow_count_emulated;
          m_pedestal = emu->pedestal_emulated;
          m_integral = emu->integral_emulated;
          m_amp = emu->first_max_amp_emulated;
          m_pktime=0;

          // Firmware q is either 1 or 0.  m_q is the q_code, 0 to 9, where 0 indicates a good time measurement
	  // Only flag a difference between these if one is 0 and the other isn't.
	  uint m_q_binary = (m_q == 0) ? 0 : 1 ;
          d_q = q - m_q_binary;

	  d_npk = 0;//npk - m_npk;
          d_time = time - m_time;
          d_overflows = overflows - m_overflows;
          d_pedestal = pedestal - m_pedestal;
          d_integral = integral - m_integral;
          d_amp = amp - m_amp;
          d_pktime=0;

	  diffs=0;
          if (d_npk || d_time || d_q || d_overflows || d_pedestal || d_integral || d_amp) diffs = 1;
	  
	}
	
        board_id=0;
        version=0;
        proc_version=0;
        proc_blocklevel=0;
        temp=0;
        cPL=0;
        cNW=0;
        cIE=0;
        cPG=0;
        cP1=0;
        cP2=0;
        cIBIT=0;
        cABIT=0;        
        cPBIT=0;
        cH=0;
        cTH=0;
        cTL=0;

	auto BORConfig = cp->GetSingle<Df125BORConfig>();
	
        if (BORConfig) {

          board_id = BORConfig->board_id;
          version = BORConfig->version;
          proc_version = BORConfig->proc_version;
          proc_blocklevel = BORConfig->proc_blocklevel;
          temp = BORConfig->temperature[0];

          //BORConfig->fe[12] holds the config params for the fadc module
          // in 12 sets of 6 channels

          uint32_t j = p_channel/6;
          f125config_fe fe  = BORConfig->fe[j];

          cPL = fe.pl & FA125_FE_PL_MASK;                       // 
          cNW = fe.nw & FA125_FE_NW_MASK;                       // 0x000003FF;
          cIE = fe.ie & FA125_FE_IE_INTEGRATION_END_MASK;       // 0x00000FFF
          cPG = (fe.ie & FA125_FE_IE_PEDESTAL_GAP_MASK) >> 12;  // 0x000FF000
          cP1 = fe.ped_sf & FA125_FE_PED_SF_NP_MASK;            // 0x000000FF
          cP2 = (fe.ped_sf & FA125_FE_PED_SF_NP2_MASK) >> 8;    // 0x0000FF00
          cIBIT = (fe.ped_sf & FA125_FE_PED_SF_IBIT_MASK) >> 16;  // 0x00070000
          cABIT = (fe.ped_sf & FA125_FE_PED_SF_ABIT_MASK) >> 19;  // 0x00380000
          cPBIT = (fe.ped_sf & FA125_FE_PED_SF_PBIT_MASK) >> 22;  // 0x01C00000
      
          int psign = (fe.ped_sf & FA125_FE_PED_SF_PBIT_SIGN) >> 25;  // (1<<25)
          if (psign) cPBIT = -1*cPBIT;

          uint32_t k = p_channel - 6*j;

          cH = fe.threshold[k]&0xFFF;    //  0x00000FFF

          if (k%2==0) cTL = (fe.timing_thres_lo[(k/2)%3] & FA125_FE_TIMING_THRES_LO_MASK(k)) >> 8;   //(0xFF<<(8+((x%2)*16)))
          if (k%2==1) cTL = (fe.timing_thres_lo[(k/2)%3] & FA125_FE_TIMING_THRES_LO_MASK(k)) >> 24;

          if (k%3==0) cTH = (fe.timing_thres_hi[(k/3)%2] & FA125_FE_TIMING_THRES_HI_MASK(k)) >> 0;  //(0x1FF<<((x%3)*9))
          if (k%3==1) cTH = (fe.timing_thres_hi[(k/3)%2] & FA125_FE_TIMING_THRES_HI_MASK(k)) >> 9;
          if (k%3==2) cTH = (fe.timing_thres_hi[(k/3)%2] & FA125_FE_TIMING_THRES_HI_MASK(k)) >> 18;

        }

        p->Fill();
  
      }

    }

  
    if (FDC && nf) {

      cdc = 0;      
      ns = 0;
      
      for (uint32_t i=0; i<nf; i++) {

        const Df125FDCPulse *fp = fdcpulses[i];
  
        p_rocid = fp->rocid;
        p_slot = fp->slot;
        p_channel = fp->channel;
        p_itrigger = fp->itrigger;
  
        word1 = fp->word1;
        word2 = fp->word2;
	npk = fp->NPK;
        time = fp->le_time;
        pedestal = fp->pedestal;
        integral = fp->integral;
        q = fp->time_quality_bit;
        overflows = fp->overflow_count;
        amp = fp->peak_amp;
        pktime = fp->peak_time;
	
        emulated = fp->emulated;

	auto wrd = fp->GetSingle<Df125WindowRawData>();	

        if (wrd) {
          ns = (uint32_t)wrd->samples.size();

          uint32_t nsave = (ns<=NSAMPLES) ? ns : NSAMPLES ;  // save the first NSAMPLES values of the array
      
          for (uint j=0; j<nsave; j++) adc[j] = wrd->samples[j];
          for (uint j=nsave; j<NSAMPLES; j++) adc[j]=0;   
        }	  
	
        if (EMU && wrd) {   // need to make a flag whether to do the emu or not
	
 	  Df125FDCPulse *emu = new Df125FDCPulse();

          em->EmulateFirmware(wrd, NULL, emu);

	  m_npk = 0; //fp->npk;
          m_time = fp->le_time_emulated;
          m_q = fp->time_quality_bit_emulated;
          m_overflows = fp->overflow_count_emulated;
          m_pedestal = fp->pedestal_emulated;
          m_integral = fp->integral_emulated;
          m_amp = fp->peak_amp_emulated;
          m_pktime = fp->peak_time_emulated;	

          // Firmware q is either 1 or 0.  m_q is the q_code, 0 to 9, where 0 indicates a good time measurement
	  // Only flag a difference between these if one is 0 and the other isn't.
	  uint m_q_binary = (m_q == 0) ? 0 : 1 ;
          d_q = q - m_q_binary;

	  d_npk = 0; //npk - d_npk;
          d_time = time - m_time;
          d_overflows = overflows - m_overflows;
          d_pedestal = pedestal - m_pedestal;
          d_integral = integral - m_integral;
          d_amp = amp - m_amp;
          d_pktime = pktime - m_pktime;

	  diffs=0;
          // FDC mode 8 does not report the integral, so don't flag those differences in the diffs branch
          if (d_time || d_q || d_overflows || d_pedestal || d_amp || d_pktime) diffs = 1;
	}


        board_id=0;
        version=0;
        proc_version=0;
        proc_blocklevel=0;
        temp=0;
        cPL=0;
        cNW=0;
        cIE=0;
        cPG=0;
        cP1=0;
        cP2=0;
        cIBIT=0;
        cABIT=0;        
        cPBIT=0;
        cH=0;
        cTH=0;
        cTL=0;

	auto BORConfig = fp->GetSingle<Df125BORConfig>();
	
        if (BORConfig) {

          board_id = BORConfig->board_id;
          version = BORConfig->version;
          proc_version = BORConfig->proc_version;
          proc_blocklevel = BORConfig->proc_blocklevel;
          temp = BORConfig->temperature[0];

          //BORConfig->fe[12] holds the config params for the fadc module
          // in 12 sets of 6 channels

          uint32_t j = p_channel/6;
          f125config_fe fe  = BORConfig->fe[j];

          cPL = fe.pl & FA125_FE_PL_MASK;                       // 
          cNW = fe.nw & FA125_FE_NW_MASK;                       // 0x000003FF;
          cIE = fe.ie & FA125_FE_IE_INTEGRATION_END_MASK;       // 0x00000FFF
          cPG = (fe.ie & FA125_FE_IE_PEDESTAL_GAP_MASK) >> 12;  // 0x000FF000
          cP1 = fe.ped_sf & FA125_FE_PED_SF_NP_MASK;            // 0x000000FF
          cP2 = (fe.ped_sf & FA125_FE_PED_SF_NP2_MASK) >> 8;    // 0x0000FF00
          cIBIT = (fe.ped_sf & FA125_FE_PED_SF_IBIT_MASK) >> 16;  // 0x00070000
          cABIT = (fe.ped_sf & FA125_FE_PED_SF_ABIT_MASK) >> 19;  // 0x00380000
          cPBIT = (fe.ped_sf & FA125_FE_PED_SF_PBIT_MASK) >> 22;  // 0x01C00000
      
          int psign = (fe.ped_sf & FA125_FE_PED_SF_PBIT_SIGN) >> 25;  // (1<<25)
          if (psign) cPBIT = -1*cPBIT;

          uint32_t k = p_channel - 6*j;

          cH = fe.threshold[k]&0xFFF;    //  0x00000FFF

          if (k%2==0) cTL = (fe.timing_thres_lo[(k/2)%3] & FA125_FE_TIMING_THRES_LO_MASK(k)) >> 8;   //(0xFF<<(8+((x%2)*16)))
          if (k%2==1) cTL = (fe.timing_thres_lo[(k/2)%3] & FA125_FE_TIMING_THRES_LO_MASK(k)) >> 24;

          if (k%3==0) cTH = (fe.timing_thres_hi[(k/3)%2] & FA125_FE_TIMING_THRES_HI_MASK(k)) >> 0;  //(0x1FF<<((x%3)*9))
          if (k%3==1) cTH = (fe.timing_thres_hi[(k/3)%2] & FA125_FE_TIMING_THRES_HI_MASK(k)) >> 9;
          if (k%3==2) cTH = (fe.timing_thres_hi[(k/3)%2] & FA125_FE_TIMING_THRES_HI_MASK(k)) >> 18;

        }
	
        p->Fill();
  
      }
  
    }

    lockService->RootUnLock();    
  }
    
  return;

}

//------------------
// EndRun
//------------------
void JEventProcessor_cdc_scan::EndRun()
{
	// This is called whenever the run number changes);
	// changed to give you a chance to clean up before processing
	// events from the next run number.
}

//------------------
// fini
//------------------
void JEventProcessor_cdc_scan::Finish()
{
	// Called before program exit after event processing is finished.
}

