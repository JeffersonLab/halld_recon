//
//    File: JEventProcessor_danarest.h
// Created: Mon Jul 1 09:08:37 EDT 2012
// Creator: Richard Jones
//

#ifndef _JEventProcessor_danarest_
#define _JEventProcessor_danarest_

#include <string>
using namespace std;

#include <JANA/JEventProcessor.h>

#include <HDDM/DEventWriterREST.h>
#include <TRIGGER/DTrigger.h>

class JEventProcessor_danarest : public JEventProcessor
{
	public:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;

 private:
		bool is_mc;
};

#endif // _JEventProcessor_danarest_
