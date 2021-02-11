// $Id$
//
//    File: DCGEMTruthHit.h
// Created: Tue Jun 16 07:07:54 EDT 2015
// Creator: davidl (on Darwin harriet.jlab.org 13.4.0 i386)
//

#ifndef _DCGEMTruthHit_
#define _DCGEMTruthHit_

#include <JANA/jerror.h>

class DCGEMTruthHit:public JObject{
	public:
		JOBJECT_PUBLIC(DCGEMTruthHit);

		int layer;   // 1-8
		//int hole;    // 1-144
		float dEdx;
		bool primary;
		int track;
		int itrack;
		int ptype;
		float x;
		float y;
		float z;
		float t;
		
		void toStrings(vector<pair<string,string> > &items)const{
		  AddString(items, "layer", "%d", layer);
		  AddString(items, "dEdx(MeV/cm)", "%1.3f", dEdx*1.0E3);
		  AddString(items, "primary", "%d", primary);
		  AddString(items, "track", "%d", track);
		  AddString(items, "itrack", "%d", itrack);
		  AddString(items, "ptype", "%d", ptype);
		  AddString(items, "t", "%3.2f", t);
		  AddString(items, "x", "%3.1f", x);
		  AddString(items, "y", "%3.1f", y);
		  AddString(items, "z", "%3.1f", z);
		}
				
};

#endif // _DCGEMTruthHit_

