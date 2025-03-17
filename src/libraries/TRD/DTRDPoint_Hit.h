//***********************************************************************
// DTRDPoint_Hit.h : modeled after DFDCPseudo 
//***********************************************************************

#ifndef DTRDPOINT_HIT_H
#define DTRDPOINT_HIT_H

#include <JANA/JObject.h>
//using namespace jana;
using namespace std;
#include "DTRDHit.h"
#include "DTRDStripCluster.h"
#include <vector>


///
/// class DTRDPoint_Hit: definition for a reconstructed point in the FDC
/// 
class DTRDPoint_Hit : public JObject {
   public :
      JOBJECT_PUBLIC(DTRDPoint_Hit);			/// DANA identifier

      /// 
      /// DTRDPoint_Hit::DTRDPoint_Hit():
      /// Default constructor-- provide the X, Y, and resolution
      ///
      DTRDPoint_Hit(){}

      double x,y,z; ///< centroid positions from two planes
      double t_x,t_y; ///< time of the two cathode clusters
      double time; ///< time corresponding to this pseudopoint.
      int status; ///< status word for pseudopoint
      double covxx,covxy,covyy; ///< Covariance terms for (x,y) 
      double dE; /// < energy deposition 
	  double dE_x,dE_y; /// < energy deposition for each plane
      //int itrack; //?

      void Summarize(JObjectSummary& summary) const override { 
         summary.add(x,"x","%3.2f");
         summary.add(y,"y","%3.2f");
	 	 summary.add(z,"z","%3.2f");
         summary.add(t_x,"t_x","%3.2f");
         summary.add(t_y,"t_y","%3.2f");
         summary.add(time, "time", "%3.1f");
         summary.add(status, "status", "%d");
         summary.add(dE, "dE", "%3.1f");
		 summary.add(dE_x, "dE_x", "%3.1f");
		 summary.add(dE_y, "dE_y", "%3.1f");
      }

};

#endif //DTRDPOINT_HIT_H
