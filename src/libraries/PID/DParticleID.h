// $Id$
//
//    File: DParticleID.h
// Created: Mon Feb 28 13:47:49 EST 2011
// Creator: staylor (on Linux ifarml1 2.6.18-128.el5 x86_64)
//

#ifndef _DParticleID_
#define _DParticleID_

#include <deque>
#include <map>
#include <memory>
#include <limits>
#include <cmath>
#include <algorithm>

#include <JANA/JObject.h>

#include <DVector3.h>
#include "HDGEOMETRY/DRootGeom.h"
#include <CDC/DCDCTrackHit.h>
#include <FDC/DFDCPseudo.h>
#include <BCAL/DBCALShower.h>
#include <BCAL/DBCALCluster.h>
#include <FCAL/DFCALShower.h>
#include <FCAL/DFCALCluster.h>
#include <FCAL/DFCALHit.h>
#include <FCAL/DFCALGeometry_factory.h>
#include <FMWPC/DCTOFPoint.h>
#include <TOF/DTOFPoint.h>
#include <TOF/DTOFPaddleHit.h>
#include <TOF/DTOFGeometry.h>
#include <TOF/DTOFPoint_factory.h>
#include <START_COUNTER/DSCHit.h>
#include <TRACKING/DTrackFitter.h>
#include <TRACKING/DTrackFinder.h>
#include <TRACKING/DReferenceTrajectory.h>
#include <particleType.h>
#include <PID/DChargedTrackHypothesis.h>
#include <PID/DNeutralParticleHypothesis.h>
#include <PID/DEventRFBunch.h>
#include <DIRC/DDIRCLut.h>
#include <DIRC/DDIRCTruthBarHit.h>
#include <DIRC/DDIRCPmtHit.h>
#include <TRACKING/DMagneticFieldStepper.h>
#include <TRACKING/DTrackWireBased.h>
#include <TRACKING/DTrackCandidate.h>

#include <TROOT.h>
#include <TFile.h>
#include <TTree.h>
#include <TH1.h>

#include <TMath.h>

#include <mutex>

class DTrackTimeBased;

class DParticleID: public JObject
{
	public:
		JOBJECT_PUBLIC(DParticleID);

		DParticleID(const std::shared_ptr<const JEvent> &event);
		virtual ~DParticleID() = default;

		class dedx_t
		{
			public:
		dedx_t(double dE,double dE_amp,double dx, double p):dE(dE),dE_amp(dE_amp),dx(dx),p(p){dEdx = dE/dx; dEdx_amp=dE_amp/dx;}
				double dE; // energy loss in layer
				double dE_amp;
				double dx; // path length in layer
				double dEdx; // ratio dE/dx
				double dEdx_amp;
				double p;  // momentum at this dE/dx measurement
		};

		virtual jerror_t CalcDCdEdxChiSq(DChargedTrackHypothesis *locChargedTrackHypothesis) const = 0;

		jerror_t GetDCdEdxHits(const DTrackTimeBased *track, vector<dedx_t>& dEdxHits_CDC,vector<dedx_t>& dEdxHits_FDC) const;
		jerror_t CalcDCdEdx(const DTrackTimeBased *locTrackTimeBased, double& locdEdx_FDC, double& locdx_FDC, double& locdEdx_CDC, double& locdEdx_CDC_amp, double& locdx_CDC, double& locdx_CDC_amp, unsigned int& locNumHitsUsedFordEdx_FDC, unsigned int& locNumHitsUsedFordEdx_CDC) const;
		jerror_t CalcDCdEdx(const DTrackTimeBased *locTrackTimeBased, const vector<dedx_t>& locdEdxHits_CDC, const vector<dedx_t>& locdEdxHits_FDC, double& locdEdx_FDC, double& locdx_FDC, double& locdEdx_CDC, double& locdEdx_CDC_amp,double& locdx_CDC, double& locdx_CDC_amp, unsigned int& locNumHitsUsedFordEdx_FDC, unsigned int& locNumHitsUsedFordEdx_CDC) const;

