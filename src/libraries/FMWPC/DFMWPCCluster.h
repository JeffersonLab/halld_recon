// $Id$
//
//    File: DFMWPCCluster.h
// Created: Wed Oct 27 10:23:28 EDT 2021
// Creator: aaustreg (on Linux ifarm1802.jlab.org 3.10.0-1062.4.1.el7.x86_64 x86_64)
//

#ifndef _DFMWPCCluster_
#define _DFMWPCCluster_

#include <JANA/JObject.h>

#include "DFMWPCHit.h"
#include "DVector3.h"

class DFMWPCCluster:public JObject{
	public:
		JOBJECT_PUBLIC(DFMWPCCluster);
		
		vector<const DFMWPCHit*> members; ///< DFMWPCHits that make up this cluster
		int layer; ///< #1-6 FMWPC layer
		int orientation; ///< Vertical/Horizontal
		float xoffset,yoffset; ///< x and y offsets of wires
		float q; ///< total charge in the cluster
		float u; ///< center of gravity of cluster in wire coordinates
		int first_wire; ///< first wire in cluster 1-144
		int last_wire; ///< last wire in cluster 1-144
		int Nhits; ///< Number of wire hits in cluster
		DVector3 pos; ///< position (x,y,z) in global coordinates
		float t;     // time in ns
		
		// This method is used primarily for pretty printing
		// the second argument to AddString is printf style format
		void Summarize(JObjectSummary& summary)const{
			summary.add(layer, "layer", "%d");
			summary.add(orientation, "orientation", "%d");
			summary.add(q, "q", "%10.2f");
			summary.add(u, "u", "%3.4f");
			summary.add(first_wire, "first_wire", "%3d");
			summary.add(last_wire, "last_wire", "%3d");
			summary.add(Nhits, "Nhits", "%d");
			summary.add(pos.x(), "x", "%1.3f");
			summary.add(pos.y(), "y", "%1.3f");
			summary.add(pos.z(), "z", "%1.3f");
			summary.add(t, "t", "%1.3f");
		}
		
};

#endif // _DFMWPCCluster_

