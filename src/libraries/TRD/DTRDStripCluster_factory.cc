
#include "DTRDStripCluster_factory.h"

#include <JANA/JEvent.h>
#include "DTRDHit.h"

static bool DTRDHit_cmp(const DTRDHit* a, const DTRDHit* b) {
  if (a->plane==b->plane){
    return a->t < b->t;
  }
  return a->plane < b->plane;
}


///
/// DTRDHit_strip_cmp():
///	a non-member function passed to std::sort() for sorting DTRDHit pointers by
/// their strip (wire or strip) numbers. Typically only used for a single layer
/// of hits.
///
static bool DTRDHit_strip_cmp(const DTRDHit* a, const DTRDHit* b) {
	if(a->strip != b->strip) return a->strip < b->strip;
	if(a->t       != b->t      ) return a->t < b->t;
	return a->pulse_height < b->pulse_height;
}

///
/// DTRDHit_time_cmp()
///    a non-member function passed to std::stable_sort() for sorting DTRDHit 
/// pointers in order of increasing time, provided that the time difference is
/// significant.
///

static bool DTRDHit_time_cmp(const DTRDHit* a, const DTRDHit* b) {
  if (fabs(a->t-b->t)>HIT_TIME_DIFF_MIN && (a->t < b->t))
    return true;
  return false;
}

///
/// DTRDStripCluster_gPlane_cmp():
/// a non-member function passed to std::sort() for sorting DTRDStripCluster pointers
/// by their gPlane (plane number over all modules, 1-74) attributes.
///
static bool DTRDStripCluster_gPlane_cmp(	const DTRDStripCluster* a, 
					const DTRDStripCluster* b) {
	return a->plane < b->plane;
}

///
/// Initialization
///
void DTRDStripCluster_factory::Init()
{
	auto app = GetApplication();

	MINIMUM_HITS_FOR_CLUSTERING = 10;
    app->SetDefaultParameter("TRDCLUSTER:MINIMUM_HITS_FOR_CLUSTERING",MINIMUM_HITS_FOR_CLUSTERING);

	CLUSTERING_THRESHOLD = 1.2;
    app->SetDefaultParameter("TRDCLUSTER:CLUSTERING_THRESHOLD",CLUSTERING_THRESHOLD);

	eps = 20.0;
	minPts = 6;
	min_total_q = 0.0;

	app->SetDefaultParameter("TRDCLUSTER:DBSCAN_EPS",eps,"DBSCAN epsilon value (default: 20.0)");
	app->SetDefaultParameter("TRDCLUSTER:DBSCAN_MINPTS",minPts,"DBSCAN minimum number of points (default: 6)");
	app->SetDefaultParameter("TRDCLUSTER:MIN_TOTAL_Q",min_total_q,"Minimum total energy for a cluster (default: 0.0)");

    return;	
}


///
/// DTRDStripCluster_factory::StripToPosition():
/// calculate position along plane
///
double DTRDStripCluster_factory::StripToPosition(int iplane, const DTRDHit *hit)
{
	// better to pull this from CCDB, also probably the pitch as well
	double ny = 0;
	if(iplane == 0) {
		ny = NUM_X_PLANES;
	} else if(iplane == 1) {
		ny = NUM_Y_PLANES;
	} 
	double ymi = 0.;     double yma = ny;
	double biny = (yma-ymi)/ny;

	return double(hit->strip)/double(ny)*(yma-ymi)+ymi-biny/2.; 
}


