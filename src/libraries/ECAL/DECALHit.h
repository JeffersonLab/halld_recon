/*
 *  File: DECALHit.h
 *
 *  Created on 01/16/2024 by A.S. 
 */


#ifndef _DECALHit_
#define _DECALHit_

#include <JANA/JFactoryT.h>

class DECALHit:public JObject{
	public:
  
               JOBJECT_PUBLIC(DECALHit);
		
		DECALHit(){}
		
		int row;
		int column;
		float x;
		float y;
		float E;
		float t;
		float intOverPeak;		
		

		void Summarize(JObjectSummary& summary) const{
		  	summary.add(row, "row", "%4d");
			summary.add(column, "column", "%4d");
			summary.add(x, "x(cm)", "%3.1f");
			summary.add(y, "y(cm)", "%3.1f");
			summary.add(E, "E(MeV)", "%2.3f");
			summary.add(t, "t(ns)", "%2.3f");
			summary.add(intOverPeak, "integral over peak",  "%2.3f");
		}
};

#endif // _DECALHit_

