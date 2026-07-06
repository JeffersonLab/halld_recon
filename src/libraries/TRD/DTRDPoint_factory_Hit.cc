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
		
		DRIFT_VELOCITY=0.0033; // cm/ns //Estimate - Will need changed later
		
		TIME_DIFF_MAX = 25.;
		app->SetDefaultParameter("TRD:XY_TIME_DIFF",TIME_DIFF_MAX);
		
		dE_DIFF_MAX = 10000.;
		app->SetDefaultParameter("TRDPOINT:dE_DIFF_MAX",dE_DIFF_MAX,
			"Difference between Point_Hit charge in X and Y planes to be considered a coincidence (default: 10000.)");

}

//------------------
// Begin
//------------------
void DTRDPoint_factory_Hit::BeginRun(const std::shared_ptr<const JEvent>& event)
{
  auto runnumber = event->GetRunNumber();
  auto app = event->GetJApplication();
  auto geo_manager = app->GetService<DGeometryManager>();
  auto dgeom = geo_manager->GetDGeometry(runnumber);

  // Get GEM geometry from xml (CCDB or private HDDS)
  dgeom->GetGEMTRDz(dTRDz);

  vector<double>xvec,yvec;
  if(dgeom->GetGEMTRDxy_vec(xvec,yvec)){
    dTRDx=xvec[0];
    dTRDy=yvec[0];
  }

  return;
}

///
/// DTRDPoint_factory_Hit::StripToPosition():
/// calculate position along plane
///
double DTRDPoint_factory_Hit::StripToPosition(int iplane, const DTRDHit *hit)
{
  // better to pull this from CCDB, also probably the pitch as well
  if(iplane == 0) {
    return -1.*STRIP_PITCH*double(NUM_X_STRIPS/2-hit->strip+0.5);
  }
  return STRIP_PITCH*double(NUM_Y_STRIPS/2-hit->strip+0.5);
}


/// this is the place that produces points from GEMTRD strip hits
///
void DTRDPoint_factory_Hit::Process(const std::shared_ptr<const JEvent>& event)
{
	// Get strip hits
	vector<const DTRDHit*> hit;
	event->Get(hit);
	if (hit.size()==0) return;
	
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
			
			// some requirements for a good point
			if(fabs(t_diff) < TIME_DIFF_MAX) {	
				// save new point
				DTRDPoint* point = new DTRDPoint;
				point->x = dTRDx+StripToPosition(0, hitX[i]);
				point->y = dTRDy+StripToPosition(1, hitY[j]);
				point->t_x = hitX[i]->t;
				point->t_y = hitY[j]->t;
				point->time = (hitX[i]->t*hitX[i]->q + hitY[j]->t*hitY[j]->q) / (dE);
				point->dE = dE;
				point->dE_x = hitX[i]->q;
				point->dE_y = hitY[j]->q;
				point->status = 1;
				//newPoint->itrack = 0;
				//point->z = dTRDz+(hitX[i]->t*hitX[i]->q + hitY[j]->t*hitY[j]->q) / dE;  //Not Useful Yet
				point->z=dTRDz-DRIFT_VELOCITY*point->time;
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
