#include "stdio.h"
// Stolen from my helicity plugin and hacked to work on the log file - KL.
//
// Here are the helicity signals (eg. quartet pattern (+--+ or -++-))
// Signals with a (U) get delivered to the Halls to go into the DAQ.
// Link to manual ...
//   
//  :                :               :                               :
//  |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   T-Settle (U)
// _|___|___|___|___|___|___|___|___|___|___|___|___|___|___|___|_
//  :___     ___    :___     ___     ___     ___     ___     ___
//  |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   Pair-sync (U)
// _|   |___|   |___|   |___|   |___|   |___|   |___|   |___|   |_   
//  :_0_  1   2   3 :___            :___            :___               
//  |   |           |   |           |   |           |   |           Pattern-sync (U)
// _|   |___________|   |___________|   |___________|   |__________   
//  :___         ___:    ___ ___    :    ___ ___    :    ___ ___     
//  | + | -  -  | + : - | +   + | - | - | +   + | - | - | +   + |   Helicity
// _|   |_______|   :___|       |___|___|       |___|___|       |_
//    *               *               *               *
//                                  :___         ___:    ___ ___    :    ___ ___        
//                                  | + | -  -  | + : - | +   + | - | - | +   + |   Delayed Helicity (U)
//   ...............................|   |_______|   :___|       |___|___|       |__
//   <---- 8 windows delay  --------> *               *               *          *                 
//
//
// 
// The 1st value in the pattern (indicated with *) is pseudo randomized by a 30 bit feedback shift register.
// After that, the subsequent helicity values are determined by the pattern (quartet here).
// If life was simple they could just deliver Helicity to the DAQs. However, the sneaky ****ers delay reporting it by some number of windows N (pair sync flips).
// This means that we know the helicity N windows ago and have to figure out what it is now by making a software version of the 30 bit feedback shift register. 
//
// Also need to remember that in multi-threaded code events don't come in order!
// .. so only pay attent to events which are furthest ahead in time when changing the state of the "current" values of pair_sync, pat_sync, hel_pred
// Then for other events, check to see if they are in the same window as the latest, or the previous window,
// and set the helicities as appropriate.


//some constants
static const int32_t           Helb2h[2] = {-1,+1};                    //small array to converge bit (0,1) to hel state(-1,1)
static const char       *HelLatestTag[2] = {"St","La"};                //tag debug output (St=Standard, or La=Last)

static const char     *HelPatternName[6] = {  "Pair","Quartet",   "Octet", "Toggle", "Hexo-Quad", "Octo-Quad"};
static const int32_t    HelPatternLen[6] = {      2 ,       4 ,        8 ,       2 ,         24 ,         32 };

static const int32_t  HelWinDelayMap[16] = {0,1,2,4,8,12,16,24,32,40,48,64,72,96,112,128};

static const int32_t  HelPattern[6][32]  = {                                                           //All the patterns
  {+1,-1},                                                                                             //Pair
  {+1,-1,-1,+1},                                                                                       //Quartet
  {+1,-1,-1,+1,-1,+1,+1,-1},                                                                           //Octet
  {+1,-1},                                                                                             //Toggle (ever used ?)
  {+1,-1,-1,+1,+1,-1,-1,+1,+1,-1,-1,+1,-1,+1,+1,-1,-1,+1,+1,-1,-1,+1,+1,-1},                           //Hexo-Quad
  {+1,-1,-1,+1,+1,-1,-1,+1,+1,-1,-1,+1,+1,-1,-1,+1,-1,+1,+1,-1,-1,+1,+1,-1,-1,+1,+1,-1,-1,+1,+1,-1}};  //Octo-Quad



enum eHelPat{PAIR,QUARTET,OCTET,TOGGLE,HEXOQUAD, OCTOQUAD};

static const uint64_t sigtminus = 10000;                               //~3 sigma of the time spread in the 30Hz pair_sync.
static const uint64_t sigtplus  = 35000;                               //~3 sigma of the time spread in the 30Hz pair_sync + 100us.


//debugging stuff
//TTree    *dBeamHelTree;
int       dHelBits[11];
FILE     *dFile;
//helicity stuff
uint32_t  fRunNumber;                     //Current run number
uint64_t  fEventno;                       //Number current event
uint64_t  fEventLatest;                   //Number of latest event (another thread may have done a future event).
uint      fIsLatest;
uint64_t  fEventRate;                     //To make an approximate count of the event rate.
uint64_t  fEventRateCount;                // ...
uint64_t  fEventRateThresh;               // ...

