//********************************************************************************************
// DTRDSegment.h : definition for a track segment built from points - derived from DFDCSegment
//********************************************************************************************

#ifndef DTRDSEGMENT_H
#define DTRDSEGMENT_H

#include <JANA/JObject.h>
using namespace jana;

#include "DTRDHit.h"
#include "DTRDPoint.h"

#include <DMatrix.h>
#include <sstream>


///
/// class DTRDSegment: definition for a track segment in the TRD
/// 
class DTRDSegment : public JObject {
public :
  JOBJECT_PUBLIC(DTRDSegment);			/// DANA identifier
  DTRDSegment(){}

  double x,y,tx,ty;
  double var_x,var_y,var_tx,var_ty;
  
  void Summarize(JObjectSummary& summary) const override {
    summary.add(x, "x", "%3.2f");
    summary.add(y, "y", "%3.2f");
    summary.add(tx, "tx", "%3.2f");
    summary.add(ty, "ty", "%3.2f");
    summary.add(var_x, "var_x", "%3.2f");
    summary.add(var_y, "var_y", "%3.2f");
    summary.add(var_tx, "var_tx", "%3.2f");
    summary.add(var_ty, "var_ty", "%3.2f");
		
  }
};

#endif //DTRDSEGMENT_H
