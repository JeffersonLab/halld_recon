// $Id$
//    File: DEventProcessor_fcal_led.cc


#include <map>
using namespace std;

#include "DEventProcessor_fcal_led.h"

#include <DANA/DEvent.h>
#include <FCAL/DFCALDigiHit.h>

#include <DAQ/Df250PulseIntegral.h>
#include <DAQ/Df250PulseData.h>
#include <DAQ/Df250WindowRawData.h>
#include <DAQ/Df250PulsePedestal.h>




// Routine used to create our DEventProcessor
extern "C"{
void InitPlugin(JApplication *app){
	InitJANAPlugin(app);
	app->Add(new DEventProcessor_fcal_led());
}
} // "C"


//------------------
// Init
//------------------
void DEventProcessor_fcal_led::Init()
{
         
  TDirectory *dir = new TDirectoryFile("FCAL","FCAL");
  dir->cd();
 
  tree1 = new TTree( "digihit", "digihit" );

  cout << " SASCHA I AM HERE " << endl;


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



  for(Int_t ii = 0; ii < 59; ii++){
    for(Int_t jj = 0; jj < 59; jj++){
           
      char title[30];
      char title1[30];

      sprintf(title,"Waveform_%d_%d",ii, jj);    
      fcal_wave[ii][jj]  = new TProfile(title,title,100,-0.5,99.5,-10.,4096);

      sprintf(title1,"Peak_%d_%d", ii, jj);    
      fcal_peak[ii][jj]  = new TH1F(title1, title1, 4096, -0.5, 4095.5);

      //      sprintf(title,"Int_%d_%d", ii, jj);    
      //      fcal_int[ii][jj]  = new TH1F(title, title, 500, 0., 20000.5);

    }
  }


  // Go back up to the parent directory
  dir->cd("../");


	return;
}

//------------------
// BeginRun
//------------------
void DEventProcessor_fcal_led::BeginRun(const std::shared_ptr<const JEvent>& event)
{

	return;
}

//------------------
// Process
//------------------
void DEventProcessor_fcal_led::Process(const std::shared_ptr<const JEvent>& event)
{


	
	vector<const DFCALDigiHit*> fcal_digihits;

	event->Get(fcal_digihits);

	cout << " Event number = " << event->GetEventNumber()  <<  endl;
	cout << " Number of hits = " << fcal_digihits.size() <<  endl;

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


        for(unsigned int ii = 0; ii < fcal_digihits.size(); ii++){
          const DFCALDigiHit *fcal_hit = fcal_digihits[ii];          

	  if(ii == 0){
	    nsamples_integral = fcal_hit->nsamples_integral;
	    nsamples_pedestal = fcal_hit->nsamples_pedestal;
	  }
  
	  row[nhit]     =  fcal_hit->row;
	  column[nhit]  =  fcal_hit->column; 
	  
	  peak[nhit]      =  fcal_hit->pulse_peak; 
	  integral[nhit]  =  fcal_hit->pulse_integral; 

	  pedestal[nhit]  =  fcal_hit->pedestal;
	  time[nhit]      =  (fcal_hit->pulse_time  & 0x7FC0) >> 6;
	  qf[nhit]        =  fcal_hit->QF;


	  // Index for histograms


	  // Assume that baseline is at fadc count 100
	  fcal_peak[column[nhit]][row[nhit]]->Fill(float(fcal_hit->pulse_peak) - 100.);
	  //	  fcal_int[column[nhit]][row[nhit]]->Fill(float(fcal_hit->pulse_integral) - 100*nsamples_integral);
	  

	  const Df250WindowRawData *windorawdata;
	  const Df250PulseData  *pulsedata; 

          fcal_hit->GetSingle(pulsedata);

	  if(pulsedata){;

	    pulsedata->GetSingle(windorawdata);
	    
	    if(windorawdata){
	      
	      const vector<uint16_t> &samplesvector = windorawdata->samples;
	      
	      unsigned int nsamples = samplesvector.size();
	      
	      for(uint16_t samp = 0; samp < nsamples; samp++){
		waveform[nhit][samp] = samplesvector[samp];

		fcal_wave[column[nhit]][row[nhit]]->Fill(float(samp),float(samplesvector[samp]));

	      } 
	      
	    }          
	    
	  }
	  
	  if(nhit < 4000)	    
	    nhit++;
	  else cout << "Too many hits " << endl;
 
	}


	if(nhit > 0){
	  tree1->Fill();
	}

	DEvent::GetLockService(event)->RootUnLock();
}

//------------------
// EndRun
//------------------
void DEventProcessor_fcal_led::EndRun()
{
	// Any final calculations on histograms (like dividing them)
	// should be done here. This may get called more than once.
}

//------------------
// Finish
//------------------
void DEventProcessor_fcal_led::Finish()
{
}

