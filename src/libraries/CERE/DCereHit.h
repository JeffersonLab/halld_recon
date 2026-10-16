/*
 * DCereHit.h
 *
 *  Created on: Oct 10, 2012
 *      Author: yqiang
 */

#ifndef DCEREHIT_H_
#define DCEREHIT_H_

#include <JANA/JObject.h>

class DCereHit: public JObject {

public:
	JOBJECT_PUBLIC (DCereHit);

	int sector; // sector hit by photons
	float pe;	// number of photon electrons
	float t;	// time

	void Summarize(JObjectSummary& summary) const override {
		summary.add(sector, "sector", "%d");
		summary.add(pe, "pe", "%1.3f");
		summary.add(t, "t", "%1.3f");
	}
};

#endif /* DCEREHIT_H_ */
