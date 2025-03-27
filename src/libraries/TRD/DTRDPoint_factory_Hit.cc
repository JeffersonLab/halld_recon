//********************************************************
// DTRDPoint_factory_Hit.cc - modeled after DFDCPseudo_factory 
//********************************************************

#include "DTRDPoint_factory_Hit.h"
#include <JANA/JEvent.h>
#include "DANA/DGeometryManager.h"

///
/// DTRDPoint_cmp(): 
/// non-member function passed to std::sort() to sort DTRDHit pointers 
/// for the wires by their plane attributes.
///
bool DTRDPoint_Hit_cmp(const DTRDPoint* a, const DTRDPoint *b){
  return a->time<b->time;
}

//------------------
// init
//------------------
void DTRDPoint_factory_Hit::Init()
{
		auto app = GetApplication();
		
		TIME_DIFF_MAX = 48.;
		app->SetDefaultParameter("TRD:XY_TIME_DIFF",TIME_DIFF_MAX);
		
		dE_DIFF_MAX = 10000.;
		app->SetDefaultParameter("TRDPOINT:dE_DIFF_MAX",dE_DIFF_MAX,
			"Difference between Point_Hit charge in X and Y planes to be considered a coincidence (default: 10000.)");

}

/// this is the place that produces points from GEMTRD strip hits
///
void DTRDPoint_factory_Hit::Process(const std::shared_ptr<const JEvent>& event)
{
	// Get strip hits
	vector<const DTRDHit*> hit;
	event->Get(hit);

	// Sift through hits and select out X and Y plane strips
	vector<const DTRDHit*> hitX,hitY;
	for(uint i=0; i<hit.size(); i++){
		if (hit[i]->plane == 1)
			hitX.push_back(hit[i]);
		else if (hit[i]->plane == 2)
			hitY.push_back(hit[i]);
	}
	
	// match hits in X and Y planes
	for(uint i=0; i<hitX.size(); i++){
		for(uint j=0; j<hitY.size(); j++){
			
			// calculate hit time and energy
			double t_diff = hitX[i]->t - hitY[j]->t;
			double dE = ( hitX[i]->q + hitY[j]->q );
			double dE_high = (hitX[i]->q + dE_DIFF_MAX);
			double dE_low = (hitX[i]->q - dE_DIFF_MAX);
			
			// some requirements for a good point
			if(fabs(t_diff) < TIME_DIFF_MAX && (hitY[j]->q < dE_high) && (hitY[j]->q > dE_low )) {
				
				// save new point
				DTRDPoint* point = new DTRDPoint;
				point->x = hitX[i]->strip;
				point->y = hitY[j]->strip;
				point->t_x = hitX[i]->t;
				point->t_y = hitY[j]->t;
				point->time = (hitX[i]->t*hitX[i]->q + hitY[j]->t*hitY[j]->q) / (dE);
				point->dE = dE;
				point->dE_x = hitX[i]->q;
				point->dE_y = hitY[j]->q;
				point->status = 1;
				//newPoint->itrack = 0;
				point->z = (hitX[i]->t*hitX[i]->q + hitY[j]->t*hitY[j]->q) / dE; // + dTRDz[0]; //Not Useful Yet
				

				point->AddAssociatedObject(hitX[i]);
				point->AddAssociatedObject(hitY[j]);

				Insert(point);
			}
		}
	}
	
	// Make sure the data are time-ordered
	std::sort(mData.begin(),mData.end(),DTRDPoint_Hit_cmp);
}

// double DTRDPoint_Hit_factory::calcClusterTime(const DTRDStripCluster *clus)
// {
// 	double meanTime = 0;
// 	double pulseHeightSum = 0;
// 	for(uint i=0; i<clus->members.size(); i++) {
// 		const DTRDHit* hit = clus->members[i];
// 		meanTime += hit->t * hit->pulse_height;
// 		pulseHeightSum += hit->pulse_height;
// 	}
// 	
// 	return meanTime/pulseHeightSum;
// }
// 
// double DTRDPoint_Hit_factory::calcClusterPosition(const DTRDStripCluster *clus)
// {
// 	double meanPosition = 0;
// 	double pulseHeightSum = 0;
// 	for(uint i=0; i<clus->members.size(); i++) {
// 		const DTRDHit* hit = clus->members[i];
// 		pulseHeightSum += hit->pulse_height;
// 		if(hit->plane == 1 || hit->plane == 5) // Wire TRD strip
// 			meanPosition += (stripY0 + hit->strip * stripY_pitch)* hit->pulse_height;
// 		else if(hit->plane == 2 || hit->plane == 6) // GEM TRD X strip
// 			meanPosition += (gemX0 + hit->strip * gem_pitch)* hit->pulse_height;
// 		else if(hit->plane == 3 || hit->plane == 7) // GEM TRD Y strip
// 			meanPosition += (gemY0 + hit->strip * gem_pitch)* hit->pulse_height;
// 	}
// 	
// 	return meanPosition/pulseHeightSum;
// }
