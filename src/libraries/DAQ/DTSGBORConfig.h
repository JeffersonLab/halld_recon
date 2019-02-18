// $Id$
//
//    File: DTSGBORConfig.h
// Created: Tue Nov 20 07:47:02 EST 2018
// Creator: davidl (on gluon114)
//

#ifndef _DTSGBORConfig_
#define _DTSGBORConfig_

#include <JANA/JObject.h>

#include<string>

// This class holds data inserted via the CDAQ system
// into the BOR. CDAQ creates the BOR internally rather
// than via the hdBOR program inserting the event externally
// as is done with CODA. As such, it may add data such as
// this that is not reflected in the DAQ/bor_roc.h file 
// like for the digitization modules.



class DTSGBORConfig:public jana::JObject{
	public:
		JOBJECT_PUBLIC(DTSGBORConfig);

		DTSGBORConfig(){}
		virtual ~DTSGBORConfig(){}
		
		uint32_t rocid;      // always 81. Needed for SortByModule in LinAssociations.h
		uint32_t slot;       // always 1
		uint32_t run_number;
		uint32_t unix_time;
		vector<uint32_t> misc_words; // extra words that may be added later
		
		// This method is used primarily for pretty printing
		// the second argument to AddString is printf style format
		void toStrings(vector<pair<string,string> > &items)const{
		
			AddString(items, "run_number" , "%d"  , run_number);
			AddString(items, "unix_time"  , "%d"  , unix_time);
			AddString(items, "Nwords"     , "%d"  , misc_words.size());
		}

};

#endif // _DTSGBORConfig_

