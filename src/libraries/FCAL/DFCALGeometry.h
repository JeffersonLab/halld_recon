// $Id$
//
//    File: DFCALGeometry.h
// Created: Wed Aug 24 10:09:27 EST 2005
// Creator: shepherd (on Darwin 129-79-159-16.dhcp-bl.indiana.edu 8.2.0 powerpc)
//

#ifndef _DFCALGeometry_
#define _DFCALGeometry_

#include <JANA/JFactory.h>
#include <JANA/JObject.h>
using namespace jana;

#include "DVector2.h"
#include "units.h"
#include <HDGEOMETRY/DGeometry.h>

class DFCALGeometry : public JObject {

  /*
#define kBlocksWide 59
#define kBlocksTall 59
#define kMaxChannels kBlocksWide * kBlocksTall
// Do not forget to adjust below formula if number of blocks chage in any direction:
//   this is now used to convert from row/col to coordiantes y/x and back - MK
#define kMidBlock (kBlocksWide-1)/2 			
#define kBeamHoleSize 3
  */

public:
	
	JOBJECT_PUBLIC(DFCALGeometry);

	DFCALGeometry(const DGeometry *geom);
	~DFCALGeometry(){}

	// these numbers are fixed for the FCAL as constructed
	// it probably doesn't make sense to retrieve them
	// from a database as they aren't going to change unless
	// the detector is reconstructed

	enum { kBlocksWide = 59 };
	enum { kBlocksTall = 59 };
	enum { kMaxChannels = 4* kBlocksWide * kBlocksTall };
	enum { kMidBlock = ( kBlocksWide - 1 ) / 2 };
	enum { kBeamHoleSize = 3 };

	static double blockSize()  { return 4.0157*k_cm; }
	static double insertBlockSize()  { return 2.09*k_cm; }
	static double radius()  { return 1.20471*k_m; }
	static double blockLength()  { return 45.0*k_cm; }
	static double insertBlockLength()  { return 20.0*k_cm; }
	//	static double fcalFaceZ()  { return 625.3*k_cm; }

	//        static double fcalMidplane() { return fcalFaceZ() + 0.5 * blockLength() ; } 
	
	bool isBlockActive( int row, int column ) const;
	bool isBlockActive(int channel) const {
	  return isBlockActive(row(channel),column(channel));
	}
	bool isInsertBlock(int row,int column) const;
	bool isFiducial(double x,double y) const;
	unsigned int numChannels() const {return m_numChannels;}
	
	DVector2 positionOnFace( int row, int column ) const;
	DVector2 positionOnFace( int channel ) const;
	double fcalFrontZ() const {return m_FCALfront;}
	double insertFrontZ() const {return m_insertFront;}
	double insertSize() const {return m_insertSize;}

	bool inInsert(int channel) const;
	int channel( int row, int column ) const;
	int channel(double x,double y) const;

	int row   ( int channel ) const { return m_row[channel];    }
	int column( int channel ) const { return m_column[channel]; }
	
	// get row and column from x and y positions
	int row   ( float y, bool in_insert=false ) const;
	int column( float x, bool in_insert=false ) const;

	void toStrings(vector<pair<string,string> > &items) const {
	  AddString(items, "kBlocksWide", "%d", (int)kBlocksWide);
	  AddString(items, "kBlocksTall", "%d", (int)kBlocksTall);
	  AddString(items, "kMaxChannels", "%d", (int)kMaxChannels);
	  AddString(items, "kBeamHoleSize", "%2.3f", (int)kBeamHoleSize);
	}
	

 protected:
	bool   m_activeBlock[2*kBlocksTall][2*kBlocksWide];
	DVector2 m_positionOnFace[2*kBlocksTall][2*kBlocksWide];

	int    m_channelNumber[2*kBlocksTall][2*kBlocksWide];
	int    m_row[kMaxChannels];
	int    m_column[kMaxChannels];
	int m_numChannels;

	int m_insertRowSize=0,m_insertMidBlock=0;
	double m_insertSize=0.;
	
	// global offsets
	double m_FCALdX,m_FCALdY,m_FCALfront,m_insertFront;
	

 private:
	DFCALGeometry(){};// force use of constructor with arguments.
	
};

#endif // _DFCALGeometry_

