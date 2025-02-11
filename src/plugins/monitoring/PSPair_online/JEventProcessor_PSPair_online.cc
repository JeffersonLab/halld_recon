// $Id$
//
//    File: JEventProcessor_PSPair_online.cc
// Created: Fri Mar 20 16:32:04 EDT 2015
// Creator: nsparks (on Linux cua2.jlab.org 2.6.32-431.5.1.el6.x86_64 x86_64)
//
#include <iostream>
#include <sstream>
#include "JEventProcessor_PSPair_online.h"

using namespace std;

#include <PAIR_SPECTROMETER/DPSCPair.h>
#include <PAIR_SPECTROMETER/DPSPair.h>
#include <PAIR_SPECTROMETER/DPSGeometry.h>
#include <TAGGER/DTAGHHit.h>
#include <TAGGER/DTAGHGeometry.h>
#include <TAGGER/DTAGMHit.h>
#include <TAGGER/DTAGMGeometry.h>
#include <TPOL/DTPOLHit.h>
#include <TPOL/DTPOLHit_factory.h>

#include <JANA/JApplication.h>
#include <JANA/JFactoryT.h>
#include <TDirectory.h>
#include <TH1.h>
#include <TH2.h>


const int Narms = DPSGeometry::NUM_ARMS; // 2
const int NC_PSC = DPSGeometry::NUM_COARSE_COLUMNS; // 8: number of PSC modules (counters) per arm
const int NC_PS = DPSGeometry::NUM_FINE_COLUMNS; // 145: number of PS columns (tiles) per arm
const int NC_TAGH = DTAGHGeometry::kCounterCount; // 274: number of TAGH counters
const int NC_TAGM = DTAGMGeometry::kColumnCount; // 102: number of TAGM columns
const int NS_TPOL = DTPOLHit_factory::NSECTORS; // 32: number of TPOL sectors
// root hist pointers
static TH1I *hPSC_NHitPairs;
static TH1I *hPS_NHitPairs;
// PSC pairs
static TH2F *hPSC_PSCIDLeftVsIDRight;
static TH2I *hPSC_tdiffVsPSCIDLeft[NC_PSC];
static TH2I *hPSC_tdiffVsPSCIDRight[NC_PSC];
// PSC,PS pairs
static TH1I *pspair_num_events;
static TH2F *hPS_PSCIDLeftVsIDRight;
static TH2F *hPS_PSIDLeftVsIDRight;
static TH2F *hPS_ElVsEr;
static TH1F *hPS_E;
static TH1F *hPS_Ediff;
static TH2F *hPS_tdiffVsE;
static TH2F *hPS_PStdiffVsE;
static TH2F *hPS_PSCPStdiffVsE;
static TH2I *hPS_PSIDLeftVsPSCIDLeft;
static TH2I *hPS_PSIDRightVsPSCIDRight;
static TH2I *hPS_ElVsPSCIDLeft;
static TH2I *hPS_ErVsPSCIDRight;
// TAGX occupancies, all hits for events with PSC,PS pairs
static TH1F *hPS_TAGHCounterID;
static TH1F *hPS_Etagh;
static TH2F *hPS_timeVsEtagh;
static TH1F *hPS_TAGMColumn;
static TH1F *hPS_Etagm;
static TH2F *hPS_timeVsEtagm;
// PSC,PS,TAGH coincidences
static TH2F *hPSTAGH_tdiffVsEdiff;
static TH2I *hPSTAGH_EVsEtagh;
static TH2F *hPSTAGH_PSCIDLeftVsIDRight;
static TH2F *hPSTAGH_PSIDLeftVsIDRight;
static TH2F *hPSTAGH_ElVsEr;
static TH1F *hPSTAGH_E;
static TH2F *hPSTAGH_timeVsE;
static TH1F *hPSTAGH_TAGHCounterID;
static TH1F *hPSTAGH_Etagh;
static TH2F *hPSTAGH_timeVsEtagh;
static TH2I *hPSTAGH_EdiffVsTAGHCounterID;
static TH2I *hPSTAGH_EdiffVsEtagh;
static TH2I *hPSTAGH_tdiffVsTAGHCounterID_L[NC_PSC];
static TH2I *hPSTAGH_tdiffVsTAGHCounterID_R[NC_PSC];
// PSC,PS,TAGM coincidences
static TH2F *hPSTAGM_tdiffVsEdiff;
static TH2I *hPSTAGM_EVsEtagm;
static TH2F *hPSTAGM_PSCIDLeftVsIDRight;
static TH2F *hPSTAGM_PSIDLeftVsIDRight;
static TH2F *hPSTAGM_ElVsEr;
static TH1F *hPSTAGM_E;
static TH2F *hPSTAGM_timeVsE;
static TH1F *hPSTAGM_TAGMColumn;
static TH1F *hPSTAGM_Etagm;
static TH2F *hPSTAGM_timeVsEtagm;
static TH2I *hPSTAGM_EdiffVsTAGMColumn;
static TH2I *hPSTAGM_EdiffVsEtagm;
static TH2I *hPSTAGM_tdiffVsTAGMColumn_L[NC_PSC];
static TH2I *hPSTAGM_tdiffVsTAGMColumn_R[NC_PSC];
// PSC,PS,TPOL coincidences
static TH1I *hPSTPOL_NHits;
static TH1I *hPSTPOL_sector;
static TH1F *hPSTPOL_phi;
static TH1I *hPSTPOL_peak;
static TH2I *hPSTPOL_peakVsSector;
static TH1I *hPSTPOL_time;
static TH2I *hPSTPOL_timeVsSector;
static TH2F *hPSTPOL_timeVsPhi;
static TH2I *hPSTPOL_timeVsPeak;
//-------------------------
// Routine used to create our JEventProcessor
extern "C"{
    void InitPlugin(JApplication *app){
        InitJANAPlugin(app);
        app->Add(new JEventProcessor_PSPair_online());

    }
} // "C"


