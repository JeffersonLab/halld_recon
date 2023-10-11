#include "JEventProcessor_FCAL_LED_shifts.h"
#include "DANA/DEvent.h"
#include "FCAL/DFCALShower.h"
#include "FCAL/DFCALGeometry.h"
#include "FCAL/DFCALHit.h"
#include "FCAL/DFCALDigiHit.h"
#include "FCAL/DFCALCluster.h"
#include "TRACKING/DTrackTimeBased.h"
#include "PID/DVertex.h"
#include "DVector3.h"
#include "ANALYSIS/DAnalysisUtilities.h"
#include <TTree.h>
#include "DVector3.h"
#include "PID/DParticleID.h"
#include "TRIGGER/DTrigger.h"
#include "GlueX.h"
#include <vector>
#include <map>
#include <deque>
#include <string>
#include <iostream>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <TDirectory.h>
#include <TH1I.h>
#include <TH2F.h>
#include <TFile.h>
#include <TFitResult.h>
#include <TFitResultPtr.h>


const int numChannels = 2800;

const int numCrates = 12;
const int firstCrate = 11;
const int numSlots = 16;
const int firstSlot = 3;

// Define Histograms


extern "C"{
  void InitPlugin(JApplication *app){
    InitJANAPlugin(app);
    app->Add(new JEventProcessor_FCAL_LED_shifts());
  }
} // "C"


//------------------
// JEventProcessor_FCAL_LED_shifts (Constructor)
//------------------
JEventProcessor_FCAL_LED_shifts::JEventProcessor_FCAL_LED_shifts()
{
	SetTypeName("JEventProcessor_FCAL_LED_shifts");
}

//------------------
// ~JEventProcessor_FCAL_LED_shifts (Destructor)
//------------------
JEventProcessor_FCAL_LED_shifts::~JEventProcessor_FCAL_LED_shifts()
{

}

//------------------
// Init
//------------------
void JEventProcessor_FCAL_LED_shifts::Init()
{
	auto app = GetApplication();
	lockService = app->GetService<JLockService>();

  	// Set parameters
  	CALC_NEW_CONSTANTS_LED = true;
	app->SetDefaultParameter("FCAL_SHIFT:CALC_NEW_CONSTANTS_LED", CALC_NEW_CONSTANTS_LED, "True if we should calculate new offsets based on a reference file for LED data");
  	CALC_NEW_CONSTANTS_BEAM = false;
	app->SetDefaultParameter("FCAL_SHIFT:CALC_NEW_CONSTANTS_BEAM", CALC_NEW_CONSTANTS_BEAM, "True if we should calculate new offsets based on a reference file for beam data");
	REFERENCE_FILE_NAME = "hd_root-r42485-led.root";
	app->SetDefaultParameter("FCAL_SHIFT:REFERENCE_FILE_NAME", REFERENCE_FILE_NAME, "Reference file for new offsets");

	FCAL_TOTAL_ENERGY_HI = 9000.;
	app->SetDefaultParameter("FCAL_SHIFT:FCAL_TOTAL_ENERGY_HI", FCAL_TOTAL_ENERGY_HI, "Total event energy cut (hi level)");
	FCAL_TOTAL_ENERGY_LO = 8500.;
	app->SetDefaultParameter("FCAL_SHIFT:FCAL_TOTAL_ENERGY_LO", FCAL_TOTAL_ENERGY_LO, "Total event energy cut (lo level)");


    // the histogram limits should be different for LED and beam data events
    NBINS_TIME=100;
    TIME_MIN=60.;
    TIME_MAX=110.;

    if(CALC_NEW_CONSTANTS_BEAM) {
        TIME_MIN=-20.;
        TIME_MAX=80.;
    }

    // This is called once at program startup. If you are creating
  	// and filling historgrams in this plugin, you should lock the
  	// ROOT mutex like this:
  	//
  	TDirectory *main = gDirectory;
  	gDirectory->mkdir("FCAL_LED_shifts")->cd();
  
  	// for monitoring of results
  	m_fadcShifts = new TH2I("fcal_fadc_shifts", "ADC Shifts", 60, 0, 60, 60, 0, 60);
  	m_fadcShifts->GetXaxis()->SetTitle("column");
  	m_fadcShifts->GetYaxis()->SetTitle("row");
  	m_fadcShifts->GetZaxis()->SetRangeUser(-8,8);
  	
  	m_totalEnergy = new TH1I("fcal_total_energy", "Total Energy", 500, 0, 10000);
  
  	for (int i = 0; i < numCrates; ++i) {
  		uint32_t crate = firstCrate + i;
    	m_crateTimes[crate] = (new TH1I(Form("crate_times_%i",crate),Form("Hit Times for Crate %i",crate),NBINS_TIME,TIME_MIN,TIME_MAX));
	  	for (int i = 0; i < numSlots; ++i) {
		  // next line commented out to avoid unused variable warning
			//uint32_t slot = firstSlot + i;
			//pair<uint32_t,uint32_t> crate_slot(crate,slot);
		   	//m_slotTimes[crate_slot] = (new TH1I(Form("slot_times_c%i_s%i",crate,slot),
		   	//							Form("Hit Times for Crate %i, Slot %i",crate,slot),200,60.,110.));
		}
  	}

  	for (int i = 0; i < numChannels; ++i) {
    	m_channelTimes.push_back( (new TH1I(Form("channel_times_%i",i),Form("Hit Times for Channel %i",i),NBINS_TIME,TIME_MIN,TIME_MAX)) );
  	}


  	main->cd();
  	

  	return;
}

