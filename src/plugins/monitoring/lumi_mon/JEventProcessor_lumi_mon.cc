// $Id$
//
//    File: JEventProcessor_lumi_mon.cc
//   A.S.
// 
//

#include <iostream>
#include <sstream>
#include "JEventProcessor_lumi_mon.h"


using namespace std;


#include <JANA/JApplication.h>
#include <JANA/JFactory.h>

#include "RF/DRFTime_factory.h"

#include <TAGGER/DTAGHDigiHit.h>
#include <TAGGER/DTAGHHit.h>

#include <TAGGER/DTAGMDigiHit.h>
#include <TAGGER/DTAGMHit.h>

#include <PAIR_SPECTROMETER/DPSHit.h>
#include <PAIR_SPECTROMETER/DPSCHit.h>

#include <TRIGGER/DL1Trigger.h>

#include <PAIR_SPECTROMETER/DPSPair.h>
#include <PAIR_SPECTROMETER/DPSCPair.h>

#include <PID/DBeamPhoton.h>


static TH1D  *de_ps_tagh;
static TH2D  *de_ps_tagh1;
		
static TH1D  *de_ps_tagm;
static TH2D  *de_ps_tagm1;

static TH1D  *htrig_bit;

static TH2D  *tagh_ps_time;
static TH2D  *tagh_ps_psc_time;
static TH2D  *tagm_ps_time;
static TH2D  *tagm_ps_psc_time;
 
static TH2D  *tagh_ps_psc_time1;
static TH2D  *tagm_ps_psc_time1;

static TH2D  *tagh_ps_psc_time20;
static TH2D  *tagm_ps_psc_time20;

static TH1D  *ps_energy;

static TH1D  *hdt_tagh_ps;
static TH1D  *hdt_tagm_ps;

static TH1D  *hdt_tagh_ps1;
static TH1D  *hdt_tagm_ps1;

static TH1D  *hdt_psc_ps;

static TH1D *hnhit_tagm;
static TH1D *htagm_dcnt;

static TH1D *hnhit_tagh;
static TH1D *htagh_dcnt;

static TH1D *hnpair;
static TH1D *hntagh, *hntagm;

static TH1D *hps_dt;
static TH1D *hpsc_dt;

static TH2D *hps_hit_time_a;
static TH2D *hps_hit_time_b;

static TH2D *hps_hit_time_rf_a;
static TH2D *hps_hit_time_rf_b;

static TH2D *hpsc_adc_time_a;
static TH2D *hpsc_adc_time_b;

static TH2D *hpsc_tdc_time_a;
static TH2D *hpsc_tdc_time_b;

static TH2D *hpsc_t_time_a;
static TH2D *hpsc_t_time_b;

static TH2D *hpsc_t_time_rf_a;
static TH2D *hpsc_t_time_rf_b;

static TH2D *htagh_hit_adc_time; 
static TH2D *htagh_hit_tdc_time;
static TH2D *htagh_hit_t_time;
static TH2D *htagh_hit_rf;

static TH2D *htagm_hit_adc_time;
static TH2D *htagm_hit_tdc_time;
static TH2D *htagm_hit_t_time;
static TH2D *htagm_hit_rf;

static TH1F *hps_hit_time_all_a;
static TH1F *hps_hit_time_all_b;

static TH1F *hpsc_adc_time_all_a;
static TH1F *hpsc_adc_time_all_b;
static TH1F *hpsc_tdc_time_all_a;
static TH1F *hpsc_tdc_time_all_b;
static TH1F *hpsc_t_time_all_a;
static TH1F *hpsc_t_time_all_b;

static TH1F *htagh_hit_adc_time_all;
static TH1F *htagh_hit_tdc_time_all;
static TH1F *htagh_hit_t_time_all;
static TH1F *htagm_hit_adc_time_all;
static TH1F *htagm_hit_tdc_time_all;
static TH1F *htagm_hit_t_time_all;

static TH1D *hdt_psc_rf, *hdt_ps_rf, *hdt_tagh_rf, *hdt_tagm_rf;


// Define RFTime_factory
DRFTime_factory* dRFTimeFactory;


