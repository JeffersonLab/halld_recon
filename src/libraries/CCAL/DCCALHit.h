/*
 *  File: DCCALHit.h
 *
 * Created on 11/25/18 by A.S. 
 */


#ifndef _DCCALHit_
#define _DCCALHit_

#include <JANA/JObject.h>
#include <DANA/DObjectID.h>

class DCCALHit: public JObject {
	public:
		
		JOBJECT_PUBLIC(DCCALHit);
		
		DCCALHit(){}

		oid_t id = reinterpret_cast<oid_t>(this);
		int row;
		int column;
		float x;
		float y;
		float E;
		float t;
		float intOverPeak;		
		

		void Summarize(JObjectSummary& summary) const override {
			summary.add(row, "row", "%4d");
			summary.add(column, "column", "%4d");
			summary.add(x, "x(cm)", "%3.1f");
			summary.add(y, "y(cm)", "%3.1f");
			summary.add(E, "E(MeV)", "%2.3f");
			summary.add(t, "t(ns)", "%2.3f");
			summary.add(intOverPeak, "integral over peak", "%2.3f");
		}
};

#endif // _DCCALHit_

