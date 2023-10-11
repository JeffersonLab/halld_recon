// $Id$
//
//    File: JEventProcessor_Pi0Finder.cc
// Created: Fri Jul 11 14:18:49 EDT 2014
// Creator: mashephe (on Darwin 149-160-160-51.dhcp-bl.indiana.edu 13.3.0 i386)
//

#include "JEventProcessor_Pi0Finder.h"

#include "TTree.h"

#include "FCAL/DFCALShower.h"
#include "FCAL/DFCALCluster.h"
#include "FCAL/DFCALHit.h"
#include "FCAL/DFCALGeometry.h"

#include "ANALYSIS/DAnalysisResults.h"
#include "ANALYSIS/DParticleCombo.h"
#include "PID/DChargedTrack.h"
#include "PID/DNeutralShower.h"
#include "PID/DDetectorMatches.h"
#include "TRACKING/DTrackWireBased.h"
#include "DFactoryGenerator_OmegaSkim.h"
#include "DLorentzVector.h"
#include "DVector3.h"
#include "TTree.h"

#include <TRACKING/DTrackTimeBased.h>
#include <TRACKING/DTrackWireBased.h>
#include <FCAL/DFCALShower.h>
#include "TTree.h"

// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
#include <JANA/JFactory.h>
extern "C"
{
	void InitPlugin(JApplication *locApplication)
	{
		InitJANAPlugin(locApplication);
		locApplication->Add(new JEventProcessor_Pi0Finder()); //register this plugin
		locApplication->Add(new DFactoryGenerator_OmegaSkim()); //register this plugin
	}
} // "C"
thread_local DTreeFillData JEventProcessor_Pi0Finder::dTreeFillData;
//------------------
// init
//------------------
void JEventProcessor_Pi0Finder::Init()
{
	// This is called once at program startup. If you are creating
	// and filling historgrams in this plugin, you should lock the
	// ROOT mutex like this:

	//	japp->RootWriteLock();

	dTreeInterface = DTreeInterface::Create_DTreeInterface("fcalPi0", "tree_fcalPi0.root");

	DTreeBranchRegister treeBranchRegister;

	treeBranchRegister.Register_Single<Float_t>("xL");
	treeBranchRegister.Register_Single<Float_t>("yL");
	treeBranchRegister.Register_Single<Float_t>("zL");
	treeBranchRegister.Register_Single<Float_t>("eL");

	treeBranchRegister.Register_Single<Float_t>("xH");
	treeBranchRegister.Register_Single<Float_t>("yH");
	treeBranchRegister.Register_Single<Float_t>("zH");
	treeBranchRegister.Register_Single<Float_t>("eH");

	treeBranchRegister.Register_Single<Int_t>("typeShL");
	treeBranchRegister.Register_Single<Int_t>("typeShH");

	treeBranchRegister.Register_Single<Float_t>("dtShL");
	treeBranchRegister.Register_Single<Float_t>("tTrL");
	treeBranchRegister.Register_Single<Float_t>("tShL");
	treeBranchRegister.Register_Single<Float_t>("disShL");

	treeBranchRegister.Register_Single<Float_t>("dtShH");
	treeBranchRegister.Register_Single<Float_t>("tTrH");
	treeBranchRegister.Register_Single<Float_t>("tShH");
	treeBranchRegister.Register_Single<Float_t>("disShH");

	treeBranchRegister.Register_Single<Float_t>("t0RF");
	//----------------------------------

	treeBranchRegister.Register_Single<Float_t>("speedShL");
	treeBranchRegister.Register_Single<Float_t>("e1e9ShL");
	treeBranchRegister.Register_Single<Float_t>("e9e25ShL");
	treeBranchRegister.Register_Single<Float_t>("sumUShL");
	treeBranchRegister.Register_Single<Float_t>("sumVShL");
	treeBranchRegister.Register_Single<Float_t>("asymUVShL");
	treeBranchRegister.Register_Single<Int_t>("nHitsL");
	treeBranchRegister.Register_Single<Float_t>("dtTrShL");


	treeBranchRegister.Register_Single<Float_t>("speedShH");
	treeBranchRegister.Register_Single<Float_t>("e1e9ShH");
	treeBranchRegister.Register_Single<Float_t>("e9e25ShH");
	treeBranchRegister.Register_Single<Float_t>("sumUShH");
	treeBranchRegister.Register_Single<Float_t>("sumVShH");
	treeBranchRegister.Register_Single<Float_t>("asymUVShH");
	treeBranchRegister.Register_Single<Int_t>("nHitsH");
	treeBranchRegister.Register_Single<Float_t>("dtTrShH");
	treeBranchRegister.Register_Single<Float_t>( "beamE");
	//


	treeBranchRegister.Register_Single<Float_t>("qualL");
	treeBranchRegister.Register_Single<Float_t>("qualH");
	treeBranchRegister.Register_Single<Float_t>("invM");
	treeBranchRegister.Register_Single<Int_t>("nTrk");

	dTreeInterface->Create_Branches(treeBranchRegister);
	//----------------------------------
	//	japp->RootUnLock();
}

