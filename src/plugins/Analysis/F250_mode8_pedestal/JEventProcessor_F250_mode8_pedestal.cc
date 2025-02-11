// $Id$
//
//    File: JEventProcessor_F250_mode8_pedestal.cc
// Created: Mon Apr 13 14:49:24 EDT 2015
// Creator: dalton (on Linux gluon02.jlab.org 2.6.32-358.18.1.el6.x86_64 x86_64)
//
// This plugin is designed to analyze F250 pedestal data on short time scales.
// It has been used with the BCAL system but is writen generally for any F250 system.
// Take data with a random trigger in mode 8 and no sparsification.  
// 

#include "JEventProcessor_F250_mode8_pedestal.h"

#include <DAQ/Df250WindowRawData.h>

#include <TStyle.h>


// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
#include <JANA/JFactoryT.h>
extern "C"{
void InitPlugin(JApplication *app){
	InitJANAPlugin(app);
	app->Add(new JEventProcessor_F250_mode8_pedestal());
}
} // "C"


//------------------
// JEventProcessor_F250_mode8_pedestal (Constructor)
//------------------
JEventProcessor_F250_mode8_pedestal::JEventProcessor_F250_mode8_pedestal()
{
	SetTypeName("JEventProcessor_F250_mode8_pedestal");
}

//------------------
// ~JEventProcessor_F250_mode8_pedestal (Destructor)
//------------------
JEventProcessor_F250_mode8_pedestal::~JEventProcessor_F250_mode8_pedestal()
{

}

//------------------
// Init
//------------------
void JEventProcessor_F250_mode8_pedestal::Init()
{
	// This is called once at program startup. If you are creating
	// and filling historgrams in this plugin, you should lock the
	// ROOT mutex like this:
	//
	// GetLockService(locEvent)->RootWriteLock();
	//  ... fill historgrams or trees ...
	// GetLockService(locEvent)->RootUnLock();
	//

	NSA_NSB = 60;
	auto app = GetApplication();
	app->SetDefaultParameter("F250_m8_ped:NSA_NSB", NSA_NSB, "The number of samples integrated in the signal.");

	// lock all root operations
	GetLockService(locEvent)->RootWriteLock();

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
		samples_rms_summary[i] = NULL;
		samplesum_rms_summary[i] = NULL;
		for (int j=0; j<highslotnum; j++) {
			for (int k=0; k<highchannum; k++) {
				samples_meansub[i][j][k] = NULL;
				samplesum_pedsub[i][j][k] = NULL;
				samplesum[i][j][k] = NULL;
				mean_crates[i] = NULL;
			}
		}
	}

	GetLockService(locEvent)->RootUnLock();
}

//------------------
// BeginRun
//------------------
void JEventProcessor_F250_mode8_pedestal::BeginRun(const std::shared_ptr<const JEvent> &event)
{
	// This is called whenever the run number changes
}

//------------------
// Process
//------------------
void JEventProcessor_F250_mode8_pedestal::Process(const std::shared_ptr<const JEvent> &event)
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

	std::vector<const Df250WindowRawData*> f250WindowRawData_vec;
	event->Get(f250WindowRawData_vec);

	GetLockService(event)->RootWriteLock();
	//printf("in the lock\n");

	unsigned int num_f250WRD = f250WindowRawData_vec.size();
	// Loop over all  objects in this event
	for(unsigned int c_chan=0; c_chan<num_f250WRD; c_chan++){
		const Df250WindowRawData *f250WindowRawData = f250WindowRawData_vec[c_chan];
		int rocid = f250WindowRawData->rocid;
		int slot = f250WindowRawData->slot;
		int channel = f250WindowRawData->channel;
 		// Get a vector of the samples for this channel
		const std::vector<uint16_t> &samplesvector = f250WindowRawData->samples;
		int nsamples=samplesvector.size();

		/// create histogram if necessary
		char histname[255], histtitle[255];
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
			float lowlimit, highlimit, difference;
			int numbins;
			// lowlimit=-250; highlimit=250;
			// lowlimit+=100*nsa_nsb;
			// highlimit+=100*nsa_nsb;
			// difference = highlimit-lowlimit;
			// numbins = difference;
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
		/// loop over the samples to calculate mean
		float total=0;
		int pedtotal=0, signaltotal=0;
		int numsamps=0;
		for (uint16_t c_samp=0; c_samp<nsamples; c_samp++) {
			if (samplesvector[c_samp]>0 && samplesvector[c_samp]<4096 ) {
				total += samplesvector[c_samp];
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
			mean_crates[rocid]->Fill(slot,channel,mean);
			for (uint16_t c_samp=0; c_samp<nsamples; c_samp++) {
				if (samplesvector[c_samp]>0 && samplesvector[c_samp]<4096 ) {
					samples_meansub[rocid][slot][channel]->Fill(samplesvector[c_samp]-mean);
				}
			}
		}
	}

	GetLockService(event)->RootUnLock();
}

//------------------
// EndRun
//------------------
void JEventProcessor_F250_mode8_pedestal::EndRun()
{
	// This is called whenever the run number changes, before it is
	// changed to give you a chance to clean up before processing
	// events from the next run number.
}

//------------------
// Finish
//------------------
void JEventProcessor_F250_mode8_pedestal::Finish()
{
	// Called before program exit after event processing is finished.

	GetLockService(locEvent)->RootWriteLock();

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
						printf("JEventProcessor_F250_mode8_pedestal::fini  creating histograms for crate %i\n",i);
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
			printf("%i min %f max %f\n",i,min,max);
			//samples_rms_summary[i]->SetMinimum(0.9);
			//samples_rms_summary[i]->SetMaximum(1.7);
		}
	}
	printf("mean RMS = %f\n",RMSsum/numchans);

	GetLockService(locEvent)->RootUnLock();
}

