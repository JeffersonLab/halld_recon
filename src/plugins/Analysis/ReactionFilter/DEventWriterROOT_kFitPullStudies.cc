// $Id$
//
//    File: DEventWriterROOT_kFitPullStudies.cc
// Created: Sa 14. Apr 18:30:24 EDT 2018
// Creator: dlersch (on Linux ifarm1402.jlab.org 3.10.0-327.el7.x86_64 x86_64)
//

#include "DEventWriterROOT_kFitPullStudies.h"

//GLUEX TTREE DOCUMENTATION: https://halldweb.jlab.org/wiki/index.php/Analysis_TTreeFormat

void DEventWriterROOT_kFitPullStudies::Create_CustomBranches_DataTree(DTreeBranchRegister& locBranchRegister, JEventLoop* locEventLoop, const DReaction* locReaction, bool locIsMCDataFlag) const
{
	if(storeErrMInfo || storePullInfo){
	
    //Get the fit-type (P4, P4 + Vertex, and Vertex-only)
    DKinFitType kfitType = locReaction->Get_KinFitType();
    
    vector<Particle_t> finalchargedPIDs,finalneutralPIDs;
    size_t nReactionSteps = locReaction->Get_NumReactionSteps();
    int nFinalChargedParticles,nFinalNeutralParticles;
    
    vector<string> pidVector = getPIDinReaction(locReaction);
    map<Particle_t,int> assignMap = getNameForParticle(locReaction,pidVector);
     
    //Check, if at least two charged particles are present in the final state: (important for vertex-fit)
    int sumAllChargedParticles = 0;
    int sumAllNeutralParticles = 0;
    //First, check how many neutral/charged particles are in the final stage
    //The pull-information is avaiablable for certain particle combinations (charged + neutral) only:
    //----------------------------------------------------------------------------------------------
    for(size_t loc_i=0;loc_i<nReactionSteps;loc_i++){
        
        //Charged Particles:
        finalchargedPIDs = locReaction->Get_ReactionStep(loc_i)->Get_FinalPIDs(true,d_Charged,true);
        nFinalChargedParticles = finalchargedPIDs.size();
        sumAllChargedParticles += nFinalChargedParticles;
        
         //Neutral Particles:
        finalneutralPIDs = locReaction->Get_ReactionStep(loc_i)->Get_FinalPIDs(true,d_Neutral,true);
        nFinalNeutralParticles = finalneutralPIDs.size();
        sumAllNeutralParticles += nFinalNeutralParticles;
    }
    //----------------------------------------------------------------------------------------------
    
    bool myFlag = true;
    if(sumAllChargedParticles == 1 && (kfitType == d_VertexFit || kfitType == d_P4AndVertexFit)){
        myFlag = false;
        cout <<"  "<< endl;
        cout <<">>> WARNING: Only one charged track in the final state! No vertex fit possible! No pulls will be stored. Have a nice day! <<<"<< endl;
        cout <<"  "<< endl;
    }
    
    if(sumAllNeutralParticles > 0 && kfitType == d_VertexFit){
        myFlag = false;
        cout <<"  "<< endl;
        cout <<">>> WARNING: Neutral particles involved in pure vertex fit! No pulls will be stored. Have a nice day! <<<"<< endl;
        cout <<"  "<< endl;
    }
    
    if(kfitType == d_NoFit){
		myFlag = false;
        cout <<"  "<< endl;
        cout <<">>> WARNING: No fit specified! No pulls will be stored. Have a nice day! <<<"<< endl;
        cout <<"  "<< endl;
	}

    setPullFlag(locReaction,myFlag);
		
    //Get Pulls for the beam:
    Particle_t beamPID = locReaction->Get_ReactionStep(0)->Get_InitialPID();
    
    string particleCovM = "numEntries_ParticleErrM";
    string showerCovM = "numEntries_ShowerErrM";
    string decayCovM = "numEntries_DecayErrM";
    if(storeErrMInfo){
       locBranchRegister.Register_Single<Int_t>(particleCovM);
       locBranchRegister.Register_Single<Int_t>(showerCovM); 
    }
    
    //Set the branches for the beam photon:
    if(myFlag)setTreePullBranches(locBranchRegister,"ComboBeam",kfitType,dInitNumComboArraySize,true);
    if(storeErrMInfo)locBranchRegister.Register_FundamentalArray< Float_t >(Build_BranchName("ComboBeam","ErrMatrix"),particleCovM,nEntriesParticleCov);
		
    //----------------------------------------------------------------------------------------------
    for(size_t loc_i=0;loc_i<nReactionSteps;loc_i++){
        
        //Charged Particles:
        finalchargedPIDs = locReaction->Get_ReactionStep(loc_i)->Get_FinalPIDs(true,d_Charged,true);
        nFinalChargedParticles = finalchargedPIDs.size();
        //----------------------------------------------------------------------------------------------
        for(int loc_j=0;loc_j<nFinalChargedParticles;loc_j++){
            //Define branches for final charged particles:
            string branchName = assignName(finalchargedPIDs.at(loc_j),assignMap);
            assignMap[finalchargedPIDs.at(loc_j)]--;
            if(branchName != "nada"){
                  if(myFlag)setTreePullBranches(locBranchRegister,branchName,kfitType,dInitNumComboArraySize,false);
                   if(storeErrMInfo)locBranchRegister.Register_FundamentalArray< Float_t >(Build_BranchName(branchName,"ErrMatrix"),particleCovM,nEntriesParticleCov);
            }
        }
        //----------------------------------------------------------------------------------------------
        
        //Neutral Particles:
        finalneutralPIDs = locReaction->Get_ReactionStep(loc_i)->Get_FinalPIDs(true,d_Neutral,true);
        nFinalNeutralParticles = finalneutralPIDs.size();
        //----------------------------------------------------------------------------------------------
        for(int loc_j=0;loc_j<nFinalNeutralParticles;loc_j++){
            //Define branches for final neutral particles:
            string branchName = assignName(finalneutralPIDs.at(loc_j),assignMap);
            assignMap[finalneutralPIDs.at(loc_j)]--;
            if(branchName != "nada"){
                if(myFlag)setTreePullBranches(locBranchRegister,branchName,kfitType,dInitNumComboArraySize,true);
                if(finalneutralPIDs.at(loc_j) == Gamma && storeErrMInfo)locBranchRegister.Register_FundamentalArray< Float_t >(Build_BranchName(branchName,"ErrMatrix"),showerCovM,nEntriesShowerCov);
            } 
         }
        //----------------------------------------------------------------------------------------------
     }
    //----------------------------------------------------------------------------------------------
    
    assignMap.clear();
    abundanceMap.clear();
    }
}

