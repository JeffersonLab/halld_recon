//
//    File: JEventProcessor_PStagstudy.h
//

#ifndef _JEventProcessor_PStagstudy_
#define _JEventProcessor_PStagstudy_

#include <JANA/JEventProcessor.h>
#include "ANALYSIS/DTreeInterface.h"
#include <TTAB/DTranslationTable.h>
#include <DAQ/DBeamCurrent_factory.h>
#include <TTree.h>
#include <TH1S.h>
#include <TH1D.h>
#include <vector>


class JEventProcessor_PStagstudy : public JEventProcessor {
 public:
   JEventProcessor_PStagstudy();
   ~JEventProcessor_PStagstudy();
   const char* className(void) { return "JEventProcessor_PStagstudy"; }

   DTreeInterface* dTreeInterface;
   //static thread_local
   DTreeFillData dTreeFillData;

   int runno;
   int eventno;
   int trigger;
   unsigned long int timestamp;
   unsigned long int epochtime;
   unsigned long int epoch_reference;
   unsigned long int bctime;
   unsigned long int beamcurrent;

   DBeamCurrent_factory *bc_factory;

   TH1D *tagm_hpedestal[128];
   TH1D *tagh_hpedestal[300];

   int nrf;

   int ntagm;
   float tagm_ped[999];
   float tagm_tlast[999];
   float tagm_plast[999];
   std::vector<std::vector<unsigned short> > tagm_raw_waveform;

   int ntagh;
   float tagh_ped[999];
   float tagh_tlast[999];
   float tagh_plast[999];
   std::vector<std::vector<unsigned short> > tagh_raw_waveform;

   int nbeam;

   int nps;
   std::vector<std::vector<unsigned short> > ps_raw_waveform;

   int npsc;
   std::vector<std::vector<unsigned short> > psc_raw_waveform;

   int npairps;
   int npairpsc;

   const DTranslationTable::DChannelInfo GetDetectorIndex(const DTranslationTable *ttab,
                                                          DTranslationTable::csc_t csc);

 private:
   void Init() override;
   void BeginRun(const std::shared_ptr<const JEvent>& event) override;
   void Process(const std::shared_ptr<const JEvent>& event) override;
   void EndRun() override;
   void Finish() override;
};

#endif // _JEventProcessor_PStagstudy_