// Routine used to create our JEventProcessor
extern "C"{
void InitPlugin(JApplication *app){


	InitJANAPlugin(app);


	app->Add(new JEventProcessor_lumi_mon());

}
} // "C"


//------------------
// init
//------------------
void JEventProcessor_lumi_mon::Init()
{
  auto app = GetApplication();
  lockService = app->GetService<JLockService>();

  TDirectory *main = gDirectory;
  TDirectory *dlumi_mon = gDirectory->mkdir("Lumi_mon");
  dlumi_mon->cd();

  hnpair = new TH1D("n_ps_pair","n_ps_pair",   100, -0.5, 99.5);
  hntagh = new TH1D("n_tagh","n_tagh",   100, -0.5, 99.5);
  hntagm = new TH1D("n_tagm","n_tagm",   100, -0.5, 99.5);
  
  ps_energy  = new TH1D("ps_energy","ps_energy", 500,5.,12.5);

  de_ps_tagh        =   new TH1D("de_ps_tagh","de_ps_tagh", 400, -1., 1.);
  de_ps_tagh1       =   new TH2D("de_ps_tagh1","de_ps_tagh1", 300, 0.5, 299.5, 200, -1., 1.);

  de_ps_tagm        =   new TH1D("de_ps_tagm","de_ps_tagm", 200, -1., 1.);
  de_ps_tagm1       =   new TH2D("de_ps_tagm1","de_ps_tagm1", 130, 0.5, 129.5, 200, -1., 1.);

  tagh_ps_time      =   new TH2D("tagh_ps_time","tagh_ps_time", 300, -0.5, 299.5, 100, -0.5, 99.5);
  tagh_ps_psc_time  =   new TH2D("tagh_ps_psc_time","tagh_ps_psc_time", 300, -0.5, 299.5, 100, -0.5, 99.5);

  tagm_ps_time      =   new TH2D("tagm_ps_time","tagm_ps_time", 300, -0.5, 299.5, 100, -0.5, 99.5);
  tagm_ps_psc_time  =   new TH2D("tagm_ps_psc_time","tagm_ps_psc_time", 300, -0.5, 299.5, 100, -0.5, 99.5);


  htrig_bit         =   new TH1D("trig_bit","trig_bit",   100, -0.5, 99.5);

  hdt_tagh_ps       =   new TH1D("dt_tagh_ps","dt_tagh_ps",   6000, -30., 30.0);
  hdt_tagh_ps1      =   new TH1D("dt_tagh_ps1","dt_tagh_ps1",   6000, -30., 30.0);

  hdt_tagm_ps       =   new TH1D("dt_tagm_ps","dt_tagm_ps",   6000, -30., 30.0);
  hdt_tagm_ps1      =   new TH1D("dt_tagm_ps1","dt_tagm_ps1",   6000, -30., 30.0);

  hdt_psc_ps        =   new TH1D("dt_psc_ps","dt_psc_ps",   6000, -30., 30.0);

  tagh_ps_psc_time1   =   new TH2D("tagh_ps_psc_time1","tagh_ps_psc_time1", 300, -0.5, 299.5, 3000, -30., 30.);
  tagm_ps_psc_time1   =   new TH2D("tagm_ps_psc_time1","tagm_ps_psc_time1", 300, -0.5, 299.5, 3000, -30., 30.);

  tagh_ps_psc_time20  =   new TH2D("tagh_ps_psc_time20","tagh_ps_psc_time20", 300, -0.5, 299.5, 3000, -30., 30.);
  tagm_ps_psc_time20  =   new TH2D("tagm_ps_psc_time20","tagm_ps_psc_time20", 300, -0.5, 299.5, 3000, -30., 30.);

  hnhit_tagm         =   new TH1D("nhit_tagm","nhit_tagm",   100, -0.5, 99.5);
  htagm_dcnt         =   new TH1D("tagm_dcnt","tagm_dcnt",   599, -299.5, 299.5);

  hnhit_tagh         =   new TH1D("nhit_tagh","nhit_tagh",   100, -0.5, 99.5);
  htagh_dcnt         =   new TH1D("tagh_dcnt","tagh_dcnt",   599, -299.5, 299.5);


  hps_dt  = new TH1D("ps_dt", "ps_dt",    1000, -12., 12.);
  hpsc_dt = new TH1D("psc_dt","psc_dt",   1000, -12., 12.);


  hps_hit_time_a   =   new TH2D("ps_hit_time_a","ps_hit_time_a", 146, -0.5, 145.5,800,-200.,200.);
  hps_hit_time_b   =   new TH2D("ps_hit_time_b","ps_hit_time_b", 146, -0.5, 145.5,800,-200.,200.);

  hps_hit_time_rf_a   =   new TH2D("ps_hit_time_rf_a","ps_hit_time_rf_a", 146, -0.5, 145.5,800,-50.,50.);
  hps_hit_time_rf_b   =   new TH2D("ps_hit_time_rf_b","ps_hit_time_rf_b", 146, -0.5, 145.5,800,-50.,50.);

  hps_hit_time_all_a = new TH1F("ps_hit_time_all_a","ps_hit_time_all_a", 800,-200.,200.);
  hps_hit_time_all_b = new TH1F("ps_hit_time_all_b","ps_hit_time_all_b", 800,-200.,200.);


  hpsc_adc_time_a   =   new TH2D("psc_adc_time_a","psc_adc_time_a", 10, -0.5, 9.5,800,-200.,200.);
  hpsc_adc_time_b   =   new TH2D("psc_adc_time_b","psc_adc_time_b", 10, -0.5, 9.5,800,-200.,200.);

  hpsc_tdc_time_a   =   new TH2D("psc_tdc_time_a","psc_tdc_time_a", 10, -0.5, 9.5,800,-200.,200.);
  hpsc_tdc_time_b   =   new TH2D("psc_tdc_time_b","psc_tdc_time_b", 10, -0.5, 9.5,800,-200.,200.);

  hpsc_t_time_a   =   new TH2D("psc_t_time_a","psc_t_time_a", 10, -0.5, 9.5,800,-200.,200.);
  hpsc_t_time_b   =   new TH2D("psc_t_time_b","psc_t_time_b", 10, -0.5, 9.5,800,-200.,200.);


  hpsc_t_time_rf_a   =   new TH2D("psc_t_time_rf_a","psc_t_time_rf_a", 10, -0.5, 9.5,1600,-50.,50.);
  hpsc_t_time_rf_b   =   new TH2D("psc_t_time_rf_b","psc_t_time_rf_b", 10, -0.5, 9.5,1600,-50.,50.);


  hpsc_adc_time_all_a = new TH1F("psc_adc_time_all_a","psc_adc_time_all_a", 800,-200.,200.);
  hpsc_adc_time_all_b = new TH1F("psc_adc_time_all_b","psc_adc_time_all_b", 800,-200.,200.);

  hpsc_tdc_time_all_a = new TH1F("psc_tdc_time_all_a","psc_tdc_time_all_a", 800,-200.,200.);
  hpsc_tdc_time_all_b = new TH1F("psc_tdc_time_all_b","psc_tdc_time_all_b", 800,-200.,200.);

  hpsc_t_time_all_a = new TH1F("psc_t_time_all_a","psc_t_time_all_a", 800,-200.,200.);
  hpsc_t_time_all_b = new TH1F("psc_t_time_all_b","psc_t_time_all_b", 800,-200.,200.);


  htagh_hit_adc_time  =  new TH2D("tagh_time_adc","tagh_time_adc", 300, -0.5, 299.5,800,-200.,200.);
  htagh_hit_tdc_time  =  new TH2D("tagh_time_tdc","tagh_time_tdc", 300, -0.5, 299.5,800,-200.,200.);
  htagh_hit_t_time    =  new TH2D("tagh_time_t", "tagh_time_t",  300, -0.5, 299.5,800,-200.,200.);
  htagh_hit_rf            = new TH2D("tagh_time_rf", "tagh_time_rf",  300, -0.5, 299.5,1600,-50.,50.);

  htagh_hit_adc_time_all  = new TH1F("tagh_hit_adc_time_all","tagh_hit_adc_time_all", 800,-200.,200.);
  htagh_hit_tdc_time_all  = new TH1F("tagh_hit_tdc_time_all","tagh_hit_tdc_time_all", 800,-200.,200.);
  htagh_hit_t_time_all    = new TH1F("tagh_hit_t_time_all", "tagh_hit_t_time_all",  800,-200.,200.);

  htagm_hit_adc_time  = new TH2D("tagm_time_adc","tagm_time_adc", 200, -0.5, 199.5,800,-200.,200.);
  htagm_hit_tdc_time  = new TH2D("tagm_time_tdc","tagm_time_tdc", 200, -0.5, 199.5,800,-200.,200.);
  htagm_hit_t_time    = new TH2D("tagm_time_t","tagm_time_t",   200, -0.5, 199.5,800,-200.,200.);
  htagm_hit_rf        = new TH2D("tagm_time_rf","tagm_time_rf",   200, -0.5, 199.5,1600,-50.,50.);

  htagm_hit_adc_time_all  = new TH1F("tagm_hit_adc_time_all", "tagm_hit_adc_time_all", 800,-200.,200.);
  htagm_hit_tdc_time_all  = new TH1F("tagm_hit_tdc_time_all", "tagm_hit_tdc_time_all", 800,-200.,200.);
  htagm_hit_t_time_all    = new TH1F("tagm_hit_t_time_all",   "tagm_hit_t_time_all",  800,-200.,200.);

  hdt_psc_rf      =   new TH1D("dt_psc_rf",  "dt_psc_rf",    6000, -30., 30.);
  hdt_ps_rf       =   new TH1D("dt_ps_rf",   "dt_ps_rf",     6000, -30., 30.);
  hdt_tagm_rf     =   new TH1D("dt_tagm_rf", "dt_tagm_rf",   6000, -30., 30.);
  hdt_tagh_rf     =   new TH1D("dt_tagh_rf", "dt_tagh_rf",   6000, -30., 30.);

  main->cd();

}


