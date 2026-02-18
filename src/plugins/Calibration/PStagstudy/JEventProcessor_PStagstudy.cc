//
//    File: JEventProcessor_PStagstudy.cc
//

//#define VERBOSE 1

#include <iostream>
#include <stdint.h>
#include <vector>
#include <map>
#include <utility>
#include <TMath.h>
#include <TInterpreter.h>
#include <TTree.h>
#include <TFile.h>

#include "JEventProcessor_PStagstudy.h"
#include <JANA/JApplication.h>
#include <JANA/Services/JLockService.h>

using namespace std;
using namespace jana;

#include <TAGGER/DTAGMHit.h>
#include <TAGGER/DTAGMDigiHit.h>
#include <TAGGER/DTAGMTDCDigiHit.h>
#include <TAGGER/DTAGHHit.h>
#include <TAGGER/DTAGHDigiHit.h>
#include <TAGGER/DTAGHTDCDigiHit.h>
#include <DAQ/Df250WindowRawData.h>
#include <TTAB/DTranslationTable.h>
#include <RF/DRFTime.h>
#include <PAIR_SPECTROMETER/DPSCPair.h>
#include <PAIR_SPECTROMETER/DPSPair.h>
#include <PAIR_SPECTROMETER/DPSCDigiHit.h>
#include <TRIGGER/DL1Trigger.h>
#include <PID/DBeamPhoton.h>
#include <DAQ/DCODAEventInfo.h>
#include <DAQ/DCODAControlEvent.h>
#include <DAQ/DCODAROCInfo.h>
#include <DAQ/Df250PulseData.h>
#include <DAQ/DBeamCurrent.h>
#include <DAQ/DTSscalers.h>
#include <GlueX.h>
#include <TDirectory.h>
#include <TH1.h>
#include <TH2.h>

#include "rxConnection.h"

std::vector<std::pair<uint32_t, uint32_t> > epoch_time_limits;
std::map<uint32_t, uint32_t> beam_current_from_epics;
std::string beam_current_record_url("root://nod65.phys.uconn.edu"
                                   "/Gluex/resilient/resources/"
                                   "HallD_beam_current_record-11-15-2024.root");
std::string beam_current_record_tree("beamcur");
unsigned long int max_run_duration(100000);

// The individual fadc250 readout thresholds are stored separately and
// loaded into the frontend modules by the CODA daq at run start time.
// Since they are not saved in any database for automatic access, I 
// simply hard-code them here for specific run periods.

#define ROCTAGM1_WINTER_2023_V1_CNF 1

const int tagm_fadc250_channels(128);
const int tagh_fadc250_channels(300);

const int tagm_fadc250_rocid(71);
const int tagm_fadc250_slot0(3);

int tagh_fadc250_readout_threshold[300] = {
  300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300,
  300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300,
  300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300,
  300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300,
  300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300,
  300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300,
  300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300,
  300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300,
  300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300,
  300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300,
  300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300,
  300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300,
  300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300,
  300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300,
  300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300,
  300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300,
  300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300,
  300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300,
  300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300,
  300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300
};

#if defined ROCTAGM1_SPRING_2017_V2_CNF

int tagm_fadc250_readout_threshold[128] = {
  350, 310, 354, 397, 411, 419, 301, 343, 384, 348, 342, 335, 345, 333, 301, 280,
  293, 339, 367, 286, 291, 329, 359, 228, 262, 254, 284, 451, 347, 211, 195, 167,
  148, 144, 152, 143, 154, 222, 187, 160, 175, 185, 203, 157, 179, 175, 201, 233,
  232, 164, 170, 183, 406, 598, 582, 555, 663, 453, 447, 550, 598, 618, 706, 536,
  536, 663, 686, 526, 518, 647, 524, 730, 500, 543, 791, 528, 607, 696, 615, 526,
  198, 659, 390, 711, 581, 618, 718, 699, 454, 656, 597, 597, 554, 480, 577, 601,
  492, 581, 773, 666, 706, 559, 731, 749, 728, 600, 668, 654, 652, 619, 566, 423,
  673, 648, 608, 581, 522, 567, 596, 669, 405, 214, 1000, 1000, 1000, 1000, 1000, 1000
};

#elif defined ROCTAGM1_SPRING_2017_V3_CNF

int tagm_fadc250_readout_threshold[128] = {
  354, 310, 350, 419, 411, 397, 348, 342, 335, 345, 333, 384, 343, 301, 293, 280,
  301, 286, 267, 339, 359, 329, 291, 254, 262, 228, 347, 451, 284, 148, 144, 152,
  143, 154, 167, 195, 211, 160, 187, 222, 203, 185, 175, 175, 179, 157, 232, 233,
  201, 183, 170, 164, 582, 598, 406, 453, 663, 555, 598, 550, 447, 536, 706, 618,
  686, 663, 536, 647, 518, 526, 500, 730, 524, 528, 791, 543, 615, 696, 607, 659,
  198, 526, 581, 711, 390, 699, 718, 618, 597, 554, 480, 577, 601, 597, 656, 454,
  773, 581, 492, 559, 706, 666, 728, 749, 731, 654, 668, 600, 566, 619, 652, 608,
  581, 522, 567, 596, 648, 673, 423, 669, 405, 214, 1000, 1000, 1000, 1000, 1000, 1000
};

#elif defined ROCTAGM1_SPRING_2018_V1_CNF

int tagm_fadc250_readout_threshold[128] = {
  250, 250, 228, 251, 250, 250, 251, 250, 252, 250, 251, 250, 250, 251, 224, 251, 
  250, 245, 250, 250, 251, 251, 250, 150, 234, 192, 251, 250, 204, 154, 154, 154, 
  154, 154, 154, 183, 196, 220, 157, 188, 162, 168, 164, 153, 145, 143, 209, 190, 
  172, 148, 164, 170, 251, 250, 250, 250, 250, 251, 250, 251, 250, 250, 249, 250, 
  250, 250, 251, 250, 250, 250, 251, 250, 250, 250, 250, 250, 250, 249, 250, 250, 
  251, 250, 250, 250, 250, 250, 250, 250, 252, 250, 252, 251, 251, 250, 250, 250, 
  250, 250, 251, 250, 250, 250, 250, 251, 250, 250, 250, 250, 250, 250, 251, 251, 
  252, 250, 251, 252, 250, 250, 250, 250, 250, 208, 1000, 1000, 1000, 1000, 1000, 1000
};

#elif defined ROCTAGM1_FALL_2018_V1_CNF

int tagm_fadc250_readout_threshold[128] = {
  250, 250, 228, 251, 250, 250, 251, 250, 252, 250, 251, 250, 250, 251, 224, 251, 
  250, 245, 250, 250, 251, 251, 250, 150, 234, 192, 251, 250, 204, 154, 154, 154, 
  154, 154, 154, 183, 196, 220, 157, 188, 162, 168, 164, 153, 145, 143, 209, 190, 
  172, 148, 164, 170, 251, 250, 250, 250, 250, 251, 250, 251, 250, 250, 249, 250, 
  250, 250, 251, 250, 250, 250, 251, 250, 250, 250, 250, 250, 250, 249, 250, 250, 
  251, 250, 250, 250, 250, 250, 250, 250, 252, 250, 252, 251, 251, 250, 250, 250, 
  250, 250, 251, 250, 250, 250, 250, 251, 250, 250, 250, 250, 250, 250, 251, 251, 
  252, 250, 251, 252, 250, 250, 250, 250, 250, 208, 1000, 1000, 1000, 1000, 1000, 1000
};

#elif defined ROCTAGM1_SPRING_2019_V1_CNF

int tagm_fadc250_readout_threshold[128] = {
  420, 424, 396, 321, 381, 373, 456, 456, 456, 456, 456, 456, 415, 421, 416, 428,
  451, 451, 438, 424, 414, 423, 387, 426, 396, 416, 416, 437, 420, 394, 394, 394,
  394, 394, 394, 378, 348, 391, 415, 388, 381, 410, 372, 368, 361, 388, 429, 450,
  436, 420, 443, 437, 285, 297, 286, 246, 305, 305, 277, 304, 286, 316, 330, 319,
  311, 329, 290, 290, 302, 319, 145, 316, 304, 305, 347, 359, 315, 344, 325, 323,
  340, 328, 302, 313, 255, 338, 360, 247, 318, 318, 318, 318, 318, 318, 321, 261,
  328, 332, 325, 296, 329, 323, 331, 347, 360, 327, 307, 322, 305, 324, 360, 300,
  300, 300, 300, 300, 300, 353, 291, 331, 225, 203, 1000, 1000, 1000, 1000, 1000, 1000
};

#elif defined ROCTAGM1_FALL_2019_V1_CNF

