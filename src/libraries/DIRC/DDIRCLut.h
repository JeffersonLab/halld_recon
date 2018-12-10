// $Id$
//
//    File: DDIRCLut.h
//

#ifndef _DDIRCLut_
#define _DDIRCLut_

#include <JANA/JFactory.h>
#include <JANA/JObject.h>
using namespace jana;

#include <DANA/DApplication.h>
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

class DDIRCLut: public JObject {

public:
	
	JOBJECT_PUBLIC(DDIRCLut);

	DDIRCLut();
	~DDIRCLut(){};

	bool brun(JEventLoop *loop);
	bool CreateDebugHistograms();
	bool CalcLUT(TVector3 locProjPos, TVector3 locProjMom, const vector<const DDIRCPmtHit*> locDIRCHits, double locFlightTime, Particle_t locPID, shared_ptr<DDIRCMatchParams>& locDIRCMatchParams, const vector<const DDIRCTruthBarHit*> locDIRCBarHits) const;
	double CalcLikelihood(double locExpectedThetaC, double locThetaC) const;
	
private:
	DApplication *dapp;
	DDIRCLutReader *dDIRCLutReader;
	const DDIRCGeometry *dDIRCGeometry;
	
	bool DIRC_DEBUG_HISTS;
	bool DIRC_TRUTH_BARHIT;

	TH1I *hDiff, *hDiffT, *hDiffD, *hDiffR, *hTime, *hCalc, *hNph, *hNphC;
	TH2I *hDiff_Pixel;
	deque<Particle_t> dFinalStatePIDs;
	TH1I* hDeltaThetaC[4];
	TH2I* hDeltaThetaC_Pixel[4];
};

#endif // _DDIRCLut_

