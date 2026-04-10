//********************************************************************
// DTRDStripCluster_factory.h: modeled after DFDCCathodeCluster_factory
//********************************************************************

#ifndef DFACTORY_DTRDSTRIPCLUSTER_H
#define DFACTORY_DTRDSTRIPCLUSTER_H

#include <JANA/JFactoryT.h>
using namespace std;

#include "DTRDStripCluster.h"
#include "DTRDHit.h"

#include <algorithm>
#include <map>
#include <cmath>

///
/// class DTRDStripCluster_factory: 
/// defines a JFactory for producing groups of cathode strips that form a cluster
///  
class DTRDStripCluster_factory:public JFactoryT<DTRDStripCluster> {
	public:
	    DTRDStripCluster_factory(){};
		~DTRDStripCluster_factory(){};
		
		///
		/// DTRDStripCluster_factory::pique():
		/// takes a single layer's worth of cathode hits and attempts to 
		/// create DTRDStripClusters
		/// by grouping together hits with consecutive strip numbers.
		///
		// void cluster(vector<const DTRDHit*>& h);
		
		double StripToPosition(int iplane, const DTRDHit *hit);
		struct Point{
			const DTRDHit *hit;
			double x,y,weight;
			bool visited;
			int clusterId;
			Point(const DTRDHit *hit, double x, double y, double weight) : hit(hit), x(x), y(y), weight(weight), visited(false), clusterId(-1) {}
		};
		double PointsDistance(Point p1, Point p2){
		  return sqrt(pow(p1.x/8.-p2.x/8., 2) + pow(p1.y-p2.y, 2));
		//   return fabs(p1.y-p2.y);
		}
		void ExpandCluster(vector<Point> &points, Point &point, int clusterId, double eps, int minPts);
		void DBSCAN(vector<Point> &points, double eps, int minPts);
		int GetNCluster(vector<Point> &points);
		int GetClusterNHits(vector<Point> &points, int clusterId);
		float GetTotalClusterEnergy(vector<Point> &points, int clusterId);
		pair<double,double> GetClusterCentroid(vector<Point> &points, int clusterId);
		Point GetMaxQPoint(vector<Point> &points, int clusterId){
			Point p_max = points[0];
			for (auto &point : points) {
				if (point.clusterId == clusterId && point.weight > p_max.weight) {
					p_max = point;
				}
			}
			return p_max;
		}
		double GetClusterPosWidth(vector<Point> &points, int clusterId) {
			double min_y = 1e9;
			double max_y = -1e9;
			for (auto &point : points) {
				if (point.clusterId == clusterId) {
					if (point.y < min_y) min_y = point.y;
					if (point.y > max_y) max_y = point.y;
				}
			}
			return max_y - min_y;
		}
		double GetClusterTimeWidth(vector<Point> &points, int clusterId) {
			double min_x = 1e9;
			double max_x = -1e9;
			for (auto &point : points) {
				if (point.clusterId == clusterId) {
					if (point.x < min_x) min_x = point.x;
					if (point.x > max_x) max_x = point.x;
				}
			}
			return max_x - min_x;
		}
			
	protected:
		///
		/// DTRDStripCluster_factory::evnt():
		/// This (along with DTRDStripCluster_factory::pique()) 
		/// is the place cathode hits are associated into cathode clusters. This function 
		/// should eventually be modified to do more sophisticated peak finding. 
		///
		void Process(const std::shared_ptr<const JEvent>& event) override;	
		void Init() override;

	private:
		int MINIMUM_HITS_FOR_CLUSTERING;
		double CLUSTERING_THRESHOLD;
		double eps;
		int minPts;
		double min_total_q;
		double max_pos_width;

		// int MinClustSize;
		// double MinClustWidth;
		// double MinClustLength;
		// double zStart;
		// double zEnd;

  	const int NUM_X_STRIPS = 720;
  	//const int NUM_Y_STRIPS = 432;
  	const int NUM_Y_STRIPS = 528; // but only 432 are instrumented?
  	const double STRIP_PITCH=0.1;

};

#endif // DFACTORY_DTRDSTRIPCLUSTER_H

