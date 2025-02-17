// $Id$
//
//    File: DMagneticFieldMapFineMesh.cc

#include <unistd.h>
#include <sys/stat.h>
#include <cmath>
using namespace std;
#ifdef HAVE_EVIO
#include <evioFileChannel.hxx>
#include <evioUtil.hxx>
using namespace evio;
#endif

#include "DMagneticFieldMapFineMesh.h"

#include <JANA/JException.h>
#include <JANA/Calibrations/JCalibrationManager.h>

#include <DAQ/HDEVIO.h>


//---------------------------------
// DMagneticFieldMapFineMesh    (Constructor)
//---------------------------------
DMagneticFieldMapFineMesh::DMagneticFieldMapFineMesh(JApplication *app, int32_t runnumber, string namepath)
{
	auto calib_svc = app->GetService<JCalibrationManager>();
	jcalib = calib_svc->GetJCalibration(runnumber);
	jresman = calib_svc->GetLargeCalibration(runnumber);

	japp->SetDefaultParameter("BFIELD_MAP", namepath);
	
	int Npoints = ReadMap(namepath, runnumber); 
	if(Npoints==0){
	  _DBG_<<"Error getting JCalibration object for magnetic field!"<<endl;
	  japp->Quit();
	}
	
	GetFineMeshMap(namepath,runnumber);
}

//---------------------------------
// DMagneticFieldMapFineMesh    (Constructor)
//---------------------------------
DMagneticFieldMapFineMesh::DMagneticFieldMapFineMesh(JCalibration *jcalib, string namepath,int32_t runnumber)
{
	this->jcalib = jcalib;
	GetFineMeshMap(namepath,runnumber);
}

//---------------------------------
// ~DMagneticFieldMapFineMesh    (Destructor)
//---------------------------------
DMagneticFieldMapFineMesh::~DMagneticFieldMapFineMesh()
{

}

