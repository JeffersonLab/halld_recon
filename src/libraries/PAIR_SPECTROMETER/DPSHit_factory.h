// $Id$
//
//    File: DPSHit_factory.h
// Created: Wed Oct 15 16:45:01 EDT 2014
// Creator: staylor (on Linux gluon05.jlab.org 2.6.32-358.18.1.el6.x86_64 x86_64)
//


#ifndef _DPSHit_factory_
#define _DPSHit_factory_

#include <JANA/JFactoryT.h>
#include <JANA/Calibrations/JCalibration.h>

#include "DPSHit.h"
#include "DPSDigiHit.h"
#include "DPSGeometry.h"

#include <utility>

typedef vector< vector<double> > ps_digi_constants_t;

class DPSHit_factory:public JFactoryT<DPSHit>{
 public:
  DPSHit_factory(){};
  ~DPSHit_factory(){};

  // config. parameter
  double ADC_THRESHOLD;

  // overall scale factors
  double a_scale;
  double t_scale;
  double t_base;

  // calibration constants stored by channel
  ps_digi_constants_t  adc_gains;
  ps_digi_constants_t  adc_pedestals;
  ps_digi_constants_t  adc_time_offsets;

  const double GetConstant( const ps_digi_constants_t &the_table,
			    const DPSGeometry::Arm in_arm, const int in_column,
			    const DPSGeometry &psGeom ) const; 
  const double GetConstant( const ps_digi_constants_t &the_table,
			    const DPSDigiHit *the_digihit, const DPSGeometry &psGeom ) const;
  const double GetConstant( const ps_digi_constants_t &the_table,
			    const DPSHit *the_hit, const DPSGeometry &psGeom ) const;

 private:
  void Init() override;
  void BeginRun(const std::shared_ptr<const JEvent>& event) override;
  void Process(const std::shared_ptr<const JEvent>& event) override;
  void EndRun() override;
  void Finish() override;

  void FillCalibTable(JCalibration* calib, ps_digi_constants_t &table, string table_name,
		      const DPSGeometry &psGeom);

  bool CHECK_FADC_ERRORS;
};

#endif // _DPSHit_factory_
