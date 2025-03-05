// $Id$
//
//    File: JEventProcessor_HELI_online.h
// Created: Thu Jun 22 16:59:57 EDT 2023
// Creator: jrsteven (on Linux ifarm1801.jlab.org 3.10.0-1160.90.1.el7.x86_64 x86_64)
//

#ifndef _JEventProcessor_HELI_online_
#define _JEventProcessor_HELI_online_

#include "TH1F.h"
#include "TTree.h"

#include <JANA/JFactoryT.h>
//#include <JANA/JEvent.h>
#include <JANA/JEventProcessor.h>
#include <mutex>
#include <DAQ/DBeamHelicity.h>

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



class JEventProcessor_HELI_online:public JEventProcessor{
 public:
  JEventProcessor_HELI_online();
  ~JEventProcessor_HELI_online();
  int         getHelicity(){return fHelicity;};                         // The whole point of the exercise - to return helicity -1,+1, 0(unknown)
  enum eHelPat{PAIR,QUARTET,OCTET,TOGGLE,HEXOQUAD, OCTOQUAD};

 private:
  std::mutex m_mtx;
  void Init() override;
  void BeginRun(const std::shared_ptr<const JEvent>& event) override;
  void Process(const std::shared_ptr<const JEvent>& event) override;
  void EndRun() override;
  void Finish() override;		        ///< Called after last event of last event source has been processed.
  int nextRand(uint32_t *bits);                                         // generate next bit pattern in the sequence, return result bit. 
  int readParms(int source);                                            // Read setup etc from database or file
  int setRunParms(int run);                                             // Set parms for specific run
  int printEvent();                                                     // print the details of the curren event (for debugging).

  //debugging stuff
  TTree    *dBeamHelTree;
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

  //  int       fDebugLevel;
  
		 
};

#endif // _JEventProcessor_HELI_online_

