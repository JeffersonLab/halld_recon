// $Id$
// $HeadURL$
//
//    File: DBadHit.h
// Created: Fri Aug 10 12:02:49 EDT 2012
// Creator: davidl (on Darwin harriet.jlab.org 11.4.0 i386)
//

#ifndef _DBadHit_
#define _DBadHit_

#include <DAQ/DDAQAddress.h>

class DBadHit:public DDAQAddress{
	
	/// Holds single hit from any module, where a data format error was found
	
	public:
		JOBJECT_PUBLIC(DBadHit);
		
		DBadHit(uint32_t rocid=0, uint32_t slot=0, uint32_t channel=0, uint32_t itrigger=0) :  DDAQAddress(rocid, slot, channel, itrigger) { }
		

		
		void Summarize(JObjectSummary& summary) const override {
			DDAQAddress::Summarize(summary);
		}
};

#endif // _DBadHit_

