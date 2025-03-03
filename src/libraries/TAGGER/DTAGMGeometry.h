//
// File: DTAGMGeometry.h
// Created: Sat Jul 5, 10:09:27 EST 2014
// Creator: jonesrt on gluey.phys.uconn.edu
//


#ifndef _DTAGMGeometry_
#define _DTAGMGeometry_

#include <string>

#include <JANA/JObject.h>
#include <JANA/JApplication.h>
#include <JANA/JEvent.h>
#include <JANA/Calibrations/JCalibration.h>
#include <JANA/Calibrations/JCalibrationCCDB.h>
#include <JANA/Calibrations/JCalibrationGeneratorCCDB.h>

#include <DANA/DEvent.h>

#include "units.h"

#define TAGM_MAX_ROW     5
#define TAGM_MAX_COLUMN  102


class DTAGMGeometry : public JObject {
 public:
   
   JOBJECT_PUBLIC(DTAGMGeometry);

   DTAGMGeometry(const std::shared_ptr<const JEvent>& event);
   DTAGMGeometry(JCalibration *jcalib, int32_t runnumber);
   ~DTAGMGeometry();

   void Initialize(JCalibration *jcalib, bool print_messages);

   static const unsigned int kRowCount;
   static const unsigned int kColumnCount;
   static const double kFiberWidth;  // cm
   static const double kFiberLength; // cm

   // columns are numbered 1..kColumnCount
   double getElow(unsigned int column) const;
   double getEhigh(unsigned int column) const;
   bool E_to_column(double E, unsigned int &column) const;

   void Summarize(JObjectSummary& summary) const override {
      summary.add(kFiberWidth, "kFiberWidth", "%f cm");
      summary.add(kFiberLength, "kFiberLength", "%f cm");
      summary.add(kRowCount, "kRowCount", "%d");
      summary.add(kColumnCount, "kColumnCount", "%d");
   }
   
 private:
   double m_endpoint_energy_GeV;
   double m_endpoint_energy_calib_GeV;
   double m_column_xlow[TAGM_MAX_COLUMN+1];
   double m_column_xhigh[TAGM_MAX_COLUMN+1];
};

#endif // _DTAGMGeometry_
