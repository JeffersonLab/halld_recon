// $Id$
//
//    File: JEventProcessor_BCAL_TDC_Timing.h
// Created: Tue Jul 28 10:55:56 EDT 2015
// Creator: mstaib (on Linux egbert 2.6.32-504.30.3.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_BCAL_TDC_Timing_
#define _JEventProcessor_BCAL_TDC_Timing_

#include <JANA/JEventProcessor.h>
#include <JANA/Compatibility/JLockService.h>
#include <BCAL/DBCALGeometry.h>
#include <TRACKING/DTrackFitter.h>

class JEventProcessor_BCAL_TDC_Timing:public JEventProcessor{
	public:
		JEventProcessor_BCAL_TDC_Timing();
		~JEventProcessor_BCAL_TDC_Timing();

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;

		std::shared_ptr<JLockService> lockService;

		uint32_t VERBOSE;
		uint32_t VERBOSEHISTOGRAMS;

        bool DONT_USE_SC;

      double Z_TARGET;

      //Used as a key for maps
      class readout_channel {
         public:
            readout_channel(int cellId, DBCALGeometry::End end) :
               cellId(cellId), end(end) {}

            int cellId;
            DBCALGeometry::End end;

            bool operator<(const readout_channel &c) const {
               if (cellId<c.cellId) return true;
               if (cellId>c.cellId) return false;
               if (end==DBCALGeometry::kUpstream && c.end==DBCALGeometry::kDownstream) return true;
               return false;
            }
      };

      //For now timewalk corrections are of the form f(ADC) = c0 + c1/(ADC-c3)^c2
      //Store all coefficients in one structure
      class timewalk_coefficients {
         public:
            timewalk_coefficients() :
               a_thresh(0), c0(0), c1(0), c2(0) {}
            timewalk_coefficients(float c0, float c1, float c2, float a_thresh) :
               a_thresh(a_thresh), c0(c0), c1(c1), c2(c2) {}
            float a_thresh,c0,c1,c2;
      };

      map<readout_channel,timewalk_coefficients> tdc_timewalk_map; 

};

#endif // _JEventProcessor_BCAL_TDC_Timing_

