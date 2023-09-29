// $Id$
//
//    File: DGEMTRDSegment_factory.cc
// Created: Wed Feb  9 13:19:03 EST 2022
// Creator: staylor (on Linux ifarm1901.jlab.org 3.10.0-1160.11.1.el7.x86_64 x86_64)
//
// Groups hits in the GEMTRD into segments


#include <iostream>
#include <iomanip>
using namespace std;

#include "DGEMTRDSegment_factory.h"
using namespace jana;

bool static GEMTRD_cmp(const DGEMTRDHit *a,const DGEMTRDHit *b){
  return (a->t < b->t);
}


//------------------
// init
//------------------
jerror_t DGEMTRDSegment_factory::init(void)
{
  return NOERROR;
}

//------------------
// brun
//------------------
jerror_t DGEMTRDSegment_factory::brun(jana::JEventLoop *eventLoop, int32_t runnumber)
{
  return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t DGEMTRDSegment_factory::evnt(JEventLoop *loop, uint64_t eventnumber)
{
  vector<const DGEMTRDHit*>hits;
  loop->Get(hits);
  if (hits.size()==0) return RESOURCE_UNAVAILABLE;

  stable_sort(hits.begin(),hits.end(),GEMTRD_cmp);
  
  vector<bool>used_in_segment(hits.size());
  vector<vector<const DGEMTRDHit *>>segments;
  vector<const DGEMTRDHit *>segment;

  int num_left=hits.size();
  unsigned int ibegin=0;
  // Loop over the hits, grouping them together by proximity into track segments
  while (num_left>0){
    used_in_segment[ibegin]=true;
    segment.push_back(hits[ibegin]);
    num_left--;

    double x1=hits[ibegin]->x;
    double y1=hits[ibegin]->y;
    for (unsigned int i=ibegin+1;i<hits.size();i++){
      if (used_in_segment[i]==false){
	double x2=hits[i]->x;
	double y2=hits[i]->y;
	double dx=x1-x2;
	double dy=y1-y2;
	double dr=sqrt(dx*dx+dy*dy);
	if (dr<R_CUT){
	  segment.push_back(hits[i]);
	  used_in_segment[i]=true;
	  num_left--;
	}
	x1=x2;
	y1=y2;
      }
    }
    if (segment.size()>2){
      segments.push_back(segment);
    }
    segment.clear();

    for (unsigned int i=ibegin+1;i<hits.size();i++){
      if (used_in_segment[i]==false){
	ibegin=i;
	break;
      }
    }    
  }

  double Dt=4.6e-6; // transverse diffusion coefficient, cm^2/ns
  for (unsigned int i=0;i<segments.size();i++){
    // Perform line fits to find the slopes and intercepts in x and y at 
    // the downstream end of the drift region
    double sumv=1e4,sumz=0,sumzz=0.,sumxz=0,sumyz=0.;
    double t0=segments[i][0]->t;
    double sumx=segments[i][0]->x*1e4;  // scale by expected variance
    double sumy=segments[i][0]->y*1e4;
    for (unsigned int j=1;j<segments[i].size();j++){
      const DGEMTRDHit *hit=segments[i][j];
      // Use linear time-to-distance relationship for now
      double dt=hit->t-t0;
      double z=0.0033*dt;
      double one_over_varxy=1./(0.01*0.01+2.*Dt*dt);
      sumv+=one_over_varxy;
      sumz+=z*one_over_varxy;
      sumzz+=z*z*one_over_varxy;
      sumy+=hit->y*one_over_varxy;
      sumyz+=z*hit->y*one_over_varxy;
      sumx+=hit->x*one_over_varxy;
      sumxz+=z*hit->x*one_over_varxy;
    }
    double denom=sumv*sumzz-sumz*sumz;
    double y0=(sumzz*sumy-sumz*sumyz)/denom;
    double dydz=-(sumv*sumyz-sumz*sumy)/denom;

    double x0=(sumzz*sumx-sumz*sumxz)/denom;
    double dxdz=-(sumv*sumxz-sumz*sumx)/denom;

    // Covariance matrix for (s)lope and (i)ntercept
    double var_i=sumzz/denom;
    double var_s=sumv/denom;
    double cov_is=-sumz/denom;

    DGEMTRDSegment *mysegment=new DGEMTRDSegment;
    mysegment->layer=segments[i][0]->layer;
    mysegment->t=segments[i][0]->t;
    mysegment->x=x0;
    mysegment->y=y0;
    mysegment->dxdz=dxdz;
    mysegment->dydz=dydz;
    mysegment->var_x=var_i;
    mysegment->var_tx=var_s;
    mysegment->cov_xtx=cov_is;

    _data.push_back(mysegment);
  }


  return NOERROR;
}

//------------------
// erun
//------------------
jerror_t DGEMTRDSegment_factory::erun(void)
{
  return NOERROR;
}

//------------------
// fini
//------------------
jerror_t DGEMTRDSegment_factory::fini(void)
{
  return NOERROR;
}

