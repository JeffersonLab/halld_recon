// $Id$
//
//    File: DECALGeometry.cc
// Created: Tue Mar 25 04:27:18 PM EDT 2025
// Creator: staylor (on Linux ifarm2401.jlab.org 5.14.0-503.19.1.el9_5.x86_64 x86_64)
//

using namespace std;

#include "DECALGeometry.h"

//---------------------------------
// DFCALGeometry    (Constructor)
//---------------------------------
DECALGeometry::DECALGeometry(const DGeometry *geom){
  // Find position of upstream face of FCAL
  double fcal_z_front=0.;
  geom->GetFCALPosition(m_FCALx,m_FCALy,fcal_z_front);

  // Find the size of the sensitive volume of each PWO crystal
  vector<double>block;
  geom->Get("//box[@name='XTBL']/@X_Y_Z",block);
  m_sensitiveBlockSize=block[0];
  
  // Get the z-position of the upstream face of the insert
  geom->GetECALZ(m_insertFrontZ);
  
  // Initialize active block map
  int ch=0;
  for( int row = 0; row < kECALBlocksTall; row++ ){
    for( int col = 0; col < kECALBlocksWide; col++ ){
      m_activeBlock[row][col]=false;
      m_channelNumber[row][col]=ch;
      m_row[ch]=row;
      m_column[ch]=col;
      ch++;
    }
  }
  
  // extract the positions of the PWO crystals
  for( int i = 0; i < 42; i++ ){
    string my_row_string="XTrow"+to_string(i);
    string my_mpos_string="//composition[@name='"+my_row_string
      +"']/mposX[@volume='XTModule']/";
    int ncopy=0;
    double x0=0,dx=0.;
    geom->Get(my_mpos_string+"@ncopy",ncopy);
    geom->Get(my_mpos_string+"@X0",x0);
    geom->Get(my_mpos_string+"@dX",dx);
    string my_pos_string="//posXYZ[@volume='"+my_row_string+"']/";
    vector<double>pos;
    geom->Get(my_pos_string+"@X_Y_Z",pos);  
    vector<double>rot;
    geom->Get(my_pos_string+"@rot",rot);
    double phi=rot[2]*M_PI/180.;
    int col0=0,my_row=i;
    if (i>=40){ //handle right side of beam hole
      col0=21;
      my_row=i-21;
    }
    for (int col=col0;col<col0+ncopy;col++){
      double x=m_FCALx+x0+double(col-col0)*dx;
      double y=m_FCALy+pos[1]+phi*x; //use small angle approximation

      m_positionOnFace[my_row][col].Set(x,y);
      m_activeBlock[my_row][col] = true;
    }
  }
  
  // outer dimensions
  m_xmax=m_positionOnFace[39][39].X()+0.5*blockSize();
  m_xmin=m_positionOnFace[0][0].X()-0.5*blockSize();
  m_ymax=m_positionOnFace[39][39].Y()+0.5*blockSize();
  m_ymin=m_positionOnFace[0][0].Y()-0.5*blockSize();
}