//---------------------------------
// ReadMap
//---------------------------------
int DMagneticFieldMapFineMesh::ReadMap(string namepath, int32_t runnumber, string context)
{
 /// Read the magnetic field map in from the calibration database.
  /// This will read in the map and figure out the number of grid
  /// points in each direction (x,y, and z) and the range in each.
  /// The gradiant of the field is calculated for all but the most
  /// exterior points and saved to use in later calls to GetField(...).

  // Read in map from calibration database. This should really be
  // built into a run-dependent geometry framework, but for now
  // we do it this way. 
  if(!jcalib){
    jerr << "ERROR: jcalib pointer is NULL in DMagneticFieldMapFineMesh::ReadMap() !" << jendl;
    return 0;
  }
  
  jout<<"Reading Magnetic field map from "<<namepath<<" ..."<<jendl;
  vector< vector<float> > Bmap;
  
  // Newer maps are stored as resources while older maps were stored
  // directly in the calib DB. Here, we hardwire in the names of the old
  // maps that are stored directly in the DB so we know whether to
  // get them via the resource manager or not.
  vector<string> old_maps;
  old_maps.push_back("Magnets/Solenoid/solenoid_1500_poisson_20090814_01");
  old_maps.push_back("Magnets/Solenoid/solenoid_1050_poisson_20091123_02");
  old_maps.push_back("Magnets/Solenoid/solenoid_1500_poisson_20090814_01");
  old_maps.push_back("Magnets/Solenoid/solenoid_1500_20090312-2");
  old_maps.push_back("Magnets/Solenoid/solenoid_1600_20090312-2");
  old_maps.push_back("Magnets/Solenoid/solenoid_1500kinked");
  
  bool is_old_map = false;
  for(unsigned int i=0; i<old_maps.size(); i++){
  	if(old_maps[i] == namepath){

		// This map is stored directly in the calibration DB.
		jcalib->Get(namepath, Bmap);

		is_old_map = true;
		break;
	}
  }

  if(!is_old_map){
    // This map is NOT stored directly in the calibration DB.
	// Try getting it as a JANA resource.
	jresman->Get(namepath, Bmap);
  }
  
  
  jout<<Bmap.size()<<" entries found (";
  
  // The map should be on a grid with equal spacing in x,y, and z.
  // Here we want to determine the number of points in each of these
  // dimensions and the range. Note that all of or maps right now are
  // 2-dimensional with extent only in x and z.
  // The easiest way to do this is to use a map<float, int> to make a
  // histogram of the entries by using the key to hold the extent
  // so that the number of entries will be equal to the number of
  // different values.
  map<float, int> xvals;
  map<float, int> yvals;
  map<float, int> zvals;
  xmin = ymin = zmin = 1.0E6;
  xmax = ymax = zmax = -1.0E6;
  for(unsigned int i=0; i<Bmap.size(); i++){
    vector<float> &a = Bmap[i];
    float &x = a[0];
    float &y = a[1];
    float &z = a[2];
    
    // Convert from inches to cm and shift in z by 26 inches to align
    // the origin with the GEANT defined one.
    x *= 2.54;
    y *= 2.54;
    z = (z-0.0)*2.54; // Removed 26" offset 6/22/2009 DL
    
    xvals[x] = 1;
    yvals[y] = 1;
    zvals[z] = 1;
    if(x<xmin)xmin=x;
    if(y<ymin)ymin=y;
    if(z<zmin)zmin=z;
    if(x>xmax)xmax=x;
    if(y>ymax)ymax=y;
    if(z>zmax)zmax=z;
  }
  Nx = xvals.size();
  Ny = yvals.size();
  Nz = zvals.size();
  cout<<" Nx="<<Nx;
  cout<<" Ny="<<Ny;
  cout<<" Nz="<<Nz;
  cout<<" )  at 0x"<<hex<<(unsigned long)this<<dec<<endl;
  
  // Create 3D vector so we can index the values by [x][y][z]
  vector<DBfieldPoint_t> zvec(Nz);
  vector< vector<DBfieldPoint_t> > yvec;
  for(int i=0; i<Ny; i++)yvec.push_back(zvec);
  for(int i=0; i<Nx; i++)Btable.push_back(yvec);
	
  // Distance between map points for r and z
  dx = (xmax-xmin)/(double)(Nx-1);
  dy = (ymax-ymin)/(double)((Ny < 2)? 1 : Ny-1);
  dz = (zmax-zmin)/(double)(Nz-1);
  
  one_over_dx=1./dx;
  one_over_dz=1./dz;

  // Copy values into Btable
  for(unsigned int i=0; i<Bmap.size(); i++){
    vector<float> &a = Bmap[i];
    int xindex = (int)floor((a[0]-xmin+dx/2.0)/dx); // the +dx/2.0 guarantees against round-off errors
    int yindex = (int)(Ny<2 ? 0:floor((a[1]-ymin+dy/2.0)/dy));
    int zindex = (int)floor((a[2]-zmin+dz/2.0)/dz);
    DBfieldPoint_t *b = &Btable[xindex][yindex][zindex];
    b->x = a[0];
    b->y = a[1];
    b->z = a[2];
    b->Bx = a[3];
    b->By = a[4];
    b->Bz = a[5];
  }
  
  // Calculate gradient at every point in the map.
  // The derivatives are calculated wrt to the *index* of the
  // field map, not the physical unit. This is because
  // the fractions used in interpolation are fractions
  // between indices.
  for(int index_x=0; index_x<Nx; index_x++){
    int index_x0 = index_x - (index_x>0 ? 1:0);
    int index_x1 = index_x + (index_x<Nx-1 ? 1:0);
    for(int index_z=1; index_z<Nz-1; index_z++){
      int index_z0 = index_z - (index_z>0 ? 1:0);
      int index_z1 = index_z + (index_z<Nz-1 ? 1:0);
      
      // Right now, all of our maps are 2-D with no y extent
      // so we comment out the following for-loop and hardwire
      // the index_y values to zero
      //for(int index_y=1; index_y<Ny-1; index_y++){
      //	int index_y0 = index_y-1;
      //	int index_y1 = index_y+1;
      if(Ny>1){
	_DBG_<<"Field map appears to be 3 dimensional. Code is currently"<<endl;
	_DBG_<<"unable to handle this. Exiting..."<<endl;
	exit(-1);
      }else{
	int index_y=0;
	/*
	int index_y0, index_y1;
	index_y=index_y0=index_y1=0;
	*/
	double d_index_x=double(index_x1-index_x0);
	double d_index_z=double(index_z1-index_z0); 

	DBfieldPoint_t *Bx0 = &Btable[index_x0][index_y][index_z];
	DBfieldPoint_t *Bx1 = &Btable[index_x1][index_y][index_z];
	//DBfieldPoint_t *By0 = &Btable[index_x][index_y0][index_z];
	//DBfieldPoint_t *By1 = &Btable[index_x][index_y1][index_z];
	DBfieldPoint_t *Bz0 = &Btable[index_x][index_y][index_z0];
	DBfieldPoint_t *Bz1 = &Btable[index_x][index_y][index_z1];
	
	DBfieldPoint_t *g = &Btable[index_x][index_y][index_z];
	g->dBxdx = (Bx1->Bx - Bx0->Bx)/d_index_x;
	//g->dBxdy = (By1->Bx - By0->Bx)/(double)(index_y1-index_y0);
	g->dBxdz = (Bz1->Bx - Bz0->Bx)/d_index_z;
	
	g->dBydx = (Bx1->By - Bx0->By)/d_index_x;
	//g->dBydy = (By1->By - By0->By)/(double)(index_y1-index_y0);
	g->dBydz = (Bz1->By - Bz0->By)/d_index_z;
       	
	g->dBzdx = (Bx1->Bz - Bx0->Bz)/d_index_x;
	//g->dBzdy = (By1->Bz - By0->Bz)/(double)(index_y1-index_y0);
	g->dBzdz = (Bz1->Bz - Bz0->Bz)/d_index_z;

	DBfieldPoint_t *B11 = &Btable[index_x1][index_y][index_z1];
	DBfieldPoint_t *B01 = &Btable[index_x0][index_y][index_z1];	
	DBfieldPoint_t *B10 = &Btable[index_x1][index_y][index_z0];
	DBfieldPoint_t *B00 = &Btable[index_x0][index_y][index_z0];
	
	g->dBxdxdz=(B11->Bx - B01->Bx - B10->Bx + B00->Bx)/d_index_x/d_index_z;
	g->dBzdxdz=(B11->Bz - B01->Bz - B10->Bz + B00->Bz)/d_index_x/d_index_z;
      }
    }
  }
  
  return Bmap.size();
}