//------------------
// brun
//------------------
void JEventProcessor_lumi_mon::BeginRun(const std::shared_ptr<const JEvent> &event)
{ 
  

  // Initialize RF time factory
  dRFTimeFactory = static_cast<DRFTime_factory*>(event->GetFactory("DRFTime", ""));
  
  // be sure that DRFTime_factory::init() and brun() are called
  vector<const DRFTime*> locRFTimes;
  event->Get(locRFTimes);
  
}

//------------------
// evnt
//------------------
void JEventProcessor_lumi_mon::Process(const std::shared_ptr<const JEvent> &event)
{

        int trig_bit[33];

	// RF 
	vector<const DRFTime*>   locRFTimes;
	event->Get(locRFTimes, "PSC");
	const DRFTime* locRFTime = NULL;
	
	if (locRFTimes.size() > 0)
	  locRFTime = locRFTimes[0];
	else
	  return;
	


	// TRIGGER
	vector<const DL1Trigger*> l1trig;
	
	// PS and PSC
	vector<const DPSPair*>    ps_pairs;
	vector<const DPSCPair*>   psc_pairs;

	// TAGH
	vector<const DTAGHHit*> tagh_hits;

	// TAGM
	vector<const DTAGMHit*> tagm_hits;

	// Photon Beam
	vector<const DBeamPhoton*> beam_ph;
   
	vector<const DPSCHit*> psc_hits;

	vector<const DPSHit*> ps_hits;

	vector<int> tagm_tmp;
	vector<int> tagh_tmp;
	vector<double> tagm_tmp_time;


	tagm_tmp.clear();
	tagh_tmp.clear();
	tagm_tmp_time.clear();


	event->Get(l1trig);

	event->Get(ps_pairs);
	event->Get(psc_pairs);

	event->Get(tagh_hits);

	event->Get(tagm_hits);

	event->Get(beam_ph);


	event->Get(ps_hits);

	event->Get(psc_hits);

	lockService->RootFillLock(this);
	

	hnpair->Fill(float(ps_pairs.size()));
	hntagh->Fill(float(tagh_hits.size()));
	hntagm->Fill(float(tagm_hits.size()));


	memset(trig_bit,0,sizeof(trig_bit));
	
	
        if( l1trig.size() > 0){
	  	  
	  for(unsigned int bit = 0; bit < 32; bit++){	    
	    
	    trig_bit[bit + 1] = (l1trig[0]->trig_mask & (1 << bit)) ? 1 : 0;
       	    
	    if(trig_bit[bit + 1] == 1) htrig_bit->Fill(Float_t(bit+1)); 
	  }
	  	  
	}
	
	
	if( trig_bit[4] != 1){
	  lockService->RootFillUnLock(this);
	  return;
	}
       
	if((ps_pairs.size() == 0) || (psc_pairs.size() == 0)){
	  lockService->RootFillUnLock(this);
	  return;
	}
	


	// Time alignment between PS and PSC

	double psc_time  = (psc_pairs[0]->ee.first->t  +  psc_pairs[0]->ee.second->t)/2.;
	double ps_time   = (ps_pairs[0]->ee.first->t   +  ps_pairs[0]->ee.second->t)/2.;

	double rf_psc = dRFTimeFactory->Step_TimeToNearInputTime(locRFTime->dTime, psc_time);

	hdt_psc_rf->Fill(psc_time - rf_psc); 	hdt_ps_rf->Fill(ps_time - rf_psc);

	hdt_psc_ps->Fill(psc_time - ps_time);

	double ps_dt   =  ps_pairs[0]->ee.first->t   -  ps_pairs[0]->ee.second->t;
	double psc_dt  =  psc_pairs[0]->ee.first->t  -  psc_pairs[0]->ee.second->t;	


	hps_dt->Fill(ps_dt);
	hpsc_dt->Fill(psc_dt);
	

	// TAGH
	
	for(unsigned int ii = 0; ii < tagh_hits.size(); ii++){
	  int counter_id = tagh_hits[ii]->counter_id;
	  
	  if(tagh_hits[ii]->has_fADC){	    

	    double time_adc =  tagh_hits[ii]->time_fadc;	   	    

	    htagh_hit_adc_time->Fill(float(counter_id),time_adc);
	    htagh_hit_adc_time_all->Fill(time_adc);
	  }
	  
	  if(tagh_hits[ii]->has_TDC){
	    
	    double time_tdc =  tagh_hits[ii]->time_tdc;	
	    double time_t  =  tagh_hits[ii]->t;	
	    
	    htagh_hit_tdc_time->Fill(float(counter_id),time_tdc);
	    htagh_hit_tdc_time_all->Fill(time_tdc);
	    htagh_hit_t_time->Fill(float(counter_id),time_t);
	    htagh_hit_t_time_all->Fill(time_t);
	    
          }	  

	}
	
	
	// TAGM
	
	for(unsigned int ii = 0; ii < tagm_hits.size(); ii++){
	  
	  int counter_id  =  tagm_hits[ii]->column;
	  int counter_row =  tagm_hits[ii]->row;
	  
	  if(counter_row != 0) continue;	 
	  	  
	  if(tagm_hits[ii]->has_fADC){	    
	    double time_adc =  tagm_hits[ii]->time_fadc;	   	    
	    htagm_hit_adc_time->Fill(float(counter_id),time_adc);
	    htagm_hit_adc_time_all->Fill(time_adc);
	  }
	  
	  if(tagm_hits[ii]->has_TDC){
	    double time_tdc =  tagm_hits[ii]->time_tdc;	   	    
	    double time_t  =  tagm_hits[ii]->t;

	    htagm_hit_tdc_time->Fill(float(counter_id),time_tdc);
	    htagm_hit_tdc_time_all->Fill(time_tdc);
	    htagm_hit_t_time->Fill(float(counter_id),time_t);
	    htagm_hit_t_time_all->Fill(time_t);

          }	
	  	  
	}


	for(unsigned int ii = 0; ii < ps_hits.size(); ii++){
	  int  arm      =  ps_hits[ii]->arm;
	  int  column   =  ps_hits[ii]->column;
	  double time   =  ps_hits[ii]->t;

	  if(arm == 0){
	    hps_hit_time_a->Fill(float(column),time);
	    hps_hit_time_all_a->Fill(time);
	    hps_hit_time_rf_a->Fill(float(column),time - rf_psc);
	  }
	  if(arm == 1){
	    hps_hit_time_b->Fill(float(column),time);
	    hps_hit_time_all_b->Fill(time);
            hps_hit_time_rf_b->Fill(float(column),time - rf_psc);	    
	  }
        }


	for(unsigned int ii = 0; ii < psc_hits.size(); ii++){
	  int arm          =   psc_hits[ii]->arm;
	  int column       =   psc_hits[ii]->module;
	  double time_adc  =   psc_hits[ii]->time_fadc;
	  double time_tdc  =   psc_hits[ii]->time_tdc;
	  double time      =   psc_hits[ii]->t;

	  if(arm == 0){
	    hpsc_adc_time_a->Fill(float(column),time_adc);
	    hpsc_adc_time_all_a->Fill(time_adc);
	    hpsc_tdc_time_a->Fill(float(column),time_tdc);
	    hpsc_tdc_time_all_a->Fill(time_tdc);
	    hpsc_t_time_a->Fill(float(column),time);
	    hpsc_t_time_rf_a->Fill(float(column),time - rf_psc);
	    hpsc_t_time_all_a->Fill(time);
	  }
	  if(arm == 1){
	    hpsc_adc_time_b->Fill(float(column),time_adc);
	    hpsc_adc_time_all_b->Fill(time_adc);
	    hpsc_tdc_time_b->Fill(float(column),time_tdc);
	    hpsc_tdc_time_all_b->Fill(time_tdc);
	    hpsc_t_time_b->Fill(float(column),time);
            hpsc_t_time_rf_b->Fill(float(column),time - rf_psc);
	    hpsc_t_time_all_b->Fill(time);
	  }
        }



	int nhit_tagm = 0;
	int nhit_tagh = 0;

	
	for(unsigned int ii = 0; ii < beam_ph.size(); ii++){
          	  
	  int detector_id  =  beam_ph[ii]->dSystem;
	  int counter_id   =  beam_ph[ii]->dCounter;
	  
	  double tagh_energy =  beam_ph[ii]->momentum().Mag();
	  
	  if(ps_pairs.size() > 0){	   
	    
	    double pair_energy = ps_pairs[0]->ee.first->E + ps_pairs[0]->ee.second->E;
	    
	    if(fabs(pair_energy - tagh_energy) < 0.2){
	      
	      double dt  = beam_ph[ii]->time() - ps_pairs[0]->ee.first->t;
	      double dt1 = beam_ph[ii]->time() - psc_pairs[0]->ee.first->t;
	      
	      
	      if(detector_id == 2048){		
		
		hdt_tagh_ps->Fill(dt);		
		hdt_tagh_ps1->Fill(dt1);

		tagh_ps_psc_time1->Fill(float(counter_id),dt);
		tagh_ps_psc_time20->Fill(float(counter_id),dt1);


		hdt_tagh_rf->Fill(beam_ph[ii]->time() - rf_psc);
		htagh_hit_rf->Fill(float(counter_id),beam_ph[ii]->time() - rf_psc);
			       
		tagh_tmp.push_back(counter_id);
		
		nhit_tagh++;
		
	      }
	      
	      if(detector_id == 128){		
		
		hdt_tagm_ps->Fill(dt);
		hdt_tagm_ps1->Fill(dt1);
		
		tagm_ps_psc_time1->Fill(float(counter_id),dt);
		tagm_ps_psc_time20->Fill(float(counter_id),dt1);

		hdt_tagm_rf->Fill(beam_ph[ii]->time() - rf_psc);
		htagm_hit_rf->Fill(float(counter_id),beam_ph[ii]->time() - rf_psc);

		tagm_tmp.push_back(counter_id);
		tagm_tmp_time.push_back(beam_ph[ii]->time());
		
		nhit_tagm++;
		
	      }
	      
	    }	      
	    
	  }	  
	}
	


	
	hnhit_tagm->Fill(float(nhit_tagm));

	if(nhit_tagm == 2){
	  int count_dif = tagm_tmp[0] - tagm_tmp[1];

	  double dt = tagm_tmp_time[0] - tagm_tmp_time[1];

	  if(fabs(dt) < 5)

	    htagm_dcnt->Fill(count_dif);


	}

	hnhit_tagh->Fill(float(nhit_tagh));

	if(nhit_tagh == 2){
	  int count_dif = tagh_tmp[0] - tagh_tmp[1];	  

	  htagh_dcnt->Fill(count_dif);
	}




	// TAGH
	
	for(unsigned int ii = 0; ii < tagh_hits.size(); ii++){
	  
	  if( (tagh_hits[ii]->has_fADC  == 0) || (tagh_hits[ii]->has_TDC  == 0)) continue;

	  int counter_id = tagh_hits[ii]->counter_id;
	  
	  const  DTAGHDigiHit* tagh_digi_hit = NULL;
	  
	  tagh_hits[ii]->GetSingle(tagh_digi_hit);  
	  
	  int adc_time = 0;
     

	  if(tagh_digi_hit)
	    adc_time    =  (tagh_digi_hit->pulse_time & 0x7FC0) >> 6;
	  
	  double tagh_energy = tagh_hits[ii]->E;
	  
	  
	  tagh_ps_time->Fill(Float_t(counter_id),Float_t(adc_time));
	  
	  
	  if(ps_pairs.size() > 0){	   
	    

	    double pair_energy = ps_pairs[0]->ee.first->E + ps_pairs[0]->ee.second->E;

	    
	    if(fabs(pair_energy - tagh_energy) < 0.2){
	      
	      de_ps_tagh->Fill(pair_energy - tagh_energy);
	      de_ps_tagh1->Fill(Float_t(counter_id),(pair_energy - tagh_energy));
	      tagh_ps_psc_time->Fill(Float_t(counter_id),Float_t(adc_time));
	      	      

	    }    // DE
	    
	  }       //  If PS pairs
	  
	}         //  Loop over TAGH hits
	
	


	// TAGM
	
	for(unsigned int ii = 0; ii < tagm_hits.size(); ii++){
	  
	  int counter_id  =  tagm_hits[ii]->column;
	  int counter_row =  tagm_hits[ii]->row;
	  
	  if(counter_row != 0) continue;	 
	  
	  //	  if( tagm_hits[ii]->has_TDC  == 0 ) continue;
	  
	  if( (tagm_hits[ii]->has_fADC  == 0) || (tagm_hits[ii]->has_TDC  == 0)) continue;
	  
	  const  DTAGMDigiHit* tagm_digi_hit = NULL;
	  
	  tagm_hits[ii]->GetSingle(tagm_digi_hit);  
	  
	  int adc_time = 0;	  
	  
	  if(tagm_digi_hit)
	    adc_time    =  (tagm_digi_hit->pulse_time & 0x7FC0) >> 6;
	  
	  double tagm_energy   =  tagm_hits[ii]->E;
	  
	  tagm_ps_time->Fill(Float_t(counter_id),Float_t(adc_time));
	  
	  if(ps_pairs.size() > 0){
	    
	    double pair_energy = ps_pairs[0]->ee.first->E + ps_pairs[0]->ee.second->E;
	    
	    if(fabs(pair_energy - tagm_energy) < 0.2){
	      
	      de_ps_tagm->Fill(pair_energy - tagm_energy);
	      de_ps_tagm1->Fill(Float_t(counter_id),(pair_energy - tagm_energy));
	      tagm_ps_psc_time->Fill(Float_t(counter_id),Float_t(adc_time));	      
	      
	    }       //   DE		       
	    
	  }           //   Pair size
	  
	}             //   TAGM hits
	
	
	

	
	// Plot energy of all reconstructed PS pairs
	if(trig_bit[4] == 1){
	  
	  if(ps_pairs.size() > 0){
	    
	    double pair_energy = ps_pairs[0]->ee.first->E + ps_pairs[0]->ee.second->E;	    
	    
	    ps_energy->Fill(pair_energy);
	  }
	  
	}
	


	lockService->RootFillUnLock(this);
	

}

//------------------
// erun
//------------------
void JEventProcessor_lumi_mon::EndRun()
{
	// Any final calculations on histograms (like dividing them)
	// should be done here. This may get called more than once.
}

//------------------
// fini
//------------------
void JEventProcessor_lumi_mon::Finish()
{
}

