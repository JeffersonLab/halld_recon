// $Id$
//
//    File: JEventProcessor_HELI_online.cc
// Created: Thu Jun 22 16:59:57 EDT 2023
// Creator: jrsteven (on Linux ifarm1801.jlab.org 3.10.0-1160.90.1.el7.x86_64 x86_64)
//
// See JEventProcessor_HELI_online.h for some mode detailed documentation.

#include "JEventProcessor_HELI_online.h"
using namespace std;


// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
//#include <JANA/JFactory.h>
extern "C"{
  void InitPlugin(JApplication *app){
    InitJANAPlugin(app);
    app->Add(new JEventProcessor_HELI_online());
  }
} // "C"

int     HELIVERBOSE = 0;
string  HELISETUP   = "helicity_setup.txt";
string  HELILOG     = "helicity.log";


//------------------
// JEventProcessor_HELI_online (Constructor)
//------------------
JEventProcessor_HELI_online::JEventProcessor_HELI_online(){
  SetTypeName("JEventProcessor_HELI_online");
}

//------------------
// ~JEventProcessor_HELI_online (Destructor)
//------------------
JEventProcessor_HELI_online::~JEventProcessor_HELI_online(){
}



//------------------
// Init
//------------------
void JEventProcessor_HELI_online::Init(void){
  // This is called once at program startup 
  
  
  fEventLatest    =  0;                                    //init all the stuff
  fEventRate      =  160000;
  fEventRateCount =  3000;
  fEventRateThresh=  5000; 
  fIsLatest       =  0;                                    //tag if its the latest event no so far.
  fNBitsReqd      = 30;
  fNBitsRead      =  0;
  fBits           =  0;                                    //bit patt to match delayed hel (ie hel in data stream)
  fBitsNow        =  0;                                    //bit patt to predict current helicity.

  fPatSyncLatest  =  0;                                    //prev, latest and current versions of things to deal with timesequence in multi-threaded analysis.
  fPairSyncLatest =  0;
  fHelPredPrev    =  0;      
  fHelPredLatest  =  0;
  fHelPred        =  0;
  fHelDelPrev     =  0;      
  fHelDelLatest   =  0;
  fHelDel         =  0;
  fHelNowPrev     =  0;    
  fHelNowLatest   =  0;  
  fHelNow         =  0;
	
  fPatNo          = QUARTET;                               //The defaults according to the manual, they'll get overwritten anyway  when per run params are read.                             
  fNWinDel        = 8;                                     //The no of flips in the delay.
  fUseTSettle     = 1;                                     //Set to 0 to ignore TSettle in case it's noisy or corrupted.

  fPatN           = 0;                                     //Position in the current pattern
  fPatFlag        = 0;                                     //Flag new pattern and count events in there
  fPairFlag       = 0;

  fHWP            = 0;                                     //Multiplier depending on fHWPStateValue: 0, 1,-1 (in, out, unknown)
                                                           //translates to                           1,-1, 0 (= don't flip hel, flip hel, set hel to 0) 
  fDiffErrorCount = 0;
  fDiffErrorMax   = 5;
  fDiffEventCount = 0;
  fDiffEventMax   = 30;
  
  fDiffVeto       = 0;                                     //veto the helicity if the predicted is different from the read hel (0 = veto, 1 keep)

  fHelicity       = 0;                                     //Returned by getHelicity() 

  dFile           = NULL;

  auto app = GetApplication();
  if(app){                                              //override with command line if needed
    app->SetDefaultParameter("HELI:VERBOSE", HELIVERBOSE, "Set level of verbosity on helicity");
    app->SetDefaultParameter("HELI:SETUP",   HELISETUP,   "Set name of helicity setup file.");
    app->SetDefaultParameter("HELI:LOG",     HELILOG,     "Set name of helicity log file.");
  }

  readParms(1);                                            //read parameters for run ranges tables  0: from caldb, 1: from ./helicity_setup.txt

  fWinDelValue      =  4;                                  //then set to some defaults
  fPolPatValue      =  1;
  fHWPEPICSUseValue =  0;
  fHWPStateValue    = -1;
  fUseTSetValue     =  1;
  strcpy(fHWPEPICSChanValue,"IGL1I00OD16_16");


  if(HELIVERBOSE > 0){                                      //if debugging       
    //    dFile = fopen("heli.log","w");                    //open a separate file for logging (maybe from infile name later)
    dFile = fopen(HELILOG.c_str(),"a");                     //open a separate file for logging (maybe from infile name later)
    for(int n=0;n<10;n++){                                  //init the array that holds event bit and helicity data.
      dHelBits[n]=0;
    }
  }
  return; //NOERROR;
}

