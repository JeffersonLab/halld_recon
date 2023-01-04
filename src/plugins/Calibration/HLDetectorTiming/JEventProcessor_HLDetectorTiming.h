// $Id$
//
//    File: JEventProcessor_HLDetectorTiming.h
// Created: Mon Jan 12 14:37:56 EST 2015
// Creator: mstaib (on Linux egbert 2.6.32-431.20.3.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_HLDetectorTiming_
#define _JEventProcessor_HLDetectorTiming_

#include <DAQ/DEPICSvalue.h>
#include <JANA/JEventProcessor.h>
#include <BCAL/DBCALHit.h>
#include <BCAL/DBCALTDCHit.h>
#include <BCAL/DBCALUnifiedHit.h>
#include <CDC/DCDCHit.h>
#include <FCAL/DFCALHit.h>
#include <FDC/DFDCHit.h>
#include <TOF/DTOFHit.h>
#include <TOF/DTOFPoint.h>
#include <START_COUNTER/DSCHit.h>
#include <TAGGER/DTAGHHit.h>
#include <TAGGER/DTAGMHit.h>
#include <RF/DRFTDCDigiTime.h>
#include <RF/DRFTime_factory.h>
#include <PID/DEventRFBunch.h>
#include <PID/DParticleID.h>
#include <TRACKING/DTrackFitter.h>
#include <TPOL/DTPOLHit.h>

#include "DFactoryGenerator_p2pi.h"

#include "TFitResult.h"
#include "TF1.h"
#include "TH1D.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TObjArray.h"
#include "TMath.h"

#include <DIRC/DDIRCGeometry.h>
#include <DIRC/DDIRCLut.h>

#include <functional>
#include <map>

using std::function;

//#include "HistogramTools.h"

//class JEventProcessor_HLDetectorTiming:public jana::JEventProcessor, public HistogramTools{
class JEventProcessor_HLDetectorTiming:public jana::JEventProcessor{
    public:
		JEventProcessor_HLDetectorTiming();
		~JEventProcessor_HLDetectorTiming();
		const char* className(void){return "JEventProcessor_HLDetectorTiming";}

	private:
		jerror_t init(void);						///< Called once at program start.
		jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
		jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
		jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
		jerror_t fini(void);						///< Called after last event of last event source has been processed.

        //HistogramTools *histoTools;     
        
        void CreateHistograms(string dirname);
        
        void DoRoughTiming();
        void DoTDCADCAlign();
        void DoTrackBased();

        int GetCCDBIndexTOF(const DTOFHit *);
        int GetCCDBIndexBCAL(const DBCALHit *); ///< Not implimented
        int GetCCDBIndexTAGM(const DTAGMHit *);
        int GetCCDBIndexCDC(const DCDCHit *);
        int GetCCDBIndexCDC(int, int);
        
        map< string, function<bool(jana::JEventLoop *eventLoop)> > dCutFunctions;
        
        double BEAM_CURRENT;
        double Z_TARGET;
        int DO_ROUGH_TIMING, DO_TDC_ADC_ALIGN, DO_TRACK_BASED, DO_VERIFY, REQUIRE_BEAM, BEAM_EVENTS_TO_KEEP, DO_CDC_TIMING, DO_OPTIONAL, DO_FITS, DO_REACTION, USE_RF_BUNCH;
		int DO_HIGH_RESOLUTION;
		unsigned int TRIGGER_MASK;
        int fBeamEventCounter;
        int dMaxDIRCChannels;
		bool NO_TRACKS;
		bool NO_FIELD;
		bool CCAL_CALIB;
		bool STRAIGHT_TRACK;
		bool NO_START_COUNTER;
		bool INCLUDE_ALL_TRIGGERS;
		bool INCLUDE_PS_TRIGGERS;
		bool PRIMEX_TRIGGERS;
		bool CPP_TRIGGERS;
		
        // The final setup requires some shifts relative to the previous values, need to store them