void DEventWriterROOT_kFitPullStudies::Create_CustomBranches_ThrownTree(DTreeBranchRegister& locBranchRegister, JEventLoop* locEventLoop) const
{
	//EXAMPLES: See Create_CustomBranches_DataTree
}

void DEventWriterROOT_kFitPullStudies::Fill_CustomBranches_DataTree(DTreeFillData* locTreeFillData, JEventLoop* locEventLoop, const DReaction* locReaction, const DMCReaction* locMCReaction, const vector<const DMCThrown*>& locMCThrowns,
	const DMCThrownMatching* locMCThrownMatching, const DDetectorMatches* locDetectorMatches,
	const vector<const DBeamPhoton*>& locBeamPhotons, const vector<const DChargedTrackHypothesis*>& locChargedHypos,
	const vector<const DNeutralParticleHypothesis*>& locNeutralHypos, const deque<const DParticleCombo*>& locParticleCombos) const
{
	if(storeErrMInfo || storePullInfo){
	
	locTreeFillData->Fill_Single<Int_t>("numEntries_ParticleErrM", nEntriesParticleCov);
	locTreeFillData->Fill_Single<Int_t>("numEntries_ShowerErrM", nEntriesShowerCov);
     
    bool writeOutPulls =  getPullFlag( locReaction );
    //Retreive and fill pull-information:
    //######################################################################################################################
    DKinFitType kfitType = locReaction->Get_KinFitType();
    
    //Now fill the pulls for all particles:
    int nParticleCombos = locParticleCombos.size();
    int nMeasuredFinalParticles;
    double currentCharge;
    Particle_t currentPID;

    //----------------------------------------------------------------------------
    for(int iPC=0;iPC<nParticleCombos;iPC++){
        Get_PullsFromFit(locParticleCombos.at(iPC));
        
        vector<string> pidVector = getPIDinReaction(locReaction);
        map<Particle_t,int> assignMap = getNameForParticle(locReaction,pidVector);
        
        
        //Look at beam photons:
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        const DParticleComboStep* locParticleComboStep = locParticleCombos[iPC]->Get_ParticleComboStep(0);
        const DKinematicData* beamPhoton = locParticleComboStep->Get_InitialParticle_Measured();
        
        if(kfitType == d_P4Fit || kfitType == d_P4AndVertexFit){
		  if(writeOutPulls){	
             map<DKinFitPullType, double> someBeamMap = getPulls(beamPhoton,NULL,kfitType);
             fillTreePullBranches(locTreeFillData,"ComboBeam",kfitType,someBeamMap,iPC,true);
             someBeamMap.clear();
	      }
	    }
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        
        //Look at the decay products:
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        vector<const DKinematicData*> finalParticlesMeasured = locParticleCombos.at(iPC)->Get_FinalParticles_Measured(locReaction, d_AllCharges);
        vector<const JObject*> finalParticleObjects = locParticleCombos.at(iPC)->Get_FinalParticle_SourceObjects(d_AllCharges);
        nMeasuredFinalParticles = finalParticlesMeasured.size();
        
        //----------------------------------------------------------------------------
        for(int iFP=0;iFP<nMeasuredFinalParticles;iFP++){
            const DKinematicData* part = finalParticlesMeasured.at(iFP);
            const  DNeutralShower* sh = dynamic_cast<const DNeutralShower*>(finalParticleObjects.at(iFP));
            bool isNeutral = false;
            
            map<DKinFitPullType, double>  someMap;
            if(writeOutPulls) someMap = getPulls(part,sh,kfitType);
            //-----------------------------------------------------
            if(sh == NULL){
				currentCharge = part->charge();
                currentPID = part->PID();
			}else{
				currentCharge = 0.0;
                currentPID = Gamma;
		    }
            
            if(currentCharge == 0) isNeutral = true;
            //-----------------------------------------------------
            
            string branchName = assignName(currentPID,assignMap);
            assignMap[currentPID]--;
            //-----------------------------------------------------
            if(branchName != "nada"){
                if(writeOutPulls)fillTreePullBranches(locTreeFillData,branchName,kfitType,someMap,iPC,isNeutral);
                fillTreeErrMBranches(locTreeFillData,branchName,kfitType,part,sh,isNeutral);
            }
            //-----------------------------------------------------
            if(writeOutPulls)someMap.clear();
		  }
        //----------------------------------------------------------------------------
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        
        myPullsMap.clear();
        assignMap.clear();
        abundanceMap.clear();
    }
    //----------------------------------------------------------------------------
    //######################################################################################################################    
    }
}

