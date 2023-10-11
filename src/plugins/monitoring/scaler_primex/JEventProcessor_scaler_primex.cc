// $Id$
//
//    File: JEventProcessor_scaler_primex.cc
//        A.S.
// 
//

#include <iostream>
#include <sstream>

#include "JEventProcessor_scaler_primex.h"


using namespace std;


#include <TRIGGER/DL1Trigger.h>

#include <DAQ/Df250Scaler.h>

static TH1D  *htrig_bit;

// Routine used to create our JEventProcessor
extern "C"{
  void InitPlugin(JApplication *app){
    

    InitJANAPlugin(app);
    
    
    app->Add(new JEventProcessor_scaler_primex());
	
  }
} // "C"


const int max_nST       =   32;
const int max_nTOF      =   200;
const int max_nPSA      =   200;
const int max_nPSB      =   200;
const int max_nPSCA     =   10;
const int max_nPSCB     =   10;
const int max_nCCAL     =   200;
const int max_nTAGH     =   300;
const int max_nTAGM     =   150;


//define static local variable //declared in header file
thread_local DTreeFillData JEventProcessor_scaler_primex::dTreeFillData;


//------------------
// init
//------------------
void JEventProcessor_scaler_primex::Init()
{


  TDirectory* locOriginalDir = gDirectory;
  TDirectory *dscaler_primex = gDirectory->mkdir("scaler_primex_hist");
  dscaler_primex->cd();

  htrig_bit         =   new TH1D("trig_bit","trig_bit",   100, -0.5, 99.5);

  // back to original dir

  //  gDirectory->cd("../");

  locOriginalDir->cd();

  //TTREE INTERFACE
  //MUST DELETE WHEN FINISHED: OR ELSE DATA WON'T BE SAVED!!!
  dTreeInterface = DTreeInterface::Create_DTreeInterface("scaler", "tree_scaler_primex.root");

  //TTREE BRANCHES


  DTreeBranchRegister locTreeBranchRegister;

  locTreeBranchRegister.Register_Single<uint32_t>("nSYNC");
  locTreeBranchRegister.Register_Single<uint32_t>("nTRIG");

  locTreeBranchRegister.Register_Single<Double_t>("trig_rate_bit0");
  locTreeBranchRegister.Register_Single<Double_t>("trig_rate_bit1");
  locTreeBranchRegister.Register_Single<Double_t>("trig_rate_bit3");
  locTreeBranchRegister.Register_Single<Double_t>("trig_rate_bit6");
  locTreeBranchRegister.Register_Single<Double_t>("trig_rate_bit7");
  locTreeBranchRegister.Register_Single<Double_t>("trig_rate_bit8");
  locTreeBranchRegister.Register_Single<Double_t>("trig_rate_bit10");


  locTreeBranchRegister.Register_Single<Int_t>("nST");
  locTreeBranchRegister.Register_FundamentalArray<Double_t>("ST_rate", "nST", max_nST);

  locTreeBranchRegister.Register_Single<Int_t>("nTOF");
  locTreeBranchRegister.Register_FundamentalArray<Double_t>("TOF_rate", "nTOF", max_nTOF);

  locTreeBranchRegister.Register_Single<Int_t>("nTAGH");
  locTreeBranchRegister.Register_FundamentalArray<Double_t>("TAGH_rate", "nTAGH", max_nTAGH);

  locTreeBranchRegister.Register_Single<Int_t>("nTAGM");
  locTreeBranchRegister.Register_FundamentalArray<Double_t>("TAGM_rate", "nTAGM", max_nTAGM);

  locTreeBranchRegister.Register_Single<Int_t>("nCCAL");
  locTreeBranchRegister.Register_FundamentalArray<Double_t>("CCAL_rate", "nCCAL", max_nCCAL);

  locTreeBranchRegister.Register_Single<Int_t>("nPSA");
  locTreeBranchRegister.Register_FundamentalArray<Double_t>("PSA_rate", "nPSA", max_nPSA);

  locTreeBranchRegister.Register_Single<Int_t>("nPSB");
  locTreeBranchRegister.Register_FundamentalArray<Double_t>("PSB_rate", "nPSB", max_nPSB);

  locTreeBranchRegister.Register_Single<Int_t>("nPSCA");
  locTreeBranchRegister.Register_FundamentalArray<Double_t>("PSCA_rate", "nPSCA", max_nPSCA);

  locTreeBranchRegister.Register_Single<Int_t>("nPSCB");
  locTreeBranchRegister.Register_FundamentalArray<Double_t>("PSCB_rate", "nPSCB", max_nPSCB);

  //REGISTER BRANCHES
  dTreeInterface->Create_Branches(locTreeBranchRegister);

}

//------------------
// brun
//------------------
void JEventProcessor_scaler_primex::BeginRun(const std::shared_ptr<const JEvent> &event)
{
}

