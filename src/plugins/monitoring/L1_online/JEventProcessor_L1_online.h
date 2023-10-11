// $Id$
//
//    File: JEventProcessor_L1_online.h
// Created: Fri Mar 20 16:32:04 EDT 2015
//

#ifndef _JEventProcessor_L1_online_
#define _JEventProcessor_L1_online_

#include <JANA/JEventProcessor.h>
#include <JANA/Compatibility/JLockService.h>

class JEventProcessor_L1_online:public JEventProcessor{
public:
    JEventProcessor_L1_online();
    ~JEventProcessor_L1_online();

private:
    void Init() override;
    void BeginRun(const std::shared_ptr<const JEvent>& event) override;
    void Process(const std::shared_ptr<const JEvent>& event) override;
    void EndRun() override;
    void Finish() override;

    std::shared_ptr<JLockService> lockService;


    int fcal_cell_thr;

    int bcal_cell_thr;

    int fcal_row_mask_min, fcal_row_mask_max, fcal_col_mask_min, fcal_col_mask_max;

    int run_number;



};

#endif // _JEventProcessor_L1_online_

