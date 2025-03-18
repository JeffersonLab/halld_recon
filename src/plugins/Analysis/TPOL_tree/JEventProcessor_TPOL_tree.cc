// $Id$
//
//    File: JEventProcessor_TPOL_tree.cc
// Created: Thu Feb  4 16:11:54 EST 2016
// Creator: nsparks (on Linux cua2.jlab.org 3.10.0-327.4.4.el7.x86_64 x86_64)
//
#include <cmath>
#include <stdint.h>
#include "JEventProcessor_TPOL_tree.h"
using namespace std;

#include <TRIGGER/DL1Trigger.h>
#include <TPOL/DTPOLHit_factory.h>
#include <PAIR_SPECTROMETER/DPSCPair.h>
#include <PAIR_SPECTROMETER/DPSPair.h>
#include <TAGGER/DTAGHHit.h>
#include <TAGGER/DTAGMHit.h>
#include <PID/DBeamPhoton.h>
#include <DANA/DEvent.h>

const int NSECTORS = DTPOLHit_factory::NSECTORS;
const double SECTOR_DIVISION = DTPOLHit_factory::SECTOR_DIVISION;

// Routine used to create our JEventProcessor
#include <JANA/JFactoryT.h>
extern "C"{
    void InitPlugin(JApplication *app){
        InitJANAPlugin(app);
        app->Add(new JEventProcessor_TPOL_tree());
    }
} // "C"

/**bool JEventProcessor_TPOL_tree::DTPOLSectorHit_fadc_cmp(const DTPOLSectorDigiHit *a,const DTPOLSectorDigiHit *b){
    if (a->sector==b->sector) return (a->pulse_time<b->pulse_time);
    return (a->sector<b->sector);
}
bool JEventProcessor_TPOL_tree::DTPOLRingHit_fadc_cmp(const DTPOLRingDigiHit *a,const DTPOLRingDigiHit *b){
    if (a->ring==b->ring) return (a->pulse_time<b->pulse_time);
    return (a->ring<b->ring);
}**/


//------------------
// JEventProcessor_TPOL_tree (Constructor)
//------------------
JEventProcessor_TPOL_tree::JEventProcessor_TPOL_tree()
{
	SetTypeName("JEventProcessor_TPOL_tree");
}

//------------------
// ~JEventProcessor_TPOL_tree (Destructor)
//------------------
JEventProcessor_TPOL_tree::~JEventProcessor_TPOL_tree()
{

}

//------------------
// Init
//------------------
void JEventProcessor_TPOL_tree::Init()
{
    // This is called once at program startup. If you are creating
    // and filling historgrams in this plugin, you should lock the
    // ROOT mutex like this:
    //
    
    unsigned int ntag = 1000000;    
    dTreeInterface = DTreeInterface::Create_DTreeInterface("TPOL_tree","tree_TPOL.root");
    DTreeBranchRegister locTreeBranchRegister;

    locTreeBranchRegister.Register_Single<UShort_t>("nadc");
    locTreeBranchRegister.Register_Single<ULong64_t>("eventnum");
    locTreeBranchRegister.Register_FundamentalArray<UShort_t>("rocid","nadc",10*NSECTORS);
    locTreeBranchRegister.Register_FundamentalArray<UShort_t>("slot","nadc",10*NSECTORS);
    locTreeBranchRegister.Register_FundamentalArray<UShort_t>("channel","nadc",10*NSECTORS);
    locTreeBranchRegister.Register_FundamentalArray<UInt_t>("itrigger","nadc",10*NSECTORS);
    locTreeBranchRegister.Register_FundamentalArray<UInt_t>("sector","nadc",10*NSECTORS);
    locTreeBranchRegister.Register_FundamentalArray<Double_t>("phi","nadc",10*NSECTORS);
    locTreeBranchRegister.Register_FundamentalArray<ULong64_t>("w_integral","nadc",10*NSECTORS);
    locTreeBranchRegister.Register_FundamentalArray<UShort_t>("w_max","nadc",10*NSECTORS);
    locTreeBranchRegister.Register_FundamentalArray<UShort_t>("w_min","nadc",10*NSECTORS);
    locTreeBranchRegister.Register_FundamentalArray<UShort_t>("w_samp1","nadc",10*NSECTORS);
    locTreeBranchRegister.Register_Single<UShort_t>("ntpol");
    locTreeBranchRegister.Register_FundamentalArray<UShort_t>("waveform","ntpol",10*NSECTORS*150);
    locTreeBranchRegister.Register_Single<Double_t>("t_lhit");
    locTreeBranchRegister.Register_Single<Double_t>("t_rhit");
    locTreeBranchRegister.Register_Single<Double_t>("E_lhit");
    locTreeBranchRegister.Register_Single<Double_t>("E_rhit");
    locTreeBranchRegister.Register_Single<UShort_t>("ntag");
    locTreeBranchRegister.Register_FundamentalArray<Bool_t>("is_tagm","ntag",ntag);
    locTreeBranchRegister.Register_FundamentalArray<Bool_t>("is_DBeam","ntag",ntag);
    locTreeBranchRegister.Register_FundamentalArray<Double_t>("E_tag","ntag",ntag);
    locTreeBranchRegister.Register_FundamentalArray<Double_t>("t_tag","ntag",ntag);
    dTreeInterface->Create_Branches(locTreeBranchRegister);

    count = 0;
    //
}

