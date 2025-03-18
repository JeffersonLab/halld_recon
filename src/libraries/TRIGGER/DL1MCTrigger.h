#ifndef _DL1MCTrigger_
#define _DL1MCTrigger_

#include <JANA/JObject.h>

class DL1MCTrigger: public JObject{
 public:
  JOBJECT_PUBLIC(DL1MCTrigger);
  
 DL1MCTrigger():trig_mask(0),fcal_en(0),fcal_adc(0),fcal_adc_en(0),fcal_gtp(0),fcal_gtp_en(0),
    bcal_en(0),bcal_adc(0),bcal_adc_en(0),bcal_gtp(0),bcal_gtp_en(0) {}
 
  
  uint32_t trig_mask;
  
  float   fcal_en;
  int     fcal_adc;
  float   fcal_adc_en;
  int     fcal_gtp;
  float   fcal_gtp_en;

  float   bcal_en;
  int     bcal_adc;
  float   bcal_adc_en;
  int     bcal_gtp;
  float   bcal_gtp_en;

  int trig_time[32];


  // the second argument to AddString is printf style format
  void Summarize(JObjectSummary& summary) const override {
    summary.add(trig_mask , "trig_mask", "0x%08x");
    
    summary.add(fcal_en, "FCAL E(GeV)", "%6.3f");
    summary.add(fcal_adc, "FCAL ADC E(cnt)", "%d");
    summary.add(fcal_adc_en, "FCAL ADC E(GeV)", "%6.3f");
    summary.add(fcal_gtp, "FCAL GTP E(cnt)", "%d");
    summary.add(fcal_gtp_en, "FCAL GTP E(GeV)", "%6.3f");
    
    summary.add(bcal_en, "BCAL E(GeV)", "%6.3f");
    summary.add(bcal_adc, "BCAL ADC E(cnt)", "%d");
    summary.add(bcal_adc_en, "BCAL ADC E(GeV)", "%6.3f");
    summary.add(bcal_gtp, "BCAL GTP E(cnt)", "%d");
    summary.add(bcal_gtp_en, "BCAL GTP E(GeV)", "%6.3f");

    summary.add(trig_time[0], "Trig Time (samp)", "%d");
  }
  
};

#endif // _DL1MCTrigger_