///
/// DTRDStripCluster_factory::evnt():
/// This (along with DTRDStripCluster_factory::pique()) 
/// is the place cathode hits are associated into cathode clusters.  
///
void DTRDStripCluster_factory::Process(const std::shared_ptr<const JEvent>& event) 
{
	vector<DTRDStripCluster*> results;

	vector<const DTRDHit*> allHits;
	vector<const DTRDHit*> planeHits[2];
	
	event->Get<DTRDHit>(allHits);
	
	if (allHits.size() == 0) 
		return;

	// require a minimum number of hits
    if (allHits.size() < MINIMUM_HITS_FOR_CLUSTERING) {
    	return;
    }

	// Sort hits by layer number and by time
	sort(allHits.begin(),allHits.end(),DTRDHit_cmp);
	
	// Sift through all hits and select out X and Y hits.
	// Also apply the raw hit analysis
	for (vector<const DTRDHit*>::iterator i = allHits.begin(); i != allHits.end(); ++i) {
		// sort hits
		int stripPlane = (*i)->plane-1;
		int strip = (*i)->strip-1;
		if( (stripPlane<0) || (stripPlane>=2) ){ // only two planes
			static int Nwarn = 0;
			if( Nwarn<10 ){
				jerr << " stripPlane is outside of array bounds!! stripPlane="<< stripPlane << std::endl;
				if( ++Nwarn==10 )jerr << " LAST WARNING!" << std::endl;
			}
			continue;
		}
		planeHits[stripPlane].push_back(*i);
		
	}

	// cout << "Event " << eventNo << " has " << allHits.size() << " hits" << endl;
	// cout << "Number of hits in plane 1: " << planeHits[0].size() << endl;
	// cout << "Number of hits in plane 2: " << planeHits[1].size() << endl;

	// do the clustering
	
 	for(uint iplane = 0; iplane < 2; iplane++) {
 
 		if(planeHits[iplane].size()>0){			
			vector<Point> points;

 			for(int ihit=0; ihit < planeHits[iplane].size(); ihit++) {
 				const DTRDHit* hit = planeHits[iplane][ihit];
 				
				// const float CL_DIST=2.7; // mm
							
				double c1 = hit->q;          // energy
				double x1=hit->t;         // UPDATE: probably need to convert
				double y1=StripToPosition(iplane, hit);
				
				if (c1 < CLUSTERING_THRESHOLD) continue;

				// cout << "Hit " << ihit << " in plane " << iplane+1 << " has energy " << c1 << " and position " << y1 << endl;

				points.emplace_back(hit, x1, y1, c1);

			}
			
			DBSCAN(points, eps, minPts);
			const int NClusters = GetNCluster(points);
			// cout << "Number of clusters: " << NClusters << endl;
			for (int iClusterId=1; iClusterId<=NClusters; iClusterId++) {
				if (GetTotalClusterEnergy(points, iClusterId) < min_total_q) continue;
				
				// make a new cluster
				DTRDStripCluster *new_cluster = new DTRDStripCluster;
				new_cluster->plane = iplane+1;
				new_cluster->q_tot = GetTotalClusterEnergy(points, iClusterId);
				pair<double,double> centroid = GetClusterCentroid(points, iClusterId);
				new_cluster->t_avg = centroid.first;
				new_cluster->num_hits = GetClusterNHits(points, iClusterId);

				Point p_max_q = GetMaxQPoint(points, iClusterId);
				new_cluster->q_max = p_max_q.weight;
				new_cluster->t_max = p_max_q.x;

				if (iplane==0)	{
					new_cluster->pos.SetXYZ(centroid.second, 0, 0);
					new_cluster->pos_max.SetXYZ(p_max_q.y, 0, 0);
				}
				else {
					new_cluster->pos.SetXYZ(0, centroid.second, 0);
					new_cluster->pos_max.SetXYZ(0, p_max_q.y, 0);
				}
				for (auto &point : points) {
					if (point.clusterId == iClusterId) {
						new_cluster->members.push_back(point.hit);
						new_cluster->AddAssociatedObject(point.hit);
					}
				}
				results.push_back(new_cluster);
			} //----------- end  clustering loop ---------------
		}
	}


	// Ensure that the data are still in order of planes.
	std::sort(results.begin(), results.end(), DTRDStripCluster_gPlane_cmp);
	
	Set(results);
	
	return;	
}			


void DTRDStripCluster_factory::DBSCAN(vector<Point> &points, double eps, int minPts)
{
	int clusterId = 1;
	for (auto &point : points) {
		if (!point.visited) {
			point.visited = true;
			ExpandCluster(points, point, clusterId, eps, minPts);
			if (point.clusterId == clusterId) {
				clusterId++;
			}
		}
	}
}

void DTRDStripCluster_factory::ExpandCluster(vector<Point> &points, Point &point, int clusterId, double eps, int minPts)
{
	vector<Point*> seeds;
	for (auto &p : points) {
		if (PointsDistance(point, p) <= eps) {
			seeds.push_back(&p);
		}
	}

	if (seeds.size() < minPts) {
		point.clusterId = 0; // Mark as noise
		return;
	}

	for (auto &seed : seeds) {
		seed->clusterId = clusterId;
	}

	seeds.erase(remove(seeds.begin(), seeds.end(), &point), seeds.end());

	while (!seeds.empty()) {
		Point *current = seeds.back();
		seeds.pop_back();

		if (!current->visited) {
			current->visited = true;

			vector<Point*> result;
			for (auto &p : points) {
				if (PointsDistance(*current, p) <= eps) {
					result.push_back(&p);
				}
			}

			if (result.size() >= minPts) {
				for (auto &res : result) {
					if (res->clusterId == -1 || res->clusterId == 0) {
						if (res->clusterId == -1) {
							seeds.push_back(res);
						}
						res->clusterId = clusterId;
					}
				}
			}
		}
	}
}

pair<double,double> DTRDStripCluster_factory::GetClusterCentroid(vector<Point> &points, int clusterId)
{
	double sumX = 0;
	double sumY = 0;
	double count = 0;
	for (auto &point : points) {
		if (point.clusterId == clusterId) {
			sumX += point.x*point.weight;
			sumY += point.y*point.weight;
			count+=point.weight;
		}
	}
	return make_pair(sumX/count, sumY/count);
}

int DTRDStripCluster_factory::GetNCluster(vector<Point> &points)
{
	int nCluster = 0;
	for (auto &point : points) {
		if (point.clusterId > nCluster) {
			nCluster = point.clusterId;
		}
	}
	return nCluster;
}

int DTRDStripCluster_factory::GetClusterNHits(vector<Point> &points, int clusterId)
{
	int nHits = 0;
	for (auto &point : points) {
		if (point.clusterId == clusterId) {
			nHits++;
		}
	}
	return nHits;
}

float DTRDStripCluster_factory::GetTotalClusterEnergy(vector<Point> &points, int clusterId)
{
	float totalEnergy = 0;
	for (auto &point : points) {
		if (point.clusterId == clusterId) {
			totalEnergy += point.weight;
		}
	}
	return totalEnergy;
}