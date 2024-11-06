// $Id$
//
//    File: DBCALTDCHit_factory.cc
// Created: Tue Aug  6 11:04:11 EDT 2013
// Creator: davidl (on Darwin harriet.jlab.org 11.4.2 i386)
//


#include <iostream>
#include <iomanip>
using namespace std;

#include <BCAL/DBCALTDCDigiHit.h>
#include <BCAL/DBCALTDCHit_factory.h>
#include <DAQ/DF1TDCHit.h>

#include <JANA/JEvent.h>
#include <JANA/Calibrations/JCalibrationManager.h>


//------------------
// init
//------------------
void DBCALTDCHit_factory::Init()
{
    /// set the base conversion scale
    t_scale    = 0.058;    // 60 ps/count
    t_base     = 0.;    // ns
    t_rollover = 65250;
    //t_offset   = 0;
}

//------------------
// brun
//------------------
void DBCALTDCHit_factory::BeginRun(const std::shared_ptr<const JEvent>& event)
{

	size_t runnumber = event->GetRunNumber();
	JCalibration *jcalib = GetApplication()->GetService<JCalibrationManager>()->GetJCalibration(runnumber);

    // Only print messages for one thread whenever run number changes
    static pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;
    static set<int> runs_announced;
    pthread_mutex_lock(&print_mutex);
    bool print_messages = false;
    if(runs_announced.find(runnumber) == runs_announced.end()){
        print_messages = true;
        runs_announced.insert(runnumber);
    }
    pthread_mutex_unlock(&print_mutex);

    /* Rollover now handled by TTABUtilities 
       t_rollover = 65250;
       if (runnumber>1776){
       t_rollover = 64678;
       }
       if (runnumber>2010){
       t_rollover = 64466;
       }
       */

    /// Read in calibration constants

    if(print_messages) jout << "In DBCALTDCHit_factory, loading constants..." << endl;

    // load scale factors
    map<string,double> scale_factors;
    if(jcalib->Get("/BCAL/digi_scales", scale_factors))
        jout << "Error loading /BCAL/digi_scales !" << endl; 
    if( scale_factors.find("BCAL_TDC_SCALE") != scale_factors.end() ) {
        t_scale = scale_factors["BCAL_TDC_SCALE"];
    } else {
        jerr << "Unable to get BCAL_TDC_SCALE from /BCAL/digi_scales !" << endl;
    }

    // load base time offset
    map<string,double> base_time_offset;
    if (jcalib->Get("/BCAL/base_time_offset",base_time_offset))
        jout << "Error loading /BCAL/base_time_offset !" << endl;
    if (base_time_offset.find("BCAL_TDC_BASE_TIME_OFFSET") != base_time_offset.end())
        t_base = base_time_offset["BCAL_TDC_BASE_TIME_OFFSET"];
    else
        jerr << "Unable to get BCAL_TDC_BASE_TIME_OFFSET from /BCAL/base_time_offset !" << endl;

    vector<double> raw_time_offsets;
    vector<double> raw_channel_global_offset;
    vector<double> raw_tdiff_u_d;

    if(jcalib->Get("/BCAL/TDC_offsets", raw_time_offsets))
        jout << "Error loading /BCAL/TDC_offsets !" << endl;
    if(jcalib->Get("/BCAL/channel_global_offset", raw_channel_global_offset))
        jout << "Error loading /BCAL/channel_global_offset !" << endl;
    if(jcalib->Get("/BCAL/tdiff_u_d", raw_tdiff_u_d))
        jout << "Error loading /BCAL/tdiff_u_d !" << endl;

    FillCalibTable(time_offsets, raw_time_offsets);
    FillCalibTableShort(channel_global_offset, raw_channel_global_offset);
    FillCalibTableShort(tdiff_u_d, raw_tdiff_u_d);
    
    return;
}

