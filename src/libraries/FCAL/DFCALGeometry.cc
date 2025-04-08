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
  // Find position of upstream face of FCAL
  geom->GetFCALPosition(m_FCALdX,m_FCALdY,m_FCALfront);

  // For backward compatibility, get grid-based geometry.  Override this
  // geometry if the insert is present.
  GetGridGeometry(geom);
  
  // Check for presence of PbWO4 insert
  if (geom->HaveInsert()){ // Geometry based on survey data for positions
    // Get the x-y positions of the crystals
    GetSurveyGeometry(geom);
  }
}

// The following routine looks up the position of each row in the xml
// specification for the geometry
void DFCALGeometry::GetSurveyGeometry(const DGeometry *geom){
  // Initialize active block map
  for( int row = 0; row < kBlocksTall; row++ ){
    for( int col = 0; col < kBlocksWide; col++ ){
      m_activeBlock[row][col]=false;
    }
  }

  // Extract block positions for each section of the lead glass from xml
  string section_names[4]={"LGLowerRow","LGUpperRow","LGNorthRow","LGSouthRow"};
  int num_rows=19;
  for (int i=0;i<4;i++){
    if (i>1) num_rows=38;
    for (int j=0;j<num_rows;j++){
      string my_row_string=section_names[i]+to_string(j);
      string my_mpos_string="//composition[@name='"+my_row_string
	+"']/mposX[@volume='LGDblock']/";
      int ncopy=0,col0=0,row=0;
      double x0=0,dx=0.;
      geom->Get(my_mpos_string+"@ncopy",ncopy);
      geom->Get(my_mpos_string+"column/@value",col0); 
      geom->Get(my_mpos_string+"row/@value",row);
      geom->Get(my_mpos_string+"@X0",x0);
      geom->Get(my_mpos_string+"@dX",dx);
      string my_pos_string="//posXYZ[@volume='"+my_row_string+"']/";
      vector<double>pos;
      geom->Get(my_pos_string+"@X_Y_Z",pos);  
      //vector<double>rot;
      //geom->Get(my_pos_string+"@rot",rot);
      //double phi=rot[2]*M_PI/180.;
      for (int col=col0;col<col0+ncopy;col++){
	double x=m_FCALdX+x0+double(col-col0)*dx;
	double y=m_FCALdY+pos[1];//+phi*x; //use small angle approximation
	m_positionOnFace[row][col].Set(x,y);
	m_activeBlock[row][col] = true;
      }
    }
  }
}

// The following routine calculates the position of each block assuming a 
// square 59x59 grid.  Blocks in the outer corners are flagged to be ignored.
void DFCALGeometry::GetGridGeometry(const DGeometry *geom){
  // Old geometry 
  double innerRadius = ( kBeamHoleSize - 1 ) / 2. * blockSize() * sqrt(2.);
  
  // inflate the inner radius by 1% to for "safe" comparison
  innerRadius *= 1.01;
  
  DVector2 XY0(m_FCALdX,m_FCALdY);
  
  // Initilize the list of active blocks to false, to be adjusted for the
  // actual geometry below.
  for( int row = 0; row < kBlocksTall; row++ ){
    for( int col = 0; col < kBlocksWide; col++ ){	
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
	m_activeBlock[row][col] = true;
	m_positionOnFace[row][col]+=XY0; // add FCAL offsets
     
	// build the "channel map"
	m_channelNumber[row][col] = ch;
	m_row[ch] = row;
	m_column[ch] = col;
	
	ch++;
      }
    }
  }
  m_numChannels=ch;
}

bool
DFCALGeometry::isBlockActive( int row, int column ) const
{
  if (row >= 0 && row < kBlocksWide && column >= 0 && column < kBlocksWide) {
	return m_activeBlock[row][column];	
   }
   return false;
}

DVector2
DFCALGeometry::positionOnFace( int row, int column ) const
{ 
  return m_positionOnFace[row][column]; 
}

DVector2
DFCALGeometry::positionOnFace( int channel ) const
{
  return positionOnFace( m_row[channel], m_column[channel] );
}

int
DFCALGeometry::channel( int row, int column ) const
{
  if( isBlockActive( row, column ) ){
    return m_channelNumber[row][column]; 
  }
  else{
    
    // cerr << "ERROR: request for channel number of inactive block!  row "
    //      << row << " column " <<  column << endl;
    return -1;
  }
}

int
DFCALGeometry::y_to_row( float y ) const 
{      
  y-=m_FCALdY;

  int my_row=static_cast<int>( y / blockSize() + kMidBlock + 0.5);
  if (my_row<0) return -1;
  if (my_row>=kBlocksTall) return -1;
  return my_row;
}

int
DFCALGeometry::x_to_column( float x ) const 
{      
  x-=m_FCALdX;

  int my_col=static_cast<int>( x / blockSize() + kMidBlock + 0.5);
  if (my_col<0) return -1;
  if (my_col>=kBlocksWide) return -1;
  return my_col;
}
