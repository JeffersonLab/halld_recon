// $Id$
//
/*! **File**: DTOFPaddleHit.h
 *+ Created: Thu Jun  9 10:05:21 EDT 2005
 *+ Creator: davidl (on Darwin wire129.jlab.org 7.8.0 powerpc)
 *+ Purpose: Container class object holding TOF Paddle matched hit data

*/
/// \addtogroup TOFDetector




/*! \file DTOFPaddleHit.h definition of container class that holds the
 * data for paddle hits. In case of hits where data from both ends are available
 * mean-time and time-differences are calculated as well as hit position and 
 * energy deposition corrected for attenuation.

 */

#ifndef _DTOFPaddleHit_
#define _DTOFPaddleHit_

#include <JANA/JObject.h>


/*! \class DTOFPaddleHit 
 * This is a container class for hits in paddles that have PMTS on both ends
 * It holds matched hit information from both ends of the paddle if a match exists (was found).
 */
class DTOFPaddleHit:public JObject{
 public:
  JOBJECT_PUBLIC(DTOFPaddleHit);
  
  int orientation;  ///<  0: vertical,  1: horizontal
  int bar;          ///<  bar number
  float t_north;    ///<  time of light at end of bar  (calibrated) 
  float E_north;    ///<  attenuated energy deposition  (calibrated)
  float t_south;    ///<  time of light at end of bar  (calibrated) 
  float E_south;    ///<  attenuated energy deposition  (calibrated)
  
  float meantime;   ///<  equivalent to time of flight
  float timediff;   ///<  north - south time difference
  float pos;        ///<  hit position in paddle in cm
  float dpos;       ///<  estimated uncertainty in hitposition
  float dE;         ///<  energy deposition calibrated and corrected for attenuation
  
  /*! \fn void toStrings(vector<pair<string,string> > &items)const
   * This print method is used by hd_dump to list of objects of this type for each event. 
   */
  void Summarize(JObjectSummary& summary)const{
    summary.add(orientation, "orientation", "%d");
    summary.add(bar, "bar", "%d");
    summary.add(t_north, "t_north", "%1.3f");
    summary.add(E_north, "E_north", "%1.3f");
    summary.add(t_south, "t_south", "%1.3f");
    summary.add(E_south, "E_south", "%1.3f");
    summary.add(meantime, "meantime", "%1.3f");
    summary.add(timediff, "timediff", "%1.3f");
    summary.add(pos, "pos", "%1.3f");
    summary.add(dpos, "dpos", "%1.3f");
    summary.add(dE, "dE", "%1.3f");
  }
};

#endif // _DTOFPaddleHit_

