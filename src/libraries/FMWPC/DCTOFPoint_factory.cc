// $Id$
//
//    File: DCTOFPoint_factory.cc
// Created: Thu Oct 28 07:48:04 EDT 2021
// Creator: staylor (on Linux ifarm1901.jlab.org 3.10.0-1062.4.1.el7.x86_64 x86_64)
//

/// Factory to combine hits on both ends of each paddle into CTOF "points" 
/// where the y-coordinate is determined by the time difference between the 
/// two ends.

#include <iostream>
#include <iomanip>
#include <cmath>
using namespace std;

#include <JANA/JEvent.h>
#include "DCTOFPoint_factory.h"
#include "DCTOFHit.h"
#include "HDGEOMETRY/DGeometry.h"
#include "DANA/DEvent.h"

//------------------
// Init
//------------------
void DCTOFPoint_factory::Init()
{
}

//------------------
// BeginRun
//------------------
void DCTOFPoint_factory::BeginRun(const std::shared_ptr<const JEvent> &event)
{
	map<string,string> installed;
	DEvent::GetCalib(event, "/CTOF/install_status", installed);

  ATTENUATION_LENGTH=400.;
  LIGHT_PROPAGATION_SPEED=15.; // cm/ns
  THRESHOLD=0.0005; // GeV

  // Get the geometry
  auto runnumber = event->GetRunNumber();
  auto app = GetApplication();
  auto geoman = app->GetService<DGeometryManager>();
  const DGeometry *geom = geoman->GetDGeometry(runnumber);
  geom->GetCTOFPositions(ctof_positions);

}

//------------------
// Process
//------------------
void DCTOFPoint_factory::Process(const std::shared_ptr<const JEvent> &event)
{
  vector<const DCTOFHit *>ctofhits;
  event->Get(ctofhits);
  
  const double paddle_length=120.; // cm
  for (unsigned int i=0;i<ctofhits.size();i++){
    const DCTOFHit *hit1=ctofhits[i];
    for (unsigned int j=i+1;j<ctofhits.size();j++){
      const DCTOFHit *hit2=ctofhits[j];
      if (hit1->bar==hit2->bar && hit1->end!=hit2->end){
	double dE=sqrt(hit1->dE*hit2->dE)
	  *exp(0.5*paddle_length/ATTENUATION_LENGTH);
	if (dE>THRESHOLD){
	  double t=0.5*(hit1->t+hit2->t-paddle_length/LIGHT_PROPAGATION_SPEED);
	  double y=0.5*LIGHT_PROPAGATION_SPEED*(hit1->t-hit2->t)
	    *((hit1->end==1)?1.:-1.);
	  
	  DCTOFPoint *myDCTOFPoint = new DCTOFPoint;
	  myDCTOFPoint->bar = hit1->bar;
	  myDCTOFPoint->pos = ctof_positions[hit1->bar-1];
	  myDCTOFPoint->pos.SetY(myDCTOFPoint->pos.y()+y);
	  myDCTOFPoint->t = t;
	  myDCTOFPoint->dE= dE;
	  
	  mData.push_back(myDCTOFPoint);
	}
      }
    }
  }
}

//------------------
// EndRun
//------------------
void DCTOFPoint_factory::EndRun()
{
}

//------------------
// Finish
//------------------
void DCTOFPoint_factory::Finish()
{
}

