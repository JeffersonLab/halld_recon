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
  if (a->detector == b->detector) return a->time<b->time;
  
  return a->detector<b->detector;
}

//------------------
// Init
//------------------
void DTRDPoint_factory::Init()
{
}


//------------------
// BeginRun
//------------------
void DTRDPoint_factory::BeginRun(const std::shared_ptr<const JEvent>& event)
{
  auto event_number = event->GetEventNumber();
  auto runnumber = event->GetRunNumber();
  auto app = event->GetJApplication();
  auto geo_manager = app->GetService<DGeometryManager>();
  auto dgeom = geo_manager->GetDGeometry(runnumber);

  // Get GEM geometry from xml (CCDB or private HDDS)
  dgeom->GetTRDZ(dTRDz);
  
  // Get TRD geometry (needs to come from geometry file or CCDB?)
  wireX0 = 26.6;
  stripY0 = -80.0;
  wireX_pitch = 1.0;  // 10 mm
  stripY_pitch = 0.5; //  5 mm

  gemX0 = 43.0;
  gemY0 = 0;
  gem_pitch = 0.04;   // 0.4 mm

  if(runnumber > 70727 && runnumber < 70735) {
	  wireX0 = -50.6;
	  gemX0 = -34.2;
  }
  if(runnumber > 70734) {
	  wireX0 = -11.8;
	  gemX0 = -5.0;
  }

  // Some parameters for defining wire and strip X/Y matching
  wire_time_max = 50;
  gem_time_max = 400;
  gem_dE_max = 1.0;
}

void DTRDPoint_factory::EndRun(){
}
///
/// DTRDPoint_factory::evnt():
/// this is the place that produces points from wire hits and GEM strips
///
void DTRDPoint_factory::Process(const std::shared_ptr<const JEvent>& event) {

	// Get all hits (wires and GEM strips)
	vector<const DTRDHit*> hits;
	event->Get(hits);
	if (hits.size()==0) return;

	// Sift through hits and select out X and Y plane wires
	vector<const DTRDHit*> wireHitsX;
	for (unsigned int i=0; i < hits.size(); i++) {
		if (hits[i]->plane == 0 || hits[i]->plane == 4)
			wireHitsX.push_back(hits[i]);
	}

	// Get strip clusters
	vector<const DTRDStripCluster*> stripClus;
	event->Get(stripClus);

	// Sift through clusters and select out X and Y plane wires
	vector<const DTRDStripCluster*> stripClusY,gemClusX,gemClusY;
	for (unsigned int i=0; i < stripClus.size(); i++) {
		if (stripClus[i]->plane == 1 || stripClus[i]->plane == 5)
			stripClusY.push_back(stripClus[i]);
		else if (stripClus[i]->plane == 2 || stripClus[i]->plane == 6)
			gemClusX.push_back(stripClus[i]);
		else if (stripClus[i]->plane == 3 || stripClus[i]->plane == 7)
			gemClusY.push_back(stripClus[i]);
	}
	
	/////////////////////
	// Wire TRD points //
	/////////////////////
	for(uint i=0; i<wireHitsX.size(); i++){
		for(uint j=0; j<stripClusY.size(); j++){
	
			// calculate strip cluster time and position
			double t_clus = calcClusterTime(stripClusY[j]);
			double y_clus = calcClusterPosition(stripClusY[j]);

			double t_diff = wireHitsX[i]->t - t_clus;
			double dE_amp = stripClusY[j]->q_tot;

			// some requirements for a good point
			if(fabs(t_diff) < wire_time_max) {
		
				// save new point
				DTRDPoint* newPoint = new DTRDPoint;     
				newPoint->x = wireX0 + wireHitsX[i]->strip * wireX_pitch;
				newPoint->y = y_clus;
				newPoint->t_x = wireHitsX[i]->t;
				newPoint->t_y = t_clus;
				newPoint->time = (wireHitsX[i]->t + t_clus) / 2.;
				newPoint->dE_amp = dE_amp;
				newPoint->detector = 2;
				newPoint->status = 1;
				newPoint->itrack = 0;
				newPoint->z = 0;
				if(dTRDz.size() == 5) newPoint->z = dTRDz[2];

				newPoint->AddAssociatedObject(wireHitsX[i]);
				newPoint->AddAssociatedObject(stripClusY[j]);

				Insert(newPoint);
			}
		}
	}

	////////////////////
	// GEM TRD points //
	////////////////////
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
				DTRDPoint* newPoint = new DTRDPoint;     
				newPoint->x = x_clus;
				newPoint->y = y_clus;
				newPoint->t_x = tx_clus;
				newPoint->t_y = ty_clus;
				newPoint->time = (tx_clus*gemClusX[i]->q_tot + ty_clus*gemClusY[j]->q_tot) / dE_amp;
				newPoint->dE_amp = dE_amp;
				newPoint->detector = 3;
				newPoint->status = 1;
				newPoint->itrack = 0;
				newPoint->z = 0;
				if(dTRDz.size() == 5) newPoint->z = dTRDz[3];

				newPoint->AddAssociatedObject(gemClusX[i]);
				newPoint->AddAssociatedObject(gemClusY[j]);

				Insert(newPoint);
			}
		}
	}

	// Make sure the data are both time- and z-ordered
	std::sort(mData.begin(),mData.end(),DTRDPoint_cmp);
}

double DTRDPoint_factory::calcClusterTime(const DTRDStripCluster *clus)
{
	double meanTime = 0;
	double pulseHeightSum = 0;
	for(uint i=0; i<clus->members.size(); i++) {
		const DTRDHit* hit = clus->members[i];
		meanTime += hit->t * hit->pulse_height;
		pulseHeightSum += hit->pulse_height;
	}
	
	return meanTime/pulseHeightSum;
}

double DTRDPoint_factory::calcClusterPosition(const DTRDStripCluster *clus)
{
	double meanPosition = 0;
	double pulseHeightSum = 0;
	for(uint i=0; i<clus->members.size(); i++) {
		const DTRDHit* hit = clus->members[i];
		pulseHeightSum += hit->pulse_height;
		if(hit->plane == 1 || hit->plane == 5) // Wire TRD strip
			meanPosition += (stripY0 + hit->strip * stripY_pitch)* hit->pulse_height;
		else if(hit->plane == 2 || hit->plane == 6) // GEM TRD X strip
			meanPosition += (gemX0 + hit->strip * gem_pitch)* hit->pulse_height;
		else if(hit->plane == 3 || hit->plane == 7) // GEM TRD Y strip
			meanPosition += (gemY0 + hit->strip * gem_pitch)* hit->pulse_height;
	}
	
	return meanPosition/pulseHeightSum;
}
