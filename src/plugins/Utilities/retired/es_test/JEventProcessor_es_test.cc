// $Id$
//
//    File: JEventProcessor_es_test.cc
//

#include "JEventProcessor_es_test.h"

#include "EVENTSTORE/DESSkimData.h"

// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
#include <JANA/JFactoryT.h>
extern "C"{
    void InitPlugin(JApplication *app){
        InitJANAPlugin(app);
        app->Add(new JEventProcessor_es_test());
    }
} // "C"


//------------------
// JEventProcessor_es_test (Constructor)
//------------------
JEventProcessor_es_test::JEventProcessor_es_test()
{
	SetTypeName("JEventProcessor_es_test");
}

//------------------
// ~JEventProcessor_es_test (Destructor)
//------------------
JEventProcessor_es_test::~JEventProcessor_es_test()
{
}

//------------------
// Init
//------------------
void JEventProcessor_es_test::Init()
{
}

//------------------
// BeginRun
//------------------
void JEventProcessor_es_test::BeginRun(const std::shared_ptr<const JEvent>& event)
{
    // This is called whenever the run number changes
}

//------------------
// Process
//------------------
void JEventProcessor_es_test::Process(const std::shared_ptr<const JEvent>& event)
{
  //const DESSkimData *es_data = NULL;
  //event->GetSingle(es_data);
  vector<const DESSkimData *> es_data;
  event->Get(es_data);

  cout << "Event " << event->GetEventNumber() << endl;
  es_data[0]->Print();
}

//------------------
// EndRun
//------------------
void JEventProcessor_es_test::EndRun()
{
    // This is called whenever the run number changes, before it is
    // changed to give you a chance to clean up before processing
    // events from the next run number.
}

//------------------
// Finish
//------------------
void JEventProcessor_es_test::Finish()
{
    // Called before program exit after event processing is finished.
}