// Use bicubic interpolation to find the field at the point (x,y).  
//See Numerical Recipes in C (2nd ed.), pp.125-127.
void DMagneticFieldMapFineMesh::GetFieldBicubic(double x,double y,double z,
						   double &Bx_,double &By_,
						   double &Bz_) const{
  // table of weight factors for bicubic interpolation
  static const int wt[16][16]=
    { {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0},
      {-3, 0, 0, 3, 0, 0, 0, 0,-2, 0, 0,-1, 0, 0, 0, 0},
      {2, 0, 0,-2, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0},
      {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0},
      {0, 0, 0, 0,-3, 0, 0, 3, 0, 0, 0, 0,-2, 0, 0,-1},
      {0, 0, 0, 0, 2, 0, 0,-2, 0, 0, 0, 0, 1, 0, 0, 1},
      {-3,3, 0, 0,-2,-1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0,-3, 3, 0, 0,-2,-1, 0, 0},
      {9,-9, 9,-9, 6, 3,-3,-6, 6,-6,-3, 3, 4, 2, 1, 2},
      {-6,6,-6, 6,-4,-2, 2, 4,-3, 3, 3,-3,-2,-1,-1,-2},
      {2,-2, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0, 2,-2, 0, 0, 1, 1, 0, 0},
      {-6, 6,-6,6,-3,-3, 3, 3,-4, 4, 2,-2,-2,-2,-1,-1},
      {4,-4, 4,-4, 2, 2,-2,-2, 2,-2,-2, 2, 1, 1, 1, 1}};
  double temp[16];
  double cl[16],coeff[4][4];

  // radial distance and radial component of bfield 
  double r = sqrt(x*x + y*y);
  double Br_=0.;
  
  // If the point (x,y,z) is outside the fine-mesh grid, interpolate 
  // on the coarse grid
  //if (true){
  if (z<zminFine || z>=zmaxFine || r>=rmaxFine){
  // Get closest indices for this point
    int index_x = (int)floor((r-xmin)*one_over_dx + 0.5);
    //if(index_x<0 || index_x>=Nx)return;
    if (index_x>=Nx) return;
    else if (index_x<0) index_x=0;
    int index_z = (int)floor((z-zmin)*one_over_dz + 0.5);	
    if(index_z<0 || index_z>=Nz)return;
    
    int index_y = 0;
    
    int i, j, k, m=0;
    int index_x1 = index_x + (index_x<Nx-1 ? 1:0);
    int index_z1 = index_z + (index_z<Nz-1 ? 1:0);
    
    // Pointers to magnetic field structure
    const DBfieldPoint_t *B00 = &Btable[index_x][index_y][index_z];
    const DBfieldPoint_t *B01 = &Btable[index_x][index_y][index_z1];
    const DBfieldPoint_t *B11 = &Btable[index_x1][index_y][index_z1]; 
    const DBfieldPoint_t *B10 = &Btable[index_x1][index_y][index_z];
    
    // First compute the interpolation for Br
    temp[0]=B00->Bx;
    temp[1]=B01->Bx;
    temp[2]=B11->Bx;
    temp[3]=B10->Bx;
    
    temp[8]=B00->dBxdx;
    temp[9]=B01->dBxdx;
    temp[10]=B11->dBxdx;
    temp[11]=B10->dBxdx;
    
    temp[4]=B00->dBxdz;
    temp[5]=B01->dBxdz;
    temp[6]=B11->dBxdz;
    temp[7]=B10->dBxdz;
    
    temp[12]=B00->dBxdxdz;
    temp[13]=B01->dBxdxdz;
    temp[14]=B11->dBxdxdz;
    temp[15]=B10->dBxdxdz;
    
    for (i=0;i<16;i++){
      double tmp2=0.0;
      for (k=0;k<16;k++) tmp2+=wt[i][k]*temp[k];
      cl[i]=tmp2;
    }
    for (i=0;i<4;i++)
      for (j=0;j<4;j++) coeff[i][j]=cl[m++];
    
    double t=(z - B00->z)*one_over_dz;
    double u=(r - B00->x)*one_over_dx;   
    for (i=3;i>=0;i--){
      Br_=t*Br_+((coeff[i][3]*u+coeff[i][2])*u+coeff[i][1])*u+coeff[i][0];
    }
    
    // Next compute the interpolation for Bz
    temp[0]=B00->Bz;
    temp[1]=B01->Bz;
    temp[2]=B11->Bz;
    temp[3]=B10->Bz;
    
    temp[8]=B00->dBzdx;
    temp[9]=B01->dBzdx;
    temp[10]=B11->dBzdx;
    temp[11]=B10->dBzdx;
    
    temp[4]=B00->dBzdz;
    temp[5]=B01->dBzdz;
    temp[6]=B11->dBzdz;
    temp[7]=B10->dBzdz;
    
    temp[12]=B00->dBzdxdz;
    temp[13]=B01->dBzdxdz;
    temp[14]=B11->dBzdxdz;
    temp[15]=B10->dBzdxdz;
    
    for (i=0;i<16;i++){
      double tmp2=0.0;
      for (k=0;k<16;k++) tmp2+=wt[i][k]*temp[k];
      cl[i]=tmp2;
    }
    m=0;
    for (i=0;i<4;i++)
      for (j=0;j<4;j++) coeff[i][j]=cl[m++];
    
    Bz_=0.;
    for (i=3;i>=0;i--){
      Bz_=t*Bz_+((coeff[i][3]*u+coeff[i][2])*u+coeff[i][1])*u+coeff[i][0];
    }
  }
  else{ // otherwise do a simple lookup in the fine-mesh table
    unsigned int indr=(unsigned int)floor((r-rminFine)*rscale);
    unsigned int indz=(unsigned int)floor((z-zminFine)*zscale);
    
    Bz_=mBfine[indr][indz].Bz;
    Br_=mBfine[indr][indz].Br;
    //	  printf("Bz Br %f %f\n",Bz,Br);
  }

  // Convert r back to x,y components
  double cos_theta = x/r;
  double sin_theta = y/r;
  if(r==0.0){
    cos_theta=1.0;
    sin_theta=0.0;
  }
  // Rotate back into phi direction
  Bx_=Br_*cos_theta;
  By_=Br_*sin_theta;

}

