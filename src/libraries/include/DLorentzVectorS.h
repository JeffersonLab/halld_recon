// $Id$
//
//    File: DLorentzVectorS.h
//

#ifndef _DLorentzVectorS_
#define _DLorentzVectorS_

#include "DVector3S.h"
#include <math.h>
#include <iostream>
using namespace std;

class DLorentzVectorS{
 public:
  DLorentzVectorS(){
    mP.SetXYZ(0.,0.,0.);
    mE=0.;
  };
  DLorentzVectorS(const double x,const double y,const double z,const double t){
    mP.SetXYZ(x,y,z);
    mE=t;
  };
  DLorentzVectorS(const DVector3S &v, const double t){
    mP=v;
    mE=t;
  };
  ~DLorentzVectorS(){};
  void SetXYZT(const double x,const double y,const double z,const double t){
    mP.SetXYZ(x,y,z);
    mE=t;
  }
  void SetXYZM(const double x,const double y,const double z,const double m){
    mP.SetXYZ(x,y,z);
    mE=sqrt(m*m+mP.Mag2());
  }
  
  // Set the 3-momentum or position part of the 4-vector
  void SetVect(const DVector3S &p){
    mP=p;
  }
  // Set the time or energy component
  void SetT(const double t){ mE=t;};
  void SetE(const double E){ mE=E;};
  // Set position components
  void SetX(const double x){mP.SetX(x);};
  void SetY(const double y){mP.SetY(y);};
  void SetZ(const double z){mP.SetZ(z);};
  
  // Routines to get position and time
  double X() const {return mP.x();};
  double Y() const {return mP.y();};
  double Z() const {return mP.z();}; 
  double T() const {return mE;};

  // Routine to get full 3-vector;
  DVector3S Vect() const {return mP;};

  // Routines to get momentum and energy
  double Px() const {return mP.x();};
  double Py() const {return mP.y();};
  double Pz() const {return mP.z();}; 
  double Pt() const {return mP.Perp();};
  double P() const {return mP.Mag();};
  double E() const {return mE;};
  double Energy() const {return mE;};

  // Spherical coordinates of spatial component
  double Rho() const { return mP.Mag();};
  double Perp() const { return mP.Perp();};

  // Angles
  double Theta() const {return mP.Theta();};
  double Phi() const {return mP.Phi();};

  // Kinematical quantities 
  double Beta() const { return P()/E();};
  double Mag2() const {return mE*mE-mP.Mag2();};
  double M() const{
    double mm = Mag2();
    return mm < 0.0 ? -sqrt(-mm) : sqrt(mm);
  }
  double M2() const {return Mag2();};
  double Mag() const {return M();};

  //Boost this Lorentz vector
  void Boost(const DVector3S &bvec){
    double b2=bvec.Mag2(); 
    double gamma = 1./sqrt(1.0 - b2);
    double bp= Vect().Dot(bvec);
    double gamma2 = b2 > 0 ? (gamma - 1.0)/b2 : 0.0;
    SetX(X() + gamma2*bp*bvec.x() + gamma*bvec.x()*T());
    SetY(Y() + gamma2*bp*bvec.y() + gamma*bvec.y()*T());
    SetZ(Z() + gamma2*bp*bvec.z() + gamma*bvec.z()*T());
    SetT(gamma*(T() + bp));
  }
  // Get boost vector (momentum scaled by energy)
  DVector3S BoostVector() const {
    return DVector3S(Px()/E(), Py()/E(), Pz()/E());
  }

  // Addition and subtraction
  DLorentzVectorS &operator+=(const DLorentzVectorS &v1){
    mP+=v1.Vect();
    mE+=v1.E();
    return *this;
  } 
  DLorentzVectorS &operator-=(const DLorentzVectorS &v1){
    mP-=v1.Vect();
    mE-=v1.E();
    return *this;
  }

  void Print() const{
    cout << "DLorentzVectorS (x,y,z,t)=(" << X() << "," << Y() << "," << Z()
	 << "," << T() << ")" << endl;

  };

 private:
  DVector3S mP;  // momentum or position vector
  double mE;  // Energy or time component
};

// Addition 
inline DLorentzVectorS operator+(const DLorentzVectorS &v1,const DLorentzVectorS &v2){
  return DLorentzVectorS(v1.Vect()+v2.Vect(),v1.E()+v2.E());
}
//Subtraction 
inline DLorentzVectorS operator-(const DLorentzVectorS &v1,const DLorentzVectorS &v2){
  return DLorentzVectorS(v1.Vect()-v2.Vect(),v1.E()-v2.E());
}

// Scaling by a constant factor
inline DLorentzVectorS operator*(const double &c, const DLorentzVectorS &v) {
  return DLorentzVectorS(c*v.Vect(),c*v.T());
}


#endif // _DLorentzVectorS_