//------------------
// evnt
//------------------
void DBCALTDCHit_factory::Process(const std::shared_ptr<const JEvent>& event)
{
    /// Generate DBCALTDCHit object for each DBCALTDCDigiHit object.
    /// This is where the first set of calibration constants
    /// is applied to convert from digitzed units into natural
    /// units.

    // Get the TTabUtilities object
    const DTTabUtilities* locTTabUtilities = NULL;
    event->Get(&locTTabUtilities);

    vector<const DBCALTDCDigiHit*> digihits;
    event->Get(digihits);
    for(unsigned int i=0; i<digihits.size(); i++){
        const DBCALTDCDigiHit *digihit = digihits[i];

        DBCALTDCHit *hit = new DBCALTDCHit;
        hit->module = digihit->module;
        hit->layer  = digihit->layer;
        hit->sector = digihit->sector;
        hit->end    = digihit->end;

        // Apply calibration constants here
        double T, T_raw;

        //See if the input object is an DF1TDCHit. If so, it is real data.  If not, it is simulated data.
        const DF1TDCHit* F1TDCHit = digihit->GetSingle<DF1TDCHit>();

        double end_sign = digihit->end ? -1.0 : 1.0; // Upstream = 0 -> Positive
        if (F1TDCHit != NULL) { // This is real data.
            T_raw = locTTabUtilities->Convert_DigiTimeToNs_F1TDC(digihit) + t_base;
        } else { // This is simulated data.  Use a simplified time conversion.
            T_raw = digihit->time * t_scale + t_base;
        }
        T = T_raw - GetConstant(time_offsets,digihit) 
            - GetConstant(channel_global_offset,digihit) 
            - (0.5 * end_sign) * GetConstant(tdiff_u_d,digihit); 
        hit->t_raw = T_raw;
        hit->t = T;

        /*
           cout << "BCAL TDC Hit: ( " << digihit->module << ", " << digihit->layer << ", "
           << digihit->sector << ", " << digihit->end << " )  ->  "
           << T << " " << GetConstant(time_offsets,digihit) << " " << hit->t << endl;
           */         
        hit->AddAssociatedObject(digihit);

        Insert(hit);
    }
}

//------------------
// erun
//------------------
void DBCALTDCHit_factory::EndRun()
{
}

//------------------
// fini
//------------------
void DBCALTDCHit_factory::Finish()
{
}


//------------------
// FillCalibTable
//------------------
void DBCALTDCHit_factory::FillCalibTable( bcal_digi_constants_t &table, 
        const vector<double> &raw_table) 
{
    char str[256];
    int channel = 0;

    // reset the table before filling it
    table.clear();

    for(int module=1; module<=BCAL_NUM_MODULES; module++) {
        for(int layer=1; layer<=BCAL_NUM_TDC_LAYERS; layer++) {  
            for(int sector=1; sector<=BCAL_NUM_SECTORS; sector++) {
                if( (channel > BCAL_MAX_TDC_CHANNELS) || (channel+1 > BCAL_MAX_TDC_CHANNELS) ) {  // sanity check
                    sprintf(str, "Too many channels for BCAL table! channel=%d (should be %d)", 
                            channel, BCAL_MAX_TDC_CHANNELS);
                    cerr << str << endl;
                    throw JException(str);
                }

                table.push_back( cell_calib_t(raw_table[channel],raw_table[channel+1]) );

                channel += 2;
            }
        }
    }

    // check to make sure that we loaded enough channels
    if(channel != BCAL_MAX_TDC_CHANNELS) { 
        sprintf(str, "Not enough channels for BCAL table! channel=%d (should be %d)", 
                channel, BCAL_MAX_TDC_CHANNELS);
        cerr << str << endl;
        throw JException(str);
    }
}

//------------------
// FillCalibTableShort
//------------------
void DBCALTDCHit_factory::FillCalibTableShort( bcal_digi_constants_t &table,
        const vector<double> &raw_table)
{
    char str[256];
    int channel = 0;

    // reset the table before filling it
    table.clear();
    // These short tables have 768 values
    for(int module=1; module<=BCAL_NUM_MODULES; module++) {
        for(int layer=1; layer<=BCAL_NUM_LAYERS; layer++) {
            for(int sector=1; sector<=BCAL_NUM_SECTORS; sector++) {
                if (layer == 4) {
                    channel += 1;
                    continue;
                }
                if( channel > BCAL_MAX_CHANNELS/2 ) {  // sanity check
                    sprintf(str, "Too many channels for BCAL table! channel=%d (should be %d)",
                            channel, BCAL_MAX_CHANNELS/2);
                    cerr << str << endl;
                    throw JException(str);
                }

                table.push_back( cell_calib_t(raw_table[channel],raw_table[channel]) );

                channel += 1;
            }
        }
    }

    // check to make sure that we loaded enough channels
    if(channel != BCAL_MAX_CHANNELS/2) {
        sprintf(str, "Not enough channels for BCAL table! channel=%d (should be %d)",
                channel, BCAL_MAX_CHANNELS/2);
        cerr << str << endl;
        throw JException(str);
    }
}