void DEventWriterROOT_kFitPullStudies::Fill_CustomBranches_ThrownTree(DTreeFillData* locTreeFillData, JEventLoop* locEventLoop, const DMCReaction* locMCReaction, const vector<const DMCThrown*>& locMCThrowns) const
{
	//EXAMPLES: See Fill_CustomBranches_DataTree
}


//I guess this is the most clumsiest way of assigning a few names to a tree-branch, but it is doing what it is supposed to do...
//*************************************************************************************************************************************
vector<string> DEventWriterROOT_kFitPullStudies::getPIDinReaction(const DReaction* locReaction)const
{
    vector <string> myVector;
    vector<Particle_t> particle = locReaction->Get_FinalPIDs(-1,false,false,d_AllCharges,true);
    int nParticleIDs = particle.size();
    //----------------------------------------------------
    for(int k=0;k<nParticleIDs;k++){
        myVector.push_back(EnumString(particle.at(k)));
    }
    //----------------------------------------------------
    return myVector;
}

//===================================================================

map<Particle_t, int> DEventWriterROOT_kFitPullStudies::getNameForParticle(const DReaction* someReaction, vector<string> someVector)const
{
    map <Particle_t,int> myMap;
    vector<Particle_t> particle = someReaction->Get_FinalPIDs(-1,false,false,d_AllCharges,true);
    int nParticleIDs = particle.size();
    int currentCounter = 0;
    int nEl = someVector.size();
    //----------------------------------------------------
    for(int k=0;k<nParticleIDs;k++){
        currentCounter = 0;
        
        //----------------------------------------------------
        for(int j=0;j<nEl;j++){
            if(EnumString(particle.at(k)) == someVector.at(j)){
                currentCounter++;
                someVector.at(j) = "";
            }
        }
        //----------------------------------------------------
        if(currentCounter > 0){
            abundanceMap[particle.at(k)] = currentCounter;
            myMap[particle.at(k)] = currentCounter;
        }
    }
    //----------------------------------------------------
    return myMap;
}

