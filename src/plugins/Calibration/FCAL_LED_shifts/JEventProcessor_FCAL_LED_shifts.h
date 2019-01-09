// $Id$
//
//    File: JEventProcessor_FCAL_LED_shifts.h
// Created: Fri Jan 30 08:18:41 EST 2015
// Creator: shepherd (on Linux ifarm1102 2.6.32-220.7.1.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_FCAL_LED_shifts_
#define _JEventProcessor_FCAL_LED_shifts_

#include <JANA/JEventProcessor.h>
#include <FCAL/DFCALGeometry.h>
#include <TTAB/DTranslationTable.h>

#include <TH1I.h>
#include <TH2I.h>

#include <vector>
#include <map>

class DFCALHit;
class TTree;

class JEventProcessor_FCAL_LED_shifts:public jana::JEventProcessor{
 public:

 
  JEventProcessor_FCAL_LED_shifts();
  ~JEventProcessor_FCAL_LED_shifts();
  const char* className(void){return "JEventProcessor_FCAL_LED_shifts";}

 private:

  jerror_t init(void);						///< Called once at program start.
  jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
  jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
  jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
  jerror_t fini(void);						///< Called after last event of last event source has been processed.


  bool CALC_NEW_CONSTANTS_LED;
  bool CALC_NEW_CONSTANTS_BEAM;
  string REFERENCE_FILE_NAME;
  
  double FCAL_TOTAL_ENERGY_HI;
  double FCAL_TOTAL_ENERGY_LO;
 
  const DTranslationTable *m_ttab;
  const DFCALGeometry* m_fcalGeom;
  
  int32_t m_runnumber;
  vector< double > old_ADCoffsets;

  map< uint32_t, TH1I * > m_crateTimes;
  map< pair<uint32_t,uint32_t>, TH1I * > m_slotTimes;
  vector< TH1I * > m_channelTimes;
  
  TH2I *m_fadcShifts;
  TH1I *m_totalEnergy;


  int NBINS_TIME;
  double TIME_MIN, TIME_MAX;

 
};

#endif // _JEventProcessor_FCAL_LED_shifts_