//------------------
// BeginRun
//------------------
void JEventProcessor_FCAL_LED_shifts::BeginRun(const std::shared_ptr<const JEvent> &event)
{
	/*
  // get the FCAL z position from the global geometry interface
  const DGeometry *geom = GetDGeometry(event);
  if( geom ) {

    geom->GetFCALZ( m_FCALfront );
  }
  else{
      
    cerr << "No geometry accessbile." << endl;
    throw JException("No geometry accessible");
  }
	*/

	// WARNING: THIS IS SUPER DANGEROUS
	// FIGURE OUT A WAY TO SAVE THIS INFO FOR erun()
  // we need an FCAL Geometry object
  vector< const DFCALGeometry* > geomVec;
  event->Get( geomVec );

  if( geomVec.size() != 1 ){

    cerr << "No geometry accessbile." << endl;
    throw JException("No FCAL geometry available");
  }

  m_fcalGeom = geomVec[0];
  
  
  // Load the translation table
  vector< const DTranslationTable* > ttabVec;
  event->Get(ttabVec);

  m_ttab = ttabVec[0];

  // save this info - not terribly thread safe, but this plugin
  // should only be used on one run at once
  m_runnumber = event->GetRunNumber();
  // load old offsets
  GetCalib(event, "/FCAL/ADC_Offsets", old_ADCoffsets);
}

