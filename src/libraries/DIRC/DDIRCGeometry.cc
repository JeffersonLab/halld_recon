// $Id$
//
//    File: DDIRCGeometry.cc
//
#include <unistd.h>

#include <TString.h>

#include <JANA/Calibrations/JCalibrationManager.h>
#include <JANA/Compatibility/JGeometryManager.h>

#include "DDIRCGeometry.h"


void
DDIRCGeometry::Initialize(const std::shared_ptr<const JEvent>& event) {

	auto event_number = event->GetEventNumber();
	auto run_number = event->GetRunNumber();
	auto app = event->GetJApplication();
	auto jcalib = app->GetService<JCalibrationManager>()->GetJCalibration(run_number);
	auto jgeom = app->GetService<JGeometryManager>()->GetJGeometry(run_number);

	CHANNEL_PER_PMT = kPixels; // 64 = 8 x 8 pixels
	PMT_ROWS = 18;
	PMT_COLUMNS = 6;
	DIRC_QZBL_DY = 3.515;

	map<string,string> installed;
    if(jcalib->GetCalib("/DIRC/install_status", installed, event_number)) return;
	if(atoi(installed["status"].data()) == 0) return;

	vector<double>DIRC;
	vector<double>DRCC;
	vector<double>DCML00_XYZ;
	vector<double>DCML01_XYZ;
	vector<double>DCML10_XYZ;
	vector<double>DCML11_XYZ;
	vector<double>WNGL00_XYZ;
	vector<double>WNGL01_XYZ;
	vector<double>WNGL10_XYZ;
	vector<double>WNGL11_XYZ;
	vector<double>OWDG_XYZ;
        bool all_found = true;
	all_found &= jgeom->Get("//section/composition/posXYZ[@volume='DIRC']/@X_Y_Z", DIRC);
	all_found &= jgeom->Get("//composition[@name='DIRC']/posXYZ[@volume='DRCC']/@X_Y_Z", DRCC);
	all_found &= jgeom->Get("//composition[@name='DRCC']/posXYZ[@volume='DCML00']/@X_Y_Z/plane[@value='4']", DCML00_XYZ);
        all_found &= jgeom->Get("//composition[@name='DRCC']/posXYZ[@volume='DCML01']/@X_Y_Z/plane[@value='3']", DCML01_XYZ);
	all_found &= jgeom->Get("//composition[@name='DRCC']/posXYZ[@volume='DCML10']/@X_Y_Z/plane[@value='1']", DCML10_XYZ);
	all_found &= jgeom->Get("//composition[@name='DRCC']/posXYZ[@volume='DCML11']/@X_Y_Z/plane[@value='2']", DCML11_XYZ);	 
	all_found &= jgeom->Get("//composition[@name='DCML00']/posXYZ[@volume='WNGL']/@X_Y_Z", WNGL00_XYZ);
        all_found &= jgeom->Get("//composition[@name='DCML01']/posXYZ[@volume='WNGL']/@X_Y_Z", WNGL01_XYZ);
	all_found &= jgeom->Get("//composition[@name='DCML10']/posXYZ[@volume='WNGL']/@X_Y_Z", WNGL10_XYZ);
	all_found &= jgeom->Get("//composition[@name='DCML11']/posXYZ[@volume='WNGL']/@X_Y_Z", WNGL11_XYZ);
	all_found &= jgeom->Get("//trd[@name='OWDG']/@Xmp_Ymp_Z", OWDG_XYZ);
	
        if( !all_found ){
                jerr << "Problem finding all DIRC geometry elements. Please make sure your HDDS is up to date!" << jendl;
                _exit(-1);
        }
        
	// set array of bar positions
	for(int i=0; i<kBars; i++) {
		// get bar length by looping over individual components
		DIRC_BAR_L[i] = 0.;
		for(int j=0; j<4; j++) {
			vector<double>locBar_XYZ;
			jgeom->Get(Form("//box[@name='B%02d%c']/@X_Y_Z", i, 'A'+j), locBar_XYZ);
			vector<double>locGlue_XYZ;
			jgeom->Get(Form("//box[@name='G%02d%c']/@X_Y_Z", i, 'A'+j), locGlue_XYZ);
			DIRC_BAR_L[i] += locBar_XYZ[0];
			DIRC_BAR_L[i] += locGlue_XYZ[0];
		}

		// get bar end and Y positions for LUT
		vector<double>DCBR_XYZ;
		if(i<12) {
			DIRC_BAR_END[i] = (DCML10_XYZ[0] - WNGL10_XYZ[0] + OWDG_XYZ[4]);
			
			// get center of bar in Y
			jgeom->Get(Form("//composition[@name='DCML10']/posXYZ[@volume='DCBR%02d']/@X_Y_Z", i), DCBR_XYZ);
			DIRC_BAR_Y[i] = DCML10_XYZ[1] - DCBR_XYZ[1];
		}
		else if(i<24) {
			DIRC_BAR_END[i] = (DCML11_XYZ[0] - WNGL11_XYZ[0] + OWDG_XYZ[4]);

			// get center of bar in Y
			jgeom->Get(Form("//composition[@name='DCML11']/posXYZ[@volume='DCBR%02d']/@X_Y_Z", i), DCBR_XYZ);
			DIRC_BAR_Y[i] = DCML11_XYZ[1] - DCBR_XYZ[1];
		}
		else if(i<36) {
			DIRC_BAR_END[i] = (DCML01_XYZ[0] + WNGL01_XYZ[0] - OWDG_XYZ[4]);

			// get center of bar in Y
			jgeom->Get(Form("//composition[@name='DCML01']/posXYZ[@volume='DCBR%02d']/@X_Y_Z", i), DCBR_XYZ);
			DIRC_BAR_Y[i] = DCML01_XYZ[1] + DCBR_XYZ[1];
		}
		else if(i<48) {
			DIRC_BAR_END[i] = (DCML00_XYZ[0] + WNGL00_XYZ[0] - OWDG_XYZ[4]);
			
			// get center of bar in Y
			jgeom->Get(Form("//composition[@name='DCML00']/posXYZ[@volume='DCBR%02d']/@X_Y_Z", i), DCBR_XYZ);
			DIRC_BAR_Y[i] = DCML00_XYZ[1] + DCBR_XYZ[1];
		}
	}
}