        int NBINS_TDIFF, NBINS_TAGGER_TIME, NBINS_MATCHING, NBINS_RF_COMPARE;
        float MIN_TDIFF, MAX_TDIFF;
        float MIN_TAGGER_TIME, MAX_TAGGER_TIME;
        float MIN_MATCHING_T, MAX_MATCHING_T;
        float MIN_RF_COMPARE, MAX_RF_COMPARE;
        double fcal_t_base, bcal_t_base, tof_t_base_fadc, tof_t_base_tdc, cdc_t_base;
        double tagm_fadc_time_offsets[103], tagm_tdc_time_offsets[103];
        double tagh_fadc_time_offsets[275], tagh_tdc_time_offsets[275];
        vector<double> sc_tdc_time_offsets;
        vector<double> tof_tdc_time_offsets;

		// constants for FCAL/TOF matching
		const double TOF_X_MEAN  =  0.75;  const double TOF_X_SIG  =  1.75;
		const double TOF_Y_MEAN  = -0.50;  const double TOF_Y_SIG  =  1.75;

		// histograms
		TH1F *dHistBeamCurrent;
		TH1F *dHistBeamEvents;
		
		map<string, TH1F*> dCDCHitTimes;
		map<string, TH2F*> dCDCHitTimesPerStraw;

		map<string, TH1F*> dFDCWireHitTimes;
		map<string, TH2F*> dFDCWireModuleHitTimes;
		map<string, TH1F*> dFDCCathodeHitTimes;
		
		map<string, TH1F*> dSCHitTimes;
		map<string, TH1F*> dSCADCHitTimes;
		map<string, TH1F*> dSCTDCHitTimes;
		map<string, TH1F*> dSCMatchedHitTimes;
		map<string, TH2F*> dSCADCTDCHitTimes;
		map<string, TH2F*> dSCMatchedHitTimesPerSector;

		map<string, TH1F*> dTOFHitTimes;
		map<string, TH1F*> dTOFADCHitTimes;
		map<string, TH1F*> dTOFTDCHitTimes;
		map<string, TH1F*> dTOFMatchedHitTimes;
		map<string, TH2F*> dTOFADCTDCHitTimes;
		map<string, TH2F*> dTOFMatchedHitTimesPerSector;

		map<string, TH1F*> dBCALADCHitTimes;
		map<string, TH2F*> dBCALADCHitTimesUpstream;
		map<string, TH2F*> dBCALADCHitTimesDownstream;
		map<string, TH1F*> dBCALTDCHitTimes;
		map<string, TH2F*> dBCALTDCHitTimesUpstream;
		map<string, TH2F*> dBCALTDCHitTimesDownstream;
		map<string, TH2F*> dBCALADCTDCHitTimesUpstream;
		map<string, TH2F*> dBCALADCTDCHitTimesDownstream;

		map<string, TH1F*> dFCALHitTimes;
		map<string, TH2F*> dFCALHitOccupancy;
		map<string, TH2F*> dFCALHitLocalTimes;
		map<string, TH2F*> dFCALHitTimesPerChannel;
		map<string, TH1F*> dFCALTotalEnergy;

		map<string, TH1F*> dCCALHitTimes;
		map<string, TH2F*> dCCALHitOccupancy;
		map<string, TH2F*> dCCALHitLocalTimes;
		map<string, TH2F*> dCCALHitTimesPerChannel;

		map<string, TH1F*> dDIRCHitTimes;
		map<string, TH2F*> dDIRCHitTimesPerChannelNorth;
		map<string, TH2F*> dDIRCHitTimesPerChannelSouth;
		map<string, TH2F*> dDIRCDeltaTimePerChannelNorth;
		map<string, TH2F*> dDIRCDeltaTimePerChannelSouth;

		map<string, TH1F*> dTPOLHitTimes;
		map<string, TH2F*> dTPOLHitTimesPerSector;

		map<string, TH1F*> dFMWPCHitTimes;
		map<string, TH2F*> dFMWPCHitTimesPerLayer;

