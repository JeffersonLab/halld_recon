// $Id$
//
//    File: DParticleID_PID1.h
// Created: Mon Feb 28 15:25:35 EST 2011
// Creator: staylor (on Linux ifarml1 2.6.18-128.el5 x86_64)
//

#ifndef _DParticleID_PID1_
#define _DParticleID_PID1_

#include "DParticleID.h"

class DParticleID_PID1:public DParticleID{
 public:
  DParticleID_PID1(const std::shared_ptr<const JEvent>& loop); // require JEventLoop in constructor;
  ~DParticleID_PID1();

	jerror_t GetdEdxMean_CDC(double locBeta, unsigned int locNumHitsUsedFordEdx, double& locMeandEdx, Particle_t locPIDHypothesis) const;
	jerror_t GetdEdxSigma_CDC(double locBeta, unsigned int locNumHitsUsedFordEdx, double& locSigmadEdx, Particle_t locPIDHypothesis) const;
	jerror_t GetdEdxMean_FDC(double locBeta, unsigned int locNumHitsUsedFordEdx, double& locMeandEdx, Particle_t locPIDHypothesis) const;
	jerror_t GetdEdxSigma_FDC(double locBeta, unsigned int locNumHitsUsedFordEdx, double& locSigmadEdx, Particle_t locPIDHypothesis) const;
	double GetProtondEdxMean_TOF(double locBeta) const;
	double GetProtondEdxSigma_TOF(double locBeta) const;
	double GetProtondEdxMean_SC(double locBeta) const;
	double GetProtondEdxSigma_SC(double locBeta) const;
	double GetEOverPMean(DetectorSystem_t detector,double p) const;
	double GetEOverPSigma(DetectorSystem_t detector,double p) const;
	double GetTimeVariance(DetectorSystem_t detector,Particle_t particle,double p) const;
	double GetTimeMean(DetectorSystem_t detector,Particle_t particle,double p) const;

	jerror_t CalcDCdEdxChiSq(DChargedTrackHypothesis *locChargedTrackHypothesis) const;
	inline double Function_dEdx(double locBetaGamma, const vector<float> &locParams) const{return locParams[0]/(locBetaGamma*locBetaGamma)+locParams[1]/locBetaGamma + locParams[2] + locParams[3]*locBetaGamma;}
	inline double Function_dEdxSigma(double locBetaGamma, const vector<float> &locParams) const{return locParams[0]/(locBetaGamma*locBetaGamma)+locParams[1]/locBetaGamma + locParams[2];}


 protected:
	vector<float> ddEdxMeanParams_FDC_Proton;
	vector<float> ddEdxMeanParams_FDC_KPlus;
	vector<float> ddEdxMeanParams_FDC_PiPlus;
	vector<float> ddEdxMeanParams_CDC_Proton;	
	vector<float> ddEdxMeanParams_FDC_Electron;
	vector<float> ddEdxMeanParams_CDC_KPlus;
	vector<float> ddEdxMeanParams_CDC_PiPlus;
	vector<float> ddEdxMeanParams_CDC_Electron;
	vector<float> ddEdxMeanParams_SC_Proton;
	vector<float> ddEdxMeanParams_TOF_Proton;

	vector<float> dEOverPMeanParams_BCAL;
	vector<float> dEOverPMeanParams_FCAL;

	vector<float> ddEdxSigmaParams_FDC_N_dependence;
	vector<float> ddEdxSigmaParams_FDC_Proton;
	vector<float> ddEdxSigmaParams_FDC_KPlus;
	vector<float> ddEdxSigmaParams_FDC_PiPlus;
	vector<float> ddEdxSigmaParams_FDC_Electron;
	vector<float> ddEdxSigmaParams_CDC_N_dependence;
	vector<float> ddEdxSigmaParams_CDC_Proton;
	vector<float> ddEdxSigmaParams_CDC_KPlus;
	vector<float> ddEdxSigmaParams_CDC_PiPlus;
	vector<float> ddEdxSigmaParams_CDC_Electron;
	vector<float> ddEdxSigmaParams_SC_Proton;
	vector<float> ddEdxSigmaParams_TOF_Proton;

	vector<float> dTimeSigmaParams_TOF_Proton;
	vector<float> dTimeSigmaParams_TOF_KPlus;
	vector<float> dTimeSigmaParams_TOF_PiPlus;
	vector<float> dTimeSigmaParams_TOF_Positron;

	vector<float> dTimeSigmaParams_BCAL_Proton;
	vector<float> dTimeSigmaParams_BCAL_KPlus;
	vector<float> dTimeSigmaParams_BCAL_PiPlus;
	vector<float> dTimeSigmaParams_BCAL_Positron;
	vector<float> dTimeMeanParams_BCAL_Proton;
	vector<float> dTimeMeanParams_BCAL_KPlus;
	vector<float> dTimeMeanParams_BCAL_PiPlus;
	vector<float> dTimeMeanParams_BCAL_Positron;
	vector<float> dEOverPSigmaParams_BCAL;

	vector<float> dTimeSigmaParams_FCAL_Proton;
	vector<float> dTimeSigmaParams_FCAL_KPlus;
	vector<float> dTimeSigmaParams_FCAL_PiPlus;
	vector<float> dTimeSigmaParams_FCAL_Positron;
	vector<float> dTimeMeanParams_FCAL_Proton;
	vector<float> dTimeMeanParams_FCAL_KPlus;
	vector<float> dTimeMeanParams_FCAL_PiPlus;
	vector<float> dTimeMeanParams_FCAL_Positron;
	vector<float> dEOverPSigmaParams_FCAL;

 private:
  int DEBUG_LEVEL;
  // Prohibit default constructor
  DParticleID_PID1();	

};

#endif // _DParticleID_PID1_

