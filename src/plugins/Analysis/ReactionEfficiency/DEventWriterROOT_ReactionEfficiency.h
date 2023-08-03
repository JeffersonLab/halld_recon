// $Id$
//
//    File: DEventWriterROOT_ReactionEfficiency.h
// Created: Sat Jun 10 10:20:19 EDT 2023
// Creator: jrsteven (on Linux ifarm1901.jlab.org 3.10.0-1160.90.1.el7.x86_64 x86_64)
//

#ifndef _DEventWriterROOT_ReactionEfficiency_
#define _DEventWriterROOT_ReactionEfficiency_

#include <map>
#include <string>

#include <ANALYSIS/DEventWriterROOT.h>

using namespace std;
using namespace jana;

class DEventWriterROOT_ReactionEfficiency : public DEventWriterROOT
{
	public:
		virtual ~DEventWriterROOT_ReactionEfficiency(void){};
		void Run_Update_Custom(JEventLoop* locEventLoop);

	protected:

		//CUSTOM FUNCTIONS: //Inherit from this class and write custom code in these functions

		virtual void Create_CustomBranches_ThrownTree(DTreeBranchRegister& locBranchRegister, JEventLoop* locEventLoop) const;
		virtual void Fill_CustomBranches_ThrownTree(DTreeFillData* locTreeFillData, JEventLoop* locEventLoop, const DMCReaction* locMCReaction, const vector<const DMCThrown*>& locMCThrowns) const;

		virtual void Create_CustomBranches_DataTree(DTreeBranchRegister& locBranchRegister, JEventLoop* locEventLoop, const DReaction* locReaction, bool locIsMCDataFlag) const;
		virtual void Fill_CustomBranches_DataTree(DTreeFillData* locTreeFillData, JEventLoop* locEventLoop, const DReaction* locReaction, const DMCReaction* locMCReaction, const vector<const DMCThrown*>& locMCThrowns,
				const DMCThrownMatching* locMCThrownMatching, const DDetectorMatches* locDetectorMatches,
				const vector<const DBeamPhoton*>& locBeamPhotons, const vector<const DChargedTrackHypothesis*>& locChargedHypos,
				const vector<const DNeutralParticleHypothesis*>& locNeutralHypos, const deque<const DParticleCombo*>& locParticleCombos) const;		

	private:

		const DParticleID* dParticleID;
		const DMagneticFieldMap *bfield;
		DReferenceTrajectory *rt;
};

#endif //_DEventWriterROOT_ReactionEfficiency_
