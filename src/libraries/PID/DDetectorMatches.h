// $Id$
//
//    File: DDetectorMatches_factory.h
// Created: Tue Aug  9 14:29:24 EST 2011
// Creator: pmatt (on Linux ifarml6 2.6.18-128.el5 x86_64)
//

#ifndef _DDetectorMatches_
#define _DDetectorMatches_

#include <vector>
#include <memory>
#include <utility>
#include <string>

#include <TRACKING/DTrackingData.h>
#include <TOF/DTOFPoint.h>
#include <FMWPC/DCTOFPoint.h>
#include <FMWPC/DFMWPCCluster.h>
#include <BCAL/DBCALShower.h>
#include <FCAL/DFCALShower.h>
#include <START_COUNTER/DSCHit.h>
#include <DIRC/DDIRCPmtHit.h>
#include <DIRC/DDIRCTruthPmtHit.h>

using namespace std;

class DBCALShowerMatchParams
{
	public:
		DBCALShowerMatchParams(void) : dBCALShower(NULL),
    dx(0.0), dFlightTime(0.0), dFlightTimeVariance(0.0), dPathLength(0.0), dDeltaPhiToShower(0.0), dDeltaPhiToShowerCut(0.0),dDeltaZToShower(0.0){}

		const DBCALShower* dBCALShower;

		double dx; //the distance the track traveled through the detector system up to the shower position
		double dFlightTime; //flight time from DKinematicData::position() to the shower
		double dFlightTimeVariance;
		double dPathLength; //path length from DKinematicData::position() to the shower
		double dDeltaPhiToShower; //between track and shower //is signed: BCAL - Track //in radians
		double dDeltaPhiToShowerCut; //momentum-dependent cut //in degrees
		double dDeltaZToShower; //between track and shower //is signed: BCAL - Track

		double Get_DistanceToTrack(void) const
		{
			double locRSq = dBCALShower->x*dBCALShower->x + dBCALShower->y*dBCALShower->y;
			return sqrt(dDeltaZToShower*dDeltaZToShower + dDeltaPhiToShower*dDeltaPhiToShower*locRSq);
		}
};

class DFCALSingleHitMatchParams
{
 public:
 DFCALSingleHitMatchParams(void): dEHit(0.0), dTHit(0.0), dx(0.0), dFlightTime(0.0), dFlightTimeVariance(0.0), dPathLength(0.0), dDOCAToHit(0.0){}
  double dEHit; // energy of FCAL hit
  double dTHit; // time of FCAL hit
  double dx; //the distance the track traveled through the block
  double dFlightTime; //flight time from DKinematicData::position() to the block
  double dFlightTimeVariance;
  double dPathLength; //path length from DKinematicData::position() to the block
  double dDOCAToHit; //DOCA of track to block
};

class DFCALShowerMatchParams
{
	public:
		DFCALShowerMatchParams(void) : dFCALShower(NULL),
    dx(0.0), dFlightTime(0.0), dFlightTimeVariance(0.0), dPathLength(0.0), dDOCAToShower(0.0), dE5x5(0.0), dE3x3(0.0), dEcenter(0.0){}

		const DFCALShower* dFCALShower;

		double dx; //the distance the track traveled through the detector system up to the shower position
		double dFlightTime; //flight time from DKinematicData::position() to the shower
		double dFlightTimeVariance;
		double dPathLength; //path length from DKinematicData::position() to the shower
		double dDOCAToShower; //DOCA of track to shower
		double dE5x5; // energy sum over 5x5 array centered on track projection
		double dE3x3; // energy sum over 3x3 array centered on track projection
		double dEcenter; // energy in "center" block
};

class DCTOFHitMatchParams
{
 public:
 DCTOFHitMatchParams(void) : dCTOFPoint(NULL),dEdx(0.0),dFlightTime(0.0), 
    dDeltaXToHit(0.0), dDeltaYToHit(0.0){}

  const DCTOFPoint* dCTOFPoint;

  double dEdx; //dE: the energy lost by the track, dx: the distance the track traveled through the detector
  double dFlightTime; //flight time from DKinematicData::position() to the hit
  double dDeltaXToHit; //between track and hit
  double dDeltaYToHit; //between track and hit

