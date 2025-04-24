#ifndef _JEventProcessor_highlevel_online_
#define _JEventProcessor_highlevel_online_

#include <JANA/JEventProcessor.h>
#include <JANA/Services/JLockService.h>

#include <TDirectory.h>
#include <TH2.h>
#include <TH1.h>
#include <TMath.h>

#include <TRACKING/DTrackTimeBased.h>
#include <PID/DBeamPhoton.h>
#include <FCAL/DFCALShower.h>
#include <PID/DChargedTrack.h>
#include <BCAL/DBCALShower.h>
#include <PID/DNeutralShower.h>
#include <PID/DNeutralParticle.h>
#include <TOF/DTOFPoint.h>
#include <START_COUNTER/DSCHit.h>
#include <PID/DDetectorMatches.h>
#include <PID/DVertex.h>
#include <PID/DEventRFBunch.h>
#include <TRIGGER/DL1Trigger.h>
#include <TAGGER/DTAGHHit.h>
#include <BCAL/DBCALDigiHit.h>
#include <FCAL/DFCALDigiHit.h>
#include <CCAL/DCCALDigiHit.h>
#include <DAQ/Df250PulsePedestal.h>
#include <PAIR_SPECTROMETER/DPSPair.h>
#include <PAIR_SPECTROMETER/DPSCPair.h>

using namespace std;

class JEventProcessor_highlevel_online:public JEventProcessor
{
	public:
		JEventProcessor_highlevel_online(){
			SetTypeName("JEventProcessor_highlevel_online");
		};
		~JEventProcessor_highlevel_online(){};

		TH1D* dHist_EventInfo;

		TH1I* dHist_BeamBunchPeriod;
		TH1F* dHist_BeamBunchPeriod_DFT;

		TH2I* dHist_NumTriggers;
		TH2I* dHist_BCALVsFCAL_TrigBit1;
		TH2I* dHist_CCALVsFCAL_TrigBit1;
		TH1I* dHist_L1bits_gtp;
		TH1I* dHist_L1bits_fp;
                TH1I* dHist_L1bits_fp_twelvehundhits; // BCAL LED Pseudo Trigger(1200 hits in BCAL)

		TH2I* dHist_NumHighLevelObjects;

		TH1I* dHist_BeamEnergy;
		TH1I* dHist_PSPairEnergy;

		TH2I* dHist_PVsTheta_Tracks;
		TH2I* dHist_PhiVsTheta_Tracks;

		TH1I* dEventVertexZ;
		TH2I* dEventVertexYVsX;

		TH1I* d2gamma;
		TH1I *dpip_pim;
		TH1I *dKp_Km;
		TH1I *dpip_pim_pi0;
		TH2I *dbeta_vs_p;
		TH2I *dbeta_vs_p_BCAL;
		TH2I *dbeta_vs_p_TOF;
		TH1I *dptrans;
		TH1I *dme_rho;
		TH1I *dme_omega;

                TH2I *dHist_heli_asym_gtp;

                TH2D *dF1TDC_fADC_tdiff;
		map<pair<int,int>, double> f1tdc_bin_map; // key=<rocid,slot> val=bin
		
		template<typename T> void FillF1Hist(vector<const T*> hits);

	private:

		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& locEvent) override;
		void EndRun() override;
		void Finish() override;

		std::shared_ptr<JLockService> lockService;

		int fcal_cell_thr;
		int bcal_cell_thr;
		int ccal_cell_thr;
		int fcal_row_mask_min, fcal_row_mask_max, fcal_col_mask_min, fcal_col_mask_max;

		vector<double> dNumHadronicTriggers_CoherentPeak_RFSignal;
		vector<double> dNumHadronicTriggers_CoherentPeak_RFSideband;

		double dShowerEOverPCut;
		double dBeamBunchPeriod;
		pair<double, double> dCoherentPeakRange;
		pair<int, int> dRFSidebandBunchRange;
		map<Particle_t, map<DetectorSystem_t, double> > dTimingCutMap;
		bool isExclusive;

		double last_timestamp;
		double unix_offset;
};

#endif // _JEventProcessor_highlevel_online_