		jerror_t CalcdEdxHit(const DVector3 &mom, const DVector3 &pos, const DCDCTrackHit *hit, dedx_t &dedx) const;
		double CalcdXHit(const DVector3 &mom,const DVector3 &pos,
				 const DCoordinateSystem *wire) const;
		jerror_t GroupTracks(vector<const DTrackTimeBased *> &tracks, vector<vector<const DTrackTimeBased*> >&grouped_tracks) const;

		void GetScintMPdEandSigma(double p,double M,double x,double &most_prob_dE, double &sigma_dE) const;
		double GetMostProbabledEdx_DC(double p,double mass,double dx, bool locIsCDCFlag) const; //bool is false for FDC
		double GetdEdxSigma_DC(double num_hits,double p,double mass, double mean_path_length, bool locIsCDCFlag) const; //bool is false for FDC

		virtual jerror_t GetdEdxMean_CDC(double locBeta, unsigned int locNumHitsUsedFordEdx, double& locMeandEdx, Particle_t locPIDHypothesis) const=0;
		virtual jerror_t GetdEdxSigma_CDC(double locBeta, unsigned int locNumHitsUsedFordEdx, double& locSigmadEdx, Particle_t locPIDHypothesis) const=0;
		virtual jerror_t GetdEdxMean_FDC(double locBeta, unsigned int locNumHitsUsedFordEdx, double& locMeandEdx, Particle_t locPIDHypothesis) const=0;
		virtual jerror_t GetdEdxSigma_FDC(double locBeta, unsigned int locNumHitsUsedFordEdx, double& locSigmadEdx, Particle_t locPIDHypothesis) const=0;
		virtual double GetProtondEdxMean_TOF(double locBeta) const=0;
		virtual double GetProtondEdxSigma_TOF(double locBeta) const=0;
		virtual double GetProtondEdxMean_SC(double locBeta) const=0;
		virtual double GetProtondEdxSigma_SC(double locBeta) const=0;
		virtual double GetTimeVariance(DetectorSystem_t detector,
					       Particle_t particle,
					       double p) const=0;
		virtual double GetTimeMean(DetectorSystem_t detector,
					       Particle_t particle,
					       double p) const=0;
		virtual double GetEOverPMean(DetectorSystem_t detector,
					     double p) const=0;
		virtual double GetEOverPSigma(DetectorSystem_t detector,
					      double p) const=0;

		/****************************************************** DISTANCE TO TRACK ******************************************************/

		// NOTE: For these functions, an initial guess for start time is expected as input so that out-of-time tracks can be skipped
		bool Distance_ToTrack(const DReferenceTrajectory* rt, const DFCALShower* locFCALShower, double locInputStartTime, shared_ptr<DFCALShowerMatchParams>& locShowerMatchParams, DVector3* locOutputProjPos = nullptr, DVector3* locOutputProjMom = nullptr) const;
		bool Distance_ToTrack(const DReferenceTrajectory* rt, const DBCALShower* locBCALShower, double locInputStartTime, shared_ptr<DBCALShowerMatchParams>& locShowerMatchParams, DVector3* locOutputProjPos = nullptr, DVector3* locOutputProjMom = nullptr) const;
		bool Distance_ToTrack(const DReferenceTrajectory* rt, const DTOFPoint* locTOFPoint, double locInputStartTime, shared_ptr<DTOFHitMatchParams>& locTOFHitMatchParams, DVector3* locOutputProjPos = nullptr, DVector3* locOutputProjMom = nullptr) const;
		bool Distance_ToTrack(const DReferenceTrajectory* rt, const DSCHit* locSCHit, double locInputStartTime, shared_ptr<DSCHitMatchParams>& locSCHitMatchParams, DVector3* locOutputProjPos = nullptr, DVector3* locOutputProjMom = nullptr) const;
		bool ProjectTo_SC(const DReferenceTrajectory* rt, unsigned int locSCSector, double& locDeltaPhi, DVector3& locProjPos, DVector3& locProjMom, DVector3& locPaddleNorm, double& locPathLength, double& locFlightTime, double& locFlightTimeVariance, int& locSCPlane) const;