//------------------
// evnt
//------------------
void JEventProcessor_scaler_primex::Process(const std::shared_ptr<const JEvent> &event)
{

        bool is_sync_event = false;



        int trig_bit[33];

	int nhit_ST    =  0;
	int nhit_TAGH  =  0;
	int nhit_TAGM  =  0;
	int nhit_PSA   =  0;
	int nhit_PSB   =  0;
	int nhit_PSCA  =  0;
	int nhit_PSCB  =  0;
	int nhit_TOF   =  0;
	int nhit_CCAL  =  0;

	uint32_t nsync_event = 0;
	uint32_t trig_number = 0;

	Double_t trig_rate[11];

	int slot = 0;
	int ch   = 0;

	memset(trig_bit,0,sizeof(trig_bit));
	memset(trig_rate,0,sizeof(trig_rate));


	// TRIGGER
	vector<const DL1Trigger*> l1trig;

	// SCALERS
	vector<const Df250Scaler*> fa250_sc;
	
	event->Get(l1trig);

	event->Get(fa250_sc);
      


	lockService->RootFillLock(this);
	
	
        if( l1trig.size() > 0){
	  	  
	  for(unsigned int bit = 0; bit < 32; bit++){	    
	    
	    trig_bit[bit + 1] = (l1trig[0]->trig_mask & (1 << bit)) ? 1 : 0;
       	    
	    if(trig_bit[bit + 1] == 1) htrig_bit->Fill(Float_t(bit+1)); 
	  }

	  if(l1trig[0]->gtp_rate.size() > 0){
	    
	    for(unsigned int bit = 0; bit <= 10; bit++)

	      trig_rate[bit] = l1trig[0]->gtp_rate[bit]/1000.;
	      	    	   

	    
	  }

	}


	
	if(fa250_sc.size() > 0){
	  
	  
	  if(fa250_sc.size() > 0){
	    nsync_event  =  fa250_sc[0]->nsync;
	    trig_number  =  fa250_sc[0]->trig_number;
	  }

          for(unsigned int ii = 0; ii < fa250_sc.size(); ii++){
	    
            const Df250Scaler *sc  =  fa250_sc[ii];

           // PS
            if(sc->crate == 83){              

	      slot  = 0;

              for(unsigned int jj = 0; jj < sc->fa250_sc.size(); jj++){                         

		// Skip words containing FADC slot
		if( (jj % 17) == 0){
		  slot = sc->fa250_sc[jj] & 0xFFF;
		  continue;
                }
		
		// Arm A
		if(slot < 11){
		  if(nhit_PSA < max_nPSA){
		    double scaler = sc->fa250_sc[jj]/1000.;
		    dTreeFillData.Fill_Array<Double_t>("PSA_rate", scaler, nhit_PSA);
		    nhit_PSA++;   
		  }    
		}
		
		// Arm B
		if(slot > 11){
		  if(nhit_PSB < max_nPSB){
		    double scaler = sc->fa250_sc[jj]/1000.;
		    dTreeFillData.Fill_Array<Double_t>("PSB_rate", scaler, nhit_PSB);
		    nhit_PSB++;   
		  }    
		}	
	      }  
            }  // PS
	    
	    // Continue with PS and PSC

            if(sc->crate == 84){              
	      
	      slot = 0;
	      ch = 0;

              for(unsigned int jj = 0; jj < sc->fa250_sc.size(); jj++){                         

		if( (jj % 17) == 0){
		  slot = sc->fa250_sc[jj] & 0xFFF;
		  ch = 0;
		  continue;
                }

		// PS Arm A
		if(slot == 3 || (slot == 5 && ch == 0)){
		  if(nhit_PSA < max_nPSA){
		    double scaler = sc->fa250_sc[jj]/1000.;
		    dTreeFillData.Fill_Array<Double_t>("PSA_rate", scaler, nhit_PSA);
		    nhit_PSA++;   
		  }    
		}
		
		// PS Arm B
		if(slot == 4 || (slot == 5 && ch == 1)){
		  if(nhit_PSB < max_nPSB){
		    double scaler = sc->fa250_sc[jj]/1000.;
		    dTreeFillData.Fill_Array<Double_t>("PSB_rate", scaler, nhit_PSB);
		    nhit_PSB++;   
		  }    
		}
		
		
		// PS Arm A
		if(slot == 6 && ch < 8){
		  if(nhit_PSCA < max_nPSCA){
		    double scaler = sc->fa250_sc[jj]/1000.;
		    dTreeFillData.Fill_Array<Double_t>("PSCA_rate", scaler, nhit_PSCA);
		    nhit_PSCA++;   
		  }    
		}

		// PS Arm A
		if(slot == 6 && ch >= 8){
		  if(nhit_PSCB < max_nPSCB){
		    double scaler = sc->fa250_sc[jj]/1000.;
		    dTreeFillData.Fill_Array<Double_t>("PSCB_rate", scaler, nhit_PSCB);
		    nhit_PSCB++;   
		  }    
		}


		ch++;


              }  
	    }  // PS and PSC



	    // ST
            if(sc->crate == 94){              
              for(unsigned int jj = 0; jj < sc->fa250_sc.size(); jj++){                         
		
                if(nhit_ST >= max_nST) break;               
                if( (jj % 17) == 0) continue;
                
                double scaler = sc->fa250_sc[jj]/1000.;
		
                dTreeFillData.Fill_Array<Double_t>("ST_rate", scaler, nhit_ST);
                nhit_ST++;          
              }  
            }  // ST



	    if(sc->crate == 77){              
              for(unsigned int jj = 0; jj < sc->fa250_sc.size(); jj++){                         
		
                if(nhit_TOF >= max_nTOF) break;             
                if( (jj % 17) == 0) continue;
                
                double scaler = sc->fa250_sc[jj]/1000.;
                
                dTreeFillData.Fill_Array<Double_t>("TOF_rate", scaler, nhit_TOF);
                nhit_TOF++;         
              }  
            }  // TOF

	    // TAGH
            if(sc->crate == 73){              
	      for(unsigned int jj = 0; jj < sc->fa250_sc.size(); jj++){                           
            
		if(nhit_TAGH >= max_nTAGH) break;               
		if( (jj % 17) == 0) continue;
            
                double scaler = sc->fa250_sc[jj]/1000.;
                                
                dTreeFillData.Fill_Array<Double_t>("TAGH_rate", scaler, nhit_TAGH);
                nhit_TAGH++;                
              }  
            }  // TAGH


            // TAGM
            if(sc->crate == 71){              
              for(unsigned int jj = 0; jj < sc->fa250_sc.size(); jj++){                         

                if(nhit_TAGM >= max_nTAGM) break;           
                if( (jj % 17) == 0) continue;
                
                double scaler = sc->fa250_sc[jj]/1000.;
                                
                dTreeFillData.Fill_Array<Double_t>("TAGM_rate", scaler, nhit_TAGM);
                nhit_TAGM++;                
              }  
            }  // TAGM


	    // CCAL
	    if(sc->crate == 90){              
              for(unsigned int jj = 0; jj < sc->fa250_sc.size(); jj++){                         
		
                if(nhit_CCAL >= max_nCCAL) break;           
                if( (jj % 17) == 0) continue;
                
                double scaler = sc->fa250_sc[jj]/1000.;
                                
                dTreeFillData.Fill_Array<Double_t>("CCAL_rate", scaler, nhit_CCAL);
                nhit_CCAL++;                
              }  
	    }  // CCAL

	  }    // Loop over scalers


	  dTreeFillData.Fill_Single<uint32_t>("nSYNC", nsync_event);
	  dTreeFillData.Fill_Single<uint32_t>("nTRIG", trig_number);


	  dTreeFillData.Fill_Single<Int_t>("nST",    nhit_ST);
	  dTreeFillData.Fill_Single<Int_t>("nTOF",   nhit_TOF);
	  dTreeFillData.Fill_Single<Int_t>("nTAGH",  nhit_TAGH);
	  dTreeFillData.Fill_Single<Int_t>("nTAGM",  nhit_TAGM);
	  dTreeFillData.Fill_Single<Int_t>("nCCAL",  nhit_CCAL);
	  dTreeFillData.Fill_Single<Int_t>("nPSA",   nhit_PSA);
	  dTreeFillData.Fill_Single<Int_t>("nPSB",   nhit_PSB);
	  dTreeFillData.Fill_Single<Int_t>("nPSCA",  nhit_PSCA);
	  dTreeFillData.Fill_Single<Int_t>("nPSCB",  nhit_PSCB);
	 

	  dTreeFillData.Fill_Single<Double_t>("trig_rate_bit0",  trig_rate[0]);
	  dTreeFillData.Fill_Single<Double_t>("trig_rate_bit1",  trig_rate[1]);
	  dTreeFillData.Fill_Single<Double_t>("trig_rate_bit3",  trig_rate[3]);
	  dTreeFillData.Fill_Single<Double_t>("trig_rate_bit6",  trig_rate[6]);
	  dTreeFillData.Fill_Single<Double_t>("trig_rate_bit7",  trig_rate[7]);
	  dTreeFillData.Fill_Single<Double_t>("trig_rate_bit8",  trig_rate[8]);
	  dTreeFillData.Fill_Single<Double_t>("trig_rate_bit10", trig_rate[10]);
	  
	  is_sync_event = true;

	}      // Scaler Bank exists	    
	


	if(is_sync_event)
	  dTreeInterface->Fill(dTreeFillData);


	lockService->RootFillUnLock(this);

	
}

//------------------
// erun
//------------------
void JEventProcessor_scaler_primex::EndRun()
{
	// Any final calculations on histograms (like dividing them)
	// should be done here. This may get called more than once.
}

//------------------
// fini
//------------------
void JEventProcessor_scaler_primex::Finish()
{

         delete dTreeInterface; //saves trees to file, closes file
}