//------------------
// BeginRun
//------------------
void JEventProcessor_HELI_online::BeginRun(const std::shared_ptr<const JEvent>& event){
  // This is called whenever the run number changes
  auto runnumber = event->GetRunNumber();
  // Init all the counters
  fEventInRun     = 0;  
  fPlusInRun      = 0;	  
  fMinusInRun     = 0;  
  fNullInRun      = 0;	  
  fHelAsym        = 0;	  
  fHelAsymError   = 0;
  fRunNumber      = runnumber;
  
  setRunParms(fRunNumber);                                 //get the parameters for the current run

  printf("\n\n*********************************************************************************\n");
  printf("HEL: Selected Helicity Params for Run %d                                     **\n\n",fRunNumber);
  printf("HEL: Name                   Value\n");
  printf("HEL: HWP_EPICS_PV       %s\n",    fHWPEPICSChanValue);
  printf("HEL: HWP_USE_EPICS      %d\n",    fHWPEPICSUseValue);
  printf("HEL: DELAYd             %d\n",    fWinDelValue);
  printf("HEL: PATTERNd           %d\n",    fPolPatValue);
  printf("HEL: HWP_STATE          %d\n",    fHWPStateValue);
  printf("HEL: USE_T_SETTLE       %d\n",    fUseTSetValue);
  printf("HEL: EVENT_RATE_THRESH %ld\n",    fEventRateThresh);
  printf("\n*********************************************************************************\n");

  
  //do anything needed to update variables based the parameters
  fNWinDel        = HelWinDelayMap[fWinDelValue];
  fPolPat         = fPolPatValue;
  fUseTSettle     = fUseTSetValue;

  fPatternName    = (char *)HelPatternName[fPatNo];
  fPatternLen     = HelPatternLen[fPatNo];
  fNPatDel        = fNWinDel/fPatternLen;                  //translated into the no of patterns

  if(fHWPStateValue == 0)       fHWP = -1;                 //Since helicity returned needs to be multiplied by this.
  else if (fHWPStateValue == 1) fHWP =  1;                 // 0 = in, so invert. 1 = out, so don't invert.
  else                          fHWP =  0;                 //Unknown

  fUseHWPEPICS    = fHWPEPICSUseValue;
  fUseTSettle     = fUseTSetValue;

  return; //NOERROR;
}


