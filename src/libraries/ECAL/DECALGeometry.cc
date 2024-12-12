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
DECALGeometry::DECALGeometry() : 
  m_numActiveBlocks( 0 ){
  
        for( int row = 0; row < kECALBlocksTall; row++ ){
	  for( int col = 0; col < kECALBlocksWide; col++ ){
      
	    // transform to beam axis
	    m_positionOnFace[row][col] = 
	      DVector2(  ( (double)col - kECALMidBlock  + 0.5 ) * blockSize(),
			 ( (double)row - kECALMidBlock + 0.5 ) * blockSize() );
	    
	    m_activeBlock[row][col] = true;
      
	    // build the "channel map"
	    m_channelNumber[row][col]    =  m_numActiveBlocks;
	    m_row[m_numActiveBlocks]     =  row;
	    m_column[m_numActiveBlocks]  =  col;
	    
	    m_numActiveBlocks++;
	  }
	}
}

bool
DECALGeometry::isBlockActive( int row, int column ) const
{
	// I'm inserting these lines to effectively disable the
	// two assert calls below. They are causing all programs
	// (hd_dump, hdview) to exit, even when I'm not interested
	// in the FCAL. This does not fix the underlying problem
	// of why we're getting invalid row/column values.
	// 12/13/05  DL
	if( row < 0 ||  row >= kECALBlocksTall )return false;
	if( column < 0 ||  column >= kECALBlocksWide )return false;

	assert(    row >= 0 &&    row < kECALBlocksTall );
	assert( column >= 0 && column < kECALBlocksWide );
	
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
	assert(    row >= 0 &&    row < kECALBlocksTall );
	assert( column >= 0 && column < kECALBlocksWide );
	
	return m_positionOnFace[row][column]; 
}

DVector2
DECALGeometry::positionOnFace( int channel ) const
{
	assert( channel >= 0 && channel < m_numActiveBlocks );
	
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
