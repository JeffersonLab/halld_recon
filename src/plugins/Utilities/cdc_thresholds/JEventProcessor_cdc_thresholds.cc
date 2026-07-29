// $Id$
//
//    File: JEventProcessor_cdc_thresholds.cc
// Created: Wed Jul  1 08:44:10 PM EDT 2026
// Creator: njarvis (on Linux ifarm2402.jlab.org 5.14.0-611.55.1.el9_7.x86_64 x86_64)
//

/// For more information on the syntax changes between JANA1 and JANA2, visit: https://jeffersonlab.github.io/JANA2/#/jana1to2/jana1-to-jana2


/// Extract hit thresholds from the fadc config data in Df125BORConfig

#include "JEventProcessor_cdc_thresholds.h"

using namespace std;

// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
extern "C"{
void InitPlugin(JApplication *app){
    InitJANAPlugin(app);
    app->Add(new JEventProcessor_cdc_thresholds());
}
} // "C"


//------------------
// JEventProcessor_cdc_thresholds (Constructor)
//------------------
JEventProcessor_cdc_thresholds::JEventProcessor_cdc_thresholds()
{
	SetTypeName(NAME_OF_THIS); // Provide JANA with this class's name

    // Parameters and Services should be accessed from Init() instead of here!
}

//------------------
// ~JEventProcessor_cdc_thresholds (Destructor)
//------------------
JEventProcessor_cdc_thresholds::~JEventProcessor_cdc_thresholds()
{
}

//------------------
// Init
//------------------
void JEventProcessor_cdc_thresholds::Init()
{
    // This is called once at program startup. 
    
    // auto app = GetApplication();
    // lockService should be initialized here like this
    // lockService = app->GetService<JLockService>();

    for (uint i=0; i<3522; i++) thresholds[i] = 0;
    thresholds_found = false;
}

//------------------
// BeginRun
//------------------
void JEventProcessor_cdc_thresholds::BeginRun(const std::shared_ptr<const JEvent> &event)
{
    // This is called whenever the run number changes

  vector< const DTranslationTable* > ttabVec;
  event->Get(ttabVec);

  ttab = ttabVec[0];
}

//------------------
// Process
//------------------
void JEventProcessor_cdc_thresholds::Process(const std::shared_ptr<const JEvent> &event)
{

    if (thresholds_found) return;

    //add extra 0 at front to use offset[1] for ring 1
    uint straw_offset[29] = {0,0,42,84,138,192,258,324,404,484,577,670,776,882,1005,1128,1263,1398,1544,1690,1848,2006,2176,2346,2528,2710,2907,3104,3313};

    run_number = event->GetRunNumber();

    vector<const Df125BORConfig*> borconfigs;
    event->Get(borconfigs);

    int nbor = (int)borconfigs.size();

    if (nbor==0) return;

    thresholds_found = true;
    
    for (int i=0; i<nbor; i++) {

	if (borconfigs[i]->rocid > 28) continue;

	uint channel = 0;

        for (int j=0; j<12; j++) { // 12 groups of 6 per fadc

    	    f125config_fe fe = borconfigs[i]->fe[j];

	    for (int k=0; k<6; k++) {

	        uint h = fe.threshold[k]&0xFFF;

		DTranslationTable::csc_t daq_index = { borconfigs[i]->rocid, borconfigs[i]->slot, channel };

		DTranslationTable::DChannelInfo channel_info;

		try {
		    channel_info = ttab->GetDetectorIndex(daq_index);
		}
		catch(...) { // not connected
		    //printf("not thresholds_found: rocid %i slot %i chan %i\n",borconfigs[i]->rocid, borconfigs[i]->slot, channel);
		    channel++;
		    continue;
		}

		channel_info.det_sys = DTranslationTable::CDC;

		uint straw_number = straw_offset[channel_info.cdc.ring] + channel_info.cdc.straw - 1;

		thresholds[straw_number] = h;

       		//cout << channel_info.cdc.ring << " " << channel_info.cdc.straw << " " << straw_number << " " << h << endl;

		channel++;

	    }
	}
    }

}

//------------------
// EndRun
//------------------
void JEventProcessor_cdc_thresholds::EndRun()
{
    // This is called whenever the run number changes, before it is
    // changed to give you a chance to clean up before processing
    // events from the next run number.

  
}

//------------------
// Finish
//------------------
void JEventProcessor_cdc_thresholds::Finish()
{

    if (thresholds_found) {
        char filename[100];
        sprintf(filename,"cdc_thresholds_%lu.txt",run_number);
        FILE *myfile;
        myfile = fopen(filename,"w");
        for (uint i=0; i<3522; i++)  fprintf(myfile,"%i\n",thresholds[i]);
        fclose(myfile);
	cout << "CDC thresholds written to " << filename << endl;
    } else {
        cout << "Threshold file not created because Df125BORConfig was not found\n" << endl;
    }
  // Called before program exit after event processing is finished.
}

