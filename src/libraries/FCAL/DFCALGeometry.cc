// $Id$
//
//    File: DFCALGeometry.cc
// Created: Wed Aug 24 10:18:56 EST 2005
// Creator: shepherd (on Darwin 129-79-159-16.dhcp-bl.indiana.edu 8.2.0 powerpc)
//

#include <cassert>
#include <math.h>
using namespace std;

#include "DFCALGeometry.h"
#include "DVector2.h"

//---------------------------------
// DFCALGeometry    (Constructor)
//---------------------------------
DFCALGeometry::DFCALGeometry(const DGeometry *geom){
  double innerRadius = ( kBeamHoleSize - 1 ) / 2. * blockSize() * sqrt(2.);
  
  // inflate the innner radius by 1% to for "safe" comparison
  innerRadius *= 1.01;
  
  // Check for presence of PbWO4 insert
  int insert_row_size=0;
  geom->GetFCALInsertRowSize(insert_row_size);
  m_insertSize=insertBlockSize()*double(insert_row_size/2);

  geom->GetFCALPosition(m_FCALdX,m_FCALdY,m_FCALfront);
  DVector2 XY0(m_FCALdX,m_FCALdY);

  vector<double>block;
  geom->GetFCALBlockSize(block);
  double back=m_FCALfront+block[2];
  geom->GetFCALInsertBlockSize(block);
  m_insertFront=0.5*(back+m_FCALfront-block[2]);
  
  // Initilize the list of active blocks to false, to be adjusted for the
  // actual geometry below.
  for( int row = 0; row < 2*kBlocksTall; row++ ){
    for( int col = 0; col < 2*kBlocksWide; col++ ){	
      m_activeBlock[row][col] = false;
    }
  }

  // Now fill in the data for the actual geometry
  unsigned int ch=0;
  for( int row = 0; row < kBlocksTall; row++ ){
    for( int col = 0; col < kBlocksWide; col++ ){
			
      // transform to beam axis
      m_positionOnFace[row][col] = 
	DVector2(  ( col - kMidBlock ) * blockSize(),
		   ( row - kMidBlock ) * blockSize());
      
      double thisRadius = m_positionOnFace[row][col].Mod();
			
      if( ( thisRadius < radius() ) && ( thisRadius > innerRadius ) 	 
	  ){
	// Carve out region for insert, if present.  For back compatibility
	// we set these blocks as inactive but maintain the length of the array
	if (fabs(m_positionOnFace[row][col].X())>m_insertSize || 
	    fabs(m_positionOnFace[row][col].Y())>m_insertSize){ 
	  m_activeBlock[row][col] = true;
	  m_positionOnFace[row][col]+=XY0; // add FCAL offsets
	}
	// build the "channel map"
	m_channelNumber[row][col] = ch;
	m_row[ch] = row;
	m_column[ch] = col;
	
	ch++;
      }
    }
  }
 
  if (insert_row_size>0){
    m_insertMidBlock=(insert_row_size-1)/2;
    for( int row = 0; row < insert_row_size; row++ ){
      for( int col = 0; col < insert_row_size; col++ ){
	
	// transform to beam axis
	int row_index=row+kBlocksTall;
	int col_index=col+kBlocksWide;
	m_positionOnFace[row_index][col_index] = 
	DVector2(  ( col - m_insertMidBlock -0.5) * insertBlockSize(),
		   ( row - m_insertMidBlock -0.5) * insertBlockSize() );
	      
	if( fabs(m_positionOnFace[row_index][col_index].X())>insertBlockSize()
	    || fabs(m_positionOnFace[row_index][col_index].Y())>insertBlockSize()
	    ){
	  m_activeBlock[row_index][col_index] = true;
	  m_positionOnFace[row_index][col_index]+=XY0; // add FCAL offsets
	  
	  // build the "channel map"
	  m_channelNumber[row_index][col_index] = ch;
	  m_row[ch] = row_index;
	  m_column[ch] = col_index;
	  
	  ch++;
	}
      }
    }
  }
  m_numChannels=ch;
}

bool
DFCALGeometry::isBlockActive( int row, int column ) const
{
   if (row>=100&&column>=100){
    row-=100-kBlocksTall;
    column-=100-kBlocksWide;
   }
   if (row >= 0 && row < 2*kBlocksWide && column >= 0 && column < 2*kBlocksWide) {
	return m_activeBlock[row][column];	
   }
   return false;
}

int
DFCALGeometry::row( float y, bool in_insert ) const 
{	
  y-=m_FCALdY;

  if (in_insert){
    return kBlocksTall+static_cast<int>( y / insertBlockSize() + m_insertMidBlock + 0.5);
  }
  int my_row=static_cast<int>( y / blockSize() + kMidBlock + 0.5);
  if (my_row<0) return -1;
  if (my_row>=kBlocksTall) return -1;
  return my_row;
}

int
DFCALGeometry::column( float x, bool in_insert ) const 
{	
  x-=m_FCALdX;
  
  if (in_insert){
    return kBlocksWide+static_cast<int>( x / insertBlockSize() + m_insertMidBlock + 0.5);
  }
  int my_col=static_cast<int>( x / blockSize() + kMidBlock + 0.5);
  if (my_col<0) return -1;
  if (my_col>=kBlocksWide) return -1;
  return my_col;
}

DVector2
DFCALGeometry::positionOnFace( int row, int column ) const
{ 
  //	assert(    row >= 0 &&    row < kBlocksTall );
  //	assert( column >= 0 && column < kBlocksWide );
  // Check for insert blocks
  if (row>=100&&column>=100){
    row-=100-kBlocksTall;
    column-=100-kBlocksWide;
  }
  return m_positionOnFace[row][column]; 
}

DVector2
DFCALGeometry::positionOnFace( int channel ) const
{
  return positionOnFace( m_row[channel], m_column[channel] );
}

int
DFCALGeometry::channel(double x,double y) const {
  bool isInsert=(fabs(x-m_FCALdX)<m_insertSize&& fabs(y-m_FCALdY)<m_insertSize);
  return channel(row(y,isInsert),column(x,isInsert));
}

int
DFCALGeometry::channel( int row, int column ) const
{
  // Check for insert blocks
  if (row>=100&&column>=100){
    row-=100-kBlocksTall;
    column-=100-kBlocksWide;
  }
	if( isBlockActive( row, column ) ){
		
		return m_channelNumber[row][column]; 
	}
	else{
		
	  // cerr << "ERROR: request for channel number of inactive block!  row "
	  //      << row << " column " <<  column << endl;
		return -1;
	}
}

bool DFCALGeometry::isFiducial(double x,double y) const{
  bool isInsert=(fabs(x-m_FCALdX)<m_insertSize&& fabs(y-m_FCALdY)<m_insertSize);
  return (row(y,isInsert)>=0 && column(x,isInsert)>=0);
}


bool DFCALGeometry::inInsert(int channel) const{
  if (fabs(positionOnFace(channel).X()-m_FCALdX)<m_insertSize
      && fabs(positionOnFace(channel).Y()-m_FCALdY)<m_insertSize){
    return true;
  }
  return false;
}

bool DFCALGeometry::isInsertBlock(int row,int column) const{
  if (row>=100&&column>=100) return true;
  return false;
}