		double Distance_ToTrack(const DFCALShower *locFCALShower,
					const DVector3 &locProjPos) const;
		double Distance_ToTrack(const DFCALHit *locFCALHit,
					const DVector3 &locProjPos) const;
		bool Distance_ToTrack(const vector<DTrackFitter::Extrapolation_t> &extrapolations, const DFCALShower* locFCALShower, double locInputStartTime, shared_ptr<DFCALShowerMatchParams>& locShowerMatchParams, DVector3* locOutputProjPos=nullptr, DVector3* locOutputProjMom=nullptr) const;
		bool Distance_ToTrack(const vector<DTrackFitter::Extrapolation_t>&extrapolations, const DTOFPoint* locTOFPoint, double locInputStartTime,shared_ptr<DTOFHitMatchParams>& locTOFHitMatchParams, DVector3* locOutputProjPos=nullptr, DVector3* locOutputProjMom=nullptr) const;
		bool Distance_ToTrack(const vector<DTrackFitter::Extrapolation_t>&extrapolations, const DCTOFPoint* locCTOFPoint, double locInputStartTime,shared_ptr<DCTOFHitMatchParams>& locCTOFHitMatchParams, DVector3* locOutputProjPos=nullptr, DVector3* locOutputProjMom=nullptr) const;
		bool Distance_ToTrack(const vector<DTrackFitter::Extrapolation_t> &extrapolations, const DSCHit* locSCHit, double locInputStartTime,shared_ptr<DSCHitMatchParams>& locSCHitMatchParams, DVector3* locOutputProjPos=nullptr, DVector3* locOutputProjMom=nullptr) const;
		bool Distance_ToTrack(const vector<DTrackFitter::Extrapolation_t> &extrapolations, const DBCALShower* locBCALShower, double locInputStartTime,shared_ptr<DBCALShowerMatchParams>& locShowerMatchParams, DVector3* locOutputProjPos=nullptr, DVector3* locOutputProjMom=nullptr) const;
		bool Distance_ToTrack(double locStartTime,const DTrackFitter::Extrapolation_t &extrapolation,const DFCALHit *locFCALHit,double &locDOCA,double &locHitTime) const;

		/********************************************************** CUT MATCH DISTANCE **********************************************************/

		// NOTE: For these functions, an initial guess for start time is expected as input so that out-of-time tracks can be skipped
		bool Cut_MatchDistance(const DReferenceTrajectory* rt, const DBCALShower* locBCALShower, double locInputStartTime, shared_ptr<DBCALShowerMatchParams>& locShowerMatchParams, DVector3 *locOutputProjPos = nullptr, DVector3 *locOutputProjMom = nullptr) const;
		bool Cut_MatchDistance(const DReferenceTrajectory* rt, const DTOFPoint* locTOFPoint, double locInputStartTime, shared_ptr<DTOFHitMatchParams>& locTOFHitMatchParams, DVector3 *locOutputProjPos = nullptr, DVector3 *locOutputProjMom = nullptr) const;
		bool Cut_MatchDistance(const DReferenceTrajectory* rt, const DSCHit* locSCHit, double locInputStartTime, shared_ptr<DSCHitMatchParams>& locSCHitMatchParams, bool locIsTimeBased, DVector3 *locOutputProjPos = nullptr, DVector3 *locOutputProjMom = nullptr) const;
		bool Cut_MatchDistance(const DReferenceTrajectory* rt, const DFCALShower* locFCALShower, double locInputStartTime, shared_ptr<DFCALShowerMatchParams>& locShowerMatchParams, DVector3 *locOutputProjPos = nullptr, DVector3 *locOutputProjMom = nullptr) const;

