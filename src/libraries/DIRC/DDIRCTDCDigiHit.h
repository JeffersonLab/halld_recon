// $Id$
//
//    File: DDIRCTDCDigiHit.h
//

#ifndef _DDIRCTDCDigiHit_
#define _DDIRCTDCDigiHit_

#include <JANA/JObject.h>
#include <JANA/JFactory.h>

class DDIRCTDCDigiHit:public jana::JObject{
	public:
		JOBJECT_PUBLIC(DDIRCTDCDigiHit);
		
		int channel;
		uint32_t edge;         ///< 0=leading edge  1=trailing edge
		uint32_t time;         ///< 16 bit relative to beginning of defined readout window
				
		// This method is used primarily for pretty printing
		// the second argument to AddString is printf style format
		void toStrings(vector<pair<string,string> > &items)const{
			AddString(items, "channel", "%d", channel);
			AddString(items, "time", "%d", time);
			AddString(items, "edge", "%d", edge);
		}
		
};

#endif // _DDIRCTDCDigiHit_

