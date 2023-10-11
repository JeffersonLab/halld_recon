// $Id$
//
/*! **File**: DSCHit_factory.h
 *+ Created: Tue Aug  6 12:53:32 EDT 2013
 *+ Creator: davidl (on Darwin harriet.jlab.org 11.4.2 i386)
 *+ Purpose: Start Counter factory to generate Start Counter hits based on matched ADC and TDC data
 * converted to units of GeV for energy and ns for time
*/
/// \addtogroup SCDetector

/*! \file DSCHit_factory.h
 * header file for the start counter hit factory, defining the DSCHit_factory class
 * and the necessary parameters for the code implementation
 */


#ifndef _DSCHit_factory_
#define _DSCHit_factory_

#include <JANA/JFactoryT.h>

#include "DSCHit.h"
#include "DSCDigiHit.h"

#include <vector>
using std::vector;

class DSCHit_factory:public JFactoryT<DSCHit>{
 public:
  DSCHit_factory(){};
  ~DSCHit_factory(){};
  
  // overall scale factors
  double a_scale; ///< ADC time to ns conversion factor
  double t_scale; ///< TDC time to ns converstion factor
  double t_base;  ///< ADC base time offset, is run dependend from CCDB
  double t_tdc_base; ///< TDC base time offset, is run dependedn from CCDB
  
  // calibration constants stored by channel
  vector<double>  a_gains; ///< ADC gain factor
  vector<double>  a_pedestals; ///< adc default pedestal
  vector<double>  adc_time_offsets; ///< adc time offsets from calibration CCDB
  vector<double>  tdc_time_offsets; ///< tdc time offsets from calibration CCDB
  vector<vector<double> >timewalk_parameters; ///< walk correction parameters from CCDB
  
  //map<string,double>  propogation_corr_factors;
  //double<string,double>  attenuation_corr_factors;
  
  double DELTA_T_ADC_TDC_MAX; ///< time window to match ADC and TDC hits
  double HIT_TIME_WINDOW; ///< acceptable time window for shifted ADC and TDC times 
  double ADC_THRESHOLD; ///< ADC threshold cut (not used currently)
  double USE_TIMEWALK_CORRECTION; ///< flag (default 1), command line switch SC:USE_TIMEWALK_CORRECTION
  int MAX_SECTORS; ///< number of SC sectors, loaded from geometry CCDB
  
  DSCHit* FindMatch(vector<DSCHit*>&schits,int sector, double T);
  
  const double GetConstant(const vector<double>  &the_table,
			   const int in_sector) const;
  const double GetConstant(const vector<double>  &the_table,
			   const DSCDigiHit *the_digihit) const;
  const double GetConstant(const vector<double>  &the_table,
			   const DSCHit *the_hit) const;
  //const double GetConstant(const vector<double>  &the_table,
  //			 const DTranslationTable *ttab,
  //			 const int in_rocid, const int in_slot, const int in_channel) const;
  
 private:
  void Init() override;
  void BeginRun(const std::shared_ptr<const JEvent>& event) override;
  void Process(const std::shared_ptr<const JEvent>& event) override;
  void EndRun() override;
  void Finish() override;

  bool CHECK_FADC_ERRORS;
  bool REQUIRE_ADC_TDC_MATCH;
};

#endif // _DSCHit_factory_

