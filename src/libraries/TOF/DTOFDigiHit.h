// $Id$
//

/*! File: DTOFDigiHit.h
 *+ Created: Wed Aug  7 09:30:38 EDT 2013
 *+ Creator: davidl (on Darwin harriet.jlab.org 11.4.2 i386)
 *+ Purpose: Container class object holding raw fADC250 data: integral, pulse, pedestal, time
 */

/// \defgroup TOFDetector TOF Detector Objects

#ifndef _DTOFDigiHit_
#define _DTOFDigiHit_

#include <JANA/JObject.h>
#include <JANA/JFactory.h>

// the follwing is for doxygen describing the class
/*! \class DTOFDigiHit 
 * This is a container class holding raw data from TOF fADC250s
 * The data is generated by the algorithm running on the FPGAs of the ADC modules
 * and converted to uint32_t values for this container class by the data reader/translator code.
 */
class DTOFDigiHit:public jana::JObject{
 public:
  JOBJECT_PUBLIC(DTOFDigiHit);
  
  int plane;   ///<  (0: vertical, 1: horizontal)
  int bar;     ///< bar number: 1, 2, .... 44 (46 for TOF2)
  int end;     ///< North/South 0/1 (same as left/right) looking in beam direction, TOP/BOTTIOM 0/1
  uint32_t pulse_integral; ///< identified pulse integral as returned by FPGA algorithm
  uint32_t pulse_time;     ///< identified pulse time as returned by FPGA algorithm
  uint32_t pedestal;       ///< pedestal info used by FPGA (if any)
  uint32_t QF;             ///<  Quality Factor from FPGA algorithms
  uint32_t nsamples_integral;    ///<  number of samples used in integral 
  uint32_t nsamples_pedestal;    ///< number of samples used in pedestal
  uint32_t pulse_peak;           ///<  maximum ADC value in pulse peak (pedestal is NOT subtracted)
  
  uint32_t datasource;           ///<  0=window raw data, 1=old(pre-Fall16) firmware, 2=Df250PulseData
  
  // This method is used primarily for pretty printing
  // the second argument to AddString is printf style format
  /*! \fn void toStrings(vecotr<pair<string,string>> &items) 
   * standard method by hd_dump to print the container class data.
  */
  void toStrings(vector<pair<string,string> > &items)const{ 
    AddString(items, "bar", "%d", bar);
    AddString(items, "plane", "%d", plane);
    AddString(items, "end", "%d", end);
    AddString(items, "pulse_integral", "%d", pulse_integral);
    AddString(items, "pulse_peak", "%d", pulse_peak);
    AddString(items, "pulse_time", "%d", pulse_time);
    AddString(items, "pedestal", "%d", pedestal);
    AddString(items, "QF", "%d", QF);
    AddString(items, "nsamples_integral", "%d", nsamples_integral);
    AddString(items, "nsamples_pedestal", "%d", nsamples_pedestal);
  }
  
};

#endif // _DTOFDigiHit_

