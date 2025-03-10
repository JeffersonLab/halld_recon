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
			return sqrt(pow(p1.x-p2.x, 2) + pow(p1.y-p2.y, 2));
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

		// int MinClustSize;
		// double MinClustWidth;
		// double MinClustLength;
		// double zStart;
		// double zEnd;

		const int NUM_X_PLANES = 720;
		const int NUM_Y_PLANES = 432;

};

#endif // DFACTORY_DTRDSTRIPCLUSTER_H

