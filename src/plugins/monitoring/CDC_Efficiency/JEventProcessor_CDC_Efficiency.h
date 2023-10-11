// $Id$
//
//    File: JEventProcessor_CDC_Efficiency.h
// Created: Tue Sep  9 15:41:38 EDT 2014
// Creator: hdcdcops (on Linux gluon05.jlab.org 2.6.32-358.18.1.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_CDC_Efficiency_
#define _JEventProcessor_CDC_Efficiency_

#include <set>
#include <map>
#include <vector>
#include <deque>
using namespace std;

#include <TTree.h>
#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
#include <TH3.h>
#include <TProfile.h>
#include <TMath.h>


#include <JANA/JFactoryT.h>
#include <JANA/JEventProcessor.h>
#include <JANA/Compatibility/JLockService.h>

#include <HDGEOMETRY/DGeometry.h>
#include <TRACKING/DTrackCandidate_factory_StraightLine.h>
#include <TRACKING/DReferenceTrajectory.h>
#include <TRACKING/DTrackWireBased.h>
#include <PID/DChargedTrack.h>
#include <PID/DParticleID.h>
#include <PID/DDetectorMatches.h>
#include <CDC/DCDCTrackHit.h>
#include <CDC/DCDCHit.h>

class JEventProcessor_CDC_Efficiency:public JEventProcessor{
	public:
		JEventProcessor_CDC_Efficiency();
		~JEventProcessor_CDC_Efficiency();

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;

		std::shared_ptr<JLockService> lockService;

	  //june12
		void Fill_Efficiency_Histos(unsigned int ringNum, const DTrackTimeBased *thisTimeBasedTrack, map<int, map<int, set<const DCDCTrackHit*> > >& locSorteDCDCTrackHits, const DParticleID * pid_algorithm, const DTrackFitter *fitter);
		bool Expect_Hit(const DTrackTimeBased* thisTimeBasedTrack, DCDCWire* wire, double distanceToWire, const DVector3 &pos, double& delta, double& dz);
                
		void Fill_MeasuredHit(bool withdEdx, int ringNum, int wireNum, double distanceToWire, const DVector3 &pos, const DVector3 &mom, DCDCWire* wire, const DCDCHit* locHit, const DParticleID * pid_algorithm);


		  //void GitRDun(unsigned int ringNum, const DTrackTimeBased *thisTimeBasedTrack, map<int, map<int, set<const DCDCTrackHit*> > >& locSorteDCDCTrackHits, const DParticleID * pid_algorithm, const DTrackFitter *fitter);
		  //bool Expect_Hit(const DTrackTimeBased* thisTimeBasedTrack, DCDCWire* wire, double distanceToWire, const DVector3 &pos, double& delta, double& dz);
		  //		void Fill_MeasuredHit(int ringNum, int wireNum, double distanceToWire, const DVector3 &pos, const DVector3 &mom, DCDCWire* wire, const DCDCHit* locHit, const DParticleID * pid_algorithm);

		void Fill_ExpectedHit(int ringNum, int wireNum, double distanceToWire);
		const DCDCTrackHit* Find_Hit(int locRing, int locProjectedStraw, map<int, set<const DCDCTrackHit*> >& locSorteDCDCTrackHits);
		double GetDOCAFieldOff(DVector3, DVector3, DVector3, DVector3, DVector3&, DVector3&);

		DGeometry * dgeom;
		bool dIsNoFieldFlag;
		double dTargetCenterZ;
		double dTargetLength;

		double MAX_DRIFT_TIME;
                vector <double> CDC_GAIN_DOCA_PARS;  // params to correct for gas deterioration spring 2018

		double dMinTrackingFOM;
		int dMinNumRingsToEvalSuperlayer;


		vector< vector< DCDCWire * > > cdcwires; // CDC Wires Referenced by [ring][straw]
		vector<vector<double> >max_sag;
		vector<vector<double> >sag_phi_offset;
		int ChannelFromRingStraw[28][209];
		int ROCIDFromRingStraw[28][209];
		int SlotFromRingStraw[28][209];
		double DOCACUT;
		double PCUTL, PCUTH;
                int FILL_DEDX_HISTOS;

