//
// File: DTAGHGeometry.h
// Created: Sat Jul 5, 10:09:27 EST 2014
// Creator: jonesrt on gluey.phys.uconn.edu
//

#ifndef _DTAGHGeometry_
#define _DTAGHGeometry_

#include <string>

#include <JANA/JFactory.h>
#include <JANA/JObject.h>
#include <JANA/JApplication.h>
#include <JANA/JEvent.h>
#include <JANA/JCalibration.h>
#include <JANA/JCalibrationCCDB.h>
#include <JANA/JCalibrationGeneratorCCDB.h>
using namespace jana;

#include <DANA/DApplication.h>

#include "units.h"

#define TAGH_MAX_COUNTER 274

class DTAGHGeometry : public JObject {
 public:
   
   JOBJECT_PUBLIC(DTAGHGeometry);

   DTAGHGeometry(JEventLoop *loop);
   DTAGHGeometry(JCalibration *jcalib, int32_t runnumber);
   ~DTAGHGeometry();

   void Initialize(JCalibration *jcalib, bool print_messages);

   static const unsigned int kCounterCount;

   // counters are numbered 1..kCounterCount
   double getElow(unsigned int counter) const;
   double getEhigh(unsigned int counter) const;
   bool E_to_counter(double E, unsigned int &counter) const;

   void toStrings(vector<pair<string,string> > &items) const {
      AddString(items, "kCounterCount", "%d", kCounterCount);
   }
   
 private:
   double m_endpoint_energy_GeV;
   double m_endpoint_energy_calib_GeV;
   double m_counter_xlow[TAGH_MAX_COUNTER+1];
   double m_counter_xhigh[TAGH_MAX_COUNTER+1];
};

#endif // _DTAGHGeometry_
