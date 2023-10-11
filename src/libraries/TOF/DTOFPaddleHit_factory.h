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

#include <JANA/JFactoryT.h>
#include "DTOFPaddleHit.h"
#include "TMath.h"


///
/// \file DTOFPaddleHit_factory.h 
/// Basis class definitions for the factory that generates
/// TOFPaddleHit objects are based on long paddles with 2-ended readout. 
/// The individual hits from either side come from DTOFHit objects.


class DTOFPaddleHit_factory:public JFactoryT<DTOFPaddleHit>{
 public:
  DTOFPaddleHit_factory(){};
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
  // void Init() override;
  void BeginRun(const std::shared_ptr<const JEvent>& event) override;
  void Process(const std::shared_ptr<const JEvent>& event) override;
  //void EndRun() override;
  //void Finish() override;
};

#endif // _DTOFPaddleHit_factory_

