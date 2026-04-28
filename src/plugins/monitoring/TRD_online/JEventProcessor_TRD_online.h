
#ifndef _JEventProcessor_TRD_online_
#define _JEventProcessor_TRD_online_

#include <JANA/JEventProcessor.h>
#include <JANA/Services/JLockService.h>

class JEventProcessor_TRD_online:public JEventProcessor{
public:
    JEventProcessor_TRD_online();
    ~JEventProcessor_TRD_online();

private:
    void Init() override;
    void BeginRun(const std::shared_ptr<const JEvent>& event) override;
    void Process(const std::shared_ptr<const JEvent>& event) override;
    void EndRun() override;
    void Finish() override;

    std::shared_ptr<JLockService> lockService;

    int wirePlaneOffset;
    int eventClusterCount; ///< Counter for number of events processed for cluster monitoring
    int eventPointCount; ///< Counter for number of events processed for point and segment monitoring
    int NEventsTrackSegmentMatch;
    int NEventsTrack;
};

#endif // _JEventProcessor_TRD_online_

