// $Id$
//
//    File: JEventProcessor_janaded.h
// Created: Fri 20 Jul 2012 10:03:48 AM EDT 
// Creator: garmon
//

#ifndef _JEventProcessor_janaded_
#define _JEventProcessor_janaded_

#include <map>
#include <string>

#include <JANA/JEventProcessor.h>
#include <JANA/JFactory.h>
#include <cMsg.hxx>
using namespace cmsg;


class JEventProcessor_janaded:public JEventProcessor,public cmsg::cMsgCallback {
	public:
		JEventProcessor_janaded();
		~JEventProcessor_janaded(){};

		enum data_type_t{
			type_unknown,
			type_int,
			type_uint,
			type_long,
			type_ulong,
			type_float,
			type_double,
			type_string
		};
		
		
	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;
		void callback(cMsgMessage *msg, void *arg);                            ///< Callback method

		unsigned int Nevents;
		
		unsigned int Nwarnings;
		unsigned int MaxWarnings;
		
		int JANADED_VERBOSE;
		vector<string> nametags_to_write_out;

};

#endif // _JEventProcessor_janaded_