int tagm_fadc250_readout_threshold[128] = {
  480, 482, 460, 318, 446, 398, 676, 616, 564, 518, 578, 518, 422, 400, 484, 502,
  528, 514, 454, 482, 468, 492, 466, 494, 464, 512, 482, 506, 498, 524, 476, 480,
  502, 492, 476, 446, 398, 420, 496, 426, 456, 504, 436, 432, 444, 424, 486, 534,
  478, 450, 486, 518, 272, 222, 244, 222, 258, 276, 222, 254, 252, 242, 280, 290,
  268, 260, 256, 262, 272, 278, 300, 276, 250, 266, 280, 332, 266, 326, 288, 286,
  290, 302, 236, 276, 226, 276, 284, 282, 404, 392, 264, 388, 474, 264, 300, 274,
  294, 296, 284, 226, 264, 294, 284, 302, 314, 264, 292, 298, 262, 276, 310, 340,
  316, 268, 422, 436, 268, 326, 248, 304, 162, 166, 999, 999, 999, 999, 999, 999
};

#elif defined ROCTAGM1_SPRING_2020_V1_CNF

int tagm_fadc250_readout_threshold[128] = {
  252, 252, 244, 188, 238, 220, 330, 306, 286, 268, 292, 268, 230, 220, 254, 260,
  270, 266, 242, 254, 246, 256, 246, 258, 246, 264, 254, 262, 260, 270, 250, 252,
  262, 256, 250, 238, 220, 228, 258, 230, 242, 262, 234, 232, 238, 230, 254, 274,
  252, 240, 254, 268, 168, 148, 158, 148, 162, 170, 150, 162, 160, 158, 172, 176,
  166, 164, 162, 164, 170, 172, 180, 170, 160, 166, 172, 192, 166, 190, 176, 174,
  176, 182, 154, 170, 150, 170, 174, 174, 222, 216, 166, 216, 250, 166, 180, 170,
  178, 178, 174, 150, 166, 178, 174, 182, 186, 166, 176, 180, 164, 170, 184, 196,
  186, 166, 228, 234, 166, 190, 158, 182, 124, 126, 999, 999, 999, 999, 999, 999
};

#elif defined ROCTAGM1_FALL_2021_V1_CNF

int tagm_fadc250_readout_threshold[128] = {
  252, 252, 244, 188, 238, 220, 330, 306, 286, 268, 292, 268, 230, 220, 254, 260,
  270, 266, 242, 254, 246, 256, 246, 258, 246, 264, 254, 262, 260, 270, 250, 252,
  262, 256, 250, 238, 220, 228, 258, 230, 242, 262, 234, 232, 238, 230, 254, 274,
  252, 240, 254, 268, 168, 148, 158, 148, 162, 170, 150, 162, 160, 158, 172, 176,
  166, 164, 162, 164, 170, 172, 180, 170, 160, 166, 172, 192, 166, 190, 176, 174,
  176, 182, 154, 170, 150, 170, 174, 174, 222, 216, 166, 216, 250, 166, 180, 170,
  178, 178, 174, 150, 166, 178, 174, 182, 186, 166, 176, 180, 164, 170, 184, 196,
  186, 166, 228, 234, 166, 190, 158, 182, 124, 126, 999, 999, 999, 999, 999, 999
};

#elif defined ROCTAGM1_SUMMER_2022_V1_CNF

int tagm_fadc250_readout_threshold[128] = {
  215, 212, 212, 178, 198, 211, 199, 199, 199, 199, 199, 223, 206, 172, 205, 210,
  213, 189, 203, 195, 179, 180, 174, 162, 162, 173, 150, 167, 166, 199, 199, 199,
  199, 199, 155, 151, 141, 140, 156, 144, 159, 159, 158, 152, 159, 157, 169, 166,
  162, 169, 163, 166, 133, 127, 130, 127, 132, 135, 125, 129, 128, 128, 135, 136,
  134, 131, 130, 131, 137, 137, 130, 199, 129, 132, 134, 146, 135, 138, 135, 135,
  140, 141, 136, 136, 127, 120, 133, 144, 199, 199, 199, 199, 199, 133, 137, 137,
  138, 146, 145, 142, 143, 134, 137, 133, 126, 132, 139, 136, 135, 135, 141, 199,
  199, 199, 199, 199, 199, 199, 199, 199, 199, 199, 199, 199, 199, 199, 199, 199
};

#elif defined ROCTAGM1_FALL_2022_V1_CNF

int tagm_fadc250_readout_threshold[128] = {
  209, 208, 206, 176, 199, 207, 199, 199, 199, 199, 199, 218, 201, 170, 211, 206,
  213, 192, 191, 199, 183, 187, 180, 166, 169, 187, 167, 179, 172, 199, 199, 199,
  199, 199, 162, 168, 152, 146, 166, 151, 163, 174, 167, 156, 165, 164, 174, 177,
  172, 174, 172, 175, 133, 128, 135, 129, 132, 134, 126, 129, 128, 130, 132, 135,
  135, 132, 128, 133, 133, 137, 133, 136, 128, 128, 129, 134, 136, 147, 134, 134,
  143, 141, 139, 138, 128, 120, 128, 145, 199, 199, 199, 199, 199, 135, 137, 128,
  138, 138, 138, 141, 135, 133, 138, 132, 130, 132, 141, 138, 135, 133, 136, 199,
  199, 199, 199, 199, 199, 199, 199, 199, 199, 199, 199, 199, 199, 199, 199, 199
};

#elif defined ROCTAGM1_WINTER_2023_V1_CNF

int tagm_fadc250_readout_threshold[128] = {
 267, 277, 275, 217, 258, 268, 287, 287, 287, 287, 287, 287, 254, 214, 276, 272,
 282, 249, 240, 258, 231, 235, 226, 207, 210, 233, 187, 207, 191, 193, 193, 193,
 193, 193, 193, 199, 172, 171, 201, 187, 197, 218, 209, 187, 202, 207, 218, 229,
 216, 220, 218, 229, 163, 152, 158, 151, 162, 164, 147, 155, 151, 154, 164, 172,
 160, 155, 149, 157, 159, 170, 155, 162, 152, 157, 157, 179, 159, 174, 164, 158,
 165, 167, 120, 160, 150, 131, 151, 166, 158, 158, 158, 158, 158, 158, 166, 163,
 163, 172, 171, 162, 163, 158, 168, 145, 145, 154, 168, 164, 155, 155, 159, 157,
 157, 157, 157, 157, 157, 160, 154, 231, 167, 171, 199, 199, 199, 199, 199, 199
};

#endif

int fadc250_rowcolumn[128] = {3,2,1,6,5,4,1009,2009,3009,4009,5009,9,8,7,12,11,
                              10,15,14,13,18,17,16,21,20,19,24,23,22,1027,2027,3027,
                              4027,5027,27,26,25,30,29,28,33,32,31,36,35,34,39,38,
                              37,42,41,40,45,44,43,48,47,46,51,50,49,54,53,52,
                              57,56,55,60,59,58,63,62,61,66,65,64,69,68,67,72,
                              71,70,75,74,73,78,77,76,1081,2081,3081,4081,5081,81,80,79,
                              84,83,82,87,86,85,90,89,88,93,92,91,96,95,94,1099,
                              2099,3099,4099,5099,99,98,97,100,101,102,
                              123,124,125,126,127,128};
std::map<int, int> fadc250_channel_from_rowcolumn;

// main trigger is bit 0, ps triggers are bit 3
//#define SELECT_TRIGGER_TYPE 3

void report_bad_pmax(const DTAGMHit *itagm) {
   std::cout << "  DTAGMHit: column=" << itagm->column
             << " row=" << itagm->row << std::endl;
   std::vector<const DTAGMDigiHit*> digi_hits;
   itagm->Get(digi_hits);
   std::vector<const DTAGMDigiHit*>::iterator atagm;
   for (atagm = digi_hits.begin(); atagm != digi_hits.end(); ++atagm) {
      std::cout << "    DTAGMDigiHit: column=" << (*atagm)->column
                << " row=" << (*atagm)->row << std::endl;
   }
   std::vector<const DTAGMTDCDigiHit*> tdc_digi_hits;
   itagm->Get(tdc_digi_hits);
   std::vector<const DTAGMTDCDigiHit*>::iterator ttagm;
   for (ttagm = tdc_digi_hits.begin(); ttagm != tdc_digi_hits.end(); ++ttagm) {
      std::cout << "    DTAGMTDCDigiHit: column=" << (*ttagm)->column
                << " row=" << (*ttagm)->row << std::endl;
   }
}

extern "C"{
   void InitPlugin(JApplication *app) {
     InitJANAPlugin(app);
     app->Add(new JEventProcessor_PStagstudy());
   }
}

JEventProcessor_PStagstudy::JEventProcessor_PStagstudy()
 : pstags(0)
{
}


JEventProcessor_PStagstudy::~JEventProcessor_PStagstudy() {
}


const DTranslationTable::DChannelInfo
JEventProcessor_PStagstudy::GetDetectorIndex(const DTranslationTable *ttab,
                                             DTranslationTable::csc_t csc)
{
   DTranslationTable::DChannelInfo index;
   try {
      index = ttab->GetDetectorIndex(csc);
   }
   catch(...) {
      index.det_sys = DTranslationTable::UNKNOWN_DETECTOR;
   }
   return index;
}

