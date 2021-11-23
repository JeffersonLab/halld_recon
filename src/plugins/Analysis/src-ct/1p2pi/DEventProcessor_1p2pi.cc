#include <map>
using namespace std;

#include "DEventProcessor_1p2pi.h"
#include "DFactoryGenerator_1p2pi.h"
#include "D1p2piData.h"

#include <DANA/DApplication.h>



// Routine used to create our DEventProcessor
extern "C"{
  void InitPlugin(JApplication *app){
    InitJANAPlugin(app);
    app->AddProcessor(new DEventProcessor_1p2pi());
	app->AddFactoryGenerator(new DFactoryGenerator_1p2pi()); //register the factory generator
  }
} // "C"

thread_local DTreeFillData DEventProcessor_1p2pi::dTreeFillData;

//------------------
// init
///------------------

jerror_t DEventProcessor_1p2pi::init(void)
{
  //TTREE INTERFACE
  //MUST DELETE WHEN FINISHED: OR ELSE DATA WON'T BE SAVED!!!
  
  string treeName = "tree_1p2pi";
  string treeFile = "tree_1p2pi.root";
  gPARMS->SetDefaultParameter("SRC_1P2PI:TREENAME", treeName);
  gPARMS->SetDefaultParameter("SRC_1P2PI:TREEFILE", treeFile);
  dTreeInterface = DTreeInterface::Create_DTreeInterface(treeName, treeFile);

  //TTREE BRANCHES
  DTreeBranchRegister locTreeBranchRegister;

  locTreeBranchRegister.Register_Single<Int_t>("eventNumber");
  locTreeBranchRegister.Register_Single<Int_t>("L1TriggerBits");

  locTreeBranchRegister.Register_Single<Int_t>("nPhotonCandidates");
  locTreeBranchRegister.Register_FundamentalArray<Double_t>("bmE",    "nPhotonCandidates");
  locTreeBranchRegister.Register_FundamentalArray<Double_t>("bmtime", "nPhotonCandidates");

  //locTreeBranchRegister.Register_FundamentalArray<Double_t>("RFTime_Measured", "RFTime_Measured");

  locTreeBranchRegister.Register_Single<Int_t>("nShower");
  locTreeBranchRegister.Register_Single<Int_t>("nHyp");
  locTreeBranchRegister.Register_FundamentalArray<Double_t>("pX_piminus", "nHyp");
  locTreeBranchRegister.Register_FundamentalArray<Double_t>("pY_piminus", "nHyp");
  locTreeBranchRegister.Register_FundamentalArray<Double_t>("pZ_piminus", "nHyp");
  locTreeBranchRegister.Register_FundamentalArray<Double_t>("E_piminus", "nHyp");
  locTreeBranchRegister.Register_FundamentalArray<Double_t>("t_piminus", "nHyp");

  locTreeBranchRegister.Register_FundamentalArray<Double_t>("pX_piplus", "nHyp");
  locTreeBranchRegister.Register_FundamentalArray<Double_t>("pY_piplus", "nHyp");
  locTreeBranchRegister.Register_FundamentalArray<Double_t>("pZ_piplus", "nHyp");
  locTreeBranchRegister.Register_FundamentalArray<Double_t>("E_piplus", "nHyp");
  locTreeBranchRegister.Register_FundamentalArray<Double_t>("t_piplus", "nHyp");

  locTreeBranchRegister.Register_FundamentalArray<Double_t>("pX_proton", "nHyp");
  locTreeBranchRegister.Register_FundamentalArray<Double_t>("pY_proton", "nHyp");
  locTreeBranchRegister.Register_FundamentalArray<Double_t>("pZ_proton", "nHyp");
  locTreeBranchRegister.Register_FundamentalArray<Double_t>("E_proton", "nHyp");
  locTreeBranchRegister.Register_FundamentalArray<Double_t>("t_proton", "nHyp");

  locTreeBranchRegister.Register_FundamentalArray<Double_t>("X_vertex", "nHyp");
  locTreeBranchRegister.Register_FundamentalArray<Double_t>("Y_vertex", "nHyp");
  locTreeBranchRegister.Register_FundamentalArray<Double_t>("Z_vertex", "nHyp");
  locTreeBranchRegister.Register_FundamentalArray<Double_t>("T_vertex", "nHyp");
  
  locTreeBranchRegister.Register_FundamentalArray<Double_t>("CLKinFit", "nHyp");
  locTreeBranchRegister.Register_FundamentalArray<Double_t>("NDF", "nHyp");
  locTreeBranchRegister.Register_FundamentalArray<Double_t>("ChiSqFit", "nHyp");
  locTreeBranchRegister.Register_FundamentalArray<Double_t>("Common_Time", "nHyp");

  //REGISTER BRANCHES
  dTreeInterface->Create_Branches(locTreeBranchRegister);

  return NOERROR;
}

