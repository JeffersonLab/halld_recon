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

#include "DCTOFPoint_factory.h"
#include "DCTOFHit.h"
using namespace jana;

//------------------
// init
//------------------
jerror_t DCTOFPoint_factory::init(void)
{
  return NOERROR;
}

//------------------
// brun
//------------------
jerror_t DCTOFPoint_factory::brun(jana::JEventLoop *eventLoop, int32_t runnumber)
{
  ATTENUATION_LENGTH=400.;
  LIGHT_PROPAGATION_SPEED=15.; // cm/ns
  THRESHOLD=0.0005; // GeV

  return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t DCTOFPoint_factory::evnt(JEventLoop *loop, uint64_t eventnumber)
{
  vector<const DCTOFHit *>ctofhits;
  loop->Get(ctofhits);
  
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
	  myDCTOFPoint->y = y;
	  myDCTOFPoint->t = t;
	  myDCTOFPoint->dE= dE;
	  
	  _data.push_back(myDCTOFPoint);
	}
      }
    }
  }

  return NOERROR;
}

//------------------
// erun
//------------------
jerror_t DCTOFPoint_factory::erun(void)
{
  return NOERROR;
}

//------------------
// fini
//------------------
jerror_t DCTOFPoint_factory::fini(void)
{
  return NOERROR;
}

