// $Id$
//
//    File: DDIRCLutReader.cc
//

#include <unistd.h>
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
	/////////////////////////////////////////
	// retrieve from LUT from file or CCDB //
	/////////////////////////////////////////
        const int luts = DDIRCGeometry::kBars;

        string lut_file;
        gPARMS->SetDefaultParameter("DIRC_LUT", lut_file, "DIRC LUT root file (will eventually be moved to resource)");
	
	// follow similar procedure as other resources (DMagneticFieldMapFineMesh)
	map<string,string> lut_map_name;
	jcalib = japp->GetJCalibration(run_number);
	if(jcalib->GetCalib("/DIRC/LUT/lut_map", lut_map_name)) 
		jout << "Can't find requested /DIRC/LUT/lut_map in CCDB for this run!" << endl;
	else if(lut_map_name.find("map_name") != lut_map_name.end() && lut_map_name["map_name"] != "None" && lut_file.empty()) {
		jresman = japp->GetJResourceManager(run_number);
		lut_file = jresman->GetResource(lut_map_name["map_name"]);
	}
	
	// only create reader if have tree from CCDB or command-line parameter is set
	if(!lut_file.empty()) {
		jout<<"Reading DIRC LUT TTree from "<<lut_file<<" ..."<<endl;
		
		auto saveDir = gDirectory;
		TFile *fLut = new TFile(lut_file.c_str());
		if( !fLut->IsOpen() ){
			jerr << "Unable to open " << lut_file << "!!" << endl;
			_exit(-1);
		}
		TTree *tLut=(TTree*) fLut->Get("lut_dirc_flat");
		if( tLut == NULL ){
			jerr << "Unable find TTree lut_dirc_flat in " << lut_file << "!!" << endl;
			_exit(-1);
		}
		
		vector<Float_t> *LutPixelAngleX[luts];
		vector<Float_t> *LutPixelAngleY[luts];
		vector<Float_t> *LutPixelAngleZ[luts];
		vector<Float_t> *LutPixelTime[luts];
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
}

DDIRCLutReader::~DDIRCLutReader() {

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

Float_t DDIRCLutReader::GetLutPixelTime(int bar, int pixel, int entry) const
{
	return lutNodeTime[bar][pixel].at(entry);
}

Long64_t DDIRCLutReader::GetLutPixelPath(int bar, int pixel, int entry) const
{
	return lutNodePath[bar][pixel].at(entry);
}
