// $Id$
//
//    File: JEventProcessor_Pi0Finder.cc
// Created: Fri Jul 11 14:18:49 EDT 2014
// Creator: mashephe (on Darwin 149-160-160-51.dhcp-bl.indiana.edu 13.3.0 i386)
//

#include "JEventProcessor_Pi0Finder.h"

using namespace jana;

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
		locApplication->AddProcessor(new JEventProcessor_Pi0Finder()); //register this plugin

		locApplication->AddFactoryGenerator(new DFactoryGenerator_OmegaSkim()); //register this plugin

	}
} // "C"

//------------------
// init
//------------------
jerror_t JEventProcessor_Pi0Finder::init(void)
{
	// This is called once at program startup. If you are creating
	// and filling historgrams in this plugin, you should lock the
	// ROOT mutex like this:

	japp->RootWriteLock();



	m_tree = new TTree( "fcalPi0", "Showers" );

	m_tree->Branch( "xL", &m_xL, "xL/F" );
	m_tree->Branch( "yL", &m_yL, "yL/F" );
	m_tree->Branch( "zL", &m_zL, "zL/F" );
	m_tree->Branch( "eL", &m_eL, "eL/F" );

	m_tree->Branch( "xH", &m_xH, "xH/F" );
	m_tree->Branch( "yH", &m_yH, "yH/F" );
	m_tree->Branch( "zH", &m_zH, "zH/F" );
	m_tree->Branch( "eH", &m_eH, "eH/F" );
	//----------------------------------
	m_tree->Branch( "typeShL", &m_typeShL, "typeShL/I" ); 
	m_tree->Branch("dtShL", &m_dtShL, "dtShL/F");
	m_tree->Branch("tTrL", &m_tTrL, "tTrL/F");
	m_tree->Branch("tShL", &m_tShL, "tShL/F");
	m_tree->Branch("disShL", &m_disShL, "disShL/F");

	m_tree->Branch( "speedShL", &m_speedShL, "speedShL/F" );
	m_tree->Branch( "e1e9ShL", &m_e1e9ShL, "e1e9ShL/F" );
	m_tree->Branch( "e9e25ShL", &m_e9e25ShL, "e9e25ShL/F" );
	m_tree->Branch( "sumUShL", &m_sumUShL, "sumUShL/F" );
	m_tree->Branch( "sumVShL", &m_sumVShL, "sumVShL/F" );
	m_tree->Branch( "asymUVShL", &m_asymUVShL, "asymUVShL/F" );
	m_tree->Branch( "nHitsL", &m_nHitsL, "nHitsL/I" );
	m_tree->Branch( "dtTrShL", &m_dtTrShL, "dtTrShL/F");

	//	m_tree->Branch( "eHitL", &m_eHitL, "eHitL/F");
	//	m_tree->Branch( "eHitH", &m_eHitH, "eHitH/F");

	//	m_tree->Branch( "beamE", &m_beamE, "beamE/F");
	//


	m_tree->Branch("t0RF", &m_t0RF, "t0RF/F");
	m_tree->Branch( "typeShH", &m_typeShH, "typeShH/I" );
	m_tree->Branch("dtShH", &m_dtShH, "dtShH/F");
	m_tree->Branch("tTrH", &m_tTrH, "tTrH/F");
	m_tree->Branch("tShH", &m_tShH, "tShH/F");
	m_tree->Branch("disShH", &m_disShH, "disShH/F");

	m_tree->Branch( "speedShH", &m_speedShH, "speedShH/F" );
	m_tree->Branch( "dtTrShH", &m_dtTrShH, "dtTrShH/F");
	m_tree->Branch( "e1e9ShH", &m_e1e9ShH, "e1e9ShH/F" );
	m_tree->Branch( "e9e25ShH", &m_e9e25ShH, "e9e25ShH/F" );
	m_tree->Branch( "sumUShH", &m_sumUShH, "sumUShH/F" );
	m_tree->Branch( "sumVShH", &m_sumVShH, "sumVShH/F" );
	m_tree->Branch( "asymUVShH", &m_asymUVShH, "asymUVShH/F" );
	m_tree->Branch( "nHitsH", &m_nHitsH, "nHitsH/I" );
	//----------------------------------
	m_tree->Branch( "qualL", &m_qualL, "qualL/F" );
	m_tree->Branch( "qualH", &m_qualH, "qualH/F" );

	m_tree->Branch( "invM", &m_invM, "invM/F" );
	m_tree->Branch( "nTrk", &m_nTrk, "nTrk/I" );

	japp->RootUnLock();

	return NOERROR;
}

