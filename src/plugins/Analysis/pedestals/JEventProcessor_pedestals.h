// $Id$
//
//    File: JEventProcessor_pedestals.h
// Created: Fri Jun 20 22:13:58 EDT 2014
// Creator: davidl (on Darwin harriet.local 13.2.0 i386)
//

#ifndef _JEventProcessor_pedestals_
#define _JEventProcessor_pedestals_

#include <TH2.h>

#include <JANA/JEventProcessor.h>
#include <DAQ/DDAQAddress.h>

class JEventProcessor_pedestals:public JEventProcessor{
	public:
		JEventProcessor_pedestals();
		~JEventProcessor_pedestals();

		class csc_t{
			public:
				csc_t(uint32_t rocid, uint32_t slot, uint32_t channel):rocid(rocid), slot(slot), channel(channel){}
				uint32_t rocid;
				uint32_t slot;
				uint32_t channel;
		};
		
		TH2D* GetHist(const DDAQAddress *hit);
		
		std::map<csc_t, TH2D*> all_hists;
		//pthread_mutex_t mutex;

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;

};

// Need to define a less-than operator for csc_t so it can be used as a key in the map
inline bool operator<(const JEventProcessor_pedestals::csc_t &a, const JEventProcessor_pedestals::csc_t &b){
	if(a.rocid < b.rocid) return true;
	if(a.rocid > b.rocid) return false;
	if(a.slot < b.slot) return true;
	if(a.slot > b.slot) return false;
	if(a.channel < b.channel) return true;
	return false;
}


#endif // _JEventProcessor_pedestals_

