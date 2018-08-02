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
#include <DIRC/DDIRCLutPhotons.h>

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

	DDIRCLut(JEventLoop *loop);
	~DDIRCLut(){};

	bool CalcLUT(TVector3 locProjPos, TVector3 locProjMom, const vector<const DDIRCTruthPmtHit*> locDIRCHits, double locFlightTime, double locMass, shared_ptr<DDIRCMatchParams>& locDIRCMatchParams) const;

	uint GetLutPixelAngleSize(int bar, int pixel) const;
	uint GetLutPixelTimeSize(int bar, int pixel) const;
	uint GetLutPixelPathSize(int bar, int pixel) const;
	TVector3 GetLutPixelAngle(int bar, int pixel, int entry) const;
	Double_t GetLutPixelTime(int bar, int pixel, int entry) const;
	Long64_t GetLutPixelPath(int bar, int pixel, int entry) const;
	
private:
	DApplication *dapp;

	vector<TVector3> lutNodeAngle[48][10864];
	vector<Double_t> lutNodeTime[48][10864];
	vector<Long64_t> lutNodePath[48][10864];

	TF1 *fAngle[4];
	
	bool DIRC_DEBUG_HISTS;
	TH1I *hDiff, *hDiffT, *hDiffD, *hDiffR, *hTime, *hCalc, *hNph, *hNphC;
	TH2I *hDiff_Pixel;
	TH1I *hDeltaThetaC[4];
	TH2I *hDeltaThetaC_Pixel[4];
};

#endif // _DDIRCLut_