// Use bicubic interpolation to find the field and field gradient at the point 
// (x,y).  See Numerical Recipes in C (2nd ed.), pp.125-127.
void DMagneticFieldMapFineMesh::InterpolateField(double r,double z,double &Br,
						 double &Bz,double &dBrdr,
						 double &dBrdz,double &dBzdr,
						 double &dBzdz) const{
  // table of weight factors for bicubic interpolation
  static const int wt[16][16]=
    { {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0},
      {-3, 0, 0, 3, 0, 0, 0, 0,-2, 0, 0,-1, 0, 0, 0, 0},
      {2, 0, 0,-2, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0},
      {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0},
      {0, 0, 0, 0,-3, 0, 0, 3, 0, 0, 0, 0,-2, 0, 0,-1},
      {0, 0, 0, 0, 2, 0, 0,-2, 0, 0, 0, 0, 1, 0, 0, 1},
      {-3,3, 0, 0,-2,-1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0,-3, 3, 0, 0,-2,-1, 0, 0},
      {9,-9, 9,-9, 6, 3,-3,-6, 6,-6,-3, 3, 4, 2, 1, 2},
      {-6,6,-6, 6,-4,-2, 2, 4,-3, 3, 3,-3,-2,-1,-1,-2},
      {2,-2, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0, 2,-2, 0, 0, 1, 1, 0, 0},
      {-6, 6,-6,6,-3,-3, 3, 3,-4, 4, 2,-2,-2,-2,-1,-1},
      {4,-4, 4,-4, 2, 2,-2,-2, 2,-2,-2, 2, 1, 1, 1, 1}};
  double temp[16];
  double cl[16],coeff[4][4];

    // Get closest indices for the point (r,z)
  int index_x = (int)floor((r-xmin)*one_over_dx + 0.5);
  if (index_x>=Nx) return;
  else if (index_x<0) index_x=0;
  int index_z = (int)floor((z-zmin)*one_over_dz + 0.5);	
  if(index_z<0 || index_z>=Nz)return; 
  int index_y = 0;
    
  int i, j, k, m=0;
  int index_x1 = index_x + (index_x<Nx-1 ? 1:0);
  int index_z1 = index_z + (index_z<Nz-1 ? 1:0);
  
  // Pointers to magnetic field structure
  const DBfieldPoint_t *B00 = &Btable[index_x][index_y][index_z];
  const DBfieldPoint_t *B01 = &Btable[index_x][index_y][index_z1];
  const DBfieldPoint_t *B11 = &Btable[index_x1][index_y][index_z1]; 
  const DBfieldPoint_t *B10 = &Btable[index_x1][index_y][index_z];
    
  // First compute the interpolation for Br
  temp[0]=B00->Bx;
  temp[1]=B01->Bx;
  temp[2]=B11->Bx;
  temp[3]=B10->Bx;
  
  temp[8]=B00->dBxdx;
  temp[9]=B01->dBxdx;
  temp[10]=B11->dBxdx;
  temp[11]=B10->dBxdx;
  
  temp[4]=B00->dBxdz;
  temp[5]=B01->dBxdz;
  temp[6]=B11->dBxdz;
  temp[7]=B10->dBxdz;
  
  temp[12]=B00->dBxdxdz;
  temp[13]=B01->dBxdxdz;
  temp[14]=B11->dBxdxdz;
  temp[15]=B10->dBxdxdz;
    
  for (i=0;i<16;i++){
    double tmp2=0.0; 
    for (k=0;k<16;k++) tmp2+=wt[i][k]*temp[k];
    cl[i]=tmp2;
  }
  for (i=0;i<4;i++)
    for (j=0;j<4;j++) coeff[i][j]=cl[m++];
  
  double t=(z - B00->z)*one_over_dz;
  double u=(r - B00->x)*one_over_dx;
  Br=dBrdr=dBrdz=0.;
  for (i=3;i>=0;i--){
    double c3u=coeff[i][3]*u;
    Br=t*Br+((c3u+coeff[i][2])*u+coeff[i][1])*u+coeff[i][0];
    dBrdr=t*dBrdr+(3.*c3u+2.*coeff[i][2])*u+coeff[i][1];
    dBrdz=u*dBrdz+(3.*coeff[3][i]*t+2.*coeff[2][i])*t+coeff[1][i];
  }
  dBrdr/=dx;
  dBrdz/=dz;
    
  // Next compute the interpolation for Bz
  temp[0]=B00->Bz;
  temp[1]=B01->Bz;
  temp[2]=B11->Bz;
  temp[3]=B10->Bz;
  
  temp[8]=B00->dBzdx;
  temp[9]=B01->dBzdx;
  temp[10]=B11->dBzdx;
  temp[11]=B10->dBzdx;
  
  temp[4]=B00->dBzdz;
  temp[5]=B01->dBzdz;
  temp[6]=B11->dBzdz;
  temp[7]=B10->dBzdz;
  
  temp[12]=B00->dBzdxdz;
  temp[13]=B01->dBzdxdz;
  temp[14]=B11->dBzdxdz;
  temp[15]=B10->dBzdxdz;
  
  for (i=0;i<16;i++){
    double tmp2=0.0;
    for (k=0;k<16;k++) tmp2+=wt[i][k]*temp[k];
    cl[i]=tmp2;
  }
  m=0;
  for (i=0;i<4;i++)
      for (j=0;j<4;j++) coeff[i][j]=cl[m++];
  
  Bz=dBzdr=dBzdz=0.;
  for (i=3;i>=0;i--){
    double c3u=coeff[i][3]*u;
    Bz=t*Bz+((c3u+coeff[i][2])*u+coeff[i][1])*u+coeff[i][0];
    dBzdr=t*dBzdr+(3.*c3u+2.*coeff[i][2])*u+coeff[i][1];
    dBzdz=u*dBzdz+(3.*coeff[3][i]*t+2.*coeff[2][i])*t+coeff[1][i];
    }
  dBzdr/=dx;
  dBzdz/=dz;
}

