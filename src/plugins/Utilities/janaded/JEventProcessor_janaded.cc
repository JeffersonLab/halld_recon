// $Id$
//
//    File: JEventProcessor_janaded.cc
// Created: Fri 20 Jul 2012 10:03:57 AM EDT 
// Creator: garmon
//

#include <iostream>
#include <fstream>
#include <queue>
using namespace std;

#include <JANA/JApplication.h>
#include "JEventProcessor_janaded.h"

#include <pthread.h>


static pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond;
static queue<string> commands;



// Routine used to allow us to register our JEventSourceGenerator
extern "C"{
void InitPlugin(JApplication *app){
	InitJANAPlugin(app);
	app->Add(new JEventProcessor_janaded());
}
} // "C"

// Tokenize a string
static inline void Tokenize(string str, vector<string> &tokens, const char delim=' ')
{
	tokens.clear();
	unsigned int cutAt;
	while( (cutAt = str.find(delim)) != (unsigned int)str.npos ){
		if(cutAt > 0)tokens.push_back(str.substr(0,cutAt));
		str = str.substr(cutAt+1);
	}
	if(str.length() > 0)tokens.push_back(str);
}

//-----------------------------------------
// JEventProcessor_janaded (constructor)
//-----------------------------------------
JEventProcessor_janaded::JEventProcessor_janaded()
{
	SetTypeName("JEventProcessor_janaded");

	string myName = "JEventProcessor_janaded";
  	string myDescription = "janaded";
  	string UDL = "cMsg:cMsg://localhost/cMsg/test";

	cMsg* conn = new cMsg(UDL, myName, myDescription);
	conn->connect();
	conn->subscribe("janaded", "test", this, NULL);
	conn->start();

	japp->monitor_heartbeat=false;

}

//------------------
// Init
//------------------
void JEventProcessor_janaded::Init()
{
	auto app = GetApplication();

	JANADED_VERBOSE=0;
	app->GetJParameterManager()->SetDefaultParameter("JANADED_VERBOSE", JANADED_VERBOSE);
}

//------------------
// BeginRun
//------------------
void JEventProcessor_janaded::BeginRun(const std::shared_ptr<const JEvent>& event)
{
}

//------------------
// Process
//------------------
void JEventProcessor_janaded::Process(const std::shared_ptr<const JEvent>& event)
{
	while(true) {
		pthread_mutex_lock(&mutex1);
		pthread_cond_wait( &cond, &mutex1 );
		string command = commands.front();
		commands.pop();
		if(command=="NEXT_EVENT") {
			cout << "Next Event Command" <<endl;
			return;
		}
		else if (command=="Whatever") {
			//do whatever
		}
		else {
			cout << "Command was" << command <<endl;
		}

		pthread_mutex_unlock(&mutex1);
	}
	
}

//------------------
// EndRun
//------------------
void JEventProcessor_janaded::EndRun()
{
	return;
}

//------------------
// Finish
//------------------
void JEventProcessor_janaded::Finish()
{

	return;
}

void JEventProcessor_janaded::callback(cMsgMessage *msg, void *arg) {  
	pthread_mutex_lock(&mutex1);
	commands.push(msg->getText());
	pthread_mutex_unlock(&mutex1);	
	//pthread_cond_broadcast( &cond );
	pthread_cond_signal( &cond ); 
	
	delete msg;
}



