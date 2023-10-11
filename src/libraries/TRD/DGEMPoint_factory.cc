//********************************************************
// DGEMPoint_factory.cc - modeled after DFDCPseudo_factory 
//********************************************************

#include "DGEMPoint_factory.h"

#include <JANA/JEvent.h>
#include "DANA/DGeometryManager.h"
#include "HDGEOMETRY/DGeometry.h"

using std::vector;

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
// Init
//------------------
void DGEMPoint_factory::Init()
{
}


//------------------
// BeginRun
//------------------
void DGEMPoint_factory::BeginRun(const std::shared_ptr<const JEvent>& event)
{
  auto runnumber = event->GetRunNumber();
  auto app = event->GetJApplication();
  auto geo_manager = app->GetService<DGeometryManager>();
  auto dgeom = geo_manager->GetDGeometry(runnumber);

  // Get GEM geometry from xml (CCDB or private HDDS)
  dgeom->GetTRDZ(dTRDz);

  // X and Y positions currently hard coded...
  gemX0 = 43.0;
  gemY0 = -80.0;
  if(runnumber > 70727 && runnumber < 70735) gemX0 = -34.3;
  if(runnumber > 70734) gemX0 = 4.75;
  gem_pitch = 0.04;   // 0.4 mm

  // Some parameters for defining wire and strip X/Y matching
  gem_time_max = 50;
  gem_dE_max = 1.0;
}

void DGEMPoint_factory::EndRun(){
}

///
/// DGEMPoint_factory::evnt():
/// this is the place that produces points from wire hits and GEM strips
///
void DGEMPoint_factory::Process(const std::shared_ptr<const JEvent>& event) {

	// Get all hits
	vector<const DGEMHit*> hits;
	event->Get(hits);
	if (hits.size()==0) return;

	// Get strip clusters
	vector<const DGEMStripCluster*> stripClus;
	event->Get(stripClus);

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
					newPoint->z = 0;
					if(dTRDz.size() == 5) newPoint->z = dTRDz[ipkg];
					
					newPoint->AddAssociatedObject(gemClusX[i]);
					newPoint->AddAssociatedObject(gemClusY[j]);
					
					Insert(newPoint);
				}
			}
		}
	}

	// Make sure the data are both time- and z-ordered
	std::sort(mData.begin(),mData.end(),DGEMPoint_cmp);
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
