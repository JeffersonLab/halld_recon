// $Id$
//
//    File: DEventProcessor_ccal_hits.cc
// Created: Mon Apr  3 11:38:03 EDT 2006
// Creator: davidl (on Darwin swire-b241.jlab.org 8.4.0 powerpc)
//

#include <map>
using namespace std;

#include "DEventProcessor_ccal_hits.h"

#include <DANA/DEvent.h>
#include <CCAL/DCCALDigiHit.h>

#include <CCAL/DCCALHit.h>

#include <DAQ/Df250WindowRawData.h>
#include <DAQ/Df250PulseData.h>


// Routine used to create our DEventProcessor
extern "C"{
void InitPlugin(JApplication *app){
	InitJANAPlugin(app);
	app->Add(new DEventProcessor_ccal_hits());
}
} // "C"


//------------------
// Init
//------------------
void DEventProcessor_ccal_hits::Init()
{
         
  TDirectory *dir = new TDirectoryFile("CCAL","CCAL");
  dir->cd();
 
  tree1 = new TTree( "digihit", "digihit" );

  tree1->Branch("nhit", &nhit, "nhit/I");
  tree1->Branch("column", &column, "column[nhit]/I");
  tree1->Branch("row", &row, "row[nhit]/I");
  tree1->Branch("peak", &peak, "peak[nhit]/I");
  tree1->Branch("integral", &integral, "integral[nhit]/I");
  tree1->Branch("pedestal", &pedestal, "pedestal[nhit]/I");
  tree1->Branch("time", &time, "time[nhit]/I");
  tree1->Branch("qf", &qf, "qf[nhit]/I");

  tree1->Branch("waveform", &waveform, "waveform[nhit][100]/I");

  tree1->Branch("nsamples_integral", &nsamples_integral, "nsamples_integral/I");
  tree1->Branch("nsamples_pedestal", &nsamples_pedestal, "nsamples_pedestal/I");



  for(Int_t ii = 0; ii < 12; ii++){
    for(Int_t jj = 0; jj < 12; jj++){
                 
      char title[30];
      
      int index = jj + ii*12;
      
      sprintf(title,"Waveform_%d",index);    
      ccal_wave[index]  = new TProfile(title,title,100,-0.5,99.5,-10.,4096);

      sprintf(title,"Peak_%d",index); 
      ccal_peak[index]  = new TH1F(title, title, 4096, -0.5, 4095.5);
      
      sprintf(title,"Int_%d",index); 
      ccal_int[index]  = new TH1F(title, title, 500, 0., 20000.5);

    }
  }


  // Go back up to the parent directory
  dir->cd("../");

}

//------------------
// BeginRun
//------------------
void DEventProcessor_ccal_hits::BeginRun(const std::shared_ptr<const JEvent>& event)
{
}

//------------------
// Process
//------------------
void DEventProcessor_ccal_hits::Process(const std::shared_ptr<const JEvent>& event)
{
	
	vector<const DCCALDigiHit*> ccal_digihits;

	vector<const DCCALHit*> ccal_hits;

	event->Get(ccal_digihits);
	event->Get(ccal_hits);

	cout << " Event number = " << event->GetEventNumber()  <<  endl;
	cout << " Number of digi hits = " << ccal_digihits.size() <<  endl;
	cout << " Number of hits = " << ccal_hits.size() <<  endl;


	nhit = 0;
	memset(column,0,sizeof(column));
	memset(row,0,sizeof(row));
	memset(peak,0,sizeof(peak));
	memset(integral,0,sizeof(integral));
	memset(pedestal,0,sizeof(pedestal));
	memset(time,0,sizeof(time));
	memset(qf,0,sizeof(qf));
       	memset(waveform,0,sizeof(waveform));
       	nsamples_integral = 0;
	nsamples_pedestal = 0;


	DEvent::GetLockService(event)->RootWriteLock();


        for(unsigned int ii = 0; ii < ccal_digihits.size(); ii++){
          const DCCALDigiHit *ccal_hit = ccal_digihits[ii];          

	  if(ii == 0){
	    nsamples_integral = ccal_hit->nsamples_integral;
	    nsamples_pedestal = ccal_hit->nsamples_pedestal;
	  }
  
	  row[nhit]     =  ccal_hit->row;
	  column[nhit]  =  ccal_hit->column; 
	  
	  peak[nhit]      =  ccal_hit->pulse_peak; 
	  integral[nhit]  =  ccal_hit->pulse_integral; 

	  pedestal[nhit]  =  ccal_hit->pedestal;
	  time[nhit]      =  (ccal_hit->pulse_time  & 0x7FC0) >> 6;
	  qf[nhit]        =  ccal_hit->QF;
	  
	  int index = ccal_hit->row*12 + ccal_hit->column;
	  
	  // Assume that baseline is at fadc count 100
	  ccal_peak[index]->Fill(float(ccal_hit->pulse_peak) - 100.);
	  ccal_int[index]->Fill(float(ccal_hit->pulse_integral) - 100*nsamples_integral);
	  

	  const Df250WindowRawData *windorawdata;
	  const Df250PulseData  *pulsedata; 

          ccal_hit->GetSingle(pulsedata);

	  if(pulsedata){;

	    pulsedata->GetSingle(windorawdata);
	    
	    if(windorawdata){
	      
	      const vector<uint16_t> &samplesvector = windorawdata->samples;
	      
	      unsigned int nsamples = samplesvector.size();
	      
	      for(uint16_t samp = 0; samp < nsamples; samp++){
		waveform[nhit][samp] = samplesvector[samp];

		ccal_wave[index]->Fill(float(samp),float(samplesvector[samp]));

	      } 
	      
	    }          
	    
	  }
	  
	  
	  nhit++;

	}


	if(nhit > 0){
	  tree1->Fill();
	}

	DEvent::GetLockService(event)->RootUnLock();
}

//------------------
// EndRun
//------------------
void DEventProcessor_ccal_hits::EndRun()
{
	// Any final calculations on histograms (like dividing them)
	// should be done here. This may get called more than once.
}

//------------------
// Finish
//------------------
void DEventProcessor_ccal_hits::Finish()
{
}