//===================================================================

string DEventWriterROOT_kFitPullStudies::assignName(Particle_t someParticle, map<Particle_t,int> someMap)const
{
    string myName = "nada";
    int abundance = abundanceMap.find(someParticle)->second;
    if(abundance == 1){
        myName = EnumString(someParticle);
    }else if(abundance > 1){
        int index = abundance - someMap.find(someParticle)->second +1;
        string addName;
        ostringstream convert;
        convert << index;
        addName = convert.str();
        if(EnumString(someParticle) == "Gamma"){
            myName = "Photon" + addName;
        }else myName = EnumString(someParticle) + addName;
    }
    
    return myName;
}

//*************************************************************************************************************************************



//*************************************************************************************************************************************
void DEventWriterROOT_kFitPullStudies::Get_PullsFromFit(const DParticleCombo* particleCombos) const
{
    //-----------------------------------------------------------------------------
    const DKinFitResults* fitResults = particleCombos->Get_KinFitResults();
    if(fitResults != NULL){
        fitResults->Get_Pulls(myPullsMap);
    }
    //-----------------------------------------------------------------------------
}

//===================================================================

map<DKinFitPullType, double> DEventWriterROOT_kFitPullStudies::getPulls(const JObject* particle, const JObject* shower, DKinFitType yourFitType) const{
	map<DKinFitPullType, double> myMap;
	
	if(particle != NULL || shower != NULL){
	
	   //----------------------------------
	   if(yourFitType == d_P4Fit || yourFitType == d_VertexFit){
		   myMap = myPullsMap.find(particle)->second;
	   }else if(yourFitType == d_P4AndVertexFit){ //Option noFit is not considered
		   //--------------------
	       if(shower == NULL){
			   myMap = myPullsMap.find(particle)->second;
		   }else myMap = myPullsMap.find(shower)->second; 	
		   //--------------------
	   }
	   //----------------------------------	 
    }
	
	return myMap;
}
//*************************************************************************************************************************************


//Get the pull-features:
//*************************************************************************************************************************************
void DEventWriterROOT_kFitPullStudies::setTreePullBranches(DTreeBranchRegister& locBranchRegister,string yourBranchName,DKinFitType yourFitType, int yourNCombos, bool isNeutral) const
{
    string locArraySizeString = "NumCombos";
    
    if(yourFitType == d_P4Fit){
        locBranchRegister.Register_FundamentalArray<Double_t>(Build_BranchName(yourBranchName,"Px_Pull"),locArraySizeString, yourNCombos);
        locBranchRegister.Register_FundamentalArray<Double_t>(Build_BranchName(yourBranchName,"Py_Pull"),locArraySizeString, yourNCombos);
        locBranchRegister.Register_FundamentalArray<Double_t>(Build_BranchName(yourBranchName,"Pz_Pull"),locArraySizeString, yourNCombos);
    }else if(yourFitType == d_P4AndVertexFit){
        //------------------------------------------------------------
        if(isNeutral){
            locBranchRegister.Register_FundamentalArray<Double_t>(Build_BranchName(yourBranchName,"E_Pull"),locArraySizeString, yourNCombos);
            locBranchRegister.Register_FundamentalArray<Double_t>(Build_BranchName(yourBranchName,"Xx_Pull"),locArraySizeString, yourNCombos);
            locBranchRegister.Register_FundamentalArray<Double_t>(Build_BranchName(yourBranchName,"Xy_Pull"),locArraySizeString, yourNCombos);
            locBranchRegister.Register_FundamentalArray<Double_t>(Build_BranchName(yourBranchName,"Xz_Pull"),locArraySizeString, yourNCombos);
        }else{
            locBranchRegister.Register_FundamentalArray<Double_t>(Build_BranchName(yourBranchName,"Px_Pull"),locArraySizeString, yourNCombos);
            locBranchRegister.Register_FundamentalArray<Double_t>(Build_BranchName(yourBranchName,"Py_Pull"),locArraySizeString, yourNCombos);
            locBranchRegister.Register_FundamentalArray<Double_t>(Build_BranchName(yourBranchName,"Pz_Pull"),locArraySizeString, yourNCombos);
            locBranchRegister.Register_FundamentalArray<Double_t>(Build_BranchName(yourBranchName,"Xx_Pull"),locArraySizeString, yourNCombos);
            locBranchRegister.Register_FundamentalArray<Double_t>(Build_BranchName(yourBranchName,"Xy_Pull"),locArraySizeString, yourNCombos);
            locBranchRegister.Register_FundamentalArray<Double_t>(Build_BranchName(yourBranchName,"Xz_Pull"),locArraySizeString, yourNCombos);
        }
        //------------------------------------------------------------
    }else if(yourFitType == d_VertexFit && !isNeutral){
        locBranchRegister.Register_FundamentalArray<Double_t>(Build_BranchName(yourBranchName,"Xx_Pull"),locArraySizeString, yourNCombos);
        locBranchRegister.Register_FundamentalArray<Double_t>(Build_BranchName(yourBranchName,"Xy_Pull"),locArraySizeString, yourNCombos);
        locBranchRegister.Register_FundamentalArray<Double_t>(Build_BranchName(yourBranchName,"Xz_Pull"),locArraySizeString, yourNCombos);
    }
}
//*************************************************************************************************************************************

