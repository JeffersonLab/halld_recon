// $Id$
//
//    File: DDIRCLut.h
//

#ifndef _DDIRCLut_
#define _DDIRCLut_

#include <JANA/JObject.h>
#include <JANA/Compatibility/JLockService.h>

#include <DANA/DGeometryManager.h>
#include <PID/DDetectorMatches.h>
#include <DIRC/DDIRCGeometry.h>
#include <DIRC/DDIRCLutReader.h>
#include <DIRC/DDIRCTruthBarHit.h>
#include <DIRC/DDIRCPmtHit.h>

#include "TROOT.h"
#include "TVector3.h"
#include "TFile.h"
#include "TTree.h"
#include "TH1.h"
#include "TH2.h"
#include "TF1.h"

#include <queue>

class DDIRCLut: public JObject {

public:
	
	JOBJECT_PUBLIC(DDIRCLut);

	DDIRCLut(){};
	~DDIRCLut(){};

	bool Init(const std::shared_ptr<const JEvent>& event);
	bool CreateDebugHistograms();
	bool CalcLUT(TVector3 locProjPos, TVector3 locProjMom, const vector<const DDIRCPmtHit*> locDIRCHits, double locFlightTime, double locMass, shared_ptr<DDIRCMatchParams>& locDIRCMatchParams, const vector<const DDIRCTruthBarHit*> locDIRCBarHits, map<shared_ptr<const DDIRCMatchParams>, vector<const DDIRCPmtHit*> >& locDIRCTrackMatchParams) const;
	vector<pair<double,double>> CalcPhoton(const DDIRCPmtHit *locDIRCHit, double locFlightTime, TVector3 posInBar, TVector3 momInBar, map<Particle_t, double> locExpectedAngle, double locAngle, Particle_t locPID, bool &isReflected, map<Particle_t, double> &logLikelihoodSum, int &nPhotonsThetaC, double &meanThetaC, double &meanDeltaT, bool &isGood) const;
	vector<pair<double,double>> CalcPhoton(const DDIRCPmtHit *locDIRCHit, double locFlightTime, TVector3 posInBar, TVector3 momInBar, map<Particle_t, double> locExpectedAngle, double locAngle, Particle_t locPID, bool &isReflected, map<Particle_t, double> &logLikelihoodSum) const;
	double CalcLikelihood(double locExpectedThetaC, double locThetaC) const;
	double CalcAngle(double locP, double locMass) const;
	map<Particle_t, double> CalcExpectedAngles(double locP) const;
	
private:
	std::shared_ptr<DGeometryManager> dGeometryManager;
	std::shared_ptr<JLockService> lockService;

	DDIRCLutReader *dDIRCLutReader;
	const DDIRCGeometry *dDIRCGeometry;
	
	bool DIRC_DEBUG_HISTS;
	bool DIRC_TRUTH_BARHIT;
	bool DIRC_TRUTH_PIXELTIME;
	bool DIRC_ROTATE_TRACK;
	bool DIRC_THETAC_OFFSET;
	bool DIRC_LUT_CORR;
	bool DIRC_CHROMATIC_CORR;

	double DIRC_CUT_TDIFFD;
	double DIRC_CUT_TDIFFR;
	double DIRC_SIGMA_THETAC;
	double DIRC_LIGHT_V;
	double DIRC_CHROMATIC_CONST;
	double DIRC_CUT_ANGLE;

	double dThetaCOffset[DDIRCGeometry::kBars][DDIRCGeometry::kPMTs]; 
	double dRotationX[DDIRCGeometry::kBars], dRotationY[DDIRCGeometry::kBars], dRotationZ[DDIRCGeometry::kBars];

	int dMaxChannels;
	double dCriticalAngle, dIndex;

	TH1I *hDiff, *hDiffT, *hDiffD, *hDiffR, *hTime, *hCalc, *hNph, *hNphC;
	TH2I *hDiff_Pixel[2];
	std::deque<Particle_t> dFinalStatePIDs;
	map<Particle_t, TH1I*> hDeltaThetaC;
	map<Particle_t, TH2I*> hDeltaThetaC_Pixel;

};

#endif // _DDIRCLut_

