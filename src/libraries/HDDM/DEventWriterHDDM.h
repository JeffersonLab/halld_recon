#ifndef _DEventWriterHDDM_
#define _DEventWriterHDDM_

#include <fstream>
#include <iostream>
#include <iomanip>
#include <vector>
#include <HDDM/hddm_s.hpp>

#include <JANA/JEvent.h>
#include <JANA/JObject.h>
#include <JANA/Compatibility/JLockService.h>

#include <CDC/DCDCHit.h>
#include <TOF/DTOFHit.h>
#include <FCAL/DFCALHit.h>
#include <ECAL/DECALHit.h>
#include <CCAL/DCCALHit.h>
#include <BCAL/DBCALDigiHit.h>
#include <BCAL/DBCALTDCDigiHit.h>
#include <START_COUNTER/DSCHit.h>
#include <FDC/DFDCHit.h>
#include <PAIR_SPECTROMETER/DPSHit.h>
#include <PAIR_SPECTROMETER/DPSCHit.h>
#include <TAGGER/DTAGHHit.h>
#include <TAGGER/DTAGMHit.h>
#include <TPOL/DTPOLHit.h>
#include <RF/DRFTime.h>
#include <DIRC/DDIRCPmtHit.h>
#include <FMWPC/DFMWPCHit.h>
#include <FMWPC/DCTOFHit.h>

using namespace std;

class DEventWriterHDDM : public JObject
{
	public:
		JOBJECT_PUBLIC(DEventWriterHDDM);

		DEventWriterHDDM(const std::shared_ptr<const JEvent>& locEventLoop, string locOutputFileBaseName);
		~DEventWriterHDDM(void);

		bool Write_HDDMEvent(const std::shared_ptr<const JEvent>& locEventLoop, string locOutputFileNameSubString) const;
		string Get_OutputFileName(string locOutputFileNameSubString) const;

	private:
		bool Write_HDDMEvent(string locOutputFileName, hddm_s::HDDM& locRecord) const;

		//contains static variables shared amongst threads
		int& Get_NumEventWriterThreads(void) const; //acquire HDDMWriter lock before modifying
		map<string, pair<ofstream*, hddm_s::ostream*> >& Get_HDDMOutputFilePointers(void) const;

		int32_t Convert_UnsignedIntToSigned(uint32_t locUnsignedInt) const;

		string dOutputFileBaseName;
		bool HDDM_USE_COMPRESSION;
		bool HDDM_USE_INTEGRITY_CHECKS;

        // metadata to save in the HDDM file
        // these should be consistent during program execution
        string HDDM_DATA_VERSION_STRING;
        string CCDB_CONTEXT_STRING;

        // Information for optional tags
        string CDC_TAG;
        string FDC_TAG;
        string TAGM_TAG;
        string TAGH_TAG;

        std::shared_ptr<JLockService> lockService;
};

#endif //_DEventWriterHDDM_