		bool Cut_MatchDistance(const vector<DTrackFitter::Extrapolation_t> &extrapolations, const DBCALShower* locBCALShower, double locInputStartTime,shared_ptr<DBCALShowerMatchParams>& locShowerMatchParams, DVector3 *locOutputProjPos=nullptr, DVector3 *locOutputProjMom=nullptr) const;
		bool Cut_MatchDistance(const vector<DTrackFitter::Extrapolation_t> &extrapolations, const DFCALShower* locFCALShower, double locInputStartTime,shared_ptr<DFCALShowerMatchParams>& locShowerMatchParams, DVector3 *locOutputProjPos=nullptr, DVector3 *locOutputProjMom=nullptr) const;
		bool Cut_MatchDistance(const vector<DTrackFitter::Extrapolation_t> &extrapolations, const DTOFPoint* locTOFPoint, double locInputStartTime,shared_ptr<DTOFHitMatchParams>& locTOFHitMatchParams, DVector3 *locOutputProjPos=nullptr, DVector3 *locOutputProjMom=nullptr) const;	
		bool Cut_MatchDistance(const vector<DTrackFitter::Extrapolation_t> &extrapolations, const DCTOFPoint* locCTOFPoint, double locInputStartTime,shared_ptr<DCTOFHitMatchParams>& locCTOFHitMatchParams, DVector3 *locOutputProjPos=nullptr, DVector3 *locOutputProjMom=nullptr) const;
		bool Cut_MatchDistance(const vector<DTrackFitter::Extrapolation_t> &extrapolations, const DSCHit* locSCHit, double locInputStartTime,shared_ptr<DSCHitMatchParams>& locSCHitMatchParams, bool locIsTimeBased, DVector3 *locOutputProjPos=nullptr, DVector3 *locOutputProjMom=nullptr) const;
		bool Cut_MatchDIRC(const vector<DTrackFitter::Extrapolation_t> &extrapolations, const vector<const DDIRCPmtHit*> locDIRCHits, double locInputStartTime, Particle_t locPID, shared_ptr<DDIRCMatchParams>& locDIRCMatchParams, const vector<const DDIRCTruthBarHit*> locDIRCBarHits, map<shared_ptr<const DDIRCMatchParams>, vector<const DDIRCPmtHit*> >& locDIRCTrackMatchParams, DVector3 *locOutputProjPos=nullptr, DVector3 *locOutputProjMom=nullptr) const;

		/********************************************************** GET BEST MATCH **********************************************************/

		// Wrappers
		bool Get_BestBCALMatchParams(const DTrackingData* locTrack, const DDetectorMatches* locDetectorMatches, shared_ptr<const DBCALShowerMatchParams>& locBestMatchParams) const;
		bool Get_BestSCMatchParams(const DTrackingData* locTrack, const DDetectorMatches* locDetectorMatches, shared_ptr<const DSCHitMatchParams>& locBestMatchParams) const;
		bool Get_BestTOFMatchParams(const DTrackingData* locTrack, const DDetectorMatches* locDetectorMatches, shared_ptr<const DTOFHitMatchParams>& locBestMatchParams) const;	
		bool Get_BestCTOFMatchParams(const DTrackingData* locTrack, const DDetectorMatches* locDetectorMatches, shared_ptr<const DCTOFHitMatchParams>& locBestMatchParams) const;
		bool Get_BestFCALMatchParams(const DTrackingData* locTrack, const DDetectorMatches* locDetectorMatches, shared_ptr<const DFCALShowerMatchParams>& locBestMatchParams) const;
		bool Get_BestFCALSingleHitMatchParams(const DTrackingData* locTrack, const DDetectorMatches* locDetectorMatches, shared_ptr<const DFCALSingleHitMatchParams>& locBestMatchParams) const;
		bool Get_DIRCMatchParams(const DTrackingData* locTrack, const DDetectorMatches* locDetectorMatches, shared_ptr<const DDIRCMatchParams>& locBestMatchParams) const;