//------------------------------------
// GetConstant
//   Allow a few different interfaces
//------------------------------------
const double DBCALTDCHit_factory::GetConstant( const bcal_digi_constants_t &the_table, 
        const int in_module, const int in_layer,
        const int in_sector, const int in_end) const
{
    char str[256];

    if( (in_module <= 0) || (in_module > BCAL_NUM_MODULES)) {
        sprintf(str, "Bad module # requested in DBCALTDCHit_factory::GetConstant()! requested=%d , should be 1-%d", in_module, BCAL_NUM_MODULES);
        cerr << str << endl;
        throw JException(str);
    }
    if( (in_layer <= 0) || (in_layer > BCAL_NUM_TDC_LAYERS)) {
        sprintf(str, "Bad layer # requested in DBCALTDCHit_factory::GetConstant()! requested=%d , should be 1-%d", in_layer, BCAL_NUM_TDC_LAYERS);
        cerr << str << endl;
        throw JException(str);
    }
    if( (in_sector <= 0) || (in_sector > BCAL_NUM_SECTORS)) {
        sprintf(str, "Bad sector # requested in DBCALTDCHit_factory::GetConstant()! requested=%d , should be 1-%d", in_sector, BCAL_NUM_SECTORS);
        cerr << str << endl;
        throw JException(str);
    }
    if( (in_sector <= 0) || (in_sector > BCAL_NUM_SECTORS)) {
        sprintf(str, "Bad sector # requested in DBCALTDCHit_factory::GetConstant()! requested=%d , should be 1-%d", in_sector, BCAL_NUM_SECTORS);
        cerr << str << endl;
        throw JException(str);
    }
    if( (in_end != DBCALGeometry::kUpstream) && (in_end != DBCALGeometry::kDownstream) ) {
        sprintf(str, "Bad end # requested in DBCALTDCHit_factory::GetConstant()! requested=%d , should be 0-1", in_end);
        cerr << str << endl;
        throw JException(str);
    }

    const int the_cell = GetCalibIndex( in_module, in_layer, in_sector);

    if(in_end == DBCALGeometry::kUpstream) {
        // handle the upstream end
        return the_table.at(the_cell).first;
    } else {
        // handle the downstream end
        return the_table.at(the_cell).second;
    }

}

const double DBCALTDCHit_factory::GetConstant( const bcal_digi_constants_t &the_table, 
        const DBCALTDCHit *in_hit) const
{
    char str[256];

    if( (in_hit->module <= 0) || (in_hit->module > BCAL_NUM_MODULES)) {
        sprintf(str, "Bad module # requested in DBCALTDCHit_factory::GetConstant()! requested=%d , should be 1-%d", in_hit->module, BCAL_NUM_MODULES);
        cerr << str << endl;
        throw JException(str);
    }
    if( (in_hit->layer <= 0) || (in_hit->layer > BCAL_NUM_TDC_LAYERS)) {
        sprintf(str, "Bad layer # requested in DBCALTDCHit_factory::GetConstant()! requested=%d , should be 1-%d", in_hit->layer, BCAL_NUM_TDC_LAYERS);
        cerr << str << endl;
        throw JException(str);
    }
    if( (in_hit->sector <= 0) || (in_hit->sector > BCAL_NUM_SECTORS)) {
        sprintf(str, "Bad sector # requested in DBCALTDCHit_factory::GetConstant()! requested=%d , should be 1-%d", in_hit->sector, BCAL_NUM_SECTORS);
        cerr << str << endl;
        throw JException(str);
    }
    if( (in_hit->end != DBCALGeometry::kUpstream) && (in_hit->end != DBCALGeometry::kDownstream) ) {
        sprintf(str, "Bad end # requested in DBCALTDCHit_factory::GetConstant()! requested=%d , should be 0-1", in_hit->end);
        cerr << str << endl;
        throw JException(str);
    }

    const int the_cell = GetCalibIndex( in_hit->module, in_hit->layer, in_hit->sector);

    if(in_hit->end == DBCALGeometry::kUpstream) {
        // handle the upstream end
        return the_table.at(the_cell).first;
        //return the_table[the_cell].first;
    } else {
        // handle the downstream end
        return the_table.at(the_cell).second;
        //return the_table[the_cell].second;
    }

}

