//********************************************************
// DTRDPoint_Hit_factory.cc - modeled after DFDCPseudo_factory 
//********************************************************

#include "DTRDPoint_Hit_factory.h"

#include <JANA/JEvent.h>
#include "DANA/DGeometryManager.h"

///
/// DTRDPoint_Hit_cmp(): 
/// non-member function passed to std::sort() to sort DTRDHit pointers 
/// for the wires by their plane attributes.
///
bool DTRDPoint_Hit_cmp(const DTRDPoint_Hit* a, const DTRDPoint_Hit *b){
  return a->time<b->time;
}

//------------------
// init
//------------------
//jerror_t DTRDPoint_Hit_factory::init(void)
//{
//  return NOERROR;
//}
void DTRDPoint_Hit_factory::Init()
{
		auto app = GetApplication();
		
		TIME_DIFF_MAX = 50.;
		
		app->SetDefaultParameter("TRDPoint:XY_TIME_DIFF",TIME_DIFF_MAX);

}

//------------------
// BeginRun
//------------------
void DTRDPoint_Hit_factory::BeginRun(const std::shared_ptr<const JEvent>& event)
{
	auto runnumber = event->GetRunNumber();
	auto app = event->GetJApplication();
	auto geo_manager = app->GetService<DGeometryManager>();
	auto dgeom = geo_manager->GetDGeometry(runnumber);
	
  	// Get GEM geometry from xml (CCDB or private HDDS)
  	dgeom->GetTRDZ(dTRDz);
  
  return;
}

void DTRDPoint_Hit_factory::EndRun(){
}

/// this is the place that produces points from GEMTRD strip hits
///
void DTRDPoint_Hit_factory::Process(const std::shared_ptr<const JEvent>& event)
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
	
	// Primitive noise suppression for cosmic
	// match hits in X and Y planes
	for(uint i=0; i<hitX.size(); i++){
		if (hitX.size()>25) continue;
		for(uint j=0; j<hitY.size(); j++){
			if (hitY.size()>25) continue;
			
			// calculate hit time and energy
			double t_diff = hitX[i]->t - hitY[j]->t;
			double dE = ( hitX[i]->q + hitY[j]->q ) / 2.;
			double dE_diff = ( hitX[i]->q - hitY[j]->q ) / ( hitX[i]->q + hitY[j]->q );
			double dE_diff_max = 0.3;
			
			// some requirements for a good point
			if(fabs(t_diff) < TIME_DIFF_MAX) {//&& fabs(dE_diff) < dE_diff_max) {
			//if(fabs(t_diff) < TIME_DIFF_MAX && (hitX[i]->q < (4000.*dE_diff_max)+hitY[j]->q*(4000.-dE_diff_max)) && (hitY[j]->q < (dE_diff_max*4000.)+hitX[i]->q*(4000.-dE_diff_max))) {
				
				// save new point
				DTRDPoint_Hit* point = new DTRDPoint_Hit;
				point->x = hitX[i]->strip;
				point->y = hitY[j]->strip;
				point->t_x = hitX[i]->t;
				point->t_y = hitY[j]->t;
				point->time = (hitX[i]->t*hitX[i]->q + hitY[j]->t*hitY[j]->q) / (dE*2.);
				point->dE = dE;
				point->dE_x = hitX[i]->q;
				point->dE_y = hitY[j]->q;
				point->status = 1;
				//newPoint->itrack = 0;
				//newPoint->z = (stripClusX[i]->pos.z()*stripClusX[i]->q_tot + stripClusY[i]->pos.z()*stripClusY[j]->q_tot) / dE + dTRDz[0];

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