//------------------
// Process
//------------------
void JEventProcessor_FCAL_LED_shifts::Process(const std::shared_ptr<const JEvent> &event)
{



  	vector< const DFCALHit*  > hits;
  	event->Get( hits );


	// save this mapping outside of the locks below, since the index lookup is pretty slow
  	map< int, pair<uint32_t,uint32_t> > crate_slot_map;

    // look at the total energy first to pick up a particle LED color
  	// since each color has different timing.  pick the most energetic to analyze
    double total_energy = 0.;
  	for( auto hit : hits ) {
  		total_energy += hit->E;
  	}
  	
  	for( auto hit : hits ) {
	 	//if(total_energy > FCAL_TOTAL_ENERGY_LO && total_energy < FCAL_TOTAL_ENERGY_HI) {
   			// convert (row,col) to channel index
  			int channel_index = m_fcalGeom->channel(hit->row, hit->column);

  			// look up crate/slot info
  			DTranslationTable::DChannelInfo channel_info;
  			channel_info.det_sys = DTranslationTable::FCAL;
  			channel_info.fcal.row = hit->row;
  			channel_info.fcal.col = hit->column;
  			auto daq_index = m_ttab->GetDAQIndex(channel_info);
  			pair<uint32_t,uint32_t> crate_slot(daq_index.rocid,daq_index.slot);
		
			//cerr << "crate = " << crate_slot.first << "  slot = " << crate_slot.second << endl;

			crate_slot_map[channel_index] = crate_slot;
            //}
	}

    //cerr << total_energy << endl;


	// FILL HISTOGRAMS
	// Since we are filling histograms local to this plugin, it will not interfere with other ROOT operations: can use plugin-wide ROOT fill lock
	lockService->RootFillLock(this); //ACQUIRE ROOT FILL LOCK
  
    m_totalEnergy->Fill(total_energy);

	//if(total_energy > FCAL_TOTAL_ENERGY_LO && total_energy < FCAL_TOTAL_ENERGY_HI) {
  	for( auto hit : hits ) {
  	  		
  		// convert (row,col) to channel index
  		int channel_index = m_fcalGeom->channel(hit->row, hit->column);
  	
  	  	pair<uint32_t,uint32_t> crate_slot = crate_slot_map[channel_index];
		//cerr << "crate = " << crate_slot.first << "  slot = " << crate_slot.second << endl;
		
		if(crate_slot.first == 0) { // ignore these hits
			continue;
		}

  		// fill histograms
  		m_crateTimes[crate_slot.first]->Fill(hit->t);
		// make these on demand, since the number of slots is different in each crate
		if(m_slotTimes.find(crate_slot) == m_slotTimes.end()) {
			TDirectory *main = gDirectory;
  			gDirectory->cd("FCAL_LED_shifts");
		   	m_slotTimes[crate_slot] = (new TH1I(Form("slot_times_c%i_s%i",crate_slot.first,crate_slot.second),
                                                Form("Hit Times for Crate %i, Slot %i",crate_slot.first,crate_slot.second),NBINS_TIME/2,TIME_MIN,TIME_MAX));
			main->cd();
  		}
  		m_slotTimes[crate_slot]->Fill(hit->t);
  		m_channelTimes[channel_index]->Fill(hit->t);
  		
  		//cout << " crate = " << daq_index.rocid << "  slot = " << daq_index.slot 
  		//	<< "  time = " << hit->t << endl;
  	}
    //}

    lockService->RootFillUnLock(this);  //RELEASE ROOT FILL LOCK
}

static double  CalcADCShift(double reference_time, double time) {
	// Assume, for now, the simplest case: fADC250 times can shift by +/- 4 ns
	// Hopefully it stays that way
	// Use some rough tolerance, return the appropriate correction factor
	
	if( fabs((time - reference_time)-4) < 1. )
		return -4.;
	else if( fabs((time - reference_time)+4) < 1. )
		return 4.;

	return 0.;
}


