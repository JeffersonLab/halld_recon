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
DFCALGeometry::DFCALGeometry(const DGeometry *geom) : 
m_numActiveBlocks( 0 )
{
  double innerRadius = ( kBeamHoleSize - 1 ) / 2. * blockSize() * sqrt(2.);
  
  // inflate the innner radius by 1% to for "safe" comparison
  innerRadius *= 1.01;
  
  // Check for presence of PbWO4 insert
  int insert_row_size=0;
  geom->Get("//composition[@name='LeadTungstateFullRow']/mposX[@volume='LTBLwrapped']/@ncopy",insert_row_size);
  m_insertSize=insertBlockSize()*double(insert_row_size/2);
  
  // Initilize the list of active blocks to false, to be adjusted for the
  // actual geometry below.
  for( int row = 0; row < 2*kBlocksTall; row++ ){
    for( int col = 0; col < 2*kBlocksWide; col++ ){	
      m_activeBlock[row][col] = false;
    }
  }
  // Now fill in the data for the actual geometry
  for( int row = 0; row < kBlocksTall; row++ ){
    for( int col = 0; col < kBlocksWide; col++ ){
			
      // transform to beam axis
      m_positionOnFace[row][col] = 
	DVector2(  ( col - kMidBlock ) * blockSize(),
		   ( row - kMidBlock ) * blockSize() );
      
      double thisRadius = m_positionOnFace[row][col].Mod();
			
      if( ( thisRadius < radius() ) && ( thisRadius > innerRadius ) 
	  && (fabs(m_positionOnFace[row][col].X())>m_insertSize || 
	      fabs(m_positionOnFace[row][col].Y())>m_insertSize) 
	  ){

	m_activeBlock[row][col] = true;
	
	// build the "channel map"
	m_channelNumber[row][col] = m_numActiveBlocks;
	m_row[m_numActiveBlocks] = row;
	m_column[m_numActiveBlocks] = col;
	
	m_numActiveBlocks++;
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
	  
	  // build the "channel map"
	  m_channelNumber[row_index][col_index] = m_numActiveBlocks;
	  m_row[m_numActiveBlocks] = row_index;
	  m_column[m_numActiveBlocks] = col_index;
	  
	  m_numActiveBlocks++;
	}
      }
    }
  }
}

bool
DFCALGeometry::isBlockActive( int row, int column ) const
{
	return m_activeBlock[row][column];	
}

int
DFCALGeometry::row( float y, bool in_insert ) const 
{	
  if (in_insert){
    return kBlocksTall+static_cast<int>( y / insertBlockSize() + m_insertMidBlock + 0.5);
  }
	return static_cast<int>( y / blockSize() + kMidBlock + 0.5);
}

int
DFCALGeometry::column( float x, bool in_insert ) const 
{	
  if (in_insert){
    return kBlocksWide+static_cast<int>( x / insertBlockSize() + m_insertMidBlock + 0.5);
  }
	return static_cast<int>( x / blockSize() + kMidBlock + 0.5);
}

DVector2
DFCALGeometry::positionOnFace( int row, int column ) const
{ 
  //	assert(    row >= 0 &&    row < kBlocksTall );
  //	assert( column >= 0 && column < kBlocksWide );
	
	return m_positionOnFace[row][column]; 
}

DVector2
DFCALGeometry::positionOnFace( int channel ) const
{
	assert( channel >= 0 && channel < m_numActiveBlocks );
	
	return positionOnFace( m_row[channel], m_column[channel] );
}

int
DFCALGeometry::channel( int row, int column ) const
{
	if( isBlockActive( row, column ) ){
		
		return m_channelNumber[row][column]; 
	}
	else{
		
	  cerr << "ERROR: request for channel number of inactive block!  row " 
	       << row << " column " <<  column << endl;
		return -1;
	}
}
