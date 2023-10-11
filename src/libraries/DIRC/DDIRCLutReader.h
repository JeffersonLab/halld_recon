// $Id$
//
//    File: DDIRCLutReader.h
//

#ifndef _DDIRCLutReader_
#define _DDIRCLutReader_

#include <JANA/Compatibility/jerror.h>
#include <JANA/JApplication.h>
#include <JANA/Calibrations/JCalibration.h>
#include <JANA/Calibrations/JLargeCalibration.h>

#include <DIRC/DDIRCGeometry.h>

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

	int GetLutCorrNbins() const;
	double GetLutCorrAngleDirect(int bar, int pmt, int bin) const;
	double GetLutCorrAngleReflected(int bar, int pmt, int bin) const;
	double GetLutCorrTimeDirect(int bar, int pmt, int bin) const;
	double GetLutCorrTimeReflected(int bar, int pmt, int bin) const;

private:

	pthread_mutex_t mutex;

	vector<TVector3> lutNodeAngle[DDIRCGeometry::kBars][DDIRCGeometry::kPMTs*DDIRCGeometry::kPixels];
	vector<Float_t> lutNodeTime[DDIRCGeometry::kBars][DDIRCGeometry::kPMTs*DDIRCGeometry::kPixels];
	vector<Long64_t> lutNodePath[DDIRCGeometry::kBars][DDIRCGeometry::kPMTs*DDIRCGeometry::kPixels];

	vector<double> lutCorrAngleDirect[DDIRCGeometry::kBars][DDIRCGeometry::kPMTs];
	vector<double> lutCorrAngleReflected[DDIRCGeometry::kBars][DDIRCGeometry::kPMTs];
	vector<double> lutCorrTimeDirect[DDIRCGeometry::kBars][DDIRCGeometry::kPMTs];
	vector<double> lutCorrTimeReflected[DDIRCGeometry::kBars][DDIRCGeometry::kPMTs];
	vector<double> lutSigmaAngleDirect[DDIRCGeometry::kBars][DDIRCGeometry::kPMTs];
	vector<double> lutSigmaAngleReflected[DDIRCGeometry::kBars][DDIRCGeometry::kPMTs];
	vector<double> lutSigmaTimeDirect[DDIRCGeometry::kBars][DDIRCGeometry::kPMTs];
	vector<double> lutSigmaTimeReflected[DDIRCGeometry::kBars][DDIRCGeometry::kPMTs];

protected:
	JCalibration *jcalib;
	JLargeCalibration *jresman;
};

#endif // _DDIRCLutReader_

