#ifndef DFACTORY_DHDDMFDCHIT_H
#define DFACTORY_DHDDMFDCHIT_H

#include "DFactory.h"
#include "DEventLoop.h"
#include "DHDDMFDCHit.h"
#include "DException.h"

class DFactory_DHDDMFDCHit : public DFactory<DHDDMFDCHit>{
	public:
		DFactory_DHDDMFDCHit();
		~DFactory_DHDDMFDCHit();
		derror_t Extract_HDDM(s_HDDM_t *hddm_s, vector<void*> &v);
		const string toString(void);
	
	protected:
		//derror_t init(void);						///< Called once at program start.
		//derror_t brun(DEventLoop *eventLoop, int runnumber);	///< Called everytime a new run number is detected.
		derror_t evnt(DEventLoop *eventLoop, int eventnumber);	///< Called every event.
		//derror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
		//derror_t fini(void);						///< Called after last event of last event source has been processed.
};

#endif // DFACTORY_DHDDMFDCHIT_H

