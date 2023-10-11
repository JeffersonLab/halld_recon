// $Id$
//
//    File: DDIRCLutReader.cc
//

#include <unistd.h>
#include <cassert>
#include <math.h>
using namespace std;

#include "DDIRCLutReader.h"

#include <JANA/Calibrations/JCalibrationManager.h>

//---------------------------------
// DDIRCLutReader    (Constructor)
//---------------------------------
DDIRCLutReader::DDIRCLutReader(JApplication *app, unsigned int run_number)
{
	/////////////////////////////////////////
	// retrieve from LUT from file or CCDB //
	/////////////////////////////////////////
        const int luts = DDIRCGeometry::kBars;

        string lut_file;
        app->SetDefaultParameter("DIRC_LUT", lut_file, "DIRC LUT root file (will eventually be moved to resource)");

	string lutcorr_file;
        app->SetDefaultParameter("DIRC_LUT_CORR", lutcorr_file, "DIRC LUT correction root file (will eventually be moved to resource)");
	
	// follow similar procedure as other resources (DMagneticFieldMapFineMesh)
	map<string,string> lut_map_name;
	jcalib = japp->GetService<JCalibrationManager>()->GetJCalibration(run_number);
	if(jcalib->GetCalib("/DIRC/LUT/lut_map", lut_map_name)) 
		jout << "Can't find requested /DIRC/LUT/lut_map in CCDB for this run!" << jendl;
	else if(lut_map_name.find("map_name") != lut_map_name.end() && lut_map_name["map_name"] != "None" && lut_file.empty()) {
		jresman = japp->GetService<JCalibrationManager>()->GetLargeCalibration(run_number);
		lut_file = jresman->GetResource(lut_map_name["map_name"]);
	}
	
	// only create reader if have tree from CCDB or command-line parameter is set
	if(!lut_file.empty()) {
		jout<<"Reading DIRC LUT TTree from "<<lut_file<<" ..."<<jendl;
		
		TDirectory *saveDir = gDirectory;
		TFile *fLut = new TFile(lut_file.c_str());
		if( !fLut->IsOpen() ){
			jerr << "Unable to open " << lut_file << "!!" << jendl;
			_exit(-1);
		}
		TTree *tLut=(TTree*) fLut->Get("lut_dirc_flat");
		if( tLut == NULL ){
			jerr << "Unable to find TTree lut_dirc_flat in " << lut_file << "!!" << endl;
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

		// attempt to open LUT correction tree
		map<string,string> lutcorr_map_name;
		if(jcalib->GetCalib("/DIRC/LUT/lutcorr_map", lutcorr_map_name)) 
		  jout << "Can't find requested /DIRC/LUT/lutcorr_map in CCDB for this run!" << endl;
		else if(lutcorr_map_name.find("map_name") != lutcorr_map_name.end() && lutcorr_map_name["map_name"] != "None" && lutcorr_file.empty()) {
                  jresman = japp->GetService<JCalibrationManager>()->GetLargeCalibration(run_number);
		  lutcorr_file = jresman->GetResource(lutcorr_map_name["map_name"]);
		}
		if(!lutcorr_file.empty()) {
		  jout<<"Reading DIRC LUT correction TTree from "<<lutcorr_file<<" ..."<<endl;
		  
		  TFile *fLutCorr = new TFile(lutcorr_file.c_str());
		  if( !fLutCorr->IsOpen() ){
		    jerr << "Unable to open " << lutcorr_file << "!!" << endl;
		    _exit(-1);
		  }
		  TTree *tLutCorr=(TTree*) fLutCorr->Get("corr");
		  if( tLutCorr == NULL ){
		    jerr << "Unable to find TTree corr in " << lutcorr_file << "!!" << endl;
		    _exit(-1);
		  }
		  
		  const int nbins = tLutCorr->GetMaximum("bin")+1;
		  double corrAD, corrAR, sigmaAD, sigmaAR, corrTD, corrTR, sigmaTD, sigmaTR;
		  int tb, tp, tbin;
		  tLutCorr->SetBranchAddress("bar", &tb);
		  tLutCorr->SetBranchAddress("pmt", &tp);
		  tLutCorr->SetBranchAddress("bin", &tbin);
		  tLutCorr->SetBranchAddress("zcorrAD", &corrAD);
		  tLutCorr->SetBranchAddress("zcorrAR", &corrAR);
		  tLutCorr->SetBranchAddress("zcorrTD", &corrTD);
		  tLutCorr->SetBranchAddress("zcorrTR", &corrTR);
		  tLutCorr->SetBranchAddress("zsigmaTD", &sigmaTD);
		  tLutCorr->SetBranchAddress("zsigmaTR", &sigmaTR);
		  tLutCorr->SetBranchAddress("zsigmaAD", &sigmaAD);
		  tLutCorr->SetBranchAddress("zsigmaAR", &sigmaAR);
		  
		  // clear arrays to fill from TTree
		  for(int bar=0; bar<DDIRCGeometry::kBars; bar++){
		    for(int pmt=0; pmt<DDIRCGeometry::kPMTs; pmt++){
		      for(int bin=0; bin<nbins; bin++){
			lutCorrAngleDirect[bar][pmt].push_back(0);
			lutCorrAngleReflected[bar][pmt].push_back(0);
			lutCorrTimeDirect[bar][pmt].push_back(0);
			lutCorrTimeReflected[bar][pmt].push_back(0);
		      }
		    }
		  }
		      
		  // fill arrays with per-PMT corrections
		  for (int i = 0; i < tLutCorr->GetEntries(); i++) {
		    tLutCorr->GetEvent(i);
		    lutCorrAngleDirect[tb][tp].at(tbin) = 0.001 * corrAD;
		    lutCorrTimeDirect[tb][tp].at(tbin) = corrTD;
		    lutCorrAngleReflected[tb][tp].at(tbin) = 0.001 * corrAR;
		    lutCorrTimeReflected[tb][tp].at(tbin) = corrTR;
		  }

		  // close LUT correction file
		  fLutCorr->Close();
		}

		// switch back to default ROOT file
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

int DDIRCLutReader::GetLutCorrNbins() const
{
  return (int)lutCorrAngleDirect[0][0].size();
}

double DDIRCLutReader::GetLutCorrAngleDirect(int bar, int pmt, int bin) const
{
        if(bin < 0 || bin >= GetLutCorrNbins()) return 0.;
	return lutCorrAngleDirect[bar][pmt].at(bin);
}

double DDIRCLutReader::GetLutCorrAngleReflected(int bar, int pmt, int bin) const
{
        if(bin < 0 || bin >= GetLutCorrNbins()) return 0.;
        return lutCorrAngleReflected[bar][pmt].at(bin);
}

double DDIRCLutReader::GetLutCorrTimeDirect(int bar, int pmt, int bin) const
{
        if(bin < 0 || bin >= GetLutCorrNbins()) return 0.;
        return lutCorrTimeDirect[bar][pmt].at(bin);
}

double DDIRCLutReader::GetLutCorrTimeReflected(int bar, int pmt, int bin) const
{
        if(bin < 0 || bin >= GetLutCorrNbins()) return 0.;
	return lutCorrTimeReflected[bar][pmt].at(bin);
}
