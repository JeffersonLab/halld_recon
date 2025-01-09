// $Id$
//
//    File: DTrackFitterKalmanSIMD_ALT1.h
// Created: Tue Mar 29 09:45:14 EDT 2011
// Creator: staylor (on Linux ifarml1 2.6.18-128.el5 x86_64)
//

#ifndef _DTrackFitterKalmanSIMD_ALT1_
#define _DTrackFitterKalmanSIMD_ALT1_

#include <JANA/Compatibility/jerror.h>
#include <TRACKING/DTrackFitterKalmanSIMD.h>

class DTrackFitterKalmanSIMD_ALT1: public DTrackFitterKalmanSIMD{
 public:
  DTrackFitterKalmanSIMD_ALT1(const std::shared_ptr<const JEvent>& event):DTrackFitterKalmanSIMD(event){};
    //DTrackFitterKalmanSIMD_ALT1();
    virtual ~DTrackFitterKalmanSIMD_ALT1(){};
    
    kalman_error_t KalmanForward(double fdc_anneal,double cdc_anneal,DMatrix5x1 &S,DMatrix5x5 &C,
			 double &chisq,unsigned int &numdof);
    jerror_t SmoothForward(void); 
  
  // Virtual methods from TrackFitter base class
  string Name(void) const {return string("KalmanSIMD_ALT1");}
 protected:
	
  
 private:
  
};

#endif // _DTrackFitterKalmanSIMD_ALT1_

