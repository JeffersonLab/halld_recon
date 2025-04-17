//******************************************************************
// DTRDSegment_factory_extrapolation.h: class definition for a factory creating
// track segments from points and track extrapolations
//******************************************************************
#ifndef DFACTORY_DTRDSEGMENT_EXTRAPOLATION_H
#define DFACTORY_DTRDSEGMENT_EXTRAPOLATION_H

#include "JANA/JFactoryT.h"
#include "JANA/JException.h"

#include "DTRDSegment.h"
#include "DTRDPoint.h"
#include "PID/DChargedTrack.h"

#include <algorithm>
#include <map>
#include <cmath>

class DTRDSegment_factory_extrapolation : public JFactoryT<DTRDSegment> {
public:
  DTRDSegment_factory_extrapolation(){ SetTag("Extrapolation"); };
  ~DTRDSegment_factory_extrapolation();
  struct TrackPoint {
    const DTRDPoint *point;
    int trackID;
    bool tagged;
    TrackPoint(const DTRDPoint *point) : point(point), trackID(-1), tagged(false) {};
  };
  
protected:
  void Init() override;
  void BeginRun(const std::shared_ptr<const JEvent>& event) override;
  void Process(const std::shared_ptr<const JEvent>& event) override;
  // 		void EndRun() override;
  // 		void Finish() override;
  
private:
  void FindSegments(const vector<const DTRDPoint *>&points,
		    vector<vector<const DTRDPoint *>>&segments) const;
  void FitLine(const vector<const DTRDPoint *>&points,
	       double &x0,double &y0,double &tx,double &ty,
	       double &var_x,double &var_y,double &var_tx,double &var_ty) const;
  void FindSegmentPoints(vector<TrackPoint> &trackPoints, vector<DTrackFitter::Extrapolation_t> &trackExtrapolations, double distToExtrp);
  double dTRDz;
  double distToExtrp;
  int DEBUG_LEVEL;
};

#endif // DFACTORY_DTRDSEGMENT_H

