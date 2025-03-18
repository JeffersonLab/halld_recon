// $Id$
//
//    File: DEventProcessor_coherent_peak_skim.h
// Created: Tue Mar 28 10:57:49 EDT 2017
// Creator: pmatt (on Linux pmattdesktop.jlab.org 2.6.32-696.el6.x86_64 x86_64)
//

#ifndef _DEventProcessor_coherent_peak_skim_
#define _DEventProcessor_coherent_peak_skim_

#include <map>

#include <JANA/JEventProcessor.h>
#include <JANA/JApplication.h>

#include <ANALYSIS/DEventWriterROOT.h>
#include <HDDM/DEventWriterREST.h>
#include <ANALYSIS/DHistogramActions.h>

#include "PID/DChargedTrack.h"
#include "PID/DBeamPhoton.h"
#include "PID/DEventRFBunch.h"
#include "GlueX.h"

using namespace std;

class DEventProcessor_coherent_peak_skim : public JEventProcessor
{
	public:
		DEventProcessor_coherent_peak_skim() {
			SetTypeName("DEventProcessor_coherent_peak_skim");
		}
		~DEventProcessor_coherent_peak_skim() override = default;

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& locEvent) override;
		void Process(const std::shared_ptr<const JEvent>& locEvent) override;

		bool Cut_ShowerEOverP(const DChargedTrackHypothesis* locChargedHypo) const;
		double Step_TimeToNearInputTime(double locTimeToStep, double locTimeToStepTo) const;

		double dBeamBunchPeriod;
		double dTargetCenterZ;
		double dShowerEOverPCut;
		pair<double, double> dCoherentPeakRange;
		map<Particle_t, map<DetectorSystem_t, double> > dTimingCutMap;

};

#endif // _DEventProcessor_coherent_peak_skim_

