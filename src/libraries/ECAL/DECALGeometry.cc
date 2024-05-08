// $Id$
//
/*
 *  File: DECALHit_factory.h
 *
 *  Created on 01/16/2024 by A.S.  
 */


#include <cassert>
#include <math.h>
using namespace std;

#include "DECALGeometry.h"
#include "DVector2.h"

//---------------------------------
// DECALGeometry    (Constructor)
//---------------------------------
DECALGeometry::DECALGeometry(const DGeometry *geom):
  m_numActiveBlocks( 0 ){

  // Get positions for each crystal from xml
  for( int i = 0; i < kECALBlocksTall+2; i++ ){
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
      double x=x0+double(col-col0)*dx;
      double y=pos[1]+phi*x; //use small angle approximation
      m_positionOnFace[my_row][col].Set(x,y);
      m_row[m_numActiveBlocks]     =  my_row;
      m_column[m_numActiveBlocks]  =  col;
      m_activeBlock[my_row][col] = true;
      
      m_numActiveBlocks++;
    }
  }
  m_activeBlock[19][19]=m_activeBlock[19][20]=false;
  m_activeBlock[20][19]=m_activeBlock[20][20]=false;
}

bool
DECALGeometry::isBlockActive( int row, int column ) const
{
  if( row < 0 ||  row >= kECALBlocksTall )return false;
  if( column < 0 ||  column >= kECALBlocksWide )return false;
	
  return m_activeBlock[row][column];	
}

int
DECALGeometry::row( float y ) const 
{	
  return static_cast<int>( y / blockSize() + kECALMidBlock );
}

int
DECALGeometry::column( float x ) const 
{	
  return static_cast<int>( x / blockSize() + kECALMidBlock );
}

DVector2
DECALGeometry::positionOnFace( int row, int column ) const
{ 
  return m_positionOnFace[row][column]; 
}

DVector2
DECALGeometry::positionOnFace( int channel ) const
{
  return positionOnFace( m_row[channel], m_column[channel] );
}

int
DECALGeometry::channel( int row, int column ) const
{
  if( isBlockActive( row, column ) ){
    return m_channelNumber[row][column]; 
  }
  else{    
    cerr << "ERROR: request for channel number of inactive block!" << endl;
    return -1;
  }
}
