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
      		}
      		
 	
	private:
      	
};


#endif  //  _DCCALShower_