//------------------
// JEventProcessor_PSPair_online (Constructor)
//------------------
JEventProcessor_PSPair_online::JEventProcessor_PSPair_online()
{
	SetTypeName("JEventProcessor_PSPair_online");
}

//------------------
// ~JEventProcessor_PSPair_online (Destructor)
//------------------
JEventProcessor_PSPair_online::~JEventProcessor_PSPair_online()
{

}

//------------------
// Init
//------------------
void JEventProcessor_PSPair_online::Init()
{
    auto app = GetApplication();
    lockService = app->GetService<JLockService>();

    // energy binning
    const double Ebw_PS = 0.1;
    const double Ebl_PS = 5.0; const double Ebl_PSarm = 2.0;
    const double Ebh_PS = 13.0; const double Ebh_PSarm = 7.0;
    const int NEb_PS = int((Ebh_PS-Ebl_PS)/Ebw_PS); const int NEb_PSarm = int((Ebh_PSarm-Ebl_PSarm)/Ebw_PS);
    const double Ebw_TAG = 0.1;
    const double Ebl_TAG = 2.5;
    const double Ebh_TAG = 10.5;
    const int NEb_TAG = int((Ebh_TAG-Ebl_TAG)/Ebw_TAG);
    // time binning
    const double Tbw = 0.06;
    const double Tbl_PS = -4.5;
    const double Tbh_PS = 4.5;
    const int NTb_PS = int((Tbh_PS-Tbl_PS)/Tbw);
    const double Tbl_TAG = -200.0;
    const double Tbh_TAG = 200.0;
    const int NTb_TAG = 1000;
    // energy difference binning
    const int NEdb = 200; const double Edbl = -1.0; const double Edbh = 1.0;

    // create root folder for pspair and cd to it, store main dir
    TDirectory *mainDir = gDirectory;
    TDirectory *psPairDir = gDirectory->mkdir("PSPair");
    psPairDir->cd();
    // book hists
    pspair_num_events = new TH1I("pspair_num_events","PS pair number of events",1,0.5,1.5);
    hPSC_NHitPairs = new TH1I("PSC_NHitPairs","PSC pair multiplicity",8,0.5,8.5);
    hPS_NHitPairs = new TH1I("PS_NHitPairs","PS pair multiplicity",8,0.5,8.5);
    //
    gDirectory->mkdir("PSC")->cd();
    hPSC_PSCIDLeftVsIDRight = new TH2F("PSC_PSCIDLeftVsIDRight","PSC pair: Coarse PS ID left arm vs. ID right arm;module(right arm);module(left arm)",NC_PSC,0.5,0.5+NC_PSC,NC_PSC,0.5,0.5+NC_PSC);
    gDirectory->mkdir("PSCRightArmTimeOffsets")->cd();
    for (int i=0;i<NC_PSC;i++) {
        stringstream ss; ss << i+1;
        TString id = ss.str();
        TString strN = "_L" + id;
        hPSC_tdiffVsPSCIDRight[i] = new TH2I("PSC_tdiffVsPSCIDRight"+strN,"PSC pair: TDC time difference vs. right arm module, fixed left arm module "+id+";module(right arm);time(module "+id+", left arm) - time(right arm) [ns]",NC_PSC,0.5,0.5+NC_PSC,NTb_PS,Tbl_PS,Tbh_PS);
    }
    gDirectory->cd("../");
    gDirectory->mkdir("PSCLeftArmTimeOffsets")->cd();
    for (int i=0;i<NC_PSC;i++) {
        stringstream ss; ss << i+1;
        TString id = ss.str();
        TString strN = "_R" + id;
        hPSC_tdiffVsPSCIDLeft[i] = new TH2I("PSC_tdiffVsPSCIDLeft"+strN,"PSC pair: TDC time difference vs. left arm module, fixed right arm module "+id+";module(left arm);time(left arm) - time(module "+id+", right arm) [ns]",NC_PSC,0.5,0.5+NC_PSC,NTb_PS,Tbl_PS,Tbh_PS);
    }
    psPairDir->cd();
    //
    gDirectory->mkdir("PSC_PS")->cd();
    hPS_PSCIDLeftVsIDRight = new TH2F("PS_PSCIDLeftVsIDRight","PS pair: Coarse PS ID left arm vs. ID right arm;module(right arm);module(left arm)",NC_PSC,0.5,0.5+NC_PSC,NC_PSC,0.5,0.5+NC_PSC);
    hPS_PSIDLeftVsIDRight = new TH2F("PS_PSIDLeftVsIDRight","PS pair: Fine PS ID left arm vs. ID right arm;column(right arm);column(left arm)",NC_PS,0.5,0.5+NC_PS,NC_PS,0.5,0.5+NC_PS);
    hPS_ElVsEr = new TH2F("PS_ElVsEr","PS pair: left-arm energy vs. right-arm energy;energy(right arm) [GeV];energy(left arm) [GeV]",NEb_PSarm,Ebl_PSarm,Ebh_PSarm,NEb_PSarm,Ebl_PSarm,Ebh_PSarm);
    hPS_E = new TH1F("PS_E","PS pair energy;PS pair energy [GeV];events",NEb_PS,Ebl_PS,Ebh_PS);
    hPS_Ediff = new TH1F("PS_Ediff","PS pair energy difference;PS pair energy difference [GeV];events",80,-4.0,4.0);
    hPS_tdiffVsE = new TH2F("PS_tdiffVsE","PSC pair time difference vs. PS pair energy;PS pair energy [GeV];PSC pair time difference [ns]",NEb_PS,Ebl_PS,Ebh_PS,200,-10.0,10.0);
    hPS_PStdiffVsE = new TH2F("PS_PStdiffVsE","PS pair time difference vs. PS pair energy;PS pair energy [GeV];PS pair time difference [ns]",NEb_PS,Ebl_PS,Ebh_PS,200,-10.0,10.0);
    hPS_PSCPStdiffVsE = new TH2F("PS_PSCPStdiffVsE","PSC/PS pair time difference of differences vs. PS pair energy;PS pair energy [GeV];PSC/PS pair time difference of differences [ns]",NEb_PS,Ebl_PS,Ebh_PS,200,-10.0,10.0);
    //
    hPS_PSIDLeftVsPSCIDLeft = new TH2I("PS_PSIDLeftVsPSCIDLeft","PS pair: Fine PS ID left arm vs. Coarse PS ID left arm;coarse PS module(left arm);fine PS column(left arm)",NC_PSC,0.5,0.5+NC_PSC,NC_PS,0.5,0.5+NC_PS);
    hPS_PSIDRightVsPSCIDRight = new TH2I("PS_PSIDRightVsPSCIDRight","PS pair: Fine PS ID right arm vs. Coarse PS ID right arm;coarse PS module(right arm);fine PS column(right arm)",NC_PSC,0.5,0.5+NC_PSC,NC_PS,0.5,0.5+NC_PS);
    hPS_ElVsPSCIDLeft = new TH2I("PS_ElVsPSCIDLeft","PS pair: PS left-arm energy vs. Coarse PS ID left arm;coarse PS module(left arm);PS energy(left arm)",NC_PSC,0.5,0.5+NC_PSC,NEb_PSarm,Ebl_PSarm,Ebh_PSarm);
    hPS_ErVsPSCIDRight = new TH2I("PS_ErVsPSCIDRight","PS pair: PS right-arm energy vs. Coarse PS ID right arm;coarse PS module(right arm);PS energy(right arm)",NC_PSC,0.5,0.5+NC_PSC,NEb_PSarm,Ebl_PSarm,Ebh_PSarm);
    //
    gDirectory->mkdir("TAGX_AllHits")->cd();
    hPS_TAGHCounterID = new TH1F("PS_TAGHCounterID","PS pair: TAGH counter ID, all hits;TAGH counter ID;hits",NC_TAGH,0.5,0.5+NC_TAGH);
    hPS_Etagh = new TH1F("PS_Etagh","PS pair: TAGH photon energy, all hits;TAGH photon energy [GeV];hits",NEb_TAG,Ebl_TAG,Ebh_TAG);
    hPS_timeVsEtagh = new TH2F("PS_timeVsEtagh","PS pair: TAGH time vs. photon energy, all hits;TAGH photon energy [GeV];time [ns]",NEb_TAG,Ebl_TAG,Ebh_TAG,NTb_TAG,Tbl_TAG,Tbh_TAG);
    hPS_TAGMColumn = new TH1F("PS_TAGMColumn","PS pair: TAGM column, all hits;TAGM column;hits",NC_TAGM,0.5,0.5+NC_TAGM);
    hPS_Etagm = new TH1F("PS_Etagm","PS pair: TAGM photon energy, all hits;TAGM photon energy [GeV];hits",NEb_PS,Ebl_PS,Ebh_PS);
    hPS_timeVsEtagm = new TH2F("PS_timeVsEtagm","PS pair: TAGM time vs. photon energy, all hits;TAGM photon energy [GeV];time [ns]",NEb_PS,Ebl_PS,Ebh_PS,NTb_TAG,Tbl_TAG,Tbh_TAG);
    psPairDir->cd();
    //
    gDirectory->mkdir("PSC_PS_TAGH")->cd();
    hPSTAGH_tdiffVsEdiff = new TH2F("PSTAGH_tdiffVsEdiff","PS pair - TAGH: PS-TAGH time difference vs. PS-TAGH energy difference;E(PS) - E(TAGH) [GeV];PSC/TAGH time difference [ns]",NEdb,Edbl,Edbh,NTb_TAG,Tbl_TAG,Tbh_TAG);
    hPSTAGH_TAGHCounterID = new TH1F("PSTAGH_TAGHCounterID","PS pair - TAGH: TAGH counter ID;TAGH counter ID;events",NC_TAGH,0.5,0.5+NC_TAGH);
    hPSTAGH_Etagh = new TH1F("PSTAGH_Etagh","PS pair - TAGH: TAGH photon energy;TAGH photon energy [GeV];events",NEb_TAG,Ebl_TAG,Ebh_TAG);
    hPSTAGH_timeVsEtagh = new TH2F("PSTAGH_timeVsEtagh","PS pair - TAGH: TAGH time vs. photon energy;TAGH photon energy [GeV];time [ns]",NEb_TAG,Ebl_TAG,Ebh_TAG,NTb_TAG,Tbl_TAG,Tbh_TAG);
    hPSTAGH_EVsEtagh = new TH2I("PSTAGH_EVsEtagh","PS pair - TAGH: PS energy vs. TAGH energy;TAGH energy [GeV];PS energy [GeV]",NEb_PS,Ebl_PS,Ebh_PS,NEb_PS,Ebl_PS,Ebh_PS);
    hPSTAGH_EdiffVsEtagh = new TH2I("PSTAGH_EdiffVsEtagh","PS pair - TAGH: PS-TAGH energy difference vs. TAGH energy;TAGH energy [GeV];E(PS) - E(TAGH) [GeV]",NEb_TAG,Ebl_TAG,Ebh_TAG,NEdb,Edbl,Edbh);
    hPSTAGH_EdiffVsTAGHCounterID = new TH2I("PSTAGH_EdiffVsTAGHCounterID","PS pair - TAGH: PS-TAGH energy difference vs. TAGH counter ID;TAGH counter ID;E(PS) - E(TAGH) [GeV]",NC_TAGH,0.5,0.5+NC_TAGH,NEdb,Edbl,Edbh);
    hPSTAGH_PSCIDLeftVsIDRight = new TH2F("PSTAGH_PSCIDLeftVsIDRight","PS pair - TAGH: Coarse PS ID left arm vs. ID right arm;module(right arm);module(left arm)",NC_PSC,0.5,0.5+NC_PSC,NC_PSC,0.5,0.5+NC_PSC);
    hPSTAGH_PSIDLeftVsIDRight = new TH2F("PSTAGH_PSIDLeftVsIDRight","PS pair - TAGH: Fine PS ID left arm vs. ID right arm;column(right arm);column(left arm)",NC_PS,0.5,0.5+NC_PS,NC_PS,0.5,0.5+NC_PS);
    hPSTAGH_ElVsEr = new TH2F("PSTAGH_ElVsEr","PS pair - TAGH: left-arm energy vs. right-arm energy;energy(right arm) [GeV];energy(left arm) [GeV]",NEb_PSarm,Ebl_PSarm,Ebh_PSarm,NEb_PSarm,Ebl_PSarm,Ebh_PSarm);
    hPSTAGH_E = new TH1F("PSTAGH_E","PS pair - TAGH: PS pair energy;PS pair energy [GeV];events",NEb_PS,Ebl_PS,Ebh_PS);
    hPSTAGH_timeVsE = new TH2F("PSTAGH_timeVsE","PSC/TAGH time difference vs. PS pair energy;PS pair energy [GeV];PSC/TAGH time difference [ns]",NEb_PS,Ebl_PS,Ebh_PS,NTb_TAG,Tbl_TAG,Tbh_TAG);
    gDirectory->mkdir("PSTAGHTimeOffsets_L")->cd();
    for (int i=0; i<NC_PSC; i++) {
        stringstream ss; ss << i+1;
        TString id = ss.str();
        hPSTAGH_tdiffVsTAGHCounterID_L[i] = new TH2I("PSTAGH_tdiffVsTAGHCounterID_L"+id,"PS-TAGH TDC time difference vs. TAGH counter ID, fixed left arm PSC module "+id+";TAGH counter ID;time(PSC module "+id+", left arm) - time(TAGH) [ns]",NC_TAGH,0.5,0.5+NC_TAGH,NTb_TAG,Tbl_TAG,Tbh_TAG);
    }
    gDirectory->cd("../");
    gDirectory->mkdir("PSTAGHTimeOffsets_R")->cd();
    for (int i=0; i<NC_PSC; i++) {
        stringstream ss; ss << i+1;
        TString id = ss.str();
        hPSTAGH_tdiffVsTAGHCounterID_R[i] = new TH2I("PSTAGH_tdiffVsTAGHCounterID_R"+id,"PS-TAGH TDC time difference vs. TAGH counter ID, fixed right arm PSC module "+id+";TAGH counter ID;time(PSC module "+id+", right arm) - time(TAGH) [ns]",NC_TAGH,0.5,0.5+NC_TAGH,NTb_TAG,Tbl_TAG,Tbh_TAG);
    }
    psPairDir->cd();
    //
    gDirectory->mkdir("PSC_PS_TAGM")->cd();
    hPSTAGM_tdiffVsEdiff = new TH2F("PSTAGM_tdiffVsEdiff","PS pair - TAGM: PS-TAGM time difference vs. PS-TAGM energy difference;E(PS) - E(TAGM) [GeV];PSC/TAGM time difference [ns]",NEdb,Edbl,Edbh,NTb_TAG,Tbl_TAG,Tbh_TAG);
    hPSTAGM_TAGMColumn = new TH1F("PSTAGM_TAGMColumn","PS pair - TAGM: TAGM column;TAGM column;events",NC_TAGM,0.5,0.5+NC_TAGM);
    hPSTAGM_Etagm = new TH1F("PSTAGM_Etagm","PS pair - TAGM: TAGM photon energy;TAGM photon energy [GeV];events",NEb_PS,Ebl_PS,Ebh_PS);
    hPSTAGM_timeVsEtagm = new TH2F("PSTAGM_timeVsEtagm","PS pair - TAGM: TAGM time vs. photon energy;TAGM photon energy [GeV];time [ns]",NEb_PS,Ebl_PS,Ebh_PS,NTb_TAG,Tbl_TAG,Tbh_TAG);
    hPSTAGM_EVsEtagm = new TH2I("PSTAGM_EVsEtagm","PS pair - TAGM: PS energy vs. TAGM energy;TAGM energy [GeV];PS energy [GeV]",NEb_PS,Ebl_PS,Ebh_PS,NEb_PS,Ebl_PS,Ebh_PS);
    hPSTAGM_EdiffVsEtagm = new TH2I("PSTAGM_EdiffVsEtagm","PS pair - TAGM: PS-TAGM energy difference vs. TAGM energy;TAGM energy [GeV];E(PS) - E(TAGM) [GeV]",NEb_PS,Ebl_PS,Ebh_PS,NEdb,Edbl,Edbh);
    hPSTAGM_EdiffVsTAGMColumn = new TH2I("PSTAGM_EdiffVsTAGMColumn","PS pair - TAGM: PS-TAGM energy difference vs. TAGM column;TAGM column;E(PS) - E(TAGM) [GeV]",NC_TAGM,0.5,0.5+NC_TAGM,NEdb,Edbl,Edbh);
    hPSTAGM_PSCIDLeftVsIDRight = new TH2F("PSTAGM_PSCIDLeftVsIDRight","PS pair - TAGM: Coarse PS ID left arm vs. ID right arm;module(right arm);module(left arm)",NC_PSC,0.5,0.5+NC_PSC,NC_PSC,0.5,0.5+NC_PSC);
    hPSTAGM_PSIDLeftVsIDRight = new TH2F("PSTAGM_PSIDLeftVsIDRight","PS pair - TAGM: Fine PS ID left arm vs. ID right arm;column(right arm);column(left arm)",NC_PS,0.5,0.5+NC_PS,NC_PS,0.5,0.5+NC_PS);
    hPSTAGM_ElVsEr = new TH2F("PSTAGM_ElVsEr","PS pair - TAGM: left-arm energy vs. right-arm energy;energy(right arm) [GeV];energy(left arm) [GeV]",NEb_PSarm,Ebl_PSarm,Ebh_PSarm,NEb_PSarm,Ebl_PSarm,Ebh_PSarm);
    hPSTAGM_E = new TH1F("PSTAGM_E","PS pair - TAGM: PS pair energy;PS pair energy [GeV];events",NEb_PS,Ebl_PS,Ebh_PS);
    hPSTAGM_timeVsE = new TH2F("PSTAGM_timeVsE","PSC/TAGM time difference vs. PS pair energy;PS pair energy [GeV];PSC/TAGM time difference [ns]",NEb_PS,Ebl_PS,Ebh_PS,NTb_TAG,Tbl_TAG,Tbh_TAG);
    gDirectory->mkdir("PSTAGMTimeOffsets_L")->cd();
    for (int i=0; i<NC_PSC; i++) {
        stringstream ss; ss << i+1;
        TString id = ss.str();
        hPSTAGM_tdiffVsTAGMColumn_L[i] = new TH2I("PSTAGM_tdiffVsTAGMColumn_L"+id,"PS-TAGM TDC time difference vs. TAGM column, fixed left arm PSC module "+id+";TAGM column;time(PSC module "+id+", left arm) - time(TAGM) [ns]",NC_TAGM,0.5,0.5+NC_TAGM,NTb_TAG,Tbl_TAG,Tbh_TAG);
    }
    gDirectory->cd("../");
    gDirectory->mkdir("PSTAGMTimeOffsets_R")->cd();
    for (int i=0; i<NC_PSC; i++) {
        stringstream ss; ss << i+1;
        TString id = ss.str();
        hPSTAGM_tdiffVsTAGMColumn_R[i] = new TH2I("PSTAGM_tdiffVsTAGMColumn_R"+id,"PS-TAGM TDC time difference vs. TAGM column, fixed right arm PSC module "+id+";TAGM column;time(PSC module "+id+", right arm) - time(TAGM) [ns]",NC_TAGM,0.5,0.5+NC_TAGM,NTb_TAG,Tbl_TAG,Tbh_TAG);
    }
    psPairDir->cd();
    //
    gDirectory->mkdir("PSC_PS_TPOL")->cd();
    hPSTPOL_NHits = new TH1I("PSTPOL_NHits","TPOL hit multiplicity;hits;events",5,0.5,5.5);
    hPSTPOL_sector = new TH1I("PSTPOL_sector","TPOL sector;sector;hits / sector",NS_TPOL,0.5,0.5+NS_TPOL);
    hPSTPOL_phi = new TH1F("PSTPOL_phi","TPOL azimuthal angle;triplet azimuthal angle [degrees];hits / sector",NS_TPOL,0.0,360.0);
    hPSTPOL_peak = new TH1I("PSTPOL_peak","TPOL fADC pulse peak;pulse peak;hits",410,0.0,4100.0);
    hPSTPOL_peakVsSector = new TH2I("PSTPOL_peakVsSector","TPOL fADC pulse peak vs. sector;sector;pulse peak",NS_TPOL,0.5,0.5+NS_TPOL,410,0.0,4100.0);
    hPSTPOL_time = new TH1I("PSTPOL_time","TPOL time;time [ns];hits / 2 ns",400,-400.0,400.0);
    hPSTPOL_timeVsSector = new TH2I("PSTPOL_timeVsSector","TPOL time vs. sector;sector;time [ns]",NS_TPOL,0.5,0.5+NS_TPOL,400,-400.0,400.0);
    hPSTPOL_timeVsPhi = new TH2F("PSTPOL_timeVsPhi","TPOL time vs. phi;#phi [degrees];time [ns]",NS_TPOL,0.0,360.0,400,-400.0,400.0);
    hPSTPOL_timeVsPeak = new TH2I("PSTPOL_timeVsPeak","TPOL time vs. peak;pulse peak;time [ns]",410,0.0,4100.0,400,-400.0,400.0);
    // back to main dir
    mainDir->cd();

}