void JEventProcessor_PStagstudy::Init() {

   // lock all root operations
   auto app = GetApplication();
   auto lock_svc = app->GetService<JLockService>();
   lock_svc->RootWriteLock();

   bc_factory = new DBeamCurrent_factory();
   bc_factory->SetApplication(app);
   bc_factory->Init();

   std::ifstream tlimits("epoch_time_limits");
   int t0, t1;
   if (tlimits.is_open()) {
      std::string line;
      while (tlimits >> t0 >> t1) {
         epoch_time_limits.emplace_back(t0, t1);
      }
      tlimits.close();
   }
   else {
      epoch_time_limits.emplace_back(0, 1999999999);
   }

   gInterpreter->GenerateDictionary("std::vector<std::vector<unsigned short> >", "vector"); 

   pstags = new TTree("pstags", "PS tag study");
   pstags->Branch("runno", &runno, "runno/i");
   pstags->Branch("eventno", &eventno, "eventno/i");
   pstags->Branch("trgigger", &trigger, "trigger/i");
   pstags->Branch("timestamp", &timestamp, "timestamp/l");
   pstags->Branch("epochtime", &epochtime, "epochtime/l");
   pstags->Branch("beamcurrent", &beamcurrent, "beamcurrent/i");

   pstags->Branch("nrf", &nrf, "nrf/I[0,999]");
   pstags->Branch("rf_sys", rf_sys, "rf_sys[nrf]/I");
   pstags->Branch("rf_time", rf_time, "rf_time[nrf]/D");

   pstags->Branch("ntagm", &ntagm, "ntagm/I[0,999]");
   pstags->Branch("tagm_seqno", tagm_seqno, "tagm_seqno[ntagm]/I");
   pstags->Branch("tagm_channel", tagm_channel, "tagm_channel[ntagm]/I");
   pstags->Branch("tagm_peak", tagm_peak, "tagm_peak[ntagm]/F");
   pstags->Branch("tagm_pint", tagm_pint, "tagm_pint[ntagm]/F");
   pstags->Branch("tagm_tadc", tagm_tadc, "tagm_tadc[ntagm]/F");
   pstags->Branch("tagm_toth", tagm_toth, "tagm_toth[ntagm]/F");
   pstags->Branch("tagm_ttdc", tagm_ttdc, "tagm_ttdc[ntagm]/F");
   pstags->Branch("tagm_time", tagm_time, "tagm_time[ntagm]/F");
   pstags->Branch("tagm_Etag", tagm_Etag, "tagm_Etag[ntagm]/F");
   pstags->Branch("tagm_pmax", tagm_pmax, "tagm_pmax[ntagm]/F");
   pstags->Branch("tagm_tlast", tagm_tlast, "tagm_tlast[ntagm]/F");
   pstags->Branch("tagm_plast", tagm_plast, "tagm_plast[ntagm]/F");
   pstags->Branch("tagm_base", tagm_base, "tagm_base[ntagm]/F");
   pstags->Branch("tagm_rothr", tagm_rothr, "tagm_rothr[ntagm]/F");
   pstags->Branch("tagm_ped", tagm_ped, "tagm_ped[ntagm]/F");
   pstags->Branch("tagm_multi", tagm_multi, "tagm_multi[ntagm]/I");
   pstags->Branch("tagm_qf", tagm_qf, "tagm_qf[ntagm]/I");
   pstags->Branch("tagm_bg", tagm_bg, "tagm_bg[ntagm]/I");
   pstags->Branch("tagm_has_adc", tagm_has_adc, "tagm_has_adc[ntagm]/I");
   pstags->Branch("tagm_has_tdc", tagm_has_tdc, "tagm_has_tdc[ntagm]/I");
   pstags->Branch("tagm_nped", tagm_nped, "tagm_nped[ntagm]/I");
   pstags->Branch("tagm_nint", tagm_nint, "tagm_nint[ntagm]/I");
   pstags->Branch("tagm_raw_waveform", &tagm_raw_waveform, 30000, 1);

   pstags->Branch("ntagh", &ntagh, "ntagh/I[0,999]");
   pstags->Branch("tagh_seqno", tagh_seqno, "tagh_seqno[ntagh]/I");
   pstags->Branch("tagh_counter", tagh_counter, "tagh_counter[ntagh]/I");
   pstags->Branch("tagh_peak", tagh_peak, "tagh_peak[ntagh]/F");
   pstags->Branch("tagh_pint", tagh_pint, "tagh_pint[ntagh]/F");
   pstags->Branch("tagh_tadc", tagh_tadc, "tagh_tadc[ntagh]/F");
   pstags->Branch("tagh_toth", tagh_toth, "tagh_toth[ntagh]/F");
   pstags->Branch("tagh_ttdc", tagh_ttdc, "tagh_ttdc[ntagh]/F");
   pstags->Branch("tagh_time", tagh_time, "tagh_time[ntagh]/F");
   pstags->Branch("tagh_Etag", tagh_Etag, "tagh_Etag[ntagh]/F");
   pstags->Branch("tagh_pmax", tagh_pmax, "tagh_pmax[ntagh]/F");
   pstags->Branch("tagh_ped", tagh_ped, "tagh_ped[ntagh]/F");
   pstags->Branch("tagh_tlast", tagh_tlast, "tagh_tlast[ntagh]/F");
   pstags->Branch("tagh_plast", tagh_plast, "tagh_plast[ntagh]/F");
   pstags->Branch("tagh_base", tagh_base, "tagh_base[ntagh]/F");
   pstags->Branch("tagh_rothr", tagh_rothr, "tagh_rothr[ntagh]/F");
   pstags->Branch("tagh_multi", tagh_multi, "tagh_multi[ntagh]/I");
   pstags->Branch("tagh_qf", tagh_qf, "tagh_qf[ntagh]/I");
   pstags->Branch("tagh_bg", tagh_bg, "tagh_bg[ntagh]/I");
   pstags->Branch("tagh_has_adc", tagh_has_adc, "tagh_has_adc[ntagh]/I");
   pstags->Branch("tagh_has_tdc", tagh_has_tdc, "tagh_has_tdc[ntagh]/I");
   pstags->Branch("tagh_nped", tagh_nped, "tagh_nped[ntagh]/I");
   pstags->Branch("tagh_nint", tagh_nint, "tagh_nint[ntagh]/I");
   pstags->Branch("tagh_raw_waveform", &tagh_raw_waveform, 30000, 1);

   pstags->Branch("nbeam", &nbeam, "nbeam/I[0,999]");
   pstags->Branch("beam_sys", beam_sys, "beam_sys[nbeam]/I");
   pstags->Branch("beam_E", beam_E, "beam_E[nbeam]/F");
   pstags->Branch("beam_t", beam_t, "beam_t[nbeam]/F");
   pstags->Branch("beam_z", beam_z, "beam_z[nbeam]/F");

   pstags->Branch("nps", &nps, "nps/I[0,999]");
   pstags->Branch("ps_seqno", ps_seqno, "ps_seqno[nps]/I");
   pstags->Branch("ps_arm", ps_arm, "ps_arm[nps]/I");
   pstags->Branch("ps_column", ps_column, "ps_column[nps]/I");
   pstags->Branch("ps_peak", ps_peak, "ps_peak[nps]/F");
   pstags->Branch("ps_pint", ps_pint, "ps_pint[nps]/F");
   pstags->Branch("ps_npix", ps_pint, "ps_npix[nps]/F");
   pstags->Branch("ps_t", ps_t, "ps_t[nps]/F");
   pstags->Branch("ps_E", ps_E, "ps_E[nps]/F");
   pstags->Branch("ps_tadc", ps_tadc, "ps_tadc[nps]/F");
   pstags->Branch("ps_toth", ps_toth, "ps_toth[nps]/F");
   pstags->Branch("ps_pmax", ps_pmax, "ps_pmax[nps]/F");
   pstags->Branch("ps_ped", ps_ped, "ps_ped[nps]/F");
   pstags->Branch("ps_multi", ps_multi, "ps_multi[nps]/I");
   pstags->Branch("ps_qf", ps_qf, "ps_qf[nps]/I");
   pstags->Branch("ps_nped", ps_nped, "ps_nped[nps]/I");
   pstags->Branch("ps_nint", ps_nint, "ps_nint[nps]/I");
   pstags->Branch("ps_raw_waveform", &ps_raw_waveform, 30000, 1);

   pstags->Branch("npsc", &npsc, "npsc/I[0,999]");
   pstags->Branch("psc_seqno", psc_seqno, "psc_seqno[npsc]/I");
   pstags->Branch("psc_arm", psc_arm, "pcs_arm[npsc]/I");
   pstags->Branch("psc_module", psc_module, "ps_module[npsc]/I");
   pstags->Branch("psc_counter", psc_counter, "psc_counter[npsc]/I");
   pstags->Branch("psc_peak", psc_peak, "psc_peak[npsc]/F");
   pstags->Branch("psc_pint", psc_pint, "psc_pint[npsc]/F");
   pstags->Branch("psc_npe", psc_npe, "psc_npe[npsc]/F");
   pstags->Branch("psc_t", psc_t, "psc_t[npsc]/F");
   pstags->Branch("psc_tadc", psc_tadc, "psc_tadc[npsc]/F");
   pstags->Branch("psc_ttdc", psc_ttdc, "psc_ttdc[npsc]/F");
   pstags->Branch("psc_toth", psc_toth, "psc_toth[npsc]/F");
   pstags->Branch("psc_pmax", psc_pmax, "psc_pmax[npsc]/F");
   pstags->Branch("psc_ped", psc_ped, "psc_ped[npsc]/F");
   pstags->Branch("psc_multi", psc_multi, "psc_multi[npsc]/I");
   pstags->Branch("psc_qf", psc_qf, "psc_qf[npsc]/I");
   pstags->Branch("psc_bg", psc_bg, "psc_bg[npsc]/I");
   pstags->Branch("psc_has_adc", psc_has_adc, "psc_has_adc[npsc]/I");
   pstags->Branch("psc_has_tdc", psc_has_tdc, "psc_has_tdc[npsc]/I");
   pstags->Branch("psc_nped", psc_nped, "psc_nped[npsc]/I");
   pstags->Branch("psc_nint", psc_nint, "psc_nint[npsc]/I");
   pstags->Branch("psc_raw_waveform", &psc_raw_waveform, 30000, 1);

   pstags->Branch("npairps", &npairps, "npairps/I[0,999]");
   pstags->Branch("Epair", Epair, "Epair[npairps]/F");
   pstags->Branch("tpair", tpair, "tpair[npairps]/F");
   pstags->Branch("psleft_peak", psleft_peak, "psleft_peak[npairps]/F");
   pstags->Branch("psright_peak", psright_peak, "psright_peak[npairps]/F");
   pstags->Branch("psleft_pint", psleft_pint, "psleft_pint[npairps]/F");
   pstags->Branch("psright_pint", psright_pint, "psright_pint[npairps]/F");
   pstags->Branch("psleft_time", psleft_time, "psleft_time[npairps]/F");
   pstags->Branch("psright_time", psright_time, "psright_time[npairps]/F");
   pstags->Branch("psEleft", psEleft, "psEleft[npairps]/F");
   pstags->Branch("psEright", psEright, "psEright[npairps]/F");
   pstags->Branch("pstleft", pstleft, "pstleft[npairps]/F");
   pstags->Branch("pstright", pstright, "pstright[npairps]/F");
   pstags->Branch("nleft_ps", nleft_ps, "nleft_ps[npairps]/I");
   pstags->Branch("nright_ps", nright_ps, "nright_ps[npairps]/I");

   pstags->Branch("npairpsc", &npairpsc, "npairpsc/I[0,999]");
   pstags->Branch("pscleft_seqno", pscleft_seqno, "pscleft_seqno[npairpsc]/I");
   pstags->Branch("pscright_seqno", pscright_seqno, "pscright_seqno[npairpsc]/I");
   pstags->Branch("pscleft_module", pscleft_module, "pscleft_module[npairpsc]/I");
   pstags->Branch("pscright_module", pscright_module, "pscright_module[npairpsc]/I");
   pstags->Branch("pscleft_peak", pscleft_peak, "pscleft_peak[npairpsc]/F");
   pstags->Branch("pscright_peak", pscright_peak, "pscright_peak[npairpsc]/F");
   pstags->Branch("pscleft_pint", pscleft_pint, "pscleft_pint[npairpsc]/F");
   pstags->Branch("pscright_pint", pscright_pint, "pscright_pint[npairpsc]/F");
   pstags->Branch("pscleft_ttdc", pscleft_ttdc, "pscleft_ttdc[npairpsc]/F");
   pstags->Branch("pscright_ttdc", pscright_ttdc, "pscright_ttdc[npairpsc]/F");
   pstags->Branch("pscleft_tadc", pscleft_tadc, "pscleft_tadc[npairpsc]/F");
   pstags->Branch("pscright_tadc", pscright_tadc, "pscright_tadc[npairpsc]/F");
   pstags->Branch("pscleft_t", pscleft_t, "pscleft_t[npairpsc]/F");
   pstags->Branch("pscright_t", pscright_t, "pscright_t[npairpsc]/F");
   pstags->Branch("pscleft_ped", pscleft_ped, "pscleft_ped[npairpsc]/F");
   pstags->Branch("pscright_ped", pscright_ped, "pscright_ped[npairpsc]/F");
   pstags->Branch("pscleft_qf", pscleft_qf, "pscleft_qf[npairpsc]/I");
   pstags->Branch("pscright_qf", pscright_qf, "pscright_qf[npairpsc]/I");

   for (int i=0; i < tagm_fadc250_channels; ++i) {
      char name[99], title[99];
      tagm_hpedestal[i] = new TH1D((sprintf(name, "tagm_hpedestal_%d", i), name),
                              (sprintf(title, "tagm pulse pedestal channel %d", i), title),
                              200, 0, 200);
      for (int j=0; j<25; ++j) {
         tagm_hpedestal[i]->Fill(100);
      }
   }
   for (int i=0; i < tagh_fadc250_channels; ++i) {
      char name[99], title[99];
      tagh_hpedestal[i] = new TH1D((sprintf(name, "tagh_hpedestal_%d", i), name),
                              (sprintf(title, "tagh pulse pedestal channel %d", i), title),
                              200, 0, 200);
      for (int j=0; j<25; ++j) {
         tagh_hpedestal[i]->Fill(100);
      }
   }

   // unlock
   lock_svc->RootUnLock();
}


