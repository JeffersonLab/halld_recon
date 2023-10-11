// $Id$
//
//    File: JEventProcessor_dumpthrowns.h
// Created: Tue Feb  4 09:29:35 EST 2014
// Creator: davidl (on Linux ifarm1102 2.6.32-220.7.1.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_dumpthrowns_
#define _JEventProcessor_dumpthrowns_

#include <map>
#include <fstream>
using namespace std;

#include <JANA/JEventProcessor.h>
#include <JANA/Compatibility/JLockService.h>
#include <HDGEOMETRY/DGeometry.h>

class JEventProcessor_dumpthrowns:public JEventProcessor{
	public:
		JEventProcessor_dumpthrowns();
		~JEventProcessor_dumpthrowns();

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;
		std::shared_ptr<JLockService> lockService;


		DGeometry *dgeom;
		map<unsigned long, int> wireID;
		ofstream *ofs;
		unsigned int MAX_CANDIDATE_FILTER;
		unsigned long events_written;
		unsigned long events_discarded;
		
		unsigned long GetCDCWireID(const DCDCWire* w){ return w->ring*100 + w->straw;}
		unsigned long GetFDCWireID(const DFDCWire* w){ return 100000 + w->layer*100 + w->wire;}
		int GetWireIndex(unsigned long id){ return wireID[id]; }
};

#endif // _JEventProcessor_dumpthrowns_

