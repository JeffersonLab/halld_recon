//***********************************************************************
// DTRDPoint.h : modeled after DFDCPseudo 
//***********************************************************************

#ifndef DTRDPOINT_H
#define DTRDPOINT_H

#include <JANA/JObject.h>

#include "DTRDHit.h"
#include "DTRDStripCluster.h"
#include <vector>

using namespace std;

///
/// class DTRDPoint: definition for a reconstructed point in the FDC
/// 
class DTRDPoint : public JObject {
   public :
      JOBJECT_PUBLIC(DTRDPoint);			/// DANA identifier

      /// 
      /// DTRDPoint::DTRDPoint():
      /// Default constructor-- provide the X, Y, and resolution
      ///
      DTRDPoint(){}

      double x,y,z; ///< centroid positions from two planes
      double t_x,t_y; ///< time of the two cathode clusters
      double time; ///< time corresponding to this pseudopoint.
      int status; ///< status word for pseudopoint
      double covxx,covxy,covyy; ///< Covariance terms for (x,y) 
      double dE; /// < energy deposition 
      double dE_x,dE_y; /// < energy deposition for each plane
	  //int itrack; //?

      void Summarize(JObjectSummary& summary) const override {
         summary.add(x, "x", "%3.2f");
         summary.add(y, "y", "%3.2f");
         summary.add(z, "z", "%3.2f");
         summary.add(t_x, "t_x", "%3.2f");
         summary.add(t_y, "t_y", "%3.2f");
         summary.add(time, "time", "%3.1f");
         summary.add(status, "status", "%d");
         summary.add(dE, "dE", "%3.1f");
		 summary.add(dE_x, "dE_x", "%3.1f");
		 summary.add(dE_y, "dE_y", "%3.1f");
      }

};

#endif //DTRDPOINT_H
