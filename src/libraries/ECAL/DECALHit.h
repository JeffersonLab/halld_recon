/*
 *  File: DECALHit.h
 *
 *  Created on 01/16/2024 by A.S. 
 */


#ifndef _DECALHit_
#define _DECALHit_

#include <JANA/jerror.h>
#include <JANA/JFactory.h>
using namespace jana;

class DECALHit:public jana::JObject{
	public:
  
               JOBJECT_PUBLIC(DECALHit);
		
		DECALHit(){}
		
		int row;
		int column;
		float E;
		float t;
		float intOverPeak;		
		

		void toStrings(vector<pair<string,string> > &items)const{
		  AddString(items, "row", "%4d", row);
			AddString(items, "column", "%4d", column);
			AddString(items, "E(MeV)", "%2.3f", E);
			AddString(items, "t(ns)", "%2.3f", t);
			AddString(items, "integral over peak",  "%2.3f", intOverPeak);
		}
};

#endif // _DECALHit_

