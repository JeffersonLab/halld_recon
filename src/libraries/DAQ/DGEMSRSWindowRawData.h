// $Id:$
// $HeadURL:$
//
//    File: DGEMSRSWindowRawData.h
//

#ifndef _DGEMSRSWindowRawData_
#define _DGEMSRSWindowRawData_

#include <DAQ/DDAQAddress.h>

class DGEMSRSWindowRawData:public DDAQAddress{
	
	/// Holds GEM window raw data
	
	public:
		JOBJECT_PUBLIC(DGEMSRSWindowRawData);
		
                DGEMSRSWindowRawData(uint32_t rocid=0, uint32_t slot=0, uint32_t channel=0, uint32_t itrigger=0, uint32_t apv_id=0, uint32_t channel_apv=0):
                DDAQAddress(rocid, slot, channel, itrigger),apv_id(apv_id),channel_apv(channel_apv){}
		
		uint32_t apv_id;           ///< APV Identifier number on the FEC card (0 to 15)
		//uint32_t fec_id;           ///< FEC Identifier number (always 0?)
		uint32_t channel_apv;      ///< APV physical channels are 0 to 127 
		vector<uint16_t> samples;  ///< ADC samples 
		
		// This method is used primarily for pretty printing
		// the second argument to AddString is printf style format
		void toStrings(vector<pair<string,string> > &items)const{
			DDAQAddress::toStrings(items);
			AddString(items, "apv_id",       "%d", apv_id);
			AddString(items, "channel_apv",  "%d", channel_apv);
			AddString(items, "samples",      "%d", samples.size());
		}
};

#endif // _DGEMSRSWindowRawData_

