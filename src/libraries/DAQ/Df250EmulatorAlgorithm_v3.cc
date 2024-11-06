#include <DAQ/Df250EmulatorAlgorithm_v3.h>

// corresponds to version 0x0C12 of the fADC250 firmware

Df250EmulatorAlgorithm_v3::Df250EmulatorAlgorithm_v3(JApplication* app){
    // Enables forced use of default values 
    FORCE_DEFAULT = 0;

    USE_CRATE_DEFAULTS = 1;

    // Default values for the essential parameters
    NSA_DEF = 20;
    NSB_DEF = 5;
    THR_DEF = 120;
    NPED_DEF = 4;
    MAXPED_DEF = 512;
    NSAT_DEF = 2;

    // DEBUG
    NSA_DEF = 15;
    NSB_DEF = 1;
    THR_DEF = 108;
    NPED_DEF = 4;
    MAXPED_DEF = 512;
    NSAT_DEF = 2;


    // Set verbosity
    VERBOSE = 0;

    app->SetDefaultParameter("EMULATION250:USE_CRATE_DEFAULTS", USE_CRATE_DEFAULTS,"Set to >0 to force use of crate-dependent default values");
    app->SetDefaultParameter("EMULATION250:FORCE_DEFAULT", FORCE_DEFAULT,"Set to >0 to force use of default values");
    app->SetDefaultParameter("EMULATION250:NSA", NSA_DEF,"Set NSA for firmware emulation, will be overwritten by BORConfig if present");
    app->SetDefaultParameter("EMULATION250:NSB", NSB_DEF,"Set NSB for firmware emulation, will be overwritten by BORConfig if present");
    app->SetDefaultParameter("EMULATION250:THR", THR_DEF,"Set threshold for firmware emulation, will be overwritten by BORConfig if present");
    app->SetDefaultParameter("EMULATION250:NPED", NPED_DEF,"Set NPED for firmware emulation, will be overwritten by BORConfig if present");
    app->SetDefaultParameter("EMULATION250:MAXPED", MAXPED_DEF,"Set MAXPED for firmware emulation, will be overwritten by BORConfig if present");
    app->SetDefaultParameter("EMULATION250:NSAT", NSAT_DEF,"Set NSAT for firmware emulation, will be overwritten by BORConfig if present");
    app->SetDefaultParameter("EMULATION250:VERBOSE", VERBOSE,"Set verbosity for f250 emulation");
}