  double Get_DistanceToTrack(void) const
  {
    return sqrt(dDeltaXToHit*dDeltaXToHit + dDeltaYToHit*dDeltaYToHit);
  }
};


class DTOFHitMatchParams
{
 public:
 DTOFHitMatchParams(void) : dTOFPoint(NULL),
    dHitTime(0.0), dHitTimeVariance(0.0), dHitEnergy(0.0), dEdx(0.0),
    dEdx1(0.0), dEdx2(0.0), dFlightTime(0.0), dFlightTimeVariance(0.0), 
    dPathLength(0.0), dDeltaXToHit(0.0), dDeltaYToHit(0.0){}

		const DTOFPoint* dTOFPoint;

		double dHitTime; //This can be different from DTOFPoint::t, if the DTOFPoint was (e.g.) an unmatched, single-ended paddle
		double dHitTimeVariance; //This can be different from DTOFPoint::tErr, if the DTOFPoint was (e.g.) an unmatched, single-ended paddle
		double dHitEnergy; //This can be different from DTOFPoint::dE, if the DTOFPoint was (e.g.) an unmatched, single-ended paddle

		double dEdx; //dE/dx; dE: the energy lost by the track, dx: the distance the track traveled through the detector system (dHitEnergy/dEdx)
		double dEdx1; // dE/dx for plane 1
		double dEdx2; // dE/dx for plane 2
		double dFlightTime; //flight time from DKinematicData::position() to the hit
		double dFlightTimeVariance;
		double dPathLength; //path length from DKinematicData::position() to the hit
		double dDeltaXToHit; //between track and hit //is signed: TOF - Track //is LARGE if TOF x-position not well-defined!
		double dDeltaYToHit; //between track and hit //is signed: TOF - Track //is LARGE if TOF x-position not well-defined!

		double Get_DistanceToTrack(void) const
		{
			return sqrt(dDeltaXToHit*dDeltaXToHit + dDeltaYToHit*dDeltaYToHit);
		}
};

class DSCHitMatchParams
{
	public:
		DSCHitMatchParams(void) : dSCHit(NULL), dHitTime(0.0), dHitTimeVariance(0.0),
		dHitEnergy(0.0), dEdx(0.0), dFlightTime(0.0), dFlightTimeVariance(0.0), dPathLength(0.0), dDeltaPhiToHit(0.0){}

		const DSCHit* dSCHit;

		double dHitTime; //not the same as DSCHit time: corrected for propagation along scintillator
		double dHitTimeVariance;
		double dHitEnergy; //not the same as DSCHit energy: corrected for attenuation

		double dEdx; //dE/dx; dE: the energy lost by the track, dx: the distance the track traveled through the detector system (dHitEnergy/dEdx)
		double dFlightTime; //flight time from DKinematicData::position() to the hit
		double dFlightTimeVariance;
		double dPathLength; //path length from DKinematicData::position() to the hit
		double dDeltaPhiToHit; //difference in phi between track and hit //units in radians
};

class DDIRCMatchParams
{
	public:
                DDIRCMatchParams(void) : dExpectedThetaC(0.0), dThetaC(0.0), dDeltaT(0.0), dLikelihoodElectron(0.0), dLikelihoodPion(0.0), dLikelihoodKaon(0.0), dLikelihoodProton(0.0), dNPhotons(0), dExtrapolatedPos(0,0,0), dExtrapolatedMom(0,0,0){} 

		double dExpectedThetaC, dThetaC, dDeltaT;
		double dLikelihoodElectron, dLikelihoodPion, dLikelihoodKaon, dLikelihoodProton;
		int dNPhotons;
		DVector3 dExtrapolatedPos, dExtrapolatedMom;
		double dExtrapolatedTime;
};

class DFMWPCMatchParams
{
 public:
  vector<int>dLayers;
  vector<int>dNhits;
  vector<int>dDists;
  vector<int>dClosestWires;
};

class DDetectorMatches : public JObject
{
	public:
		JOBJECT_PUBLIC(DDetectorMatches);

