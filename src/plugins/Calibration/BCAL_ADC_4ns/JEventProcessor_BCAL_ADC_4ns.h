// $Id$
//
//    File: JEventProcessor_BCAL_ADC_4ns.h
// Created: Fri Jul 21 10:41:38 EDT 2017
// Creator: dalton (on Linux gluon106.jlab.org 2.6.32-642.3.1.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_BCAL_ADC_4ns_
#define _JEventProcessor_BCAL_ADC_4ns_

#include <JANA/JEventProcessor.h>

#include <TH2F.h>

class JEventProcessor_BCAL_ADC_4ns:public JEventProcessor{
	public:
		JEventProcessor_BCAL_ADC_4ns();
		~JEventProcessor_BCAL_ADC_4ns();
		const char* className(void){return "JEventProcessor_BCAL_ADC_4ns";}

		static const int nummodule=48;
		static const int numlayer=4;
		static const int numsector=4;


	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;

		TH2F *hZvsDeltat[JEventProcessor_BCAL_ADC_4ns::nummodule][JEventProcessor_BCAL_ADC_4ns::numlayer][JEventProcessor_BCAL_ADC_4ns::numsector];

};

#endif // _JEventProcessor_BCAL_ADC_4ns_

