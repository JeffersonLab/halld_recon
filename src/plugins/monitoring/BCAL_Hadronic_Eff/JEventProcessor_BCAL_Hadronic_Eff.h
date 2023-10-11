// $Id$
//
// File: JEventProcessor_BCAL_Hadronic_Eff.h
//

#ifndef _JEventProcessor_BCAL_Hadronic_Eff_
#define _JEventProcessor_BCAL_Hadronic_Eff_

#include <JANA/JEventProcessor.h>
#include <JANA/Compatibility/JLockService.h>

#include "TH1I.h"
#include "TH2I.h"

#include "BCAL/DBCALShower.h"
#include "BCAL/DBCALPoint.h"
#include "BCAL/DBCALUnifiedHit.h"
#include "BCAL/DBCALGeometry.h"
#include "TRIGGER/DTrigger.h"
#include "TRACKING/DTrackTimeBased.h"

#include "PID/DChargedTrack.h"
#include "PID/DChargedTrackHypothesis.h"
#include "PID/DParticleID.h"
#include "PID/DDetectorMatches.h"
#include "ANALYSIS/DCutActions.h"
#include "ANALYSIS/DTreeInterface.h"

#include <vector>
#include <string>
#include <iostream>
#include <map>
#include <set>
#include <thread>

using namespace std;

class JEventProcessor_BCAL_Hadronic_Eff : public JEventProcessor
{
	public:
		JEventProcessor_BCAL_Hadronic_Eff(){
			SetTypeName("JEventProcessor_BCAL_Hadronic_Eff");
		};
		~JEventProcessor_BCAL_Hadronic_Eff(){};

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& locEvent) override;
		void Process(const std::shared_ptr<const JEvent>& locEvent) override;
		void EndRun() override;
		void Finish() override;

		std::shared_ptr<JLockService> lockService;

		double Calc_AverageSector(const map<int, set<const DBCALPoint*> >& locBCALPoints);
		double Calc_ProjectedSector(int locLayer, const map<int, map<int, set<const DBCALPoint*> > >& locSortedPoints);

		pair<const DBCALPoint*, double> Find_NearestPoint(double locProjectedSector, const map<int, set<const DBCALPoint*> >& locLayerBCALPoints, const DBCALCluster* locBCALCluster, double locTimeCut = -1.0);
		pair<const DBCALUnifiedHit*, double> Find_NearestHit(double locProjectedSector, const map<int, set<const DBCALUnifiedHit*> >& locLayerUnifiedHits, const DBCALCluster* locBCALCluster, const DBCALGeometry *locBCALGeom, double locTimeCut = -1.0);

		const DBCALPoint* Find_ClosestTimePoint(const set<const DBCALPoint*>& locPoints, const DBCALCluster* locBCALCluster, double locTimeCut);
		const DBCALUnifiedHit* Find_ClosestTimeHit(const set<const DBCALUnifiedHit*>& locHits, const DBCALCluster* locBCALCluster, double locTimeCut, const DBCALGeometry *locBCALGeom);

		template <typename DType> DType Calc_DeltaSector(DType locHitSector, DType locProjectedSector) const;

		bool Cut_BCALTiming(const DChargedTrackHypothesis* locChargedTrackHypothesis);

		//TRACK REQUIREMENTS
		double dMinTrackingFOM, dMaxBCALDeltaT;
		unsigned int dMinNumTrackHits;
		int dMinHitRingsPerCDCSuperlayer, dMinHitPlanesPerFDCPackage;
		double dMaxVertexR;
		DCutAction_TrackHitPattern* dCutAction_TrackHitPattern;

		//HISTOGRAMS
		int dHistFoundDeltaSector; //for histograms ONLY!!!
		map<int, map<bool, TH1I*> > dHistMap_HitFound, dHistMap_HitTotal; //int = layer, bool = isUpstream

		//TREE
		DTreeInterface* dTreeInterface;
		//thread_local: Each thread has its own object: no lock needed
			//important: manages it's own data internally: don't want to call new/delete every event!
		static thread_local DTreeFillData dTreeFillData;

		//VERTEX-Z
		double dTargetCenterZ;

		//EFFECTIVE VELOCITIES
		vector<double> effective_velocities;
		
};

template <typename DType> inline DType JEventProcessor_BCAL_Hadronic_Eff::Calc_DeltaSector(DType locHitSector, DType locProjectedSector) const
{
	//beware 2pi wrap-around!
	double locDeltaSector = double(locHitSector) - locProjectedSector;
	if(locDeltaSector > DType(96))
		locDeltaSector -= DType(192);
	if(locDeltaSector < -DType(96))
		locDeltaSector += DType(192);
	return locDeltaSector;
}

#endif // _JEventProcessor_BCAL_Hadronic_Eff_
