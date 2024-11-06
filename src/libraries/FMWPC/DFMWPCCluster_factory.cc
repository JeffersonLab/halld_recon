// $Id$
//
//    File: DFMWPCCluster_factory.cc
// Created: Wed Oct 27 10:23:29 EDT 2021
// Creator: aaustreg (on Linux ifarm1802.jlab.org 3.10.0-1062.4.1.el7.x86_64 x86_64)
//


#include <iostream>
#include <iomanip>
#include <cmath>
using namespace std;

#include "DFMWPCCluster_factory.h"
#include <DANA/DEvent.h>


bool DFMWPCHit_cmp(const DFMWPCHit* a, const DFMWPCHit* b) {
  if (a->layer==b->layer){
    return a->t < b->t;
  }
  return a->layer < b->layer;
}

///
/// DFMWPCHit_wire_cmp():
///	a non-member function passed to std::sort() for sorting DFMWPCHit pointers by
/// their wire numbers. Typically only used for a single layer of hits.
///
bool DFMWPCHit_wire_cmp(const DFMWPCHit* a, const DFMWPCHit* b) {
	if(a->wire != b->wire) return a->wire < b->wire;
	if(a->t       != b->t      ) return a->t < b->t;
	return a->q < b->q;
}


//------------------
// init
//------------------
void DFMWPCCluster_factory::Init()
{
  auto app = GetApplication();
  // Future calibration constants
  TIME_SLICE=10000.0; //ns
  gPARMS->SetDefaultParameter("FMWPC:CLUSTER_TIME_SLICE",TIME_SLICE);
  
  return NOERROR;
}

//------------------
// brun
//------------------
void DFMWPCCluster_factory::BeginRun(const std::shared_ptr<const JEvent> &event)
{

  // Get pointer to DGeometry object
  dgeom  = DEvent::GetDGeometry(event);

  // Get the FMWPC z,x and y positions from the HDDM geometry
  // if they are not in there, use hard-coded values
  if (!dgeom->GetFMWPCZ_vec(zvec))
    zvec = {935.366,948.536,961.706,976.226,993.246,1016.866};
  if (!dgeom->GetFMWPCXY_vec(xvec, yvec)){
    xvec = {0.0,0.0,0.0,0.0,0.0,0.0};
    yvec = {0.0,0.0,0.0,0.0,0.0,0.0};
  }

  // Get the FMWPC wire spacing in cm (should be 1.016)
  dgeom->GetFMWPCWireSpacing( FMWPC_WIRE_SPACING );

  // Get the FMWPC wire orientation (should be vertical, horizontal, ...)
  dgeom->GetFMWPCWireOrientation( fmwpc_wire_orientation );

  return NOERROR;
}

//------------------
// evnt
//------------------
void DFMWPCCluster_factory::Process(const std::shared_ptr<const JEvent> &event)
{

  vector<const DFMWPCHit*> allHits;
  vector<vector<const DFMWPCHit*> >thisLayer;

  try {
    event->Get(allHits);
    
    if (allHits.size()>0) {
      // Sort hits by layer number and by time
      sort(allHits.begin(),allHits.end(),DFMWPCHit_cmp);

      // Layer by layer, create clusters of hits.
      thisLayer.clear();
      vector<const DFMWPCHit*>::iterator i = allHits.begin();
      for (int iLayer=1;iLayer<7;iLayer++){
	if (i==allHits.end()) break;
	  
	vector<const DFMWPCHit*> hits;	
	float old_time=(*i)->t;
	while((i!=allHits.end()) && ((*i)->layer == iLayer)){ 
	  // Look for hits falling within a time slice
	  if (fabs((*i)->t-old_time)>TIME_SLICE){
	    // Sort hits by wire number
	    sort(hits.begin(),hits.end(),DFMWPCHit_wire_cmp);
	    // put into the vector
	    thisLayer.push_back(hits);
	    hits.clear();
	    old_time=(*i)->t;
	  }
	  hits.push_back(*i);
	    
	  i++;
	}
	// Sort hits by wire number
	sort(hits.begin(),hits.end(),DFMWPCHit_wire_cmp);
	// add the last vector of hits
	thisLayer.push_back(hits);
	  
	// Create clusters from these lists of hits
	for (unsigned int k=0;k<thisLayer.size();k++) pique(thisLayer[k]);
	  
	// Clear the hits and layer vectors for the next ones
	thisLayer.clear();	
	hits.clear();
      }
    }
  }
  catch (JException d) {
    cout << d << endl;
  }	
  catch (...) {
    cerr << "exception caught in DFMWPCCluster_factory" << endl;
  }
}

//-----------------------------
// pique
//-----------------------------
void DFMWPCCluster_factory::pique(vector<const DFMWPCHit*>& H)
{
  /// Find clusters within wire plane.
  ///
  /// Upon entry, the vector "H" should already be sorted
  /// by wire number and should only contains hits from
  /// the same layer that are in time with each other.
  /// This will form clusters from all contiguous wires.
  
  // Loop over hits
  for(uint32_t istart=0; istart<H.size(); istart++){
    const DFMWPCHit *first_hit = H[istart];
    
    // Find end of contiguous section
    uint32_t iend=istart+1;
    for(; iend<H.size(); iend++){
      if(iend>=H.size()) break;
      if( (H[iend]->wire - H[iend-1]->wire) > 1 ) break;
    }
    
    // istart should now point to beginning of cluster 
    // and iend to one past end of cluster
    DFMWPCCluster* newCluster = new DFMWPCCluster();
    newCluster->q = 0.0;
    newCluster->u = 0.0;
    newCluster->layer = first_hit->layer;
    newCluster->first_wire = first_hit->wire;
    newCluster->Nhits = 0;
    newCluster->t = 0.0;
    for(uint32_t i=istart; i<iend; i++){
      newCluster->q += H[i]->q;
      newCluster->u += H[i]->wire * H[i]->q; // weigh position with charge
      newCluster->t += H[i]->t * H[i]->q; // weigh time with charge
      newCluster->members.push_back(H[i]);
      newCluster->last_wire = H[i]->wire;
      newCluster->Nhits++;
    }
    if (newCluster->q != 0){
      newCluster->u /= newCluster->q; // normalize to total charge
      newCluster->t /= newCluster->q; // normalize to total charge
    }

    // global coordinate system
    // set to -777 for not measured coordinate
    auto orientation = fmwpc_wire_orientation[newCluster->layer-1];
    newCluster->orientation=orientation;
    newCluster->xoffset=xvec[newCluster->layer-1];
    newCluster->yoffset=yvec[newCluster->layer-1];
    double x = (orientation==DGeometry::kFMWPC_WIRE_ORIENTATION_VERTICAL  ) ? xvec[newCluster->layer-1]+(newCluster->u-72.5)*FMWPC_WIRE_SPACING : -777 ;
    double y = (orientation==DGeometry::kFMWPC_WIRE_ORIENTATION_HORIZONTAL) ? yvec[newCluster->layer-1]+(newCluster->u-72.5)*FMWPC_WIRE_SPACING : -777 ;
    double z = zvec[newCluster->layer-1];
    DVector3 pos(x,y,z);
    newCluster->pos = pos;

     _data.push_back(newCluster);
		
    istart = iend-1;
  }
}

//------------------
// EndRun
//------------------
void DFMWPCCluster_factory::EndRun()
{
}

//------------------
// Finish
//------------------
void DFMWPCCluster_factory::Finish()
{
}