//------------------
// BeginRun
//------------------
void JEventProcessor_PSPair_online::BeginRun(const std::shared_ptr<const JEvent>& event)
{
    // This is called whenever the run number changes
    // extract the PS geometry
    vector<const DPSGeometry*> psGeomVect;
    event->Get(psGeomVect);
    if (psGeomVect.size() == 0) throw JException("Missing DPSGeometry object");
    const DPSGeometry& psGeom = *(psGeomVect[0]);
    // get photon energy bin lows for variable-width energy binning
    double Elows_PSarm[Narms][NC_PS+1];
    double wl_min=0.05,wr_min = 0.05;
    for (int i=0;i<NC_PS;i++) {
        Elows_PSarm[0][i] = psGeom.getElow(0,i+1);
        Elows_PSarm[1][i] = psGeom.getElow(1,i+1);
        // find smallest bin widths to use for PS pair energy binning
        double wl = fabs(psGeom.getEhigh(0,i+1) - psGeom.getElow(0,i+1));
        double wr = fabs(psGeom.getEhigh(1,i+1) - psGeom.getElow(1,i+1));
        if (wl<wl_min) wl_min = wl;
        if (wr<wr_min) wr_min = wr;
    }
    // add the upper limits
    Elows_PSarm[0][NC_PS] = psGeom.getEhigh(0,NC_PS);
    Elows_PSarm[1][NC_PS] = psGeom.getEhigh(1,NC_PS);
    // PS pair energy binning
    double Ebw_PS = wl_min + wr_min;
    const double Ebl_PS = psGeom.getElow(0,1) + psGeom.getElow(1,1);
    const double Ebh_PS = psGeom.getEhigh(0,NC_PS) + psGeom.getEhigh(1,NC_PS);
    double range = fabs(Ebh_PS-Ebl_PS);
    int NEb_PS = range/Ebw_PS-int(range/Ebw_PS) < 0.5 ? int(range/Ebw_PS) : int(range/Ebw_PS) + 1;
    double Elows_PS[NEb_PS+1];
    for (int i=0;i<NEb_PS+1;i++) {
        Elows_PS[i] = Ebl_PS + i*Ebw_PS;
    }
    // extract the TAGH geometry
    vector<const DTAGHGeometry*> taghGeomVect;
    event->Get(taghGeomVect);
    if (taghGeomVect.size() == 0) throw JException("Missing DTAGHGeometry object");
    const DTAGHGeometry& taghGeom = *(taghGeomVect[0]);
    // get photon energy bin low of each counter for energy histogram binning
    double Elows_TAGH[NC_TAGH+1];
    for (int i=0;i<NC_TAGH;i++) {
        Elows_TAGH[i] = taghGeom.getElow(NC_TAGH-i);
    }
    // add the upper limit
    Elows_TAGH[NC_TAGH] = taghGeom.getEhigh(1);
    // extract the TAGM geometry
    vector<const DTAGMGeometry*> tagmGeomVect;
    event->Get(tagmGeomVect);
    if (tagmGeomVect.size() < 1) throw JException("Missing DTAGMGeometry");
    const DTAGMGeometry& tagmGeom = *(tagmGeomVect[0]);
    // get photon energy bin low of each counter for energy histogram binning
    double Elows_TAGM[NC_TAGM+1];
    for (int i=0;i<NC_TAGM;i++) {
        Elows_TAGM[i] = tagmGeom.getElow(NC_TAGM-i);
    }
    // add the upper limit
    Elows_TAGM[NC_TAGM] = tagmGeom.getEhigh(1);
    //
    const int NEdiff = 200;
    double EdiffLows[NEdiff+1];
    for (int i=0;i<NEdiff+1;i++) {
        EdiffLows[i] = -1.0 + i*0.01;
    }
    double modules[NC_PSC+1];
    for (int i=0;i<=NC_PSC;i++) {
        modules[i] = 0.5+i;
    }
    const int NTb = 2000;
    double Tlows[NTb+1];
    for (int i=0;i<=NTb;i++) {
        Tlows[i] = -400.0+i*0.4;
    }
    const int NTb_PS = 200;
    double Tlows_PS[NTb_PS+1];
    for (int i=0;i<=NTb_PS;i++) {
        Tlows_PS[i] = -10.0+i*0.1;
    }

    // Since we are filling histograms local to this plugin, it will not interfere with other ROOT operations: can use plugin-wide ROOT fill lock
    lockService->RootWriteLock(); //ACQUIRE ROOT FILL LOCK

    // set variable-width energy bins if histogram is empty
    // PS
    if (hPS_ElVsEr->GetEntries()==0) hPS_ElVsEr->SetBins(NC_PS,Elows_PSarm[1],NC_PS,Elows_PSarm[0]);
    if (hPS_ElVsPSCIDLeft->GetEntries()==0) hPS_ElVsPSCIDLeft->SetBins(NC_PSC,modules,NC_PS,Elows_PSarm[0]);
    if (hPS_ErVsPSCIDRight->GetEntries()==0) hPS_ErVsPSCIDRight->SetBins(NC_PSC,modules,NC_PS,Elows_PSarm[1]);
    if (hPSTAGH_ElVsEr->GetEntries()==0) hPSTAGH_ElVsEr->SetBins(NC_PS,Elows_PSarm[1],NC_PS,Elows_PSarm[0]);
    if (hPSTAGM_ElVsEr->GetEntries()==0) hPSTAGM_ElVsEr->SetBins(NC_PS,Elows_PSarm[1],NC_PS,Elows_PSarm[0]);
    if (hPS_E->GetEntries()==0) hPS_E->SetBins(NEb_PS,Elows_PS);
    if (hPSTAGH_E->GetEntries()==0) hPSTAGH_E->SetBins(NEb_PS,Elows_PS);
    if (hPSTAGM_E->GetEntries()==0) hPSTAGM_E->SetBins(NEb_PS,Elows_PS);
    if (hPS_tdiffVsE->GetEntries()==0) hPS_tdiffVsE->SetBins(NEb_PS,Elows_PS,NTb_PS,Tlows_PS);
    if (hPS_PStdiffVsE->GetEntries()==0) hPS_PStdiffVsE->SetBins(NEb_PS,Elows_PS,NTb_PS,Tlows_PS);
    if (hPS_PSCPStdiffVsE->GetEntries()==0) hPS_PSCPStdiffVsE->SetBins(NEb_PS,Elows_PS,NTb_PS,Tlows_PS);
    if (hPSTAGH_timeVsE->GetEntries()==0) hPSTAGH_timeVsE->SetBins(NEb_PS,Elows_PS,NTb,Tlows);
    if (hPSTAGM_timeVsE->GetEntries()==0) hPSTAGM_timeVsE->SetBins(NEb_PS,Elows_PS,NTb,Tlows);
    // TAGH
    if (hPS_Etagh->GetEntries()==0) hPS_Etagh->SetBins(NC_TAGH,Elows_TAGH);
    if (hPS_timeVsEtagh->GetEntries()==0) hPS_timeVsEtagh->SetBins(NC_TAGH,Elows_TAGH,NTb,Tlows);
    if (hPSTAGH_Etagh->GetEntries()==0) hPSTAGH_Etagh->SetBins(NC_TAGH,Elows_TAGH);
    if (hPSTAGH_EVsEtagh->GetEntries()==0) hPSTAGH_EVsEtagh->SetBins(NC_TAGH,Elows_TAGH,NEb_PS,Elows_PS);
    if (hPSTAGH_timeVsEtagh->GetEntries()==0) hPSTAGH_timeVsEtagh->SetBins(NC_TAGH,Elows_TAGH,NTb,Tlows);
    if (hPSTAGH_EdiffVsEtagh->GetEntries()==0) hPSTAGH_EdiffVsEtagh->SetBins(NC_TAGH,Elows_TAGH,NEdiff,EdiffLows);
    // TAGM
    if (hPS_Etagm->GetEntries()==0) hPS_Etagm->SetBins(NC_TAGM,Elows_TAGM);
    if (hPS_timeVsEtagm->GetEntries()==0) hPS_timeVsEtagm->SetBins(NC_TAGM,Elows_TAGM,NTb,Tlows);
    if (hPSTAGM_Etagm->GetEntries()==0) hPSTAGM_Etagm->SetBins(NC_TAGM,Elows_TAGM);
    if (hPSTAGM_EVsEtagm->GetEntries()==0) hPSTAGM_EVsEtagm->SetBins(NC_TAGM,Elows_TAGM,NEb_PS,Elows_PS);
    if (hPSTAGM_timeVsEtagm->GetEntries()==0) hPSTAGM_timeVsEtagm->SetBins(NC_TAGM,Elows_TAGM,NTb,Tlows);
    if (hPSTAGM_EdiffVsEtagm->GetEntries()==0) hPSTAGM_EdiffVsEtagm->SetBins(NC_TAGM,Elows_TAGM,NEdiff,EdiffLows);

    lockService->RootUnLock(); //RELEASE ROOT FILL LOCK

}

