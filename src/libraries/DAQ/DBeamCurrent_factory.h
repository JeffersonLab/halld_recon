// $Id$
//
//    File: DBeamCurrent_factory.h
// Created: Tue Feb 21 04:25:04 EST 2017
// Creator: davidl (on Linux gluon48.jlab.org 2.6.32-431.20.3.el6.x86_64 x86_64)
//

#ifndef _DBeamCurrent_factory_
#define _DBeamCurrent_factory_

#include <JANA/JFactoryT.h>
#include "DBeamCurrent.h"
using std::vector;

class DBeamCurrent_factory:public JFactoryT<DBeamCurrent>{
	public:
		DBeamCurrent_factory(){};
		~DBeamCurrent_factory(){};

		typedef struct{
			double t;
			double Ibeam;
			double t_trip_prev; // time relative to this boundary of previous beam trip
			double t_trip_next; // time relative to this boundary of next beam trip
		}Boundary;

		double BEAM_ON_MIN_PSCOUNTS;
		bool USE_EPICS_FOR_BEAM_ON;
		double BEAM_ON_MIN_nA;
		double BEAM_TRIP_MIN_T;
		uint32_t SYNCSKIM_ROCID;

		vector<Boundary> boundaries; // key=time  val=Ibeam
		vector<double> trip;
		vector<double> recover;
		
		double   ticks_per_sec;
		uint64_t rcdb_250MHz_offset_tics;
		uint32_t rcdb_start_time;

		
		// This returns the integrated fiducial time between the
		// given times. The values t_start and t_end should be
		// in seconds since the start of the run. If t_end is
		// set to 0.0 then the integration is done for the entire
		// run. WARNING: that will span all files in the run!
		double IntegratedFiducialTime(double t_start=0.0, double t_end=0.0);
		
		// Return the total integrated time of the run.
		double IntegratedTime(void);
		
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& aEvent) override;
		void Process(const std::shared_ptr<const JEvent>& aEvent) override;
		void EndRun() override;
		void Finish() override;
};

#endif // _DBeamCurrent_factory_

