//********************************************************
// DTRDPoint_factory.cc - modeled after DFDCPseudo_factory 
//********************************************************

#include "DTRDPoint_factory.h"

#include <JANA/JEvent.h>
#include "DANA/DGeometryManager.h"

///
/// DTRDPoint_cmp(): 
/// non-member function passed to std::sort() to sort DTRDHit pointers 
/// for the wires by their plane attributes.
///
bool DTRDPoint_cmp(const DTRDPoint* a, const DTRDPoint *b){
  return a->time<b->time;
}

//------------------
// Init
//------------------
void DTRDPoint_factory::Init()
{
	auto app = GetApplication();
  
  // Some parameters for defining matching
  TIME_DIFF_MAX = 1.;
//   DIST_DIFF_MAX = 10.;
  //DE_DIFF_MAX = 1.0;

  app->SetDefaultParameter("TRDPOINT:TIME_DIFF_MAX",TIME_DIFF_MAX);
//   app->SetDefaultParameter("TRDPOINT:DIST_DIFF_MAX",DIST_DIFF_MAX);

  DRIFT_VELOCITY=0.0033; // cm/ns
}


//------------------
// BeginRun
//------------------
void DTRDPoint_factory::BeginRun(const std::shared_ptr<const JEvent>& event)
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

void DTRDPoint_factory::EndRun(){
}
///
/// DTRDPoint_factory::evnt():
/// this is the place that produces points from wire hits and GEM strips
///
void DTRDPoint_factory::Process(const std::shared_ptr<const JEvent>& event) 
{

	// Get strip clusters
	vector<const DTRDStripCluster*> stripClus;
	event->Get(stripClus);

//     cout << "DTRDPoint_factory::Process() ..." << endl;
//     cout << "  num input clusters = " << stripClus.size() << endl;

	// Sift through clusters and select out X and Y plane wires
	vector<const DTRDStripCluster*> stripClusX,stripClusY;
	for (unsigned int i=0; i < stripClus.size(); i++) {
		// TODO: make some enums so it's more clear what plane 1 and 2 are...
		if (stripClus[i]->plane == 1)
			stripClusX.push_back(stripClus[i]);
		else if (stripClus[i]->plane == 2)
			stripClusY.push_back(stripClus[i]);
	}

	
	// match clusters in X and Y planes
	for(uint i=0; i<stripClusX.size(); i++){
		for(uint j=0; j<stripClusY.size(); j++){
	
			// calculate strip cluster time and position
			double t_diff = stripClusX[i]->t_avg - stripClusY[j]->t_avg;
			double dE = stripClusX[i]->q_tot + stripClusY[j]->q_tot;

			// some requirements for a good point
			if(fabs(t_diff) < TIME_DIFF_MAX) {   // && fabs(dE_amp_diff) <gem_dE_max) {
		
				// save new point
				DTRDPoint* newPoint = new DTRDPoint;     
				newPoint->x = dTRDx+stripClusX[i]->pos.x();
				newPoint->y = dTRDy+stripClusY[j]->pos.y();
				newPoint->t_x = stripClusX[i]->t_avg;
				newPoint->t_y = stripClusY[j]->t_avg;
				newPoint->time = (stripClusX[i]->t_avg*stripClusX[i]->q_tot + stripClusY[j]->t_avg*stripClusY[j]->q_tot) / dE;
				newPoint->dE = dE;
				newPoint->status = 1;
				//newPoint->itrack = 0;
				//newPoint->z = (stripClusX[i]->pos.z()*stripClusX[i]->q_tot + stripClusY[j]->pos.z()*stripClusY[j]->q_tot) / dE + dTRDz[0];
				//newPoint->z = dTRDz+(stripClusX[i]->pos.z()*stripClusX[i]->q_tot + stripClusY[j]->pos.z()*stripClusY[j]->q_tot) / dE;  // FOR TESTING
				newPoint->z=dTRDz-DRIFT_VELOCITY*newPoint->time;

				newPoint->AddAssociatedObject(stripClusX[i]);
				newPoint->AddAssociatedObject(stripClusY[j]);

				Insert(newPoint);
			}
		}
	}

	// Make sure the data are both time- and z-ordered
	std::sort(mData.begin(),mData.end(),DTRDPoint_cmp);
}

// double DTRDPoint_factory::calcClusterTime(const DTRDStripCluster *clus)
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
// double DTRDPoint_factory::calcClusterPosition(const DTRDStripCluster *clus)
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
