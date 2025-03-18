// $Id$
//
//    File: JEventProcessor_CDC_PerStrawReco.h
// Created: Mon Jul  6 13:00:51 EDT 2015
// Creator: mstaib (on Linux egbert 2.6.32-504.16.2.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_CDC_PerStrawReco_
#define _JEventProcessor_CDC_PerStrawReco_

#include <JANA/JEventProcessor.h>

#include <TH1F.h>
#include <TH2F.h>
#include <JANA/Services/JLockService.h>

using std::vector;

class JEventProcessor_CDC_PerStrawReco:public JEventProcessor{
	public:
		JEventProcessor_CDC_PerStrawReco();
		~JEventProcessor_CDC_PerStrawReco();

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;

		std::shared_ptr<JLockService> lockService;

        int EXCLUDERING;
        vector<vector<double> >max_sag;
        vector<vector<double> >sag_phi_offset;
        
        vector<vector<TH1F*>> hResiduals;
		vector<vector<TH2F*>> hResidualsVsMomentum;
		vector<vector<TH2F*>> hResidualsVsTheta;
		vector<vector<TH2F*>> hResidualsVsZ;
		vector<vector<TH2F*>> hResidualsVsTrackingFOM;
		
        vector<vector<TH1F*>> hDriftTime;
        vector<vector<TH1F*>> hDriftDistance;
        vector<vector<TH1F*>> hPredictedDriftDistance;

		vector<vector<TH2F*>> hResidualsVsDriftTime;
		vector<vector<TH2F*>> hResidualsVsDriftDistance;
		vector<vector<TH2F*>> hResidualsVsPredictedDriftDistance;
		vector<vector<TH2F*>> hPredictedDriftDistanceVsDriftTime;
		
		
        vector<vector<vector<TH1F*>>> hStrawResidual;
		vector<vector<vector<TH2F*>>> hStrawDriftTimeVsPhiDOCA;
		vector<vector<vector<TH2F*>>> hStrawPredictedDistanceVsPhiDOCA;
		vector<vector<vector<TH2F*>>> hStrawResidualVsZ;

		vector<vector<vector<TH2F*>>> hStrawResidualVsDelta;
		vector<vector<vector<TH2F*>>> hStrawPredictedDriftDistanceVsDriftTime;
		vector<vector<vector<TH2F*>>> hStrawPredictedDriftDistanceVsDelta;

		vector<vector<vector<TH2F*>>> hPredictedDriftDistanceVsDriftTime_PosDelta;
		vector<vector<vector<TH2F*>>> hResidualVsDriftTime_PosDelta;
		vector<vector<vector<TH1F*>>> hResidual_PosDelta;

		vector<vector<vector<TH2F*>>> hPredictedDriftDistanceVsDriftTime_NegDelta;
		vector<vector<vector<TH2F*>>> hResidualVsDriftTime_NegDelta;
		vector<vector<vector<TH1F*>>> hResidual_NegDelta;



};

#endif // _JEventProcessor_CDC_PerStrawReco_