		//GETTERS:
		inline bool Get_BCALMatchParams(const DTrackingData* locTrack, vector<shared_ptr<const DBCALShowerMatchParams> >& locMatchParams) const;
		inline bool Get_FCALSingleHitMatchParams(const DTrackingData* locTrack, vector<shared_ptr<const DFCALSingleHitMatchParams> >& locMatchParams) const;
		inline bool Get_FCALMatchParams(const DTrackingData* locTrack, vector<shared_ptr<const DFCALShowerMatchParams> >& locMatchParams) const;
		inline bool Get_TOFMatchParams(const DTrackingData* locTrack, vector<shared_ptr<const DTOFHitMatchParams> >& locMatchParams) const;
		inline bool Get_CTOFMatchParams(const DTrackingData* locTrack, vector<shared_ptr<const DCTOFHitMatchParams> >& locMatchParams) const;
		inline bool Get_FMWPCMatchParams(const DTrackingData* locTrack, vector<shared_ptr<const DFMWPCMatchParams> >& locMatchParams) const;
		inline bool Get_SCMatchParams(const DTrackingData* locTrack, vector<shared_ptr<const DSCHitMatchParams> >& locMatchParams) const;
		inline bool Get_DIRCMatchParams(const DTrackingData* locTrack, shared_ptr<const DDIRCMatchParams>& locMatchParams) const;
	
		inline bool Get_IsMatchedToTrack(const DBCALShower* locBCALShower) const;
		inline bool Get_IsMatchedToTrack(const DFCALShower* locFCALShower) const;
		inline bool Get_IsMatchedToHit(const DTrackingData* locTrack) const;
		inline bool Get_IsMatchedToDetector(const DTrackingData* locTrack, DetectorSystem_t locDetectorSystem) const;

		inline bool Get_TrackMatchParams(const DBCALShower* locBCALShower, vector<shared_ptr<const DBCALShowerMatchParams> >& locMatchParams) const;
		inline bool Get_TrackMatchParams(const DFCALShower* locFCALShower, vector<shared_ptr<const DFCALShowerMatchParams> >& locMatchParams) const;
		inline bool Get_TrackMatchParams(const DTOFPoint* locTOFPoint, vector<shared_ptr<const DTOFHitMatchParams> >& locMatchParams) const;
		inline bool Get_TrackMatchParams(const DSCHit* locSCHit, vector<shared_ptr<const DSCHitMatchParams> >& locMatchParams) const;
		inline bool Get_DIRCTrackMatchParamsMap(map<shared_ptr<const DDIRCMatchParams>, vector<const DDIRCPmtHit*> >& locDIRCTrackMatchParamsMap);
		inline bool Get_TrackMatchParams(const DCTOFPoint* locCTOFPoint, vector<shared_ptr<const DCTOFHitMatchParams> >& locMatchParams) const;	

		inline bool Get_DistanceToNearestTrack(const DBCALShower* locBCALShower, double& locDistance) const;
		inline bool Get_DistanceToNearestTrack(const DBCALShower* locBCALShower, double& locDeltaPhi, double& locDeltaZ) const;
		inline bool Get_DistanceToNearestTrack(const DFCALShower* locFCALShower, double& locDistance) const;

		inline bool Get_FlightTimePCorrelation(const DTrackingData* locTrack, DetectorSystem_t locDetectorSystem, double& locCorrelation) const;

		//Get # Matches
		inline size_t Get_NumTrackBCALMatches(void) const;
		inline size_t Get_NumTrackFCALMatches(void) const;
		inline size_t Get_NumTrackTOFMatches(void) const;
		inline size_t Get_NumTrackSCMatches(void) const;
		inline size_t Get_NumTrackDIRCMatches(void) const;
		inline size_t Get_NumTrackCTOFMatches(void) const;
		inline size_t Get_NumTrackFMWPCMatches(void) const;


