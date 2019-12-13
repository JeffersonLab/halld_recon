/*
 *  File: DCCALHit.h
 *
 * Created on 11/25/18 by A.S. 
 */


#ifndef _DCCALHit_
#define _DCCALHit_

#include <JANA/jerror.h>
#include <JANA/JFactory.h>
using namespace jana;

class DCCALHit:public jana::JObject{
	public:
  
               JOBJECT_PUBLIC(DCCALHit);
		
		DCCALHit(){}
		
		int row;
		int column;
		float x;
		float y;
		float E;
		float t;
		float intOverPeak;		
		

		void toStrings(vector<pair<string,string> > &items)const{
		  AddString(items, "row", "%4d", row);
			AddString(items, "column", "%4d", column);
			AddString(items, "x(cm)", "%3.1f", x);
			AddString(items, "y(cm)", "%3.1f", y);
			AddString(items, "E(MeV)", "%2.3f", E);
			AddString(items, "t(ns)", "%2.3f", t);
			AddString(items, "integral over peak",  "%2.3f", intOverPeak);
		}
};

#endif // _DCCALHit_

