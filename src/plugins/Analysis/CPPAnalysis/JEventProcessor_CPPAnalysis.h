// $Id$
//
//    File: JEventProcessor_CPPAnalysis.h
// Created: Fri Mar  4 08:20:58 EST 2022
// Creator: staylor (on Linux ifarm1901.jlab.org 3.10.0-1160.11.1.el7.x86_64 x86_64)
//

#ifndef _JEventProcessor_CPPAnalysis_
#define _JEventProcessor_CPPAnalysis_

#include <JANA/JEventProcessor.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TH3F.h>
#include <TDirectory.h>
#include <TRACKING/DTrackTimeBased.h>
#include <PID/DNeutralParticleHypothesis.h>
#include <PID/DBeamPhoton.h>
#include <PID/DChargedTrack.h>
#include <PID/DNeutralParticle.h>
#include <KINFITTER/DKinFitter.h>
#include <ANALYSIS/DKinFitUtils_GlueX.h>

class JEventProcessor_CPPAnalysis:public jana::JEventProcessor{
public:
JEventProcessor_CPPAnalysis();
~JEventProcessor_CPPAnalysis();
const char* className(void){return "JEventProcessor_CPPAnalysis";}

private:
jerror_t init(void);						///< Called once at program start.
jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
jerror_t fini(void);						///< Called after last event of last event source has been processed.

bool VetoNeutrals(double t0_rf,const DVector3 &vect,
		    vector<const DNeutralParticle*>&neutrals) const;
void DoKinematicFit(const DBeamPhoton *beamphoton,
		      const DTrackTimeBased *negative,
		      const DTrackTimeBased *positive,
		      DKinFitUtils_GlueX *dKinFitUtils,
		      DKinFitter *dKinFitter,
		      DAnalysisUtilities *dAnalysisUtilities) const;
void EpEmAnalysis(DKinFitter *dKinFitter,double Eelectron,double Epositron,
		    double weight) const;
void PipPimAnalysis(DKinFitter *dKinFitter,double weight) const;

double SPLIT_CUT,FCAL_THRESHOLD,BCAL_THRESHOLD,GAMMA_DT_CUT;
double EPEM_CL_CUT,PIPPIM_CL_CUT;
Particle_t m_TargetParticle=Pb208;

TH2F *H_CL_comp,*H_E_over_P_electron,*H_E_over_P_positron;
TH2F *H_VertexXY;
TH1F *H_EpEmMass,*H_EpEmMass_Cut,*H_PipPimMass,*H_VertexZ;

};

#endif // _JEventProcessor_CPPAnalysis_