		// Actual
		shared_ptr<const DBCALShowerMatchParams> Get_BestBCALMatchParams(DVector3 locMomentum, vector<shared_ptr<const DBCALShowerMatchParams> >& locShowerMatchParams) const;
		shared_ptr<const DSCHitMatchParams> Get_BestSCMatchParams(vector<shared_ptr<const DSCHitMatchParams> >& locSCHitMatchParams) const;
		shared_ptr<const DTOFHitMatchParams> Get_BestTOFMatchParams(vector<shared_ptr<const DTOFHitMatchParams> >& locTOFHitMatchParams) const;
		shared_ptr<const DCTOFHitMatchParams> Get_BestCTOFMatchParams(vector<shared_ptr<const DCTOFHitMatchParams> >& locCTOFHitMatchParams) const;
		shared_ptr<const DFCALShowerMatchParams> Get_BestFCALMatchParams(vector<shared_ptr<const DFCALShowerMatchParams> >& locShowerMatchParams) const;
		shared_ptr<const DFCALSingleHitMatchParams> Get_BestFCALSingleHitMatchParams(vector<shared_ptr<const DFCALSingleHitMatchParams> >& locMatchParams) const;

		/********************************************************** GET CLOSEST TO TRACK **********************************************************/

		// NOTE: an initial guess for start time is expected as input so that out-of-time hits can be skipped
		bool Get_ClosestToTrack(const DReferenceTrajectory* rt, const vector<const DBCALShower*>& locBCALShowers, bool locCutFlag, double& locStartTime, shared_ptr<const DBCALShowerMatchParams>& locBestMatchParams, double* locStartTimeVariance = nullptr, DVector3* locBestProjPos = nullptr, DVector3* locBestProjMom = nullptr) const;
		bool Get_ClosestToTrack(const DReferenceTrajectory* rt, const vector<const DTOFPoint*>& locTOFPoints, bool locCutFlag, double& locStartTime, shared_ptr<const DTOFHitMatchParams>& locBestMatchParams, double* locStartTimeVariance = nullptr, DVector3* locBestProjPos = nullptr, DVector3* locBestProjMom = nullptr) const;
		bool Get_ClosestToTrack(const DReferenceTrajectory* rt, const vector<const DFCALShower*>& locFCALShowers, bool locCutFlag, double& locStartTime, shared_ptr<const DFCALShowerMatchParams>& locBestMatchParams, double* locStartTimeVariance = nullptr, DVector3* locBestProjPos = nullptr, DVector3* locBestProjMom = nullptr) const;
		bool Get_ClosestToTrack(const DReferenceTrajectory* rt, const vector<const DSCHit*>& locSCHits, bool locIsTimeBased, bool locCutFlag, double& locStartTime, shared_ptr<const DSCHitMatchParams>& locBestMatchParams, double* locStartTimeVariance = nullptr, DVector3* locBestProjPos = nullptr, DVector3* locBestProjMom = nullptr) const;
		const DTOFPaddleHit* Get_ClosestTOFPaddleHit_Horizontal(const DReferenceTrajectory* locReferenceTrajectory, const vector<const DTOFPaddleHit*>& locTOFPaddleHits, double locInputStartTime, double& locBestDeltaY, double& locBestDistance) const;
		const DTOFPaddleHit* Get_ClosestTOFPaddleHit_Vertical(const DReferenceTrajectory* locReferenceTrajectory, const vector<const DTOFPaddleHit*>& locTOFPaddleHits, double locInputStartTime, double& locBestDeltaX, double& locBestDistance) const;