uint64_t  fEventInRun;                    //For file stats and running measurements of things via get()s.
uint64_t  fPlusInRun;	  
uint64_t  fMinusInRun;  
uint64_t  fNullInRun;	  
double    fHelAsym;	  
double    fHelAsymError;

uint      fNBitsReqd;                     //no of bits needed to be able to genetate pseudorandom sequence
uint      fNBitsRead;                     //no of bits accummulated
uint32_t  fBits;                          //shift register with delayed bit pattern (hel from this should match DAQ data)
uint32_t  fBitsNow;                       //shift register with now 
uint      fNWinDel;                       //no of windows used in delayed reporting
uint      fNPatDel;                       //no of patterns in above
int       fPolPat;                        //result from shift reg - whether to flip the polarity of the pattern (-1 or 1)
int       fPolPatNow;                       
uint      fPatSyncLatest;                 //state from previous event
uint      fPairSyncLatest;
int       fHelPredPrev;                   //prev helicity predicted by pseodo random sequence
int       fHelPredLatest;                 //latest
int       fHelPred;                       //current
int       fHelDelPrev;                    //prev delayed helicity read from the data stream
int       fHelDelLatest;                  //latest
int       fHelDel;                        //current
int       fHelNowPrev;                    //prev now helicity predicted by pseodo random sequence
int       fHelNowLatest;                  //latest
int       fHelNow;                        //current
int       fHelDiff;                       //difference between predicted and data stream
char     *fPatternName;
uint      fPatternLen;
uint      fPatNo;                         //Index of pattern
uint      fPatN;                          //position in pattern
uint      fPatFlag;                       //Flag start of pattern sync, and count events after it
uint      fPairFlag;                       //Flag start of pattern sync, and count events after it
int       fUseTSettle;                    //Use T settle information 
int       fUseHWPEPICS;                   //Use EPICS PV events to update WPH status
int       fHWP;                           //Half wave plate as multiplier (1 if out, -1 if in , or 0 if unknown) 
int       fDiffErrorCount;                //Count errors
int       fDiffErrorMax;                  //Max allowed errors before resetting the 30 bit word and starting again. 
int       fDiffEventCount;                //Count errors
int       fDiffEventMax;                  //Max allowed errors before resetting the 30 bit word and starting again. 
int       fDiffVeto;
uint      f_pattern_sync;
uint      f_t_settle;
uint      f_helicity;
uint      f_pair_sync;
uint      f_ihwp;
uint      f_beam_on;

int       fHelicity;                      //Returned by the getHelicity() function  
int       fHelicityLast;                 

//store parameters read from setupfile. Only HWPN is likely to change, but for consistency they are allowed a few different settings.
//HPW Stats up to 100, others up to 5. Hard coded numbers! 
int       fWinDelN;                       //No of delay windows
int       fWinDelTabLow[5];
int       fWinDelTabHigh[5];
int       fWinDelTabValue[5];
int       fWinDelValue;

int       fPolPatN;                       //Pattern
int       fPolPatTabLow[5];
int       fPolPatTabHigh[5];
int       fPolPatTabValue[5];
int       fPolPatValue;

int       fHWPEPICSChanN;                 //EPICS HWP PV Name
int       fHWPEPICSChanTabLow[5];
int       fHWPEPICSChanTabHigh[5];
char      fHWPEPICSChanTabValue[5][60];
char      fHWPEPICSChanValue[60];

int       fHWPEPICSUseN;                  //FLAG to use EPICS to update HWP state.
int       fHWPEPICSUseTabLow[5];
int       fHWPEPICSUseTabHigh[5];
int       fHWPEPICSUseTabValue[5];
int       fHWPEPICSUseValue;

int       fHWPStateN;                     //Half wave plate state. This is really the only one likely to change
int       fHWPStateTabLow[100];
int       fHWPStateTabHigh[100];
int       fHWPStateTabValue[100];
int       fHWPStateValue;

int       fUseTSetN;                      //FLAG to use t settle
int       fUseTSetTabLow[5];
int       fUseTSetTabHigh[5];
int       fUseTSetTabValue[5];
int       fUseTSetValue;

int       fIsInit = 0;
		 
int     HELIVERBOSE = 3;
char HELISETUP[]    = "helicity_setup.txt";
char   HELILOG[200] = "full_helicity.log";

uint64_t *sync_events;
uint64_t *sync_times;
int64_t td1;
int64_t td2;
uint64_t sync_ind=0;
uint64_t  nsync;
uint64_t eventnumber;
uint64_t timestamp;
uint64_t last_settle = 10;
uint64_t last_pair   = 10;