// Find the field and field gradient at the point (x,y,z).  
void DMagneticFieldMapFineMesh::GetFieldAndGradient(double x,double y,double z,
						    double &Bx_,double &By_,
						    double &Bz_,
						    double &dBxdx_, 
						    double &dBxdy_,
						    double &dBxdz_,
						    double &dBydx_, 
						    double &dBydy_,
						    double &dBydz_,
						    double &dBzdx_, 
						    double &dBzdy_,
						    double &dBzdz_) const{
  // radial distance
  double r = sqrt(x*x + y*y);
  if (r>xmax || z>zmax || z<zmin){
    Bx_=0.0,By_=0.0,Bz_=0.0;
    dBxdx_=0.0,dBxdy_=0.0,dBxdz_=0.0;
    dBydx_=0.0,dBydy_=0.0,dBydz_=0.0;
    dBzdx_=0.0,dBzdy_=0.0,dBzdz_=0.0;
    return;
  }

  // radial component of B and gradient
  double Br_=0.,dBrdx_=0.,dBrdz_=0.;
  // Initialize z-component
  Bz_=0.;
  
  // If the point (x,y,z) is outside the fine-mesh grid, interpolate 
  // on the coarse grid
  //if (true){
  if (z<zminFine || z>=zmaxFine || r>=rmaxFine){
    //InterpolateField(r,z,Br_,Bz_,dBrdx_,dBrdz_,dBzdx_,dBzdz_);
    // Get closest indices for this point
    int index_x = static_cast<int>(r*one_over_dx);
    int index_z = static_cast<int>((z-zmin)*one_over_dz);	
  int index_y = 0;
  
  if(index_x>=0 && index_x<Nx && index_z>=0 && index_z<Nz){
    const DBfieldPoint_t *B = &Btable[index_x][index_y][index_z];
    
    // Fractional distance between map points.
    double ur = (r - B->x)*one_over_dx;
    double uz = (z - B->z)*one_over_dz;
    
    // Use gradient to project grid point to requested position
    Br_ = B->Bx+B->dBxdx*ur+B->dBxdz*uz;
    Bz_ = B->Bz+B->dBzdx*ur+B->dBzdz*uz;
    dBrdx_=B->dBxdx;
    dBrdz_=B->dBxdz;
    dBzdx_=B->dBzdx;
    dBzdz_=B->dBzdz;
  }
  }
  else{ // otherwise do a simple lookup in the fine-mesh table
    unsigned int indr=static_cast<unsigned int>(r*rscale);
    unsigned int indz=static_cast<unsigned int>((z-zminFine)*zscale);
    const DBfieldCylindrical_t *field=&mBfine[indr][indz];

    Bz_=field->Bz;
    Br_=field->Br;
    dBrdx_=field->dBrdr;
    dBrdz_=field->dBrdz;
    dBzdz_=field->dBzdz;
    dBzdx_=field->dBzdr;
    
    //	  printf("Bz Br %f %f\n",Bz,Br);
  }


  // Convert r back to x,y components
  double cos_theta = x/r;
  double sin_theta = y/r;
  if(r==0.0){
    cos_theta=1.0;
    sin_theta=0.0;
  }
  // Rotate back into phi direction
  Bx_=Br_*cos_theta;
  By_=Br_*sin_theta;

  dBxdx_ =dBrdx_*cos_theta*cos_theta;
  dBxdy_ =dBrdx_*cos_theta*sin_theta;
  dBxdz_ =dBrdz_*cos_theta;
  dBydx_=dBxdy_;
  dBydy_ = dBrdx_*sin_theta*sin_theta;
  dBydz_ = dBrdz_*sin_theta;
  dBzdx_ = dBzdx_*cos_theta;
  dBzdy_ = dBzdx_*sin_theta;

  /*
  printf("Grad %f %f %f %f %f %f %f %f %f\n",dBxdx_,dBxdy_,dBxdz_,
	 dBydx_,dBydy_,dBydz_,dBzdx_,dBzdy_,dBzdz_);
  */
}

//-------------
// GetFieldGradient
//-------------
void DMagneticFieldMapFineMesh::GetFieldGradient(double x, double y, double z,
						double &dBxdx, double &dBxdy,
						double &dBxdz,
						double &dBydx, double &dBydy,
						double &dBydz,		
						double &dBzdx, double &dBzdy,
						double &dBzdz) const{
  
  	// Get closest indices for this point
	double r = sqrt(x*x + y*y);
	int index_x = (int)floor((r-xmin)*one_over_dx + 0.5);
	//if(index_x<0 || index_x>=Nx)return;	
	if (index_x>=Nx) return;
	else if (index_x<0) index_x=0;
	int index_z = (int)floor((z-zmin)*one_over_dz + 0.5);	
	if(index_z<0 || index_z>=Nz)return;
	
	int index_y = 0;

	const DBfieldPoint_t *B = &Btable[index_x][index_y][index_z];

	// Convert r back to x,y components
	double cos_theta = x/r;
	double sin_theta = y/r;
	if(r==0.0){
		cos_theta=1.0;
		sin_theta=0.0;
	}

	// Rotate back into phi direction
	dBxdx = B->dBxdx*cos_theta*cos_theta*one_over_dx;
	dBxdy = B->dBxdx*cos_theta*sin_theta*one_over_dx;
	dBxdz = B->dBxdz*cos_theta*one_over_dz;
	dBydx = B->dBxdx*sin_theta*cos_theta*one_over_dx;
	dBydy = B->dBxdx*sin_theta*sin_theta*one_over_dx;
	dBydz = B->dBxdz*sin_theta*one_over_dz;
	dBzdx = B->dBzdx*cos_theta*one_over_dx;
	dBzdy = B->dBzdx*sin_theta*one_over_dx;
	dBzdz = B->dBzdz*one_over_dz;
	/*
	printf("old Grad %f %f %f %f %f %f %f %f %f\n",dBxdx,dBxdy,dBxdz,
	 dBydx,dBydy,dBydz,dBzdx,dBzdy,dBzdz);
	*/
}