//------------------
// Process
//------------------
void JEventProcessor_HELI_online::Process(const std::shared_ptr<const JEvent>& event){

 

  auto eventnumber = event->GetEventNumber();
  vector<const DBeamHelicity*> locBH;
  event->Get(locBH);                                       //get the BH from the current evnt
  
  if(locBH.empty()){ 
    return; //NOERROR;
  }

  // Get DCODAROCInfo for this ROC
  vector<const DCODAROCInfo*> locCODAROCInfos;
  event->Get(locCODAROCInfos);
  uint64_t locReferenceClockTime = 0;
  for (const auto& locCODAROCInfo : locCODAROCInfos) {
    if(locCODAROCInfo->rocid == 71) {
      locReferenceClockTime = locCODAROCInfo->timestamp;
    }
  }    
  m_mtx.lock();                                           //lock this thread
  fReferenceClockTime = locReferenceClockTime;

  //do some event inits

  fEventno    = eventnumber;                              //save the event number
  fDiffVeto   = 1;                                        // sets hel to 0 in GetHel() functions when predicted != read helicity (assume OK)
  fIsLatest   = 0;

  f_pattern_sync = locBH[0]->pattern_sync;                //save local copies
  f_t_settle     = locBH[0]->t_settle;
  f_helicity     = locBH[0]->helicity;
  f_pair_sync    = locBH[0]->pair_sync;
  f_ihwp         = locBH[0]->ihwp;
  f_beam_on      = locBH[0]->beam_on;

  //t settle present, so skip the event
  if(fUseTSettle && f_t_settle){ 
    fHelicity = 0;  
    if(HELIVERBOSE > 0)printEvent();
    m_mtx.unlock();                                            // Unlock main mutex
    return; //NOERROR;
  }






  
  //------------------------------------------ start latest event handling -------------------------------------------------------------------
  //For multi threaded running need to handle furthers ahead events differently from  standard events                                        |

  if(fEventno > fEventLatest){                            //update this only when event is later than those already processed by other threads.	  
    fIsLatest=1;                                          //flag that this event is the latest so far
    if(f_pattern_sync > fPatSyncLatest){                  //if pattern sync 0 -> 1
      fPatFlag = 1;                                       //flag the pattern sync rising edge / event lag counter
      fPatN    = 0;                                       //go to start of pattern

      fEventRate        = fEventRateCount*30/fPatternLen; //store the current count as the event rate (conv to Hz)
      fEventRateCount   = 0;                              //and zero the counter.

      if(fEventRate < fEventRateThresh){                  //the rate has dropped too low to be sensible
	printf("HEL: Warning: Helicity processor - Low event rate of ~ %ld Hz. Resetting when rate get over ~ %ld Hz\n",fEventRate,fEventRateThresh);
	fNBitsRead     = 0;                               //this will trigger rereading of the word
	fHelPredLatest = 0;                               //set fHelPredLatest to unknown
	fHelNowLatest  = 0;                               //set fHelNowLatest to unknown
      }

      if(fNBitsRead >= fNBitsReqd){                       // if already have the reqd no of bits
	nextRand(&fBits);                                 // move to the next step in the pattern
	fPolPat = Helb2h[fBits&1];                        // fPolPat is the delayed last bit of the 30 bits (converted to -1,1)
	nextRand(&fBitsNow);                              // move to the next step in the pattern
	fPolPatNow = Helb2h[fBitsNow&1];                  // fPolPat is the current last bit of the 30 nowbits (converted to -1,1)
      }
      else{
	fPolPat    = 0;                                   //zero the pattern polarities until we have enough bits
	fPolPatNow = 0;                                   
      }
                                     
      if(fPlusInRun && fMinusInRun){                      // At every sync pulse update the beam charge asymmetry.
	fHelAsym = (double)(fPlusInRun-fMinusInRun)/(double)(fPlusInRun+fMinusInRun);
	fHelAsymError = 2.0*sqrt((fPlusInRun*pow(fMinusInRun,2)+fMinusInRun*pow(fPlusInRun,2))/pow(fPlusInRun+fMinusInRun,4));
      }
      else{
	fHelAsym      = 0;
	fHelAsymError = 0;
      }
    }
    fPatSyncLatest = f_pattern_sync;                      //save as the latest 

    if(fPatFlag){                                         //if just passed a pattern sync rising edge
      fPatFlag++;                                         //increment the event flag/counter
    }

    if(fNBitsRead < fNBitsReqd){                          //if we don't have enough bits yet ...
      fPolPat    = 0;                                     //zero the pattern polarities until we have enough bits
      fPolPatNow = 0;                                     
      if(fPatFlag>2){                                     //required no of events lag past rising pat_sync edge ...
                                                          //in case t-settle is missing, make sure we're in the window.
	fBits <<= 1;
	fBits  += f_helicity;                             //acquire another bit
	fNBitsRead++;                                     //increment bit counter
	fPatFlag=0;                                       //reset
	if(HELIVERBOSE > 1){
	  fprintf(dFile,"#NewBit %ld, Hel %d, Got %d bits %d\n",fEventno,f_helicity,fNBitsRead,fBits);
	}
	if(fNBitsRead == fNBitsReqd){
	  fBitsNow = fBits;                               //copy the delayed generator word to the current one
	  for(uint d=0; d<fNPatDel;d++){                  //and move on by the required number of patterns to make it the present generator.
	    nextRand(&fBitsNow);
	    if(HELIVERBOSE > 1)fprintf(dFile,"#Got Extra Bit %d for Now Helicity\n",d+1);
	  }
	}
      }
    }
    
    if(f_pair_sync != fPairSyncLatest){                         //pair sync flipped (normally at 30Hz)
      fHelPredPrev      = fHelPredLatest;
      fHelPredLatest    = fPolPat*HelPattern[fPatNo][fPatN];    //helicity del, based on prediction
      fHelNowPrev       = fHelNowLatest;
      fHelNowLatest     = fPolPatNow*HelPattern[fPatNo][fPatN]; //helicity now, on prediction
      fPatN++;
      fPairSyncLatest   = f_pair_sync;                            //update the latest predicted
    }
    fHelPred          = fHelPredLatest;
    fHelNow           = fHelNowLatest;
    fEventLatest      = fEventno;                                 //update latest event number
  }
  //                                                                                                                                        |
  // -------------------------------------- End of latest event handling -------------------------------------------------------------------

  // -------------------------------------- Start of standard event handling (ie not the latest event)        ------------------------------
  //                                                                                                                                        |
  else{                                                      
    if(fNBitsRead < fNBitsReqd){                             //don't have enough bits yet
      fHelPred = 0;                                          //set fHelPred to unknown
      fHelNow  = 0;                                          //set fHelNow to unknown
    }
    else{
      if(f_pair_sync == fPairSyncLatest){                    //if pair sync is same as latest use latest values
	fHelPred = fHelPredLatest;
	fHelNow  = fHelNowLatest;
      }
      else{                                                  //otherwise previous ones (in the previous window)
	fHelPred = fHelPredPrev;
	fHelNow  = fHelNowPrev;
      }
    }
  }
  //                                                                                                                                        |
  // ----------------------------------------- End of standard event handling ---------------------------------------------------------------
  

  // ------------------------------------ Start Compare predicted and read versions of helicity and act if needed ---------------------------
  //                                                                                                                                        |
  if(fHelPred == 0) fHelDiff = 0;                           //if fHelPred is zero, diff should also be zero, since not really a difference.
  else fHelDiff = fHelPred - Helb2h[f_helicity];            //Work out the difference between predicted and read helicity.

  
  if(fHelDiff!=0){                                          //If there's a real discrepancy between predicted and read hel(delayed)
    fDiffVeto = 0;                                          //Set veto to 0 (ie veto set hel to 0 in GetHelicity())
    fDiffErrorCount++;                                      //and add to accumulated errors
    printf("HEL: Warning: Helicity mismatch in event %ld.\n",fEventno);
  }
  if(fDiffErrorCount) fDiffEventCount++;                    //if there's been an error, count events since it happened
  
  if(fDiffEventCount>fDiffEventMax){                        //at the limit of the events to consider after a mismatch 
    if(fDiffErrorCount > fDiffErrorMax){                    //If more than the allowed no of mismatches within the next bunch of events
      printf("HEL: Warning: Helicity mismatch in event %ld. Max no of mismatches (%d) in  %d events reached. Re-reading the 30 bit word for the random seed.\n",fEventno,fDiffErrorMax,fDiffEventMax);
      fNBitsRead = 0;                                       //Set the no of bits to zero, which will trigger the re-reading of the bits for the random seed.
      fBits      = 0;                                       //Set the 30 bit seed word to 0;
    }
    else{
      fDiffVeto  = 1;                                       //stop vetoing events
    } 
    fDiffErrorCount  = 0;                                   //and reset the counters
    fDiffEventCount  = 0;
  }  
  //                                                                                                                                        |
  // -------------------------------------End Compare predicted and read versions of helicity and act if needed ----------------------------

    


  fHelicity = fDiffVeto*fHWP*fHelNow;                       //final helicity is product of predicted * HPW state and veto.
  fEventRateCount++;                                        //increment the rate counter

  fEventInRun++;                                            //and other counters
  if       (fHelicity == -1) fMinusInRun++;
  else if  (fHelicity ==  1) fPlusInRun++;
  else                       fNullInRun++;

  if(HELIVERBOSE > 0)printEvent();

  m_mtx.unlock();                                            // Unlock main mutex
  
  return; //NOERROR;
}

