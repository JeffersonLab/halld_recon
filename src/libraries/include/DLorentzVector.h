// $Id$
//
//    File: DLorentzVector.h
//

#ifndef _DLorentzVector_
#define _DLorentzVector_

#include "DVector3.h"
#include <math.h>
#include <iostream>
using namespace std;

class DLorentzVector{
 public:
  DLorentzVector(){
    mP.SetXYZ(0.,0.,0.);
    mE=0.;
  };
  DLorentzVector(const double x,const double y,const double z,const double t){
    mP.SetXYZ(x,y,z);
    mE=t;
  };
  DLorentzVector(const DVector3 &v, const double t){
    mP=v;
    mE=t;
  };
  ~DLorentzVector(){};
  
  // Copy and assignment 
  DLorentzVector(const DLorentzVector &v){mP=v.Vect(); mE=v.E();}
  DLorentzVector& operator=(const DLorentzVector &v){
    mP=v.Vect(); mE=v.E(); return *this;
  }

  void SetXYZT(const double x,const double y,const double z,const double t){
    mP.SetXYZ(x,y,z);
    mE=t;
  }
  // Set the 3-momentum or position part of the 4-vector
  void SetVect(const DVector3 &p){
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
  DVector3 Vect() const {return mP;};

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
  void Boost(const DVector3 &bvec){
    double b2=bvec.Mag2(); 
    double gamma = 1./sqrt(1.0 - b2);
    double bp= Vect().Dot(bvec);
    double gamma2 = b2 > 0 ? (gamma - 1.0)/b2 : 0.0;
    SetX(X() + gamma2*bp*bvec.x() + gamma*bvec.x()*T());
    SetY(Y() + gamma2*bp*bvec.y() + gamma*bvec.y()*T());
    SetZ(Z() + gamma2*bp*bvec.z() + gamma*bvec.z()*T());
    SetT(gamma*(T() + bp));
  }
  DVector3 BoostVector() const { return DVector3(X()/T(), Y()/T(), Z()/T());}

  // Addition and subtraction
  DLorentzVector &operator+=(const DLorentzVector &v1){
    mP+=v1.Vect();
    mE+=v1.E();
    return *this;
  } 
  DLorentzVector &operator-=(const DLorentzVector &v1){
    mP-=v1.Vect();
    mE-=v1.E();
    return *this;
  } 
  // Unary minus
  DLorentzVector operator-() const {return DLorentzVector(-X(),-Y(),-Y(),-T());}

  void Print() const{
    cout << "DLorentzVector (x,y,z,t)=(" << X() << "," << Y() << "," << Z()
	 << "," << T() << ")" << endl;

  };

 private:
  DVector3 mP;  // momentum or position vector
  double mE;  // Energy or time component
};

// Addition 
inline DLorentzVector operator+(const DLorentzVector &v1,const DLorentzVector &v2){
  return DLorentzVector(v1.Vect()+v2.Vect(),v1.E()+v2.E());
}
//Subtraction 
inline DLorentzVector operator-(const DLorentzVector &v1,const DLorentzVector &v2){
  return DLorentzVector(v1.Vect()-v2.Vect(),v1.E()-v2.E());
}

// Scaling by a constant factor
inline DLorentzVector operator*(const double &c, const DLorentzVector &v) {
  return DLorentzVector(c*v.Vect(),c*v.T());
}


#endif // _DLorentzVector_

