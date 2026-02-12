// $Id$
//
//    File: JEventProcessor_FCAL_Pi0HFA.h
// Created: Wed Aug 30 16:23:49 EDT 2017
// Creator: mstaib (on Linux egbert 2.6.32-696.10.1.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_FCAL_Pi0HFA_
#define _JEventProcessor_FCAL_Pi0HFA_
#include <JANA/JApplication.h>
#include <JANA/JEventProcessor.h>
#include <PID/DNeutralParticle.h>
#include <FCAL/DFCALShower.h>
#include <FCAL/DFCALCluster.h>
#include <HDGEOMETRY/DGeometry.h>
#include <FCAL/DFCALGeometry.h>
#include "units.h"
#include "DLorentzVector.h"
#include "DVector3.h"
#include "HistogramTools.h"
#include "TProfile.h"

using namespace std;

class JEventProcessor_FCAL_Pi0HFA:public JEventProcessor{
	public:
		JEventProcessor_FCAL_Pi0HFA();
		~JEventProcessor_FCAL_Pi0HFA();

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;

    double m_targetZ;

      TProfile *hCurrentGainConstants;
};

#endif // _JEventProcessor_FCAL_Pi0HFA_

