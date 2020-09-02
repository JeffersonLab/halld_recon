//********************************************************
// DPadGEMPoint_factory.cc 
//********************************************************

#include "DPadGEMPoint_factory.h"

///
/// DPadGEMPoint_cmp(): 
/// non-member function passed to std::sort() to sort DTRDHit pointers 
/// for the wires by their plane attributes.
///
bool DPadGEMPoint_cmp(const DGEMPoint* a, const DGEMPoint *b){
  if (a->detector == b->detector) return a->time<b->time;
  
  return a->detector<b->detector;
}

//------------------
// init
//------------------
jerror_t DPadGEMPoint_factory::init(void)
{

  return NOERROR;
}


//------------------
// brun
//------------------
jerror_t DPadGEMPoint_factory::brun(JEventLoop *loop, int32_t runnumber)
{
  // Get pointer to DGeometry object
  DApplication* dapp=dynamic_cast<DApplication*>(eventLoop->GetJApplication());
  const DGeometry *dgeom  = dapp->GetDGeometry(runnumber);
  
  // Get GEM geometry from xml (CCDB or private HDDS)
  dgeom->GetTRDZ(dTRDz);
  
  // Get TRD geometry (needs to come from geometry file or CCDB?)
  X0 = 0.0;
  Y0 = 0.0;
  pitch = 1.0;  // 10 mm

  return NOERROR;
}

jerror_t DPadGEMPoint_factory::erun(void){
 
  return NOERROR;
}
///
/// DPadGEMPoint_factory::evnt():
/// this is the place that produces points from GEM pads
///
jerror_t DPadGEMPoint_factory::evnt(JEventLoop* eventLoop, uint64_t eventNo) {

	// Get all hits (wires and GEM strips)
	vector<const DTRDHit*> hits;
	eventLoop->Get(hits);
	if (hits.size()==0) return NOERROR;
	
	////////////////////
	// Pad GEM points //
	////////////////////
	for(uint i=0; i<hits.size(); i++){
	
		if (hits[i]->plane != 3) continue;
		
		// save new point
		DGEMPoint* newPoint = new DGEMPoint;     
		newPoint->x = X0 + hits[i]->strip%10 * pitch;
		newPoint->y = Y0 + hits[i]->strip/10 * pitch;;
		newPoint->t_x = hits[i]->t;
		newPoint->t_y = hits[i]->t;
		newPoint->time = hits[i]->t;
		newPoint->dE_amp = hits[i]->pulse_height;
		newPoint->detector = 1;
		newPoint->status = 1;
		newPoint->itrack = 0;
		newPoint->z = dTRDz[1];

		newPoint->AddAssociatedObject(hits[i]);
		
		_data.push_back(newPoint);
	}

	// Make sure the data are both time- and z-ordered
	std::sort(_data.begin(),_data.end(),DPadGEMPoint_cmp);
	
	return NOERROR;
}

