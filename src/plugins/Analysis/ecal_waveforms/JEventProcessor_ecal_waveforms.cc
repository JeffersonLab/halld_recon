// $Id$
//
//    File: JEventProcessor_ecal_waveforms.cc
// Created: Fri May 30 04:30:17 PM EDT 2025
// Creator: somov (on Linux ifarm2401.jlab.org 5.14.0-503.19.1.el9_5.x86_64 x86_64)
//

/// For more information on the syntax changes between JANA1 and JANA2, visit: https://jeffersonlab.github.io/JANA2/#/jana1to2/jana1-to-jana2

#include "JEventProcessor_ecal_waveforms.h"
using namespace std;

// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
extern "C"{
void InitPlugin(JApplication *app){
    InitJANAPlugin(app);
    app->Add(new JEventProcessor_ecal_waveforms());
}
} // "C"


//------------------
// JEventProcessor_ecal_waveforms (Constructor)
//------------------
JEventProcessor_ecal_waveforms::JEventProcessor_ecal_waveforms()
{
	SetTypeName(NAME_OF_THIS); // Provide JANA with this class's name

    // Parameters and Services should be accessed from Init() instead of here!
}

//------------------
// ~JEventProcessor_ecal_waveforms (Destructor)
//------------------
JEventProcessor_ecal_waveforms::~JEventProcessor_ecal_waveforms()
{
}

//------------------
// Init
//------------------
void JEventProcessor_ecal_waveforms::Init()
{
    // This is called once at program startup. 
    
    auto app = GetApplication();
    // lockService should be initialized here like this
    lockService = app->GetService<JLockService>();

    par_row = 5;
    par_col = 5;
    
    JParameterManager *parms = app->GetJParameterManager();
    parms->SetDefaultParameter("ROW", par_row);
    parms->SetDefaultParameter("COL", par_col);

    par_plot_ref = 0;

    // Set PLOT:REF to 1 to plot waveforms for reference PMTs
    parms->SetDefaultParameter("PLOT:REF", par_plot_ref);
    
    gStyle->SetOptStat(0);
    
    gStyle->SetTitleFontSize(0.05);
    
    gStyle->SetTitleFont(130,"xy");
    gStyle->SetLabelFont(130,"xy");
    
    gStyle->SetLineWidth(1.5);
    gStyle->SetTextSize(1.2);
        
    gStyle->SetLabelSize(0.04,"xy");
    gStyle->SetTitleSize(0.03,"xy");
    
    gStyle->SetTitleOffset(0.5,"x");
    gStyle->SetTitleOffset(0.2,"y");
    
    gStyle->SetTitleBorderSize(0);
   
    app1 =  new TApplication("app1",NULL,0);
    
    for(int ii = 0; ii < 40; ii++){
      for(int jj = 0; jj < 40; jj++){
	char name[100];
	sprintf(name,"Col: %i Row: %i",par_col,par_row);
	ecal_wave_ind[ii][jj] = new TProfile(name,name,100,-0.5,99.5,-10.,8193);
      }
    }

    ecal_ref1 = new TProfile("Ref PMT1","Ref PMT2",100,-0.5,99.5,-10.,8193);
    ecal_ref2 = new TProfile("Ref PMT2","Ref PMT2",100,-0.5,99.5,-10.,8193);
    
    my_canvas  = new TCanvas("ECAL ", "ECAL ",1000, 600);
   
}

//------------------
// BeginRun
//------------------
void JEventProcessor_ecal_waveforms::BeginRun(const std::shared_ptr<const JEvent> &event)
{
    // This is called whenever the run number changes
}

