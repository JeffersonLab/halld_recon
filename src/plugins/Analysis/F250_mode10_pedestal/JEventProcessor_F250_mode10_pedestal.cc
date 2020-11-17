// $Id$
//
//    File: JEventProcessor_F250_mode10_pedestal.cc
// Created: Mon Apr 13 14:49:24 EDT 2015
// Creator: dalton (on Linux gluon02.jlab.org 2.6.32-358.18.1.el6.x86_64 x86_64)
//
// This plugin is designed to analyze F250 pedestal data on short time scales.
// It has been used with the BCAL system but is writen generally for any F250 system.
// Take data with a random trigger in mode 8 and no sparsification.  
// 

#include "JEventProcessor_F250_mode10_pedestal.h"
using namespace jana;

#include <DAQ/Df250WindowRawData.h>

#include <TStyle.h>


// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
#include <JANA/JFactory.h>
extern "C"{
void InitPlugin(JApplication *app){
	InitJANAPlugin(app);
	app->AddProcessor(new JEventProcessor_F250_mode10_pedestal());
}
} // "C"


//------------------
// JEventProcessor_F250_mode10_pedestal (Constructor)
//------------------
JEventProcessor_F250_mode10_pedestal::JEventProcessor_F250_mode10_pedestal()
{

	NSA_NSB = 60;
	debug=0;
	
	if(gPARMS){
		gPARMS->SetDefaultParameter("F250_m10_ped:NSA_NSB", NSA_NSB, "The number of samples integrated in the signal.");
		gPARMS->SetDefaultParameter("F250_m10_ped:debug",   debug,   "Debug level");
	}

}

//------------------
// ~JEventProcessor_F250_mode10_pedestal (Destructor)
//------------------
JEventProcessor_F250_mode10_pedestal::~JEventProcessor_F250_mode10_pedestal()
{

}

//------------------
// init
//------------------
jerror_t JEventProcessor_F250_mode10_pedestal::init(void)
{
	// This is called once at program startup. If you are creating
	// and filling historgrams in this plugin, you should lock the
	// ROOT mutex like this:
	//
	// japp->RootWriteLock();
	//  ... fill historgrams or trees ...
	// japp->RootUnLock();
	//

	// lock all root operations
	japp->RootWriteLock();

	gStyle->SetTitleOffset(1, "Y");
  	gStyle->SetTitleSize(0.05,"xyz");
	gStyle->SetTitleSize(0.08,"h");
	gStyle->SetLabelSize(0.05,"xyz");
	gStyle->SetTitleX(0);
	gStyle->SetTitleAlign(13);
	gStyle->SetNdivisions(505,"xy");
	gStyle->SetPadRightMargin(0.15);

	// set all the pointers to NULL
	for (int i=0; i<highcratenum; i++) {
		mean_crates[i] = NULL;
		samples_rms_summary[i] = NULL;
		samplesum_pedsub_rms_summary[i] = NULL;
		samplesum_rms_summary[i] = NULL;
		windowSigma_crate[i] = NULL;
		for (int j=0; j<highslotnum; j++) {
			for (int k=0; k<highchannum; k++) {
				samples_meansub[i][j][k] = NULL;
				samplesum_pedsub[i][j][k] = NULL;
				samplesum[i][j][k] = NULL;
				windowSigma[i][j][k] = NULL;
			}
		}
	}

	japp->RootUnLock();
	return NOERROR;
}