//------------------ 
// brun
//------------------  
jerror_t DEventProcessor_1p2pi::brun(JEventLoop *eventLoop, int32_t runnumber)
{
  return NOERROR;
}

//------------------
// evnt
//------------------

jerror_t DEventProcessor_1p2pi::evnt(JEventLoop *loop, uint64_t eventnumber)
{

	vector<const D1p2piData *> event_data_vec;
	loop->Get(event_data_vec);
		
	if(event_data_vec.size() == 0)
		return NOERROR;
		
	// fill a tree
	const D1p2piData *event_data = event_data_vec[0];

    dTreeFillData.Fill_Single<Int_t>("nHyp", event_data->nHyp_fitted);

	dTreeFillData.Fill_Single<Int_t>("nPhotonCandidates", event_data->nPhotonCandidates);

	if(event_data->nPhotonCandidates>0){
	 for(Int_t ii = 0; ii < event_data->nPhotonCandidates; ii++){
		dTreeFillData.Fill_Array<Double_t>("bmE", event_data->bmE[ii], ii);
		dTreeFillData.Fill_Array<Double_t>("bmtime", event_data->bmtime[ii], ii);
	 }
	}

	dTreeFillData.Fill_Single<Int_t>("eventNumber", event_data->eventNumber); 
	dTreeFillData.Fill_Single<Double_t>("L1TriggerBits", event_data->L1TriggerBits);


	for(int i=0; i<event_data->nHyp_fitted; i++) {
	
		dTreeFillData.Fill_Array<Double_t>("T_vertex",  event_data->T_vertex[i], i);

		dTreeFillData.Fill_Array<Double_t>("X_vertex", event_data->X_vertex[i], i);
		dTreeFillData.Fill_Array<Double_t>("Y_vertex", event_data->Y_vertex[i], i);
		dTreeFillData.Fill_Array<Double_t>("Z_vertex", event_data->Z_vertex[i], i);
		dTreeFillData.Fill_Array<Double_t>("CLKinFit", event_data->CLKinFit[i], i);
		dTreeFillData.Fill_Array<Double_t>("NDF", event_data->NDF[i], i);
		dTreeFillData.Fill_Array<Double_t>("ChiSqFit", event_data->ChiSqFit[i], i);
		dTreeFillData.Fill_Array<Double_t>("Common_Time", event_data->Common_Time[i], i);

		dTreeFillData.Fill_Array<Double_t>("pX_piminus", event_data->pX_piminus[i],i);
		dTreeFillData.Fill_Array<Double_t>("pY_piminus", event_data->pY_piminus[i],i);
		dTreeFillData.Fill_Array<Double_t>("pZ_piminus", event_data->pZ_piminus[i],i);
		dTreeFillData.Fill_Array<Double_t>("E_piminus",  event_data->E_piminus[i],i);
		dTreeFillData.Fill_Array<Double_t>("t_piminus",  event_data->t_piminus[i],i);

		dTreeFillData.Fill_Array<Double_t>("pX_piplus", event_data->pX_piplus[i],i);
		dTreeFillData.Fill_Array<Double_t>("pY_piplus", event_data->pY_piplus[i],i);
		dTreeFillData.Fill_Array<Double_t>("pZ_piplus", event_data->pZ_piplus[i],i);
		dTreeFillData.Fill_Array<Double_t>("E_piplus",  event_data->E_piplus[i],i);
		dTreeFillData.Fill_Array<Double_t>("t_piplus",  event_data->t_piplus[i],i);

		dTreeFillData.Fill_Array<Double_t>("pX_proton", event_data->pX_proton[i],i);
		dTreeFillData.Fill_Array<Double_t>("pY_proton", event_data->pY_proton[i],i); 
		dTreeFillData.Fill_Array<Double_t>("pZ_proton", event_data->pZ_proton[i],i);
		dTreeFillData.Fill_Array<Double_t>("E_proton",  event_data->E_proton[i],i);
		dTreeFillData.Fill_Array<Double_t>("t_proton",  event_data->t_proton[i],i);

	}

	dTreeInterface->Fill(dTreeFillData);

  return NOERROR;

}

//------------------
jerror_t DEventProcessor_1p2pi::erun(void)
{
	// Any final calculations on histograms (like dividing them)
	// should be done here. This may get called more than once.
	return NOERROR;
}

//------------------
// fini
//------------------

jerror_t DEventProcessor_1p2pi::fini(void)
{
	delete dTreeInterface; //saves trees to file, closes file
	return NOERROR;
}

