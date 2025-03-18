#ifndef _Df250AsyncPedestal_
#define _Df250AsyncPedestal_

#include <JANA/JObject.h>

using namespace std;

class Df250AsyncPedestal : public JObject {
public:
	JOBJECT_PUBLIC(Df250AsyncPedestal);

	uint32_t nsync;
	uint32_t trig_number;
	int crate;
	vector<uint32_t> fa250_ped;

	void Summarize(JObjectSummary &summary) const override {
		summary.add(nsync, NAME_OF(nsync), "%d");
		summary.add(trig_number, NAME_OF(trig_number), "%d");
		summary.add(crate, NAME_OF(crate), "%d");
		summary.add(fa250_ped.size(), "fa250_ped", "%d");
	}
};

#endif // _Df250AsyncPedestal_