		map<string, TH1F*> dCTOFHitTimes;
		map<string, TH2F*> dCTOFHitTimesPerLayer;

		map<string, TH1F*> dPSHitTimes;
		map<string, TH2F*> dPSHitTimesPerColumn;

		map<string, TH1F*> dPSCHitTimes;
		map<string, TH1F*> dPSCADCHitTimes;
		map<string, TH1F*> dPSCTDCHitTimes;
		map<string, TH1F*> dPSCMatchedHitTimes;
		map<string, TH2F*> dPSCADCTDCHitTimes;
		map<string, TH2F*> dPSCMatchedHitTimesPerSector;

		map<string, TH1F*> dTAGHHitTimes;
		map<string, TH1F*> dTAGHADCHitTimes;
		map<string, TH1F*> dTAGHTDCHitTimes;
		map<string, TH1F*> dTAGHMatchedHitTimes;
		map<string, TH2F*> dTAGHADCTDCHitTimes;
		map<string, TH2F*> dTAGHMatchedHitTimesPerSector;

		map<string, TH1F*> dTAGMHitTimes;
		map<string, TH1F*> dTAGMADCHitTimes;
		map<string, TH1F*> dTAGMTDCHitTimes;
		map<string, TH1F*> dTAGMMatchedHitTimes;
		map<string, TH2F*> dTAGMADCTDCHitTimes;
		map<string, TH2F*> dTAGMMatchedHitTimesPerSector;

		map<string, TH1F*> dTaggerRFTime;
		map<string, TH1F*> dTAGHRFTime;
		map<string, TH1F*> dTAGMRFTime;
		map<string, TH2F*> dTaggerRFEnergyTime;
		map<string, TH2F*> dTAGHRFCounterTime;
		map<string, TH2F*> dTAGMRFCounterTime;

		map<string, TH1F*> dTaggerSCTime;
		map<string, TH2F*> dTaggerSCEnergyTime;
		map<string, TH2F*> dTAGHSCCounterTime;
		map<string, TH2F*> dTAGMSCCounterTime;

		map<string, TH1F*> dSCRFTime;
		map<string, TH1F*> dSCRFTime_AllHits;
		map<string, TH2F*> dSCRFTimeVsSector;
		map<string, TH1F*> dCDCSCTime;
		map<string, TH1F*> dTOFRFTime;
		map<string, TH1F*> dTOFSCTime;
		map<string, TH1F*> dEarliestCDCTime;
		map<string, TH1F*> dEarliestFDCTime;
		map<string, TH1F*> dBCALShowerRFTime;
		map<string, TH1F*> dBCALShowerSCTime;
		map<string, TH2F*> dBCALShowerSCTimeVsCorrection;
		map<string, TH1F*> dFCALShowerRFTime;
		map<string, TH1F*> dFCALShowerSCTime;

		
		map<string, TH1F*> dBCALShowerRFTime_NoTracks;
		map<string, TH2F*> dBCALShowerRFTimeVsEnergy_NoTracks;
		map<string, TH1F*> dFCALShowerRFTime_NoTracks;
		map<string, TH2F*> dFCALShowerRFTimeVsEnergy_NoTracks;
		map<string, TH1F*> dCCALShowerRFTime_NoTracks;
		map<string, TH2F*> dCCALShowerRFTimeVsEnergy_NoTracks;
		map<string, TH1F*> dTOFShowerRFTime_NoTracks;

		map<string, vector<TH1F*> > dTAGHADCRFCompareTimes;
		map<string, vector<TH1F*> > dTAGHTDCRFCompareTimes;
		map<string, vector<TH1F*> > dTAGMADCRFCompareTimes;
		map<string, vector<TH1F*> > dTAGMTDCRFCompareTimes;
		map<string, vector<TH1F*> > dSCTargetRFCompareTimes;

};

#endif // _JEventProcessor_HLDetectorTiming_

