// $Id$
//
//    File: DMagneticFieldMapFineMesh_StandAlone.cc

#include "DMagneticFieldMapFineMesh_StandAlone.h"

//---------------------------------
// DMagneticFieldMapFineMesh_StandAlone    (Constructor)
//---------------------------------
DMagneticFieldMapFineMesh_StandAlone::DMagneticFieldMapFineMesh_StandAlone(const string& coarseMeshMsgpackFileName,	const string& fineMeshMsgpackFileName)
{
	ReadMsgpackFileCoarseMesh(coarseMeshMsgpackFileName);
	ReadMsgpackFileFineMesh(fineMeshMsgpackFileName);
}

//---------------------------------
// ~DMagneticFieldMapFineMesh_StandAlone    (Destructor)
//---------------------------------
DMagneticFieldMapFineMesh_StandAlone::~DMagneticFieldMapFineMesh_StandAlone()
{

}

//---------------------------------
// GetField
//---------------------------------
void DMagneticFieldMapFineMesh_StandAlone::GetField(double x, double y, double z, double &Bx, double &By, double &Bz, int method) const
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

void DMagneticFieldMapFineMesh_StandAlone::ReadMsgpackFileCoarseMesh(const string& msgpackFileName)
{
	cout << "Reading coarse-mesh B-field data from " << msgpackFileName << endl;
	std::ifstream inFile(msgpackFileName, std::ios::binary);
	if (not inFile) {
		cerr << "Could not open file " << msgpackFileName << " for reading." << endl;
		return;
	}
	std::vector<uint8_t> buffer(std::istreambuf_iterator<char>(inFile), {});
	nlohmann::json json = nlohmann::json::from_msgpack(buffer);

	// deserialize data from JSON
	Btable = json["Btable"].get<vector<vector<vector<DMagneticFieldMapFineMesh_StandAlone::DBfieldPoint_t>>>>();
	Nx     = json["Nx"    ].get<int>();
	xmin   = json["xmin"  ].get<float>();
	xmax   = json["xmax"  ].get<float>();
	Ny     = json["Ny"    ].get<int>();
	ymin   = json["ymin"  ].get<float>();
	ymax   = json["ymax"  ].get<float>();
	Nz     = json["Nz"    ].get<int>();
	zmin   = json["zmin"  ].get<float>();
	zmax   = json["zmax"  ].get<float>();

	// calculate parameters
	dx = (xmax - xmin) / (double)(Nx - 1);
	dy = (ymax - ymin) / (double)((Ny < 2) ? 1 : Ny - 1);
	dz = (zmax - zmin) / (double)(Nz - 1);
	one_over_dx = 1.0 / dx;
	one_over_dz = 1.0 / dz;
}

void DMagneticFieldMapFineMesh_StandAlone::ReadMsgpackFileFineMesh(const string& msgpackFileName)
{
	cout << "Reading coarse-mesh B-field data from " << msgpackFileName << endl;
	std::ifstream inFile(msgpackFileName, std::ios::binary);
	if (not inFile) {
		cerr << "Could not open file " << msgpackFileName << " for reading." << endl;
		return;
	}
	std::vector<uint8_t> buffer(std::istreambuf_iterator<char>(inFile), {});
	nlohmann::json json = nlohmann::json::from_msgpack(buffer);

	// deserialize data from JSON
	mBfine   = json["mBfine"  ].get<vector<vector<DMagneticFieldMapFineMesh_StandAlone::DBfieldCylindrical_t>>>();
	rminFine = json["rminFine"].get<double>();
	rmaxFine = json["rmaxFine"].get<double>();
	drFine   = json["drFine"  ].get<double>();
	zminFine = json["zminFine"].get<double>();
	zmaxFine = json["zmaxFine"].get<double>();
	dzFine   = json["dzFine"  ].get<double>();

	// calculate parameters
	zscale = 1.0 / dzFine;
	rscale = 1.0 / drFine;
	NrFine = static_cast<unsigned int>((rmaxFine - rminFine) / drFine + 0.5);
	NzFine = static_cast<unsigned int>((zmaxFine - zminFine) / dzFine + 0.5);
}
