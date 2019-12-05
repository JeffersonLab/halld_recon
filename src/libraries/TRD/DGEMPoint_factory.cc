//********************************************************
// DGEMPoint_factory.cc - modeled after DFDCPseudo_factory 
//********************************************************

#include "DGEMPoint_factory.h"

///
/// DGEMPoint_cmp(): 
/// non-member function passed to std::sort() to sort DGEMHit pointers 
/// for the wires by their plane attributes.
///
bool DGEMPoint_cmp(const DGEMPoint* a, const DGEMPoint *b){
  if (a->detector == b->detector) return a->time<b->time;
  
  return a->detector<b->detector;
}

//------------------
// init
//------------------
jerror_t DGEMPoint_factory::init(void)
{

  return NOERROR;
}


//------------------
// brun
//------------------
jerror_t DGEMPoint_factory::brun(JEventLoop *loop, int32_t runnumber)
{
  // Get pointer to DGeometry object
  //DApplication* dapp=dynamic_cast<DApplication*>(eventLoop->GetJApplication());
  //const DGeometry *dgeom  = dapp->GetDGeometry(runnumber);
    
  // Get GEM geometry (needs to come from geometry file or CCDB?)
  gemX0 = 43.0;
  gemY0 = -80.0;
  gem_pitch = 0.04;   // 0.4 mm

  // Some parameters for defining wire and strip X/Y matching
  gem_time_max = 50;
  gem_dE_max = 1.0;

  return NOERROR;
}

jerror_t DGEMPoint_factory::erun(void){
 
  return NOERROR;
}
///
/// DGEMPoint_factory::evnt():
/// this is the place that produces points from wire hits and GEM strips
///
jerror_t DGEMPoint_factory::evnt(JEventLoop* eventLoop, uint64_t eventNo) {

	// Get all hits
	vector<const DGEMHit*> hits;
	eventLoop->Get(hits);
	if (hits.size()==0) return NOERROR;

	// Get strip clusters
	vector<const DGEMStripCluster*> stripClus;
	eventLoop->Get(stripClus);

	// Sift through clusters and select out X and Y plane wires
	vector<const DGEMStripCluster*> gemPackageClusX[5];
	vector<const DGEMStripCluster*> gemPackageClusY[5];
	for (unsigned int i=0; i < stripClus.size(); i++) {
		int plane = stripClus[i]->plane;
		if (plane%2 == 0)
			gemPackageClusX[plane/2].push_back(stripClus[i]);
		else
			gemPackageClusY[plane/2].push_back(stripClus[i]);
	}

	////////////////////
	// GEM SRS points //
	////////////////////
	for(uint ipkg=0; ipkg<5; ipkg++){
		vector<const DGEMStripCluster*> gemClusX = gemPackageClusX[ipkg];
		vector<const DGEMStripCluster*> gemClusY = gemPackageClusY[ipkg];
		
		for(uint i=0; i<gemClusX.size(); i++){
			for(uint j=0; j<gemClusY.size(); j++){
				
				// calculate strip cluster time and position
				double tx_clus = calcClusterTime(gemClusX[i]);
				double x_clus = calcClusterPosition(gemClusX[i]);
				double ty_clus = calcClusterTime(gemClusY[j]);
				double y_clus = calcClusterPosition(gemClusY[j]);
				
				double t_diff = tx_clus - ty_clus;
				double dE_amp = gemClusX[i]->q_tot + gemClusY[j]->q_tot;
				
				// some requirements for a good point
				if(fabs(t_diff) < gem_time_max) {// && fabs(dE_amp_diff) <gem_dE_max) {
					
					// save new point
					DGEMPoint* newPoint = new DGEMPoint;     
					newPoint->x = x_clus;
					newPoint->y = y_clus;
					newPoint->t_x = tx_clus;
					newPoint->t_y = ty_clus;
					newPoint->time = (tx_clus*gemClusX[i]->q_tot + ty_clus*gemClusY[j]->q_tot) / dE_amp;
					newPoint->dE_amp = dE_amp;
					newPoint->detector = ipkg;
					newPoint->status = 1;
					newPoint->itrack = 0;
					
					newPoint->AddAssociatedObject(gemClusX[i]);
					newPoint->AddAssociatedObject(gemClusY[j]);
					
					_data.push_back(newPoint);
				}
			}
		}
	}

	// Make sure the data are both time- and z-ordered
	std::sort(_data.begin(),_data.end(),DGEMPoint_cmp);
	
	return NOERROR;
}

double DGEMPoint_factory::calcClusterTime(const DGEMStripCluster *clus)
{
	double meanTime = 0;
	double pulseHeightSum = 0;
	for(uint i=0; i<clus->members.size(); i++) {
		const DGEMHit* hit = clus->members[i];
		meanTime += hit->t * hit->pulse_height;
		pulseHeightSum += hit->pulse_height;
	}
	
	return meanTime/pulseHeightSum;
}

double DGEMPoint_factory::calcClusterPosition(const DGEMStripCluster *clus)
{
	double meanPosition = 0;
	double pulseHeightSum = 0;
	for(uint i=0; i<clus->members.size(); i++) {
		const DGEMHit* hit = clus->members[i];
		pulseHeightSum += hit->pulse_height;
		if(hit->plane%2 == 0) // GEM X strip
			meanPosition += (gemX0 - hit->strip * gem_pitch)* hit->pulse_height;
		else // GEM Y strip
			meanPosition += (gemY0 + hit->strip * gem_pitch)* hit->pulse_height;
	}
	
	return meanPosition/pulseHeightSum;
}
