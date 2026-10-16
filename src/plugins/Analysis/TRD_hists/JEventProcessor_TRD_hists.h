
#ifndef _JEventProcessor_TRD_hists_
#define _JEventProcessor_TRD_hists_

#include <JANA/JEventProcessor.h>
#include <TRACKING/DTrackFitter.h>
#include <TRACKING/DTrackWireBased.h>
#include <TRACKING/DTrackTimeBased.h>
#include <HDGEOMETRY/DMagneticFieldMapNoField.h>

class JEventProcessor_TRD_hists:public JEventProcessor{
public:
    JEventProcessor_TRD_hists();
    ~JEventProcessor_TRD_hists();

private:
    void Init() override;
	void BeginRun(const std::shared_ptr<const JEvent>& event) override;
	void Process(const std::shared_ptr<const JEvent>& event) override;
	void EndRun() override;
    void Finish() override;
    int wirePlaneOffset;
    bool dIsNoFieldFlag;
};

#endif // _JEventProcessor_TRD_hists_

