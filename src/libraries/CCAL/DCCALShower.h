/*
 *  File: DCCALHit_factory.h
 *
 * Created on 11/25/18 by A.S. 
 * use structure similar to FCAL
 */


#ifndef _DCCALShower_
#define _DCCALShower_

#include <math.h>
#include <DVector3.h>
#include <DLorentzVector.h>
#include <DMatrix.h>
#include <DMatrix.h>
#include <TMatrixFSym.h>
#include "DCCALHit.h"
#include "ccal.h"

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
      
      		/*----------------------------------------//
      
      		Description of DCCALShower variables:
      		
      		- E:       Energy of shower as reconstructed from island algorithm
      		- Esum:    Energy of shower calculated as simple sum of block energies
      		
      		- x , y :  Shower position reported from island algorithm
      		- x1, y1:  Shower position calculated using logarithmic weights
      		- z:       Z-Position of CCAL Face
      		
      		- dime:    Number of hits contributing to shower
      		- idmax:   The channel number of cell with maximum energy deposition
      		- id:      Only non-zero for showers that share a single peak
			
      		- chi2:    The chi2-value reported from island
      		- sigma_E: Energy resolution (needs work)
      		- Emax:    Energy of maximum cell
      		- time:    Energy-weighted average of shower's constituent times 
				(with time-walk correction)
      		
      		- ClusterType (enum):
        	
		  SinglePeak - shower is in a single-peak cluster
		  MultiPeak  - shower is in a multi-peak cluster
		
      		- PeakType (enum) :
      		
        	  OneGamma  - The peak this shower belongs to has just one gamma reconstructed
		  TwoGammas - The peak this shower belongs to has two gammas reconstructed
		
		
      		
      		//----------------------------------------*/
		
      		double E;
      		double Esum;
      		
      		double x;
      		double y;
      		double x1;
      		double y1;
      		double z;
      		
      		double chi2;
      		double sigma_E;
      		double Emax;
      		double time;
      		double sigma_t;
      		
      		int dime;
      		int idmax;
      		int id;
      		int type;
		
		ClusterType_t ClusterType;
		PeakType_t PeakType;
      		
		
		TMatrixFSym ExyztCovariance;
		
		float EErr() const { return sqrt(ExyztCovariance(0,0)); }
		float xErr() const { return sqrt(ExyztCovariance(1,1)); }
		float yErr() const { return sqrt(ExyztCovariance(2,2)); }
		float zErr() const { return sqrt(ExyztCovariance(3,3)); }
		float tErr() const { return sqrt(ExyztCovariance(4,4)); }
		float XYcorr() const {
		  if (xErr()>0 && yErr()>0) return ExyztCovariance(1,2)/xErr()/yErr();
		  else return 0;
		}
		float XZcorr() const {
		  if (xErr()>0 && zErr()>0) return ExyztCovariance(1,3)/xErr()/zErr();
		  else return 0;
		}
		float YZcorr() const {
		  if (yErr()>0 && zErr()>0) return ExyztCovariance(2,3)/yErr()/zErr();
		  else return 0;
		}
		float EXcorr() const {
		  if (EErr()>0 && xErr()>0) return ExyztCovariance(0,1)/EErr()/xErr();
		  else return 0;
		}
		float EYcorr() const {
		  if (EErr()>0 && yErr()>0) return ExyztCovariance(0,2)/EErr()/yErr();
		  else return 0;
		}
		float EZcorr() const {
		  if (EErr()>0 && zErr()>0) return ExyztCovariance(0,3)/EErr()/zErr();
		  else return 0;
		}
		float XTcorr() const {
		  if (xErr()>0 && tErr()>0) return ExyztCovariance(1,4)/xErr()/tErr();
		  else return 0;
		}
		float YTcorr() const {
		  if (yErr()>0 && tErr()>0) return ExyztCovariance(2,4)/yErr()/tErr();
		  else return 0;
		}
		float ZTcorr() const {
		  if (zErr()>0 && tErr()>0) return ExyztCovariance(3,4)/zErr()/tErr();
		  else return 0;
		}
		float ETcorr() const {
		  if (EErr()>0 && tErr()>0) return ExyztCovariance(0,4)/EErr()/tErr();
		  else return 0;
		}

      		void toStrings(vector<pair<string,string> > &items) const {
		  AddString(items, "E(GeV)",      "%2.3f",  E);
		  AddString(items, "Emax(GeV)",   "%2.3f",  Emax);
		  AddString(items, "x(cm)",       "%3.3f",  x);
		  AddString(items, "y(cm)",       "%3.3f",  y);
		  AddString(items, "x1(cm)",      "%3.3f",  x1);
		  AddString(items, "y1(cm)",      "%3.3f",  y1);
		  AddString(items, "z(cm)",       "%3.3f",  z);
		  AddString(items, "chi2",        "%3.3f",  chi2);
		  AddString(items, "dime",        "%3d",    dime);
		  AddString(items, "idmax",       "%3d",    idmax);
		  AddString(items, "id",          "%3d",    id);
		  AddString(items, "sigma_E",     "%3.1f",  sigma_E);
		  AddString(items, "t(ns)",       "%2.3f",  time);
		  AddString(items, "ClusterType", "%d",     (int)ClusterType);
		  AddString(items, "PeakType",    "%d",     (int)PeakType);

		  AddString(items, "EXcorr", "%5.3f", EXcorr());
		  AddString(items, "EYcorr", "%5.3f", EYcorr());
		  AddString(items, "EZcorr", "%5.3f", EZcorr());
		  AddString(items, "ETcorr", "%5.3f", ETcorr());
		  AddString(items, "XYcorr", "%5.3f", XYcorr());
		  AddString(items, "XZcorr", "%5.3f", XZcorr());
		  AddString(items, "XTcorr", "%5.3f", XTcorr());
		  AddString(items, "YZcorr", "%5.3f", YZcorr());
		  AddString(items, "YTcorr", "%5.3f", YTcorr());
		  AddString(items, "ZTcorr", "%5.3f", ZTcorr());
      		}
      		
         private:


};


#endif  //  _DCCALShower_

