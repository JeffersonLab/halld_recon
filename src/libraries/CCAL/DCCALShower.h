/*
 *  File: DCCALHit_factory.h
 *
 * Created on 11/25/18 by A.S. 
 * use structure similar to FCAL
 */


#ifndef _DCCALShower_
#define _DCCALShower_

#include <DVector3.h>
#include "DCCALHit.h"
#include "hycal.h"

using namespace std;

#include <JANA/JObject.h>
#include <JANA/JFactory.h>
using namespace jana;

#define CCAL_USER_HITS_MAX  1000

class DCCALShower : public JObject {
  public:
      JOBJECT_PUBLIC(DCCALShower);
      
      DCCALShower();
      ~DCCALShower();
      
      
      int type;
      int dime;
      int status;
      int id;
      int idmax;

      double E;
      double x;
      double y;
      double z;
      double x1;
      double y1;
      double chi2;
      double sigma_E;
      double Emax;
      double time;
      double sigma_t;
      
      int id_storage[MAX_CC];
      double en_storage[MAX_CC];
      double t_storage[MAX_CC];
      
      void toStrings(vector<pair<string,string> > &items) const {
	AddString(items, "E(GeV)",    "%2.3f",  E);
	AddString(items, "Emax(GeV)", "%2.3f",  Emax);
	AddString(items, "x(cm)",     "%3.1f",  x);
	AddString(items, "y(cm)",     "%3.1f",  y);
	AddString(items, "z(cm)",     "%3.1f",  z);
	AddString(items, "x1(cm)",    "%3.1f",  x1);
	AddString(items, "y1(cm)",    "%3.1f",  y1);
	AddString(items, "chi2",      "%3.1f",  chi2);
	AddString(items, "type",      "%3d",    type);
	AddString(items, "dime",      "%3d",    dime);
	AddString(items, "status",    "%3d",    status);
	AddString(items, "id",        "%3d",    id);
	AddString(items, "idmax",     "%3d",    idmax);
	AddString(items, "sigma_E",   "%3.1f",  sigma_E);
	AddString(items, "t(ns)",     "%2.3f",  time);
      }
      
 private:
      
};


#endif  //  _DCCALShower_

