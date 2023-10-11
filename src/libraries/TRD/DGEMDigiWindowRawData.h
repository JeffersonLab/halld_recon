// $Id$
//
//    File: DGEMDigiWindowRawData.h
//

#ifndef _DGEMDigiWindowRawData_
#define _DGEMDigiWindowRawData_

#include <JANA/JObject.h>

class DGEMDigiWindowRawData : public JObject{
	public:
		JOBJECT_PUBLIC(DGEMDigiWindowRawData);
		
		uint32_t plane;
		uint32_t strip;
		// other information retrieved via object in DAQ library

		//uint32_t pulse_peak;      ///< identified pulse peak as returned by FPGA algorithm
		//uint32_t pulse_time;      ///< identified pulse time as returned by FPGA algorithm

		void Summarize(JObjectSummary& summary) const override {
			summary.add(plane, NAME_OF(plane), "%d");
			summary.add(strip, NAME_OF(strip), "%d");
			//summary.add(pulse_peak, NAME_OF(pulse_peak), "%d");
			//summary.add(pulse_time, NAME_OF(pulse_time), "%d");
		}
};

#endif // _DGEMDigiWindowRawData_

