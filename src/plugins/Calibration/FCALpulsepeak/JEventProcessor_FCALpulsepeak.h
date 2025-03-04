// $Id$
//
//    File: JEventProcessor_FCALpulsepeak.h
// Created: Tue Sep 27 11:18:28 EDT 2016
// Creator: asubedi (on Linux stanley.physics.indiana.edu 2.6.32-573.18.1.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_FCALpulsepeak_
#define _JEventProcessor_FCALpulsepeak_

#include <JANA/JEventProcessor.h>

class JEventProcessor_FCALpulsepeak:public JEventProcessor{
	public:
		JEventProcessor_FCALpulsepeak();
		~JEventProcessor_FCALpulsepeak();

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;
		int m_x;
  		int m_y;
  		int m_chan;
		double m_peak;


};

#endif // _JEventProcessor_FCALpulsepeak_

