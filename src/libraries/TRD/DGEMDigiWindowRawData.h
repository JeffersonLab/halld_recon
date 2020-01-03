// $Id$
//
//    File: DGEMDigiWindowRawData.h
//

#ifndef _DGEMDigiWindowRawData_
#define _DGEMDigiWindowRawData_

#include <JANA/JObject.h>
#include <JANA/JFactory.h>

class DGEMDigiWindowRawData:public jana::JObject{
	public:
		JOBJECT_PUBLIC(DGEMDigiWindowRawData);
		
		uint32_t plane;
		uint32_t strip;
		// other information retrieved via object in DAQ library

		//uint32_t pulse_peak;      ///< identified pulse peak as returned by FPGA algorithm
		//uint32_t pulse_time;      ///< identified pulse time as returned by FPGA algorithm
		
		// This method is used primarily for pretty printing
		// the second argument to AddString is printf style format
		void toStrings(vector<pair<string,string> > &items)const{
			AddString(items, "plane", "%d", plane);
			AddString(items, "strip", "%d", strip);
			//AddString(items, "pulse_peak", "%d", pulse_peak);
			//AddString(items, "pulse_time", "%d", pulse_time);
		}		
};

#endif // _DGEMDigiWindowRawData_

