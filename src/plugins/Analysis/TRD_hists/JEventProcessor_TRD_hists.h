
#ifndef _JEventProcessor_TRD_hists_
#define _JEventProcessor_TRD_hists_

#include <JANA/JEventProcessor.h>
#include <TRACKING/DTrackFitter.h>
#include <TRACKING/DTrackWireBased.h>
#include <TRACKING/DTrackTimeBased.h>
#include <HDGEOMETRY/DMagneticFieldMapNoField.h>

class JEventProcessor_TRD_hists:public jana::JEventProcessor{
public:
    JEventProcessor_TRD_hists();
    ~JEventProcessor_TRD_hists();
    const char* className(void){return "JEventProcessor_TRD_hists";}

private:
    jerror_t init(void); ///< Called once at program start.
    jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber); ///< Called everytime a new run number is detected.
    jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber); ///< Called every event.
    jerror_t erun(void); ///< Called everytime run number changes, provided brun has been called.
    jerror_t fini(void); ///< Called after last event of last event source has been processed.
    int wirePlaneOffset;
    bool dIsNoFieldFlag;
};

#endif // _JEventProcessor_TRD_hists_

