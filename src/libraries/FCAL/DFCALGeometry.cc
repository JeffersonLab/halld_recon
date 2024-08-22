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

  // Find the size of the sensitive volume of each lead glass block
  vector<double>block;
  geom->Get("//box[@name='LGBL']/@X_Y_Z",block);
  m_sensitiveBlockSize=block[0];
 
  // Check for presence of PbWO4 insert
  if (geom->HaveInsert()){ // Geometry based on survey data for positions
    // Get size of the insert 
    int insert_row_size=0;
    geom->GetFCALInsertRowSize(insert_row_size);
    m_insertSize=insertBlockSize()*double(insert_row_size/2);

    // Find the size of the sensitive volume of each PWO crystal
    geom->Get("//box[@name='XTBL']/@X_Y_Z",block);
    m_insertSensitiveBlockSize=block[0];

    // Get the z-position of the upstream face of the insert
    geom->GetECALZ(m_insertFront);

    // Get the x-y positions of the crystals
    GetSurveyGeometry(geom);
  }
  else{ // Geometry based on grid(s) of fixed dimensions and block sizes
    GetGridGeometry(geom);
  }
}

// The following routine looks up the position of each row in the xml
// specification for the geometry
void DFCALGeometry::GetSurveyGeometry(const DGeometry *geom){
  unsigned int ch=0;
  
  // Initialize active block map
  for( int row = 0; row < 2*kBlocksTall; row++ ){
    for( int col = 0; col < 2*kBlocksWide; col++ ){
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
	m_row[ch]     =  row;
	m_column[ch]  =  col;
	m_channelNumber[row][col]=ch;
	m_activeBlock[row][col] = true;
	
	ch++;
      }
    }
  }
  m_numFcalChannels=ch;
  
  // Now extract the positions of the PWO crystals 
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
      double x=m_FCALdX+x0+double(col-col0)*dx;
      double y=m_FCALdY+pos[1]+phi*x; //use small angle approximation

      int row_index=my_row+kBlocksTall;
      int col_index=col+kBlocksWide;
      m_positionOnFace[row_index][col_index].Set(x,y);
      m_row[ch]     =  row_index;
      m_column[ch]  =  col_index;
      m_activeBlock[row_index][col_index] = true;
      m_channelNumber[row_index][col_index] = ch;

      ch++;
    }
  }

  m_numChannels=ch;
}

// The following routine calculates the position of each block assuming a 
// square 59x59 grid.  Blocks in the outer corners are flagged to be ignored.
void DFCALGeometry::GetGridGeometry(const DGeometry *geom){
  // Old geometry 
  double innerRadius = ( kBeamHoleSize - 1 ) / 2. * blockSize() * sqrt(2.);
  
  // inflate the innner radius by 1% to for "safe" comparison
  innerRadius *= 1.01;
  int insert_row_size=0;
  geom->GetFCALInsertRowSize(insert_row_size);
  m_insertSize=insertBlockSize()*double(insert_row_size/2);
 
  DVector2 XY0(m_FCALdX,m_FCALdY);

  // The following is for backward compatibility with an older model for the
  // FCAL insert, now superceded by a more realistic model of the geometry
  // (SJT 4/30/24)
  double back=m_FCALfront+blockLength();
  m_insertFront=0.5*(back+m_FCALfront-insertBlockLength());
  
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
  m_numFcalChannels=ch;
 
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
    return kBlocksTall+static_cast<int>( y / insertBlockSize() + m_insertMidBlock )+1;
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
    return kBlocksWide+static_cast<int>( x / insertBlockSize() + m_insertMidBlock )+1;
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
  if (channel<m_numFcalChannels) return false;
  return true;
}

bool DFCALGeometry::isInsertBlock(int row,int column) const{
  if (row>=100&&column>=100) return true;
  return false;
}

bool DFCALGeometry::hitPairHasInsertHit(int row1, int row2) const {
  if (row1>=100 && row2<100) return true;
  if (row2>=100 && row1<100) return true;
  return false;
}