		//SETTERS:
		inline void Add_Match(const DTrackingData* locTrack, const DBCALShower* locBCALShower, const shared_ptr<const DBCALShowerMatchParams>& locShowerMatchParams);
		inline void Add_Match(const DTrackingData* locTrack, const shared_ptr<const DFCALSingleHitMatchParams>& locFCALSingleHitMatchParams);
		inline void Add_Match(const DTrackingData* locTrack, const DFCALShower* locFCALShower, const shared_ptr<const DFCALShowerMatchParams>& locShowerMatchParams);
		inline void Add_Match(const DTrackingData* locTrack, const DTOFPoint* locTOFPoint, const shared_ptr<const DTOFHitMatchParams>& locHitMatchParams);
		inline void Add_Match(const DTrackingData* locTrack, const DCTOFPoint* locCTOFPoint, const shared_ptr<const DCTOFHitMatchParams>& locHitMatchParams);
		inline void Add_Match(const DTrackingData* locTrack, const DSCHit* locSCHit, const shared_ptr<const DSCHitMatchParams>& locHitMatchParams);
		inline void Add_Match(const DTrackingData* locTrack, const shared_ptr<const DDIRCMatchParams>& locDIRCMatchParams);
		inline void Add_Match(const DTrackingData *locTrack,
				      const shared_ptr<const DFMWPCMatchParams>& locFMWPCMatchParams);

		inline void Set_DistanceToNearestTrack(const DBCALShower* locBCALShower, double locDeltaPhi, double locDeltaZ);
		inline void Set_DistanceToNearestTrack(const DFCALShower* locFCALShower, double locDistanceToNearestTrack);
		inline void Set_FlightTimePCorrelation(const DTrackingData* locTrack, DetectorSystem_t locDetectorSystem, double locCorrelation);

		void Summarize(JObjectSummary& summary) const override
		{
			summary.add(Get_NumTrackBCALMatches(), "#_Track_BCAL_Matches", "%d");
			summary.add(Get_NumTrackFCALMatches(), "#_Track_FCAL_Matches", "%d");
			summary.add(Get_NumTrackTOFMatches(), "#_Track_TOF_Matches", "%d");
			summary.add(Get_NumTrackSCMatches(), "#_Track_SC_Matches", "%d");
			summary.add(Get_NumTrackDIRCMatches(), "#_Track_DIRC_Matches", "%d");
			summary.add(Get_NumTrackCTOFMatches(), "#_Track_CTOF_Matches", "%d");
			summary.add(Get_NumTrackFMWPCMatches(), "#_Track_FMWPC_Matches", "%d");
		}

	private:

		//for each track, stores the information about each match
		map<const DTrackingData*, vector<shared_ptr<const DBCALShowerMatchParams> > > dTrackBCALMatchParams;
		map<const DTrackingData*, vector<shared_ptr<const DFCALSingleHitMatchParams> > > dTrackFCALSingleHitMatchParams;
		map<const DTrackingData*, vector<shared_ptr<const DFCALShowerMatchParams> > > dTrackFCALMatchParams;
		map<const DTrackingData*, vector<shared_ptr<const DTOFHitMatchParams> > > dTrackTOFMatchParams;
		map<const DTrackingData*, vector<shared_ptr<const DCTOFHitMatchParams> > > dTrackCTOFMatchParams;
		map<const DTrackingData*, vector<shared_ptr<const DFMWPCMatchParams> > > dTrackFMWPCMatchParams;
		map<const DTrackingData*, vector<shared_ptr<const DSCHitMatchParams> > > dTrackSCMatchParams;
		map<const DTrackingData*, shared_ptr<const DDIRCMatchParams> > dTrackDIRCMatchParams;

		//reverse-direction maps of the above (match params are the same objects)
		map<const DBCALShower*, vector<shared_ptr<const DBCALShowerMatchParams> > > dBCALTrackMatchParams;
		map<const DFCALShower*, vector<shared_ptr<const DFCALShowerMatchParams> > > dFCALTrackMatchParams;
		map<const DTOFPoint*, vector<shared_ptr<const DTOFHitMatchParams> > > dTOFTrackMatchParams;
		map<const DCTOFPoint*, vector<shared_ptr<const DCTOFHitMatchParams> > > dCTOFTrackMatchParams;
		map<const DSCHit*, vector<shared_ptr<const DSCHitMatchParams> > > dSCTrackMatchParams;
		map<shared_ptr<const DDIRCMatchParams>, vector<const DDIRCPmtHit*> > dDIRCTrackMatchParams;

