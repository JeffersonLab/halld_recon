
#include "DGEMStripCluster_factory.h"

#include <JANA/JEvent.h>

bool DGEMHit_cmp(const DGEMHit* a, const DGEMHit* b) {
  if (a->plane==b->plane){
    return a->t < b->t;
  }
  return a->plane < b->plane;
}


///
/// DGEMHit_strip_cmp():
///	a non-member function passed to std::sort() for sorting DGEMHit pointers by
/// their strip (wire or strip) numbers. Typically only used for a single layer
/// of hits.
///
bool DGEMHit_strip_cmp(const DGEMHit* a, const DGEMHit* b) {
	if(a->strip != b->strip) return a->strip < b->strip;
	if(a->t       != b->t      ) return a->t < b->t;
	return a->pulse_height < b->pulse_height;
}

///
/// DGEMHit_time_cmp()
///    a non-member function passed to std::stable_sort() for sorting DGEMHit 
/// pointers in order of increasing time, provided that the time difference is
/// significant.
///

bool DGEMHit_time_cmp(const DGEMHit* a, const DGEMHit* b) {
  if (fabs(a->t-b->t)>HIT_TIME_DIFF_MIN && (a->t < b->t))
    return true;
  return false;
}

///
/// DGEMStripCluster_gPlane_cmp():
/// a non-member function passed to std::sort() for sorting DGEMStripCluster pointers
/// by their gPlane (plane number over all modules, 1-74) attributes.
///
bool DGEMStripCluster_gPlane_cmp(	const DGEMStripCluster* a, 
					const DGEMStripCluster* b) {
	return a->plane < b->plane;
}

///
/// Initialization
///
void DGEMStripCluster_factory::Init(){
  TIME_SLICE=50.0; //ns
  auto app = GetApplication();
  app->SetDefaultParameter("GEM:CLUSTER_TIME_SLICE",TIME_SLICE);
}

///
/// DGEMStripCluster_factory::evnt():
/// This (along with DGEMStripCluster_factory::pique()) 
/// is the place cathode hits are associated into cathode clusters.  
///
void DGEMStripCluster_factory::Process(const std::shared_ptr<const JEvent>& event) {
	vector<const DGEMHit*> allHits;
	vector<const DGEMHit*> planeHits[10];
	vector<vector<const DGEMHit*> >thisLayer;
	
	event->Get(allHits);
	
	if (allHits.size()>0) {
		// Sort hits by layer number and by time
		sort(allHits.begin(),allHits.end(),DGEMHit_cmp);
		
		// Sift through all hits and select out X and Y hits.
		for (vector<const DGEMHit*>::iterator i = allHits.begin(); i != allHits.end(); ++i){
			int stripPlane = (*i)->plane;
			planeHits[stripPlane].push_back(*i);
		} 

		// Plane by plane, create clusters of strips
		for(uint iplane = 0; iplane < 10; iplane++) {
			if (planeHits[iplane].size()>0){
				thisLayer.clear();
				vector<const DGEMHit*>::iterator i = planeHits[iplane].begin();	      
				vector<const DGEMHit*> hits;	
				float old_time=(*i)->t;
				while(i!=planeHits[iplane].end()){ 
					// Look for hits falling within a time slice
					if (fabs((*i)->t-old_time)>TIME_SLICE){
						// Sort hits by strip number
						sort(hits.begin(),hits.end(),DGEMHit_strip_cmp);
						// put into the vector
						thisLayer.push_back(hits);
						hits.clear();
						old_time=(*i)->t;
					}
					hits.push_back(*i);
					
					i++;
				}
				
				// Sort hits by strip number
				sort(hits.begin(),hits.end(),DGEMHit_strip_cmp);
				// add the last vector of hits
				thisLayer.push_back(hits);
				
				// Create clusters from these lists of hits
				for (unsigned int k=0;k<thisLayer.size();k++) pique(thisLayer[k]);
				
				// Clear the hits and layer vectors for the next ones
				thisLayer.clear();	
				hits.clear();
			}
		}

		// Ensure that the data are still in order of planes.
		std::sort(mData.begin(), mData.end(), DGEMStripCluster_gPlane_cmp);
	}
}

//-----------------------------
// pique
//-----------------------------
void DGEMStripCluster_factory::pique(vector<const DGEMHit*>& H)
{
	/// Find clusters within GEM plane.
	///
	/// Upon entry, the vector "H" should already be sorted
	/// by strip number and should only contains hits from
	/// the same plane that are in time with each other.
	/// This will form clusters from all contiguous strips.

	// Loop over hits
	for(uint32_t istart=0; istart<H.size(); istart++){
		const DGEMHit *first_hit = H[istart];
		
		// Find end of contiguous section
		uint32_t iend=istart+1;
		for(; iend<H.size(); iend++){
			if(iend>=H.size()) break;
			if( (H[iend]->strip - H[iend-1]->strip) > 1 ) break;
		}
		if( (iend-istart)<2 ) continue; // don't allow single strip clusters
		
		// istart should now point to beginning of cluster 
		// and iend to one past end of cluster
		DGEMStripCluster* newCluster = new DGEMStripCluster();
		newCluster->q_tot   = 0.0;
		newCluster->plane   = first_hit->plane;
		double max_strip = 0;
		for(uint32_t i=istart; i<iend; i++){
			newCluster->q_tot += H[i]->pulse_height;
			newCluster->members.push_back(H[i]);
			if(H[i]->pulse_height < max_strip)
				max_strip = H[i]->pulse_height;
		}

		// apply seed strip threshold
		if(max_strip < 800)
			Insert(newCluster);
		
		istart = iend-1;
	}
}

