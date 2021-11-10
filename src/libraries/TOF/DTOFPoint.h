// $Id$
//
/*! **File**: DTOFPoint.h
 *+ Created: Tue Oct 18 09:50:52 EST 2005
 *+ Creator: remitche (on Linux mantrid00 2.4.20-18.8smp i686)
 *+ Purpose: Container class object holding TOF Point matched data by combined hits from the two
 *  orthogonal planes.
*/
/// \addtogroup TOFDetector

/*! \file DTOFPoint.h container class definition file for TOF points generated from
 * TOF Paddle hits of two opposing planes (vertical and horizontal).
 
 * two member functions:
 * \fn Is_XPositionWellDefined(void): boolean return
 * \fn Is_YPositionWellDefined(void): boolean return

 */

#ifndef _DTOFPoint_
#define _DTOFPoint_

#include <JANA/JObject.h>
#include <JANA/JFactory.h>
#include <DVector3.h>
using namespace jana;

class DTOFPoint : public JObject
{
 public:
  JOBJECT_PUBLIC(DTOFPoint);
  
  // DTOFPoint objects are created for DTOFPaddleHit matches between planes, AND for un-matched DTOFPaddleHits.
  
  // Note the hit position along a PADDLE is not well-defined if:
  // It is single-ended, or if one of the ends has a hit energy that is below threshold
  
  // Sometimes, these partially-defined hits are not matched to hits in the other plane. 
  // In these cases, a DTOFPoint object is created from each hit, with the following properties:
  //The hit position is reported at the center of the paddle
  //The energy & hit are reported at this position
  // The properties of these hits are fully-defined through matching to tracks (in the other direction)
  // The corrected hit energy & time (propagated to the track location) are stored in DDetectorMatches
  
  DVector3 pos;   	///< reconstructed position (x,y,z) in cm
  float t;               ///< reconstructed time in ns
  float dE;            ///< reconstructed deposited energy in GeV
  float tErr; ///< uncertainty on reconstructed time
  float dE1;    ///< energy deposition dE first plane in GeV
  float dE2;    ///< energy deposition dE second plane in GeV


  int dHorizontalBar; ///< 0 for none (unmatched)
  int dVerticalBar; ///< 0 for none (unmatched)
  
  //Status: 0 if no hit (or none above threshold), 1 if only North hit above threshold, 2 if only South hit above threshold, 3 if both hits above threshold
  //For horizontal, North/South is +x/-x
  int dHorizontalBarStatus;  ///< 0, 1, 2, 3: no hit, north only, south only, both ends 
  int dVerticalBarStatus; ///< 0, 1, 2, 3: no hit, north only, south only, both ends 
  
  bool Is_XPositionWellDefined(void) const
  {
    return ((dVerticalBar != 0) || (dHorizontalBarStatus == 3));
  }
  bool Is_YPositionWellDefined(void) const
  {
    return ((dHorizontalBar != 0) || (dVerticalBarStatus == 3));
  }
  
  void toStrings(vector<pair<string,string> > &items) const
  {
    AddString(items, "H-Bar", "%d", dHorizontalBar);
    AddString(items, "V-Bar", "%d", dVerticalBar);
    AddString(items, "x", "%1.3f", pos.x());
    AddString(items, "y", "%1.3f", pos.y());
    AddString(items, "z", "%1.3f", pos.z());
    AddString(items, "t", "%1.3f", t);
    AddString(items, "dE", "%1.6f", dE);    //dE average
    AddString(items, "dE1", "%1.6f", dE1);  //dE form first plane
    AddString(items, "dE2", "%1.6f", dE2);  //dE from second plane
    AddString(items, "H-Status", "%d", dHorizontalBarStatus);
    AddString(items, "V-Status", "%d", dVerticalBarStatus);
  }
};

#endif // _DTOFPoint_

