// $Id$
//
//    File: DPSCHit_factory.h
// Created: Wed Oct 15 16:45:33 EDT 2014
// Creator: staylor (on Linux gluon05.jlab.org 2.6.32-358.18.1.el6.x86_64 x86_64)
//

#ifndef _DPSCHit_factory_
#define _DPSCHit_factory_

#include <vector>
#include <utility>

#include <JANA/JFactoryT.h>
#include "TTAB/DTTabUtilities.h"

#include "DPSCHit.h"
#include "DPSCDigiHit.h"
#include "DPSCTDCDigiHit.h"
#include "DPSGeometry.h"

typedef vector< pair<double,double> > psc_digi_constants_t;

class DPSCHit_factory:public JFactoryT<DPSCHit>{
 public:
  DPSCHit_factory(){};
  ~DPSCHit_factory(){};

  // config. parameters
  double DELTA_T_ADC_TDC_MAX;  
  double ADC_THRESHOLD;

  // overall scale factors
  double a_scale;
  double t_scale;
  double t_base;
  double t_tdc_base;

  // timewalk parameters per module
  vector<vector<double> > tw_parameters;
  double c0;
  double c1;
  double c2;
  double thresh;
  double P_0;

  // calibration constants stored by channel
  psc_digi_constants_t  adc_gains;
  psc_digi_constants_t  adc_pedestals;
  psc_digi_constants_t  adc_time_offsets;
  psc_digi_constants_t  tdc_time_offsets;

  const DPSGeometry::Arm GetArm(const int counter_id,const int num_counters_per_arm) const;
  const int GetModule(const int counter_id,const int num_counters_per_arm) const;
  DPSCHit* FindMatch(DPSGeometry::Arm arm, int module, double T);

  const double GetConstant( const psc_digi_constants_t &the_table,
			    const DPSGeometry::Arm in_arm, const int in_module,
			    const DPSGeometry &psGeom ) const; 
  const double GetConstant( const psc_digi_constants_t &the_table,
			    const DPSCDigiHit *the_digihit, const DPSGeometry &psGeom ) const;
  const double GetConstant( const psc_digi_constants_t &the_table,
			    const DPSCTDCDigiHit *the_hit, const DPSGeometry &psGeom ) const;
  const double GetConstant( const psc_digi_constants_t &the_table,
			    const DPSCHit *the_hit, const DPSGeometry &psGeom ) const;

 private:
  void Init() override;
  void BeginRun(const std::shared_ptr<const JEvent>& event) override;
  void Process(const std::shared_ptr<const JEvent>& event) override;
  void EndRun() override;
  void Finish() override;

  void FillCalibTable(psc_digi_constants_t &table, vector<double> &raw_table,
		      const DPSGeometry &tofGeom);

  bool CHECK_FADC_ERRORS;
};

#endif // _DPSCHit_factory_