void Df250EmulatorAlgorithm_v3::EmulateFirmware(const Df250WindowRawData* rawData,
                                                std::vector<Df250PulseData*> &pdat_objs)
{
    // This is the main routine called by JEventSource_EVIO::GetObjects() and serves as the entry point for the code.
    if (VERBOSE > 0) {
        jout << " Df250EmulatorAlgorithm_v3::EmulateFirmware ==> Starting emulation <==" << endl;
        jout << "rocid : " << rawData->rocid << " slot: " << rawData->slot << " channel: " << rawData->channel << endl;
    }

    // First check that we have window raw data available
    if (rawData == NULL) {
        jerr << " ERROR: Df250EmulatorAlgorithm_v3::EmulateFirmware - raw sample data is missing" << endl;
        jerr << " Contact mstaib@jlab.org" << endl;
	return;
    } 
    if (rawData->samples.size() == 0) {
	jerr << " ERROR: Df250EmulatorAlgorithm_v3::EmulateFirmware - raw sample data has zero size" << endl;
        jerr << "rocid : " << rawData->rocid << " slot: " << rawData->slot << " channel: " << rawData->channel << endl;
        //jerr << " Contact mstaib@jlab.org" << endl;
	return;
    } 

    // We need the channel number to get the threshold
    uint32_t channel = rawData->channel;

    // First grab the config objects from the raw data and get the quantities we need from them
    // The only things we need for this version of the f250 firmware are NSB, NSA, and the threshold.
    // These are all stored in the BOR config. We can grab this from the raw data since that was already associated in JEventSource_EVIO::GetObjects. 
    const Df250BORConfig *f250BORConfig = NULL;
    rawData->GetSingle(f250BORConfig);

    uint32_t NSA;
    int32_t NSB;
    uint32_t NPED, MAXPED;
    uint16_t THR;
    uint16_t NSAT;
    //If this does not exist, or we force it, use the default values
    if (f250BORConfig == NULL || FORCE_DEFAULT){
        static int counter = 0;
        NSA = NSA_DEF;
        NSB = NSB_DEF;
        THR = THR_DEF;
        NPED = NPED_DEF;
        MAXPED = MAXPED_DEF;
        NSAT = NSAT_DEF;
        if (counter < 10){
            counter++;
            if (counter == 10) jout << " WARNING Df250EmulatorAlgorithm_v3::EmulateFirmware No Df250BORConfig == Using default values == LAST WARNING" << endl;
            else jout << " WARNING Df250EmulatorAlgorithm_v3::EmulateFirmware No Df250BORConfig == Using default values  " << endl;
			 //<< rawData->rocid << "/" << rawData->slot << "/" << rawData->channel << endl;
        }


	if(USE_CRATE_DEFAULTS) {
		// BASED on run 71137
		// FCAL, crates = 11-22
		if( (rawData->rocid >= 11) && (rawData->rocid <= 22) ) {
			NSA = 15;
			NSB = 1;
			THR = 108;
			//NPED = NPED_DEF;
			//MAXPED = MAXPED_DEF;
			NSAT = 2;
		}
		// BCAL, crates = 31-46
		else if( (rawData->rocid >= 31) && (rawData->rocid <= 46) ) {
			NSA = 26;
			NSB = 1;
			THR = 105;
			//NPED = NPED_DEF;
			//MAXPED = MAXPED_DEF;
			NSAT = 2;
		}
		// TAGH, crates 73-74, 75[slot 7-16]
		else if( (rawData->rocid == 73) || (rawData->rocid == 74) || ( (rawData->rocid == 75) && (rawData->slot >= 7) && (rawData->slot <= 16)) ) {
			NSA = 6;
			NSB = 3;
			THR = 300;
			//NPED = NPED_DEF;
			//MAXPED = MAXPED_DEF;
			NSAT = 2;
		}
		// TAGM, crates 71-72, 75[slot 3-6]
		else if( (rawData->rocid == 71) || (rawData->rocid == 72) || ( (rawData->rocid == 75) && (rawData->slot >= 3) && (rawData->slot <= 6)) ) {
			NSA = 6;
			NSB = 3;
			THR = 150;
			//NPED = NPED_DEF;
			//MAXPED = MAXPED_DEF;
			NSAT = 2;
		}
		// TOF, crates = 77
		else if( rawData->rocid == 77 ) {
			NSA = 10;
			NSB = 1;
			THR = 160;
			//NPED = NPED_DEF;
			//MAXPED = MAXPED_DEF;
			NSAT = 2;
		}
		// PS, crates = 83-84
		else if( (rawData->rocid >= 83) && (rawData->rocid <= 84) ) {
			NSA = 10;
			NSB = 3;
			THR = 130;
			//NPED = NPED_DEF;
			//MAXPED = MAXPED_DEF;
			NSAT = 2;
		}
		// ST, crates = 94
		else if( rawData->rocid == 94 ) {
			NSA = 20;
			NSB = 5;
			THR = 120;
			//NPED = NPED_DEF;
			//MAXPED = MAXPED_DEF;
			NSAT = 2;
		}       
	}
    }
    else{
        NSA    = f250BORConfig->NSA;
        NSB    = f250BORConfig->NSB;
        THR    = f250BORConfig->adc_thres[channel];
        NPED   = f250BORConfig->NPED;
        MAXPED = f250BORConfig->MaxPed;
        NSAT   = f250BORConfig->NSAT;
        //if (VERBOSE > 0) jout << "Df250EmulatorAlgorithm_v3::EmulateFirmware NSA: " << NSA << " NSB: " << NSB << " THR: " << THR << endl; 
    }

    if (VERBOSE > 0) jout << "Df250EmulatorAlgorithm_v3::EmulateFirmware NSA: " << NSA << " NSB: " << NSB << " THR: " << THR << endl; 

    // Note that in principle we could get this information from the Df250Config objects as well, but generally only NPED and the value of NSA+NSB are saved
    // not the individual NSA and NSB values
    
    /*
    // TEST
    if( (rawData->rocid >= 31) || (rawData->rocid <= 46) ) {
      NSA = NSA_DEF;
      NSB = NSB_DEF;
    }
    */

    // quality bits
    bool bad_pedestal = false;
    bool bad_timing_pedestal = false;
    bool no_timing_calculation = false;

    // Now we can start to loop over the raw data
    // This requires a few passes due to some features in the way the quantities are calculated...
    // The first step is to scan the samples for TC (threshold crossing sample) and compute the
    // integrals of all pulses found.

    vector<uint16_t> samples = rawData->samples; 
    uint16_t NW = samples.size();
    uint32_t npulses = 0;
    const int max_pulses = 3;
    uint32_t TC[max_pulses] = {};
    uint32_t TMIN[max_pulses] = {3};
    //uint32_t TNSAT[max_pulses] = {};
    uint32_t pulse_integral[max_pulses] = {};
    bool has_overflow_samples[max_pulses] = {false};
    bool has_underflow_samples[max_pulses] = {false};
    uint32_t number_samples_above_threshold[max_pulses] = {0};
    bool NSA_beyond_PTW[max_pulses] = {false};
    bool vpeak_beyond_NSA[max_pulses] = {false};
    bool vpeak_not_found[max_pulses] = {false};

    // some verbose debugging output
    if(VERBOSE > 0) {
        for (unsigned int i=0; i < NW; i++) {
            if(VERBOSE > 2) {
                if(samples[i] == 0x1fff)
                    jout << "Overflow at sample " << i << endl;
                if(samples[i] == 0x1000)
                    jout << "Underflow at sample " << i << endl;
            }
            if (VERBOSE > 5) jout << "Df250EmulatorAlgorithm_v3::EmulateFirmware samples[" << i << "]: " << samples[i] << endl;
        }
    }

   
    // look for the threhold crossings and compute the integrals
    //unsigned int MAX_SAMPLE = (NSB>0) ? (NW-NSAT) : (NW-NSAT+NSB-1)); // check this
    unsigned int MAX_SAMPLE = NW-NSAT;
    //cerr << " MAX_SAMPLE = " << MAX_SAMPLE << "  NW = " << NW << "  NSAT = " << NSAT << endl;
    //for (unsigned int i=0; i < MAX_SAMPLE; i++) {
    for (unsigned int i=0; i < MAX_SAMPLE; i++) {
        if ((samples[i] & 0xfff) > THR) {
            if (VERBOSE > 1) {
                jout << "threshold crossing at " << i << endl;
            }

            // save threshold crossing - could be overwritten
            TC[npulses] = i+1;

            // check that we have more than NSAT samples over threshold
	    if( NSAT>1 ){
                int samples_over_threshold = 1;

		if(i==0) {
		  // the algorithm only terminates if we dip below threshold...
			//for(unsigned int j=i+1; ((samples[j]&0xfff)>=THR) && (j<MAX_SAMPLE+1); j++) {
		  for(unsigned int j=i+1; ((samples[j]&0xfff)>THR) && (j<MAX_SAMPLE+1); j++) {
		    // only count samples actually above threshold
		    if ((samples[j] & 0xfff) > THR) 
		      samples_over_threshold++;
		    
		    if( samples_over_threshold == NSAT ) {
		      //TC[npulses] = j+1;
		      //i=j;
		      break;
		    }

		  }
		} else {
		  for(unsigned int j=i+1; ((samples[j]&0xfff)>THR) && (j<MAX_SAMPLE+1); j++) {
		    samples_over_threshold++;
		    
		    if( samples_over_threshold == NSAT ) 
		      break;
		  }		    
		}
		
		// if we couldn't find NSAT samples above threshold, move on...
		if( samples_over_threshold != NSAT )
		  continue;
            } 
	    //else {
	    //TNSAT[npulses] = TC[npulses];
	    //}

           
            // calculate integral
            unsigned int ibegin;
            if(NSB > 0)
                ibegin = i > uint32_t(NSB) ? (i - NSB) : 0; // Set to beginning of window if too early
            else {
                ibegin = i - NSB;
                if(ibegin > uint32_t(NW))  // make sure we don't start looking outside the window
                    break;
            }
            unsigned int iend = (i + NSA) < uint32_t(NW) ? (i + NSA) : NW; // Set to last sample if too late
            // check to see if NSA extends beyond the end of the window
            NSA_beyond_PTW[npulses] = (i + NSA - 1) >= uint32_t(NW);
            for (i = ibegin; i < iend; ++i) {
                pulse_integral[npulses] += (samples[i] & 0xfff);
                // quality monitoring
                if(samples[i] == 0x1fff) {
                    has_overflow_samples[npulses] = true;
                }
                if(samples[i] == 0x1000) {
                    has_underflow_samples[npulses] = true;
                }
                // count number of samples within NSA that are above thresholds
                if( (i+1>=TC[npulses]) && ((samples[i] & 0xfff) > THR) )
                    number_samples_above_threshold[npulses]++;
            }
            for (; i < NW && (samples[i] & 0xfff) >= THR; ++i) {}
            if (++npulses == max_pulses)
               break;
            TMIN[npulses] = i;
        }
    }

    // That concludes the first pass over the data.
    // Now we can head into the fine timing pass over the data.

    uint32_t VPEAK[max_pulses] = {};
    uint32_t TPEAK[max_pulses] = {};
    uint16_t TMID[max_pulses] = {};
    uint16_t VMID[max_pulses] = {};
    uint16_t TFINE[max_pulses] = {};
    uint32_t pulse_time[max_pulses] = {};

    // The pulse pedestal is the sum of NPED (4-15) samples at the beginning of the window
    uint32_t pedestal = 0;
    uint32_t VMIN = 0;   // VMIN is just the average of the first 4 samples, needed for timing algorithm
    for (unsigned int i=0; i < NPED; i++) {
        pedestal += (samples[i] & 0xfff);
        if(i<4)
            VMIN += (samples[i] & 0xfff);
        // error conditions
        // sample larger than MaxPed
        if ((samples[i] & 0xfff) > MAXPED) {
            bad_pedestal = true;
        }
        // samples with underflow/overflow
        if( (samples[i] == 0x1fff) || (samples[i] == 0x1000) ) {
            bad_pedestal = true;
        }
    }
    VMIN /= NPED;   // compute average

    // error conditions for timing algorithm
    //bool pedestal_underflow = false;
    for (unsigned int i=0; i < 4; i++) {
        // We set the "Time Quality bit 0" to 1 if any of the first 4 samples is greated than MaxPed or TET...
        if ( ((samples[i] & 0xfff) > MAXPED) || ((samples[i] & 0xfff) > THR) ) {
            bad_timing_pedestal = true;
        }
        // ... or is overflow or underflow
        if ( (samples[i] == 0x1000) || (samples[i] == 0x1fff) ) {
            bad_timing_pedestal = true;
        }
        //}

        // "If any of the first 4 samples is greater than TET the TDC will NOT proceed..."
        // Waiit for iiit...
        if( (samples[i] & 0xfff) > THR ) {
            no_timing_calculation = true;
        }
    }


    for (unsigned int p=0; p < npulses; ++p) {

        // "If any of the first 4 samples is greater than TET or underflow the TDC will NOT proceed
        //   1. pulse time is set to TC
        //   2. pulse peak is set to zero - not anymore!
        //   3. Time quality bits 0 and 1 are set to 1"
        if(no_timing_calculation) {
            TMID[p] = TC[p];
            TFINE[p] = 0;
            VPEAK[p] = 0;
            //vpeak_not_found[p] = true;   // this is "time quality bit 1"
            // "Time Quality bit 0" should already be set
        }   // should just put an else here...

        // we set up a loop so that we can break out of it at appropriate times...
        // note that currently the timing algorithm is run when the pedestal has underflow samples,
        // but according to the documentation, it shouldn't...
	// NOTE that we should always run the calculation since we always need to search for the
	// peak position, just in some edge cases we don't need to run the timing algorithm

        //while ( (!no_timing_calculation || pedestal_underflow) && true) {
        while (true) {
            //if (VMIN == 99999) {
            //    VPEAK[p] = 0;
            //    reportTC[p] = true;
            //    pulse_time[p] = (TC[p] << 6);
            //    break;
            // }

            // search for the peak of the pulse
            // has to be after the threshold crossing (NO?)
            // has to be before the last sample
            unsigned int ipeak;
            for (ipeak = TC[p]; (int)ipeak < NW-1; ++ipeak) {
            //for (ipeak = TC[p]+1; ipeak < NW-1; ++ipeak) {
                if ((samples[ipeak] & 0xfff) < (samples[ipeak-1] & 0xfff)) {
                    VPEAK[p] = (samples[ipeak-1] & 0xfff);
                    TPEAK[p] = ipeak-1;
                    break;
                }
            }

            // check to see if the peak is beyond the NSA
            if(ipeak > TC[p]+NSA)
                vpeak_beyond_NSA[p] = true;

            if (VERBOSE > 1) {
                jout << " pulse " << p << ": VMIN: " << VMIN 
                     << " TC: " << TC[p] << " VPEAK: " << VPEAK[p] << endl;  
            }

            // set error conditions in case we didn't find the peak
            if (VPEAK[p] == 0) { 
                TMID[p] = TC[p];
                TFINE[p] = 0;
                VPEAK[p] = 0;
                vpeak_beyond_NSA[p] = true;
                vpeak_not_found[p] = true;
                break;
            }

	    // we have found the peak position, now ignore the timing calculation if need be...
	    if(no_timing_calculation)
		    break;

            // VMID is the half amplitude
            VMID[p] = (VMIN + VPEAK[p]) >> 1;
            

            // look down the leading edge for the sample that satisfies V(N1) <= VMID < V(N+1)
            // N1 is then the coarse time
            // note that when analyzing pulses after the first, we could end up with a time for the
            // second pulse that is before the first one!  this is a little crazy, but is how
            // the algorithm is currently implemented
            for (unsigned int i = TPEAK[p]; i >= 1; --i) { 
		    if ( ((samples[i-1] & 0xfff) <= VMID[p]) && ((samples[i] & 0xfff) > VMID[p]) ) {  // V(N1) <= VMID < V(N+1)
		    // if ( ((samples[i-1] & 0xfff) <= VMID[p])                         // V(N1) <= VMID < V(N+1)
                    // || ( (samples[i-1] & 0xfff) > (samples[i] & 0xfff) ) ) {   // we aren't on the leading edge anymore
                    TMID[p] = i;
                    break;
                }
            }

            if (TMID[p] == 0) {  // handle the case where we couldn't find a coarse time - redundant?
                TFINE[p] = 0;
            }
            else {
                // fine timing algorithm (see documentation)
                int Vnext = (samples[TMID[p]] & 0xfff); 
                int Vlast = (samples[TMID[p]-1] & 0xfff);
                if (VERBOSE > 2) {
                    jout << "   TMIN = " << TMIN[p] << "  TMID  = " << TMID[p] << "  TPEAK = " << TPEAK[p] << endl
                         << "   VMID = " << VMID[p] << "  Vnext = " << Vnext   << "  Vlast = " << Vlast    << endl;
                }
                if (Vnext > Vlast && VMID[p] >= Vlast)
                    TFINE[p] = 64 * (VMID[p] - Vlast) / (Vnext - Vlast);
                else
                    TFINE[p] = 62;
                if(TFINE[p] == 64)
                    TFINE[p] = 0;
            }
            pulse_time[p] = ((TMID[p]-1) << 6) + TFINE[p];
            break;
        }
        VMIN = (VMIN < 99999)? VMIN : 0;  // deprecated?

        if (VERBOSE > 1) {
            jout << " pulse " << p << ": VMID: " << VMID[p] << " TMID: " << TMID[p] 
                 << " TFINE: " << TFINE[p] << " time: " << pulse_time[p]
                 << " integral: " << pulse_integral[p] << endl;
            if (VERBOSE > 2) {
                jout << "   TMIN = " << TMIN[p] << "  TMID  = " << TMID[p] << "  TPEAK = " << TPEAK[p] << endl;
                    //<< "   VMID = " << VMID[p] << "  Vnext = " << Vnext   << "  Vlast = " << Vlast    << endl;
            }
        }

        // algorithm is finished, fill the information
        Df250PulseData* f250PulseData;
        if( p < pdat_objs.size() ) {
            f250PulseData = pdat_objs[p];
            
            if(f250PulseData == NULL) {
                jerr << " NULL f250PulseData object!" << endl;
                continue;
            }
        } else {
            // make a fresh object if one does not exist
            f250PulseData = new Df250PulseData;

            f250PulseData->rocid = rawData->rocid;
            f250PulseData->slot = rawData->slot;
            f250PulseData->channel = rawData->channel;
            f250PulseData->itrigger = rawData->itrigger;
            // word 1
            f250PulseData->event_within_block = 1;
            f250PulseData->QF_pedestal = bad_pedestal; 
            f250PulseData->pedestal = pedestal;
            // word 2
            f250PulseData->integral = pulse_integral[p];
            f250PulseData->QF_NSA_beyond_PTW = NSA_beyond_PTW[p];
            f250PulseData->QF_overflow = has_overflow_samples[p];
            f250PulseData->QF_underflow = has_underflow_samples[p];
            f250PulseData->nsamples_over_threshold = number_samples_above_threshold[p];
            // word 3
            f250PulseData->course_time = TMID[p]; 
            f250PulseData->fine_time = TFINE[p];  
            f250PulseData->QF_vpeak_beyond_NSA = vpeak_beyond_NSA[p];
            f250PulseData->QF_vpeak_not_found = vpeak_not_found[p];
            f250PulseData->QF_bad_pedestal = bad_timing_pedestal;
            // other information
            f250PulseData->pulse_number = p;
            f250PulseData->nsamples_integral = NSA + NSB;
            f250PulseData->nsamples_pedestal = NPED;
            f250PulseData->emulated = true;

            f250PulseData->AddAssociatedObject(rawData);
            const_cast<Df250WindowRawData*>(rawData)->AddAssociatedObject(f250PulseData);
            pdat_objs.push_back(f250PulseData);
        }

        // copy over emulated values
        f250PulseData->integral_emulated = pulse_integral[p];
        f250PulseData->pedestal_emulated = pedestal;
        f250PulseData->pulse_peak_emulated = VPEAK[p]; 
        f250PulseData->course_time_emulated = TMID[p];
        f250PulseData->fine_time_emulated = TFINE[p];

        // check the emulated quality factors as well
        uint32_t QF = 0; // make single quality factor number for compactness
        if( bad_pedestal         ) QF |= (1<<0);
        if( NSA_beyond_PTW[p]   ) QF |= (1<<1);
        if( has_overflow_samples[p]         ) QF |= (1<<2);
        if( has_underflow_samples[p]        ) QF |= (1<<3);
        if( vpeak_beyond_NSA[p] ) QF |= (1<<4);
        if( vpeak_not_found[p]  ) QF |= (1<<5);
        if( bad_timing_pedestal  ) QF |= (1<<6);
        f250PulseData->QF_emulated = QF;

        if(VERBOSE > 3) {
            cout << boolalpha;
            cout << "bad_pedestal          = " << bad_pedestal << endl;
            cout << "NSA_beyond_PTW        = " << NSA_beyond_PTW[p] << endl;
            cout << "has_overflow_samples  = " << has_overflow_samples[p] << endl;
            cout << "has_underflow_samples = " << has_underflow_samples[p] << endl;
            cout << "vpeak_beyond_NSA      = " << vpeak_beyond_NSA[p] << endl;
            cout << "vpeak_not_found       = " << vpeak_not_found[p] << endl;
            cout << "bad_timing_pedestal   = " << bad_timing_pedestal << endl;
            cout << "total QF              = " << QF << endl;
        }

        // if we are using the emulated values, copy them
        if( f250PulseData->emulated ) {
            f250PulseData->integral    = f250PulseData->integral_emulated;
            f250PulseData->pedestal    = f250PulseData->pedestal_emulated;
            f250PulseData->pulse_peak  = f250PulseData->pulse_peak_emulated;
            f250PulseData->course_time = f250PulseData->course_time_emulated;
            f250PulseData->fine_time   = f250PulseData->fine_time_emulated;
	    
	    /*
	    if( (rawData->rocid >= 31) || (rawData->rocid <= 46) ) {
	      f250PulseData->nsamples_integral = NSA + NSB;
	    }
	    */

        }

    }

    if (VERBOSE > 0) jout << " Df250EmulatorAlgorithm_v3::EmulateFirmware ==> Emulation complete <==" << endl;    
    return;
}