void JEventProcessor_PStagstudy::BeginRun(const std::shared_ptr<const JEvent>& event) {

   // lock all root operations
   auto app = GetApplication();
   auto lock_svc = app->GetService<JLockService>();
   lock_svc->RootWriteLock();

   bc_factory->BeginRun(event);
   int runno = event->GetRunNumber();

   string RCDB_CONNECTION;
   if (getenv("RCDB_CONNECTION")!= NULL)
      RCDB_CONNECTION = getenv("RCDB_CONNECTION");
   else
      RCDB_CONNECTION = "mysql://rcdb@hallddb.jlab.org/rcdb";   // default to outward-facing MySQL DB

   rcdb::rxConnection rcdbconn(RCDB_CONNECTION);
   epoch_reference = rcdbconn.GetRunStartTime(runno);

   beam_current_from_epics.clear();
   TFile *bcfile = TFile::Open(beam_current_record_url.c_str());
   if (!bcfile || bcfile->IsZombie()) {
      std::cout << "JEventProcessor_PStagstudy::brun error - "
                << "failed to open EPICS beam current record at url "
                << beam_current_record_url << std::endl
                << "...continuing on without beam current information from EPICS"
                << std::endl;
      lock_svc->RootUnLock();
      return;
   }
   TTree *bctree = dynamic_cast<TTree*>(bcfile->Get(beam_current_record_tree.c_str()));
   if (!bctree) {
      std::cout << "JEventProcessor_PStagstudy::brun error - "
                << "failed to read tree " << beam_current_record_tree
                << " from " << beam_current_record_url << std::endl
                << "...continuing on without beam current information from EPICS"
                << std::endl;
      bcfile->Close();
      lock_svc->RootUnLock();
      return;
   }
   uint32_t tepoch_s;
   uint32_t ibeam_nA;
   bctree->SetBranchAddress("tepoch", &tepoch_s);
   bctree->SetBranchAddress("ibeam", &ibeam_nA);
   uint64_t nentries = bctree->GetEntries();
   for (uint64_t i=0; i < nentries; ++i) {
      bctree->GetEntry(i);
      if (tepoch_s >= epoch_reference)
         beam_current_from_epics[tepoch_s] = ibeam_nA;
      if (tepoch_s > epoch_reference + max_run_duration)
         break;
   }
   bcfile->Close();
   std::cout << "JEventProcessor_PStagstudy::brun read "
             << beam_current_from_epics.size() << " records"
             << " from EPICS for run " << runno << std::endl;
 
   // unlock
   lock_svc->RootUnLock();
}


