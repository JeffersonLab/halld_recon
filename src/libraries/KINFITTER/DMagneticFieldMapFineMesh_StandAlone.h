// $Id$
//
//    File: DMagneticFieldMapFineMesh_StandAlone.h

#ifndef _DMagneticFieldMapFineMesh_StandAlone_
#define _DMagneticFieldMapFineMesh_StandAlone_

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
using namespace std;

// originally defined in `src/libraries/DAQ/HDEVIO.h`
#ifndef _DBG_
#define _DBG_ cout<<__FILE__<<":"<<__LINE__<<" "
#endif
#ifndef _DBG__
#define _DBG__ cout<<__FILE__<<":"<<__LINE__<<endl
#endif

#include <nlohmann/json.hpp>

// minimal standalone version of `src/libraries/HDGEOMETRY/DMagneticFieldMapFineMesh.h`
// alternatively, instead of replicating the code here, the original
// code could be modified by inserting `#ifndef _KINFITTER_STANDALONE_`
// to disable unneeded code
class DMagneticFieldMapFineMesh_StandAlone {
 public:
  DMagneticFieldMapFineMesh_StandAlone(const string& coarseMeshMsgpackFileName, const string& fineMeshMsgpackFileName);
  virtual ~DMagneticFieldMapFineMesh_StandAlone();
  void GetField(double x, double y, double z, double &Bx, double &By, double &Bz, int method=0) const;
  void ReadMsgpackFileCoarseMesh(const string& msgpackFileName);
  void ReadMsgpackFileFineMesh(const string& msgpackFileName);
  
  typedef struct{
    float x,y,z,Bx,By,Bz;
    double dBxdx, dBxdy, dBxdz;
    double dBydx, dBydy, dBydz;
    double dBzdx, dBzdy, dBzdz;
    double dBxdxdy,dBxdxdz,dBxdydz;
    double dBydxdy,dBydxdz,dBydydz;
    double dBzdxdy,dBzdxdz,dBzdydz;
    double Bmag;
  }DBfieldPoint_t;
  
  typedef struct{
    double Br,Bz,Bmag;
    double dBrdr,dBrdz,dBzdr,dBzdz;
  }DBfieldCylindrical_t;
  
 protected:
  
  vector< vector< vector<DBfieldPoint_t> > > Btable;
  
  float xmin, xmax, ymin, ymax, zmin, zmax;
  int Nx, Ny, Nz;
  double dx, dy,dz;
  double one_over_dx,one_over_dz;
  
  vector<vector<DBfieldCylindrical_t> >mBfine;
  double zminFine,rminFine,zmaxFine,rmaxFine,drFine,dzFine;
  unsigned int NrFine,NzFine;  
  double zscale,rscale;
};

// define JSON serialization for the B-field structs
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
	DMagneticFieldMapFineMesh_StandAlone::DBfieldCylindrical_t,
	Br, Bz, Bmag,
	dBrdr, dBrdz, dBzdr, dBzdz
)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
	DMagneticFieldMapFineMesh_StandAlone::DBfieldPoint_t,
	x, y, z, Bx, By, Bz,
	dBxdx, dBxdy, dBxdz,
	dBydx, dBydy, dBydz,
	dBzdx, dBzdy, dBzdz,
	dBxdxdy, dBxdxdz, dBxdydz,
	dBydxdy, dBydxdz, dBydydz,
	dBzdxdy, dBzdxdz, dBzdydz,
	Bmag
)

#endif // _DMagneticFieldMapFineMesh_StandAlone_