//Fill the tree branches:
//*************************************************************************************************************************************
void DEventWriterROOT_kFitPullStudies::fillTreePullBranches(DTreeFillData* locTreeFillData,string yourBranchName,DKinFitType yourFitType,map<DKinFitPullType, double> yourPullsMap, int yourIndex, bool isNeutral) const
{
    if(yourFitType == d_P4Fit){
        locTreeFillData->Fill_Array<Double_t>(Build_BranchName(yourBranchName, "Px_Pull"), yourPullsMap.find(d_PxPull)->second,yourIndex);
        locTreeFillData->Fill_Array<Double_t>(Build_BranchName(yourBranchName, "Py_Pull"), yourPullsMap.find(d_PyPull)->second,yourIndex);
        locTreeFillData->Fill_Array<Double_t>(Build_BranchName(yourBranchName, "Pz_Pull"), yourPullsMap.find(d_PzPull)->second,yourIndex);
    }else if(yourFitType == d_P4AndVertexFit){
        //------------------------------------------------------------
        if(isNeutral){
            locTreeFillData->Fill_Array<Double_t>(Build_BranchName(yourBranchName, "E_Pull"), yourPullsMap.find(d_EPull)->second,yourIndex);
            locTreeFillData->Fill_Array<Double_t>(Build_BranchName(yourBranchName, "Xx_Pull"), yourPullsMap.find(d_XxPull)->second,yourIndex);
            locTreeFillData->Fill_Array<Double_t>(Build_BranchName(yourBranchName, "Xy_Pull"), yourPullsMap.find(d_XyPull)->second,yourIndex);
            locTreeFillData->Fill_Array<Double_t>(Build_BranchName(yourBranchName, "Xz_Pull"), yourPullsMap.find(d_XzPull)->second,yourIndex);
        }else{
            locTreeFillData->Fill_Array<Double_t>(Build_BranchName(yourBranchName, "Px_Pull"), yourPullsMap.find(d_PxPull)->second,yourIndex);
            locTreeFillData->Fill_Array<Double_t>(Build_BranchName(yourBranchName, "Py_Pull"), yourPullsMap.find(d_PyPull)->second,yourIndex);
            locTreeFillData->Fill_Array<Double_t>(Build_BranchName(yourBranchName, "Pz_Pull"), yourPullsMap.find(d_PzPull)->second,yourIndex);
            locTreeFillData->Fill_Array<Double_t>(Build_BranchName(yourBranchName, "Xx_Pull"), yourPullsMap.find(d_XxPull)->second,yourIndex);
            locTreeFillData->Fill_Array<Double_t>(Build_BranchName(yourBranchName, "Xy_Pull"), yourPullsMap.find(d_XyPull)->second,yourIndex);
            locTreeFillData->Fill_Array<Double_t>(Build_BranchName(yourBranchName, "Xz_Pull"), yourPullsMap.find(d_XzPull)->second,yourIndex);
        }
        //------------------------------------------------------------
    }else if(yourFitType == d_VertexFit && !isNeutral){
        locTreeFillData->Fill_Array<Double_t>(Build_BranchName(yourBranchName, "Xx_Pull"), yourPullsMap.find(d_XxPull)->second,yourIndex);
        locTreeFillData->Fill_Array<Double_t>(Build_BranchName(yourBranchName, "Xy_Pull"), yourPullsMap.find(d_XyPull)->second,yourIndex);
        locTreeFillData->Fill_Array<Double_t>(Build_BranchName(yourBranchName, "Xz_Pull"), yourPullsMap.find(d_XzPull)->second,yourIndex);
    }
}
//*************************************************************************************************************************************