// This is called once at program startup. 
void Init();
void BeginRun(int run);
void EndRun(int run);
void ProcessRun(int run);
void ProcessEvent(char *line);
int nextRand(uint32_t *bits);                                         // generate next bit pattern in the sequence, return result bit. 
int readParms(int source);                                            // Read setup etc from database or file
int setRunParms(int run);                                             // Set parms for specific run
int printEvent();                                                     // print the details of the curren event (for debugging).

void makeBigTableNoSeed(const char *filelist, const char *helilog=NULL,const char *syncname=NULL, int nlines=0){
  int run        = 0;
  FILE *flist    = NULL;                                  //input filelist (of the form "<blahblah>_<runno>.list)
  FILE *logfile  = NULL;                                  //current logfile
  FILE *syncfile = NULL;
  char logfname[200];                                     //
  char line[200];
  char *p;
  if(helilog) sprintf(HELILOG,"%s",helilog);              //outputlogfile
  Init();

  nsync=0;                                                //if we are going to fake t-settle
  if(syncname){
    sync_events = new uint64_t[nlines+1];                   //to hold the sync times and events
    sync_times  = new uint64_t[nlines+1];
  
    syncfile    = fopen(syncname,"r");  
    while(fgets(line,200,syncfile)){                        
      sscanf(line,"%ld%ld",&sync_events[nsync],&sync_times[nsync]);
      nsync++;
    }
  }
  sync_events[nsync] = sync_events[nsync-1]+(sync_events[nsync-1]-sync_events[nsync-2]);  // add a sync event and time at the end.
  sync_times[nsync] = sync_times[nsync-1]+(sync_times[nsync-1]-sync_times[nsync-2]);
  
  sscanf(strstr(filelist,".list")-6,"%d.list",&run);
  //sscanf(filelist,"_%d.list",&run);                        //scan the run number from the filelist name
  BeginRun(run);                                           //do any setup for that run

  flist=fopen(filelist,"r");                               //a list of log files
  while(fgets(logfname,200,flist)){                        //process them all
    if (strstr(logfname,"#")) continue;                    //skip any #comments              
    if ((p = strchr(logfname, '\n')) != NULL)*p = '\0';    //strip off the newline char
    logfile = fopen(logfname,"r");
    while(fgets(line,200,logfile)){                        //get the string
      if (strstr(logfname,"#")) continue;                  //skip any #comments              
      ProcessEvent(line);                                  //process the event
    }
    fclose(logfile);
  }
  fclose(flist);                                           //close the list of files.
}

void Init(){
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
  fHelicityLast   = 0;                                      
  
  dFile           = NULL;
  
  readParms(1);                                            //read parameters for run ranges tables  0: from caldb, 1: from ./helicity_setup.txt
  
  fWinDelValue      =  4;                                  //then set to some defaults
  fPolPatValue      =  1;
  fHWPEPICSUseValue =  0;
  fHWPStateValue    = -1;
  fUseTSetValue     =  1;
  strcpy(fHWPEPICSChanValue,"IGL1I00OD16_16");
  
  
  if(HELIVERBOSE > 0){                                     //if debugging       
    dFile = fopen(HELILOG,"w");                    //open a separate file for logging (maybe from infile name later)
    for(int n=0;n<10;n++){                                 //init the array that holds event bit and helicity data.
      dHelBits[n]=0;
    }
  }
}


