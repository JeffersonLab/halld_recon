
// DTRDSegment_factory_extrapolation.cc - factory producing track segments from points
//************************************************************************

#include "DTRDSegment_factory_extrapolation.h"
#include "DANA/DApplication.h"
#include "DANA/DEvent.h"

//#include "HDGEOMETRY/DLorentzMapCalibDB.h"
#include <math.h>
#include <cmath>
using namespace std;

DTRDSegment_factory_extrapolation::~DTRDSegment_factory_extrapolation() {
}

void DTRDSegment_factory_extrapolation::Init() 
{
  auto app = GetApplication();
  
  // DEBUG_LEVEL=0;
  // app->SetDefaultParameter("TRD:DEBUG_LEVEL", DEBUG_LEVEL);

  distToExtrp = 2.0; // cm  
  app->SetDefaultParameter("TRD:DistToExtrp", distToExtrp, "Distance to extrapolation point (default: 2.0 cm)");
}

void DTRDSegment_factory_extrapolation::BeginRun(const std::shared_ptr<const JEvent>& event) 
{ 
  // get the geometry
  DGeometry *geom = DEvent::GetDGeometry(event);
  // Get GEM geometry from xml (CCDB or private HDDS)
  geom->GetGEMTRDz(dTRDz);
  
  return;
}

///
/// DFDCSegment_factory::evnt():
/// Routine where pseudopoints are combined into track segments
///
void DTRDSegment_factory_extrapolation::Process(const std::shared_ptr<const JEvent>& event)
{
  vector<const DTRDPoint*> points;
  event->Get<DTRDPoint>(points);
  
  vector<const DChargedTrack*> tracks; 
  event->Get(tracks);

  if (tracks.size()==0) {
    return;
  }

  // cout << "Number of tracks: " << tracks.size() << endl;

  vector<TrackPoint> trackPoints;
  vector<DTrackFitter::Extrapolation_t> trackExtrapolations;
  for (auto &track: tracks) {
    const DChargedTrackHypothesis *hypElectron=track->Get_Hypothesis(Electron);
    const DChargedTrackHypothesis *hypPositron=track->Get_Hypothesis(Positron);   
    const DChargedTrackHypothesis *hyp = (hypElectron != nullptr) ? hypElectron : hypPositron;
    if (hyp == nullptr) continue;
    const DTrackTimeBased *trackTB=hyp->Get_TrackTimeBased();
    if (trackTB==nullptr) continue; 
    vector<DTrackFitter::Extrapolation_t> extrapolations = trackTB->extrapolations.at(SYS_TRD);
    if (extrapolations.size()==0) continue;
    DTrackFitter::Extrapolation_t extrapolation = extrapolations[0];

    if ((extrapolations[0].position.x() < -83.47 || extrapolations[0].position.x() > -11.47) || 
        (extrapolations[0].position.y() < -68.6 || extrapolations[0].position.y() > -32.61)) continue;
    
    trackExtrapolations.push_back(extrapolation);
  }

  for (auto &point: points) {
    trackPoints.emplace_back(point);
  }

  FindSegmentPoints(trackPoints, trackExtrapolations, distToExtrp);

  vector<vector<const DTRDPoint *>> segments_TRDPoints;
  for (unsigned int iSegmentPoint=0;iSegmentPoint<trackExtrapolations.size();iSegmentPoint++){
    vector<const DTRDPoint *> segmentPoints;
    for (unsigned int i=0;i<trackPoints.size();i++){
      // cout << "TrackID: " << trackPoints[i].trackID << " x: " << trackPoints[i].point->x << " y: " << trackPoints[i].point->y << endl;
      if (trackPoints[i].trackID==iSegmentPoint){
        segmentPoints.push_back(trackPoints[i].point);
      }
    }
    segments_TRDPoints.push_back(segmentPoints);
  }

  for (unsigned int i=0;i<trackExtrapolations.size();i++){
    if (segments_TRDPoints[i].size()==0) continue;
    DTRDSegment *myTRDSegment = new DTRDSegment;
    double x=0,y=0,tx=0,ty=0;
    double var_x=0.,var_y=0.,var_tx=0.,var_ty=0.;
    FitLine(segments_TRDPoints[i],x,y,tx,ty,var_x,var_y,var_tx,var_ty);
    
    myTRDSegment->x=x;
    myTRDSegment->y=y;
    myTRDSegment->tx=tx;
    myTRDSegment->ty=ty;
    myTRDSegment->var_x=var_x;
    myTRDSegment->var_y=var_y;
    myTRDSegment->var_tx=var_tx;
    myTRDSegment->var_ty=var_ty;
    
    Insert(myTRDSegment);  
  }

  return;
}  
  
