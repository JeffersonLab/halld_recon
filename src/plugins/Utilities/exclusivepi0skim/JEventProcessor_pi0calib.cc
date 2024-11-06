// $Id$
//
//    File: JEventProcessor_pi0calib.cc
// Created: Wed May 24 13:46:12 EDT 2017
// Creator: mashephe (on Linux stanley.physics.indiana.edu 2.6.32-642.6.2.el6.x86_64 unknown)
//

#include "JEventProcessor_pi0calib.h"
#include "DFactoryGenerator_pi0calib.h"


#include "evio_writer/DEventWriterEVIO.h"

#include <ANALYSIS/DAnalysisResults.h>
#include <ANALYSIS/DEventWriterROOT.h>

// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
#include <JANA/JFactoryT.h>
extern "C"{
  void InitPlugin(JApplication *app){
    InitJANAPlugin(app);
    app->Add(new JEventProcessor_pi0calib());
    app->Add( new DFactoryGenerator_pi0calib() );
  }
} // "C"


//------------------
// JEventProcessor_pi0calib (Constructor)
//------------------
JEventProcessor_pi0calib::JEventProcessor_pi0calib()
{
  SetTypeName("JEventProcessor_pi0calib");
}

//------------------
// ~JEventProcessor_pi0calib (Destructor)
//------------------
JEventProcessor_pi0calib::~JEventProcessor_pi0calib()
{
}

//------------------
// Init
//------------------
void JEventProcessor_pi0calib::Init()
{
	// This is called once at program startup.
	auto app = GetApplication();

	WRITE_EVIO_FILE = 1;
	app->SetDefaultParameter( "WRITE_EVIO_FILE", WRITE_EVIO_FILE );

	WRITE_ROOT_TREE = 0;
	app->SetDefaultParameter( "WRITE_ROOT_TREE", WRITE_ROOT_TREE );
}

//------------------
// BeginRun
//------------------
void JEventProcessor_pi0calib::BeginRun(const std::shared_ptr<const JEvent>& event)
{
  // This is called whenever the run number changes
}

//------------------
// Process
//------------------
void JEventProcessor_pi0calib::Process(const std::shared_ptr<const JEvent>& event)
{

  vector<const DAnalysisResults*> locAnalysisResultsVector;
  event->Get( locAnalysisResultsVector );
  
  if( WRITE_EVIO_FILE ){
    
    const DEventWriterEVIO* eventWriterEVIO = NULL;
    event->GetSingle(eventWriterEVIO);

    // write out BOR events
    if(GetStatusBit(event, kSTATUS_BOR_EVENT)) {

      eventWriterEVIO->Write_EVIOEvent(event, "exclusivepi0");
      return;
    }

    //Make sure that there are combos that survived for ****THIS**** CHANNEL!!!
    bool locSuccessFlag = false;
    for(size_t loc_i = 0; loc_i < locAnalysisResultsVector.size(); ++loc_i)  {
        const DReaction* locReaction = locAnalysisResultsVector[loc_i]->Get_Reaction();
        if(locReaction->Get_ReactionName() != "excl_pi0calib")
            continue;

        deque<const DParticleCombo*> locPassedParticleCombos;
        locAnalysisResultsVector[loc_i]->Get_PassedParticleCombos(locPassedParticleCombos);
        locSuccessFlag = !locPassedParticleCombos.empty();
        break;
    }

    if( (locAnalysisResultsVector.size() > 0) && locSuccessFlag) {  // there are combos that satisfy our reaction
      //if( (locAnalysisResultsVector.size() > 0) && (locAnalysisResultsVector[0]->Get_NumPassedParticleCombos() != 0) ) {
            // SIMPLE - write out the full event
            //eventWriterEVIO->Write_EVIOEvent(loop, "exclusivepi0");
            
            // Instead:  Write out each combo as an "event", only saving the combo vertex, RF bunch, and showers associated with the combo
            // This should be a lot smaller

	    vector< const JObject* > locObjectsToSave;
            
            vector<const DEventRFBunch*> locEventRFBunches;
            event->Get(locEventRFBunches);
	    //locObjectsToSave.push_back(static_cast<const JObject *>(locEventRFBunches));
	    locObjectsToSave.push_back(locEventRFBunches[0]);

            vector<const DVertex*> kinfitVertex;
            event->Get(kinfitVertex);
	    locObjectsToSave.push_back(kinfitVertex[0]);
	    
            if(kinfitVertex.size() > 0) {   // pretty sure this should always be true if we have a combo....
                for( auto result : locAnalysisResultsVector ) {
                    deque<const DParticleCombo*> combos;
                    result->Get_PassedParticleCombos(combos);
                    for( auto combo : combos ) { 
                        // need to save the RF bunch info
                        //locObjectsToSave.push_back(static_cast<const JObject *>(combo->Get_EventRFBunch()));
			
                        // need to make a new vertex object - base it on the old one
                        // we probably don'e need most of this information, but keep it reasonable I guess
                        //DVertex *comboVertex = new DVertex(*(kinfitVertex[0]));
                        //comboVertex->dSpacetimeVertex = combo->Get_EventVertex();
                    
                        // Save the actual showers - the EVIO writer will only write out the associated hits
                        //set< const JObject *> bcalShowers;
                        auto particles = combo->Get_FinalParticles_Measured(result->Get_Reaction());
                        for( auto particle : particles ) {
                            if(ParticleCharge(particle->PID()) == 0) {
                                auto locNeutralParticleHypothesis = static_cast<const DNeutralParticleHypothesis*>(particle);
                                auto locNeutralShower = locNeutralParticleHypothesis->Get_NeutralShower();
				if(find(locObjectsToSave.begin(),locObjectsToSave.end(),locNeutralShower->dBCALFCALShower) == locObjectsToSave.end())
				  locObjectsToSave.push_back(locNeutralShower->dBCALFCALShower);
                                //bcalShowers.insert(locNeutralShower->dBCALFCALShower);
                            }
                        }
                        
                        // actually write the event out
                        //eventWriterEVIO->Write_EVIOEvent( loop, "exclusivepi0", locObjectsToSave );                    
                    }            
                }
            }

	    // actually write the event out
	    eventWriterEVIO->Write_EVIOEvent( event, "exclusivepi0", locObjectsToSave );

	    //}

        //for( auto *shower_ptr : bcalShowers ) {
        //    locObjectsToSave.push_back(static_cast<const JObject *>(shower_ptr));
        //}
        
    }
  }

  if( WRITE_ROOT_TREE ){
    
    //Recommended: Write surviving particle combinations (if any) to output ROOT TTree
    //If no cuts are performed by the analysis actions added to a DReaction, then this saves all of its particle combinations.
    //The event writer gets the DAnalysisResults objects from JANA, performing the analysis.
    // string is DReaction factory tag: will fill trees for all DReactions that are defined in the specified factory

    const DEventWriterROOT* eventWriterROOT = NULL;
    event->GetSingle(eventWriterROOT);
    eventWriterROOT->Fill_DataTrees(event, "excl_pi0calib");
  }
}

//------------------
// EndRun
//------------------
void JEventProcessor_pi0calib::EndRun()
{
  // This is called whenever the run number changes, before it is
  // changed to give you a chance to clean up before processing
  // events from the next run number.
}

//------------------
// Finish
//------------------
void JEventProcessor_pi0calib::Finish()
{
  // Called before program exit after event processing is finished.
}

