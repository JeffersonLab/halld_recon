// $Id$
//
//    File: JEventProcessor_FCAL_Pi0TOF.h
// Created: Wed Aug 30 16:23:49 EDT 2017
// Creator: mstaib (on Linux egbert 2.6.32-696.10.1.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_FCAL_Pi0TOF_
#define _JEventProcessor_FCAL_Pi0TOF_
#include <JANA/JEventProcessor.h>
#include <JANA/Compatibility/JLockService.h>
#include <PID/DNeutralParticle.h>
#include <FCAL/DFCALShower.h>
#include <FCAL/DFCALCluster.h>
#include <HDGEOMETRY/DGeometry.h>
#include <FCAL/DFCALGeometry.h>
#include <TOF/DTOFPoint.h>
#include "units.h"
#include "DLorentzVector.h"
#include "DVector3.h"
#include "HistogramTools.h"
#include "TProfile.h"
#include "DVector3.h"
#include "PID/DVertex.h"

#include "PID/DBeamPhoton.h"
#include "PID/DEventRFBunch.h"
#include "TRIGGER/DL1Trigger.h"

using namespace std;

class JEventProcessor_FCAL_Pi0TOF:public JEventProcessor{
	public:
		JEventProcessor_FCAL_Pi0TOF();
		~JEventProcessor_FCAL_Pi0TOF();

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;

		std::shared_ptr<JLockService> lockService;
		
		//vector<const DTOFPoint*> tof_points;
		double bar2x( int bar );
		int TOF_Match(double kinVertexX, double kinVertexY, double kinVertexZ, double x, double y, double z);
                double m_beamSpotX;
                double m_beamSpotY;
                double m_targetZ;

		int DO_METHOD;
		int USE_TRACKS;
		double m_time_FCALRF_cut;
		double m_time_FCALFCAL_cut;
		const double frac_thres_1_to_5 = 0.5;
		const double frac_thres_6_to_23 = 0.1;
  


      TProfile *hCurrentGainConstants;
};

#endif // _JEventProcessor_FCAL_Pi0TOF_