int
DDIRCGeometry::GetPmtID( int channel ) const 
{
	int MAX_BOX_CHANNEL = CHANNEL_PER_PMT * PMT_ROWS * PMT_COLUMNS;
	if(channel < MAX_BOX_CHANNEL) 
		return channel/CHANNEL_PER_PMT;
	else 
		return (channel-MAX_BOX_CHANNEL)/CHANNEL_PER_PMT;
}

int
DDIRCGeometry::GetPixelID( int channel ) const 
{
	return channel%CHANNEL_PER_PMT;
}

int DDIRCGeometry::GetPmtColumn( int channel ) const 
{
	int pmt = GetPmtID(channel);
	return pmt/PMT_ROWS; //  0 - 5
}

int DDIRCGeometry::GetPmtRow( int channel ) const 
{
	int pmt = GetPmtID(channel);
	return pmt%PMT_ROWS; //  0 - 17
}

int DDIRCGeometry::GetPmtPixelColumn( int channel ) const 
{
	int pix = GetPixelID(channel);
	return pix/8; //  0 - 7
}

int DDIRCGeometry::GetPmtPixelRow( int channel ) const 
{
	int pix = GetPixelID(channel);
	return pix%8; //  0 - 7
}

int DDIRCGeometry::GetPixelRow( int channel ) const 
{
	int pmt_row = GetPmtRow(channel);
	int pixel_row = GetPmtPixelRow(channel);
	return 8*pmt_row + pixel_row; //  0 - 143
}

int DDIRCGeometry::GetPixelColumn( int channel ) const 
{
	int pmt_column = GetPmtColumn(channel);
	int pixel_column = GetPmtPixelColumn(channel);
	return 8*pmt_column + pixel_column; // 0 - 47
}
			
int DDIRCGeometry::GetPixelX( int channel ) const 
{
	return abs(GetPixelRow(channel) - 143); //  0 - 143
}

int DDIRCGeometry::GetPixelY( int channel ) const 
{
	return 47 - GetPixelColumn(channel); // 0 - 47
}

int
DDIRCGeometry::GetBar( float y ) const 
{	
	int locBar = -1;

	for(int i=0; i<kBars; i++) {
		if(y > (DIRC_BAR_Y[i] - DIRC_QZBL_DY/2.0) && y < (DIRC_BAR_Y[i] + DIRC_QZBL_DY/2.0))
			locBar = i;
	}

	return static_cast<int>( locBar );
}

double
DDIRCGeometry::GetBarY( int locBar ) const 
{	
	if(locBar < 0 || locBar > 47)
		return -999;
	
	return static_cast<double>( DIRC_BAR_Y[locBar] );
}

double
DDIRCGeometry::GetBarEnd( int locBar ) const 
{	
	if(locBar < 0 || locBar > 47)
		return -999;
	
	return static_cast<double>( DIRC_BAR_END[locBar] );
}

double
DDIRCGeometry::GetBarLength( int locBar ) const 
{	
	if(locBar < 0 || locBar > 47)
		return -999;
	
	return static_cast<double>( DIRC_BAR_L[locBar] );
}


