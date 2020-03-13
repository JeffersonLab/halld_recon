//***********************************************************************
// DGEMPoint.h : modeled after DFDCPseudo 
//***********************************************************************

#ifndef DGEMPOINT_H
#define DGEMPOINT_H

#include <JANA/JObject.h>
using namespace jana;

#include "DGEMHit.h"
#include "DGEMStripCluster.h"
#include <vector>


///
/// class DGEMPoint: definition for a reconstructed point in the FDC
/// 
class DGEMPoint : public JObject {
   public :
      JOBJECT_PUBLIC(DGEMPoint);			/// DANA identifier

      /// 
      /// DGEMPoint::DGEMPoint():
      /// Default constructor-- provide the X, Y, and resolution
      ///
      DGEMPoint(){}

      double x,y,z; ///< centroid positions from two planes
      double t_x,t_y; ///< time of the two cathode clusters
      double time; ///< time corresponding to this pseudopoint.
      int status; ///< status word for pseudopoint
      int detector; ///< GEM plane
      double covxx,covxy,covyy; ///< Covariance terms for (x,y) 
      double dE_amp; /// < energy deposition, from pulse height
      int itrack;

      void toStrings(vector<pair<string,string> > &items)const{ 
         AddString(items,"x","%3.2f",x);
         AddString(items,"y","%3.2f",y);
	 AddString(items,"z","%3.2f",z);
         AddString(items,"t_x","%3.2f",t_x);
         AddString(items,"t_y","%3.2f",t_y);
         AddString(items, "time", "%3.1f", time);
         AddString(items, "status", "%d", status);
	 AddString(items, "detector", "%d", detector);
         AddString(items, "dE_amp", "%3.1f", dE_amp);
      }

};

#endif //DGEMPOINT_H