//------------------
// brun
//------------------
jerror_t JEventProcessor_F250_mode10_pedestal::brun(JEventLoop *eventLoop, int32_t runnumber)
{
	// This is called whenever the run number changes
	return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t JEventProcessor_F250_mode10_pedestal::evnt(JEventLoop *loop, uint64_t eventnumber) {
	// This is called for every event. Use of common resources like writing
	// to a file or filling a histogram should be mutex protected. Using
	// loop->Get(...) to get reconstructed objects (and thereby activating the
	// reconstruction algorithm) should be done outside of any mutex lock
	// since multiple threads may call this method at the same time.
	// Here's an example:
	//
	// vector<const MyDataClass*> mydataclasses;
	// loop->Get(mydataclasses);
	//
	// japp->RootWriteLock();
	//  ... fill historgrams or trees ...
	// japp->RootUnLock();

	
	
	vector<const Df250WindowRawData*> f250WindowRawData_vec;
	loop->Get(f250WindowRawData_vec);

	japp->RootWriteLock();
	// printf("in the lock\n");

	
	unsigned int num_f250WRD = f250WindowRawData_vec.size();
	// printf("num_f250WRD: %f",num_f250WRD);
	// Loop over all  objects in this event
	for(unsigned int c_chan=0; c_chan<num_f250WRD; c_chan++){
		const Df250WindowRawData *f250WindowRawData = f250WindowRawData_vec[c_chan];
		int rocid = f250WindowRawData->rocid;
		int slot = f250WindowRawData->slot;
		int channel = f250WindowRawData->channel;
 		// Get a vector of the samples for this channel
		const vector<uint16_t> &samplesvector = f250WindowRawData->samples;
		int nsamples=samplesvector.size();

		/// create histogram if necessary
		char histname[255], histtitle[255];
		if (windowSigma[rocid][slot][channel] == NULL) {
			if (rocid > highcratenum || slot > highslotnum || channel > highchannum) {
				printf ("(%i,%i,%i) is grater than (%i,%i,%i)\n",rocid,slot,channel,highcratenum,highslotnum,highchannum);
				continue;
			}
			sprintf(histname,"windowSigma_%02i_%02i_%02i",rocid,slot,channel);
			sprintf(histtitle,"Sigma of each window (%2i,%2i,%2i)",rocid,slot,channel);
			windowSigma[rocid][slot][channel] = new TH1I(histname,histtitle,400,0.0,2.0);
		}
		if (samples_meansub[rocid][slot][channel] == NULL) {
			if (rocid > highcratenum || slot > highslotnum || channel > highchannum) {
				printf ("(%i,%i,%i) is grater than (%i,%i,%i)\n",rocid,slot,channel,highcratenum,highslotnum,highchannum);
				continue;
			}
			float lowlimit=-7, highlimit=7;
			// adjust limits to prevent binning effects
			float difference = highlimit-lowlimit;
			int numbins = nsamples*difference;
			lowlimit -= difference/numbins;
			highlimit += difference/numbins;
			sprintf(histname,"pedmeansub_%02i_%02i_%02i",rocid,slot,channel);
			sprintf(histtitle,"hit samples, mean subtracted (%2i,%2i,%2i)",rocid,slot,channel);
			//printf("CREATING:  %s %s\n",histname,histtitle);
			samples_meansub[rocid][slot][channel] = new TH1I(histname,histtitle,numbins+1,lowlimit,highlimit);
		} 
		if (samplesum_pedsub[rocid][slot][channel] == NULL) {
			float lowlimit, highlimit, difference;
			int numbins;
			lowlimit=-250; highlimit=250;
			difference = highlimit-lowlimit;
			numbins = difference;
			// adjust limits to prevent binning effects
			sprintf(histname,"samplesumpedsub_%02i_%02i_%02i",rocid,slot,channel);
			sprintf(histtitle,"empty hit signal, ped subtracted (%2i,%2i,%2i)",rocid,slot,channel);
			samplesum_pedsub[rocid][slot][channel] = new TH1I(histname,histtitle,numbins,lowlimit,highlimit);
		}
		if (samplesum[rocid][slot][channel] == NULL) {
			int numbins;
			numbins=1000;
			// adjust limits to prevent binning effects
			sprintf(histname,"samplesum_%02i_%02i_%02i",rocid,slot,channel);
			sprintf(histtitle,"empty hit signal (%2i,%2i,%2i)",rocid,slot,channel);
			samplesum[rocid][slot][channel] = new TH1I(histname,histtitle,numbins,0,0);
		}
		if (mean_crates[rocid] == NULL) {
			sprintf(histname,"mean_crate%02i",rocid);
			sprintf(histtitle,"Crate %i pedestal mean;Slot;Channel",rocid);
			mean_crates[rocid] = new TProfile2D(histname,histtitle,21,0.5,21.5,16,-0.5,15.5);
			mean_crates[rocid]->SetStats(0);
		}
		if (windowSigma_crate[rocid] == NULL) {
			sprintf(histname,"windowSigma_crate%02i",rocid);
			sprintf(histtitle,"Crate %i channel sigma;Slot;Channel",rocid);
			windowSigma_crate[rocid] = new TProfile2D(histname,histtitle,21,0.5,21.5,16,-0.5,15.5);
			windowSigma_crate[rocid]->SetStats(0);
		}
		/// loop over the samples to calculate mean, RMS and sigma
		float total=0, sumofsquares=0;
		int pedtotal=0, signaltotal=0;
		int numsamps=0;
		for (uint16_t c_samp=0; c_samp<nsamples; c_samp++) {
			if (samplesvector[c_samp]>0 && samplesvector[c_samp]<4096 ) {
				total += samplesvector[c_samp];
				sumofsquares += samplesvector[c_samp]*samplesvector[c_samp];
				numsamps++;
				if (c_samp<4) pedtotal += samplesvector[c_samp];
				if (c_samp>=4 && c_samp<(4+NSA_NSB)) signaltotal += samplesvector[c_samp];
			}
		}
		int ped = pedtotal/4;  // integer division of pedestal to emulate firmware
		int signal = signaltotal-(NSA_NSB*ped);
		//printf("%i %i %i %i\n",pedtotal,signaltotal,ped,signal);
		samplesum_pedsub[rocid][slot][channel]->Fill(signal);
		samplesum[rocid][slot][channel]->Fill(signaltotal);

		if (numsamps>0) {
			float mean = total/numsamps;
            float RMS = sqrt(sumofsquares/numsamps);
            float sigma = sqrt(RMS*RMS-mean*mean);
            if (debug>=2) if (rocid==31 && slot==3 && channel==0) printf("%5lu %2i %2i %2i  %8.0f %i mean %10f, RMS %10f, Sigma %f\n",
                                                                         eventnumber,rocid,slot,channel,total,numsamps,mean,RMS, sigma);
            windowSigma[rocid][slot][channel]->Fill(sigma);
			mean_crates[rocid]->Fill(slot,channel,mean);
			windowSigma_crate[rocid]->Fill(slot,channel,sigma);
			for (uint16_t c_samp=0; c_samp<nsamples; c_samp++) {
				if (samplesvector[c_samp]>0 && samplesvector[c_samp]<4096 ) {
					float sampsubmean=samplesvector[c_samp]-mean;
					samples_meansub[rocid][slot][channel]->Fill(sampsubmean);
					if (debug>=3) if (rocid==31 && slot==3 && channel==0) printf("\t%3i %4i %f %f\n",c_samp,samplesvector[c_samp],mean,sampsubmean);
				}
			}
		}
	}

	japp->RootUnLock();

	return NOERROR;
}

//------------------
// erun
//------------------
jerror_t JEventProcessor_F250_mode10_pedestal::erun(void)
{
	// This is called whenever the run number changes, before it is
	// changed to give you a chance to clean up before processing
	// events from the next run number.
	return NOERROR;
}

//------------------
// fini
//------------------
jerror_t JEventProcessor_F250_mode10_pedestal::fini(void)
{
	// Called before program exit after event processing is finished.

	japp->RootWriteLock();

	float RMSsum=0;
	int numchans=0;
	for (int i=0; i<highcratenum; i++) {
		for (int j=0; j<highslotnum; j++) {
			for (int k=0; k<highchannum; k++) {
				if (samples_meansub[i][j][k]!=NULL) {
					float RMS = samples_meansub[i][j][k]->GetRMS();
					float integRMS_pedsub = samplesum_pedsub[i][j][k]->GetRMS();
					float integRMS = samplesum[i][j][k]->GetRMS();
					RMSsum+=RMS;
					numchans++;
					// make summary histograms
					if (samples_rms_summary[i]==NULL) {
						printf("JEventProcessor_F250_mode10_pedestal::fini  creating histograms for crate %i\n",i);
						char cratename[255],title[255];
						sprintf(cratename,"samples_rms_summary_crate%i",i);
						sprintf(title,"Crate %i pedestal sample RMS (F250);Slot;Channel",i);
						samples_rms_summary[i] = new TH2D(cratename,title,21,0.5,21.5,16,-0.5,15.5);
						samples_rms_summary[i]->SetStats(0);
						sprintf(cratename,"samplesum_rms_summary_crate%i",i);
						sprintf(title,"Crate %i: %i-sample signal RMS (F250);Slot;Channel",i,NSA_NSB);
						samplesum_rms_summary[i] = new TH2D(cratename,title,21,0.5,21.5,16,-0.5,15.5);
						samplesum_rms_summary[i]->SetStats(0);
						sprintf(cratename,"samplesum_pedsub_rms_summary_crate%i",i);
						sprintf(title,"Crate %i: %i-sample signal ped. sub. RMS (F250);Slot;Channel",i,NSA_NSB);
						samplesum_pedsub_rms_summary[i] = new TH2D(cratename,title,21,0.5,21.5,16,-0.5,15.5);
						samplesum_pedsub_rms_summary[i]->SetStats(0);
					}
					samples_rms_summary[i]->SetBinContent(j,k+1,RMS);
					samplesum_rms_summary[i]->SetBinContent(j,k+1,integRMS);
					samplesum_pedsub_rms_summary[i]->SetBinContent(j,k+1,integRMS_pedsub);
					//printf("%2i %2i %2i %f\n",i,j,k,RMS);
				} 
			}
		}
	}
	for (int i=0; i<highcratenum; i++) {
		if (samples_rms_summary[i]!=NULL) {
			float min = samples_rms_summary[i]->GetMinimum(0.1);
			float max = samples_rms_summary[i]->GetMaximum();
			printf("crate %i: RMS of channels:  min %f  max %f\n",i,min,max);
			//samples_rms_summary[i]->SetMinimum(0.9);
			//samples_rms_summary[i]->SetMaximum(1.7);
		}
	}
	printf("mean RMS = %f\n",RMSsum/numchans);

	japp->RootUnLock();

	return NOERROR;
}