//------------------
// brun
//------------------
void JEventProcessor_Pi0Finder::BeginRun(const std::shared_ptr<const JEvent> &event)
{
	// This is called whenever the run number changes

	const DGeometry *geom = GetDGeometry(event);

	if( geom ) {

		geom->GetTargetZ( m_zTarget );
		geom->GetFCALZ( m_FCALUpstream );
	}
	else{

		cerr << "No geometry accessbile." << endl;
		return; // RESOURCE_UNAVAILABLE;
	}
}

//------------------
// evnt
//------------------
void JEventProcessor_Pi0Finder::Process(const std::shared_ptr<const JEvent> &event)
{
	vector<const DAnalysisResults*> analysisResultsVector;
	event->Get( analysisResultsVector );

	vector<const DFCALShower*> showerVector;
	event->Get( showerVector );

	vector<const DEventRFBunch*> eventRFBunches;
	event->Get( eventRFBunches );

	vector<const DFCALCluster*> fcalClusters;
	event->Get(fcalClusters);

	vector<const DChargedTrack*> chargedTracks;
        event->Get( chargedTracks );

        dTreeFillData.Fill_Single<Int_t>("nTrk", chargedTracks.size());
	

	map< const DFCALShower*, double > showerQualityMap;
	vector< const DNeutralShower* > neutralShowers;
	event->Get( neutralShowers );

	for( size_t i = 0; i < neutralShowers.size(); ++i ){

		if( neutralShowers[i]->dDetectorSystem == SYS_FCAL ){

			const DFCALShower* fcalShower = dynamic_cast< const DFCALShower* >( neutralShowers[i]->dBCALFCALShower );	
			showerQualityMap[fcalShower] = neutralShowers[i]->dQuality;
		}
	}
	///********************


	for(vector< const DAnalysisResults* >::const_iterator res = analysisResultsVector.begin(); res!= analysisResultsVector.end(); ++res){
		const DReaction* reaction = (**res).Get_Reaction();

		if( reaction->Get_ReactionName() != "p3pi_excl" ) continue;
		size_t nCombos = (**res).Get_NumPassedParticleCombos();

		if( nCombos != 1 ) return;
		deque< const DParticleCombo* > combos;
		(**res).Get_PassedParticleCombos( combos );
		const DParticleCombo* combo = combos[0];

		vector< std::shared_ptr< const DFCALShowerMatchParams > > matchedShowers;

		const DParticleComboStep* initialStep = combo->Get_ParticleComboStep( 0 );
		DLorentzVector beam = initialStep->Get_InitialParticle_Measured()->lorentzMomentum();

		dTreeFillData.Fill_Single<Float_t>("beamE", beam.E());

		map<const DFCALShower*, int> showerMatchIndex;
		map<const DFCALShower*, const DKinematicData*> showerMatchTrack;
		list<const DFCALShower*> pi0Showers;
		vector< const JObject* > neutralSource = combo->Get_FinalParticle_SourceObjects( d_Neutral );

		for( size_t i = 0; i < neutralSource.size(); ++i ){

			const DNeutralShower* neutShower = dynamic_cast< const DNeutralShower* >( neutralSource[i] );
			if( neutShower == NULL ) continue;

			if( neutShower->dDetectorSystem == SYS_FCAL ){

				pi0Showers.push_back( dynamic_cast< const DFCALShower* >( neutShower->dBCALFCALShower ) );
			}
		}

		vector<const DKinematicData*> tracks =
			combo->Get_FinalParticles( reaction, false, false, d_Charged );
		for( size_t iTrk = 0; iTrk < tracks.size(); ++iTrk ){

			const DChargedTrackHypothesis* ctHypo =
				dynamic_cast<const DChargedTrackHypothesis*>( tracks[iTrk] );
			std::shared_ptr< const DFCALShowerMatchParams > match =
				ctHypo->Get_FCALShowerMatchParams();

			if( match == NULL ) continue;

			const DFCALShower* showerPtr = match->dFCALShower;
			showerMatchTrack[showerPtr] = tracks[iTrk];
			showerMatchIndex[showerPtr] = matchedShowers.size();
			matchedShowers.push_back( match );
		}



		DVector3 targetCenter( 0, 0, m_zTarget ); 


		dTreeFillData.Fill_Single<Float_t>("t0RF", eventRFBunches[0]->dTime);
		t0RF = eventRFBunches[0]->dTime;


		for( size_t iSh = 0; iSh < showerVector.size(); ++iSh ){
			for( size_t jSh = iSh+1; jSh < showerVector.size(); ++jSh ){



				const DFCALShower* shL = ( showerVector[iSh]->getEnergy() >
						showerVector[jSh]->getEnergy() ?
						showerVector[jSh] : showerVector[iSh] );

				const DFCALShower* shH = ( showerVector[iSh]->getEnergy() <
						showerVector[jSh]->getEnergy() ?
						showerVector[jSh] : showerVector[iSh] );

				DVector3 posL = shL->getPosition();
				posL.SetZ( posL.Z() - m_zTarget );
				dTreeFillData.Fill_Single<Float_t>("xL", posL.X());
				dTreeFillData.Fill_Single<Float_t>("yL", posL.Y());
				dTreeFillData.Fill_Single<Float_t>("zL", posL.Z());
				dTreeFillData.Fill_Single<Float_t>("eL", shL->getEnergy());
				dTreeFillData.Fill_Single<Float_t>("qualL", showerQualityMap[shL]);
				eL = shL->getEnergy();
				posL.SetMag( eL );
				DLorentzVector p4L( posL, eL );
				//--------------------

				dTreeFillData.Fill_Single<Float_t>("disShL", ( shL->getPosition() - targetCenter ).Mag());
				disShL =  ( shL->getPosition() - targetCenter ).Mag();
				dTreeFillData.Fill_Single<Float_t>("dtShL", shL->getTime() - t0RF);
				dtShL = shL->getTime() - t0RF;
				dTreeFillData.Fill_Single<Float_t>("speedShL", disShL/dtShL);

				dTreeFillData.Fill_Single<Float_t>("e1e9ShL", shL->getE1E9());
				dTreeFillData.Fill_Single<Float_t>("e9e25ShL", shL->getE9E25());
				dTreeFillData.Fill_Single<Float_t>("sumUShL", shL->getSumU());
				dTreeFillData.Fill_Single<Float_t>("sumVShL", shL->getSumV());

				sumUShL = shL->getSumU();
				sumVShL = shL->getSumV();

				dTreeFillData.Fill_Single<Float_t>("asymUVShL", ( sumUShL - sumVShL ) / ( sumUShL + sumVShL ));
				dTreeFillData.Fill_Single<Int_t>("nHitsL", shL->getNumBlocks());
				dTreeFillData.Fill_Single<Float_t>("tTrL", shL->getTimeTrack());
				dTreeFillData.Fill_Single<Float_t>("tShL", shL->getTime());
				tTrL = shL->getTimeTrack();
				tShL = shL->getTime();
				dTreeFillData.Fill_Single<Float_t>("dtTrShL", dtShL - tTrL -0.3);


				dTreeFillData.Fill_Single<Float_t>("disShH", ( shH->getPosition() - targetCenter ).Mag());
				disShH =( shH->getPosition() - targetCenter ).Mag(); 
				dTreeFillData.Fill_Single<Float_t>("dtShH", shH->getTime() - t0RF);
				dtShH =  shH->getTime() - t0RF;
				dTreeFillData.Fill_Single<Float_t>("speedShH", disShH/dtShH);

				dTreeFillData.Fill_Single<Float_t>("e1e9ShH", shH->getE1E9());
				dTreeFillData.Fill_Single<Float_t>("e9e25ShH", shH->getE9E25());
				dTreeFillData.Fill_Single<Float_t>("sumUShH", shH->getSumU());
				dTreeFillData.Fill_Single<Float_t>("sumVShH", shH->getSumV());

				sumUShH = shH->getSumU();
				sumVShH = shH->getSumV();

				dTreeFillData.Fill_Single<Float_t>("asymUVShH", ( sumUShH - sumVShH ) / ( sumUShH + sumVShH ));
				dTreeFillData.Fill_Single<Int_t>("nHitsH", shH->getNumBlocks());
				dTreeFillData.Fill_Single<Float_t>("tTrH", shH->getTimeTrack());
				dTreeFillData.Fill_Single<Float_t>("tShH", shH->getTime());

				tTrH = shH->getTimeTrack();
				tShH = shH->getTime();
				dTreeFillData.Fill_Single<Float_t>("dtTrShH", dtShH - tTrH - 0.3);
				//--------------------
				if( showerMatchIndex.find(shL) != showerMatchIndex.end()){
					typeShL =1;
				}
				else if( find(pi0Showers.begin(), pi0Showers.end(), shL) !=pi0Showers.end()){
					typeShL=0;
				}
				else{
					typeShL=2;
				}

				dTreeFillData.Fill_Single<Int_t>("typeShL", typeShL);			

				if( showerMatchIndex.find(shH) != showerMatchIndex.end()){
					typeShH =1;
				}
				else if( find(pi0Showers.begin(), pi0Showers.end(), shH) !=pi0Showers.end()){
					typeShH=0;
				}
				else{
					typeShH=2;
				}

				dTreeFillData.Fill_Single<Int_t>("typeShH", typeShH);			

				DVector3 posH = shH->getPosition();
				posH.SetZ( posH.Z() - m_zTarget );
				dTreeFillData.Fill_Single<Float_t>("xH", posH.X());
				dTreeFillData.Fill_Single<Float_t>("yH", posH.Y());
				dTreeFillData.Fill_Single<Float_t>("zH", posH.Z());
				dTreeFillData.Fill_Single<Float_t>("eH", shH->getEnergy());
				dTreeFillData.Fill_Single<Float_t>("qualH", showerQualityMap[shH]);
				eH = shH->getEnergy();
				posH.SetMag( eH );
				DLorentzVector p4H( posH, eH );

				dTreeFillData.Fill_Single<Float_t>("invM", ( p4L + p4H ).M());

				dTreeInterface->Fill(dTreeFillData);
				//	m_tree->Fill();
			}
		}

		//	japp->RootFillUnLock(this);

	}
}

//------------------
// erun
//------------------
void JEventProcessor_Pi0Finder::EndRun()
{
	// This is called whenever the run number changes, before it is
	// changed to give you a chance to clean up before processing
	// events from the next run number.
}

//------------------
// fini
//------------------
void JEventProcessor_Pi0Finder::Finish()
{
	// Called before program exit after event processing is finished.
	//japp->RootWriteLock();
	//m_tree->Write();
	//japp->RootUnLock();
	delete dTreeInterface;
}
