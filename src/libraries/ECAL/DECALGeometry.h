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
  static const int kECALMaxChannels  = kECALBlocksWide * kECALBlocksTall;
  static const int kECALBeamHoleSize = 2;

  static double blockSize()  { return  2.09 * k_cm; }
  static double blockLength(){ return  20.0 * k_cm; }
  
  int numActiveBlocks() const { return m_numActiveBlocks; }
  DVector2 positionOnFace(int row,int column) const {return m_positionOnFace[row][column];}
  DVector2 positionOnFace( int channel ) const {return positionOnFace( m_row[channel], m_column[channel]);} 
  double sensitiveBlockSize() const {return m_sensitiveBlockSize;}
  double insertFrontZ() const {return m_insertFrontZ;}
  
  int channel( int row, int column ) const {return m_channelNumber[row][column];} 
  int row   ( int channel ) const { return m_row[channel];    }
  int column( int channel ) const { return m_column[channel]; }
	
  // get row and column from x and y positions
  int row   ( double y ) const;
  int column( double x ) const;
  // get channel number from x and y
  int channel(double x,double y) const;
  
  void Summarize(JObjectSummary& summary) const{
    summary.add((int) kECALBlocksWide, "kECALBlocksWide", "%d");
    summary.add((int) kECALBlocksTall, "kECALBlocksTall", "%d");
    summary.add((int) kECALMaxChannels, "kECALMaxChannels", "%d");
    summary.add((int) kECALBeamHoleSize, "kECALBeamHoleSize", "%2.3f");
  }
  
private:
  DVector2 m_positionOnFace[kECALBlocksTall][kECALBlocksWide];
  double m_insertFrontZ;
  double m_sensitiveBlockSize;
  
  int    m_channelNumber[kECALBlocksTall][kECALBlocksWide];
  int    m_row[kECALMaxChannels];
  int    m_column[kECALMaxChannels];
  
  int    m_numActiveBlocks;

  DECALGeometry(){};// force use of constructor with arguments.
};

#endif // _DECALGeometry_

