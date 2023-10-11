// $Id$
//
//    File: JEventProcessor_st_tw_corr_auto.cc
// Created: Mon Oct 26 10:35:45 EDT 2015
// Creator: mkamel (on Linux ifarm1401 2.6.32-431.el6.x86_64 x86_64)
//
#include "JEventProcessor_st_tw_corr_auto.h"
#include "TRIGGER/DTrigger.h"

// Routine used to create our JEventProcessor
#include <JANA/JFactoryT.h>
#include <DANA/DEvent.h>

extern "C"{
void InitPlugin(JApplication *app){
	InitJANAPlugin(app);
	app->Add(new JEventProcessor_st_tw_corr_auto());
}
} // "C"

//------------------
// JEventProcessor_st_tw_corr_auto (Constructor)
//------------------
JEventProcessor_st_tw_corr_auto::JEventProcessor_st_tw_corr_auto()
{
	SetTypeName("JEventProcessor_st_tw_corr_auto");
}

//------------------
// ~JEventProcessor_st_tw_corr_auto (Destructor)
//------------------
JEventProcessor_st_tw_corr_auto::~JEventProcessor_st_tw_corr_auto()
{

}

//------------------
// Init
//------------------
void JEventProcessor_st_tw_corr_auto::Init()
{
	// This is called once at program startup. If you are creating
	// and filling historgrams in this plugin, you should lock the
	// ROOT mutex like this:
	//
	// GetLockService(locEvent)->RootWriteLock();
	//  ... fill historgrams or trees ...
	// GetLockService(locEvent)->RootUnLock();
	//

	auto app = GetApplication();
	lockService = app->GetService<JLockService>();

  USE_TIMEWALK_CORRECTION = 1.;
  app->SetDefaultParameter("SC:USE_TIMEWALK_CORRECTION", USE_TIMEWALK_CORRECTION,
			      "Flag to decide if timewalk corrections should be applied.");
  // ***************** define constants*************************
  NCHANNELS          = 30;
  tdc_thresh_mV   = 50.0;
  tdc_gain_factor = 5.0;
  adc_max_chan    = 4096.0;
  adc_max_mV      = 2000.0;
  adc_thresh_calc = (tdc_thresh_mV/tdc_gain_factor)*(adc_max_chan/adc_max_mV);
  // **************** define histograms *************************
  
  h_pp_chan = new TH1I*[NCHANNELS];
  h_stt_chan = new TH1I*[NCHANNELS];
  h2_stt_vs_pp_chan = new TH2I*[NCHANNELS];
  h1_st_corr_time = new TH1I*[NCHANNELS];
  h2_st_corr_vs_pp = new TH2I*[NCHANNELS];
  // Book Histos
  for (Int_t i = 0; i < NCHANNELS; i++)
    { 
      h2_stt_vs_pp_chan[i] = new TH2I(Form("stt_vs_pp_chan_%i", i+1), "Hit Time vs. Pulse Peak; Pulse Peak (channels); #delta_{t} (ns)", 4096,0.0,4095.0, 160, -5.0, 5.0);
      
      h_pp_chan[i]  = new TH1I(Form("pp_chan_%i", i+1), "Pulse Peak; Pulse Peak (channels); Counts",  4096,0.0,4095.0);
      
      h_stt_chan[i] = new TH1I(Form("h_stt_chan_%i",i+1), "ST Time; ST Time (ns); Counts", 160, -5.0, 5.0);

      h1_st_corr_time[i] = new TH1I(Form("h1_st_corr_time_%i",i+1), "ST Time; ST Time (ns); Counts", 160, -5.0, 5.0);
      
      h2_st_corr_vs_pp[i]= new TH2I(Form("h2_st_corr_vs_pp_%i", i+1), "Hit Time vs. Pulse Peak; Pulse Peak (channels); #delta_{t} (ns)", 4096,0.0,4095.0, 160, -5.0, 5.0);
    }
 
}
//------------------
// BeginRun
//------------------
void JEventProcessor_st_tw_corr_auto::BeginRun(const std::shared_ptr<const JEvent>& event)
{
  // This is called whenever the run number changes
  // load constant tables
  // timewalk_parameters (timewalk_parms)
  if(GetCalib(event, "START_COUNTER/timewalk_parms_v2", timewalk_parameters))
    jout << "Error loading /START_COUNTER/timewalk_parms_v2 !" << endl;
  
}

