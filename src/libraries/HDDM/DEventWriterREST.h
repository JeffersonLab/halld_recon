#ifndef _DEventWriterREST_
#define _DEventWriterREST_

#include <math.h>
#include <vector>
#include <string>

#include <HDDM/hddm_r.hpp>

#include <JANA/JObject.h>
#include <JANA/JEvent.h>
#include <JANA/Compatibility/JLockService.h>

#include <DVector3.h>
#include <DMatrix.h>

#include "PID/DMCReaction.h"
#include "PID/DBeamPhoton.h"
#include "TRACKING/DMCThrown.h"
#include "FCAL/DFCALShower.h"
#include "FCAL/DFCALHit.h"
#include "CCAL/DCCALShower.h"
#include "PID/DNeutralShower.h"
#include <PID/DDetectorMatches.h>
#include "BCAL/DBCALShower.h"
#include "TOF/DTOFPoint.h"
#include "START_COUNTER/DSCHit.h"
#include "TRACKING/DTrackTimeBased.h"
#include "TRIGGER/DTrigger.h"
#include "DIRC/DDIRCPmtHit.h"
#include "FMWPC/DFMWPCHit.h"
#include "HDDM/DEventHitStatistics.h"
#include "RF/DRFTime.h"
#include "DAQ/DBeamHelicity.h"

using namespace std;

class DEventWriterREST : public JObject
{
	public:
		JOBJECT_PUBLIC(DEventWriterREST);

		DEventWriterREST(const std::shared_ptr<const JEvent>& locEventLoop, string locOutputFileBaseName);
		~DEventWriterREST(void);

		bool Write_RESTEvent(const std::shared_ptr<const JEvent>& locEventLoop, string locOutputFileNameSubString) const;
		string Get_OutputFileName(string locOutputFileNameSubString) const;

	private:
		bool Write_RESTEvent(string locOutputFileName, hddm_r::HDDM& locRecord) const;

		//contains static variables shared amongst threads
		int& Get_NumEventWriterThreads(void) const; //acquire RESTWriter lock before modifying
		map<string, pair<ofstream*, hddm_r::ostream*> >& Get_RESTOutputFilePointers(void) const;

		int32_t Convert_UnsignedIntToSigned(uint32_t locUnsignedInt) const;

		string dOutputFileBaseName;
		bool HDDM_USE_COMPRESSION;
		bool HDDM_USE_INTEGRITY_CHECKS;
		bool REST_WRITE_DIRC_HITS;
		bool REST_WRITE_FMWPC_HITS;
		bool REST_WRITE_CCAL_SHOWERS;
		bool REST_WRITE_TRACK_EXIT_PARAMS;
		bool REST_WRITE_FDC_TRACK_POS;
		bool ADD_FCAL_DATA_FOR_CPP;
		bool REST_WRITE_FCAL_HITS;

        // metadata to save in the REST file
        // these should be consistent during program execution
        string HDDM_DATA_VERSION_STRING;
        string CCDB_CONTEXT_STRING;

        std::shared_ptr<JLockService> lockService;
};

#endif //_DEventWriterREST_
