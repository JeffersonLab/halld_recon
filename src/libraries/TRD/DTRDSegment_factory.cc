
// DTRDSegment_factory.cc - factory producing track segments from points
//************************************************************************

#include "DTRDSegment_factory.h"
#include "DANA/DApplication.h"
#include "DANA/DEvent.h"

//#include "HDGEOMETRY/DLorentzMapCalibDB.h"
#include <math.h>
#include <cmath>
using namespace std;

DTRDSegment_factory::DTRDSegment_factory() {
}

DTRDSegment_factory::~DTRDSegment_factory() {
}

void DTRDSegment_factory::Init() 
{
  auto app = GetApplication();
  
  DEBUG_LEVEL=0;
  app->SetDefaultParameter("TRD:DEBUG_LEVEL", DEBUG_LEVEL);
}

void DTRDSegment_factory::BeginRun(const std::shared_ptr<const JEvent>& event) 
{ 
	map<string,string> installed;
	DEvent::GetCalib(event, "/TRD/install_status", installed);
	if(atoi(installed["status"].data()) == 0)
		INSTALLED = false;
	else
		INSTALLED = true;
		
	if(!INSTALLED) return;

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
void DTRDSegment_factory::Process(const std::shared_ptr<const JEvent>& event)
{
  vector<const DTRDPoint*> points;
  event->Get<DTRDPoint>(points);
  //event->Get<DTRDPoint>(points,"Hit");  
  
  vector<vector<const DTRDPoint *>>segments;
  FindSegments(points,segments);
  
  for (unsigned int i=0;i<segments.size();i++){
    vector<const DTRDPoint *>segment=segments[i];
    if (segment.size()>1){
      DTRDSegment *myTRDSegment = new DTRDSegment;
      double x=0,y=0,tx=0,ty=0;
      double var_x=0.,var_y=0.,var_tx=0.,var_ty=0.;
      FitLine(segment,x,y,tx,ty,var_x,var_y,var_tx,var_ty);
      
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
  }

  return;
}  
  
void DTRDSegment_factory::FindSegments(const vector<const DTRDPoint *>&unsorted_points,
				      vector<vector<const DTRDPoint *>>&segments) const{
  double MAX_T=1; //45 degrees
  double MAX_RESID=1; // 1 cm

  const int MAX_POINTS=30;
  if (unsorted_points.size()>MAX_POINTS)
    return;

  auto points = unsorted_points;  //convert this into a non const.
  
  std::sort(points.begin(), points.end(), [](const DTRDPoint* a, const DTRDPoint* b) {
        return a->z < b->z; // Change to > for descending
    });
  
  std::array<bool, MAX_POINTS> matched{}; 
  for (unsigned int i=0;i<points.size();i++){
    if(matched[i])
      continue;
    // Do some pattern recognition here
    for(unsigned int j=points.size()-1;j>i; j--){
      if(matched[j])
	continue;
      double tx=(points[i]->x-points[j]->x)/(points[i]->z-points[j]->z);
      double ty=(points[i]->y-points[j]->y)/(points[i]->z-points[j]->z);
      if (hypot(tx,ty)>MAX_T)
	continue;
      vector<const DTRDPoint *>mysegment_points;
      mysegment_points.push_back(points[i]);
      mysegment_points.push_back(points[j]);
      double x1=points[i]->x;
      double y1=points[i]->y;
      double z1=points[i]->z;
      for(unsigned int k=i+1;k<j; k++){
	if (matched[k])
	  continue;
	// check interpolation residual
	double residx=points[k]->x-(x1+tx*(points[k]->z-z1));
	double residy=points[k]->y-(y1+ty*(points[k]->z-z1));
	if(hypot(residx,residy)>MAX_RESID)
	  continue;
	mysegment_points.push_back(points[k]);
	matched[k]=true;
      }
      segments.push_back(mysegment_points);
    }
  }
}


void DTRDSegment_factory::FitLine(const vector<const DTRDPoint *>&points,
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