void BeginRun(int runnumber){
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
void ProcessEvent(char *line){

  uint locBH[11];
  sscanf(line,"%ld%ld%d%d%d%d%d%d%d%d%d%d%d",&eventnumber,&timestamp,
	 &locBH[0],&locBH[1],&locBH[2],&locBH[3],&locBH[4],&locBH[5],&locBH[6],&locBH[7],&locBH[8],&locBH[9],&locBH[10]);
  
  //do some event inits
  
  fEventno    = eventnumber;                              //save the event number
  fDiffVeto   = 1;                                        // sets hel to 0 in GetHel() functions when predicted != read helicity (assume OK)
  fIsLatest   = 0;
  //  For old data before decoder board.
  f_t_settle     = locBH[0];
  f_pattern_sync = locBH[1];               
  f_pair_sync    = locBH[2];
  f_helicity     = locBH[3];                  
  f_ihwp         = locBH[4];
  //fEventRate     = 1000*locBH[8];
  f_beam_on      = locBH[9];
  //For new data after decodeer board ... if we ever need to do it this way
  //f_t_settle     = locBH[2];
  //f_pattern_sync = locBH[3];               
  //f_pair_sync    = locBH[0];
  //f_helicity     = locBH[1];                  
  //f_ihwp         = locBH[4];
  ////fEventRate     = 1000*locBH[8];
  //f_beam_on      = locBH[9];

  //t settle present, so skip the event
  if(fUseTSettle && f_t_settle){ 
    fHelicity = 0;  
    if(HELIVERBOSE > 1)printEvent();
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

      //printf("%ld: Event rate of ~ %ld Hz. Resetting when rate get over ~ %ld Hz\n",fEventno,fEventRate,fEventRateThresh);
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
	if(HELIVERBOSE > 2){
	  fprintf(dFile,"#NewBit %ld, Hel %d, Got %d bits %d\n",fEventno,f_helicity,fNBitsRead,fBits);
	}
	if(fNBitsRead == fNBitsReqd){
	  fBitsNow = fBits;                               //copy the delayed generator word to the current one
	  for(uint d=0; d<fNPatDel;d++){                  //and move on by the required number of patterns to make it the present generator.
	    nextRand(&fBitsNow);
	    if(HELIVERBOSE > 2)fprintf(dFile,"#Got Extra Bit %d for Now Helicity\n",d+1);
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

  
  //final veto from fake t_settle based on timestamps 
  f_t_settle=0;                                           //no settle = 0
  while(sync_events[sync_ind]<fEventno)sync_ind++;        //get the index of the next sync event
  //fprintf(stdout,"%ld < %ld < %ld\n",sync_events[sync_ind-1],fEventno,sync_events[sync_ind]);
  td1 = timestamp - sync_times[sync_ind-1];
  td2 = sync_times[sync_ind] - timestamp;
  if(((timestamp>sync_times[sync_ind-1]-sigtminus)&&(timestamp<sync_times[sync_ind-1]+sigtplus))|| //check to see if its withing 3sigma of transition 
     ((timestamp>sync_times[sync_ind  ]-sigtminus)&&(timestamp<sync_times[sync_ind  ]+sigtplus))){
    f_t_settle=1;                                         //and fake the tsettle if needed
    //fprintf(stdout,"Faking %ld < %ld < %ld, %ld %ld\n",sync_events[sync_ind-1],fEventno,sync_events[sync_ind],td1,td2);
  }
  //apply the fake t_settle
  fHelicity*=(!f_t_settle);

  if       (fHelicity == -1) fMinusInRun++;
  else if  (fHelicity ==  1) fPlusInRun++;
  else                       fNullInRun++;

  //only print events where t_settle or pair sync changed
  //if((f_t_settle!=last_settle)||(f_pair_sync!=last_pair)){
  // if(f_pair_sync!=last_pair){
  if(fHelicity != fHelicityLast){
    if(HELIVERBOSE > 1)printEvent();
    //fprintf(stdout," %ud, %ud\n", f_t_settle, f_pair_sync);
  }
  fHelicityLast = fHelicity;
  last_settle = f_t_settle;
  last_pair   = f_pair_sync;

  return; //NOERROR;
}

//------------------
// EndRun
//------------------
void EndRun(){
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
void Finish(void){
  // Called before program exit after event processing is finished.
  fclose(dFile);
  return; //NOERROR;
}




int readParms(int source=1){    // Read setup etc from database or file

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
    parmfile = fopen(HELISETUP,"r");
    if(parmfile==NULL){
      printf("HEL: Warning: Failed to open helicity_setup.txt\n");
      return -1;
    }
    else{
      printf("HEL: Opened helicity_setup.txt\n");
    }
    while(fgets(lline,200,parmfile) != NULL){
      nline++;
      printf("Scanned line %d: %s",nline,lline);
      
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


int setRunParms(int run){                               // Set parms for specific run

  

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

int printEvent(){
  dHelBits[0]  = f_t_settle;
  dHelBits[1]  = f_pattern_sync;
  dHelBits[2]  = f_pair_sync;
  dHelBits[3]  = f_helicity;
  //dHelBits[4]  = f_ihwp;
  dHelBits[4]  = fHWP;
  dHelBits[5]  = fHelPred;                                 
  dHelBits[6]  = fHelNow;                                  
  dHelBits[7]  = fHelDiff;                                 
  dHelBits[8]  = (int)(fEventRate/1000);                                 
  dHelBits[9]  = f_beam_on;                                 
  dHelBits[10] = fHelicity;                                 
  
  
  fprintf(dFile,"%lu %lu", fEventno,timestamp);                        //print events to file. No # tag for these, but all other log lines tagged with #label
  for(int n=0;n<11;n++){                                  //and all the bits
    fprintf(dFile," %d",dHelBits[n]);
  }
  fprintf(dFile," %s\n", HelLatestTag[fIsLatest]);        //and the tag to say whether it's the latest event so far
  return 0;
}

int nextRand(uint32_t *bits){  // Apply the shift register algorithm to generate pseudorandom number (0,1)
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
