// $Id$
//
//    File: DMagneticFieldMapSpoiled.cc
// Created: Wed Mar 25 04:04:16 EDT 2009
// Creator: davidl (on Darwin Amelia.local 9.6.0 i386)
//

#include "DMagneticFieldMapSpoiled.h"
#include "DMagneticFieldMapCalibDB.h"

#include <iostream>
#include <cmath>
using namespace std;

#include <JANA/JApplication.h>

#include <DVector3.h>

//---------------------------------
// DMagneticFieldMapSpoiled    (Constructor)
//---------------------------------
DMagneticFieldMapSpoiled::DMagneticFieldMapSpoiled(JApplication *japp, unsigned int run_number, string namepath)
{
	bfield = new DMagneticFieldMapCalibDB(japp, run_number, namepath);
	app = japp;
	initialized = false;
}

//---------------------------------
// DMagneticFieldMapSpoiled    (Constructor)
//---------------------------------
DMagneticFieldMapSpoiled::DMagneticFieldMapSpoiled(JCalibration *jcalib, string namepath)
{
	bfield = new DMagneticFieldMapCalibDB(jcalib, namepath);
	
	initialized = false;
}

//---------------------------------
// ~DMagneticFieldMapSpoiled    (Destructor)
//---------------------------------
DMagneticFieldMapSpoiled::~DMagneticFieldMapSpoiled()
{
	if(bfield)delete bfield;
}

//---------------------------------
// Init
//---------------------------------
void DMagneticFieldMapSpoiled::Init(void)
{
	phi_amp = 0.0;
	phi_omega = M_PI/(2.0*M_PI);
	r_amp = 0.0;
	r_omega = M_PI/65.0;
	z_amp = 0.0;
	z_omega = M_PI/(20.0);

	app->SetDefaultParameter("BFIELD:PHI_AMP",   phi_amp);
	app->SetDefaultParameter("BFIELD:PHI_OMEGA", phi_omega);
	app->SetDefaultParameter("BFIELD:R_AMP",     r_amp);
	app->SetDefaultParameter("BFIELD:R_OMEGA",   r_omega);
	app->SetDefaultParameter("BFIELD:Z_AMP",     z_amp);
	app->SetDefaultParameter("BFIELD:Z_OMEGA",   z_omega);
	
	initialized=true;
}

void DMagneticFieldMapSpoiled::GetField(const DVector3 &pos,DVector3 &Bout) const{
  double Bx,By,Bz;
  GetField(pos.x(),pos.y(),pos.z(),Bx,By,Bz);
  Bout.SetXYZ(Bx,By,Bz);
}

//---------------------------------
// GetField
//---------------------------------
void DMagneticFieldMapSpoiled::GetField(double x, double y, double z, double &Bx, double &By, double &Bz, int method) const
{
	if(!initialized){
		DMagneticFieldMapSpoiled *mythis = const_cast<DMagneticFieldMapSpoiled*>(this);
		mythis->Init();
	}
	if(!initialized)return;
	bfield->GetField(x, y, z, Bx, By, Bz, method);
	
	DVector3 B(Bx, By, Bz);
	double r = sqrt(x*x + y*y);
	double phi = atan2(y,x);
	if(phi<0.0)phi+=2.0*M_PI;
	
	// Apply phi dependance
	double fac = phi_amp*sin(phi_omega*phi) + 1.0;
	
	// Apply r dependance
	fac *= r_amp*sin(r_omega*r) + 1.0;
	
	// Apply z dependance
	fac *= z_amp*sin(z_omega*z) + 1.0;
	
	B *= fac;
	Bx = B.X();
	By = B.Y();
	Bz = B.Z();

}


//---------------------------------
// get z-component of magnetic field
//---------------------------------
double DMagneticFieldMapSpoiled::GetBz(double x, double y, double z) const
{
	if(!initialized){
		DMagneticFieldMapSpoiled *mythis = const_cast<DMagneticFieldMapSpoiled*>(this);
		mythis->Init();
	}
	if(!initialized)return 0.;
	double Bx,By,Bz;
	bfield->GetField(x, y, z, Bx, By, Bz);
	
	DVector3 B(Bx, By, Bz);
	double r = sqrt(x*x + y*y);
	double phi = atan2(y,x);
	if(phi<0.0)phi+=2.0*M_PI;
	
	// Apply phi dependance
	double fac = phi_amp*sin(phi_omega*phi) + 1.0;
	
	// Apply r dependance
	fac *= r_amp*sin(r_omega*r) + 1.0;
	
	// Apply z dependance
	fac *= z_amp*sin(z_omega*z) + 1.0;
	
	B *= fac;
	return B.Z();

}


//---------------------------------
// GetFieldGradient
//---------------------------------
void DMagneticFieldMapSpoiled::GetFieldGradient(double x, double y, double z,
				      double &dBxdx, double &dBxdy,
				      double &dBxdz,
				      double &dBydx, double &dBydy,
				      double &dBydz,		
				      double &dBzdx, double &dBzdy,
				      double &dBzdz) const
{
	if(!initialized){
		DMagneticFieldMapSpoiled *mythis = const_cast<DMagneticFieldMapSpoiled*>(this);
		mythis->Init();
	}
	if(!initialized)return;
	bfield->GetFieldGradient(x, y, z, dBxdx, dBxdy, dBxdz, dBydx, dBydy, dBydz, dBzdx, dBzdy, dBzdz);
}


void DMagneticFieldMapSpoiled::GetFieldBicubic(double x,double y,double z,
					      double &Bx,double &By,double &Bz) const{
  	if(!initialized){
		DMagneticFieldMapSpoiled *mythis = const_cast<DMagneticFieldMapSpoiled*>(this);
		mythis->Init();
	}
	if(!initialized)return;
	bfield->GetFieldBicubic(x,y,z,Bx,By,Bz);
}


void DMagneticFieldMapSpoiled::GetFieldAndGradient(double x,double y,double z,
						  double &Bx,double &By,
						  double &Bz,
						  double &dBxdx, double &dBxdy,
						  double &dBxdz,
						  double &dBydx, double &dBydy,
						  double &dBydz,
						  double &dBzdx, double &dBzdy,
						  double &dBzdz) const{
  	if(!initialized){
		DMagneticFieldMapSpoiled *mythis = const_cast<DMagneticFieldMapSpoiled*>(this);
		mythis->Init();
	}
	if(!initialized)return;
	bfield->GetFieldAndGradient(x,y,z,Bx,By,Bz,dBxdx,dBxdy,dBxdz,
				    dBydx,dBydy,dBydz,dBzdx,dBzdy,dBzdz);  
}