        vector<TH2D*> cdc_measured_ring; //Filled with total actually detected before division at end
        vector<TH2D*> cdc_expected_ring; // Contains total number of expected hits by DOCA
        map<int, vector<TH2D*> > cdc_measured_ringmap; //int: DOCA bin //vector: total + rings
        map<int, vector<TH2D*> > cdc_expected_ringmap; //int: DOCA bin

        vector<TH2D*> cdc_measured_with_dedx_ring; //Filled with total actually detected before division at end
        map<int, vector<TH2D*> > cdc_measured_with_dedx_ringmap; //int: DOCA bin //vector: total + rings


		TH2I *ChargeVsTrackLength;
		TH1I * hChi2OverNDF;
		TH2I *hResVsT;

	 
		TH1F *hExpectedHitsVsPathLength;
		TH1F *hExpectedHitsVsDOCA;
		TH1F *hExpectedHitsVsTrackingFOM;
		TH1F *hExpectedHitsVsTheta;
		TH1F *hExpectedHitsVsMom;
		TH1F *hExpectedHitsVsDelta;
		TH2F *hExpectedHitsMomVsTheta;
		TH1F *hExpectedHitsVsN;
	
		TH1F *hMeasuredHitsVsPathLength;
		TH1F *hMeasuredHitsVsDOCA;
		TH1F *hMeasuredHitsVsTrackingFOM;
		TH1F *hMeasuredHitsVsTheta;
		TH1F *hMeasuredHitsVsMom;
		TH1F *hMeasuredHitsVsDelta;
		TH2F *hMeasuredHitsMomVsTheta;
		TH1F *hMeasuredHitsVsN;
	
		TH1F *hMeasuredHitsWithDEDxVsPathLength;
		TH1F *hMeasuredHitsWithDEDxVsDOCA;
		TH1F *hMeasuredHitsWithDEDxVsTrackingFOM;
		TH1F *hMeasuredHitsWithDEDxVsTheta;
		TH1F *hMeasuredHitsWithDEDxVsMom;
		TH1F *hMeasuredHitsWithDEDxVsDelta;
		TH2F *hMeasuredHitsWithDEDxMomVsTheta;
		TH1F *hMeasuredHitsWithDEDxVsN;
	
		TProfile *hEfficiencyVsPathLength;
		TProfile *hEfficiencyVsDOCA;
		TProfile *hEfficiencyVsTrackingFOM;
		TProfile *hEfficiencyVsTheta;
		TProfile *hEfficiencyVsMom;
		TProfile *hEfficiencyVsDelta;
		TProfile *hEfficiencyVsN;
	
		TProfile *hEfficiencyWithDEDxVsPathLength;
		TProfile *hEfficiencyWithDEDxVsDOCA;
		TProfile *hEfficiencyWithDEDxVsTrackingFOM;
		TProfile *hEfficiencyWithDEDxVsTheta;
		TProfile *hEfficiencyWithDEDxVsMom;
		TProfile *hEfficiencyWithDEDxVsDelta;
		TProfile *hEfficiencyWithDEDxVsN;
	
		TProfile *hEfficiencyVsChannel;
		map<int, TProfile *> hEfficiencyVsSlotROC;
		map<int, TProfile *> hEfficiencyVsChannelROC;
	
		TProfile *hEfficiencyWithDEDxVsChannel;
		map<int, TProfile *> hEfficiencyWithDEDxVsSlotROC;
		map<int, TProfile *> hEfficiencyWithDEDxVsChannelROC;
	
		TProfile2D *hEfficiencyMomVsTheta;
		TProfile2D *hEfficiencyDistanceVsDelta;
		TProfile2D *hEfficiencyZVsDelta;

		TProfile2D *hEfficiencyWithDEDxMomVsTheta;
		TProfile2D *hEfficiencyWithDEDxDistanceVsDelta;
		TProfile2D *hEfficiencyWithDEDxZVsDelta;


};

#endif // _JEventProcessor_CDC_Efficiency_

