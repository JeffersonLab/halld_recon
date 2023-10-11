// $Id$
//
//    File: JEventProcessor_FCAL_LED_shifts.h
// Created: Fri Jan 30 08:18:41 EST 2015
// Creator: shepherd (on Linux ifarm1102 2.6.32-220.7.1.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_FCAL_LED_shifts_
#define _JEventProcessor_FCAL_LED_shifts_

#include <JANA/JEventProcessor.h>
#include <JANA/Compatibility/JLockService.h>
#include <FCAL/DFCALGeometry.h>
#include <TTAB/DTranslationTable.h>

#include <TH1I.h>
#include <TH2I.h>

#include <vector>
#include <map>

class DFCALHit;
class TTree;

class JEventProcessor_FCAL_LED_shifts:public JEventProcessor{
 public:

 
  JEventProcessor_FCAL_LED_shifts();
  ~JEventProcessor_FCAL_LED_shifts();

 private:

  void Init() override;
  void BeginRun(const std::shared_ptr<const JEvent>& event) override;
  void Process(const std::shared_ptr<const JEvent>& event) override;
  void EndRun() override;
  void Finish() override;

  std::shared_ptr<JLockService> lockService;


  bool CALC_NEW_CONSTANTS_LED;
  bool CALC_NEW_CONSTANTS_BEAM;
  string REFERENCE_FILE_NAME;
  
  double FCAL_TOTAL_ENERGY_HI;
  double FCAL_TOTAL_ENERGY_LO;
 
  int32_t m_runnumber;
  vector< double > old_ADCoffsets;

  map< uint32_t, TH1I * > m_crateTimes;
  map< pair<uint32_t,uint32_t>, TH1I * > m_slotTimes;
  vector< TH1I * > m_channelTimes;
  
  TH2I *m_fadcShifts;
  TH1I *m_totalEnergy;
  
  const DFCALGeometry* m_fcalGeom;
  const DTranslationTable* m_ttab;

  int NBINS_TIME;
  double TIME_MIN, TIME_MAX;

 
};

#endif // _JEventProcessor_FCAL_LED_shifts_

