// $Id$
//
//    File: DEventWriterROOT_kFitPullStudies.h
// Created: Sa 14. Apr 18:30:24 EDT 2018
// Creator: dlersch (on Linux ifarm1402.jlab.org 3.10.0-327.el7.x86_64 x86_64)
//

#ifndef _DEventWriterROOT_kFitPullStudies_
#define _DEventWriterROOT_kFitPullStudies_

#include <map>
#include <string>

#include <ANALYSIS/DEventWriterROOT.h>

using namespace std;
using namespace jana;

class DEventWriterROOT_kFitPullStudies : public DEventWriterROOT
{
	public:
		virtual ~DEventWriterROOT_kFitPullStudies(void){};
    
    

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
    //Decide whether to store pull-/error matrix- information:
    //==>> This has to be included as option for JANA!
    mutable bool storePullInfo = true;
    mutable bool storeErrMInfo = true; 

    //Stuff to name and fill the branches correctly:
    //Meaning, if is more than one particle of the same species (e.g. gamma),
    //The naming will be consistent with the default naming in the default root-tree,
    //e.g. Photon1__PullPx, Photon2__PullPx, and so on...
    //The same holds for charged particles...
    //****************************************************************************************************************
    vector<string> getPIDinReaction(const DReaction* locReaction)const;
    map<Particle_t,int> getNameForParticle(const DReaction* someReaction, vector<string> someVector)const;
    mutable map<Particle_t,int> abundanceMap;
    string assignName(Particle_t someParticle, map<Particle_t,int> someMap)const;
    //****************************************************************************************************************
    
    int dInitNumComboArraySize = 100;
    mutable map<const DReaction*, bool> writePulls;
    
    //Lets try to get some pulls:
    //****************************************************************************************************************
    mutable map< const JObject*, map<DKinFitPullType, double> > myPullsMap;
    void Get_PullsFromFit(const DParticleCombo* particleCombos)const;
    map<DKinFitPullType, double> getPulls(const JObject* particle, const JObject* shower, DKinFitType yourFitType) const;
    //****************************************************************************************************************
    
    //Cosmetics to set and get the pulls-name/-type
    //****************************************************************************************************************
    //Set the tree branches.
    void setTreePullBranches(DTreeBranchRegister& locBranchRegister,string yourBranchName,DKinFitType yourFitType, int yourNCombos, bool isNeutral) const;
    
    //Fill the branches:
    void fillTreePullBranches(DTreeFillData* locTreeFillData,string yourBranchName,DKinFitType yourFitType,map<DKinFitPullType, double> yourPullsMap, int yourIndex, bool isNeutral) const;
    //****************************************************************************************************************
    
    //Check, if pull-information is available (preserves crashes)
    //****************************************************************************************************************
    void setPullFlag(const DReaction* currentReaction, bool myFlag) const;
    bool getPullFlag(const DReaction* currentReaction) const;
    //****************************************************************************************************************
    
    //Store the error martices for the kinematic fit:
    //****************************************************************************************************************
    int nEntriesParticleCov = 7+6+5+4+3+2+1; //--> (7X7) Particle Matrix is symmetric, therefore no need to look at all elements
    int nEntriesShowerCov = 5+4+3+2+1; //--> (5X5) Shower Matrix is symmetric, therefore no need to look at all elements
    int nEntriesDecayCov = 11+10+9+8+7+6+5+4+3+2+1; //--> (11x11) Decaying Particle Matrix is symmetric, therefore no need to look at all elements
    //****************************************************************************************************************

    //Save the elements of the error matrix to the tree: 
    //****************************************************************************************************************
    void fillTreeErrMBranches(DTreeFillData* locTreeFillData,string yourBranchName,DKinFitType yourFitType, const DKinematicData* particle, const DNeutralShower* shower, bool isNeutral) const;
    //****************************************************************************************************************
    
    //Fill tree for 7x7-Matrices:
    //****************************************************************************************************************
    void fillTreeParticleErrMBranches(DTreeFillData* locTreeFillData,string yourBranchName, const DKinematicData* particle) const;
    //****************************************************************************************************************
    
     //Fill tree for 5x5-Matrices (showers):
    //****************************************************************************************************************
    void fillTreeShowerErrMBranches(DTreeFillData* locTreeFillData,string yourBranchName, const DNeutralShower* shower) const;
    //****************************************************************************************************************


};

#endif //_DEventWriterROOT_kFitPullStudies_
