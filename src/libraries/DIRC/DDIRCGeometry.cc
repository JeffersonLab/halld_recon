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
	vector<double>DCML_ZX;
	vector<double>DCML_Y0;
	vector<double>DCML_DY;
	vector<double>DCBR_Y0;
	vector<double>DCBR_DY;
	vector<double>QZBL_DX;
	vector<double>QZBL_YZ;
	vector<double>QZBL_DXDYDZ;
	jgeom->Get("//section/composition/posXYZ[@volume='DIRC']/@X_Y_Z", DIRC);
	jgeom->Get("//composition[@name='DIRC']/posXYZ[@volume='DRCC']/@X_Y_Z", DRCC);
	jgeom->Get("//composition[@name='DRCC']/mposY[@volume='DCML']/@Z_X/plane[@value='3']", DCML_ZX);
	jgeom->Get("//composition[@name='DRCC']/mposY[@volume='DCML']/@Y0/plane[@value='3']", DCML_Y0);
	jgeom->Get("//composition[@name='DRCC']/mposY[@volume='DCML']/@dY/plane[@value='3']", DCML_DY);
	jgeom->Get("//composition[@name='DCML']/mposY[@volume='DCBR']/@Y0", DCBR_Y0);
	jgeom->Get("//composition[@name='DCML']/mposY[@volume='DCBR']/@dY", DCBR_DY);
	jgeom->Get("//composition[@name='DCBR']/mposX[@volume='QZBL']/@dX", QZBL_DX);
	jgeom->Get("//composition[@name='DCBR']/mposX[@volume='QZBL']/@Y_Z", QZBL_YZ);
	jgeom->Get("//box[@name='QZBL']/@X_Y_Z", QZBL_DXDYDZ);
	
	DIRC_LUT_X = (DCML_ZX[1] + 4*QZBL_DX[0]);
	DIRC_LUT_Z = (DIRC[2] + DRCC[2] + DCML_ZX[0] + QZBL_YZ[1]);
	DIRC_DCML_Y = DCML_Y0[0];
	DIRC_DCML_DY = DCML_DY[0];
	DIRC_DCBR_Y = fabs(DCBR_Y0[0]);
	DIRC_DCBR_DY = DCBR_DY[0];
	DIRC_QZBL_DY = QZBL_DXDYDZ[1];
	DIRC_QZBL_DZ = QZBL_DXDYDZ[2];

	// set array of bar positions
	double arr[4] = {-1.*DIRC_DCML_Y+DIRC_DCBR_Y, -1.*(DIRC_DCML_Y+DIRC_DCML_DY)+DIRC_DCBR_Y, DIRC_DCML_Y-DIRC_DCBR_Y, DIRC_DCML_Y+DIRC_DCML_DY-DIRC_DCBR_Y};
	
	for(int i=0; i<48; i++) {
		double y = arr[i/12]+(i%12)*DIRC_DCBR_DY;
		if(i < 24)
			y = arr[i/12]-(i%12)*DIRC_DCBR_DY;

		DIRC_BAR_Y[i] = y;
	}
	
	return;
}

int
DDIRCGeometry::GetPmtID( int channel ) const 
{
	return channel/CHANNEL_PER_PMT;
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


