// $Id$
//
//    File: JEventProcessor_PhiSkim.cc
// Created: Wed May 24 13:46:12 EDT 2017
// Creator: mashephe (on Linux stanley.physics.indiana.edu 2.6.32-642.6.2.el6.x86_64 unknown)
//

#include "JEventProcessor_PhiSkim.h"
#include "DFactoryGenerator_PhiSkim.h"
#include "DANA/DStatusBits.h"


#include "evio_writer/DEventWriterEVIO.h"

#include <ANALYSIS/DAnalysisResults.h>
#include <ANALYSIS/DEventWriterROOT.h>

// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
#include <JANA/JFactory.h>
extern "C"{
  void InitPlugin(JApplication *app){
    InitJANAPlugin(app);
    app->Add(new JEventProcessor_PhiSkim());
    app->Add(new DFactoryGenerator_PhiSkim());
  }
} // "C"


//------------------
// JEventProcessor_PhiSkim (Constructor)
//------------------
JEventProcessor_PhiSkim::JEventProcessor_PhiSkim()
{
  auto app = GetApplication();
  WRITE_EVIO_FILE = 1;
  app->SetDefaultParameter( "WRITE_EVIO_FILE", WRITE_EVIO_FILE );

  WRITE_ROOT_TREE = 0;
  app->SetDefaultParameter( "WRITE_ROOT_TREE", WRITE_ROOT_TREE );
}

//------------------
// ~JEventProcessor_PhiSkim (Destructor)
//------------------
JEventProcessor_PhiSkim::~JEventProcessor_PhiSkim()
{

}

//------------------
// Init
//------------------
void JEventProcessor_PhiSkim::Init()
{
  // This is called once at program startup. 

  return;
}

//------------------
// BeginRun
//------------------
void JEventProcessor_PhiSkim::BeginRun(const std::shared_ptr<const JEvent>& event)
{
  // This is called whenever the run number changes
  return;
}

//------------------
// Process
//------------------
void JEventProcessor_PhiSkim::Process(const std::shared_ptr<const JEvent>& event)
{

  vector<const DAnalysisResults*> locAnalysisResultsVector;
  event->Get( locAnalysisResultsVector );

    const DEventWriterEVIO* eventWriterEVIO = NULL;
  if( WRITE_EVIO_FILE ){
    

    event->GetSingle(eventWriterEVIO);

    // write out BOR events
    if(event->GetSingle<DStatusBits>()->GetStatusBit(kSTATUS_BOR_EVENT)) {
      eventWriterEVIO->Write_EVIOEvent(event, "phi");
      return;
    }
  }

	//Make sure that there are combos that survived for ****THIS**** CHANNEL!!!
	bool locSuccessFlag = false;
	for(size_t loc_i = 0; loc_i < locAnalysisResultsVector.size(); ++loc_i)
	{
		const DReaction* locReaction = locAnalysisResultsVector[loc_i]->Get_Reaction();
		if( (locReaction->Get_ReactionName() != "kk_skim_excl") && (locReaction->Get_ReactionName() != "kk_skim_incl") )
			continue;

		deque<const DParticleCombo*> locPassedParticleCombos;
		locAnalysisResultsVector[loc_i]->Get_PassedParticleCombos(locPassedParticleCombos);
		locSuccessFlag = !locPassedParticleCombos.empty();
		break;
	}
	if(!locSuccessFlag)
		return;

  if( WRITE_EVIO_FILE ){
   eventWriterEVIO->Write_EVIOEvent(event, "phi");

  }

  if( WRITE_ROOT_TREE ){
    
    //Recommended: Write surviving particle combinations (if any) to output ROOT TTree
    //If no cuts are performed by the analysis actions added to a DReaction, then this saves all of its particle combinations.
    //The event writer gets the DAnalysisResults objects from JANA, performing the analysis.
    // string is DReaction factory tag: will fill trees for all DReactions that are defined in the specified factory

    const DEventWriterROOT* eventWriterROOT = NULL;
    event->GetSingle(eventWriterROOT);
    eventWriterROOT->Fill_DataTrees(event, "PhiSkim");
  }
  
  return;
}

//------------------
// EndRun
//------------------
void JEventProcessor_PhiSkim::EndRun()
{
  // This is called whenever the run number changes, before it is
  // changed to give you a chance to clean up before processing
  // events from the next run number.
  return;
}

//------------------
// Finish
//------------------
void JEventProcessor_PhiSkim::Finish()
{
  // Called before program exit after event processing is finished.
  return;
}

