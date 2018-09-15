// $Id$
//
//    File: DDIRCGeometry.cc
//

#include <DANA/DApplication.h>

#include "DDIRCGeometry.h"

//---------------------------------
// DDIRCGeometry    (Constructor)
//---------------------------------
DDIRCGeometry::DDIRCGeometry(int runnumber) {

	CHANNEL_PER_PMT = 64;
	PMT_ROWS = 18;
	PMT_COLUMNS = 6;
	DIRC_QZBL_DY = 3.515; 

	// Initialize DDIRCGeometry variables
	Initialize(runnumber);
}

void
DDIRCGeometry::Initialize(int runnumber) {

	//Get pointer to DGeometry object
	DApplication* dapp=dynamic_cast<DApplication*>(japp);
	JGeometry *jgeom  = dapp->GetJGeometry(runnumber);	

	vector<double>DIRC;
	vector<double>DRCC;
	vector<double>DCML00_XYZ;
	vector<double>DCML01_XYZ;
	vector<double>DCML10_XYZ;
	vector<double>DCML11_XYZ;
	jgeom->Get("//section/composition/posXYZ[@volume='DIRC']/@X_Y_Z", DIRC);
	jgeom->Get("//composition[@name='DIRC']/posXYZ[@volume='DRCC']/@X_Y_Z", DRCC);
	jgeom->Get("//composition[@name='DRCC']/posXYZ[@volume='DCML00']/@X_Y_Z/plane[@value='4']", DCML00_XYZ);
        jgeom->Get("//composition[@name='DRCC']/posXYZ[@volume='DCML01']/@X_Y_Z/plane[@value='3']", DCML01_XYZ);
	jgeom->Get("//composition[@name='DRCC']/posXYZ[@volume='DCML10']/@X_Y_Z/plane[@value='1']", DCML10_XYZ);
	jgeom->Get("//composition[@name='DRCC']/posXYZ[@volume='DCML11']/@X_Y_Z/plane[@value='2']", DCML11_XYZ);	 
	
	// set array of bar positions
	for(int i=0; i<48; i++) {
		vector<double>DCBR_XYZ;
		if(i<12) {
			jgeom->Get(Form("//composition[@name='DCML10']/posXYZ[@volume='DCBR%02d']/@X_Y_Z", i), DCBR_XYZ);
			DIRC_BAR_Y[i] = DCML10_XYZ[1] - DCBR_XYZ[1];
		}
		else if(i<24) {
			jgeom->Get(Form("//composition[@name='DCML11']/posXYZ[@volume='DCBR%02d']/@X_Y_Z", i), DCBR_XYZ);
			DIRC_BAR_Y[i] = DCML11_XYZ[1] - DCBR_XYZ[1];
		}
		else if(i<36) {
			jgeom->Get(Form("//composition[@name='DCML01']/posXYZ[@volume='DCBR%02d']/@X_Y_Z", i), DCBR_XYZ);
			DIRC_BAR_Y[i] = DCML01_XYZ[1] - DCBR_XYZ[1];
		}
		else if(i<48) {
			jgeom->Get(Form("//composition[@name='DCML00']/posXYZ[@volume='DCBR%02d']/@X_Y_Z", i), DCBR_XYZ);
			DIRC_BAR_Y[i] = DCML00_XYZ[1] - DCBR_XYZ[1];
		}
	}
	
	return;
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

int DDIRCGeometry::GetPixelColumn( int channel ) const 
{
	int pix = GetPixelID(channel);
	return pix/8; //  0 - 7
}

int DDIRCGeometry::GetPixelRow( int channel ) const 
{
	int pix = GetPixelID(channel);
	return pix%8; //  0 - 7
}
			
int DDIRCGeometry::GetPixelX( int channel ) const 
{
	int pmt_row = GetPmtRow(channel);
	int pixel_row = GetPixelRow(channel);
	return abs(8*pmt_row + pixel_row - 143); //  0 - 143
}

int DDIRCGeometry::GetPixelY( int channel ) const 
{
	int pmt_column = GetPmtColumn(channel);
	int pixel_column = GetPixelColumn(channel);
	return 47 - (8*pmt_column + pixel_column); // 0 - 47
}

int
DDIRCGeometry::GetBar( float y ) const 
{	
	int locBar = -1;

	for(int i=0; i<48; i++) {
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