		//correlations between: (the flight time from a given detector system hit/shower to DKinematicData::position()), and the momentum at DKinematicData::position()
			//Note that it is assumed that these correlations will not change between the different objects of each type
		map<const DTrackingData*, map<DetectorSystem_t, double> > dFlightTimePCorrelations;

		//for BDT: help to determine if a shower is neutral or not
		map<const DBCALShower*, pair<double, double> > dBCALShowerDistanceToNearestTrack; //first double is delta-phi, second is delta-z
		map<const DFCALShower*, double> dFCALShowerDistanceToNearestTrack;
};

inline bool DDetectorMatches::Get_BCALMatchParams(const DTrackingData* locTrack, vector<shared_ptr<const DBCALShowerMatchParams> >& locMatchParams) const
{
	locMatchParams.clear();
	auto locIterator = dTrackBCALMatchParams.find(locTrack);
	if(locIterator == dTrackBCALMatchParams.end())
		return false;
	locMatchParams = locIterator->second;
	return true;
}

inline bool DDetectorMatches::Get_FCALSingleHitMatchParams(const DTrackingData* locTrack, vector<shared_ptr<const DFCALSingleHitMatchParams> >& locMatchParams) const
{
  locMatchParams.clear();
  auto locIterator = dTrackFCALSingleHitMatchParams.find(locTrack);
  if(locIterator == dTrackFCALSingleHitMatchParams.end())
    return false;
  locMatchParams = locIterator->second;
  return true;
}

inline bool DDetectorMatches::Get_FCALMatchParams(const DTrackingData* locTrack, vector<shared_ptr<const DFCALShowerMatchParams> >& locMatchParams) const
{
	locMatchParams.clear();
	auto locIterator = dTrackFCALMatchParams.find(locTrack);
	if(locIterator == dTrackFCALMatchParams.end())
		return false;
	locMatchParams = locIterator->second;
	return true;
}

inline bool DDetectorMatches::Get_CTOFMatchParams(const DTrackingData* locTrack, vector<shared_ptr<const DCTOFHitMatchParams> >& locMatchParams) const
{
	locMatchParams.clear();
	auto locIterator = dTrackCTOFMatchParams.find(locTrack);
	if(locIterator == dTrackCTOFMatchParams.end())
		return false;
	locMatchParams = locIterator->second;
	return true;
}

inline bool DDetectorMatches::Get_FMWPCMatchParams(const DTrackingData* locTrack, vector<shared_ptr<const DFMWPCMatchParams> >& locMatchParams) const
{
	locMatchParams.clear();
	auto locIterator = dTrackFMWPCMatchParams.find(locTrack);
	if(locIterator == dTrackFMWPCMatchParams.end())
		return false;
	locMatchParams = locIterator->second;
	return true;
}

inline bool DDetectorMatches::Get_TOFMatchParams(const DTrackingData* locTrack, vector<shared_ptr<const DTOFHitMatchParams> >& locMatchParams) const
{
	locMatchParams.clear();
	auto locIterator = dTrackTOFMatchParams.find(locTrack);
	if(locIterator == dTrackTOFMatchParams.end())
		return false;
	locMatchParams = locIterator->second;
	return true;
}

inline bool DDetectorMatches::Get_SCMatchParams(const DTrackingData* locTrack, vector<shared_ptr<const DSCHitMatchParams> >& locMatchParams) const
{
	locMatchParams.clear();
	auto locIterator = dTrackSCMatchParams.find(locTrack);
	if(locIterator == dTrackSCMatchParams.end())
		return false;
	locMatchParams = locIterator->second;
	return true;
}

inline bool DDetectorMatches::Get_DIRCMatchParams(const DTrackingData* locTrack, shared_ptr<const DDIRCMatchParams>& locMatchParams) const
{
	auto locIterator = dTrackDIRCMatchParams.find(locTrack);
	if(locIterator == dTrackDIRCMatchParams.end())
		return false;
	locMatchParams = locIterator->second;
	return true;
}

inline bool DDetectorMatches::Get_IsMatchedToTrack(const DBCALShower* locBCALShower) const
{
	return (dBCALTrackMatchParams.find(locBCALShower) != dBCALTrackMatchParams.end());
}