//------------------
// EndRun
//------------------
void JEventProcessor_HELI_online::EndRun(){
  // This is called whenever the run number changes, before it is
  // changed to give you a chance to clean up before processing
  // events from the next run number.

  printf("\n\n*********************************************************************************\n");
  printf("** Beam Helicity Summary for Run %d                                    **\n\n",fRunNumber);
  printf("HEL: Total number of events processed:      %ld\n", fEventInRun); 
  printf("HEL: Total with Plus HEL::              %ld (= %5.2lf %%)\n", fPlusInRun, 100.0*(double)fPlusInRun/(double)fEventInRun); 
  printf("HEL: Total with Minus HEL::             %ld (= %5.2lf %%)\n", fMinusInRun, 100.0*(double)fMinusInRun/(double)fEventInRun); 
  printf("HEL: Total with Undefined HEL::         %ld (= %5.2lf %%)\n", fNullInRun, 100.0*(double)fNullInRun/(double)fEventInRun);
  printf("HEL: Beam Charge Asymmetry (p-m)/(p+m)      %lf +/- %lf ( = %lf %% +/- %lf %% )\n", fHelAsym,fHelAsymError,fHelAsym,fHelAsymError);
  printf("\n\n*********************************************************************************\n");

  if(HELIVERBOSE > 0){                                      // write the final value of the random seed, can be used to start the next file from the same run.
    fprintf(dFile,"#Seed %d\n",fBits);
  }
    
  return; //NOERROR;
}

