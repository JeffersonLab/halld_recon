#ifndef _DLorentzMapCalibDB_
#define _DLorentzMapCalibDB_

#include "DLorentzDeflections.h"

#include <JANA/JApplication.h>
#include <JANA/Calibrations/JCalibration.h>

class DLorentzMapCalibDB:public DLorentzDeflections{
 public:
  DLorentzMapCalibDB(JApplication *japp, unsigned int run_number=1);
  DLorentzMapCalibDB(JCalibration *jcalib);
  ~DLorentzMapCalibDB(){};
  
  unsigned int GetLorentzDeflections(void);

 protected:
  JCalibration *jcalib;
  
};

#endif // _DLorentzMapCalibDB_