//------------------
// EndRun
//------------------
void JEventProcessor_FCAL_LED_shifts::EndRun()
{
  	// This is called whenever the run number changes, before it is
  	// changed to give you a chance to clean up before processing
  	// events from the next run number.
 
   
  	if(CALC_NEW_CONSTANTS_BEAM) {
  		// calculate time shifts
  		//cerr << "opening " << REFERENCE_FILE_NAME << endl;
  		auto ref_file = new TFile(REFERENCE_FILE_NAME.c_str());
	
		ofstream outf(Form("fcal_adc_offsets_r%i.txt", m_runnumber));	
		
		// Have to define an object to pass into TH1::Fit() below since ROOT is being weird 
		auto fgaus = new TF1("fgaus", "gaus", -200, 200);


		// calculate crate shifts
		map<uint32_t, double> crate_shifts;
  		for (int i = 0; i < numCrates; ++i) {
	  		uint32_t crate = firstCrate + i;
	  		
	  		auto ref_hist = (TH1I*)ref_file->Get(Form("FCAL_LED_shifts/crate_times_%i",crate));
            if(ref_hist == NULL) {
                cout << "skipping crate " << crate << " ..." <<endl;
                continue;
            }
			// reference time
            double max = ref_hist->GetBinCenter(ref_hist->GetMaximumBin());
            TFitResultPtr ref_fr = ref_hist->Fit(fgaus, "SQ", "", max - 2.5, max + 2.5);
			double reference_time = ref_fr->Parameter(1);

            if(m_crateTimes.find(crate) == m_crateTimes.end()) continue;

            max = m_crateTimes[crate]->GetBinCenter(m_crateTimes[crate]->GetMaximumBin());
            TFitResultPtr fr = m_crateTimes[crate]->Fit(fgaus, "SQ", "", max - 2.5, max + 2.5);
	  		double time = fr->Parameter(1);
		
			double adc_shift = CalcADCShift(reference_time, time);
			crate_shifts[crate] = adc_shift;
			
			cout << "crate " << crate << " ADC shift = " << (int)adc_shift
				<< "  reference time = " << reference_time << "  time = " << time << endl;
		}
  		
  		// calculate slot shifts
		map< pair<uint32_t,uint32_t>, double> slot_shifts;
  		for (auto &slotTimeEntry : m_slotTimes) {
	  		uint32_t crate = slotTimeEntry.first.first;
	  		uint32_t slot = slotTimeEntry.first.second;
	  		
	  		auto ref_hist = (TH1I*)ref_file->Get(Form("FCAL_LED_shifts/slot_times_c%i_s%i",crate,slot));
            if(ref_hist == NULL) {
                cout << "skipping crate " << crate << " slot " << slot << " ..." <<endl;
                continue;
            }

			// reference time
            double max = ref_hist->GetBinCenter(ref_hist->GetMaximumBin());
            TFitResultPtr ref_fr = ref_hist->Fit(fgaus, "SQ", "", max - 2.5, max + 2.5);
			double reference_time = ref_fr->Parameter(1);

            max = slotTimeEntry.second->GetBinCenter(slotTimeEntry.second->GetMaximumBin());
            TFitResultPtr fr = slotTimeEntry.second->Fit(fgaus, "SQ", "", max - 2.5, max + 2.5);
	  		double time = fr->Parameter(1);
		
			double adc_shift = CalcADCShift(reference_time, time);
			slot_shifts[slotTimeEntry.first] = adc_shift;
			
			cout << "crate " << crate  << "slot " << slot << " ADC shift = " << (int)adc_shift
				<< "  reference time = " << reference_time << "  time = " << time << endl;
				
		}

		// calculate channel shifts
		for (int i = 0; i < numChannels; ++i) {
			// figure out detector location and indexing
			int row = m_fcalGeom->row(i);
			int col = m_fcalGeom->column(i);
			
			// look up crate/slot info
  			DTranslationTable::DChannelInfo channel_info;
  			channel_info.det_sys = DTranslationTable::FCAL;
  			channel_info.fcal.row = row;
  			channel_info.fcal.col = col;
  			auto daq_index = m_ttab->GetDAQIndex(channel_info);
  			pair<uint32_t,uint32_t> crate_slot(daq_index.rocid,daq_index.slot);
			
    		//m_channelTimes.push_back( (new TH1I(Form("channel_times_%i",i),Form("Hit Times for Channel %i",i),50,60.,110.)) );

			// determine the shifts for individual channels in the following way:
			// 1) if the whole crate has shifted, use this value
			// 2) if the whole slot has shifted, use this value
			// 3) use the individual channel
			double adc_shift = 0.;
			if(static_cast<int>(crate_shifts[daq_index.rocid]) != 0) {
				adc_shift = crate_shifts[daq_index.rocid];
			} else if(static_cast<int>(slot_shifts[crate_slot]) != 0) {
				adc_shift = slot_shifts[crate_slot];
			} else {
				auto ref_hist = (TH1I*)ref_file->Get(Form("FCAL_LED_shifts/channel_times_%i",i));
                if(ref_hist == NULL) {
                    cout << "skipping channel " << i << " ..." <<endl;
                    continue;
                }

				// reference time
            	double max = ref_hist->GetBinCenter(ref_hist->GetMaximumBin());
           		TFitResultPtr ref_fr = ref_hist->Fit(fgaus, "SQ", "", max - 2.5, max + 2.5);
				if(ref_fr>=0) {  // make sure the fits converge...
					double reference_time = ref_fr->Parameter(1);

    	        	max = m_channelTimes[i]->GetBinCenter(m_channelTimes[i]->GetMaximumBin());
        	    	TFitResultPtr fr = m_channelTimes[i]->Fit(fgaus, "SQ", "", max - 2.5, max + 2.5);
					if(fr>=0) {
		  				double time = fr->Parameter(1);
		
						adc_shift = CalcADCShift(reference_time, time);
					}
				}
			}
			
			// histogram the results for monitoring purposes
			m_fadcShifts->Fill(col,row,adc_shift);
			
			outf << (old_ADCoffsets[i] + adc_shift) << endl;
  		}

  		
  		ref_file->Close();
  		outf.close();
	  	delete fgaus;

  	}
  	
  	if(CALC_NEW_CONSTANTS_LED) {
  		// calculate time shifts
  		//cerr << "opening " << REFERENCE_FILE_NAME << endl;
  		auto ref_file = new TFile(REFERENCE_FILE_NAME.c_str());
	
		ofstream outf(Form("fcal_adc_offsets_r%i.txt", m_runnumber));	
		
		// calculate crate shifts
		map<uint32_t, double> crate_shifts;
  		for (int i = 0; i < numCrates; ++i) {
	  		uint32_t crate = firstCrate + i;
	  		
	  		auto ref_hist = (TH1I*)ref_file->Get(Form("FCAL_LED_shifts/crate_times_%i",crate));
			// reference time
			double reference_time = ref_hist->GetMean();
	  		double time =  m_crateTimes[crate]->GetMean();
		
			double adc_shift = CalcADCShift(reference_time, time);
			crate_shifts[crate] = adc_shift;
			
			//cout << "crate " << crate << " ADC shift = " << (int)adc_shift
			//	<< "  reference time = " << reference_time << "  time = " << time << endl;
		}
  		
  		// calculate slot shifts
		map< pair<uint32_t,uint32_t>, double> slot_shifts;
  		for (auto &slotTimeEntry : m_slotTimes) {
	  		uint32_t crate = slotTimeEntry.first.first;
	  		uint32_t slot = slotTimeEntry.first.second;
	  		
	  		auto ref_hist = (TH1I*)ref_file->Get(Form("FCAL_LED_shifts/slot_times_c%i_s%i",crate,slot));
			// reference time
			double reference_time = ref_hist->GetMean();
	  		double time =  slotTimeEntry.second->GetMean();
		
			double adc_shift = CalcADCShift(reference_time, time);
			slot_shifts[slotTimeEntry.first] = adc_shift;
			
			//cout << "crate " << crate  << "slot " << slot << " ADC shift = " << (int)adc_shift
			//	<< "  reference time = " << reference_time << "  time = " << time << endl;
				
		}

		// calculate channel shifts
		for (int i = 0; i < numChannels; ++i) {
			// figure out detector location and indexing
			int row = m_fcalGeom->row(i);
			int col = m_fcalGeom->column(i);
			
			// look up crate/slot info
  			DTranslationTable::DChannelInfo channel_info;
  			channel_info.det_sys = DTranslationTable::FCAL;
  			channel_info.fcal.row = row;
  			channel_info.fcal.col = col;
  			auto daq_index = m_ttab->GetDAQIndex(channel_info);
  			pair<uint32_t,uint32_t> crate_slot(daq_index.rocid,daq_index.slot);
			
    		//m_channelTimes.push_back( (new TH1I(Form("channel_times_%i",i),Form("Hit Times for Channel %i",i),50,60.,110.)) );

			// determine the shifts for individual channels in the following way:
			// 1) if the whole crate has shifted, use this value
			// 2) if the whole slot has shifted, use this value
			// 3) use the individual channel
			double adc_shift = 0.;
			if(static_cast<int>(crate_shifts[daq_index.rocid]) != 0) {
				adc_shift = crate_shifts[daq_index.rocid];
			} else if(static_cast<int>(slot_shifts[crate_slot]) != 0) {
				adc_shift = slot_shifts[crate_slot];
			} else {
				auto ref_hist = (TH1I*)ref_file->Get(Form("FCAL_LED_shifts/channel_times_%i",i));
				// reference time
				double reference_time = ref_hist->GetMean();
	  			double time = m_channelTimes[i]->GetMean();
	  			adc_shift = CalcADCShift(reference_time, time);
			}
			
			// histogram the results for monitoring purposes
			m_fadcShifts->Fill(col,row,adc_shift);
			
			outf << (old_ADCoffsets[i] + adc_shift) << endl;
  		}

  		
  		ref_file->Close();
  		outf.close();

  	}
}

//------------------
// Finish
//------------------
void JEventProcessor_FCAL_LED_shifts::Finish()
{
  // Called before program exit after event processing is finished.
}