//---------------------------------
// GetField
//---------------------------------
void DMagneticFieldMapFineMesh::GetField(double x, double y, double z, double &Bx, double &By, double &Bz, int method) const
{
	/// This calculates the magnetic field at an arbitrary point
	/// in space using the field map read from the calibaration
	/// database. It interpolates between grid points using the
	/// gradient values calculated in ReadMap (called from the
	/// constructor).
  
  Bx = By = Bz = 0.0;
	double Br=0.0;

	if(Ny>1){
		_DBG_<<"Field map appears to be 3 dimensional. Code is currently"<<endl;
		_DBG_<<"unable to handle this. Treating as phi symmetric using y=0."<<endl;
	}

	// radial position and angles
	double r = sqrt(x*x + y*y);
	if (r>xmax || z>zmax || z<zmin){
	  return;
	}

	double cos_theta = x/r;
	double sin_theta = y/r;
	if(r==0.0){
		cos_theta=1.0;
		sin_theta=0.0;
	}

	// If the point (x,y,z) is outside the fine-mesh grid, interpolate 
	// on the coarse grid
	if (z<zminFine || z>=zmaxFine || r>=rmaxFine){
	  // Get closest indices for this point
	  int index_x = static_cast<int>(r*one_over_dx);
	  //if(index_x<0 || index_x>=Nx)return;
	  if (index_x>=Nx) return;
       
	  int index_z = static_cast<int>((z-zmin)*one_over_dz);	
	  if(index_z<0 || index_z>=Nz)return;
	  
	  int index_y = 0;
	  
	  const DBfieldPoint_t *B = &Btable[index_x][index_y][index_z];
	  
	  // Fractional distance between map points.
	  double ur = (r - B->x)*one_over_dx;
	  double uz = (z - B->z)*one_over_dz;
	  
	  // Use gradient to project grid point to requested position
	  Br = B->Bx+B->dBxdx*ur+B->dBxdz*uz;
	  Bz = B->Bz+B->dBzdx*ur+B->dBzdz*uz;
	}
        else{ // otherwise do a simple lookup in the fine-mesh table
	  unsigned int indr=static_cast<unsigned int>(r*rscale);
	  unsigned int indz=static_cast<unsigned int>((z-zminFine)*zscale);
	  const DBfieldCylindrical_t *field=&mBfine[indr][indz];

	  Bz=field->Bz;
	  Br=field->Br;
	  //	  printf("Bz Br %f %f\n",Bz,Br);
	}

	// Rotate back into phi direction
	Bx = Br*cos_theta;
	By = Br*sin_theta;
}

//---------------------------------
// GetField
//---------------------------------
void DMagneticFieldMapFineMesh::GetField(const DVector3 &pos,DVector3 &Bout) const
{
	/// This calculates the magnetic field at an arbitrary point
	/// in space using the field map read from the calibaration
	/// database. It interpolates between grid points using the
	/// gradient values calculated in ReadMap (called from the
	/// constructor).
  
  double Bz=0.,Br=0.0;

	if(Ny>1){
		_DBG_<<"Field map appears to be 3 dimensional. Code is currently"<<endl;
		_DBG_<<"unable to handle this. Treating as phi symmetric using y=0."<<endl;
	}

	// radial position and angles
	double r = pos.Perp();
	double z = pos.z();
	if (r>xmax || z>zmax || z<zmin){
	  Bout.SetXYZ(0.0, 0.0, 0.0);
	  return;
	}

	double cos_theta = pos.x()/r;
	double sin_theta = pos.y()/r;
	if(r==0.0){
		cos_theta=1.0;
		sin_theta=0.0;
	}

	// If the point (x,y,z) is outside the fine-mesh grid, interpolate 
	// on the coarse grid
	if (z<zminFine || z>=zmaxFine || r>=rmaxFine){
	  // Get closest indices for this point
	  int index_x = static_cast<int>(r*one_over_dx);
	  //if(index_x<0 || index_x>=Nx)return;
	  if (index_x>=Nx) {Bout.SetXYZ(0.0, 0.0, 0.0); return;}
       
	  int index_z = static_cast<int>((z-zmin)*one_over_dz);	
	  if(index_z<0 || index_z>=Nz){Bout.SetXYZ(0.0, 0.0, 0.0); return;}
	  
	  int index_y = 0;
	  
	  const DBfieldPoint_t *B = &Btable[index_x][index_y][index_z];
	  
	  // Fractional distance between map points.
	  double ur = (r - B->x)*one_over_dx;
	  double uz = (z - B->z)*one_over_dz;
	  
	  // Use gradient to project grid point to requested position
	  Br = B->Bx+B->dBxdx*ur+B->dBxdz*uz;
	  Bz = B->Bz+B->dBzdx*ur+B->dBzdz*uz;
	}
        else{ // otherwise do a simple lookup in the fine-mesh table
	  unsigned int indr=static_cast<unsigned int>(r*rscale);
	  unsigned int indz=static_cast<unsigned int>((z-zminFine)*zscale);
	  const DBfieldCylindrical_t *field=&mBfine[indr][indz];

	  Bz=field->Bz;
	  Br=field->Br;
	  //	  printf("Bz Br %f %f\n",Bz,Br);
	}

	// Rotate back into phi direction
	Bout.SetXYZ(Br*cos_theta,Br*sin_theta,Bz);
}




// Get the z-component of the magnetic field
double DMagneticFieldMapFineMesh::GetBz(double x, double y, double z) const{
  // radial position 
  double r = sqrt(x*x + y*y);
  if (r>xmax || z>zmax || z<zmin){
    return 0.;
  }

  // If the point (x,y,z) is outside the fine-mesh grid, interpolate 
  // on the coarse grid
  if (z<zminFine || z>=zmaxFine || r>=rmaxFine){
    // Get closest indices for this point
    int index_x = static_cast<int>(r*one_over_dx);
    //if(index_x<0 || index_x>=Nx)return 0.;
    if (index_x>=Nx) return 0.;
  
    int index_z = static_cast<int>((z-zmin)*one_over_dz);	
    if(index_z<0 || index_z>=Nz)return 0.;
    
    int index_y = 0;
    
    const DBfieldPoint_t *B = &Btable[index_x][index_y][index_z];
    
    // Fractional distance between map points.
    double ur = (r - B->x)*one_over_dx;
    double uz = (z - B->z)*one_over_dz;
	  
    // Use gradient to project grid point to requested position
    return (B->Bz + B->dBzdx*ur + B->dBzdz*uz);
  }
 
  // otherwise do a simple lookup in the fine-mesh table
  unsigned int indr=static_cast<unsigned int>(r*rscale);
  unsigned int indz=static_cast<unsigned int>((z-zminFine)*zscale);
  
  return mBfine[indr][indz].Bz;
}

