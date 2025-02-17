// $Id$
//
//    File: DFMWPCHit_factory.h
//

#ifndef _DFMWPCHit_factory_
#define _DFMWPCHit_factory_

#include <vector>
using namespace std;

#include <JANA/JFactory.h>
#include <HDGEOMETRY/DGeometry.h>
#include <TTAB/DTranslationTable.h>
#include <DAQ/Df125PulseIntegral.h>
#include <DAQ/Df125Config.h>
#include <DAQ/Df125CDCPulse.h>

#include "DFMWPCHit.h"
#include "DFMWPCDigiHit.h"

// store constants indexed by layer/wire number
typedef  vector< vector<double> >  fmwpc_digi_constants_t;

class DFMWPCHit_factory:public JFactoryT<DFMWPCHit>{
 public:
  DFMWPCHit_factory(){};
  ~DFMWPCHit_factory(){};
//  const char* Tag(void){return "";}

  // hit threshold
  double hit_amp_threshold;
  double hit_int_threshold;

  // timing cut limits
  double t_raw_min;
  double t_raw_max;

  // overall scale factors.
  double a_scale, amp_a_scale;
  double t_scale;
  double t_base;
  
  // calibration constant tables
  fmwpc_digi_constants_t gains;
  fmwpc_digi_constants_t pedestals;
  fmwpc_digi_constants_t time_offsets;
  
  const double GetConstant(const fmwpc_digi_constants_t &the_table,
			   const int in_layer, const int in_wire) const;
  const double GetConstant(const fmwpc_digi_constants_t &the_table,
			   const DFMWPCDigiHit *the_digihit) const;
  const double GetConstant(const fmwpc_digi_constants_t &the_table,
			   const DFMWPCHit *the_hit) const;
  
 private:
  void Init() override;
  void BeginRun(const std::shared_ptr<const JEvent>& event) override;
  void Process(const std::shared_ptr<const JEvent>& event) override;
  void EndRun() override;
  void Finish() override;

  //void FillCalibTable(vector< vector<double> > &table, vector<double> &raw_table);
  
  // Geometry information
  unsigned int maxChannels;
  unsigned int Nlayers; // number of layers
  vector<unsigned int> Nwires; // number of wires for each layer
  
};

#endif // _DFMWPCHit_factory_

