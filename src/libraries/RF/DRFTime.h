// $Id$
//
//    File: DRFTime.h
// Created: Tue Aug  9 14:29:24 EST 2011
// Creator: pmatt (on Linux ifarml6 2.6.18-128.el5 x86_64)
//

#ifndef _DRFTime_
#define _DRFTime_

#include <vector>
#include <utility>
#include <string>

#include "JANA/JObject.h"

using namespace std;

class DRFTime : public JObject
{
	public:
		JOBJECT_PUBLIC(DRFTime);

		double dTime; //This time is defined at the center of the target. 
		double dTimeVariance;

		void Summarize(JObjectSummary& summary) const override {
			summary.add(dTime, "t", "%3.5f");
			summary.add(dTimeVariance, "var_t", "%3.2f");
		}
};

#endif // _DRFTime_

