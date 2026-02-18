//
//    File: JEventProcessor_PStagstudy.h
//

#ifndef _JEventProcessor_PStagstudy_
#define _JEventProcessor_PStagstudy_

#include <JANA/JEventProcessor.h>
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

   TTree *pstags;

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
   int rf_sys[999];
   double rf_time[999];

   int ntagm;
   int tagm_seqno[999];
   int tagm_channel[999];
   float tagm_Etag[999];
   float tagm_peak[999];
   float tagm_pint[999];
   float tagm_tadc[999];
   float tagm_toth[999];
   float tagm_ttdc[999];
   float tagm_time[999];
   float tagm_pmax[999];
   float tagm_ped[999];
   float tagm_tlast[999];
   float tagm_plast[999];
   float tagm_base[999];
   float tagm_rothr[999];
   int tagm_multi[999];
   int tagm_qf[999];
   int tagm_bg[999];
   int tagm_has_adc[999];
   int tagm_has_tdc[999];
   int tagm_nped[999];
   int tagm_nint[999];
   std::vector<std::vector<unsigned short> > tagm_raw_waveform;

   int ntagh;
   int tagh_seqno[999];
   int tagh_counter[999];
   float tagh_Etag[999];
   float tagh_peak[999];
   float tagh_pint[999];
   float tagh_tadc[999];
   float tagh_toth[999];
   float tagh_ttdc[999];
   float tagh_time[999];
   float tagh_pmax[999];
   float tagh_ped[999];
   float tagh_tlast[999];
   float tagh_plast[999];
   float tagh_base[999];
   float tagh_rothr[999];
   int tagh_multi[999];
   int tagh_qf[999];
   int tagh_bg[999];
   int tagh_has_adc[999];
   int tagh_has_tdc[999];
   int tagh_nped[999];
   int tagh_nint[999];
   std::vector<std::vector<unsigned short> > tagh_raw_waveform;

   int nbeam;
   int beam_sys[999];
   float beam_E[999];
   float beam_t[999];
   float beam_z[999];

   int nps;
   int ps_seqno[999];
   int ps_arm[999];   // North(left): 0, South(right): 1
   int ps_column[999];
   float ps_E[999];
   float ps_t[999];
   float ps_tadc[999];
   float ps_pint[999];
   float ps_peak[999];
   float ps_npix[999];
   float ps_toth[999];
   float ps_pmax[999];
   float ps_ped[999];
   int ps_multi[999];
   int ps_qf[999];
   int ps_nped[999];
   int ps_nint[999];
   std::vector<std::vector<unsigned short> > ps_raw_waveform;

   int npsc;
   int psc_seqno[999];
   int psc_arm[999];
   int psc_module[999];
   int psc_counter[999];
   float psc_t[999];
   float psc_pint[999];
   float psc_peak[999];
   float psc_ttdc[999];
   float psc_tadc[999];
   float psc_toth[999];
   float psc_npe[999];
   float psc_pmax[999];
   float psc_ped[999];
   int psc_has_adc[999];
   int psc_has_tdc[999];
   int psc_multi[999];
   int psc_qf[999];
   int psc_bg[999];
   int psc_nped[999];
   int psc_nint[999];
   std::vector<std::vector<unsigned short> > psc_raw_waveform;

   int npairps;
   float Epair[999];
   float tpair[999];
   float psleft_peak[999];
   float psright_peak[999];
   float psleft_pint[999];
   float psright_pint[999];
   float psleft_time[999];
   float psright_time[999];
   float psEleft[999];
   float psEright[999];
   float pstleft[999];
   float pstright[999];
   int nleft_ps[999];
   int nright_ps[999];

   int npairpsc;
   int pscleft_seqno[999];
   int pscright_seqno[999];
   int pscleft_module[999];
   int pscright_module[999];
   float pscleft_peak[999];
   float pscright_peak[999];
   float pscleft_pint[999];
   float pscright_pint[999];
   float pscleft_ttdc[999];
   float pscright_ttdc[999];
   float pscleft_tadc[999];
   float pscright_tadc[999];
   float pscleft_t[999];
   float pscright_t[999];
   float pscleft_ped[999];
   float pscright_ped[999];
   int pscleft_qf[999];
   int pscright_qf[999];

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