inline bool DDetectorMatches::Get_IsMatchedToTrack(const DFCALShower* locFCALShower) const
{
	return (dFCALTrackMatchParams.find(locFCALShower) != dFCALTrackMatchParams.end());
}

inline bool DDetectorMatches::Get_IsMatchedToHit(const DTrackingData* locTrack) const
{
	if(dTrackBCALMatchParams.find(locTrack) != dTrackBCALMatchParams.end())
		return true;
	if(dTrackFCALMatchParams.find(locTrack) != dTrackFCALMatchParams.end())
		return true;
	if(dTrackTOFMatchParams.find(locTrack) != dTrackTOFMatchParams.end())
		return true;
	if(dTrackSCMatchParams.find(locTrack) != dTrackSCMatchParams.end())
		return true;
	return false;
}

inline bool DDetectorMatches::Get_IsMatchedToDetector(const DTrackingData* locTrack, DetectorSystem_t locDetectorSystem) const
{
	if(locDetectorSystem == SYS_BCAL)
		return (dTrackBCALMatchParams.find(locTrack) != dTrackBCALMatchParams.end());
	else if(locDetectorSystem == SYS_FCAL)
		return (dTrackFCALMatchParams.find(locTrack) != dTrackFCALMatchParams.end());
	else if(locDetectorSystem == SYS_TOF)
		return (dTrackTOFMatchParams.find(locTrack) != dTrackTOFMatchParams.end());
	else if(locDetectorSystem == SYS_START)
		return (dTrackSCMatchParams.find(locTrack) != dTrackSCMatchParams.end());
	else if(locDetectorSystem == SYS_CTOF)
		return (dTrackCTOFMatchParams.find(locTrack) != dTrackCTOFMatchParams.end());
	else if(locDetectorSystem == SYS_FMWPC)
		return (dTrackFMWPCMatchParams.find(locTrack) != dTrackFMWPCMatchParams.end());
	else
		return false;
}

inline bool DDetectorMatches::Get_TrackMatchParams(const DBCALShower* locBCALShower, vector<shared_ptr<const DBCALShowerMatchParams> >& locMatchParams) const
{
	locMatchParams.clear();
	auto locIterator = dBCALTrackMatchParams.find(locBCALShower);
	if(locIterator == dBCALTrackMatchParams.end())
		return false;
	locMatchParams = locIterator->second;
	return true;
}

inline bool DDetectorMatches::Get_TrackMatchParams(const DFCALShower* locFCALShower, vector<shared_ptr<const DFCALShowerMatchParams> >& locMatchParams) const
{
	locMatchParams.clear();
	auto locIterator = dFCALTrackMatchParams.find(locFCALShower);
	if(locIterator == dFCALTrackMatchParams.end())
		return false;
	locMatchParams = locIterator->second;
	return true;
}

inline bool DDetectorMatches::Get_TrackMatchParams(const DCTOFPoint* locCTOFPoint, vector<shared_ptr<const DCTOFHitMatchParams> >& locMatchParams) const
{
	locMatchParams.clear();
	auto locIterator = dCTOFTrackMatchParams.find(locCTOFPoint);
	if(locIterator == dCTOFTrackMatchParams.end())
		return false;
	locMatchParams = locIterator->second;
	return true;
}

inline bool DDetectorMatches::Get_TrackMatchParams(const DTOFPoint* locTOFPoint, vector<shared_ptr<const DTOFHitMatchParams> >& locMatchParams) const
{
	locMatchParams.clear();
	auto locIterator = dTOFTrackMatchParams.find(locTOFPoint);
	if(locIterator == dTOFTrackMatchParams.end())
		return false;
	locMatchParams = locIterator->second;
	return true;
}

inline bool DDetectorMatches::Get_TrackMatchParams(const DSCHit* locSCHit, vector<shared_ptr<const DSCHitMatchParams> >& locMatchParams) const
{
	locMatchParams.clear();
	auto locIterator = dSCTrackMatchParams.find(locSCHit);
	if(locIterator == dSCTrackMatchParams.end())
		return false;
	locMatchParams = locIterator->second;
	return true;
}