//------------------
// Process
//------------------
void JEventProcessor_PSPair_online::Process(const std::shared_ptr<const JEvent>& event)
{
    // This is called for every event. Use of common resources like writing
    // to a file or filling a histogram should be mutex protected. Using
    // event->Get(...) to get reconstructed objects (and thereby activating the
    // reconstruction algorithm) should be done outside of any mutex lock
    // since multiple threads may call this method at the same time.
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
    // TPOL hits
    vector<const DTPOLHit*> tpolhits;
    event->Get(tpolhits);

    // FILL HISTOGRAMS
    // Since we are filling histograms local to this plugin, it will not interfere with other ROOT operations: can use plugin-wide ROOT fill lock
    lockService->RootWriteLock(); //ACQUIRE ROOT FILL LOCK

    hPSC_NHitPairs->Fill(cpairs.size());
    hPS_NHitPairs->Fill(fpairs.size());
    // PSC coincidences
    if (cpairs.size()>=1) {
        // take pair with smallest time difference from sorted vector
        const DPSCHit* clhit = cpairs[0]->ee.first; // left hit in coarse PS
        const DPSCHit* crhit = cpairs[0]->ee.second;// right hit in coarse PS
        hPSC_PSCIDLeftVsIDRight->Fill(crhit->module,clhit->module);
        double PSC_tdiff = clhit->t-crhit->t;
        hPSC_tdiffVsPSCIDRight[clhit->module-1]->Fill(crhit->module,PSC_tdiff);
        hPSC_tdiffVsPSCIDLeft[crhit->module-1]->Fill(clhit->module,PSC_tdiff);
        // PSC,PS coincidences
        if (fpairs.size()>=1) {
            pspair_num_events->Fill(1);
            // take pair with smallest time difference from sorted vector
            const DPSPair::PSClust* flhit = fpairs[0]->ee.first;  // left hit in fine PS
            const DPSPair::PSClust* frhit = fpairs[0]->ee.second; // right hit in fine PS
            double E_pair = flhit->E+frhit->E;
            hPS_PSCIDLeftVsIDRight->Fill(crhit->module,clhit->module);
            hPS_PSIDLeftVsIDRight->Fill(frhit->column,flhit->column);
            double PS_Ediff = flhit->E-frhit->E;
            double PS_tdiff = flhit->t-frhit->t;
            hPS_Ediff->Fill(PS_Ediff);
            hPS_ElVsEr->Fill(frhit->E,flhit->E);
            hPS_E->Fill(E_pair);
            hPS_tdiffVsE->Fill(E_pair,PSC_tdiff);
            hPS_PStdiffVsE->Fill(E_pair,PS_tdiff);
            hPS_PSCPStdiffVsE->Fill(E_pair,PSC_tdiff-PS_tdiff);
            // correlation between PSC and PS ids for each arm
            hPS_PSIDLeftVsPSCIDLeft->Fill(clhit->module,flhit->column);
            hPS_PSIDRightVsPSCIDRight->Fill(crhit->module,frhit->column);
            hPS_ElVsPSCIDLeft->Fill(clhit->module,flhit->E);
            hPS_ErVsPSCIDRight->Fill(crhit->module,frhit->E);
            // PSC,PS,TAGH coincidences
            double EdiffMax = 0.3; // max difference of tagger hit and pair energies
            for (unsigned int i=0; i < taghhits.size(); i++) {
                const DTAGHHit* tag = taghhits[i];
                if (!tag->has_TDC||!tag->has_fADC) continue;
                hPS_TAGHCounterID->Fill(tag->counter_id);
                hPS_Etagh->Fill(tag->E);
                hPS_timeVsEtagh->Fill(tag->E,tag->t);
                double Ediff = E_pair-tag->E;
                hPSTAGH_tdiffVsEdiff->Fill(Ediff,clhit->t-tag->t);
                if (fabs(Ediff) > EdiffMax) continue; // loose cut on energy difference
                hPSTAGH_TAGHCounterID->Fill(tag->counter_id);
                hPSTAGH_Etagh->Fill(tag->E);
                hPSTAGH_timeVsEtagh->Fill(tag->E,tag->t);
                hPSTAGH_EVsEtagh->Fill(tag->E,E_pair);
                hPSTAGH_EdiffVsEtagh->Fill(tag->E,Ediff);
                hPSTAGH_EdiffVsTAGHCounterID->Fill(tag->counter_id,Ediff);
                hPSTAGH_tdiffVsTAGHCounterID_L[clhit->module-1]->Fill(tag->counter_id,clhit->t-tag->t);
                hPSTAGH_tdiffVsTAGHCounterID_R[crhit->module-1]->Fill(tag->counter_id,crhit->t-tag->t);
                hPSTAGH_PSCIDLeftVsIDRight->Fill(crhit->module,clhit->module);
                hPSTAGH_PSIDLeftVsIDRight->Fill(frhit->column,flhit->column);
                hPSTAGH_ElVsEr->Fill(frhit->E,flhit->E);
                hPSTAGH_E->Fill(E_pair);
                hPSTAGH_timeVsE->Fill(E_pair,clhit->t-tag->t);
            }
            // PSC,PS,TAGM coincidences
            for (unsigned int i=0; i < tagmhits.size(); i++) {
                const DTAGMHit* tag = tagmhits[i];
                if (!tag->has_TDC||!tag->has_fADC) continue;
                if (tag->row!=0) continue;
                hPS_TAGMColumn->Fill(tag->column);
                hPS_Etagm->Fill(tag->E);
                hPS_timeVsEtagm->Fill(tag->E,tag->t);
                double Ediff = E_pair-tag->E;
                hPSTAGM_tdiffVsEdiff->Fill(Ediff,clhit->t-tag->t);
                if (fabs(Ediff) > EdiffMax) continue; // loose cut on energy difference
                hPSTAGM_TAGMColumn->Fill(tag->column);
                hPSTAGM_Etagm->Fill(tag->E);
                hPSTAGM_timeVsEtagm->Fill(tag->E,tag->t);
                hPSTAGM_EVsEtagm->Fill(tag->E,E_pair);
                hPSTAGM_EdiffVsEtagm->Fill(tag->E,Ediff);
                hPSTAGM_EdiffVsTAGMColumn->Fill(tag->column,Ediff);
                hPSTAGM_tdiffVsTAGMColumn_L[clhit->module-1]->Fill(tag->column,clhit->t-tag->t);
                hPSTAGM_tdiffVsTAGMColumn_R[crhit->module-1]->Fill(tag->column,crhit->t-tag->t);
                hPSTAGM_PSCIDLeftVsIDRight->Fill(crhit->module,clhit->module);
                hPSTAGM_PSIDLeftVsIDRight->Fill(frhit->column,flhit->column);
                hPSTAGM_ElVsEr->Fill(frhit->E,flhit->E);
                hPSTAGM_E->Fill(E_pair);
                hPSTAGM_timeVsE->Fill(E_pair,clhit->t-tag->t);
            }
            // PSC,PS,TPOL coincidences
            if (fabs(PS_Ediff) > 1.75) {lockService->RootUnLock(); return;}
            if (fabs(PSC_tdiff) > 1.3) {lockService->RootUnLock(); return;}
            if (E_pair < 8.4 || E_pair > 9.0) {lockService->RootUnLock(); return;}
            double ph_cut = 100.0;
            int N_TPOL = 0;
            for(unsigned int i=0; i<tpolhits.size(); i++) {
                if (tpolhits[i]->pulse_peak>=ph_cut) N_TPOL++;
            }
            hPSTPOL_NHits->Fill(N_TPOL);
            if (N_TPOL>1) {lockService->RootUnLock(); return;}
            for(unsigned int i=0; i<tpolhits.size(); i++) {
                const DTPOLHit* hit = tpolhits[i];
                hPSTPOL_peak->Fill(hit->pulse_peak);
                hPSTPOL_peakVsSector->Fill(hit->sector,hit->pulse_peak);
                hPSTPOL_timeVsPeak->Fill(hit->pulse_peak,hit->t);
                if (hit->pulse_peak<ph_cut) continue;
                hPSTPOL_sector->Fill(hit->sector);
                hPSTPOL_phi->Fill(hit->phi);
                hPSTPOL_time->Fill(hit->t);
                hPSTPOL_timeVsSector->Fill(hit->sector,hit->t);
                hPSTPOL_timeVsPhi->Fill(hit->phi,hit->t);
            }
        }
    }
    //
    lockService->RootUnLock(); //RELEASE ROOT FILL LOCK

}

//------------------
// EndRun
//------------------
void JEventProcessor_PSPair_online::EndRun()
{
    // This is called whenever the run number changes, before it is
    // changed to give you a chance to clean up before processing
    // events from the next run number.
}

//------------------
// Finish
//------------------
void JEventProcessor_PSPair_online::Finish()
{
    // Called before program exit after event processing is finished.
}