//------------------
// brun
//------------------
jerror_t JEventProcessor_Pi0Finder::brun(jana::JEventLoop* loop, int32_t runnumber)
{
	// This is called whenever the run number changes

	DApplication *dapp = dynamic_cast<DApplication*>(loop->GetJApplication());
	const DGeometry *geom = dapp->GetDGeometry(runnumber);

	if( geom ) {

		geom->GetTargetZ( m_zTarget );
		geom->GetFCALZ( m_FCALUpstream );
	}
	else{

		cerr << "No geometry accessbile." << endl;
		return RESOURCE_UNAVAILABLE;
	}

	return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t JEventProcessor_Pi0Finder::evnt(jana::JEventLoop* loop, uint64_t evtnumber)
{
	vector<const DAnalysisResults*> analysisResultsVector;
	loop->Get( analysisResultsVector );

	vector<const DFCALShower*> showerVector;
	loop->Get( showerVector );

	vector< const DTrackTimeBased* > allTBTracks;
	loop->Get( allTBTracks );

	vector<const DEventRFBunch*> eventRFBunches;
	loop->Get( eventRFBunches );

	vector< const DTrackWireBased* > allWBTracks;
	loop->Get( allWBTracks );
	vector< const DTrackWireBased* > wbTracks = filterWireBasedTracks( allWBTracks );


	vector<const DFCALCluster*> fcalClusters;
	loop->Get(fcalClusters);

	//	vector<const DParticleCombo*> partCombo;

	//	loop->Get(partCombo);

	//
	//	const DParticleComboStep* initialStep = partCombo->Get_ParticleComboStep( 0 );

	//	DLorentzVector beam = initialStep->Get_InitialParticle_Measured()->lorentzMomentum();
	//TLorentzVector locBeamP4 = dComboBeamWrapper->Get_P4();

	//	m_beamE = beam.E();


	map< const DFCALShower*, double > showerQualityMap;
	vector< const DNeutralShower* > neutralShowers;
	loop->Get( neutralShowers );

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

		if( nCombos != 1 ) return NOERROR;
		deque< const DParticleCombo* > combos;
		(**res).Get_PassedParticleCombos( combos );
		const DParticleCombo* combo = combos[0];

		vector< std::shared_ptr< const DFCALShowerMatchParams > > matchedShowers;

		const DParticleComboStep* initialStep = combo->Get_ParticleComboStep( 0 );
		DLorentzVector beam = initialStep->Get_InitialParticle_Measured()->lorentzMomentum();

		m_beamE = beam.E();

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




		// entering the tree-filling, thread-unsafe, region...


		/*	for( vector< const DFCALCluster* >::const_iterator clItr = fcalClusters.begin();
			clItr != fcalClusters.end();  ++clItr ){
			const DFCALCluster* cluster=*clItr;

			vector< const DFCALHit* > hitVec;
			cluster->Get( hitVec );

			for( vector< const DFCALHit* >::const_iterator hit = hitVec.begin();
			hit != hitVec.end(); ++hit ){

			m_eHitL = (**hit).E;

			}



			}*/







		DVector3 targetCenter( 0, 0, m_zTarget ); 
		m_nTrk = wbTracks.size();
		japp->RootFillLock(this);  


		for( size_t iSh = 0; iSh < fcalClusters.size(); ++iSh ){
			for( size_t jSh = iSh+1; jSh < fcalClusters.size(); ++jSh ){






				const DFCALCluster* shL = ( fcalClusters[iSh]->getEnergy() >
						fcalClusters[jSh]->getEnergy() ?
						fcalClusters[jSh] : fcalClusters[iSh] );

				const DFCALCluster* shH = ( fcalClusters[iSh]->getEnergy() <
						fcalClusters[jSh]->getEnergy() ?
						fcalClusters[jSh] : fcalClusters[iSh] );





				/*	vector< const DFCALHit* > hitVec;
					shL->Get( hitVec );

					for( vector< const DFCALHit* >::const_iterator hit = hitVec.begin();
					hit != hitVec.end(); ++hit ){

					m_eHitL = (**hit).E;

					}

					vector< const DFCALHit* > hitVecH;
					shH->Get(hitVecH);
					for( vector< const DFCALHit* >::const_iterator hit = hitVecH.begin();
					hit != hitVecH.end(); ++hit){



					m_eHitH = (**hit).E;
					}
					*/
					} 


		}


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
				m_xL = posL.X();
				m_yL = posL.Y();
				m_zL = posL.Z();
				m_eL = shL->getEnergy();
				m_qualL = showerQualityMap[shL];
				posL.SetMag( m_eL );
				DLorentzVector p4L( posL, m_eL );
				//--------------------

				m_t0RF = eventRFBunches[0]->dTime;


				m_disShL = ( shL->getPosition() - targetCenter ).Mag();
				m_dtShL = shL->getTime() - m_t0RF;
				m_speedShL = m_disShL/m_dtShL;

				m_e1e9ShL = shL->getE1E9();
				m_e9e25ShL = shL->getE9E25();
				m_sumUShL = shL->getSumU();
				m_sumVShL = shL->getSumV();
				m_asymUVShL = ( m_sumUShL - m_sumVShL ) / ( m_sumUShL + m_sumVShL );
				m_nHitsL = shL->getNumBlocks();
				m_tTrL = shL->getTimeTrack();
				m_tShL = shL->getTime();
				m_dtTrShL = m_dtShL - m_tTrL -0.3;



				m_disShH = ( shH->getPosition() - targetCenter ).Mag();
				m_dtShH = shH->getTime() - m_t0RF;
				m_speedShH = m_disShH/m_dtShH;

				m_e1e9ShH = shH->getE1E9();
				m_e9e25ShH = shH->getE9E25();
				m_sumUShH = shH->getSumU();
				m_sumVShH = shH->getSumV();
				m_asymUVShH = ( m_sumUShH - m_sumVShH ) / ( m_sumUShH + m_sumVShH );
				m_nHitsH = shH->getNumBlocks();
				m_tTrH = shH->getTimeTrack();
				m_tShH = shH->getTime();
				m_dtTrShH = m_dtShH - m_tTrH - 0.3;
				//--------------------
				if( showerMatchIndex.find(shL) != showerMatchIndex.end()){
					m_typeShL =1;
				}
				else if( find(pi0Showers.begin(), pi0Showers.end(), shL) !=pi0Showers.end()){
					m_typeShL=0;
				}
				else{
					m_typeShL=2;
				}




				if( showerMatchIndex.find(shH) != showerMatchIndex.end()){
					m_typeShH =1;
				}
				else if( find(pi0Showers.begin(), pi0Showers.end(), shH) !=pi0Showers.end()){
					m_typeShH=0;
				}
				else{
					m_typeShH=2;
				}

				DVector3 posH = shH->getPosition();
				posH.SetZ( posH.Z() - m_zTarget );
				m_xH = posH.X();
				m_yH = posH.Y();
				m_zH = posH.Z();
				m_eH = shH->getEnergy();
				m_qualH = showerQualityMap[shH];
				posH.SetMag( m_eH );
				DLorentzVector p4H( posH, m_eH );

				m_invM = ( p4L + p4H ).M();

				m_tree->Fill();
			}
		}

		japp->RootFillUnLock(this);

	}
	return NOERROR;
}