inline bool DDetectorMatches::Get_DIRCTrackMatchParamsMap(map<shared_ptr<const DDIRCMatchParams>, vector<const DDIRCPmtHit*> >& locDIRCTrackMatchParamsMap)
{
	locDIRCTrackMatchParamsMap = dDIRCTrackMatchParams;
	return true;
}

inline bool DDetectorMatches::Get_DistanceToNearestTrack(const DBCALShower* locBCALShower, double& locDistance) const
{
	double locDeltaPhi, locDeltaZ;
	if(!Get_DistanceToNearestTrack(locBCALShower, locDeltaPhi, locDeltaZ))
		return false;
	double locRSq = locBCALShower->x*locBCALShower->x + locBCALShower->y*locBCALShower->y;
	locDistance = sqrt(locDeltaZ*locDeltaZ + locDeltaPhi*locDeltaPhi*locRSq);
	return true;
}

inline bool DDetectorMatches::Get_DistanceToNearestTrack(const DBCALShower* locBCALShower, double& locDeltaPhi, double& locDeltaZ) const
{
	auto locIterator = dBCALShowerDistanceToNearestTrack.find(locBCALShower);
	if(locIterator == dBCALShowerDistanceToNearestTrack.end())
		return false;
	locDeltaPhi = locIterator->second.first;
	locDeltaZ = locIterator->second.second;
	return true;
}

inline bool DDetectorMatches::Get_DistanceToNearestTrack(const DFCALShower* locFCALShower, double& locDistance) const
{
	auto locIterator = dFCALShowerDistanceToNearestTrack.find(locFCALShower);
	if(locIterator == dFCALShowerDistanceToNearestTrack.end())
		return false;
	locDistance = locIterator->second;
	return true;
}

inline bool DDetectorMatches::Get_FlightTimePCorrelation(const DTrackingData* locTrack, DetectorSystem_t locDetectorSystem, double& locCorrelation) const
{
	auto locTrackIterator = dFlightTimePCorrelations.find(locTrack);
	if(locTrackIterator == dFlightTimePCorrelations.end())
		return false;
	const map<DetectorSystem_t, double>& locDetectorMap = locTrackIterator->second;
	map<DetectorSystem_t, double>::const_iterator locDetectorIterator = locDetectorMap.find(locDetectorSystem);
	if(locDetectorIterator == locDetectorMap.end())
		return false;
	locCorrelation = locDetectorIterator->second;
	return true;
}

//Get # Matches
inline size_t DDetectorMatches::Get_NumTrackBCALMatches(void) const
{
	auto locIterator = dTrackBCALMatchParams.begin();
	unsigned int locNumTrackMatches = 0;
	for(; locIterator != dTrackBCALMatchParams.end(); ++locIterator)
		locNumTrackMatches += locIterator->second.size();
	return locNumTrackMatches;
}

inline size_t DDetectorMatches::Get_NumTrackFCALMatches(void) const
{
	auto locIterator = dTrackFCALMatchParams.begin();
	unsigned int locNumTrackMatches = 0;
	for(; locIterator != dTrackFCALMatchParams.end(); ++locIterator)
		locNumTrackMatches += locIterator->second.size();
	return locNumTrackMatches;
}

inline size_t DDetectorMatches::Get_NumTrackTOFMatches(void) const
{
	auto locIterator = dTrackTOFMatchParams.begin();
	unsigned int locNumTrackMatches = 0;
	for(; locIterator != dTrackTOFMatchParams.end(); ++locIterator)
		locNumTrackMatches += locIterator->second.size();
	return locNumTrackMatches;
}

inline size_t DDetectorMatches::Get_NumTrackCTOFMatches(void) const
{
	auto locIterator = dTrackCTOFMatchParams.begin();
	unsigned int locNumTrackMatches = 0;
	for(; locIterator != dTrackCTOFMatchParams.end(); ++locIterator)
		locNumTrackMatches += locIterator->second.size();
	return locNumTrackMatches;
}

inline size_t DDetectorMatches::Get_NumTrackFMWPCMatches(void) const
{
	auto locIterator = dTrackFMWPCMatchParams.begin();
	unsigned int locNumTrackMatches = 0;
	for(; locIterator != dTrackFMWPCMatchParams.end(); ++locIterator)
		locNumTrackMatches += locIterator->second.size();
	return locNumTrackMatches;
}

