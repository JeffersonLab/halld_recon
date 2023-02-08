// $Id$
//
//    File: DVector3S.h
// Created: Thu Feb  2 14:09:08 EST 2023
// Creator: staylor (on Linux ifarm1801.jlab.org 3.10.0-1160.81.1.el7.x86_64 x86_64)
//

#ifndef _DVector3S_
#define _DVector3S_

#include <math.h>
#include <iostream>
#include <iomanip>
using namespace std;

#define RAD2DEG 180./M_PI

class DVector3S{
 public:
  DVector3S(){mx=0.;my=0.;mz=0.;} 
  DVector3S(double xx,double yy, double zz){mx=xx;my=yy;mz=zz;}
  ~DVector3S(){};
  
  // Get component by index
  double operator () (int ind) const {
    switch (ind){
    case 0:
      return x();
      break;
    case 1:
      return y();
      break;
    case 2:
      return z();
    default:
      // cerr << "Invalid index." <<endl;
      break;
    }
    return 0.;
  }
  double operator [] (int ind) const {
    switch (ind){
    case 0:
      return x();
      break;
    case 1:
      return y();
      break;
    case 2:
      return z();
    default:
      // cerr << "Invalid index." <<endl;
      break;
    }
    return 0.;
  }
 
  // Check for equality
  bool operator==(const DVector3S &v1) const {
    return ((x()==v1.x() && y()==v1.y() && z()==v1.z())? true : false);
  }
  // Check for inequality 
  bool operator!=(const DVector3S &v1) const{
    return ((x()!=v1.x() || y()!=v1.y() || z()!=v1.z())? true : false);
  }

  double x(void) const {return mx;}
  double y(void) const {return my;} 
  double z(void) const {return mz;}
  double X(void) const {return mx;}
  double Y(void) const {return my;} 
  double Z(void) const {return mz;}
  double Px(void) const {return mx;}
  double Py(void) const {return my;} 
  double Pz(void) const {return mz;}

  void SetX(double xx){mx=xx;}
  void SetY(double yy){my=yy;}
  void SetZ(double zz){mz=zz;}
  void SetXYZ(double xx,double yy, double zz){mx=xx;my=yy;mz=zz;} 
  void SetMagThetaPhi(double p, double theta, double phi){
    double my_p=fabs(p);
    double pt=my_p*sin(theta); 
    mx=pt*cos(phi);
    my=pt*sin(phi);
    mz=my_p*cos(theta);
  }
  DVector3S& SetMag(double a){ double s=a/Mag(); mx*=s; my*=s; mz*=s; return *this;}
  DVector3S Unit() const {
    double s=1./Mag();
    return DVector3S(mx*s,my*s,mz*s);
  }

  double Perp() const {return sqrt(mx*mx+my*my);}
  double Pt() const{ return Perp();}
  double Perp2() const {return mx*mx+my*my;}
  double Mag() const {return sqrt(mx*mx+my*my+mz*mz);}
  double Mag2() const {return mx*mx+my*my+mz*mz;}
  double CosTheta() const { double r=Mag();return r == 0.0 ? 1.0 : mz/r;}
  double Theta() const {return acos(CosTheta());}
  double Phi() const {return atan2(my,mx);}
 
  // Angle between two vectors
  double Angle(const DVector3S &v) const{
    double v1mag=Mag();
    double v2mag=v.Mag();
    if (v2mag>0. && v1mag > 0.){
      double costheta=Dot(v)/v1mag/v2mag;
      // Due to round-off errors, costheta could be epsilon greater than 1...
      if (costheta>1.) return 0.;
      if (costheta<-1.) return M_PI; 
      return acos(costheta);
    }
    return 0.;
  }

 
  // Copy and assignment 
  DVector3S(const DVector3S &v){mx=v.x(); my=v.y(); mz=v.z();}
  DVector3S& operator=(const DVector3S &v){
    mx=v.x(); my=v.y(); mz=v.z(); return *this;
  }

  DVector3S operator+(const DVector3S &v) const {
    return DVector3S(mx+v.x(),my+v.y(),mz+v.z());
  }
  DVector3S operator-(const DVector3S &v) const {
    return DVector3S(mx-v.x(),my-v.y(),mz-v.z());
  }
  // Unary minus
  DVector3S operator-() const {return DVector3S(-mx,-my,-mz);}

  DVector3S& operator*=(const double &f){mx*=f; my*=f; mz*=f; return *this;}
  DVector3S& operator/=(const double &f){mx/=f; my/=f; mz/=f; return *this;}
  DVector3S& operator+=(const DVector3S &v){mx+=v.x(); my+=v.y(); mz+=v.z(); return *this;}
  DVector3S& operator-=(const DVector3S &v){mx-=v.x(); my-=v.y(); mz-=v.z(); return *this;}
  
