// $Id$
//
//    File: DEventProcessor_monitoring_hists.h
// Created: Mon Apr  3 11:38:03 EDT 2006
// Creator: pmatt (on Darwin swire-b241.jlab.org 8.4.0 powerpc)
//

#ifndef _DEventProcessor_monitoring_hists_
#define _DEventProcessor_monitoring_hists_

#include "TFile.h"
#include "TROOT.h"

#include "JANA/JEventProcessor.h"

#include "ANALYSIS/DHistogramActions.h"
#include <TRIGGER/DTrigger.h>


class DEventProcessor_monitoring_hists : public JEventProcessor
{
	public:
		DEventProcessor_monitoring_hists(){
			SetTypeName("DEventProcessor_monitoring_hists");
		};
		~DEventProcessor_monitoring_hists(){};

		void Read_MemoryUsage(double& vm_usage, double& resident_set);

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;

		DHistogramAction_TrackMultiplicity dHistogramAction_TrackMultiplicity;
		DHistogramAction_ThrownParticleKinematics dHistogramAction_ThrownParticleKinematics;
		DHistogramAction_DetectedParticleKinematics dHistogramAction_DetectedParticleKinematics;
		DHistogramAction_TrackShowerErrors dHistogramAction_TrackShowerErrors;
		DHistogramAction_GenReconTrackComparison dHistogramAction_GenReconTrackComparison;
		DHistogramAction_NumReconstructedObjects dHistogramAction_NumReconstructedObjects;
		DHistogramAction_ReconnedThrownKinematics dHistogramAction_ReconnedThrownKinematics;
		DHistogramAction_EventVertex dHistogramAction_EventVertex;

		DHistogramAction_DetectorMatchParams dHistogramAction_DetectorMatchParams;
		DHistogramAction_Neutrals dHistogramAction_Neutrals;
		DHistogramAction_DetectorPID dHistogramAction_DetectorPID;
		DHistogramAction_DetectorMatching dHistogramAction_DetectorMatching;
		DHistogramAction_Reconstruction dHistogramAction_Reconstruction;
		DHistogramAction_ObjectMemory dHistogramAction_ObjectMemory;
		DHistogramAction_TriggerStudies dHistogramAction_TriggerStudies;

		TH1D* dHist_IsEvent; //counts events

		unsigned int dNumMemoryMonitorEvents;

        double MIN_TRACKING_FOM;
};

#endif // _DEventProcessor_monitoring_hists_
