
#ifndef _JEventProcessor_TPOL_online_
#define _JEventProcessor_TPOL_online_

#include <JANA/JEventProcessor.h>
#include <JANA/Compatibility/JLockService.h>


class JEventProcessor_TPOL_online:public JEventProcessor{
public:
    JEventProcessor_TPOL_online();
    ~JEventProcessor_TPOL_online();

    // static const UInt_t NSECTORS = 32;

private:
    void Init() override;
    void BeginRun(const std::shared_ptr<const JEvent>& event) override;
    void Process(const std::shared_ptr<const JEvent>& event) override;
    void EndRun() override;
    void Finish() override;

    std::shared_ptr<JLockService> lockService;
};

#endif // _JEventProcessor_TPOL_online_