  // dot product of "this" and vector v
  double Dot(const DVector3S &v) const {return (mx*v.x()+my*v.y()+mz*v.z());}
  double operator*(const DVector3S &v1) const{
    return (mx*v1.X()+my*v1.Y()+mz*v1.Z());
  }  

  // Create a vector orthogonal to "this" 
  DVector3S Orthogonal() const{  
    double xx= mx<0.0 ? -mx : mx;
    double yy= my<0.0 ? -my : my;
    double zz= mz<0.0 ? -mz : mz;
    if (xx < yy) {
      return xx < zz ? DVector3S(0.,mz,-my) : DVector3S(my,-mx,0.);
    } else {
      return yy < zz ? DVector3S(-mz,0.,mx) : DVector3S(my,-mx,0.);
    }
  };
  // Cross product of "this"=(x,y,z) and v
  //  |x'| |y*vz-z*vy|
  //  |y'|=|z*vx-x*vz|
  //  |z'| |x*vy-y*vx|
  DVector3S Cross(const DVector3S &v) const {
    return DVector3S( my*v.z()-mz*v.y(), mz*v.x()-mx*v.z(), mx*v.y()-my*v.x());
  }

  // Rotate by angle a about the axis specified by v. 
  //            sa = sin(a), ca= cos(a)
  //            dx=vx/|v|, dy=vy/|v|, dz=vz/|v|
  // |x'| |ca+(1-ca)*dx*dx          (1-ca)*dx*dy-sa*dz    (1-ca)*dx*dz+sa*dy||x|
  // |y'|=|   (1-ca)*dy*dx+sa*dz ca+(1-ca)*dy*dy          (1-ca)*dy*dz-sa*dx||y|
  // |z'| |   (1-ca)*dz*dx-sa*dy    (1-ca)*dz*dy+sa*dx ca+(1-ca)*dz*dz      ||z|
  //
  void Rotate(const double a,const DVector3S &v){
    double vmag=v.Mag();
    double dx=v.x()/vmag;
    double dy=v.y()/vmag;
    double dz=v.z()/vmag;
    double sa=sin(a);
    double ca=cos(a);
    double one_minus_ca=1.-ca;
    double xx=(ca+one_minus_ca*dx*dx)*mx + (one_minus_ca*dx*dy-sa*dz)*my
      + (one_minus_ca*dx*dz+sa*dy)*mz;
    double yy=(one_minus_ca*dy*dx+sa*dz)*mx + (ca+one_minus_ca*dy*dy)*my
      + (one_minus_ca*dy*dz-sa*dx)*mz;
    double zz=(one_minus_ca*dz*dx-sa*dy)*mx + (one_minus_ca*dz*dy+sa*dx)*my
      + (ca+one_minus_ca*dz*dz)*mz;
    mx=xx;
    my=yy;
    mz=zz;
  }
  // Rotate by angle a about the x-axis
  void RotateX(const double a){
    double ca=cos(a);
    double sa=sin(a);
    double yy=my;
    my=ca*yy-sa*mz;
    mz=sa*yy+ca*mz;
  }
  // Rotate by angle a about the y-axis
  void RotateY(const double a){
    double ca=cos(a);
    double sa=sin(a);
    double zz=mz;
    mz=ca*zz-sa*mx;
    mx=sa*zz+ca*mx;
  }
  // Rotate by angle a about the z-axis
  void RotateZ(const double a){
    double ca=cos(a);
    double sa=sin(a);
    double xx=mx;
    mx=ca*xx-sa*my;
    my=sa*xx+ca*my;
  }
  
  // Print the components to the screen 
  void Print() const{
    cout << "DVector3 (x,y,z)=("<<setprecision(5)<<x() << ","
         << setprecision(5) << y() << ","<<setprecision(5) << z() << "), " 
         <<"(rho,theta,phi)=("<<setprecision(5) << Mag() << "," 
         << setprecision(5) <<RAD2DEG*Theta() << "," 
         << setprecision(5) << RAD2DEG*Phi() << ")" 
	 <<endl;   
  };
  
  
   
 private:
  double mx,my,mz;
};

inline DVector3S operator*(const double &f, const DVector3S &vec) {
  return DVector3S(f*vec.x(), f*vec.y(), f*vec.z());
}
inline DVector3S operator*(const DVector3S &vec,const double &f) {
  return DVector3S(f*vec.x(), f*vec.y(), f*vec.z());
}

inline DVector3S operator/(const DVector3S &vec,const double &f) {
  return DVector3S(vec.x()/f, vec.y()/f, vec.z()/f);
}

#endif // _DVector3S_

