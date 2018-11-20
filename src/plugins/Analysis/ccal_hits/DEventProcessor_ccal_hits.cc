// $Id$
//
//    File: DEventProcessor_ccal_hits.cc
// Created: Mon Apr  3 11:38:03 EDT 2006
// Creator: davidl (on Darwin swire-b241.jlab.org 8.4.0 powerpc)
//

#include <map>
using namespace std;

#include "DEventProcessor_ccal_hits.h"

#include <TLorentzVector.h>

#include <DANA/DApplication.h>
#include <CCAL/DCCALDigiHit.h>

#include <DAQ/Df250PulseIntegral.h>
#include <DAQ/Df250PulseData.h>
#include <DAQ/Df250WindowRawData.h>
#include <DAQ/Df250PulsePedestal.h>




// Routine used to create our DEventProcessor
extern "C"{
void InitPlugin(JApplication *app){
	InitJANAPlugin(app);
	app->AddProcessor(new DEventProcessor_ccal_hits());
}
} // "C"


//------------------
// init
//------------------
jerror_t DEventProcessor_ccal_hits::init(void)
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
           
      int column_tmp, row_tmp;
      
      if(ii < 6) column_tmp = 6 - ii; 
      else column_tmp = 5 - ii;

      if(jj < 6) row_tmp = jj - 6; 
      else row_tmp = jj - 5;    

      char title[30];
      char title_col[10];
      char title_row[10];

      int index = ii*12 + jj;


      if(column_tmp > 0) sprintf(title_col,"%d",column_tmp);
      else sprintf(title_col,"n%d",-column_tmp);

      if(row_tmp > 0) sprintf(title_row,"%d",row_tmp);
      else sprintf(title_row,"n%d",-row_tmp);

      sprintf(title,"Waveform_%s_%s",title_col, title_row);    
      ccal_wave[index]  = new TProfile(title,title,100,-0.5,99.5,-10.,4096);

      sprintf(title,"Peak_%s_%s", title_col, title_row);    
      ccal_peak[index]  = new TH1F(title, title, 4096, -0.5, 4095.5);

      sprintf(title,"Int_%s_%s", title_col, title_row);    
      ccal_int[index]  = new TH1F(title, title, 500, 0., 20000.5);

    }
  }


  // Go back up to the parent directory
  dir->cd("../");


	return NOERROR;
}

//------------------
// brun
//------------------
jerror_t DEventProcessor_ccal_hits::brun(JEventLoop *eventLoop, int32_t runnumber)
{

	return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t DEventProcessor_ccal_hits::evnt(JEventLoop *loop, uint64_t eventnumber)
{

	vector<const DCCALDigiHit*> ccal_digihits;

	loop->Get(ccal_digihits);

	cout << " Event number = " << eventnumber  <<  endl;
	cout << " Number of hits = " << ccal_digihits.size() <<  endl;

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


	japp->RootWriteLock(); 


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


	  // Index for histograms

	  int column_tmp, row_tmp;

	  if(column[nhit] > 0) column_tmp = 6 - column[nhit];
	  else column_tmp = 5 - column[nhit];

	  if(row[nhit] > 0) row_tmp = 5 + row[nhit];
	  else row_tmp = 6 + row[nhit];
	  
	  int index = row_tmp*12 + column_tmp;


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

	japp->RootUnLock();

	return NOERROR;
}

//------------------
// erun
//------------------
jerror_t DEventProcessor_ccal_hits::erun(void)
{
	// Any final calculations on histograms (like dividing them)
	// should be done here. This may get called more than once.
	return NOERROR;
}

//------------------
// fini
//------------------
jerror_t DEventProcessor_ccal_hits::fini(void)
{
	return NOERROR;
}