//------------------
// Process
//------------------
void JEventProcessor_st_tw_corr_auto::Process(const std::shared_ptr<const JEvent>& event)
{
	// This is called for every event. Use of common resources like writing
	// to a file or filling a histogram should be mutex protected. Using
	// event->Get(...) to get reconstructed objects (and thereby activating the
	// reconstruction algorithm) should be done outside of any mutex lock
	// since multiple threads may call this method at the same time.
	// Here's an example:
	//
	// vector<const MyDataClass*> mydataclasses;
	// event->Get(mydataclasses);
	//
	// GetLockService(locEvent)->RootWriteLock();
	//  ... fill historgrams or trees ...
	// GetLockService(locEvent)->RootUnLock();

        // select events with physics events, i.e., not LED and other front panel triggers
        const DTrigger* locTrigger = NULL; 
	event->GetSingle(locTrigger); 
	if(locTrigger->Get_L1FrontPanelTriggerBits() != 0) 
	  return;

  vector<const DSCHit*>      st_hits; 
  event->Get(st_hits); 
  
  for (unsigned int k = 0; k < st_hits.size(); k++)
    {
      if(!st_hits[k]->has_fADC || !st_hits[k]->has_TDC)
	continue;
      // get the associated digihit so that we can access the
      // pulse peak and pedestal information
      const DSCDigiHit* the_digihit = NULL;
      st_hits[k]->GetSingle(the_digihit);
      if(the_digihit == NULL)
	continue;
      
      // Extract the information we're interested in 
      double pulse_peak = the_digihit->pulse_peak;
      double pedestal = the_digihit->pedestal;
      double nsamples_pedestal = the_digihit->nsamples_pedestal;
      double adc_pp = pulse_peak - pedestal/nsamples_pedestal;
      
      double T   = st_hits[k]->t;             // this is always set to the TDC time
      // timewalk corrections are controlled by command line flag
      double adc_t   = st_hits[k]->t_fADC;
      int sector     = st_hits[k]->sector;

		// FILL HISTOGRAMS
		// Since we are filling histograms local to this plugin, it will not interfere with other ROOT operations: can use plugin-wide ROOT fill lock
		lockService->RootFillLock(this); //ACQUIRE ROOT FILL LOCK
      {
	double	st_time = T - adc_t;
	h_stt_chan[sector-1]->Fill(st_time);
	if (USE_TIMEWALK_CORRECTION==0)
	  {
	    h_pp_chan[sector-1]->Fill(adc_pp);
	    h2_stt_vs_pp_chan[sector-1]->Fill(adc_pp,st_time);
	  }
	//  Correct for the time walk
	else if (USE_TIMEWALK_CORRECTION==1)
	  { 
	    double C0       = timewalk_parameters[sector -1][0];
	    double C1       = timewalk_parameters[sector -1][1];
	    double C2       = timewalk_parameters[sector -1][2];
	    double A_THRESH = timewalk_parameters[sector -1][3];
	    double A0       = timewalk_parameters[sector -1][4];
	    // cout<<  C1 << "\t" << C2 << "\t" << A_THRESH << "\t" << A0  <<endl;
	    
	    st_time -= C0 + C1 *(TMath::Power(A0/A_THRESH, C2));
	    // cout << "st_time =" << st_time << endl;
	    h1_st_corr_time[sector-1]->Fill(st_time);
	    h2_st_corr_vs_pp[sector-1]->Fill(adc_pp,st_time);
	  }
      }
		lockService->RootFillUnLock(this); //RELEASE ROOT FILL LOCK
    }
}

//------------------
// EndRun
//------------------
void JEventProcessor_st_tw_corr_auto::EndRun()
{
	// This is called whenever the run number changes, before it is
	// changed to give you a chance to clean up before processing
	// events from the next run number.
}

//------------------
// Finish
//------------------
void JEventProcessor_st_tw_corr_auto::Finish()
{
}