//------------------
// Process
//------------------
void JEventProcessor_ecal_waveforms::Process(const std::shared_ptr<const JEvent> &event)
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
    // If you have lockService initialized in Init() then you can acquire locks like this
    // lockService->RootFillLock(this);
    //  ... fill historgrams or trees ...
    // lockService->RootFillUnLock(this);

    int debug  =  0;
    unsigned int plot_mod_thr  =  120;
    unsigned int plot_ref_thr  =  120;

    
    int plot_led = 0;
    
    vector<const DECALDigiHit *> ecal_digi_hits; 
    vector<const DECALRefDigiHit*> ecal_ref_digihits;

    event->Get(ecal_digi_hits);
    event->Get(ecal_ref_digihits);

    // Disable if want to plot REF PMTs
    if(par_plot_ref == 1) plot_led = 0;
    
    // Don't use the trigger type, just go by the hit multiplicity
    if( (plot_led == 1) &&  (ecal_digi_hits.size() < 1000)){
      return;
    }

    if( (par_plot_ref == 1) && (ecal_ref_digihits.size() == 0)){
      return;
    }
    
    if(debug)
      cout << " Number of hits = " << ecal_digi_hits.size() <<  " Number of REF hits = " <<  ecal_ref_digihits.size() << endl;
    
    lockService->RootFillLock(this);
    
    int codeRow;
    int codeCol;
    
    if((par_col > 0)&&(par_row > 0))
      {
	codeCol = par_col + 19;
	codeRow = par_row + 19;
      }
    else if((par_col < 0)&&(par_row < 0))
      {
	codeCol = par_col + 20;
	codeRow = par_row + 20;
      }
    else if((par_col > 0)&&(par_row < 0))
      {
	codeCol = par_col + 19;
	codeRow = par_row + 20;
      }
    else if((par_col < 0)&&(par_row > 0))
      {
	codeCol = par_col + 20;
	codeRow = par_row + 19;
      }


    int module_found = 0;
    
    for(unsigned int ii = 0; ii < ecal_digi_hits.size(); ii++){
      const DECALDigiHit *ecal_hit = ecal_digi_hits[ii];
      
      int row      =  ecal_hit->row;
      int column   =  ecal_hit->column; 	  

      const Df250WindowRawData *windorawdata;
      const Df250PulseData     *pulsedata; 
      
      ecal_hit->GetSingle(pulsedata);         
      pulsedata->GetSingle(windorawdata);
      
      if(pulsedata){
	
	if(windorawdata){
	  
	  unsigned int max_amp = 0;
	  
	  const vector<uint16_t> &samplesvector = windorawdata->samples;
          
	  unsigned int nsamples = samplesvector.size();
	  
	  
	  for(uint16_t samp = 0; samp < nsamples; samp++){
	    
	    unsigned int adc_amp = samplesvector[samp];
	    
	    
	    if (adc_amp > max_amp) max_amp = adc_amp;
	    
	    ecal_wave_ind[column][row]->Fill(float(samp),float(adc_amp));
	    
	  }  // Loop over samples

	  if( (row == codeRow) && (column == codeCol) && (max_amp > plot_mod_thr) ) module_found = 1;	  
	}	
      }      
    }  // Loop over digi hits
    


    int ref_found = 0;
    
    for(unsigned int ii = 0; ii < ecal_ref_digihits.size(); ii++){
      const DECALRefDigiHit  *ecal_ref_hit = ecal_ref_digihits[ii];
      
      int ref_id         =  ecal_ref_hit->id;
      uint32_t  adc_time = (ecal_ref_hit->pulse_time  & 0x7FC0) >> 6;
      int ref_peak       =  ecal_ref_hit->pulse_peak;
      int pedestal       =  ecal_ref_hit->pedestal;     
      //      double peak = ref_peak - double(pedestal)/4.;
      
      if(debug)
	cout << " REF ID = " << ref_id << "  Peak = " << ref_peak << " ADC time = " << adc_time <<
	  " Pedestal = " << double(pedestal)/4. << endl;
	
	
      const Df250WindowRawData *windorawdata;
      const Df250PulseData     *pulsedata; 
      
      ecal_ref_hit->GetSingle(pulsedata);         
      pulsedata->GetSingle(windorawdata);
      
      if(pulsedata){	
	
	if(windorawdata){

	  unsigned int max_amp = 0;
	  
	  const vector<uint16_t> &samplesvector = windorawdata->samples;
              
	  unsigned int nsamples = samplesvector.size();
	  
	  
	  for(uint16_t samp = 0; samp < nsamples; samp++){
	    
	    unsigned int adc_amp = samplesvector[samp];
	    
	    
	    if (adc_amp > max_amp) max_amp = adc_amp;

	    if(ref_id == 1)
	      ecal_ref1->Fill(float(samp),float(adc_amp));
	    if(ref_id == 2)
	      ecal_ref2->Fill(float(samp),float(adc_amp));
	    
	  }  // Loop over samples

	  if (max_amp > plot_ref_thr) ref_found = 1;
	} else cout << " BAD DATA FORMAT -> NOT RAW DATA (CHECK FLASH ADC MODE)" << endl;
	
      }
    }  //  Loop over ref digi hits
    
        
    lockService->RootFillUnLock(this);	

    
    if(debug)
      cout << " Par_plot_ref = " << par_plot_ref << "  ECAL module found =  " << module_found <<
	"  REF PMT found =   "  << ref_found << endl;
    
    if(par_plot_ref == 0){
      if(module_found == 1){
	cout << "  ---------- ECAL MODULE FOUND ----------------  Event = " <<
	  event->GetEventNumber() << "    Number of hits = " <<   ecal_digi_hits.size() <<  endl;
	ecal_wave_ind[codeCol][codeRow]->Draw();
	my_canvas->Update();
	getchar();
      }
    }
    
    if(par_plot_ref == 1){
      if(ref_found == 1){
	my_canvas->Divide(1,2,0.0001,0.0001);
	
	cout << "  ---------- Reference PMT  FOUND ---------------- Event =   " <<
	  event->GetEventNumber() << endl;
	my_canvas->cd(1);
	ecal_ref1->Draw();
	my_canvas->cd(2);
	ecal_ref2->Draw();
	my_canvas->Update();
	getchar();
      }
    }
    
    my_canvas->Update();   
    my_canvas->Clear();
    
    //Reseting individual histograms    
    for(int  col = 0; col < 40; col++){
      for(int  row = 0; row < 40; row++){
	ecal_wave_ind[col][row]->Reset("ICESM");
      }
    }      
    
    ecal_ref1->Reset("ICESM");  ecal_ref2->Reset("ICESM");
    
}

//------------------
// EndRun
//------------------
void JEventProcessor_ecal_waveforms::EndRun()
{
    // This is called whenever the run number changes, before it is
    // changed to give you a chance to clean up before processing
    // events from the next run number.
}

//------------------
// Finish
//------------------
void JEventProcessor_ecal_waveforms::Finish()
{
    // Called before program exit after event processing is finished.
}

