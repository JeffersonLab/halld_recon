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
#include "DVector3.h"

class DFMWPCCluster:public jana::JObject{
	public:
		JOBJECT_PUBLIC(DFMWPCCluster);
		
		vector<const DFMWPCHit*> members; ///< DFMWPCHits that make up this cluster
		int layer; ///< #1-6 FMWPC layer
		float q; ///< total charge in the cluster
		float u; ///< center of gravity of cluster in wire coordinates
		int first_wire; ///< first wire in cluster 1-144
		int last_wire; ///< last wire in cluster 1-144
		int Nhits; ///< Number of wire hits in cluster
		DVector3 pos; ///< position (x,y,z) in global coodinates
		
		// This method is used primarily for pretty printing
		// the second argument to AddString is printf style format
		void toStrings(vector<pair<string,string> > &items)const{
			AddString(items, "layer", "%d", layer);
			AddString(items, "q", "%10.2f", q);
			AddString(items, "u", "%3.4f", u);
			AddString(items, "first_wire", "%3d", first_wire);
			AddString(items, "last_wire", "%3d", last_wire);
			AddString(items, "Nhits", "%d", Nhits);
			AddString(items, "x", "%1.3f", pos.x());
			AddString(items, "y", "%1.3f", pos.y());
			AddString(items, "z", "%1.3f", pos.z());
		}
		
};

#endif // _DFMWPCCluster_

