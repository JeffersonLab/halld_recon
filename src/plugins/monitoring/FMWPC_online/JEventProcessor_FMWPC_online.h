// $Id$
//
//    File: JEventProcessor_FMWPC_online.h
// Created: Fri Oct 22 13:34:04 EDT 2021
// Creator: aaustreg (on Linux ifarm1802.jlab.org 3.10.0-1062.4.1.el7.x86_64 x86_64)
//

#ifndef _JEventProcessor_FMWPC_online_
#define _JEventProcessor_FMWPC_online_

#include <JANA/JEventProcessor.h>
#include <JANA/Compatibility/JLockService.h>
#include <vector>

class JEventProcessor_FMWPC_online:public JEventProcessor{
	public:
		JEventProcessor_FMWPC_online();
		~JEventProcessor_FMWPC_online();

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;

		double ctof_t_base_adc,ctof_t_base_tdc;
		std::vector<double> ctof_adc_time_offsets, ctof_tdc_time_offsets;
};

#endif // _JEventProcessor_FMWPC_online_

