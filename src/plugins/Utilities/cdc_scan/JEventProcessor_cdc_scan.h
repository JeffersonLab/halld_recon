// $Id$
//
//    File: JEventProcessor_cdc_scan.h
// Created: Sat Dec  6 21:34:19 EST 2014
// Creator: njarvis (on Linux maria 2.6.32-431.20.3.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_cdc_scan_
#define _JEventProcessor_cdc_scan_

#include <JANA/JEventProcessor.h>
#include <JANA/Compatibility/JLockService.h>


class JEventProcessor_cdc_scan:public JEventProcessor{
	public:
		JEventProcessor_cdc_scan();
		~JEventProcessor_cdc_scan();
		const char* className(void){return "JEventProcessor_cdc_scan";}

		//                TTree *cdctree;

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& locEvent) override;
		void Process(const std::shared_ptr<const JEvent>& locEvent) override;
		void EndRun() override;
		void Finish() override;

                std::shared_ptr<JLockService> lockService;

                int EMU; // set to 1 to run emulation if window raw data is present
                int FDC; // set to 0 to skip FDC data

};

#endif // _JEventProcessor_cdc_scan_

