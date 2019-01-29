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
#include "DANA/DApplication.h"
#include "DANA/DStatusBits.h"
#include "TProfile.h"

using namespace std;

class JEventProcessor_FCAL_Pi0HFA:public jana::JEventProcessor{
	public:
		JEventProcessor_FCAL_Pi0HFA();
		~JEventProcessor_FCAL_Pi0HFA();
		const char* className(void){return "JEventProcessor_FCAL_Pi0HFA";}

	private:
		jerror_t init(void);						///< Called once at program start.
		jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
		jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
		jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
		jerror_t fini(void);						///< Called after last event of last event source has been processed.

    double m_targetZ;

      TProfile *hCurrentGainConstants;
};

#endif // _JEventProcessor_FCAL_Pi0HFA_

