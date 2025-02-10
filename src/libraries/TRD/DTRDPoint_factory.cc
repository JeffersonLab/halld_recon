//********************************************************
// DTRDPoint_factory.cc - modeled after DFDCPseudo_factory 
//********************************************************

#include "DTRDPoint_factory.h"

///
/// DTRDPoint_cmp(): 
/// non-member function passed to std::sort() to sort DTRDHit pointers 
/// for the wires by their plane attributes.
///
bool DTRDPoint_cmp(const DTRDPoint* a, const DTRDPoint *b){
  return a->time<b->time;
}

//------------------
// init
//------------------
jerror_t DTRDPoint_factory::init(void)
{

  return NOERROR;
}


//------------------
// brun
//------------------
jerror_t DTRDPoint_factory::brun(JEventLoop *loop, int32_t runnumber)
{
  // Get pointer to DGeometry object
  DApplication* dapp=dynamic_cast<DApplication*>(eventLoop->GetJApplication());
  const DGeometry *dgeom  = dapp->GetDGeometry(runnumber);
  
  // Get GEM geometry from xml (CCDB or private HDDS)
  dgeom->GetTRDZ(dTRDz);
  
  // Some parameters for defining matching
  TIME_DIFF_MAX = 50.;
//   DIST_DIFF_MAX = 10.;
  //DE_DIFF_MAX = 1.0;

  gPARMS->SetDefaultParameter("TRDPOINT:TIME_DIFF_MAX",TIME_DIFF_MAX);
//   gPARMS->SetDefaultParameter("TRDPOINT:DIST_DIFF_MAX",DIST_DIFF_MAX);

  return NOERROR;
}

jerror_t DTRDPoint_factory::erun(void){
 
  return NOERROR;
}
///
/// DTRDPoint_factory::evnt():
/// this is the place that produces points from wire hits and GEM strips
///
jerror_t DTRDPoint_factory::evnt(JEventLoop* eventLoop, uint64_t eventNo) {

	// Get strip clusters
	vector<const DTRDStripCluster*> stripClus;
	eventLoop->Get(stripClus);

	// Sift through clusters and select out X and Y plane wires
	vector<const DTRDStripCluster*> stripClusX,stripClusY;
	for (unsigned int i=0; i < stripClus.size(); i++) {
		if (stripClus[i]->plane == 0)
			stripClusX.push_back(stripClus[i]);
		else if (stripClus[i]->plane == 1)
			stripClusY.push_back(stripClus[i]);
	}
	
	// match clusters in X and Y planes
	for(uint i=0; i<stripClusX.size(); i++){
		for(uint j=0; j<stripClusY.size(); j++){
	
			// calculate strip cluster time and position
			double t_diff = stripClusX[i]->t_avg - stripClusY[i]->t_avg;
			double dE = stripClusX[i]->q_tot + stripClusY[j]->q_tot;

			// some requirements for a good point
			if(fabs(t_diff) < TIME_DIFF_MAX) {   // && fabs(dE_amp_diff) <gem_dE_max) {
		
				// save new point
				DTRDPoint* newPoint = new DTRDPoint;     
				newPoint->x = stripClusX[i]->pos.x();
				newPoint->y = stripClusY[i]->pos.x();
				newPoint->t_x = stripClusX[i]->t_avg;
				newPoint->t_y = stripClusY[i]->t_avg;
				newPoint->time = (stripClusX[i]->t_avg*stripClusX[i]->q_tot + stripClusY[i]->t_avg*stripClusY[j]->q_tot) / dE;
				newPoint->dE = dE;
				newPoint->status = 1;
				//newPoint->itrack = 0;
				newPoint->z = (stripClusX[i]->pos.z()*stripClusX[i]->q_tot + stripClusY[i]->pos.z()*stripClusY[j]->q_tot) / dE + dTRDz[0];

				newPoint->AddAssociatedObject(stripClusX[i]);
				newPoint->AddAssociatedObject(stripClusY[j]);

				_data.push_back(newPoint);
			}
		}
	}

	// Make sure the data are both time- and z-ordered
	std::sort(_data.begin(),_data.end(),DTRDPoint_cmp);
	
	return NOERROR;
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
