// $Id$
//
//    File: DDIRCGeometry.h
//

#ifndef _DDIRCGeometry_
#define _DDIRCGeometry_

#include <JANA/JFactory.h>
#include <JANA/JObject.h>
#include <JANA/JCalibration.h>

using namespace jana;

class DDIRCGeometry : public JObject {

public:
	
	JOBJECT_PUBLIC(DDIRCGeometry);

	DDIRCGeometry(int runnumber);
	~DDIRCGeometry(){}

	// these numbers are fixed for the DIRC as constructed
	// it probably doesn't make sense to retrieve them
	// from a database as they aren't going to change unless
	// the detector is reconstructed

	enum { kBars = 48 };
	enum { kPMTs = 108 };
	enum { kPixels = 64 };	

	// get bar # from y positions
	int GetBar   ( float y ) const;
	// get bar X, Y and Length from bar #
	double GetBarY ( int locBar ) const;
	double GetBarEnd ( int locBar ) const;
	double GetBarLength ( int locBar ) const;

	// get PMT and Pixel indices
	int GetPmtID      ( int channel ) const;
	int GetPmtRow     ( int channel ) const;
	int GetPmtColumn  ( int channel ) const;
	int GetPixelID      ( int channel ) const;
	int GetPmtPixelRow     ( int channel ) const;
	int GetPmtPixelColumn  ( int channel ) const;
	int GetPixelRow     ( int channel ) const;
	int GetPixelColumn  ( int channel ) const;
	int GetPixelX       ( int channel ) const;
	int GetPixelY       ( int channel ) const;

	void toStrings(vector<pair<string,string> > &items) const {
		//AddString(items, "kBlocksWide", "%d", (int)kBlocksWide);
		//AddString(items, "kBlocksTall", "%d", (int)kBlocksTall);
		//AddString(items, "kMaxChannels", "%d", (int)kMaxChannels);
		//AddString(items, "kBeamHoleSize", "%2.3f", (int)kBeamHoleSize);
	}
	
private:

	DDIRCGeometry(); // forbid default constructor
	void Initialize(int runnumber);

	int CHANNEL_PER_PMT, PMT_ROWS, PMT_COLUMNS;

	double DIRC_BAR_END[kBars], DIRC_BAR_Y[kBars], DIRC_BAR_L[kBars];
	double DIRC_LUT_Z;
	double DIRC_QZBL_DY;
};

#endif // _DDIRCGeometry_

