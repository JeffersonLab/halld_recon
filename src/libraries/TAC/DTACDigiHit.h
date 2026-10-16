/*
 * DTACDigiHit.h
 *
 *  Created on: Mar 24, 2017
 *      Author: Hovanes Egiyan
 */

#ifndef LIBRARIES_TAC_DTACDIGIHIT_H_
#define LIBRARIES_TAC_DTACDIGIHIT_H_

#include <JANA/JObject.h>

class DTACDigiHit: public JObject {
public:
	JOBJECT_PUBLIC(DTACDigiHit)

	uint32_t pulse_integral;          ///< identified pulse integral as returned by FPGA algorithm
	uint32_t pulse_time;          ///< identified pulse time as returned by FPGA algorithm
	uint32_t pedestal;          ///< pedestal info used by FPGA (if any)
	uint32_t QF;             ///< Quality Factor from FPGA algorithms
	uint32_t nsamples_integral;          ///< number of samples used in integral
	uint32_t nsamples_pedestal;          ///< number of samples used in pedestal
	uint32_t pulse_peak;           ///<  maximum sample in pulse
	uint32_t datasource;          ///<  0=window raw data, 1=old(pre-Fall16) firmware, 2=Df250PulseData



	void Summarize(JObjectSummary& summary) const override {
		summary.add(pulse_integral, NAME_OF(pulse_integral), "%d");
		summary.add(pulse_peak, NAME_OF(pulse_peak), "%d");
		summary.add(pulse_time, NAME_OF(pulse_time), "%d");
		summary.add(pedestal, NAME_OF(pedestal), "%d");
		summary.add(QF, NAME_OF(QF), "%d");
		summary.add(nsamples_integral, NAME_OF(nsamples_integral), "%d");
		summary.add(nsamples_pedestal, NAME_OF(nsamples_pedestal), "%d");
	}

	uint32_t getDataSource() const {
		return datasource;
	}

	void setDataSource(uint32_t datasource) {
		this->datasource = datasource;
	}

	uint32_t getNsamplesIntegral() const {
		return nsamples_integral;
	}

	void setNsamplesIntegral(uint32_t nsamplesIntegral) {
		nsamples_integral = nsamplesIntegral;
	}

	uint32_t getNsamplesPedestal() const {
		return nsamples_pedestal;
	}

	void setNsamplesPedestal(uint32_t nsamplesPedestal) {
		nsamples_pedestal = nsamplesPedestal;
	}

	uint32_t getPedestal() const {
		return pedestal;
	}

	void setPedestal(uint32_t pedestal) {
		this->pedestal = pedestal;
	}

	uint32_t getPulseIntegral() const {
		return pulse_integral;
	}

	void setPulseIntegral(uint32_t pulseIntegral) {
		pulse_integral = pulseIntegral;
	}

	uint32_t getPulseTime() const {
		return pulse_time;
	}

	void setPulseTime(uint32_t pulseTime) {
		pulse_time = pulseTime;
	}

	uint32_t getQF() const {
		return QF;
	}

	void setQF(uint32_t qf) {
		QF = qf;
	}

	uint32_t getPulsePeak() const {
		return pulse_peak;
	}

	void setPulsePeak(uint32_t pulsePeak) {
		pulse_peak = pulsePeak;
	}
};

#endif /* LIBRARIES_TAC_DTACDIGIHIT_H_ */
