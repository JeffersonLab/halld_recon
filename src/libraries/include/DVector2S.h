// $Id$
//
//    File: DVector2S.h
// Created: Fri Dec 28 07:19:32 EST 2007
// Creator: davidl (on Darwin Amelia.local 8.10.1 i386)
//

#ifndef _DVector2S_
#define _DVector2S_
#include <math.h>

class DVector2S{
public:
  DVector2S(){x=0;y=0;};
  DVector2S(double xx, double yy){x=xx; y=yy;}
  ~DVector2S(){};
		
  double X(void) const {return x;}
  double Y(void) const {return y;}
  void Set(double xx, double yy){x=xx; y=yy;}
  double Mod2(void) const {return x*x+y*y;}
  double Mod(void) const {return sqrt(Mod2());}
  double Phi(void) const {return atan2(y,x);}
  double Phi_0_2pi(double phi) const {
    while (phi>=2.*M_PI) phi -= 2.*M_PI;
    while (phi<0.) phi += 2.*M_PI;
    return phi;
  }
  DVector2S Rotate(double phi) const {
    double cosphi=cos(phi),sinphi=sin(phi);
    return DVector2S(X()*cosphi-Y()*sinphi,X()*sinphi+Y()*cosphi);
  }

  double operator*(const DVector2S &v) const {return X()*v.X()+Y()*v.Y();}
  DVector2S& operator*=(const double &f){x*=f; y*=f; return *(this);}
  DVector2S& operator/=(const double &f){x/=f; y/=f; return *(this);}
  DVector2S& operator+=(const DVector2S &v){x+=v.X(); y+=v.Y(); return *(this);}
  DVector2S& operator-=(const DVector2S &v){x-=v.X(); y-=v.Y(); return *(this);}
  
protected:
  
private:
  double x;
  double y;
};

inline DVector2S operator*(const double &f, const DVector2S &vec){
	DVector2S s(f*vec.X(), f*vec.Y());
	return s;
}

inline DVector2S operator*(const DVector2S &vec, const double &f){
	DVector2S s(f*vec.X(), f*vec.Y());
	return s;
}

inline DVector2S operator/(const DVector2S &vec, const double &f){
	DVector2S s(vec.X()/f, vec.Y()/f);
	return s;
}

inline DVector2S operator+(const DVector2S &vec1, const DVector2S &vec2){
	DVector2S s(vec1.X()+vec2.X(), vec1.Y()+vec2.Y());
	return s;
}

inline DVector2S operator-(const DVector2S &vec1, const DVector2S &vec2){
	DVector2S s(vec1.X()-vec2.X(), vec1.Y()-vec2.Y());
	return s;
}


#endif // _DVector2S_

