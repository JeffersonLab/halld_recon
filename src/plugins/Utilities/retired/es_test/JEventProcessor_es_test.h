// $Id$
//
//    File: JEventProcessor_es_test.h
// Created: Mon May 18 09:52:08 EDT 2015
//

#ifndef _JEventProcessor_es_test_
#define _JEventProcessor_es_test_

#include <JANA/JEventProcessor.h>

class JEventProcessor_es_test:public JEventProcessor{
public:
    JEventProcessor_es_test();
    ~JEventProcessor_es_test();

private:
    void Init() override;
    void BeginRun(const std::shared_ptr<const JEvent>& event) override;
    void Process(const std::shared_ptr<const JEvent>& event) override;
    void EndRun() override;
    void Finish() override;
};

#endif // _JEventProcessor_es_test_

