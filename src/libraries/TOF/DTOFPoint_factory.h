// $Id$
//
/*! **File**: DTOFPoint_factory.h
 *+ Created: Tue Oct 18 09:50:52 EST 2005
 *+ Creator: remitche (on Linux mantrid00 2.4.20-18.8smp i686)
 *+ Purpose: Create TOF points from TOF paddle hit ojects (DTOFPaddleHit) by 
 * combining such hits from both the two orthogonal planes to form space points.
*/

/// \addtogroup TOFDetector

/*! \file DTOFPoint_factory.h
 * Basic class definition to create TOF Points (space points) based on Paddle hits
 */



#ifndef _DTOFPoint_factory_
#define _DTOFPoint_factory_

#include "JANA/JFactory.h"
#include "DTOFGeometry_factory.h"
#include "DTOFPoint.h"
#include "DTOFPaddleHit.h"
#include "DTOFHit.h"
#include <deque>

/// \htmlonly
/// <A href="index.html#legend">
///	<IMG src="CORE.png" width="100">
///	</A>
/// \endhtmlonly

/// 2-plane (4-fold) TOF coincidences. The 2-hit coincidences come from DTOFPaddleHit objects
/// which are combined into coincidnces between the two planes to form 4-D space points
/// which are represented by DTOFPoint objects.

using namespace std;

class DTOFPoint_factory : public JFactory<DTOFPoint>
{
 public:
  
  double HALFPADDLE;
  double HALFPADDLE_ONESIDED;
  double E_THRESHOLD;
  double ATTEN_LENGTH;
  double ONESIDED_PADDLE_MIDPOINT_MAG; //+/- this number for North/South
  
  int NUM_BARS;

  vector < vector <float> > AttenuationLengths;
  
  const DTOFGeometry* dTOFGeometry;
  
  class tof_spacetimehit_t
  {
  public:
    double x;
    double y;
    double t;
    double pos_cut; //x_cut for horizontal bars, y_cut for vertical bars
    double t_cut;
    const DTOFPaddleHit* TOFHit;
    bool dIsDoubleEndedBar;
    bool dPositionWellDefinedFlag;
    bool dIsSingleEndedNorthPaddle; //if !this and !dIsDoubleEndedBar, is single-ended south paddle
  };
  
  class tof_spacetimehitmatch_t
  {
  public:
    double delta_r;
    double delta_t;
    tof_spacetimehit_t* dTOFSpacetimeHit_Horizontal;
    tof_spacetimehit_t* dTOFSpacetimeHit_Vertical;
    bool dBothPositionsWellDefinedFlag;
  };
  
  tof_spacetimehit_t* Build_TOFSpacetimeHit_Horizontal(const DTOFPaddleHit* locTOFHit);
  tof_spacetimehit_t* Build_TOFSpacetimeHit_Vertical(const DTOFPaddleHit* locTOFHit);
  
 private:
  jerror_t brun(JEventLoop *loop, int32_t runnumber);
  jerror_t evnt(JEventLoop *loop, uint64_t eventnumber);
  jerror_t fini(void);
  
  tof_spacetimehit_t* Get_TOFSpacetimeHitResource(void);
  
  bool Match_Hits(tof_spacetimehit_t* locTOFSpacetimeHit_Horizontal, tof_spacetimehit_t* locTOFSpacetimeHit_Vertical, tof_spacetimehitmatch_t& locTOFSpacetimeHitMatch);
  
  void Create_MatchedTOFPoint(const tof_spacetimehit_t* locTOFSpacetimeHit_Horizontal, const tof_spacetimehit_t* locTOFSpacetimeHit_Vertical);
  void Create_UnMatchedTOFPoint(const tof_spacetimehit_t* locTOFSpacetimeHit);
  
  float dPositionMatchCut_DoubleEnded;
  float dTimeMatchCut_PositionWellDefined;
  float dTimeMatchCut_PositionNotWellDefined;
  
  size_t MAX_TOFSpacetimeHitPoolSize;
  deque<tof_spacetimehit_t*> dTOFSpacetimeHitPool_All;
  deque<tof_spacetimehit_t*> dTOFSpacetimeHitPool_Available;
  
  // calibration tables
  vector<double> propagation_speed;
  vector<double> paddle_resolutions;
  
};

#endif // _DTOFPoint_factory_