void DTRDSegment_factory_extrapolation::FindSegments(const vector<const DTRDPoint *>&points,
				      vector<vector<const DTRDPoint *>>&segments) const{
  vector<const DTRDPoint *>mysegment_points;
  for (unsigned int i=0;i<points.size();i++){
    mysegment_points.push_back(points[i]);
  }
  segments.push_back(mysegment_points);

}


void DTRDSegment_factory_extrapolation::FitLine(const vector<const DTRDPoint *>&points,
				  double &x0,double &y0,double &tx,double &ty,
				  double &var_x,double &var_y,double &var_tx,
				  double &var_ty) const{
  double varx=0.01,vary=0.01; // just a guess for now
  double S1=0,S1z=0.,S1y=0.,S1zz=0.,S1zy=0.;
  double S2=0,S2z=0.,S2x=0.,S2zz=0.,S2zx=0.; 
  for (unsigned int i=0;i<points.size();i++){
    double x=points[i]->x;
    double y=points[i]->y;
    double z=points[i]->z-dTRDz;
    double one_over_var1=1/vary;
    double one_over_var2=1/varx;

    S1+=one_over_var1;
    S1z+=z*one_over_var1;
    S1y+=y*one_over_var1;
    S1zz+=z*z*one_over_var1;
    S1zy+=z*y*one_over_var1;    
    
    S2+=one_over_var2;
    S2z+=z*one_over_var2;
    S2x+=x*one_over_var2;
    S2zz+=z*z*one_over_var2;
    S2zx+=z*x*one_over_var2;
   }
  
   double D1=S1*S1zz-S1z*S1z;
   y0=(S1zz*S1y-S1z*S1zy)/D1;
   var_y=S1zz/D1;
   ty=(S1*S1zy-S1z*S1y)/D1;
   var_ty=S1/D1;

   double D2=S2*S2zz-S2z*S2z;
   x0=(S2zz*S2x-S2z*S2zx)/D2;
   var_x=S2zz/D2;
   tx=(S2*S2zx-S2z*S2x)/D2;
   var_tx=S2/D2;
}

void DTRDSegment_factory_extrapolation::FindSegmentPoints(vector<TrackPoint> &trackPoints, vector<DTrackFitter::Extrapolation_t> &trackExtrapolations, double distToExtrp)
{
  for (int itrack=0;itrack<trackExtrapolations.size();itrack++){
    DTrackFitter::Extrapolation_t extrapolation = trackExtrapolations[itrack];
    double x0 = extrapolation.position.x();
    double y0 = extrapolation.position.y();
    double z0 = extrapolation.position.z();
    double dxdz = extrapolation.momentum.x()/extrapolation.momentum.z();
    double dydz = extrapolation.momentum.y()/extrapolation.momentum.z();

    auto dist = [x0, dxdz, y0, dydz, z0](double x, double y, double z) -> double {
      double x1 = x0 + dxdz*(z-z0);
      double y1 = y0 + dydz*(z-z0);
      return sqrt((x-x1)*(x-x1) + (y-y1)*(y-y1));
    };
    
    for (auto &point: trackPoints) {
      if (point.tagged) continue;
      double xpoint = point.point->x;
      double ypoint = point.point->y;
      double zpoint = point.point->z;

      // cout << "Extrapolation: " << x0 << ", " << y0 << ", " << z0 << endl;
      // cout << "Point: " << xpoint << ", " << ypoint << ", " << zpoint << endl;
      
      // cout << "Distance to extrapolation: " << dist(xpoint, ypoint, zpoint) << ", "
      //      << "Distance to extrapolation threshold: " << distToExtrp << endl;

      if (dist(xpoint, ypoint, zpoint) < distToExtrp) {
        point.trackID = itrack;
        point.tagged = true;
      }
    }
  }
}
