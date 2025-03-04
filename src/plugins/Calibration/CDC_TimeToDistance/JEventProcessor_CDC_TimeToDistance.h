// $Id$
//
//    File: JEventProcessor_CDC_TimeToDistance.h
// Created: Mon Nov  9 12:37:01 EST 2015
// Creator: mstaib (on Linux ifarm1102 2.6.32-431.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_CDC_TimeToDistance_
#define _JEventProcessor_CDC_TimeToDistance_

#include <JANA/JEventProcessor.h>
#include <JANA/Services/JLockService.h>
#include "TProfile2D.h"
#include "TProfile.h"
#include "TH1F.h"
#include "TH2F.h"
#include "HDGEOMETRY/DMagneticFieldMap.h"

using std::vector;

class JEventProcessor_CDC_TimeToDistance:public JEventProcessor{
	public:
		JEventProcessor_CDC_TimeToDistance();
		~JEventProcessor_CDC_TimeToDistance();

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;

		std::shared_ptr<JLockService> lockService;

        vector<vector<double> >max_sag;
        vector<vector<double> >sag_phi_offset;
        
        const DMagneticFieldMap* dMagneticField;
        int UNBIASED_RING;
        double MIN_FOM;

        TProfile *HistCurrentConstants;

        TH1F *dHistZ2tracks0001;
        TH1F *dHistZ2tracks001;
        TH1F *dHistZ2tracks01;

        TH2F *dHistResidualVslogFOM;
        TH2F *dHistResidualVsFOM;
        TH2F *dHistEarlyDriftTimesPerChannel;
        TH2F *dHistResidualVsDriftTime;
        TH2F *dHistResidualVsDriftTimeFOM09;
        TH2F *dHistResidualVsDriftTimeFOM06;
        TH2F *dHistResidualVsDriftTimeFOM01;
        TH2F *dHistResidualVsDriftTimeFOM001;
        TH2F *dHistResidualVsDriftTimeStraightStraws;

        TH1F *dHistBz; 
   
        TProfile2D *dHistPredictedDistanceVsDeltaVsDrift;
        TProfile2D *dHistPredictedDistanceVsDeltaVsDriftFOM09;
        TProfile2D *dHistPredictedDistanceVsDeltaVsDriftFOM06;
        TProfile2D *dHistPredictedDistanceVsDeltaVsDriftFOM01;
        TProfile2D *dHistPredictedDistanceVsDeltaVsDriftFOM001;
        TProfile2D *dHistPredictedDistanceVsDeltaVsDrift05;
        TProfile2D *dHistPredictedDistanceVsDeltaVsDrift10;
        TProfile2D *dHistPredictedDistanceVsDeltaVsDrift15;
        TProfile2D *dHistPredictedDistanceVsDeltaVsDrift20;
        TProfile2D *dHistPredictedDistanceVsDeltaVsDrift25;
        TProfile2D *dHistPredictedDistanceVsDeltaVsDriftBz18;

};

#endif // _JEventProcessor_CDC_TimeToDistance_