//------------------
// Finish
//------------------
void JEventProcessor_HELI_online::Finish(void){
  // Called before program exit after event processing is finished.
  if(dFile!=NULL)
  	fclose(dFile);
  return; //NOERROR;
}

int  JEventProcessor_HELI_online::nextRand(uint32_t *bits){  // Apply the shift register algorithm to generate pseudorandom number (0,1)
  uint result=0;
  uint bit7,bit28,bit29,bit30;
  
  bit7        = (*bits & 0x00000040) != 0;
  bit28       = (*bits & 0x08000000) != 0;
  bit29       = (*bits & 0x10000000) != 0;
  bit30       = (*bits & 0x20000000) != 0;
  result      = (bit30 ^ bit29 ^ bit28 ^ bit7) & 0x1;
  *bits       = ((*bits << 1 )| result ) & 0x3FFFFFFF;
  return result;
}

// Below here stuff to read setup parameters from calib database, or text file.

int JEventProcessor_HELI_online::readParms(int source=0){    // Read setup etc from database or file

  int  nline=0;
  char lline[200];
  char keyword[30];
  FILE *parmfile = NULL;
  static const char *keys[] =  {"HEL_DELAYd", "HEL_PATTERNd", "HEL_USE_T_SETTLE", "HEL_HWP_USE_EPICS", "HEL_HWP_STATE", "HEL_HWP_PV",     NULL};
  enum                 eHelKEys{ HEL_DELAYd,   HEL_PATTERNd,   HEL_USE_T_SETTLE,   HEL_HWP_USE_EPICS,   HEL_HWP_STATE,   HEL_HWP_PV,  HEL_NULL};

  int key = 0;

  fWinDelN        = 0;
  fPolPatN        = 0;
  fUseTSetN       = 0;
  fHWPEPICSUseN   = 0;
  fHWPStateN      = 0;
  fHWPEPICSChanN  = 0;

  printf("\n\n********************************************************************************\n");
  printf("** Setting up Beam Helicity Handler Params from file - Started                **\n\n");
  if(source){
    parmfile = fopen(HELISETUP.c_str(),"r");
    if(parmfile==NULL){
      printf("HEL: Warning: Failed to open helicity_setup.txt\n");
      return -1;
    }
    else{
      printf("HEL: Opened helicity_setup.txt\n");
    }
    while(fgets(lline,200,parmfile) != NULL){
      nline++;
      //printf("Scanned line %d: %s",nline,lline);
      
      if((lline[0] == '*')||(lline[0] == '#')) continue; //skip comments
      if(sscanf(lline,"%s",keyword)<1)continue;          //and blank lines
      key=0;
      while((key<HEL_NULL)&&(strcmp(keys[key],keyword)!=0))key++;
      if(key>=HEL_NULL) printf("HEL: Warning, unknown keyword in helicity_setup.txt: %s\n",keyword);
      else{
	//printf("HEL: Parsing line %d, key value=%d, key word=%s\n", nline, key,keys[key]);
	switch(key){
	case HEL_DELAYd:
	  if(sscanf(lline, "%*s%d%d%d",&fWinDelTabLow[fWinDelN],&fWinDelTabHigh[fWinDelN],&fWinDelTabValue[fWinDelN])==3) fWinDelN++;
	  else printf("HEL: Warning - failed to scan helicity_setup.txt line %d: %s\n",nline,lline);
	  break;
	case HEL_PATTERNd:
	  if(sscanf(lline, "%*s%d%d%d",&fPolPatTabLow[fPolPatN],&fPolPatTabHigh[fPolPatN],&fPolPatTabValue[fPolPatN])==3) fPolPatN++;
	  else printf("HEL: Warning - failed to scan helicity_setup.txt line %d: %s\n",nline,lline);
	  break;

	case HEL_HWP_PV:
	  if(sscanf(lline, "%*s%d%d%s",&fHWPEPICSChanTabLow[fHWPEPICSChanN],&fHWPEPICSChanTabHigh[fHWPEPICSChanN],fHWPEPICSChanTabValue[fHWPEPICSChanN])==3) fHWPEPICSChanN++;
	  else printf("HEL: Warning - failed to scan helicity_setup.txt line %d: %s\n",nline,lline);
	  break;

	case HEL_HWP_USE_EPICS:
	  if(sscanf(lline, "%*s%d%d%d",&fHWPEPICSUseTabLow[fHWPEPICSUseN],&fHWPEPICSUseTabHigh[fHWPEPICSUseN],&fHWPEPICSUseTabValue[fHWPEPICSUseN])==3) fHWPEPICSUseN++;
	  else printf("HEL: Warning - failed to scan helicity_setup.txt line %d: %s\n",nline,lline);
	  break;

	case HEL_HWP_STATE:
	  if(sscanf(lline, "%*s%d%d%d",&fHWPStateTabLow[fHWPStateN],&fHWPStateTabHigh[fHWPStateN],&fHWPStateTabValue[fHWPStateN])==3) fHWPStateN++;
	  else printf("HEL: Warning - failed to scan helicity_setup.txt line %d: %s\n",nline,lline);
	  break;

	case HEL_USE_T_SETTLE:
	  if(sscanf(lline, "%*s%d%d%d",&fUseTSetTabLow[fUseTSetN],&fUseTSetTabHigh[fUseTSetN],&fUseTSetTabValue[fUseTSetN])==3) fUseTSetN++;
	  else printf("HEL: Warning - failed to scan helicity_setup.txt line %d: %s\n",nline,lline);

	default:
	  break;
	}
      }
    }
    printf("\n** Setting up Beam Helicity Handler Params from file - Ended                  **\n");
    printf("********************************************************************************\n\n");
  }
  else{ //read from the cal database
  }
  return 0;
}


