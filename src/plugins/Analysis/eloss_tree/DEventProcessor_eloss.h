// $Id$
//
//    File: DEventProcessor_eloss.h
// Created: Mon Mar 22 15:58:47 EDT 2010
// Creator: davidl (on Darwin harriet.jlab.org 9.8.0 i386)
//

#ifndef _DEventProcessor_eloss_
#define _DEventProcessor_eloss_

#include <TTree.h>

#include <JANA/JEventProcessor.h>

class DEventProcessor_eloss:public JEventProcessor{
	public:
		DEventProcessor_eloss();
		~DEventProcessor_eloss();

		typedef struct{
			int event;
			float s;
			float x,y,z;
			float P;
			float dP;
			int mech;
		}event_t;
		
		TTree *geant;
		TTree *dana;
		event_t geant_event;
		event_t dana_event;

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;
};

#endif // _DEventProcessor_eloss_