		// The following routines use extrapolations from the track
		bool Get_ClosestToTrack(const vector<DTrackFitter::Extrapolation_t> &extrapolations, const vector<const DBCALShower*>& locBCALShowers, bool locCutFlag, double& locStartTime,shared_ptr<const DBCALShowerMatchParams>& locBestMatchParams, double* locStartTimeVariance=nullptr, DVector3* locBestProjPos=nullptr, DVector3* locBestProjMom=nullptr) const;
		bool Get_ClosestToTrack(const vector<DTrackFitter::Extrapolation_t> &extrapolations, const vector<const DTOFPoint*>& locTOFPoints, bool locCutFlag, double& locStartTime, shared_ptr<const DTOFHitMatchParams>& locBestMatchParams, double* locStartTimeVariance=nullptr, DVector3* locBestProjPos=nullptr, DVector3* locBestProjMom=nullptr) const;
		bool Get_ClosestToTrack(const vector<DTrackFitter::Extrapolation_t> &extrapolations, const vector<const DFCALShower*>& locFCALShowers, bool locCutFlag, double& locStartTime,shared_ptr<const DFCALShowerMatchParams>& locBestMatchParams, double* locStartTimeVariance=nullptr, DVector3* locBestProjPos=nullptr, DVector3* locBestProjMom=nullptr) const;
		bool Get_ClosestToTrack(const vector<DTrackFitter::Extrapolation_t> &extrapolations, const vector<const DSCHit*>& locSCHits, bool locIsTimeBased, bool locCutFlag, double& locStartTime,shared_ptr<const DSCHitMatchParams>& locBestMatchParams, double* locStartTimeVariance=nullptr, DVector3* locBestProjPos=nullptr, DVector3* locBestProjMom=nullptr) const;
		const DTOFPaddleHit* Get_ClosestTOFPaddleHit_Horizontal(const vector<DTrackFitter::Extrapolation_t> &extrapolations, const vector<const DTOFPaddleHit*>& locTOFPaddleHits, double locInputStartTime, double& locBestDeltaY, double& locBestDistance) const;
		const DTOFPaddleHit* Get_ClosestTOFPaddleHit_Vertical(const vector<DTrackFitter::Extrapolation_t> &extrapolations, const vector<const DTOFPaddleHit*>& locTOFPaddleHits, double locInputStartTime, double& locBestDeltaX, double& locBestDistance) const;


		/********************************************************** PREDICT HIT ELEMENT **********************************************************/

		bool PredictFCALHit(const DReferenceTrajectory *rt, unsigned int &row, unsigned int &col, DVector3 *intersection = nullptr) const;
		bool PredictBCALWedge(const DReferenceTrajectory *rt, unsigned int &module,unsigned int &sector, DVector3 *intersection = nullptr) const;
		bool PredictTOFPaddles(const DReferenceTrajectory *rt, unsigned int &hbar,unsigned int &vbar, DVector3 *intersection = nullptr) const;
		unsigned int PredictSCSector(const DReferenceTrajectory* rt, DVector3* locOutputProjPos = nullptr, bool* locProjBarrelRegion = nullptr, double* locMinDPhi = nullptr) const;
		unsigned int PredictSCSector(const DReferenceTrajectory* rt, double& locDeltaPhi, DVector3& locProjPos, DVector3& locProjMom, DVector3& locPaddleNorm, double& locPathLength, double& locFlightTime, double& locFlightTimeVariance, int& locSCPlane) const;

		unsigned int PredictSCSector(const vector<DTrackFitter::Extrapolation_t> &extrapolations, double& locDeltaPhi, DVector3& locProjPos, DVector3& locProjMom, DVector3& locPaddleNorm, double& locPathLength, double& locFlightTime, double& locFlightTimeVariance, int& locSCPlane) const;
		unsigned int PredictSCSector(const vector<DTrackFitter::Extrapolation_t> &extrapolations, DVector3* locOutputProjPos=nullptr, bool* locProjBarrelRegion=nullptr, double* locMinDPhi=nullptr) const;
		bool PredictFCALHit(const vector<DTrackFitter::Extrapolation_t>&extrapolations, unsigned int &row, unsigned int &col, DVector3 *intersection=nullptr) const;
		bool PredictBCALWedge(const vector<DTrackFitter::Extrapolation_t>&extrapolations, unsigned int &module,unsigned int &sector, DVector3 *intersection=nullptr) const;
		bool PredictTOFPaddles(const vector<DTrackFitter::Extrapolation_t>&extrapolations, unsigned int &hbar,unsigned int &vbar, DVector3 *intersection=nullptr) const;

