/*
 * DTACTDCDigiHit.h
 *
 *  Created on: May 3, 2017
 *      Author: Hovanes Egiyan
 */

#ifndef LIBRARIES_TAC_DTACTDCDIGIHIT_H_
#define LIBRARIES_TAC_DTACTDCDIGIHIT_H_

#include <JANA/JObject.h>


class DTACTDCDigiHit: public JObject {
public:
	uint32_t time = 0;
public:
	JOBJECT_PUBLIC(DTACTDCDigiHit)

	DTACTDCDigiHit() {
	}
	virtual ~DTACTDCDigiHit() {
	}

	void Summarize(JObjectSummary& summary) const override {
		summary.add(time, NAME_OF(time), "%d");
	}

	uint32_t getTime() const {
		return time;
	}

	void setTime(uint32_t time = 0) {
		this->time = time;
	}
};

#endif /* LIBRARIES_TAC_DTACTDCDIGIHIT_H_ */
