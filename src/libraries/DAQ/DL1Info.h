// $Id$
// $HeadURL$
//
//    File: DL1Info.h


#ifndef _DL1Info_
#define _DL1Info_

#include <JANA/JObject.h>

using namespace std;

class DL1Info : public JObject {
public:
	JOBJECT_PUBLIC(DL1Info);

	uint32_t nsync;
	uint32_t trig_number;
	uint32_t live_time;
	uint32_t busy_time;
	uint32_t live_inst;
	uint32_t unix_time;

	vector<uint32_t> gtp_sc;
	vector<uint32_t> fp_sc;
	vector<uint32_t> gtp_rate;
	vector<uint32_t> fp_rate;


	void Summarize(JObjectSummary &summary) const override {
		summary.add(nsync, NAME_OF(nsync), "%d");
		summary.add(trig_number, NAME_OF(trig_number), "%d");
		summary.add(live_time, NAME_OF(live_time), "%d");
		summary.add(busy_time, NAME_OF(busy_time), "%d");
		summary.add(live_inst, NAME_OF(live_inst), "%d");
		summary.add(unix_time, NAME_OF(unix_time), "%d");

		summary.add(gtp_sc.size(), "gtp_sc", "%d");
		summary.add(fp_sc.size(), "fp_sc", "%d");
		summary.add(gtp_rate.size(), "gtp_rate", "%d");
        summary.add(fp_rate.size(), "fp_rate", "%d");
	}

	//		void Summarize(JObjectSummary& summary) const override {
	//			summary.add(rocid, NAME_OF(rocid), "%d");
	//			summary.add(timestamp, NAME_OF(timestamp), "%ld");
	//          summary.add(misc.size(), "Nmisc", "%d");
	//		}

};

#endif // _DL1Info_

