// $Id$
//
/*! **File**: DTOFPaddleHit_factory.h
 *+ Created: Thu Jun  9 10:05:21 EDT 2005
 *+ Creator: davidl (on Darwin wire129.jlab.org 7.8.0 powerpc)
 *+ Purpose: Create TOF paddle hits for long paddles with both ends instrumented with PMTs based
 * on matched DTOFHit objects.
 *+ Addition: command line parmeter -PTOF:TOF_POINT_TAG=TRUTH will initiate 
 *           the use of TOFHitRaw::TRUTH information to calculate the TOFHit positions 
 *           (e.q. use of unsmeared data)
*/

/// \addtogroup TOFDetector


#ifndef _DTOFPaddleHit_factory_
#define _DTOFPaddleHit_factory_

#include "JANA/JFactory.h"
#include "JANA/JApplication.h"
#include "JANA/JParameterManager.h"
#include "JANA/JEventLoop.h"
#include "DTOFPaddleHit.h"
#include "DTOFGeometry.h"
#include "TMath.h"
using namespace jana;

///
/// \file DTOFPaddleHit_factory.h 
/// Basis class definitions for the factory that generates
/// TOFPaddleHit objects are based on long paddles with 2-ended readout. 
/// The individual hits from either side come from DTOFHit objects.


class DTOFPaddleHit_factory:public JFactory<DTOFPaddleHit>{
 public:
  DTOFPaddleHit_factory(){TOF_POINT_TAG="";gPARMS->SetDefaultParameter("TOF:TOF_POINT_TAG", TOF_POINT_TAG,"");};
  ~DTOFPaddleHit_factory(){};
  
  string TOF_POINT_TAG; ///< JFactory tag for TOF Paddle Hit factory: command line -PTOF:TOF_POINT_TAG=XXX
  double C_EFFECTIVE;   ///< effective speed of light in paddle
  double HALFPADDLE;    ///< length of short paddles
  double E_THRESHOLD;   ///< energy threshold applied to ADC hits
  double ATTEN_LENGTH;  ///< attenuation length in paddle
  double TIME_COINCIDENCE_CUT; ///< timing window to find match between both ends of a paddle

  int TOF_NUM_PLANES; ///< number of TOF planes is 2
  int TOF_NUM_BARS;   ///< number of paddles in a plane 44 ( 46 for TOF2)
  int FirstShortBar,LastShortBar;

  vector<double> propagation_speed; ///< effective speed of light in paddle (each paddle separate) from CCDB

  vector < vector <float> > AttenuationLengths; ///< attenuation length in paddle from CCDB


 protected:
  //jerror_t init(void);					///< Called once at program start.
  jerror_t brun(JEventLoop *eventLoop, int32_t runnumber);	        ///< Called everytime a new run number is detected.
  jerror_t evnt(JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
  //jerror_t erun(void);					///< Called everytime run number changes, provided brun has been called.
  //jerror_t fini(void);					///< Called after last event of last event source has been processed.
};

#endif // _DTOFPaddleHit_factory_

