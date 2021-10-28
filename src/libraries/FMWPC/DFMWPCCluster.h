// $Id$
//
//    File: DFMWPCCluster.h
// Created: Wed Oct 27 10:23:28 EDT 2021
// Creator: aaustreg (on Linux ifarm1802.jlab.org 3.10.0-1062.4.1.el7.x86_64 x86_64)
//

#ifndef _DFMWPCCluster_
#define _DFMWPCCluster_

#include <JANA/JObject.h>
#include <JANA/JFactory.h>

#include "DFMWPCHit.h"

class DFMWPCCluster:public jana::JObject{
	public:
		JOBJECT_PUBLIC(DFMWPCCluster);
		
		vector<const DFMWPCHit*> members; ///< DFMWPCHits that make up this cluster
		int layer; ///< #1-6 FMWPC layer
		float q; ///< total charge in the cluster
		float u; ///< center of gravity of cluster in wire coordinates
		
		// This method is used primarily for pretty printing
		// the second argument to AddString is printf style format
		void toStrings(vector<pair<string,string> > &items)const{
			AddString(items, "layer", "%d", layer);
			AddString(items, "q", "%10.2f", q);
			AddString(items, "u", "%3.4f", u);
		}
		
};

#endif // _DFMWPCCluster_

