// $Id$
//
//    File: DCPPEpEm_factory.h
// Created: Thu Mar 17 14:49:42 EDT 2022
// Creator: staylor (on Linux ifarm1801.jlab.org 3.10.0-1160.11.1.el7.x86_64 x86_64)
//

#ifndef _DCPPEpEm_factory_
#define _DCPPEpEm_factory_

#include <mutex>

#ifdef HAVE_TENSORFLOWLITE
#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/model.h"
#include "tensorflow/lite/optional_debug_tools.h"
#endif // HAVE_TENSORFLOWLITE

#include <JANA/JFactory.h>
#include "DCPPEpEm.h"
#include <TRACKING/DTrackTimeBased.h>
#include <PID/DNeutralParticleHypothesis.h>
#include <PID/DBeamPhoton.h>
#include <PID/DChargedTrack.h>
#include <PID/DNeutralParticle.h>
#include <KINFITTER/DKinFitter.h>
#include <ANALYSIS/DKinFitUtils_GlueX.h>
#include <FMWPC/DFMWPCMatchedTrack.h>
#include <FMWPC/DFMWPCHit.h>

class DCPPEpEm_factory:public jana::JFactory<DCPPEpEm>{
public:
DCPPEpEm_factory(){};
~DCPPEpEm_factory(){};

private:
jerror_t init(void);						///< Called once at program start.
  jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
  jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
  jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
  jerror_t fini(void);						///< Called after last event of last event source has been processed.

void DoKinematicFit(const DBeamPhoton *beamphoton,
		      const DTrackTimeBased *negative,
		      const DTrackTimeBased *positive,
		      DKinFitUtils_GlueX *dKinFitUtils,
		      DKinFitter *dKinFitter,
		      DAnalysisUtilities *dAnalysisUtilities) const;
bool VetoNeutrals(double t0_rf,const DVector3 &vect,
		    vector<const DNeutralParticle*>&neutrals) const;

bool PiMuFillFeatures(jana::JEventLoop *loop, const DTrackTimeBased *piplus, const DTrackTimeBased *piminus, float *features);

double SPLIT_CUT,FCAL_THRESHOLD,BCAL_THRESHOLD,GAMMA_DT_CUT;
string PIMU_MODEL_FILE;
int    VERBOSE;
std::mutex pimu_model_mutex;

#if HAVE_TENSORFLOWLITE
std::unique_ptr<tflite::FlatBufferModel> pimu_model;
std::unique_ptr<tflite::Interpreter>     pimu_interpreter;
float *pimu_input  = nullptr;
float *pimu_output = nullptr;
#endif // HAVE_TENSORFLOWLITE

};

#endif // _DCPPEpEm_factory_

