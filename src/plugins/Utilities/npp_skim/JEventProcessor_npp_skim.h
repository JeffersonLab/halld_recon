// $Id$
//
//    File: JEventProcessor_npp_skim.h
// Created: Tue Apr 16 10:14:04 EDT 2024
// Creator: gxproj2 (on Linux ifarm1802.jlab.org 3.10.0-1160.102.1.el7.x86_64 x86_64)
//

#ifndef _JEventProcessor_npp_skim_
#define _JEventProcessor_npp_skim_

#include <JANA/JEventProcessor.h>
#include "evio_writer/DEventWriterEVIO.h"

#include <FCAL/DFCALShower.h>
#include <BCAL/DBCALShower.h>

#include <thread>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <iomanip>

class JEventProcessor_npp_skim:public jana::JEventProcessor{
 public:
  JEventProcessor_npp_skim();
  ~JEventProcessor_npp_skim();
  const char* className(void){return "JEventProcessor_npp_skim";}
  int num_epics_events;
  
 private:
  jerror_t init(void);						///< Called once at program start.
  jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
  jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
  jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
  jerror_t fini(void);						///< Called after last event of last event source has been processed.
  const double z_CPP_Target = 1.;
  
  double mgg(double x1,double y1, double z1, double e1,double x2,double y2, double z2, double e2) {
    double r1 = sqrt(x1*x1+y1*y1+z1*z1);
    double r2 = sqrt(x2*x2+y2*y2+z2*z2);
    double px1= x1/r1*e1;
    double py1= y1/r1*e1;
    double pz1= z1/r1*e1;
    double px2= x2/r2*e2;
    double py2= y2/r2*e2;
    double pz2= z2/r2*e2;
    
    double m2 = e1*e2-px1*px2-py1*py2-pz1*pz2;
    m2        = sqrt(2.*m2);
    
    return m2;
  }
  
};

#endif // _JEventProcessor_npp_skim_

