//******************************************************************
// DTRDPoint_factory.h: modeled after DFDCPeudo_factory
//******************************************************************
#ifndef DFACTORY_DTRDPOINT_H
#define DFACTORY_DTRDPOINT_H

#include <JANA/JFactoryT.h>

#include "DTRDPoint.h"
#include "HDGEOMETRY/DGeometry.h"
#include <tuple>

///
/// class DTRDPoint_factory: definition for a JFactory that
/// produces points from strip hits
/// 
class DTRDPoint_factory : public JFactoryT<DTRDPoint> {
	public:
		
		DTRDPoint_factory(){};
		~DTRDPoint_factory(){};					

	protected:
		///
		/// DTRDPoint_factory::Process:
		/// this is the place that wire hits and GEM strip clusters 
		/// are organized into points.
		///
		void Init() override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;

// 		double calcClusterTime(const DTRDStripCluster *clus);
// 		double calcClusterPosition(const DTRDStripCluster *clus);
		struct Cluster{
			const DTRDStripCluster *clus;
			bool matched;
			int pointID;
			Cluster(const DTRDStripCluster *c, bool m, int id) : clus(c), matched(m), pointID(id) {}
		};

		struct PointCandidates{
			vector<Cluster> clustersX;
			vector<Cluster> clustersY;

			PointCandidates() : clustersX(), clustersY() {}
			void AddClusterX(const DTRDStripCluster *clus, bool matched, int pointID) {
				clustersX.emplace_back(clus, matched, pointID);
			}
			void AddClusterY(const DTRDStripCluster *clus, bool matched, int pointID) {
				clustersY.emplace_back(clus, matched, pointID);
			}
		};

		void MatchClusters(PointCandidates &candidate){	
			// Calculate the time difference between clusters in X and Y planes for all possible pairs
			vector<tuple<double, size_t, size_t>> clusterPairs;
			for (size_t ix = 0; ix < candidate.clustersX.size(); ++ix) {
			    for (size_t iy = 0; iy < candidate.clustersY.size(); ++iy) {
			        double timeDiff = abs(candidate.clustersX[ix].clus->t_avg - candidate.clustersY[iy].clus->t_avg);
			        if (timeDiff < TIME_DIFF_MAX) {
			            clusterPairs.emplace_back(timeDiff, ix, iy);
			        }
			    }
			}

			// Sort pairs by time difference
			sort(clusterPairs.begin(), clusterPairs.end(),
			    [](const tuple<double, size_t, size_t> &a, const tuple<double, size_t, size_t> &b) {
			        return get<0>(a) < get<0>(b);
			    });
			
			int pointID = 0;
			for (const auto &pair : clusterPairs) {
			    size_t ix = get<1>(pair);
			    size_t iy = get<2>(pair);
			
			    // Check if clusters are already matched
			    if (!candidate.clustersX[ix].matched && !candidate.clustersY[iy].matched) {
			        // Mark clusters as matched
			        candidate.clustersX[ix].matched = true;
			        candidate.clustersY[iy].matched = true;
			        candidate.clustersX[ix].pointID = pointID;
			        candidate.clustersY[iy].pointID = pointID;
			        pointID++;
			    }
			}
		}

		int GetNUnmatchedClusters(const PointCandidates &candidate) const {
			int nUnmatched = 0;
			for (const auto &clusX : candidate.clustersX) {
				if (!clusX.matched) nUnmatched++;
			}
			for (const auto &clusY : candidate.clustersY) {
				if (!clusY.matched) nUnmatched++;
			}
			return nUnmatched;
		}		
 		
	private:		
  		double TIME_DIFF_MAX;
  		double DRIFT_VELOCITY;
  		double dTRDx=0.,dTRDy=0.,dTRDz=0.;
   		double dE_DIFF_MAX;
		int MIN_NClusters;
};

#endif // DFACTORY_DTRDPOINT_H

