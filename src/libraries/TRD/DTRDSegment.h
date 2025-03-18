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

  double x0,y0,tx,ty;
  
  void Summarize(JObjectSummary& summary) const override {
    summary.add(x0, "x0", "%3.2f");
    summary.add(y0, "y0", "%3.2f");
    summary.add(tx, "tx", "%3.2f");
    summary.add(ty, "ty", "%3.2f");
  }
};

#endif //DTRDSEGMENT_H