inline size_t DDetectorMatches::Get_NumTrackSCMatches(void) const
{
	auto locIterator = dTrackSCMatchParams.begin();
	unsigned int locNumTrackMatches = 0;
	for(; locIterator != dTrackSCMatchParams.end(); ++locIterator)
		locNumTrackMatches += locIterator->second.size();
	return locNumTrackMatches;
}

inline size_t DDetectorMatches::Get_NumTrackDIRCMatches(void) const
{
  	return dTrackDIRCMatchParams.size();
}

//SETTERS:
inline void DDetectorMatches::Add_Match(const DTrackingData* locTrack, const DBCALShower* locBCALShower, const shared_ptr<const DBCALShowerMatchParams>& locShowerMatchParams)
{
	dTrackBCALMatchParams[locTrack].push_back(locShowerMatchParams);
	dBCALTrackMatchParams[locBCALShower].push_back(locShowerMatchParams);
}
inline void DDetectorMatches::Add_Match(const DTrackingData* locTrack, const shared_ptr<const DFCALSingleHitMatchParams>& locFCALSingleHitMatchParams){
  dTrackFCALSingleHitMatchParams[locTrack].push_back(locFCALSingleHitMatchParams);
}
inline void DDetectorMatches::Add_Match(const DTrackingData* locTrack, const DFCALShower* locFCALShower, const shared_ptr<const DFCALShowerMatchParams>& locShowerMatchParams)
{
	dTrackFCALMatchParams[locTrack].push_back(locShowerMatchParams);
	dFCALTrackMatchParams[locFCALShower].push_back(locShowerMatchParams);
}
inline void DDetectorMatches::Add_Match(const DTrackingData* locTrack, const DTOFPoint* locTOFPoint, const shared_ptr<const DTOFHitMatchParams>& locHitMatchParams)
{
	dTrackTOFMatchParams[locTrack].push_back(locHitMatchParams);
	dTOFTrackMatchParams[locTOFPoint].push_back(locHitMatchParams);
}
inline void DDetectorMatches::Add_Match(const DTrackingData* locTrack, const DCTOFPoint* locCTOFPoint, const shared_ptr<const DCTOFHitMatchParams>& locHitMatchParams)
{
	dTrackCTOFMatchParams[locTrack].push_back(locHitMatchParams);
	dCTOFTrackMatchParams[locCTOFPoint].push_back(locHitMatchParams);
}
inline void DDetectorMatches::Add_Match(const DTrackingData* locTrack, const shared_ptr<const DFMWPCMatchParams>& locHitMatchParams)
{
	dTrackFMWPCMatchParams[locTrack].push_back(locHitMatchParams);
}
inline void DDetectorMatches::Add_Match(const DTrackingData* locTrack, const DSCHit* locSCHit, const shared_ptr<const DSCHitMatchParams>& locHitMatchParams)
{
	dTrackSCMatchParams[locTrack].push_back(locHitMatchParams);
	dSCTrackMatchParams[locSCHit].push_back(locHitMatchParams);
}
inline void DDetectorMatches::Add_Match(const DTrackingData* locTrack, const shared_ptr<const DDIRCMatchParams>& locHitMatchParams)
{
        dTrackDIRCMatchParams[locTrack] = locHitMatchParams;
}
inline void DDetectorMatches::Set_DistanceToNearestTrack(const DBCALShower* locBCALShower, double locDeltaPhi, double locDeltaZ)
{
	dBCALShowerDistanceToNearestTrack[locBCALShower] = pair<double, double>(locDeltaPhi, locDeltaZ);
}
inline void DDetectorMatches::Set_DistanceToNearestTrack(const DFCALShower* locFCALShower, double locDistanceToNearestTrack)
{
	dFCALShowerDistanceToNearestTrack[locFCALShower] = locDistanceToNearestTrack;
}
inline void DDetectorMatches::Set_FlightTimePCorrelation(const DTrackingData* locTrack, DetectorSystem_t locDetectorSystem, double locCorrelation)
{
	dFlightTimePCorrelations[locTrack][locDetectorSystem] = locCorrelation;
}

#endif // _DDetectorMatches_
