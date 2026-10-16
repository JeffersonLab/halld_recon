// $Id$
//
//    File: DPSCPair_factory.cc
// Created: Tue Mar 24 21:35:49 EDT 2015
// Creator: nsparks (on Linux cua2.jlab.org 2.6.32-431.5.1.el6.x86_64 x86_64)
//


#include <iostream>
#include <iomanip>
#include <math.h>
using namespace std;

#include "DPSCPair_factory.h"
#include "DPSCHit.h"

#include <JANA/JEvent.h>


inline bool DPSCPair_SortByTimeDifference(const DPSCPair* pair1, const DPSCPair* pair2)
{
  double tdiff1 = fabs(pair1->ee.first->t-pair1->ee.second->t);
  double tdiff2 = fabs(pair2->ee.first->t-pair2->ee.second->t);
  return (tdiff1<tdiff2);
}

//------------------
// Init
//------------------
void DPSCPair_factory::Init()
{
  auto app = GetApplication();
  DELTA_T_PAIR_MAX = 10.0; // ns
  app->SetDefaultParameter("PSCPair:DELTA_T_PAIR_MAX",DELTA_T_PAIR_MAX,
			      "Maximum difference in ns between a pair of hits"
			      " in left and right arm of coarse PS");
  app->SetDefaultParameter("PSCPair:USE_ADC_HITS", USE_ADC_HITS, "Require PSC hits to have a fADC time (default: true)");
  app->SetDefaultParameter("PSCPair:USE_TDC_HITS", USE_TDC_HITS, "Require PSC hits to have a TDC time (default: true)");
  
}

//------------------
// BeginRun
//------------------
void DPSCPair_factory::BeginRun(const std::shared_ptr<const JEvent>& event)
{
}

//------------------
// Process
//------------------
void DPSCPair_factory::Process(const std::shared_ptr<const JEvent>& event)
{
  if(!USE_ADC_HITS && !USE_TDC_HITS)
  	return;

  // get coarse pair spectrometer hits
  vector<const DPSCHit*> hits;
  event->Get(hits);
  // form PSC left-right hit pairs and sort by time difference
  pair<const DPSCHit*,const DPSCHit*> ee;
  if (hits.size()>1) {
    for (unsigned int i=0; i < hits.size()-1; i++) {
      for (unsigned int j=i+1; j < hits.size(); j++) {
          if (USE_TDC_HITS && (!hits[i]->has_TDC||!hits[j]->has_TDC)) continue;
          if (USE_ADC_HITS && (!hits[i]->has_fADC||!hits[j]->has_fADC)) continue;
          
          double t1 = -1.e6, t2 = -1.e6;
          if( USE_ADC_HITS && USE_TDC_HITS ) {
          	t1 = hits[i]->t;
          	t2 = hits[j]->t;
          } else if(!USE_TDC_HITS) {
          	t1 = hits[i]->time_fadc;
          	t2 = hits[j]->time_fadc;
          } else {  // !USE_ADC_HITS
          	t1 = hits[i]->t;
          	t2 = hits[j]->t;
          }
          
          if ( (std::abs(hits[i]->arm-hits[j]->arm)==1&&fabs(t1-t2)<DELTA_T_PAIR_MAX) ) {
              if (hits[i]->arm==0) {
                  ee.first = hits[i];
                  ee.second = hits[j];
              }
              else if (hits[i]->arm==1) {
                  ee.first = hits[j];
                  ee.second = hits[i];
              }
              DPSCPair *pair = new DPSCPair;
              pair->ee = ee;
              Insert(pair);
          }
      }
    }
  }
  sort(mData.begin(),mData.end(),DPSCPair_SortByTimeDifference);
}

//------------------
// EndRun
//------------------
void DPSCPair_factory::EndRun()
{
}

//------------------
// Finish
//------------------
void DPSCPair_factory::Finish()
{
}