//------------------
// erun
//------------------
jerror_t JEventProcessor_Pi0Finder::erun(void)
{
	// This is called whenever the run number changes, before it is
	// changed to give you a chance to clean up before processing
	// events from the next run number.
	return NOERROR;
}

//------------------
// fini
//------------------
jerror_t JEventProcessor_Pi0Finder::fini(void)
{
	// Called before program exit after event processing is finished.
	japp->RootWriteLock();
	m_tree->Write();
	japp->RootUnLock();
	return NOERROR;
}


vector< const DTrackWireBased* >
JEventProcessor_Pi0Finder::filterWireBasedTracks( vector< const DTrackWireBased* >& wbTracks ) const {

	vector< const DTrackWireBased* > finalTracks;
	map< unsigned int, vector< const DTrackWireBased* > > sortedTracks;

	// first sort the wire based tracks into lists with a common candidate id
	// this means that they all come from the same track in the detector

	for( unsigned int i = 0; i < wbTracks.size(); ++i ){

		unsigned int id = wbTracks[i]->candidateid;

		if( sortedTracks.find( id ) == sortedTracks.end() ){

			sortedTracks[id] = vector< const DTrackWireBased* >();
		}

		sortedTracks[id].push_back( wbTracks[i] );
	}

	// now loop through that list of unique tracks and for each set
	// of wire based tracks, choose the one with the highest FOM
	// (this is choosing among different particle hypotheses)

	for( map< unsigned int, vector< const DTrackWireBased* > >::const_iterator
			anId = sortedTracks.begin();
			anId != sortedTracks.end(); ++anId ){

		double maxFOM = 0;
		unsigned int bestIndex = 0;

		for( unsigned int i = 0; i < anId->second.size(); ++i ){

			if( anId->second[i]->Ndof < 15 ) continue;

			if( anId->second[i]->FOM > maxFOM ){

				maxFOM = anId->second[i]->FOM;
				bestIndex = i;
			}
		}

		finalTracks.push_back( anId->second[bestIndex] );
	}

	return finalTracks;
}