		/************** Routines to get start time for tracking *****/
		bool Get_StartTime(const vector<DTrackFitter::Extrapolation_t> &extrapolations,
				   const vector<const DFCALShower*>& FCALShowers,
				   double& StartTime) const;
		bool Get_StartTime(const vector<DTrackFitter::Extrapolation_t> &extrapolations,
				   const vector<const DFCALHit*>& FCALHits,
				   double& StartTime) const;
		bool Get_StartTime(const vector<DTrackFitter::Extrapolation_t> &extrapolations,
				   const vector<const DSCHit*>& SCHits, 
				   double& StartTime) const;
		bool Get_StartTime(const vector<DTrackFitter::Extrapolation_t> &extrapolations,
				   const vector<const DTOFPoint*>& TOFPoints, 
				   double& StartTime) const;
		bool Get_StartTime(const vector<DTrackFitter::Extrapolation_t> &extrapolations,
				   const vector<const DBCALShower*>& locBCALShowers,
				   double& StartTime) const;  
		  
		/********************************************************** MISCELLANEOUS **********************************************************/

		double Calc_BCALFlightTimePCorrelation(const DTrackingData* locTrack, DDetectorMatches* locDetectorMatches) const;
		double Calc_FCALFlightTimePCorrelation(const DTrackingData* locTrack, DDetectorMatches* locDetectorMatches) const;
		double Calc_TOFFlightTimePCorrelation(const DTrackingData* locTrack, DDetectorMatches* locDetectorMatches) const;
		double Calc_SCFlightTimePCorrelation(const DTrackingData* locTrack, const DDetectorMatches* locDetectorMatches) const;

		double Calc_PropagatedRFTime(const DKinematicData* locKinematicData, const DEventRFBunch* locEventRFBunch) const;
		double Calc_TimingChiSq(const DChargedTrackHypothesis* locChargedHypo, unsigned int &locNDF, double& locTimingPull) const;
		double Calc_TimingChiSq(const DNeutralParticleHypothesis* locNeutralHypo, unsigned int &locNDF, double& locTimingPull) const;
		void Calc_ChargedPIDFOM(DChargedTrackHypothesis* locChargedTrackHypothesis) const;

		unsigned int Get_CDCRingBitPattern(vector<const DCDCTrackHit*>& locCDCTrackHits) const;
		unsigned int Get_FDCPlaneBitPattern(vector<const DFDCPseudo*>& locFDCPseudos) const;
		void Get_CDCRings(unsigned int locBitPattern, set<int>& locCDCRings) const;
		void Get_FDCPlanes(unsigned int locBitPattern, set<int>& locFDCPlanes) const;

		void Get_CDCNumHitRingsPerSuperlayer(int locBitPattern, map<int, int>& locNumHitRingsPerSuperlayer) const;
		void Get_CDCNumHitRingsPerSuperlayer(const set<int>& locCDCRings, map<int, int>& locNumHitRingsPerSuperlayer) const;
		void Get_FDCNumHitPlanesPerPackage(int locBitPattern, map<int, int>& locNumHitPlanesPerPackage) const;
		void Get_FDCNumHitPlanesPerPackage(const set<int>& locFDCPlanes, map<int, int>& locNumHitPlanesPerPackage) const;

		double Get_CorrectedHitTime(const DTOFPoint* locTOFPoint,
					    const DVector3 &locProjPos) const;	
		double Get_CorrectedHitEnergy(const DTOFPoint* locTOFPoint,
					      const DVector3 &locProjPos) const;
		double Get_CorrectedHitEnergy(const DSCHit* locSCHit,
					      const DVector3 &locProjPos) const;
		double Get_CorrectedHitTime(const DSCHit* locSCHit,
					    const DVector3 &locProjPos) const;
		
		const DDIRCLut *Get_DIRCLut() const;
		void GetSingleFCALHits(vector<const DFCALShower*>&locFCALShowers,vector<const DFCALHit*>&locFCALHits,vector<const DFCALHit*>&singleHits) const;
	