//Make sure, the pull information is written out, when available
//*********************************************************************************************************************
void DEventWriterROOT_kFitPullStudies::setPullFlag(const DReaction* currentReaction, bool myFlag) const
{
    writePulls[currentReaction] = myFlag;
}

bool DEventWriterROOT_kFitPullStudies::getPullFlag(const DReaction* currentReaction) const
{
    bool outFlag = writePulls.find(currentReaction)->second;
    return outFlag;
}
//*********************************************************************************************************************



//Now fill the individual elements of the error matrix to the tree:
//*********************************************************************************************************************
void DEventWriterROOT_kFitPullStudies::fillTreeErrMBranches(DTreeFillData* locTreeFillData,string yourBranchName,DKinFitType yourFitType, const DKinematicData* particle, const DNeutralShower* shower, bool isNeutral) const
{
    if(yourFitType == d_P4Fit){
		fillTreeParticleErrMBranches(locTreeFillData,yourBranchName,particle);
	}else if(yourFitType == d_P4AndVertexFit){
		if(isNeutral){
			fillTreeShowerErrMBranches(locTreeFillData,yourBranchName,shower);
		}else fillTreeParticleErrMBranches(locTreeFillData,yourBranchName,particle);
	}else if(yourFitType == d_VertexFit && !isNeutral){
		fillTreeParticleErrMBranches(locTreeFillData,yourBranchName,particle);
	}     	
}
//*********************************************************************************************************************

//Fill the 7x7 error matrix elements:
//*********************************************************************************************************************
void DEventWriterROOT_kFitPullStudies::fillTreeParticleErrMBranches(DTreeFillData* locTreeFillData,string yourBranchName, const DKinematicData* particle) const
{
	if(particle != NULL){	
	   const TMatrixTSym<float> errMatrix = *( particle->errorMatrix() );
	
	   int mIndex = 0;
	   float matrixEl = 0.0;
	   //--------------------------------------
	   for(int row=0;row<7;row++){
		   //--------------------------------------
		   for(int col=0;col<7;col++){
			    if(col >= row){
					matrixEl = errMatrix[row][col];
					locTreeFillData->Fill_Array<Float_t>(Build_BranchName(yourBranchName, "ErrMatrix"),matrixEl,mIndex);
					mIndex++;
				}
		   }
		   //--------------------------------------
	   }
	   //--------------------------------------
    }
}
//*********************************************************************************************************************


//Fill the 5x5 error matrix elements:
//*********************************************************************************************************************
void DEventWriterROOT_kFitPullStudies::fillTreeShowerErrMBranches(DTreeFillData* locTreeFillData,string yourBranchName, const DNeutralShower* shower) const
{
	if(shower != NULL){
	   const TMatrixTSym <float> errMatrix = *( shower->dCovarianceMatrix );
	   float matrixEl = 0.0;
	   
	   int mIndex = 0;
	   //--------------------------------------
	   for(int row=0;row<5;row++){
		   //--------------------------------------
		   for(int col=0;col<5;col++){
			    if(col >= row){
				   matrixEl = errMatrix[row][col];
			       locTreeFillData->Fill_Array<Float_t>(Build_BranchName(yourBranchName, "ErrMatrix"),matrixEl,mIndex);
				   mIndex++;
				}
		   }
		   //--------------------------------------
	   }
	   //--------------------------------------
    }
}
//*********************************************************************************************************************