const double DBCALTDCHit_factory::GetConstant( const bcal_digi_constants_t &the_table, 
        const DBCALTDCDigiHit *in_digihit) const
{
    char str[256];

    if( (in_digihit->module <= 0) || (in_digihit->module > static_cast<unsigned int>(BCAL_NUM_MODULES))) {
        sprintf(str, "Bad module # requested in DBCALTDCHit_factory::GetConstant()! requested=%d , should be 1-%d", in_digihit->module, BCAL_NUM_MODULES);
        cerr << str << endl;
        throw JException(str);
    }
    if( (in_digihit->layer <= 0) || (in_digihit->layer > static_cast<unsigned int>(BCAL_NUM_TDC_LAYERS))) {
        sprintf(str, "Bad layer # requested in DBCALTDCHit_factory::GetConstant()! requested=%d , should be 1-%d", in_digihit->layer, BCAL_NUM_TDC_LAYERS);
        cerr << str << endl;
        throw JException(str);
    }
    if( (in_digihit->sector <= 0) || (in_digihit->sector >static_cast<unsigned int>( BCAL_NUM_SECTORS))) {
        throw JException(str);
    }

    const int the_cell = GetCalibIndex( in_digihit->module, in_digihit->layer, in_digihit->sector);

    if(in_digihit->end == DBCALGeometry::kUpstream) {
        // handle the upstream end
        return the_table.at(the_cell).first;
    } else {
        // handle the downstream end
        return the_table.at(the_cell).second;
    }

}
/*
   const double DBCALTDCHit_factory::GetConstant( const bcal_digi_constants_t &the_table,
   const DTranslationTable *ttab,
   const int in_rocid, const int in_slot, const int in_channel) const {

   char str[256];

   DTranslationTable::csc_t daq_index = { in_rocid, in_slot, in_channel };
   DTranslationTable::DChannelInfo channel_info = ttab->GetDetectorIndex(daq_index);

   if( (channel_info.bcal.module <= 0) 
   || (channel_info.bcal.module > static_cast<unsigned int>(BCAL_NUM_MODULES))) {
   sprintf(str, "Bad module # requested in DBCALTDCHit_factory::GetConstant()! requested=%d , should be 1-%d", channel_info.bcal.module, BCAL_NUM_MODULES);
   cerr << str << endl;
   throw JException(str);
   }
   if( (channel_info.bcal.layer <= 0) 
   || (channel_info.bcal.layer > static_cast<unsigned int>(BCAL_NUM_TDC_LAYERS))) {
   sprintf(str, "Bad layer # requested in DBCALTDCHit_factory::GetConstant()! requested=%d , should be 1-%d", channel_info.bcal.layer, BCAL_NUM_TDC_LAYERS);
   cerr << str << endl;
   throw JException(str);
   }
   if( (channel_info.bcal.sector <= 0) 
   || (channel_info.bcal.sector > static_cast<unsigned int>(BCAL_NUM_SECTORS))) {
   sprintf(str, "Bad sector # requested in DBCALTDCHit_factory::GetConstant()! requested=%d , should be 1-%d", channel_info.bcal.sector, BCAL_NUM_SECTORS);
   cerr << str << endl;
   throw JException(str);
   }
   if( (channel_info.bcal.end != DBCALGeometry::kUpstream) 
   && (channel_info.bcal.end != DBCALGeometry::kDownstream) ) {
   sprintf(str, "Bad end # requested in DBCALTDCHit_factory::GetConstant()! requested=%d , should be 0-1", channel_info.bcal.end);
   cerr << str << endl;
   throw JException(str);
   }

   int the_cell = DBCALGeometry::cellId(channel_info.bcal.module,
   channel_info.bcal.layer,
   channel_info.bcal.sector);

   if(channel_info.bcal.end == DBCALGeometry::kUpstream) {
// handle the upstream end
return the_table.at(the_cell).first;
} else {
// handle the downstream end
return the_table.at(the_cell).second;
}
}
*/