//------------------
// BeginRun
//------------------
void JEventProcessor_TPOL_tree::BeginRun(const std::shared_ptr<const JEvent>& event)
{
    // This is called whenever the run number changes
}

//------------------
// Process
//------------------
void JEventProcessor_TPOL_tree::Process(const std::shared_ptr<const JEvent>& event)
{
    // This is called for every event. Use of common resources like writing
    // to a file or filling a histogram should be mutex protected. Using
    // event->Get(...) to get reconstructed objects (and thereby activating the
    // reconstruction algorithm) should be done outside of any mutex lock
    // since multiple threads may call this method at the same time.
    //

    auto eventnumber = event->GetEventNumber();

    //if (count > 10000) return;

    const DL1Trigger *trig_words = NULL;
    uint32_t trig_mask, fp_trig_mask;
    try {
        event->GetSingle(trig_words);
    } catch(...) {};
    if (trig_words) {
        trig_mask = trig_words->trig_mask;
        fp_trig_mask = trig_words->fp_trig_mask;
    }
    else {
        trig_mask = 0;
        fp_trig_mask = 0;
    }
    int trig_bits = fp_trig_mask > 0 ? 10 + fp_trig_mask:trig_mask;
    // skim PS triggers
    if (trig_bits!=8) {
        return;
    }
    //
    //vector<const DTPOLHit*> hits;
    //event->Get(hits);
    vector<const DTPOLHit*> tpolhits;
    event->Get(tpolhits);
    //sort(sectordigihits.begin(),sectordigihits.end(),DTPOLSectorHit_fadc_cmp);

    //vector<const Df250WindowRawData*> windowraws;
    //event->Get(windowraws);
    // coarse PS pairs
    vector<const DPSCPair*> cpairs;
    event->Get(cpairs);
    // fine PS pairs
    vector<const DPSPair*> fpairs;
    event->Get(fpairs);
    // tagger hits
    vector<const DTAGHHit*> taghhits;
    event->Get(taghhits);
    vector<const DTAGMHit*> tagmhits;
    event->Get(tagmhits);
    // Get beam photons
    vector<const DBeamPhoton*> beamPhotons;
    event->Get(beamPhotons);

    GetLockService(event)->RootFillLock(this);
    // PSC coincidences
    if (cpairs.size()>=1) {
        // take pair with smallest time difference from sorted vector
        const DPSCHit* clhit = cpairs[0]->ee.first; // left hit in coarse PS
        const DPSCHit* crhit = cpairs[0]->ee.second;// right hit in coarse PS
        // PSC,PS coincidences
        if (fpairs.size()>=1) {
            count++;
            // take pair with smallest time difference from sorted vector
            const DPSPair::PSClust* flhit = fpairs[0]->ee.first;  // left hit in fine PS
            const DPSPair::PSClust* frhit = fpairs[0]->ee.second; // right hit in fine PS
            double E_lhit = flhit->E; Double_t E_rhit = frhit->E;
            double t_lhit = clhit->t; Double_t t_rhit = crhit->t;
	    double E_pair = flhit->E+frhit->E;	  
 
	    dTreeFillData.Fill_Single<ULong64_t>("eventnum",eventnumber);
	    dTreeFillData.Fill_Single<Double_t>("E_lhit",E_lhit);
            dTreeFillData.Fill_Single<Double_t>("E_rhit",E_rhit);
	    dTreeFillData.Fill_Single<Double_t>("t_lhit",t_lhit);
	    dTreeFillData.Fill_Single<Double_t>("t_rhit",t_rhit);

            // PSC,PS,TAGX coincidences
            unsigned int ntag = 0;
            double EdiffMax = 12.0; double tdiffMax = 15.0;
            for (unsigned int i=0; i < taghhits.size(); i++) {
                const DTAGHHit* tag = taghhits[i];
                if (!tag->has_TDC||!tag->has_fADC) continue;
                if (fabs(E_pair-tag->E) < EdiffMax && fabs(t_lhit-tag->t) < tdiffMax && ntag < ntag_max) {
		    dTreeFillData.Fill_Array<Double_t>("E_tag",tag->E,ntag);
		    dTreeFillData.Fill_Array<Double_t>("t_tag",tag->t,ntag);
		    dTreeFillData.Fill_Array<Bool_t>("is_tagm",false,ntag);

		    unsigned int same = 0;
		    for (unsigned int j=0; j < beamPhotons.size(); j++)
		    {
			const DTAGHHit* tagh = NULL;
			beamPhotons[j]->GetSingle(tagh);
			if (tagh == NULL) continue;
			if (tagh->t != tag->t || tagh->E != tag->E || tagh->counter_id != tag->counter_id) continue;
			same++;
		    }

		    if (same == 0) dTreeFillData.Fill_Array<Bool_t>("is_DBeam",false,ntag);
		    else dTreeFillData.Fill_Array<Bool_t>("is_DBeam",true,ntag);
    		    ntag++;
	            }
            }
            for (unsigned int i=0; i < tagmhits.size(); i++) {
                const DTAGMHit* tag = tagmhits[i];
                if (!tag->has_TDC||!tag->has_fADC) continue;
                if (tag->row!=0) continue;
                if (fabs(E_pair-tag->E) < EdiffMax && fabs(t_lhit-tag->t) < tdiffMax && ntag < ntag_max) {
                    dTreeFillData.Fill_Array<Double_t>("E_tag",tag->E,ntag);
                    dTreeFillData.Fill_Array<Double_t>("t_tag",tag->t,ntag);
                    dTreeFillData.Fill_Array<Bool_t>("is_tagm",true,ntag);

		   unsigned int same = 0;
                    for (unsigned int j=0; j < beamPhotons.size(); j++)
                    {
                        const DTAGMHit* tagm = NULL;
                        beamPhotons[j]->GetSingle(tagm);
			if (tagm == NULL) continue;
                        if (tagm->t != tag->t || tagm->E != tag->E || tagm->column != tag->column) continue;
                        same++;
                    }

                    if (same == 0) dTreeFillData.Fill_Array<Bool_t>("is_DBeam",false,ntag);
                    else dTreeFillData.Fill_Array<Bool_t>("is_DBeam",true,ntag);
            	    ntag++;
		    }
            }
            dTreeFillData.Fill_Single<UShort_t>("ntag",ntag);
 
	    unsigned int hit = 0;
	    unsigned int ntpol = 0;
	    //ULong64_t w_integral = 0;
	    //unsigned int w_max = 0;
	    //unsigned int w_min = 0;
	    //unsigned int w_samp1 = 0;
            //for(unsigned int i=0; i< windowraws.size(); i++) {
            //    const Df250WindowRawData *windowraw = windowraws[i];
            //    if (windowraw->rocid!=84) continue;
            //    if (!(windowraw->slot==13||windowraw->slot==14||windowraw->slot==15||windowraw->slot==16)) continue; // azimuthal sectors, rings: 15,16

	    //for (unsigned int i = 0; i < sectordigihits.size(); i++){
	    //    const DTPOLSectorDigiHit* sectordigihit = sectordigihits[i];

	    for (unsigned int i = 0; i < tpolhits.size(); i++){
		const DTPOLHit *tpolhit = tpolhits[i];

        	vector<const Df250WindowRawData*> windowraws;
		tpolhit->Get(windowraws);
		if (windowraws.size() < 1) continue;
		const Df250WindowRawData* windowraw = windowraws[0];

		unsigned int rocid = windowraw->rocid;
                unsigned int slot = windowraw->slot;
                unsigned int channel = windowraw->channel;
                unsigned int itrigger = windowraw->itrigger;
		unsigned int sector = tpolhit->sector;
		double phi = GetPhi(sector); 

		// Get a vector of the samples for this channel
                const vector<uint16_t> &samplesvector = windowraw->samples;
                unsigned int nsamples = samplesvector.size();
                // loop over the samples to calculate integral, min, max
                if (nsamples==0) jerr << "Raw samples vector is empty." << endl;
                if (samplesvector[0] > 133.0) continue; // require first sample below readout threshold
                for (uint16_t c_samp=0; c_samp<nsamples; c_samp++) {
                    dTreeFillData.Fill_Array<UShort_t>("waveform",samplesvector[c_samp],ntpol);
		    ntpol++;
		    /**if (c_samp==0) {  // use first sample for initialization
                        w_integral = samplesvector[0];
                        w_min = samplesvector[0];
                        w_max = samplesvector[0];
                        w_samp1 = samplesvector[0];
                    } else {
                        w_integral += samplesvector[c_samp];
                        if (w_min > samplesvector[c_samp]) w_min = samplesvector[c_samp];
                        if (w_max < samplesvector[c_samp]) w_max = samplesvector[c_samp];
                    }**/
                }

		dTreeFillData.Fill_Array<UShort_t>("rocid",rocid,hit);
		dTreeFillData.Fill_Array<UShort_t>("slot",slot,hit);
		dTreeFillData.Fill_Array<UShort_t>("channel",channel,hit);
	        dTreeFillData.Fill_Array<UInt_t>("itrigger",itrigger,hit);
		dTreeFillData.Fill_Array<UInt_t>("sector",sector,hit);
		dTreeFillData.Fill_Array<Double_t>("phi",phi,hit);
		dTreeFillData.Fill_Array<ULong64_t>("w_integral",tpolhit->integral,hit);
		dTreeFillData.Fill_Array<UShort_t>("w_max",tpolhit->w_max,hit);
		dTreeFillData.Fill_Array<UShort_t>("w_min",tpolhit->w_min,hit);
		dTreeFillData.Fill_Array<UShort_t>("w_samp1",tpolhit->w_samp1,hit);
                hit++;
            }
            UShort_t nadc = hit;
            if (nadc>NSECTORS) jerr << "nathan_TPOL_tree plugin error: nadc exceeds nmax(" << NSECTORS << ")." << endl;
	    dTreeFillData.Fill_Single<UShort_t>("nadc",nadc);
   	    dTreeFillData.Fill_Single<UShort_t>("ntpol",ntpol);

   	    //if (nadc == 0 || ntag == 0) return;

   	    dTreeInterface->Fill(dTreeFillData);
   	    count++;

        }
    }
    GetLockService(event)->RootFillUnLock(this);
}

double JEventProcessor_TPOL_tree::GetPhi(unsigned int sector)
{
    double phi = -10.0; 
    if(sector <= 8) phi = (sector + 23)*SECTOR_DIVISION + 0.5*SECTOR_DIVISION;
    if(sector >= 9) phi = (sector - 9)*SECTOR_DIVISION + 0.5*SECTOR_DIVISION;
    return phi;
}

//------------------
// EndRun
//------------------
void JEventProcessor_TPOL_tree::EndRun()
{
    // This is called whenever the run number changes, before it is
    // changed to give you a chance to clean up before processing
    // events from the next run number.
}

//------------------
// Finish
//------------------
void JEventProcessor_TPOL_tree::Finish()
{
    // Called before program exit after event processing is finished.
    delete dTreeInterface;
}
