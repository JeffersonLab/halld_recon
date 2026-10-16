//************************************************
// DFDCHit.h: A class defining a general FDC hit
// Author: Craig Bookwalter, David Lawrence
// Date:	March 2006
//************************************************

#ifndef DFDCHIT_H
#define DFDCHIT_H

#include <sstream>
using namespace std;

#include <JANA/JObject.h>

///
/// class DFDCHit: definition for a basic FDC hit data type.
///
class DFDCHit : public JObject{
	public:
		JOBJECT_PUBLIC(DFDCHit);		
		int layer;			// 1(V), 2(X), or 3(U)
		int module;			// 1 through 8, 1 module = 3 detection layers
		int element;			// wire or strip number
	    int plane;				// for cathodes only: u(3) or v(1) plane, u@+15,v@-15  
	    int gPlane;				// 1 through 72
	    int gLayer;				// 1 through 24
	    float q;				// charge deposited
	    float pulse_height;                 // amplitude of signal
       float pulse_height_raw; //amplitude of signal without gain correction
	    float t;				// drift time
	    float r;				// perpendicular distance from 
	    					// center of chamber to wire/strip center
	    float d;                            // DOCA distance of closest approach (only for MC data on wires)
	    // Enum to take into account split cathode strips near center in 
	    // addition to wires-versus-cathodes
	    enum fdc_hit_type{
	      AnodeWire,
	      FullCathodeStrip,
	      HalfCathodeStripA,
	      HalfCathodeStripB
	    };
	    int type;		// value according to above enum
	    

	    int itrack;                         // track number causing the hit
	    int ptype;                          // particle type causing the hit

		void Summarize(JObjectSummary& summary) const override {
			summary.add(layer, "layer", "%d");
			summary.add(module, "module", "%d");
			summary.add(element, "w/s #", "%d");
			summary.add(plane==1 ? "V":(plane==2 ? "X":"U"), "plane", "%s");
			summary.add(gPlane, "gPlane", "%d");
			summary.add(gLayer, "gLayer", "%d");
			summary.add(q, "q", "%10.2f");
			summary.add(pulse_height, "pulse height", "%10.2f");
			summary.add(t, "t", "%10.2f");
			summary.add(r, "r", "%10.2f");
			summary.add(d, "d", "%f");
			summary.add(type, "type", "%d");
			summary.add(itrack, "itrack", "%d");
			summary.add(ptype, "ptype", "%d");
		}
	    	
};


#endif // DFDCHIT_H

