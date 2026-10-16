// $Id$
//
//    File: JEventProcessor_EventTagPi0.h
// Created: Fri Feb  5 23:23:22 EST 2016
// Creator: davidl (on Darwin harriet 13.4.0 i386)
//

#ifndef _JEventProcessor_EventTagPi0_
#define _JEventProcessor_EventTagPi0_

#include <JANA/JEventProcessor.h>

class JEventProcessor_EventTagPi0:public JEventProcessor{
	public:
		JEventProcessor_EventTagPi0();
		~JEventProcessor_EventTagPi0();

		double Emin_MeV;
		double Rmin_cm;
		double Rmin_cm_2;

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;
};

#endif // _JEventProcessor_EventTagPi0_