// Read a fine-mesh B-field map from an evio file
void DMagneticFieldMapFineMesh::GetFineMeshMap(string namepath,int32_t runnumber){ 
    // The solenoid field map files are stored in CCDB as /Magnets/Solenoid/BFIELD_MAP_NAME
    // The fine-mesh files are now stored as /Magnets/Solenoid/finemeshes/BFIELD_MAP_NAME
    size_t ipos = namepath.rfind("/");
    if(ipos == string::npos)
        throw JException("Could not parse field map: "+namepath);
    string finemesh_namepath = namepath.substr(0,ipos) + "/finemeshes" + namepath.substr(ipos);
    string evioFileName = "";
    string evioFileNameToWrite = "";
    // see if we can get the EVIO file as a resource
    try {
        evioFileName = jresman->GetResource(finemesh_namepath);
    } catch ( JException e ) {
        // if we can't get it as a resource, try to get it from a local file
        size_t ipos=namepath.find("/");
        size_t ipos2=namepath.find("/",ipos+1);
        evioFileName = namepath.substr(ipos2+1)+".evio";
        // see if the file exists
        struct stat stFileInfo;
        int intStat = stat(evioFileName.c_str(),&stFileInfo);
        if (intStat != 0) {
            evioFileNameToWrite = evioFileName;
            evioFileName = "";
        }
    }

    if(evioFileName != "") {
        ReadEvioFile(evioFileName);
    } else{
    cout << "Fine-mesh evio file does not exist." <<endl;
    cout << "Constructing the fine-mesh B-field map..." << endl;    
    GenerateFineMesh();
#ifdef HAVE_EVIO
    WriteEvioFile(evioFileNameToWrite);
#endif
    }

  cout << " rmin: " << rminFine << " rmax: " << rmaxFine 
       << " dr: " << drFine << " zmin: " << zminFine << " zmax: "
       << zmaxFine << " dz: " << dzFine <<endl;  
  cout << " Number of points in z = " <<NzFine <<endl;
  cout << " Number of points in r = " << NrFine << endl;
}

void DMagneticFieldMapFineMesh::GenerateFineMesh(void){
  rminFine=0.;
  rmaxFine=88.5;
  drFine=0.1;
  zminFine=0.;
  zmaxFine=600.;
  dzFine=0.1;
  zscale=1./dzFine;
  rscale=1./drFine;
  NrFine=(unsigned int)floor((rmaxFine-rminFine)/drFine+0.5);
  NzFine=(unsigned int)floor((zmaxFine-zminFine)/dzFine+0.5);

  vector<DBfieldCylindrical_t>zrow;
  for (unsigned int i=0;i<NrFine;i++){
    double x=rminFine+drFine*double(i);
    for (unsigned int j=0;j<NzFine;j++){
      double z=zminFine+dzFine*double(j);
      DBfieldCylindrical_t temp;
      InterpolateField(x,z,temp.Br,temp.Bz,temp.dBrdr,temp.dBrdz,temp.dBzdr,
		       temp.dBzdz);
      zrow.push_back(temp);
    }
    mBfine.push_back(zrow);
    zrow.clear();
  }
}

void DMagneticFieldMapFineMesh::WriteEvioFile(string evioFileName){
  cout << "Writing fine-mesh B-field data to " << evioFileName << "..." <<endl;

  // Create vectors of the B-field components and gradients
  vector<float>Br_;
  vector<float>Bz_;
  vector<float>dBrdr_;
  vector<float>dBrdz_;  
  vector<float>dBzdr_;
  vector<float>dBzdz_;
  for (unsigned int i=0;i<NrFine;i++){
    for (unsigned int j=0;j<NzFine;j++){
      Br_.push_back(mBfine[i][j].Br);  
      Bz_.push_back(mBfine[i][j].Bz); 
      dBrdr_.push_back(mBfine[i][j].dBrdr);   
      dBrdz_.push_back(mBfine[i][j].dBrdz);  
      dBzdr_.push_back(mBfine[i][j].dBzdr);
      dBzdz_.push_back(mBfine[i][j].dBzdz);
    }
  }

  // Calculate total buffer size needed (in 32bit words)
  // and allocate buffer
  uint32_t buff_size = 8; // EVIO block header
  buff_size += 2;         // outer bank length and header
  buff_size += 2+6;       // minmaxdelta length, header words plus 6 payload words
  buff_size += 6*(2+NrFine*NzFine); // 6 banks, each with length,header words and NrFine*NzFine payload
  uint32_t *buff = new uint32_t[buff_size+8]; // +8 for EVIO block trailer
  
  // EVIO block header
  uint32_t *iptr = buff;
  *iptr++ = buff_size;  // Number of 32 bit words in evio block
  *iptr++ = 1;          // Block number
  *iptr++ = 8;          // Length of block header (words)
  *iptr++ = 1;          // Event Count
  *iptr++ = 0;          // Reserved 1
  *iptr++ = 0x4;        // 0x4=EVIO version 4
  *iptr++ = 0;          // Reserved 2
  *iptr++ = 0xc0da0100; // Magic number
  
  // Outermost bank
  *iptr++ = buff_size - 1 - 8; // -1 for length word. -8 for evio block header
  *iptr++ = (0x1<<16) + (0x0E<<8) + (0);
  
  // Table dimensions bank
  *iptr++ = 2+6 - 1;
  *iptr++ = (0x2<<16) + (0x02<<8) + (0);
  *(float*)iptr++ = (float)rminFine;
  *(float*)iptr++ = (float)rmaxFine;
  *(float*)iptr++ = (float)drFine;
  *(float*)iptr++ = (float)zminFine;
  *(float*)iptr++ = (float)zmaxFine;
  *(float*)iptr++ = (float)dzFine;
  
  // Table values banks
  for(uint32_t i=0; i<6; i++){
    vector<float> *d = NULL;
	 switch(i){
	   case 0: d = &Br_;     break;
	   case 1: d = &Bz_;     break;
	   case 2: d = &dBrdr_;  break;
	   case 3: d = &dBrdz_;  break;
	   case 4: d = &dBzdr_;  break;
	   case 5: d = &dBzdz_;  break;
	 }
    *iptr++ = 2+NrFine*NzFine - 1;
    *iptr++ = (0x3<<16) + (0x02<<8) + (i);
	 for(float f : *d) *(float*)iptr++ = f;
	 //for(uint32_t j=0; j<NrFine*NzFine; j++) *(float*)iptr++ = d->at(j);
  }
  
  // EVIO block trailer
  *iptr++ = 8;             // Number of 32 bit words in evio block
  *iptr++ = 2;             // Block number
  *iptr++ = 8;             // Length of block header (words)
  *iptr++ = 0;             // Event Count
  *iptr++ = 0;             // Reserved 1
  *iptr++ = (1<<9) + 0x4;  // (1<<9) last event in stack 0x4=EVIO version 4
  *iptr++ = 0;             // Reserved 2
  *iptr++ = 0xc0da0100;    // Magic number

  
  // Write the actual file
  ofstream ofs(evioFileName);
  ofs.write((char*)buff, (buff_size+8)*4);
  ofs.close();
  
  delete[] buff;

#ifdef HAVE_EVIO
  // Open the evio file channel
  unsigned long bufsize=NrFine*NzFine*6*sizeof(float)+6;
  evioFileChannel chan(evioFileName,"w",bufsize);
  chan.open();
  
  // create an event tree, root node has (tag=1,num=0)
  evioDOMTree tree(1,0);

  float minmaxdelta[6]={(float)rminFine,(float)rmaxFine,(float)drFine,(float)zminFine,(float)zmaxFine,(float)dzFine};
  tree.addBank(2,0,minmaxdelta,6);

  // Add the banks
  tree.addBank(3,0,Br_);
  tree.addBank(3,1,Bz_);
  tree.addBank(3,2,dBrdr_);
  tree.addBank(3,3,dBrdz_);
  tree.addBank(3,4,dBzdr_);
  tree.addBank(3,5,dBzdz_);

  chan.write(tree);
  chan.close();
#endif  // HAVE_EVIO
}

