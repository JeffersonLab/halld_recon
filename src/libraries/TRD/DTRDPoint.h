//***********************************************************************
// DTRDPoint.h : modeled after DFDCPseudo 
//***********************************************************************

#ifndef DTRDPOINT_H
#define DTRDPOINT_H

#include <JANA/JObject.h>

#include "DTRDHit.h"
#include "DTRDStripCluster.h"
#include <vector>


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
      int detector; ///< WIRE = 0 and GEM = 1
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

#endif //DTRDPOINT_H