void JEventProcessor_PStagstudy::Process(const std::shared_ptr<const JEvent>& event) {
   // This is called for every event. Use of common resources like writing
   // to a file or filling a histogram should be mutex protected. Using
   // event->Get(...) to get reconstructed objects (and thereby activating the
   // reconstruction algorithm) should be done outside of any mutex lock
   // since multiple threads may call this method at the same time.

   auto app = GetApplication();
   auto lock_svc = app->GetService<JLockService>();
   lock_svc->RootWriteLock();

   bc_factory->Process(event);
   double ticks_per_sec = bc_factory->ticks_per_sec;

   std::vector<const DCODAEventInfo*> event_info;
   event->Get(event_info);
   if (event_info.size() == 0) {
      lock_svc->RootUnLock();
      return;
   }

   std::vector<const DTSscalers*> scalers;
   event->Get(scalers);
   std::vector<const DTSscalers*>::iterator isc;
   for (isc = scalers.begin(); isc != scalers.end(); ++isc) {
      std::cout << "scalers found with time " << (*isc)->time << std::endl;
   }

   // only examine PS triggers
   const DL1Trigger *trig_words = 0;
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
   int trig_bits = fp_trig_mask > 0 ? 10 + fp_trig_mask : trig_mask;
#ifdef SELECT_TRIGGER_TYPE
   if ((trig_bits & (1 << SELECT_TRIGGER_TYPE)) == 0) {
      lock_svc->RootUnLock();
      return;
   }
#endif
 
   runno = event_info[0]->run_number;
   eventno = event_info[0]->event_number;
   timestamp = event_info[0]->avg_timestamp;
   trigger = trig_bits;

   std::vector<const DCODAControlEvent*> controls;
   event->Get(controls);
   std::vector<const DCODAControlEvent*>::iterator ictrl;
   for (ictrl = controls.begin(); ictrl != controls.end(); ++ictrl) {
      std::cout << "found control event with unix_time "
                << (*ictrl)->unix_time << std::endl;
      epoch_reference = (*ictrl)->unix_time;
   } 

   std::vector<const DBeamCurrent*> currents;
   event->Get(currents);
   std::vector<const DBeamCurrent*>::iterator icur;
   for (icur = currents.begin(); icur != currents.end(); ++icur) {
      // std::cout << "found DBeamCurrent with t "
      //           << (*icur)->t << std::endl;
      beamcurrent = (*icur)->Ibeam;
      bctime = (*icur)->t;
   } 

   epochtime = epoch_reference + timestamp/ticks_per_sec;
   int within_time_limits = 0;
   for (auto titer : epoch_time_limits) {
      if (epochtime > titer.first and epochtime < titer.second) {
         ++within_time_limits;
      }
   }
   if (! within_time_limits) {
      if ((eventno % 100000) < 2)
         std::cerr << "event " << eventno << " is outside time limits, discarding" << std::endl;
      lock_svc->RootUnLock();
      return;
   }

   if (beam_current_from_epics.size() > 0) {
      auto it = beam_current_from_epics.lower_bound(epochtime);
      beamcurrent = it->second;
   }

   std::vector<const DRFTime*> rf_times;
   event->Get(rf_times, "PSC");
   std::vector<const DRFTime*>::iterator irf;
   nrf = 0;
   for (irf = rf_times.begin(); irf != rf_times.end(); ++irf) {
      rf_sys[nrf] = 0x4000;
      rf_time[nrf] = (*irf)->dTime;
      nrf++;
   }
   event->Get(rf_times, "TAGH");
   for (irf = rf_times.begin(); irf != rf_times.end(); ++irf) {
      rf_sys[nrf] = 0x800;
      rf_time[nrf] = (*irf)->dTime;
      nrf++;
   }

   // get the raw window data, if any
   std::vector<const Df250WindowRawData*> traces;
   event->Get(traces);

   std::vector<const DTranslationTable*> ttables;
   event->Get(ttables);
   if (ttables.size() != 1) {
      std::cout << "Serious error in PStagstudy plugin - "
                << "unable to acquire the DAQ translation table!"
                << std::endl;
      lock_svc->RootUnLock();
      return;
   }
   const DTranslationTable *ttab = ttables[0];

   tagm_raw_waveform.clear();
   tagh_raw_waveform.clear();
   ps_raw_waveform.clear();
   psc_raw_waveform.clear();

   std::vector<const DTAGMHit*> tagm_hits;
   event->Get(tagm_hits, "Calib");
   std::vector<std::vector<float> > timelist;
   std::vector<std::vector<float> > peaklist;
   for (int i=0; i < tagm_fadc250_channels; ++i) {
      std::vector<float> evec;
      timelist.push_back(evec);
      peaklist.push_back(evec);
   }
   std::vector<const DTAGMHit*>::iterator itagm;
   int ntagm_per_channel[6][128] = {0};
   ntagm = 0;
   for (itagm = tagm_hits.begin(); itagm != tagm_hits.end(); ++itagm) {
      int row = (*itagm)->row;
      int column = (*itagm)->column;
      int channel = fadc250_channel_from_rowcolumn[row * 1000 + column];
      tagm_tlast[ntagm] = 0;
      tagm_plast[ntagm] = 0;
      for (int i=0; i < (int)timelist[channel].size(); ++i) {
         if (timelist[channel][i] > tagm_tlast[ntagm]) {
            tagm_tlast[ntagm] = timelist[channel][i];
            tagm_plast[ntagm] = peaklist[channel][i];
         }
      }
      timelist[channel].push_back((*itagm)->time_fadc);
      peaklist[channel].push_back((*itagm)->pulse_peak);
      tagm_rothr[ntagm] = tagm_fadc250_readout_threshold[channel];
      tagm_seqno[ntagm] = ntagm_per_channel[row][column]++;
      tagm_channel[ntagm] = column + row * 1000;
      tagm_peak[ntagm] = (*itagm)->pulse_peak;
      tagm_pint[ntagm] = (*itagm)->integral;
      tagm_tadc[ntagm] = (*itagm)->time_fadc;
      tagm_toth[ntagm] = 999;
      tagm_ttdc[ntagm] = (*itagm)->time_tdc;
      tagm_Etag[ntagm] = (*itagm)->E;
      tagm_time[ntagm] = (*itagm)->t;
      tagm_multi[ntagm] = 0;
      tagm_pmax[ntagm] = 999;
      tagm_ped[ntagm] = 999;
      tagm_qf[ntagm] = 999;
      tagm_bg[ntagm] = (*itagm)->bg;
      tagm_has_adc[ntagm] = (*itagm)->has_fADC;
      tagm_has_tdc[ntagm] = (*itagm)->has_TDC;
      tagm_nped[ntagm] = 999;
      tagm_nint[ntagm] = 999;
      std::vector<const DTAGMDigiHit*> digi_hits;
      (*itagm)->Get(digi_hits);
      std::vector<const DTAGMDigiHit*>::iterator atagm;
      for (atagm = digi_hits.begin(); atagm != digi_hits.end(); ++atagm) {
         if ((*atagm)->row == row &&
             (*atagm)->column == column)
         {
            tagm_pmax[ntagm] = (*atagm)->pulse_peak;
            tagm_ped[ntagm] = (*atagm)->pedestal / (*atagm)->nsamples_pedestal;
            tagm_qf[ntagm] = (*atagm)->QF;
            tagm_nped[ntagm] = (*atagm)->nsamples_pedestal;
            tagm_nint[ntagm] = (*atagm)->nsamples_integral;
            std::vector<const Df250PulseData*> pulse_data;
            (*atagm)->Get(pulse_data);
            std::vector<const Df250PulseData*>::iterator ptagm;
            for (ptagm = pulse_data.begin(); ptagm != pulse_data.end(); ++ptagm) {
               tagm_toth[ntagm] = (*ptagm)->nsamples_over_threshold * 4;
               // f_qpedestal = ((*ptagm)->QF_pedestal)? 1 : 0;
               // f_latepulse = ((*ptagm)->QF_NSA_beyond_PTW)? 1 : 0;
               // f_underflow = ((*ptagm)->QF_underflow)? 1 : 0;
               // f_overflow = ((*ptagm)->QF_overflow)? 1 : 0;
               // f_notpeak = ((*ptagm)->QF_vpeak_beyond_NSA)? 1 : 0;
               // f_nopeak = ((*ptagm)->QF_vpeak_not_found)? 1 : 0;
               // f_badped = ((*ptagm)->QF_bad_pedestal)? 1 : 0;
            }
            tagm_hpedestal[channel]->Fill(tagm_ped[ntagm]);
         }
      }
      int maxbin(tagm_hpedestal[channel]->GetMaximumBin());
      double bsum[2] = {0,0};
      for (int i=-1; i<2; ++i) {
         bsum[0] += tagm_hpedestal[channel]->GetBinContent(maxbin + i);
         bsum[1] += tagm_hpedestal[channel]->GetBinContent(maxbin + i) *
                    tagm_hpedestal[channel]->GetXaxis()->GetBinCenter(maxbin + i);
      }
      tagm_base[ntagm] = bsum[1] / bsum[0];

      std::vector<const DTAGMHit*> assoc_hits;
      (*itagm)->Get(assoc_hits);
      std::vector<const DTAGMHit*>::iterator jtagm;
      int mtagm = 0;
      for (jtagm = assoc_hits.begin(); jtagm != assoc_hits.end(); ++jtagm) {
         ntagm++;
         int row = (*jtagm)->row;
         int column = (*jtagm)->column;
         int channel = fadc250_channel_from_rowcolumn[row * 1000 + column];
         tagm_seqno[ntagm] = ntagm_per_channel[row][column]++;
         tagm_channel[ntagm] = column + row * 1000;
         tagm_tlast[ntagm] = 0;
         tagm_plast[ntagm] = 0;
         for (int i=0; i < (int)timelist[channel].size(); ++i) {
            if (timelist[channel][i] > tagm_tlast[ntagm]) {
               tagm_tlast[ntagm] = timelist[channel][i];
               tagm_plast[ntagm] = peaklist[channel][i];
            }
         }
         timelist[channel].push_back((*jtagm)->time_fadc);
         peaklist[channel].push_back((*jtagm)->pulse_peak);
         tagm_rothr[ntagm] = tagm_fadc250_readout_threshold[channel];
         tagm_peak[ntagm] = (*jtagm)->pulse_peak;
         tagm_pint[ntagm] = (*jtagm)->integral;
         tagm_tadc[ntagm] = (*jtagm)->time_fadc;
         tagm_toth[ntagm] = 999;
         tagm_ttdc[ntagm] = (*jtagm)->time_tdc;
         tagm_Etag[ntagm] = (*jtagm)->E;
         tagm_time[ntagm] = (*jtagm)->t;
         tagm_multi[ntagm] = ++mtagm;
         tagm_pmax[ntagm] = 999;
         tagm_ped[ntagm] = 999;
         tagm_qf[ntagm] = 999;
         tagm_bg[ntagm] = (*jtagm)->bg;
         tagm_has_adc[ntagm] = (*jtagm)->has_fADC;
         tagm_has_tdc[ntagm] = (*jtagm)->has_TDC;
         tagm_nped[ntagm] = 999;
         tagm_nint[ntagm] = 999;
         (*jtagm)->Get(digi_hits);
         for (atagm = digi_hits.begin(); atagm != digi_hits.end(); ++atagm) {
            if ((*atagm)->row == (*jtagm)->row &&
                (*atagm)->column == (*jtagm)->column)
            {
               tagm_pmax[ntagm] = (*atagm)->pulse_peak;
               tagm_ped[ntagm] = (*atagm)->pedestal / (*atagm)->nsamples_pedestal;
               tagm_qf[ntagm] = (*atagm)->QF;
               tagm_nped[ntagm] = (*atagm)->nsamples_pedestal;
               tagm_nint[ntagm] = (*atagm)->nsamples_integral;
               std::vector<const Df250PulseData*> pulse_data;
               (*atagm)->Get(pulse_data);
               std::vector<const Df250PulseData*>::iterator ptagm;
               for (ptagm = pulse_data.begin(); ptagm != pulse_data.end(); ++ptagm) {
                  tagm_toth[ntagm] = (*ptagm)->nsamples_over_threshold * 4;
                  // f_qpedestal = ((*ptagm)->QF_pedestal)? 1 : 0;
                  // f_latepulse = ((*ptagm)->QF_NSA_beyond_PTW)? 1 : 0;
                  // f_underflow = ((*ptagm)->QF_underflow)? 1 : 0;
                  // f_overflow = ((*ptagm)->QF_overflow)? 1 : 0;
                  // f_notpeak = ((*ptagm)->QF_vpeak_beyond_NSA)? 1 : 0;
                  // f_nopeak = ((*ptagm)->QF_vpeak_not_found)? 1 : 0;
                  // f_badped = ((*ptagm)->QF_bad_pedestal)? 1 : 0;
               }
               tagm_hpedestal[channel]->Fill(tagm_ped[ntagm]);
            }
         }
         int maxbin(tagm_hpedestal[channel]->GetMaximumBin());
         double bsum[2] = {0,0};
         for (int i=-1; i<2; ++i) {
            bsum[0] += tagm_hpedestal[channel]->GetBinContent(maxbin + i);
            bsum[1] += tagm_hpedestal[channel]->GetBinContent(maxbin + i) *
                       tagm_hpedestal[channel]->GetXaxis()->GetBinCenter(maxbin + i);
         }
         tagm_base[ntagm] = bsum[1] / bsum[0];
      }
      std::vector<unsigned short> trace;
      std::vector<const Df250WindowRawData*>::iterator itrace;
      for (itrace = traces.begin(); itrace != traces.end(); ++itrace) {
         DTranslationTable::csc_t csc = {(*itrace)->rocid, (*itrace)->slot, (*itrace)->channel};
         const DTranslationTable::DChannelInfo chaninfo = GetDetectorIndex(ttab, csc);
         if (chaninfo.det_sys == DTranslationTable::TAGM) {
            if ((int)chaninfo.tagm.row == row && (int)chaninfo.tagm.col == column) {
               trace = (*itrace)->samples;
            }
         }
      }
      tagm_raw_waveform.push_back(trace);
      ntagm++;
   }

   std::vector<const DTAGHHit*> tagh_hits;
   event->Get(tagh_hits, "Calib");
   timelist.clear();
   peaklist.clear();
   for (int i=0; i < tagh_fadc250_channels; ++i) {
      std::vector<float> evec;
      timelist.push_back(evec);
      peaklist.push_back(evec);
   }
   std::vector<const DTAGHHit*>::iterator itagh;
   int ntagh_per_counter[512] = {0};
   ntagh = 0;
   for (itagh = tagh_hits.begin(); itagh != tagh_hits.end(); ++itagh) {
      tagh_seqno[ntagh] = ntagh_per_counter[(*itagh)->counter_id]++;
      tagh_counter[ntagh] = (*itagh)->counter_id;
      int channel = (*itagh)->counter_id;
      tagh_tlast[ntagh] = 0;
      tagh_plast[ntagh] = 0;
      for (int i=0; i < (int)timelist[channel].size(); ++i) {
         if (timelist[channel][i] > tagh_tlast[ntagh]) {
            tagh_tlast[ntagh] = timelist[channel][i];
            tagh_plast[ntagh] = peaklist[channel][i];
         }
      }
      timelist[channel].push_back((*itagh)->time_fadc);
      peaklist[channel].push_back((*itagh)->pulse_peak);
      tagh_rothr[ntagh] = tagh_fadc250_readout_threshold[channel];
      tagh_peak[ntagh] = (*itagh)->pulse_peak;
      tagh_pint[ntagh] = (*itagh)->integral;
      tagh_tadc[ntagh] = (*itagh)->time_fadc;
      tagh_toth[ntagh] = 999;
      tagh_ttdc[ntagh] = (*itagh)->time_tdc;
      tagh_Etag[ntagh] = (*itagh)->E;
      tagh_time[ntagh] = (*itagh)->t;
      tagh_multi[ntagh] = 0;
      tagh_pmax[ntagh] = 999;
      tagh_ped[ntagh] = 999;
      tagh_qf[ntagh] = 999;
      tagh_bg[ntagh] = (*itagh)->bg;
      tagh_has_adc[ntagh] = (*itagh)->has_fADC;
      tagh_has_tdc[ntagh] = (*itagh)->has_TDC;
      tagh_nped[ntagh] = 999;
      tagh_nint[ntagh] = 999;
      std::vector<const DTAGHDigiHit*> digi_hits;
      (*itagh)->Get(digi_hits);
      std::vector<const DTAGHDigiHit*>::iterator atagh;
      for (atagh = digi_hits.begin(); atagh != digi_hits.end(); ++atagh) {
         if ((*atagh)->counter_id == (*itagh)->counter_id) {
            tagh_pmax[ntagh] = (*atagh)->pulse_peak;
            tagh_ped[ntagh] = (*atagh)->pedestal / (*atagh)->nsamples_pedestal;
            tagh_qf[ntagh] = (*atagh)->QF;
            tagh_nped[ntagh] = (*atagh)->nsamples_pedestal;
            tagh_nint[ntagh] = (*atagh)->nsamples_integral;
            std::vector<const Df250PulseData*> pulse_data;
            (*atagh)->Get(pulse_data);
            std::vector<const Df250PulseData*>::iterator ptagh;
            for (ptagh = pulse_data.begin(); ptagh != pulse_data.end(); ++ptagh) {
               tagh_toth[ntagh] = (*ptagh)->nsamples_over_threshold * 4;
               // f_qpedestal = ((*ptagh)->QF_pedestal)? 1 : 0;
               // f_latepulse = ((*ptagh)->QF_NSA_beyond_PTW)? 1 : 0;
               // f_underflow = ((*ptagh)->QF_underflow)? 1 : 0;
               // f_overflow = ((*ptagh)->QF_overflow)? 1 : 0;
               // f_notpeak = ((*ptagh)->QF_vpeak_beyond_NSA)? 1 : 0;
               // f_nopeak = ((*ptagh)->QF_vpeak_not_found)? 1 : 0;
               // f_badped = ((*ptagh)->QF_bad_pedestal)? 1 : 0;
            }
            tagh_hpedestal[channel]->Fill(tagh_ped[ntagh]);
         }
      }
      int maxbin(tagh_hpedestal[channel]->GetMaximumBin());
      double bsum[2] = {0,0};
      for (int i=-1; i<2; ++i) {
         bsum[0] += tagh_hpedestal[channel]->GetBinContent(maxbin + i);
         bsum[1] += tagh_hpedestal[channel]->GetBinContent(maxbin + i) *
                    tagh_hpedestal[channel]->GetXaxis()->GetBinCenter(maxbin + i);
      }
      tagh_base[ntagh] = bsum[1] / bsum[0];

      std::vector<const DTAGHHit*> assoc_hits;
      (*itagh)->Get(assoc_hits);
      std::vector<const DTAGHHit*>::iterator jtagh;
      int mtagh = 0;
      for (jtagh = assoc_hits.begin(); jtagh != assoc_hits.end(); ++jtagh) {
         ntagh++;
         int channel = (*jtagh)->counter_id;
         tagh_tlast[ntagh] = 0;
         tagh_plast[ntagh] = 0;
         for (int i=0; i < (int)timelist[channel].size(); ++i) {
            if (timelist[channel][i] > tagh_tlast[ntagh]) {
               tagh_tlast[ntagh] = timelist[channel][i];
               tagh_plast[ntagh] = peaklist[channel][i];
            }
         }
         timelist[channel].push_back((*jtagh)->time_fadc);
         peaklist[channel].push_back((*jtagh)->pulse_peak);
         tagh_rothr[ntagh] = tagh_fadc250_readout_threshold[channel];
         tagh_seqno[ntagh] = ntagh_per_counter[(*jtagh)->counter_id]++;
         tagh_counter[ntagh] = (*jtagh)->counter_id;
         tagh_peak[ntagh] = (*jtagh)->pulse_peak;
         tagh_pint[ntagh] = (*jtagh)->integral;
         tagh_tadc[ntagh] = (*jtagh)->time_fadc;
         tagh_toth[ntagh] = 999;
         tagh_ttdc[ntagh] = (*jtagh)->time_tdc;
         tagh_Etag[ntagh] = (*jtagh)->E;
         tagh_time[ntagh] = (*jtagh)->t;
         tagh_multi[ntagh] = ++mtagh;
         tagh_pmax[ntagh] = 999;
         tagh_ped[ntagh] = 999;
         tagh_qf[ntagh] = 999;
         tagh_bg[ntagh] = (*jtagh)->bg;
         tagh_has_adc[ntagh] = (*jtagh)->has_fADC;
         tagh_has_tdc[ntagh] = (*jtagh)->has_TDC;
         tagh_nped[ntagh] = 999;
         tagh_nint[ntagh] = 999;
         (*jtagh)->Get(digi_hits);
         for (atagh = digi_hits.begin(); atagh != digi_hits.end(); ++atagh) {
            if ((*atagh)->counter_id == (*itagh)->counter_id) {
               tagh_pmax[ntagh] = (*atagh)->pulse_peak;
               tagh_ped[ntagh] = (*atagh)->pedestal / (*atagh)->nsamples_pedestal;
               tagh_qf[ntagh] = (*atagh)->QF;
               tagh_nped[ntagh] = (*atagh)->nsamples_pedestal;
               tagh_nint[ntagh] = (*atagh)->nsamples_integral;
               std::vector<const Df250PulseData*> pulse_data;
               (*atagh)->Get(pulse_data);
               std::vector<const Df250PulseData*>::iterator ptagh;
               for (ptagh = pulse_data.begin(); ptagh != pulse_data.end(); ++ptagh) {
                  tagh_toth[ntagh] = (*ptagh)->nsamples_over_threshold * 4;
                  // f_qpedestal = ((*ptagh)->QF_pedestal)? 1 : 0;
                  // f_latepulse = ((*ptagh)->QF_NSA_beyond_PTW)? 1 : 0;
                  // f_underflow = ((*ptagh)->QF_underflow)? 1 : 0;
                  // f_overflow = ((*ptagh)->QF_overflow)? 1 : 0;
                  // f_notpeak = ((*ptagh)->QF_vpeak_beyond_NSA)? 1 : 0;
                  // f_nopeak = ((*ptagh)->QF_vpeak_not_found)? 1 : 0;
                  // f_badped = ((*ptagh)->QF_bad_pedestal)? 1 : 0;
               }
               tagh_hpedestal[channel]->Fill(tagh_ped[ntagh]);
            }
         }
         int maxbin(tagh_hpedestal[channel]->GetMaximumBin());
         double bsum[2] = {0,0};
         for (int i=-1; i<2; ++i) {
            bsum[0] += tagh_hpedestal[channel]->GetBinContent(maxbin + i);
            bsum[1] += tagh_hpedestal[channel]->GetBinContent(maxbin + i) *
                       tagh_hpedestal[channel]->GetXaxis()->GetBinCenter(maxbin + i);
         }
         tagh_base[ntagh] = bsum[1] / bsum[0];
      }
      std::vector<unsigned short> trace;
      std::vector<const Df250WindowRawData*>::iterator itrace;
      for (itrace = traces.begin(); itrace != traces.end(); ++itrace) {
         DTranslationTable::csc_t csc = {(*itrace)->rocid, (*itrace)->slot, (*itrace)->channel};
         const DTranslationTable::DChannelInfo chaninfo = GetDetectorIndex(ttab, csc);
         if (chaninfo.det_sys == DTranslationTable::TAGH) {
            if ((int)chaninfo.tagh.id == (*itagh)->counter_id) {
               trace = (*itrace)->samples;
            }
         }
      }
      tagh_raw_waveform.push_back(trace);
      ntagh++;
   }

   std::vector<const DPSHit*> ps_hits;
   event->Get(ps_hits);
   std::vector<const DPSHit*>::iterator ips;
   int nps_per_counter[512] = {0};
   nps = 0;
   for (ips = ps_hits.begin(); ips != ps_hits.end(); ++ips) {
      ps_arm[nps] = (*ips)->arm;
      ps_column[nps] = (*ips)->column;
      ps_seqno[nps] = nps_per_counter[ps_arm[nps] * 256 + ps_column[nps]]++;
      ps_E[nps] = (*ips)->E;
      ps_t[nps] = (*ips)->t;
      ps_pint[nps] = (*ips)->integral;
      ps_peak[nps] = (*ips)->pulse_peak;
      ps_npix[nps] = (*ips)->npix_fadc;
      ps_toth[nps] = 999;
      ps_tadc[nps] = 999;
      ps_multi[nps] = 0;
      ps_pmax[nps] = 999;
      ps_ped[nps] = 999;
      ps_qf[nps] = 999;
      ps_nped[nps] = 999;
      ps_nint[nps] = 999;
      std::vector<const DPSDigiHit*> digi_hits;
      (*ips)->Get(digi_hits);
      std::vector<const DPSDigiHit*>::iterator aps;
      for (aps = digi_hits.begin(); aps != digi_hits.end(); ++aps) {
         if ((*aps)->column == (*ips)->column && (*aps)->arm == (*ips)->arm) {
            ps_tadc[nps] = (*aps)->pulse_time;
            ps_pmax[nps] = (*aps)->pulse_peak;
            ps_ped[nps] = (*aps)->pedestal;
            ps_qf[nps] = (*aps)->QF;
            ps_nped[nps] = (*aps)->nsamples_pedestal;
            ps_nint[nps] = (*aps)->nsamples_integral;
            std::vector<const Df250PulseData*> pulse_data;
            (*aps)->Get(pulse_data);
            std::vector<const Df250PulseData*>::iterator pps;
            for (pps = pulse_data.begin(); pps != pulse_data.end(); ++pps) {
               ps_toth[nps] = (*pps)->nsamples_over_threshold * 4;
               // f_qpedestal = ((*pps)->QF_pedestal)? 1 : 0;
               // f_latepulse = ((*pps)->QF_NSA_beyond_PTW)? 1 : 0;
               // f_underflow = ((*pps)->QF_underflow)? 1 : 0;
               // f_overflow = ((*pps)->QF_overflow)? 1 : 0;
               // f_notpeak = ((*pps)->QF_vpeak_beyond_NSA)? 1 : 0;
               // f_nopeak = ((*pps)->QF_vpeak_not_found)? 1 : 0;
               // f_badped = ((*pps)->QF_bad_pedestal)? 1 : 0;
            }
         }
      }
      std::vector<unsigned short> trace;
      std::vector<const Df250WindowRawData*>::iterator itrace;
      for (itrace = traces.begin(); itrace != traces.end(); ++itrace) {
         DTranslationTable::csc_t csc = {(*itrace)->rocid, (*itrace)->slot, (*itrace)->channel};
         const DTranslationTable::DChannelInfo chaninfo = GetDetectorIndex(ttab, csc);
         if (chaninfo.det_sys == DTranslationTable::PS) {
            if ((int)chaninfo.ps.side == (*ips)->arm && (int)chaninfo.ps.id == (*ips)->column) {
               trace = (*itrace)->samples;
            }
         }
      }
      ps_raw_waveform.push_back(trace);
      nps++;
   }

   std::vector<const DPSCHit*> psc_hits;
   event->Get(psc_hits);
   std::vector<const DPSCHit*>::iterator ipsc;
   int npsc_per_counter[512] = {0};
   npsc = 0;
   for (ipsc = psc_hits.begin(); ipsc != psc_hits.end(); ++ipsc) {
      psc_arm[npsc] = (*ipsc)->arm;
      psc_module[npsc] = (*ipsc)->module;
      psc_counter[npsc] = ((*ipsc)->arm * 8 + (*ipsc)->module);
      psc_seqno[npsc] = npsc_per_counter[psc_counter[npsc]]++;
      psc_t[npsc] = (*ipsc)->t;
      psc_pint[npsc] = (*ipsc)->integral;
      psc_peak[npsc] = (*ipsc)->pulse_peak;
      psc_npe[npsc] = (*ipsc)->npe_fadc;
      psc_tadc[npsc] = (*ipsc)->time_fadc;
      psc_ttdc[npsc] = (*ipsc)->time_tdc;
      psc_has_adc[npsc] = 0;
      psc_has_tdc[npsc] = 0;
      psc_toth[npsc] = 999;
      psc_multi[npsc] = 0;
      psc_pmax[npsc] = 999;
      psc_ped[npsc] = 999;
      psc_qf[npsc] = 999;
      psc_nped[npsc] = 999;
      psc_nint[npsc] = 999;
      std::vector<const DPSCDigiHit*> digi_hits;
      (*ipsc)->Get(digi_hits);
      std::vector<const DPSCDigiHit*>::iterator apsc;
      for (apsc = digi_hits.begin(); apsc != digi_hits.end(); ++apsc) {
         if ((*apsc)->counter_id == (*ipsc)->arm * 8 + (*ipsc)->module) {
            psc_has_adc[npsc] = 1;
            psc_tadc[npsc] = (*apsc)->pulse_time;
            psc_pmax[npsc] = (*apsc)->pulse_peak;
            psc_ped[npsc] = (*apsc)->pedestal;
            psc_qf[npsc] = (*apsc)->QF;
            psc_nped[npsc] = (*apsc)->nsamples_pedestal;
            psc_nint[npsc] = (*apsc)->nsamples_integral;
            std::vector<const Df250PulseData*> pulse_data;
            (*apsc)->Get(pulse_data);
            std::vector<const Df250PulseData*>::iterator ppsc;
            for (ppsc = pulse_data.begin(); ppsc != pulse_data.end(); ++ppsc) {
               psc_toth[npsc] = (*ppsc)->nsamples_over_threshold * 4;
               // f_qpedestal = ((*pps)->QF_pedestal)? 1 : 0;
               // f_latepulse = ((*pps)->QF_NSA_beyond_PTW)? 1 : 0;
               // f_underflow = ((*pps)->QF_underflow)? 1 : 0;
               // f_overflow = ((*pps)->QF_overflow)? 1 : 0;
               // f_notpeak = ((*pps)->QF_vpeak_beyond_NSA)? 1 : 0;
               // f_nopeak = ((*pps)->QF_vpeak_not_found)? 1 : 0;
               // f_badped = ((*pps)->QF_bad_pedestal)? 1 : 0;
            }
         }
      }
      std::vector<const DPSCTDCDigiHit*> tdc_hits;
      (*ipsc)->Get(tdc_hits);
      std::vector<const DPSCTDCDigiHit*>::iterator tpsc;
      for (tpsc = tdc_hits.begin(); tpsc != tdc_hits.end(); ++tpsc) {
         if ((*tpsc)->counter_id == (*ipsc)->arm * 8 + (*ipsc)->module) {
            psc_has_tdc[npsc] = 1;
            psc_ttdc[npsc] = (*tpsc)->time;
         }
      }
      std::vector<unsigned short> trace;
      std::vector<const Df250WindowRawData*>::iterator itrace;
      for (itrace = traces.begin(); itrace != traces.end(); ++itrace) {
         DTranslationTable::csc_t csc = {(*itrace)->rocid, (*itrace)->slot, (*itrace)->channel};
         const DTranslationTable::DChannelInfo chaninfo = GetDetectorIndex(ttab, csc);
         if (chaninfo.det_sys == DTranslationTable::PSC) {
            if ((int)chaninfo.psc.id == (*ipsc)->arm * 8 + (*ipsc)->module) {
               trace = (*itrace)->samples;
            }
         }
      }
      ps_raw_waveform.push_back(trace);
      npsc++;
   }

   std::vector<const DBeamPhoton*> beams;
   event->Get(beams);
   std::vector<const DBeamPhoton*>::iterator ibeam;
   nbeam = 0;
   for (ibeam = beams.begin(); ibeam != beams.end(); ++ibeam) {
      beam_sys[nbeam] = (*ibeam)->dSystem;
      beam_E[nbeam] = (*ibeam)->energy();
      beam_t[nbeam] = (*ibeam)->time();
      beam_z[nbeam] = (*ibeam)->z();
      nbeam++;
   }

   std::vector<const DPSPair*> ps_pairs;
   event->Get(ps_pairs);
   std::vector<const DPSPair*>::iterator ipair;
   npairps = 0;
   for (ipair = ps_pairs.begin(); ipair != ps_pairs.end(); ++ipair) {
      Epair[npairps] = (*ipair)->left->E + (*ipair)->right->E;
      tpair[npairps] = ((*ipair)->left->t + (*ipair)->right->t)/2;
      psleft_peak[npairps] = (*ipair)->left->pulse_peak;
      psright_peak[npairps] = (*ipair)->right->pulse_peak;
      psleft_pint[npairps] = (*ipair)->left->integral;
      psright_pint[npairps] = (*ipair)->right->integral;
      psleft_time[npairps] = (*ipair)->left->t_tile;
      psright_time[npairps] = (*ipair)->right->t_tile;
      psEleft[npairps] = (*ipair)->left->E;
      psEright[npairps] = (*ipair)->right->E;
      pstleft[npairps] = (*ipair)->left->t;
      pstright[npairps] = (*ipair)->right->t;
      nleft_ps[npairps] = (*ipair)->right->ntiles;
      nright_ps[npairps] = (*ipair)->right->ntiles;
      ++npairps;
   }

   std::vector<const DPSCPair*> psc_pairs;
   event->Get(psc_pairs);
   std::vector<const DPSCPair*>::iterator icpair;
   int npsc_per_module[2][9] = {0};
   double tpsc_per_module[2][9] = {0};
   npairpsc = 0;
   for (icpair = psc_pairs.begin(); icpair != psc_pairs.end(); ++icpair) {
      int mod0 = (*icpair)->ee.first->module;
      int mod1 = (*icpair)->ee.second->module;
      if (tpsc_per_module[0][mod0] != (*icpair)->ee.first->time_tdc) {
         tpsc_per_module[0][mod0] = (*icpair)->ee.first->time_tdc;
         ++npsc_per_module[0][mod0];
      }
      if (tpsc_per_module[1][mod1] != (*icpair)->ee.second->time_tdc) {
         tpsc_per_module[1][mod1] = (*icpair)->ee.second->time_tdc;
         ++npsc_per_module[1][mod1];
      }
      pscleft_seqno[npairpsc] = npsc_per_module[0][mod0] - 1;
      pscright_seqno[npairpsc] = npsc_per_module[1][mod1] - 1;
      pscleft_module[npairpsc] = (*icpair)->ee.first->module;
      pscright_module[npairpsc] = (*icpair)->ee.second->module;
      pscleft_peak[npairpsc] = (*icpair)->ee.first->pulse_peak;
      pscright_peak[npairpsc] = (*icpair)->ee.second->pulse_peak;
      pscleft_pint[npairpsc] = (*icpair)->ee.first->integral;
      pscright_pint[npairpsc] = (*icpair)->ee.second->integral;
      pscleft_ttdc[npairpsc] = (*icpair)->ee.first->time_tdc;
      pscright_ttdc[npairpsc] = (*icpair)->ee.second->time_tdc;
      pscleft_tadc[npairpsc] = (*icpair)->ee.first->time_fadc;
      pscright_tadc[npairpsc] = (*icpair)->ee.second->time_fadc;
      pscleft_t[npairpsc] = (*icpair)->ee.first->t;
      pscright_t[npairpsc] = (*icpair)->ee.second->t;
      pscleft_ped[npairpsc] = 999;
      pscright_ped[npairpsc] = 999;
      pscleft_qf[npairpsc] = 999;
      pscright_qf[npairpsc] = 999;
      std::vector<const DPSCDigiHit*> digi_hits;
      (*icpair)->ee.first->Get(digi_hits);
      std::vector<const DPSCDigiHit*>::iterator apsc;
      for (apsc = digi_hits.begin(); apsc != digi_hits.end(); ++apsc) {
         pscleft_ped[npairpsc] = (*apsc)->pedestal;
         pscleft_qf[npairpsc] = (*apsc)->QF;
      }
      (*icpair)->ee.second->Get(digi_hits);
      for (apsc = digi_hits.begin(); apsc != digi_hits.end(); ++apsc) {
         pscright_ped[npairpsc] = (*apsc)->pedestal;
         pscright_qf[npairpsc] = (*apsc)->QF;
      }
      npairpsc++;
   }

#ifdef VERBOSE
   printf("Filling pstags with ntagm=%d, ntagh=%d, npairps=%d, npairpsc=%d\n",
          ntagm, ntagh, npairps, npairpsc);
#endif
   pstags->Fill();

   lock_svc->RootUnLock();
}


void JEventProcessor_PStagstudy::EndRun() {
}


void JEventProcessor_PStagstudy::Finish() {
}
