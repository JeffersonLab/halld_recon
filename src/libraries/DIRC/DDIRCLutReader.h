// $Id$
//
//    File: DDIRCLutReader.h
//

#ifndef _DDIRCLutReader_
#define _DDIRCLutReader_

#include <JANA/jerror.h>
#include <JANA/JApplication.h>
#include <JANA/JCalibration.h>

#include <DANA/DApplication.h>

#include "TROOT.h"
#include "TVector3.h"
#include "TFile.h"
#include "TTree.h"

class DDIRCLutReader{

public:
	
	DDIRCLutReader(JApplication *japp, unsigned int run_number);
	virtual ~DDIRCLutReader();

	uint GetLutPixelAngleSize(int bar, int pixel) const;
	uint GetLutPixelTimeSize(int bar, int pixel) const;
	uint GetLutPixelPathSize(int bar, int pixel) const;
	TVector3 GetLutPixelAngle(int bar, int pixel, int entry) const;
	Float_t GetLutPixelTime(int bar, int pixel, int entry) const;
	Long64_t GetLutPixelPath(int bar, int pixel, int entry) const;		

private:

	pthread_mutex_t mutex;

	vector<TVector3> lutNodeAngle[48][6912];
	vector<Float_t> lutNodeTime[48][6912];
	vector<Long64_t> lutNodePath[48][6912];

protected:
	JCalibration *jcalib;
	JResourceManager *jresman;
};

#endif // _DDIRCLutReader_

