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
  double total_dE;

  int NumHitsX, NumHitsY;
  int NumStripClustersX, NumStripClustersY;
  int NumPoints, maxE_point_x, maxE_point_y, maxE_point_t, maxE_point_dE;

  void Summarize(JObjectSummary& summary) const override {
    summary.add(x, "x", "%3.2f");
    summary.add(y, "y", "%3.2f");
    summary.add(tx, "tx", "%3.2f");
    summary.add(ty, "ty", "%3.2f");
    summary.add(var_x, "var_x", "%3.2f");
    summary.add(var_y, "var_y", "%3.2f");
    summary.add(var_tx, "var_tx", "%3.2f");
    summary.add(var_ty, "var_ty", "%3.2f");
    summary.add(total_dE, "total_dE", "%3.2f");
    summary.add(NumHitsX, "NumHitsX", "%d");
    summary.add(NumHitsY, "NumHitsY", "%d");
    summary.add(NumStripClustersX, "NumStripClustersX", "%d");
    summary.add(NumStripClustersY, "NumStripClustersY", "%d");
    summary.add(NumPoints, "NumPoints", "%d");
    summary.add(maxE_point_x, "maxE_points_x", "%d");
    summary.add(maxE_point_y, "maxE_points_y", "%d");
    summary.add(maxE_point_t, "maxE_points_t", "%d");
    summary.add(maxE_point_dE, "maxE_points_dE", "%d");

  }
};

#endif //DTRDSEGMENT_H
