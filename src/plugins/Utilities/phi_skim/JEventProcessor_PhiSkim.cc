// $Id$
//
//    File: JEventProcessor_PhiSkim.cc
// Created: Wed May 24 13:46:12 EDT 2017
// Creator: mashephe (on Linux stanley.physics.indiana.edu 2.6.32-642.6.2.el6.x86_64 unknown)
//

#include "JEventProcessor_PhiSkim.h"
#include "DFactoryGenerator_PhiSkim.h"

using namespace jana;

#include "evio_writer/DEventWriterEVIO.h"

#include <ANALYSIS/DAnalysisResults.h>
#include <ANALYSIS/DEventWriterROOT.h>

// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
#include <JANA/JFactory.h>
extern "C"{
  void InitPlugin(JApplication *app){
    InitJANAPlugin(app);
    app->AddProcessor(new JEventProcessor_PhiSkim());
    app->AddFactoryGenerator( new DFactoryGenerator_PhiSkim() );
  }
} // "C"


//------------------
// JEventProcessor_PhiSkim (Constructor)
//------------------
JEventProcessor_PhiSkim::JEventProcessor_PhiSkim()
{

  WRITE_EVIO_FILE = 1;
  gPARMS->SetDefaultParameter( "WRITE_EVIO_FILE", WRITE_EVIO_FILE );

  WRITE_ROOT_TREE = 1;
  gPARMS->SetDefaultParameter( "WRITE_ROOT_TREE", WRITE_ROOT_TREE );
}

//------------------
// ~JEventProcessor_PhiSkim (Destructor)
//------------------
JEventProcessor_PhiSkim::~JEventProcessor_PhiSkim()
{

}

//------------------
// init
//------------------
jerror_t JEventProcessor_PhiSkim::init(void)
{
  // This is called once at program startup. 

  return NOERROR;
}

//------------------
// brun
//------------------
jerror_t JEventProcessor_PhiSkim::brun(JEventLoop *eventLoop, int32_t runnumber)
{
  // This is called whenever the run number changes
  return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t JEventProcessor_PhiSkim::evnt(JEventLoop *loop, uint64_t eventnumber)
{

  vector<const DAnalysisResults*> locAnalysisResultsVector;
  loop->Get( locAnalysisResultsVector );

    const DEventWriterEVIO* eventWriterEVIO = NULL;
  if( WRITE_EVIO_FILE ){
    

    loop->GetSingle(eventWriterEVIO);

    // write out BOR events
    if(loop->GetJEvent().GetStatusBit(kSTATUS_BOR_EVENT)) {
      eventWriterEVIO->Write_EVIOEvent(loop, "phi");
      return NOERROR;
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
		return NOERROR;

  if( WRITE_EVIO_FILE ){
   eventWriterEVIO->Write_EVIOEvent(loop, "phi");

  }

  if( WRITE_ROOT_TREE ){
    
    //Recommended: Write surviving particle combinations (if any) to output ROOT TTree
    //If no cuts are performed by the analysis actions added to a DReaction, then this saves all of its particle combinations.
    //The event writer gets the DAnalysisResults objects from JANA, performing the analysis.
    // string is DReaction factory tag: will fill trees for all DReactions that are defined in the specified factory

    const DEventWriterROOT* eventWriterROOT = NULL;
    loop->GetSingle(eventWriterROOT);
    eventWriterROOT->Fill_DataTrees(loop, "PhiSkim");
  }
  
  return NOERROR;
}

//------------------
// erun
//------------------
jerror_t JEventProcessor_PhiSkim::erun(void)
{
  // This is called whenever the run number changes, before it is
  // changed to give you a chance to clean up before processing
  // events from the next run number.
  return NOERROR;
}

//------------------
// fini
//------------------
jerror_t JEventProcessor_PhiSkim::fini(void)
{
  // Called before program exit after event processing is finished.
  return NOERROR;
}

