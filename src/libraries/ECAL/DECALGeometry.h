// $Id$
//
//    File: DECALGeometry.h
// Created: Tue Nov 30 15:42:41 EST 2010
// Creator: davidl (on Linux ifarml6 2.6.18-128.el5 x86_64)
//

#ifndef _DECALGeometry_
#define _DECALGeometry_

#include <JANA/JObject.h>
#include <JANA/JFactoryT.h>

#include "DVector2.h"
#include "units.h"
#include <HDGEOMETRY/DGeometry.h>

class DECALGeometry:public JObject{
public:
  JOBJECT_PUBLIC(DECALGeometry);
  
  DECALGeometry(const DGeometry *geom);
  ~DECALGeometry(){}
  
  static const int kECALBlocksWide   = 40;
  static const int kECALBlocksTall   = 40;
  static const int kECALMidBlock     = 20;
  static const int kECALMaxChannels  = kECALBlocksWide * kECALBlocksTall;
  static const int kECALBeamHoleSize = 2;

  static double blockSize()  { return  2.09 * k_cm; }
  static double blockLength(){ return  20.0 * k_cm; }
  bool isBlockActive( int row, int column ) const;
  
  DVector2 positionOnFace(int row,int column) const {return m_positionOnFace[row][column];}
  DVector2 positionOnFace( int channel ) const {return positionOnFace( m_row[channel], m_column[channel]);} 
  double sensitiveBlockSize() const {return m_sensitiveBlockSize;}
  double insertFrontZ() const {return m_insertFrontZ;}
  bool isFiducial(double x,double y) const;
  
  int channel( int row, int column ) const {return m_channelNumber[row][column];} 
  int row   ( int channel ) const { return m_row[channel];    }
  int column( int channel ) const { return m_column[channel]; }
	
  // get row and column from x and y positions
  int y_to_row   ( double y ) const;
  int x_to_column( double x ) const;
  
  void Summarize(JObjectSummary& summary) const{
    summary.add((int) kECALBlocksWide, "kECALBlocksWide", "%d");
    summary.add((int) kECALBlocksTall, "kECALBlocksTall", "%d");
    summary.add((int) kECALMaxChannels, "kECALMaxChannels", "%d");
    summary.add((int) kECALBeamHoleSize, "kECALBeamHoleSize", "%2.3f");
  }
  
private:
  DVector2 m_positionOnFace[kECALBlocksTall][kECALBlocksWide];
  double m_insertFrontZ,m_FCALx,m_FCALy;
  double m_xmin,m_ymin,m_xmax,m_ymax;
  double m_sensitiveBlockSize;

  bool   m_activeBlock[kECALBlocksTall][kECALBlocksWide];
  int    m_channelNumber[kECALBlocksTall][kECALBlocksWide];
  int    m_row[kECALMaxChannels];
  int    m_column[kECALMaxChannels];
  
  DECALGeometry(){};// force use of constructor with arguments.
};

inline int DECALGeometry::y_to_row( double y ) const {
  y-=m_FCALy;
  
  int my_row=static_cast<int>( y / blockSize() + kECALMidBlock + 0.5);
  if (my_row<0) return -1;
  if (my_row>=kECALBlocksTall) return -1;
  
  return my_row;
}

inline int DECALGeometry::x_to_column( double x ) const {
  x-=m_FCALx;

  int my_col=static_cast<int>( x / blockSize() + kECALMidBlock + 0.5);
  if (my_col<0) return -1;
  if (my_col>=kECALBlocksWide) return -1;
  
  return my_col;
}

// Check that x and y are in the bounds of the detector
inline bool DECALGeometry::isFiducial(double x,double y) const {
  if (x<m_xmax && x>m_xmin && y<m_ymax && y>m_ymin) return true;
  
  return false;
}

inline bool DECALGeometry::isBlockActive( int row, int column ) const {
  if (row>=0 && row<kECALBlocksTall && column >= 0 && column<kECALBlocksWide){
    return m_activeBlock[row][column];
  }
  return false;
}


#endif // _DECALGeometry_

