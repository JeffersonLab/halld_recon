// $Id$
//
//    File: DDIRCGeometry.h
//

#ifndef _DDIRCGeometry_
#define _DDIRCGeometry_

#include <JANA/JObject.h>
#include <JANA/JEvent.h>
#include <JANA/Calibrations/JCalibration.h>

class DDIRCGeometry : public JObject {

public:
	
	JOBJECT_PUBLIC(DDIRCGeometry);

	DDIRCGeometry(){}
	~DDIRCGeometry(){}

	// We put the dependency on JEvent in Initialize so that we
	// have other ways of creating DDIRCGeometry objects.
	// This improves testability
	void Initialize(const std::shared_ptr<const JEvent>& event);

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

	void Summarize(JObjectSummary& summary) const override {
		// summary.add((int)kBlocksWide, "kBlocksWide", "%d");
		// summary.add((int)kBlocksTall, "kBlocksTall", "%d");
		// summary.add((int)kMaxChannels, "kMaxChannels", "%d");
		// summary.add((int)kBeamHoleSize, "kBeamHoleSize", "%2.3f");
	}

private:
	int CHANNEL_PER_PMT, PMT_ROWS, PMT_COLUMNS;

	double DIRC_BAR_END[kBars], DIRC_BAR_Y[kBars], DIRC_BAR_L[kBars];
	double DIRC_LUT_Z;
	double DIRC_QZBL_DY;
};

#endif // _DDIRCGeometry_

