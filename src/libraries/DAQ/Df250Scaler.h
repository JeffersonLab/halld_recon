#ifndef _Df250Scaler_
#define _Df250Scaler_

#include <JANA/JObject.h>

using namespace std;

class Df250Scaler : public JObject {
public:
	JOBJECT_PUBLIC(Df250Scaler);

	uint32_t nsync;
	uint32_t trig_number;
	uint32_t version;

	int crate;

	vector<uint32_t> fa250_sc;

	// This method is used primarily for pretty printing
	// the second argument to AddString is printf style format
	void Summarize(JObjectSummary &summary) const override {
		summary.add(nsync, NAME_OF(nsync), "%d");
		summary.add(trig_number, NAME_OF(trig_number), "%d");
		summary.add(version, NAME_OF(version), "%d");
		summary.add(crate, NAME_OF(crate), "%d");
		summary.add(fa250_sc.size(), "fa250_sc", "%d");
	}
};

#endif // _Df250Scaler_

