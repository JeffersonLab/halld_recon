//***********************************************************************
// DGEMPoint.h : modeled after DFDCPseudo 
//***********************************************************************

#ifndef DGEMPOINT_H
#define DGEMPOINT_H

#include <JANA/JObject.h>

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

      void Summarize(JObjectSummary& summary) const override {
         summary.add(x, "x", "%3.2f");
         summary.add(y, "y", "%3.2f");
         summary.add(z, "z", "%3.2f");
         summary.add(t_x, "t_x", "%3.2f");
         summary.add(t_y, "t_y", "%3.2f");
         summary.add(time, "time", "%3.1f");
         summary.add(status, "status", "%d");
         summary.add(detector, "detector", "%d");
         summary.add(dE_amp, "dE_amp", "%3.1f");
      }

};

#endif //DGEMPOINT_H
