
//
//    File: DCDCHit_factory.h
// Created: Tue Aug  6 11:29:56 EDT 2013
// Creator: davidl (on Darwin harriet.jlab.org 11.4.2 i386)
//

#ifndef _DCDCHit_factory_
#define _DCDCHit_factory_

#include <JANA/JFactoryT.h>
#include <DAQ/Df125CDCPulse.h>
#include <TTAB/DTranslationTable.h>

#include "DCDCHit.h"

using namespace std;


class DCDCHit_factory: public JFactoryT<DCDCHit>{
 public:
  DCDCHit_factory() = default;
  ~DCDCHit_factory() override = default;
  
  // we need to store information on the hits with respect to their readout channels in order to look for correlated hits
  struct cdchit_info_t{
    uint32_t rocid;
    uint32_t slot;
    uint32_t connector;
    
    double time;
    double max;
    
    inline bool operator==(const struct cdchit_info_t &rhs) const {
      return (rocid==rhs.rocid) && (slot==rhs.slot) && (connector==rhs.connector);
    }
  };
  
  
  int RemoveCorrelationHits;
  double CorrelationHitsCut;
  double CorrelatedHitPeak;
  int Disable_CDC_TimingCuts;

  // timing cut limits
  double LowTCut;
  double HighTCut;
  
 private:
  void Init() override;
  void BeginRun(const std::shared_ptr<const JEvent>& event) override;
  void Process(const std::shared_ptr<const JEvent>& event) override;
  void EndRun() override;
  void Finish() override;
  
	bool USE_CDC;  
  vector<const DTranslationTable *> ttab;
};

#endif // _DCDCHit_factory_