	protected:
		// gas material properties
		double dKRhoZoverA_FDC, dRhoZoverA_FDC, dLnI_FDC;	
		double dKRhoZoverA_Scint, dRhoZoverA_Scint, dLnI_Scint;
		double dKRhoZoverA_CDC, dRhoZoverA_CDC, dLnI_CDC;
		double dDensity_FDC;
		double dDensity_CDC;
		double dA_CDC;
		double dA_FDC;

		double CTOF_MATCH_X_CUT=20.,CTOF_MATCH_Y_CUT=20.0;
		double BCAL_Z_CUT,BCAL_PHI_CUT_PAR1,BCAL_PHI_CUT_PAR2, BCAL_PHI_CUT_PAR3;
		double FCAL_CUT_PAR1,FCAL_CUT_PAR2,FCAL_CUT_PAR3;
		double TOF_CUT_PAR1, TOF_CUT_PAR2, TOF_CUT_PAR3, TOF_CUT_PAR4;
		vector<double> dSCCutPars_TimeBased, dSCCutPars_WireBased;

		double C_EFFECTIVE; // start counter light propagation speed
		double ATTEN_LENGTH; // Start counter attenuation length
		double OUT_OF_TIME_CUT; //for all matches

                vector<double> CDC_GAIN_DOCA_PARS;  // params to correct for gas deterioration spring 2018

		// Start counter resolution parameters
		vector<double> SC_BOUNDARY1, SC_BOUNDARY2, SC_BOUNDARY3;
		vector<double> SC_SECTION1_P0, SC_SECTION1_P1;
		vector<double> SC_SECTION2_P0, SC_SECTION2_P1;
		vector<double> SC_SECTION3_P0, SC_SECTION3_P1;
		vector<double> SC_SECTION4_P0, SC_SECTION4_P1;

		// define bool in case there is no Start Counter in geometry (e.g. CPP)
		bool START_EXIST = true;

		// start counter geometry parameters
		double sc_leg_tcor;
		double sc_angle_cor;
		vector<vector<DVector3> >sc_dir; // direction vector in plane of plastic
		vector<vector<DVector3> >sc_pos;
		vector<vector<DVector3> >sc_norm;
		double dSCdphi;
		double dSCphi0;
		// start counter calibration parameters
		// Propagation time (pt) parameters
		enum sc_region_t{
			SC_STRAIGHT,
			SC_BEND,
			SC_NOSE,
		};
		vector<double>sc_veff[3];
		vector<double>sc_pt_yint[3];
		vector<double>sc_pt_slope[3];
		// Attenuation (attn) calibration paramerters
		enum sc_region_attn{
			SC_STRAIGHT_ATTN,
			SC_BENDNOSE_ATTN,
		};
		vector<double> sc_attn_A[2];
		vector<double> sc_attn_B[2];
		vector<double> sc_attn_C[2];

		// FCAL geometry
		double dFCALz;
		const DFCALGeometry *dFCALGeometry;

		// FCAL calibration constants
		double dFCALTimewalkPar1,dFCALTimewalkPar2;

		// TOF calibration constants
		// used to update hit energy & time when matching to un-matched, position-ill-defined bars
		const DTOFGeometry* dTOFGeometry;
		vector<double> propagation_speed;
		//double TOF_HALFPADDLE;
		double dHalfPaddle_OneSided;
		double TOF_ATTEN_LENGTH;
		double TOF_E_THRESHOLD;
		double ONESIDED_PADDLE_MIDPOINT_MAG; //+/- this number for North/South
		// time cut for cdc hits
		double CDC_TIME_CUT_FOR_DEDX;
        
                bool CDC_TRUNCATE_DEDX;            // dE/dx truncation: ignore hits with highest dE
		bool ADD_FCAL_DATA_FOR_CPP;

		double dTargetZCenter;

		const DTrackFinder *finder;
		const DTrackFitter *fitter;
		DTOFPoint_factory* dTOFPointFactory;
		
		// DIRC LUT
		const DDIRCLut* dDIRCLut;

	private:

		int DEBUG_LEVEL;
		// Prohibit default constructor
		DParticleID();

};

#endif // _DParticleID_