int JEventProcessor_HELI_online::setRunParms(int run){                               // Set parms for specific run

  

  int i=0;
  while(((run<fWinDelTabLow[i])||(run>fWinDelTabHigh[i]))&&(i<fWinDelN))i++;
  if(i<fWinDelN) fWinDelValue = fWinDelTabValue[i];
  else printf("HEL: Warning. No value for param HELDELAYd for run %d. Using previous value of %d\n",run,fWinDelValue);

  i=0;
  while(((run<fPolPatTabLow[i])||(run>fPolPatTabHigh[i]))&&(i<fPolPatN))i++;
  if(i<fPolPatN) fPolPatValue = fPolPatTabValue[i];
  else printf("HEL: Warning. No value for param HELPATTERNd for run %d. Using previous value of %d\n",run,fPolPatValue);

  i=0;
  while(((run<fHWPEPICSChanTabLow[i])||(run>fHWPEPICSChanTabHigh[i]))&&(i<fHWPEPICSChanN))i++;
  if(i<fHWPEPICSChanN) strcpy(fHWPEPICSChanValue,fHWPEPICSChanTabValue[i]);
  else printf("Warning. No value for param HWP_EPICS_PV for run %d. Using previous value of %s\n",run,fHWPEPICSChanValue);

  i=0;
  while(((run<fHWPEPICSUseTabLow[i])||(run>fHWPEPICSUseTabHigh[i]))&&(i<fHWPEPICSUseN))i++;
  if(i<fHWPEPICSUseN) fHWPEPICSUseValue = fHWPEPICSUseTabValue[i];
  else printf("Warning. No value for param HWP_USE_EPICS for run %d. Using previous value of %d\n",run,fHWPEPICSUseValue);

  i=0;
  while(((run<fHWPStateTabLow[i])||(run>fHWPStateTabHigh[i]))&&(i<fHWPStateN))i++;
  if(i<fHWPStateN) fHWPStateValue = fHWPStateTabValue[i];
  else printf("HEL: Warning. No value for param HWP_STATE for run %d. Using previous value of %d\n",run,fHWPStateValue);

  i=0;
  while(((run<fUseTSetTabLow[i])||(run>fUseTSetTabHigh[i]))&&(i<fUseTSetN))i++;
  if(i<fUseTSetN) fUseTSetValue = fUseTSetTabValue[i];
  else printf("HEL: Warning. No value for param USE_T_SETTLE for run %d. Using previous value of %d\n",run,fUseTSetValue);

  return 0;
}

int JEventProcessor_HELI_online::printEvent(){
  dHelBits[0]  = f_t_settle;
  dHelBits[1]  = f_pattern_sync;
  dHelBits[2]  = f_pair_sync;
  dHelBits[3]  = f_helicity;
  //dHelBits[3]  = Helb2h[f_helicity];
  dHelBits[4]  = f_ihwp;
  dHelBits[5]  = fHelPred;                                 
  dHelBits[6]  = fHelNow;                                  
  dHelBits[7]  = fHelDiff;                                 
  dHelBits[8]  = (int)(fEventRate/1000);                                 
  dHelBits[9]  = f_beam_on;                                 
  dHelBits[10] = fHelicity;                                 
  
  fprintf(dFile,"%lu %lu", fEventno,fReferenceClockTime);  //print events to file. No # tag for these, but all other log lines tagged with #label
  for(int n=0;n<11;n++){                                  //and all the bits
    fprintf(dFile," %d",dHelBits[n]);
  }
  fprintf(dFile," %s\n", HelLatestTag[fIsLatest]);        //and the tag to say whether it's the latest event so far
  return 0;
}
