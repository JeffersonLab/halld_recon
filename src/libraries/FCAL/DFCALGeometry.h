// $Id$
//
//    File: DFCALGeometry.h
// Created: Wed Aug 24 10:09:27 EST 2005
// Creator: shepherd (on Darwin 129-79-159-16.dhcp-bl.indiana.edu 8.2.0 powerpc)
//

#ifndef _DFCALGeometry_
#define _DFCALGeometry_

#include <JANA/JObject.h>

#include "DVector2.h"
#include "units.h"
#include <HDGEOMETRY/DGeometry.h>

class DFCALGeometry: public JObject {
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
  enum { kMaxChannels = kBlocksWide * kBlocksTall };
  enum { kMidBlock = ( kBlocksWide - 1 ) / 2 };
  enum { kBeamHoleSize = 3 }; // Before adding insert for FCAL-2

  static double blockSize()  { return 4.0157*k_cm; }
  static double radius()  { return 1.20471*k_m; }
  static double blockLength()  { return 45.0*k_cm; }
  
  bool isBlockActive( int row, int column ) const;
  bool isBlockActive(int channel) const {
    return isBlockActive(row(channel),column(channel));
  }
  int numChannels() const {return m_numChannels;}
	
  DVector2 positionOnFace( int row, int column ) const;
  DVector2 positionOnFace( int channel ) const;
  double fcalFrontZ() const {return m_FCALfront;}

  int channel( int row, int column ) const;
  
  int row   ( int channel ) const { return m_row[channel];    }
  int column( int channel ) const { return m_column[channel]; }

  // get row and column from x and y positions
  int y_to_row   ( float y ) const;
  int x_to_column( float x ) const;

  void Summarize(JObjectSummary& summary) const override {
    summary.add((int)kBlocksWide, "kBlocksWide", "%d");
    summary.add((int)kBlocksTall, "kBlocksTall", "%d");
    summary.add((int)kMaxChannels, "kMaxChannels", "%d");
    summary.add((int)kBeamHoleSize, "kBeamHoleSize", "%2.3f");
  }
	
protected:
  bool   m_activeBlock[kBlocksTall][kBlocksWide];
  DVector2 m_positionOnFace[kBlocksTall][kBlocksWide];

  int    m_channelNumber[kBlocksTall][kBlocksWide];
  int    m_row[kMaxChannels];
  int    m_column[kMaxChannels];
  int m_numChannels;

  // global offsets
  double m_FCALdX,m_FCALdY,m_FCALfront;
  // global rotation angles
  double m_FCALthetaX,m_FCALthetaY,m_FCALthetaZ;

 private:
  void GetGridGeometry(const DGeometry *geom);
  void GetSurveyGeometry(const DGeometry *geom);
  DFCALGeometry(){};// force use of constructor with arguments.
};

#endif // _DFCALGeometry_