// Read the B-field data from the evio file
void DMagneticFieldMapFineMesh::ReadEvioFile(string evioFileName){
	cout << "Reading fine-mesh B-field data from "<< evioFileName << endl;

	// Open EVIO file
	HDEVIO hdevio(evioFileName, false);
	if(!hdevio.is_open){
		jerr << " Unable to open fine-mesh B-field file!" << jendl;
		return;
	}

	// Allocate a small buffer and attempt to read in event.
	// This will fail due to the small buffer, but will leave
	// the size needed in hdevio.last_event_len so we can reallocate.
	uint32_t buff_size=10; // initially allocate small buffer
	uint32_t *buff = new uint32_t[buff_size];
	hdevio.readNoFileBuff(buff, buff_size);
	if(hdevio.err_code == HDEVIO::HDEVIO_USER_BUFFER_TOO_SMALL){
		delete[] buff;
		buff_size = hdevio.last_event_len;
		buff = new uint32_t[buff_size];
		hdevio.readNoFileBuff(buff, buff_size);
	}
	if(hdevio.err_code != HDEVIO::HDEVIO_OK){
		jerr << " Problem reading fine-mesh B-field" << jendl;
		jerr << hdevio.err_mess.str() << jendl;
		delete[] buff;
		return;
	}
	
	// Top-level bank of banks has tag=1, num=0
	uint32_t *iptr = buff;
	uint32_t *iend = &iptr[*iptr +1];
	iptr = &iptr[2];

	// First bank has tag=2, num=0 and length 6 data words
	if(iptr[0] != 6+1){
		jerr << " Bad length for minmaxdelta bank!" <<jendl;
		_exit(-1);
	}
	float *minmaxdelta = (float*)&iptr[2];
	rminFine = minmaxdelta[0];       
	rmaxFine = minmaxdelta[1]; 
	drFine   = minmaxdelta[2];	
	zminFine = minmaxdelta[3];
	zmaxFine = minmaxdelta[4];	
	dzFine   = minmaxdelta[5];
	iptr = &iptr[*iptr + 1];

	zscale=1./dzFine;
	rscale=1./drFine;

	NrFine=(unsigned int)floor((rmaxFine-rminFine)/drFine+0.5);
	NzFine=(unsigned int)floor((zmaxFine-zminFine)/dzFine+0.5);
	mBfine.resize(NrFine);
	for(auto &m : mBfine) m.resize(NzFine);

	// Next 6 banks have tag=3 and num=0-5 and hold
	// the actual table data
	for(int num=0; num<=5; num++){
		uint32_t mynum = iptr[1] & 0xFF;
		float *fptr = (float*)&iptr[2];
		uint32_t N = iptr[0] - 1;
		iptr = &iptr[N+2];
		if(iptr > iend){
			jerr << " Bad format of fine mesh B-field file!" << jendl;
			_exit(-1);
		}
		
		for(uint32_t k=0; k<N; k++){
			uint32_t indr=k/NzFine;
			uint32_t indz=k%NzFine;
			switch( mynum ){
				case 0: mBfine[indr][indz].Br    = *fptr++;  break;  // Br
				case 1: mBfine[indr][indz].Bz    = *fptr++;  break;  // Bz
				case 2: mBfine[indr][indz].dBrdr = *fptr++;  break;  // dBrdr
				case 3: mBfine[indr][indz].dBrdz = *fptr++;  break;  // dBrdz
				case 4: mBfine[indr][indz].dBzdr = *fptr++;  break;  // dBzdr
				case 5: mBfine[indr][indz].dBzdz = *fptr++;  break;  // dBzdz
			}
		}
	}
	
	delete[] buff;
}
