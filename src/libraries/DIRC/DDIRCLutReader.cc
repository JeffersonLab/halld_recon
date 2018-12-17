// $Id$
//
//    File: DDIRCLutReader.cc
//

#include <cassert>
#include <math.h>
using namespace std;

#include "DDIRCLutReader.h"
#include "DANA/DApplication.h"

//---------------------------------
// DDIRCLutReader    (Constructor)
//---------------------------------
DDIRCLutReader::DDIRCLutReader(JApplication *japp, unsigned int run_number) 
{
	/////////////////////////////////
	// retrieve from LUT from file //
	/////////////////////////////////
        const int luts = 48;
        
        string lut_file = "/group/halld/Users/jrsteven/2018-dirc/dircsim-2018_08-ver07/lut_all_flat.root";
        gPARMS->SetDefaultParameter("DIRC_LUT", lut_file, "DIRC LUT root file (will eventually be moved to resource)");
	
	// eventually needs to come from CCDB with run number index
	auto saveDir = gDirectory;
	TFile *fLut = new TFile(lut_file.c_str());
        TTree *tLut=(TTree*) fLut->Get("lut_dirc_flat");

	vector<Double_t> *LutPixelAngleX[luts];
	vector<Double_t> *LutPixelAngleY[luts];
	vector<Double_t> *LutPixelAngleZ[luts];
	vector<Double_t> *LutPixelTime[luts];
	vector<Long64_t> *LutPixelPath[luts];

	// clear arrays to fill from TTree
	for(int l=0; l<luts; l++){
		LutPixelAngleX[l] = 0;
		LutPixelAngleY[l] = 0;
		LutPixelAngleZ[l] = 0;
		LutPixelTime[l] = 0;
		LutPixelPath[l] = 0;
	}

        for(int l=0; l<luts; l++){
		tLut->SetBranchAddress(Form("LUT_AngleX_%d",l),&LutPixelAngleX[l]); 
		tLut->SetBranchAddress(Form("LUT_AngleY_%d",l),&LutPixelAngleY[l]); 
		tLut->SetBranchAddress(Form("LUT_AngleZ_%d",l),&LutPixelAngleZ[l]); 
		tLut->SetBranchAddress(Form("LUT_Time_%d",l),&LutPixelTime[l]); 
		tLut->SetBranchAddress(Form("LUT_Path_%d",l),&LutPixelPath[l]); 
        }

        // fill nodes with LUT info for each bar/pixel combination
	for(int i=0; i<tLut->GetEntries(); i++) { // get pixels from TTree
		tLut->GetEntry(i);

		for(int l=0; l<luts; l++){ // loop over bars
			for(uint j=0; j<LutPixelAngleX[l]->size(); j++) { // loop over possible paths
				
				TVector3 angle(LutPixelAngleX[l]->at(j), LutPixelAngleY[l]->at(j), LutPixelAngleZ[l]->at(j));
				lutNodeAngle[l][i].push_back(angle);
				lutNodeTime[l][i].push_back(LutPixelTime[l]->at(j));
				lutNodePath[l][i].push_back(LutPixelPath[l]->at(j));
			}
		}
	}

	// close LUT file
	fLut->Close();
	saveDir->cd();
}

DDIRCLutReader::~DDIRCLutReader() {
	delete[] lutNodeAngle;
	delete[] lutNodeTime;
	delete[] lutNodePath;
}

uint DDIRCLutReader::GetLutPixelAngleSize(int bar, int pixel) const
{
	return lutNodeAngle[bar][pixel].size();
}
	
uint DDIRCLutReader::GetLutPixelTimeSize(int bar, int pixel) const
{
	return lutNodeTime[bar][pixel].size();
}
	
uint DDIRCLutReader::GetLutPixelPathSize(int bar, int pixel) const
{
	return lutNodePath[bar][pixel].size();
}

TVector3 DDIRCLutReader::GetLutPixelAngle(int bar, int pixel, int entry) const
{
	return lutNodeAngle[bar][pixel].at(entry);
}

Double_t DDIRCLutReader::GetLutPixelTime(int bar, int pixel, int entry) const
{
	return lutNodeTime[bar][pixel].at(entry);
}

Long64_t DDIRCLutReader::GetLutPixelPath(int bar, int pixel, int entry) const
{
	return lutNodePath[bar][pixel].at(entry);
}
