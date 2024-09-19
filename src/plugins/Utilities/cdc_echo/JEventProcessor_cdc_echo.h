// $Id$
//
//    File: JEventProcessor_cdc_echo.h
// Created: Sat Dec  6 21:34:19 EST 2014
// Creator: njarvis (on Linux maria 2.6.32-431.20.3.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_cdc_echo_
#define _JEventProcessor_cdc_echo_

#include <JANA/JEventProcessor.h>



class JEventProcessor_cdc_echo:public JEventProcessor{
	public:
		JEventProcessor_cdc_echo();
		~JEventProcessor_cdc_echo();
		const char* className(void){return "JEventProcessor_cdc_echo";}

		//                TTree *cdctree;

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;		   


		TH1I *counts = NULL;
		TH2I *amp_t = NULL;
		TH2I *amp_tt = NULL;
 
  		
  //  unsigned int ECHO_ORIGIN; // threshold to consider large pulses as possibly causing afterpulses (adc range 0-4095), "511" would be 4088
  unsigned int ECHO_A; // do not suppress possible afterpulses larger than this (adc range 0-4095)
  unsigned int ECHO_DT;  // search up to this many samples after the main pulse for afterpulses
		
};

#endif // _JEventProcessor_cdc_echo_

