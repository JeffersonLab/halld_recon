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

using namespace jana;

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
jerror_t DFMWPCCluster_factory::init(void)
{
  // Future calibration constants
  TIME_SLICE=10000.0; //ns
  FMWPC_WIRE_SPACING = 1.016; // distance between wires of FMWPC in cm
  gPARMS->SetDefaultParameter("FMWPC:CLUSTER_TIME_SLICE",TIME_SLICE);
  
  return NOERROR;
}

//------------------
// brun
//------------------
jerror_t DFMWPCCluster_factory::brun(jana::JEventLoop *eventLoop, int32_t runnumber)
{

  // Get pointer to DGeometry object
  DApplication* dapp=dynamic_cast<DApplication*>(eventLoop->GetJApplication());
  dgeom  = dapp->GetDGeometry(runnumber);

  // Get the FMWPC z,x and y positions from the HDDM geometry
  // if they are not in there, use hard-coded values
  if (!dgeom->GetFMWPCZ_vec(zvec))
    zvec = {935.366,948.536,961.706,976.226,993.246,1016.866};
  if (!dgeom->GetFMWPCXY_vec(xvec, yvec)){
    xvec = {0.0,0.0,0.0,0.0,0.0,0.0};
    yvec = {0.0,0.0,0.0,0.0,0.0,0.0};
  }
  rot = {90.0,0.0,90.0,0.0,90.0,0.0};

  return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t DFMWPCCluster_factory::evnt(JEventLoop *loop, uint64_t eventnumber)
{

  vector<const DFMWPCHit*> allHits;
  vector<vector<const DFMWPCHit*> >thisLayer;

  try {
    eventLoop->Get(allHits);
    
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

  return NOERROR;
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
    for(uint32_t i=istart; i<iend; i++){
      newCluster->q += H[i]->q;
      newCluster->u += H[i]->wire * H[i]->q; // weigh position with charge
      newCluster->members.push_back(H[i]);
      newCluster->last_wire = H[i]->wire;
      newCluster->Nhits++;
    }
    if (newCluster->q != 0) newCluster->u /= newCluster->q; // normalize to total charge

    // global coordinate system
    // set to -777 for not measured coordinate
    double x = (rot[newCluster->layer-1]==90.0) ? xvec[newCluster->layer-1]+(newCluster->u-72.5)*FMWPC_WIRE_SPACING : -777 ;
    double y = (rot[newCluster->layer-1]==0.0) ? yvec[newCluster->layer-1]+(newCluster->u-72.5)*FMWPC_WIRE_SPACING : -777 ;
    double z = zvec[newCluster->layer-1];
    DVector3 pos(x,y,z);
    newCluster->pos = pos;

    _data.push_back(newCluster);
		
    istart = iend-1;
  }
}

//------------------
// erun
//------------------
jerror_t DFMWPCCluster_factory::erun(void)
{
	return NOERROR;
}

//------------------
// fini
//------------------
jerror_t DFMWPCCluster_factory::fini(void)
{
	return NOERROR;
}

