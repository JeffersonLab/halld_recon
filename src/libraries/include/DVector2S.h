// $Id$
//
//    File: DVector2S.h
// Created: Fri Dec 28 07:19:32 EST 2007
// Creator: davidl (on Darwin Amelia.local 8.10.1 i386)
//

#ifndef _DVector2S_
#define _DVector2S_

#include <math.h>
#include <iostream>
using namespace std;

class DVector2S{
 public:
  DVector2S(){mx=0.; my=0.;}
  DVector2S(double xx, double yy){mx=xx; my=yy;}
  ~DVector2S(){};
  
  double X(void) const {return mx;}
  double Y(void) const {return my;}
  void Set(double xx, double yy){mx=xx; my=yy;}
  double Mod(void) const {return sqrt(mx*mx + my*my);} 
  double Mod2(void) const {return mx*mx + my*my;}
  double Phi(void) const {return atan2(my,mx);}
  // return phi angle between 0 and 2pi
  double Phi_0_2pi(double angle){
    double twopi=2.*M_PI;
    while (angle>=twopi) angle-=twopi;
    while (angle<0) angle+=twopi;
    return angle;
  }
  

  // Dot product
  double operator*(const DVector2S &v1) const{
    return (mx*v1.X()+my*v1.Y());
  }
  // Copy and assignment 
  DVector2S(const DVector2S &v){mx=v.X();my=v.Y();}
  DVector2S& operator=(const DVector2S &v){mx=v.X();my=v.Y();return *this;}

  DVector2S operator+(const DVector2S &v) const {
    return DVector2S(mx+v.X(),my+v.Y());
  }
  DVector2S operator-(const DVector2S &v) const {
    return DVector2S(mx-v.X(),my-v.Y());
  }
  
  DVector2S& operator*=(const double &f){mx*=f; my*=f; return *this;}
  DVector2S& operator/=(const double &f){mx/=f; my/=f; return *this;}
  DVector2S& operator+=(const DVector2S &v){mx+=v.X(); my+=v.Y(); return *this;}
  DVector2S& operator-=(const DVector2S &v){mx-=v.X(); my-=v.Y(); return *this;}
  
  void Print(){
    cout << "DVector2 (x,y)=("<<X()<<","<<Y()<<") (rho,phi)=("<< Mod()
	 <<","<< 180./M_PI*Phi()<<")"<<endl;
  }


 private:
  double mx;
  double my;
};

inline DVector2S operator*(const double &f, const DVector2S &vec) {
	DVector2S s(f*vec.X(), f*vec.Y());
	return s;
}

inline DVector2S operator*(const DVector2S &vec, const double &f) {
	DVector2S s(f*vec.X(), f*vec.Y());
	return s;
}

inline DVector2S operator/(const DVector2S &vec, const double &f) {
	DVector2S s(vec.X()/f, vec.Y()/f);
	return s;
}

#endif // _DVector2S_

