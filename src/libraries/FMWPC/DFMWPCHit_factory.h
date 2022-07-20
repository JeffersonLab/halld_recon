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

class DFMWPCHit_factory:public jana::JFactory<DFMWPCHit>{
 public:
  DFMWPCHit_factory(){};
  ~DFMWPCHit_factory(){};
//  const char* Tag(void){return "";}

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
  jerror_t init(void);						///< Called once at program start.
  jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
  jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
  jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
  jerror_t fini(void);						///< Called after last event of last event source has been processed.
  
  //void FillCalibTable(vector< vector<double> > &table, vector<double> &raw_table);
  
  // Geometry information
  unsigned int maxChannels;
  unsigned int Nlayers; // number of layers
  vector<unsigned int> Nwires; // number of wires for each layer
  
};

#endif // _DFMWPCHit_factory_

