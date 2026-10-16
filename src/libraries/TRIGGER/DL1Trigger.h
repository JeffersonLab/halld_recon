#ifndef _DL1Trigger_
#define _DL1Trigger_

#include <JANA/JObject.h>

using std::vector;

class DL1Trigger:public JObject{
 public:
  JOBJECT_PUBLIC(DL1Trigger);
  
  DL1Trigger():event_type(0),timestamp(0),trig_mask(0),fp_trig_mask(0),nsync(0),trig_number(0),live(0),busy(0),live_inst(0),unix_time(0){}

  
  int event_type;  // 0 - unknown , 1 - trigger   2 - SYNC and trig
  
  
  uint64_t timestamp; 
  uint32_t trig_mask;
  uint32_t fp_trig_mask;
  

  uint32_t nsync;
  uint32_t trig_number;
  uint32_t live;
  uint32_t busy;
  uint32_t live_inst;
  uint32_t unix_time;

  vector<uint32_t> gtp_sc;
  vector<uint32_t> fp_sc;
  vector<uint32_t> gtp_rate;
  vector<uint32_t> fp_rate;

 
  typedef struct {
    int line;
    int type;
    int fcal;
    int bcal;
    float en_thr;
    int nhit;
    unsigned int pattern;
    int prescale;
  } gtp_par;
  
  vector<gtp_par> gtp_conf; 
  

  // the second argument to AddString is printf style format
  void Summarize(JObjectSummary& summary) const override {
    summary.add(timestamp , "timestamp", "%ld");     
    summary.add(event_type , "event_type", "%d");
    summary.add(trig_mask , "trig_mask", "0x%08x");
    summary.add(fp_trig_mask , "fp_trig_mask", "0x%08x");    

    summary.add(nsync, "nsync", "%d");
    summary.add(trig_number, "trig_number", "%d");
    summary.add(live, "live", "%d");
    summary.add(busy, "busy", "%d");
    summary.add(live_inst, "live_inst", "%d");
    summary.add(unix_time, "unix_time", "%d");
		  
    summary.add(gtp_sc.size(), "gtp_sc", "%d");
    summary.add(fp_sc.size(), "fp_sc", "%d");
    summary.add(gtp_rate.size(), "gtp_rate", "%d");
    summary.add(fp_rate.size(), "fp_rate", "%d");

  }
  
};

#endif // _DL1Trigger_

