#include "JEventProcessor_highlevel_online.h"

#include <DAQ/Df250PulseData.h>

#include <START_COUNTER/DSCHit.h>
#include <TAGGER/DTAGMHit.h>
#include <TAGGER/DTAGHHit.h>
#include <TOF/DTOFHit.h>
#include <BCAL/DBCALUnifiedHit.h>
#include <BCAL/DBCALHit.h>
#include <BCAL/DBCALPoint.h>
#include <RF/DRFTime.h>
#include <DAQ/DF1TDCHit.h>
#include <DAQ/DCODAEventInfo.h>
#include <DAQ/DEPICSvalue.h>
#include <DAQ/DBeamHelicity.h>
#include <DANA/DEvent.h>
#include <PAIR_SPECTROMETER/DPSCHit.h>
#include <FDC/DFDCHit.h>

// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
#include <JANA/JFactoryT.h>
extern "C"{
  void InitPlugin(JApplication *app){
    InitJANAPlugin(app);
    app->Add(new JEventProcessor_highlevel_online());
  }
} // "C"


// Hit types that include F1TDC information
#define F1Types(X) \
	X(DRFTime)  \
	X(DPSCHit)   \
	X(DSCHit)   \
	X(DFDCHit)  \
	X(DBCALUnifiedHit)

// We need to access the tagger hits differently in order to get at
// the "raw" (i.e. unmerged) hits. 
#define F1TaggerTypes(X) \
	X(DTAGHHit) \
	X(DTAGMHit) \

//.......................................................
// These templates are used by the FillF1Hist method below
// to accomodate the different member names
template<typename T> static bool   F1Check(const T* hit); // return true if all info (TDC and ADC) is present
template<typename T> static bool   F1CheckADCOnly(const T* hit); // return true if only ADC info is present
template<typename T> static bool   F1CheckTDCOnly(const T* hit); // return true if only TDC info is present
template<typename T> static double F1tdiff(const T* hit); // return time difference between TDC and ADC
template<typename T> static double F1tdiff2(const T* hit,const T* hit2); // return time difference between TDC and ADC times stored in two different hits
template<typename T> static bool   F1CheckSameChannel(const T* hit,const T* hit2); // return true if both hits are on the same channel

template<> bool   F1Check<DRFTime        >(const DRFTime*         hit){ return true;                          }
template<> bool   F1CheckADCOnly<DRFTime >(const DRFTime*         hit){ return false;                         }
template<> bool   F1CheckTDCOnly<DRFTime >(const DRFTime*         hit){ return false;                         }
template<> double F1tdiff<DRFTime        >(const DRFTime*         hit){ return hit->dTime;                    }
template<> double F1tdiff2<DRFTime       >(const DRFTime* hit,const DRFTime* hit2) { return hit->dTime;  }  // dummy functions
template<> bool   F1CheckSameChannel<DRFTime >(const DRFTime* hit,const DRFTime* hit2){ return true;    }


template<> bool   F1Check<DSCHit         >(const DSCHit*          hit){ return hit->has_fADC && hit->has_TDC;  }
template<> bool   F1CheckADCOnly<DSCHit  >(const DSCHit*          hit){ return hit->has_fADC && !hit->has_TDC; }
template<> bool   F1CheckTDCOnly<DSCHit  >(const DSCHit*          hit){ return !hit->has_fADC && hit->has_TDC; }
template<> double F1tdiff<DSCHit         >(const DSCHit*          hit){ return hit->t_TDC - hit->t_fADC;       }
template<> double F1tdiff2<DSCHit        >(const DSCHit* hit, const DSCHit* hit2){ return hit->t_TDC - hit2->t_fADC; }
template<> bool   F1CheckSameChannel<DSCHit >(const DSCHit* hit,const DSCHit* hit2){ return hit->sector==hit2->sector;    }

template<> bool   F1Check<DPSCHit        >(const DPSCHit*          hit){ return hit->has_fADC && hit->has_TDC;  }
template<> bool   F1CheckADCOnly<DPSCHit >(const DPSCHit*          hit){ return hit->has_fADC && !hit->has_TDC; }
template<> bool   F1CheckTDCOnly<DPSCHit >(const DPSCHit*          hit){ return !hit->has_fADC && hit->has_TDC; }
template<> double F1tdiff<DPSCHit        >(const DPSCHit*          hit){ return hit->time_tdc - hit->time_fadc;       }
template<> double F1tdiff2<DPSCHit       >(const DPSCHit* hit, const DPSCHit* hit2){ return hit->time_tdc - hit2->time_fadc; }
template<> bool   F1CheckSameChannel<DPSCHit >(const DPSCHit* hit,const DPSCHit* hit2){ return hit->module==hit2->module;    }

template<> bool   F1Check<DTAGHHit       >(const DTAGHHit*        hit){ return hit->has_fADC && hit->has_TDC; }
template<> bool   F1CheckADCOnly<DTAGHHit>(const DTAGHHit*        hit){	return hit->has_fADC && !(hit->has_TDC); }
template<> bool   F1CheckTDCOnly<DTAGHHit>(const DTAGHHit*        hit){	return !(hit->has_fADC) && hit->has_TDC; }
template<> double F1tdiff<DTAGHHit       >(const DTAGHHit*        hit){ return hit->time_tdc - hit->time_fadc;}
template<> double F1tdiff2<DTAGHHit      >(const DTAGHHit* hit, const DTAGHHit* hit2) { return hit->time_tdc - hit2->time_fadc; }
template<> bool   F1CheckSameChannel<DTAGHHit >(const DTAGHHit* hit,const DTAGHHit* hit2){ return hit->counter_id==hit2->counter_id;  }

template<> bool   F1Check<DTAGMHit       >(const DTAGMHit*        hit){ return hit->has_fADC && hit->has_TDC; }
template<> bool   F1CheckADCOnly<DTAGMHit>(const DTAGMHit*        hit){ return hit->has_fADC && !(hit->has_TDC); }
template<> bool   F1CheckTDCOnly<DTAGMHit>(const DTAGMHit*        hit){ return !(hit->has_fADC) && hit->has_TDC; }
template<> double F1tdiff<DTAGMHit       >(const DTAGMHit*        hit){ return hit->time_tdc - hit->time_fadc;}
template<> double F1tdiff2<DTAGMHit      >(const DTAGMHit* hit, const DTAGMHit* hit2){ return hit->time_tdc - hit2->time_fadc; }
template<> bool   F1CheckSameChannel<DTAGMHit >(const DTAGMHit* hit,const DTAGMHit* hit2)
                    { return (hit->row==hit2->row) && (hit->column==hit2->column);    }

template<> bool   F1Check<DFDCHit        >(const DFDCHit*         hit){ return true;                          }
template<> bool   F1CheckADCOnly<DFDCHit >(const DFDCHit*         hit){ return false;                         }
template<> bool   F1CheckTDCOnly<DFDCHit >(const DFDCHit*         hit){ return false;                         }
template<> double F1tdiff<DFDCHit        >(const DFDCHit*         hit){ return hit->t/50.0;                   }
template<> double F1tdiff2<DFDCHit       >(const DFDCHit* hit, const DFDCHit* hit2){ return hit->t/50.0;  }  // dummy
template<> bool   F1CheckSameChannel<DFDCHit >(const DFDCHit* hit, const DFDCHit* hit2)
                    { return (hit->gPlane==hit2->gPlane) && (hit->element==hit2->element);    }

template<> bool   F1Check<DBCALUnifiedHit>(const DBCALUnifiedHit* hit){ return hit->has_TDC_hit;              }
// right now we don't use a window to restrict ADC/TDC hit matching like we do in other detectors
// so if there are hits in the ADC and TDC for a given channel, they will always match
// and we won't have to fall into the other loop
template<> bool   F1CheckADCOnly<DBCALUnifiedHit >(const DBCALUnifiedHit*         hit){ return false;                         }
template<> bool   F1CheckTDCOnly<DBCALUnifiedHit >(const DBCALUnifiedHit*         hit){ return false;                         }
template<> double F1tdiff<DBCALUnifiedHit>(const DBCALUnifiedHit* hit){ return hit->t_TDC - hit->t_ADC;       }
template<> double F1tdiff2<DBCALUnifiedHit>(const DBCALUnifiedHit* hit, const DBCALUnifiedHit* hit2)
                                                     { return hit->t_TDC - hit2->t_ADC;       }
template<> bool   F1CheckSameChannel<DBCALUnifiedHit >(const DBCALUnifiedHit* hit,const DBCALUnifiedHit* hit2)
                    { return (hit->module==hit2->module) && (hit->layer==hit2->layer) 
				    && (hit->sector==hit2->sector) && (hit->end==hit2->end);    }
//.......................................................


//------------------
// FillF1Hist
//
// Template routine used to fill dF1TDC_fADC_tdiff for all types in F1Types.
//------------------
template<typename T>
void JEventProcessor_highlevel_online::FillF1Hist(vector<const T*> hits)
{
	for(auto hit : hits){
		// Check hits where the ADC and TDC hits have been matched
		if( F1Check(hit) ) {
			vector<const DF1TDCHit*> f1hits;
			hit->Get(f1hits);
			for(auto f1hit : f1hits){
				pair<int,int> rocid_slot(f1hit->rocid, f1hit->slot);
				double fbin = f1tdc_bin_map[rocid_slot];
				double tdiff = F1tdiff(hit);
				dF1TDC_fADC_tdiff->Fill(fbin+1, tdiff);
			}
		}

		// Check hits where the ADC and TDC hits were not matched
		// We have to manually loop over these hits, since the calibrated hits
		// only have matched ADC & TDC information within a certain window
		// which is detector-specific but generally smaller than the 32 ns shifts
		// (which are peculiar to the F1TDC) that we want to detect
		if( F1CheckADCOnly(hit) ) {
			for(auto hit2 : hits) {
				if( F1CheckTDCOnly(hit2) ) {
					if( F1CheckSameChannel(hit, hit2) ) {
						vector<const DF1TDCHit*> f1hits;
						hit2->Get(f1hits);
						for(auto f1hit : f1hits){
							pair<int,int> rocid_slot(f1hit->rocid, f1hit->slot);
							double fbin = f1tdc_bin_map[rocid_slot];
							double tdiff = F1tdiff2(hit2,hit);
							dF1TDC_fADC_tdiff->Fill(fbin+1, tdiff);
						}
					}
				}
			}
		}
	}
}

//------------------
// Init
//------------------
void JEventProcessor_highlevel_online::Init()
{
	auto app = GetApplication();
	lockService = app->GetService<JLockService>();

	//timing cuts
	dTimingCutMap[Gamma][SYS_BCAL] = 3.0;
	dTimingCutMap[Gamma][SYS_FCAL] = 5.0;
	dTimingCutMap[Gamma][SYS_ECAL] = 3.0;
	dTimingCutMap[Proton][SYS_NULL] = -1.0;
	dTimingCutMap[Proton][SYS_TOF] = 2.5;
	dTimingCutMap[Proton][SYS_BCAL] = 2.5;
	dTimingCutMap[Proton][SYS_FCAL] = 3.0;
	dTimingCutMap[Proton][SYS_ECAL] = 3.0;
	dTimingCutMap[PiPlus][SYS_NULL] = -1.0;
	dTimingCutMap[PiPlus][SYS_TOF] = 2.0;
	dTimingCutMap[PiPlus][SYS_BCAL] = 2.5;
	dTimingCutMap[PiPlus][SYS_FCAL] = 3.0;
	dTimingCutMap[PiPlus][SYS_ECAL] = 3.0;
	dTimingCutMap[PiMinus][SYS_NULL] = -1.0;
	dTimingCutMap[PiMinus][SYS_TOF] = 2.0;
	dTimingCutMap[PiMinus][SYS_BCAL] = 2.5;
	dTimingCutMap[PiMinus][SYS_FCAL] = 3.0;
	dTimingCutMap[PiMinus][SYS_ECAL] = 3.0;
	dTimingCutMap[Electron][SYS_NULL] = -1.0;
	dTimingCutMap[Electron][SYS_TOF] = 2.0;
	dTimingCutMap[Electron][SYS_BCAL] = 2.5;
	dTimingCutMap[Electron][SYS_FCAL] = 3.0;
	dTimingCutMap[Electron][SYS_ECAL] = 3.0;
	dTimingCutMap[Positron][SYS_NULL] = -1.0;
	dTimingCutMap[Positron][SYS_TOF] = 2.0;
	dTimingCutMap[Positron][SYS_BCAL] = 2.5;
	dTimingCutMap[Positron][SYS_FCAL] = 3.0;
	dTimingCutMap[Positron][SYS_ECAL] = 3.0;

	// All histograms go in the "highlevel" directory
	TDirectory *main = gDirectory;

	gDirectory->mkdir("highlevel")->cd();
	
	/************************************************************** Event Info ************************************************************/
	last_timestamp = 0.0;
	unix_offset = 0.0;
	dHist_EventInfo = new TH1D("EventInfo", "Misc. event info used to communicate event time to macros", 2, 0.0 , 2.0);

	/***************************************************************** RF *****************************************************************/

	int NRFbins = 1024;
	double ns_per_bin = 0.2;
	double RFlo = 0.0;
	double RFhi = RFlo + ns_per_bin*(double)NRFbins;
	double dft_min =  50.0; // min frequency in Mhz
	double dft_max = 900.0; // max frequency in Mhz
	int Ndft_bins = (1.0/dft_min - 1.0/dft_max)*1000.0/ns_per_bin;
	dHist_BeamBunchPeriod     = new TH1I("RFBeamBunchPeriod", "RF Beam Bunch Period;TAGH #Deltat (Within Same Counter) (ns)", NRFbins, RFlo, RFhi);
	dHist_BeamBunchPeriod_DFT = new TH1F("RFBeamBunchPeriod_DFT", "Fourier Transform of RF Beam Bunch Period;Beam bunch frequency (MHz)", Ndft_bins, dft_min, dft_max);

	/*************************************************************** TRIGGER **************************************************************/

	dNumHadronicTriggers_CoherentPeak_RFSignal.assign(33, 0.0);
	dNumHadronicTriggers_CoherentPeak_RFSideband.assign(33, 0.0);

	dRFSidebandBunchRange = pair<int, int>(3, 5);
	dShowerEOverPCut = 0.75;

	dHist_NumTriggers = new TH2I("NumTriggers", ";Trigger Bit", 33, 0.5, 33.5, 4, 0.5, 4.5); //"bit" 33: Total
	dHist_NumTriggers->GetYaxis()->SetBinLabel(1, "# Triggers");
	dHist_NumTriggers->GetYaxis()->SetBinLabel(2, "# Front Panel Triggers");
	dHist_NumTriggers->GetYaxis()->SetBinLabel(3, "# Hadronic Triggers");
	dHist_NumTriggers->GetYaxis()->SetBinLabel(4, "# Hadronic Triggers, Coherent Peak");
	for(int loc_i = 1; loc_i <= 32; ++loc_i)
	{
		ostringstream locBinStream;
		locBinStream << loc_i;
		dHist_NumTriggers->GetXaxis()->SetBinLabel(loc_i, locBinStream.str().c_str());
	}
	dHist_NumTriggers->GetXaxis()->SetBinLabel(33, "Total");


	dHist_BCALVsFCAL2_TrigBit1 = new TH2I("BCALVsFCAL2_TrigBit1","TRIG BIT 1;E (FCAL2) (count);E (BCAL) (count)", 200, 0., 10000, 200, 0., 50000);
	dHist_BCALVsFCAL_TrigBit1 = new TH2I("BCALVsFCAL_TrigBit1","TRIG BIT 1;E (FCAL) (count);E (BCAL) (count)", 200, 0., 10000, 200, 0., 50000);
	dHist_BCALVsECAL_TrigBit1 = new TH2I("BCALVsECAL_TrigBit1","TRIG BIT 1;E (ECAL) (count);E (BCAL) (count)", 200, 0., 10000, 200, 0., 50000);
	dHist_ECALVsFCAL_TrigBit1 = new TH2I("ECALVsFCAL_TrigBit1","TRIG BIT 1;E (FCAL) (count);E (ECAL) (count)", 200, 0., 20000, 200, 0., 20000);

	dHist_L1bits_gtp = new TH1I("L1bits_gtp", "L1 trig bits from GTP;Trig. bit (1-32)", 34, 0.5, 34.5);
	dHist_L1bits_fp  = new TH1I("L1bits_fp", "L1 trig bits from FP;Trig. bit (1-32)", 32, 0.5, 32.5);
        // BCAL LED Pseudo Trigger(1200 hits in BCAL) //
        dHist_L1bits_fp_twelvehundhits  = new TH1I("L1bits_fp_twelvehundhits", "Pseudo-trig bits (FP or >1200 hits);Trig. bit (1-32)", 5, 6.5, 11.5);
	/****************************************************** NUM RECONSTRUCTED OBJECTS *****************************************************/

	//2D Summary
	dHist_NumHighLevelObjects = new TH2I("NumHighLevelObjects", ";;# Objects / Event", 12, 0.5, 12.5, 101, -0.5, 100.5);
	dHist_NumHighLevelObjects->GetXaxis()->SetBinLabel(1, "DSCHit");
	dHist_NumHighLevelObjects->GetXaxis()->SetBinLabel(2, "DTOFPoint");
	dHist_NumHighLevelObjects->GetXaxis()->SetBinLabel(3, "DBCALShower");
	dHist_NumHighLevelObjects->GetXaxis()->SetBinLabel(4, "DFCALShower");
	dHist_NumHighLevelObjects->GetXaxis()->SetBinLabel(5, "DTimeBasedTrack");
	dHist_NumHighLevelObjects->GetXaxis()->SetBinLabel(6, "TrackSCMatches");
	dHist_NumHighLevelObjects->GetXaxis()->SetBinLabel(7, "TrackTOFMatches");
	dHist_NumHighLevelObjects->GetXaxis()->SetBinLabel(8, "TrackBCALMatches");
	dHist_NumHighLevelObjects->GetXaxis()->SetBinLabel(9, "TrackFCALMatches");
	dHist_NumHighLevelObjects->GetXaxis()->SetBinLabel(10, "DBeamPhoton");
	dHist_NumHighLevelObjects->GetXaxis()->SetBinLabel(11, "DChargedTrack");
	dHist_NumHighLevelObjects->GetXaxis()->SetBinLabel(12, "DNeutralShower");

	/************************************************************* KINEMATICS *************************************************************/

	// Beam Energy from tagger
	dHist_BeamEnergy = new TH1I("BeamEnergy", "Reconstructed Tagger Beam Energy;Beam Energy (GeV)", 240, 0.0, 12.0);

	// Beam Energy from PS
	dHist_PSPairEnergy = new TH1I("PSPairEnergy", "Reconstructed PS Beam Energy;Beam Energy (GeV)", 450, 3., 12.);

	// PVsTheta Time-Based Tracks
	dHist_PVsTheta_Tracks = new TH2I("PVsTheta_Tracks", "P vs. #theta for time-based tracks;#theta#circ;p (GeV/c)", 280, 0.0, 140.0, 150, 0.0, 12.0);

	// PhiVsTheta Time-Based Tracks
	dHist_PhiVsTheta_Tracks = new TH2I("PhiVsTheta_Tracks", "#phi vs. #theta for time-based tracks;#theta#circ;#phi#circ", 280, 0.0, 140.0, 60, -180.0, 180.0);

	/*************************************************************** VERTEX ***************************************************************/

	// Event Vertex-Z
	dEventVertexZ = new TH1I("EventVertexZ", "Reconstructed Event Vertex Z;Event Vertex-Z (cm)", 600, -100.0, 200.0);

	// Event Vertex-Y Vs Vertex-X
	dEventVertexYVsX = new TH2I("EventVertexYVsX", "Reconstructed Event Vertex X/Y;Event Vertex-X (cm);Event Vertex-Y (cm)", 400, -10.0, 10.0, 400, -10.0, 10.0);

	/*************************************************************** 2 gamma inv. mass ***************************************************************/
	// 2-gamma inv. mass
	d2gamma = new TH1I("TwoGammaMass", "2#gamma inv. mass;2#gamma inv. mass (GeV)", 400, 0.0, 1.2);

	isExclusive = true;
	if(app){
	  app->SetDefaultParameter("HIGHLEVEL_ONLINE:EXCLUSIVE", isExclusive, "Require exclusivity for meson production plots");
	}

	// pi+ pi-
	char str[256];
	if (isExclusive)
	  sprintf(str, "#pi^{+}#pi^{-} inv. mass w/ identified proton;#pi^{+}#pi^{-} inv. mass (GeV)");
	else
	  sprintf(str, "#pi^{+}#pi^{-} inv. mass w/o reconstructed recoil;#pi^{+}#pi^{-} inv. mass (GeV)");
	dpip_pim = new TH1I("PiPlusPiMinus", str, 400, 0.0, 2.0);

	// K+ K-
	if (isExclusive)
	  sprintf(str, "K^{+}K^{-} inv. mass w/ identified proton;K^{+}K^{-} inv. mass (GeV)");
	else
	  sprintf(str, "K^{+}K^{-} inv. mass w/o reconstructed recoil;K^{+}K^{-} inv. mass (GeV)");
	dKp_Km = new TH1I("KPlusKMinus", str, 400, 0.0, 2.0);

	// pi+ pi- pi0
	if (isExclusive)
	  sprintf(str, "#pi^{+}#pi^{-}#pi^{o} inv. mass w/ identified proton;#pi^{+}#pi^{-}#pi^{o} inv. mass (GeV)");
	else
	  sprintf(str, "#pi^{+}#pi^{-}#pi^{o} inv. mass w/o reconstructed recoil;#pi^{+}#pi^{-}#pi^{o} inv. mass (GeV)");
	dpip_pim_pi0 = new TH1I("PiPlusPiMinusPiZero", str, 200, 0.035, 2.0);
	dptrans = new TH1I("PiPlusPiMinusPiZeroProton_t", ";#pi^{+}#pi^{-}#pi^{0}p transverse momentum (GeV)", 500, 0.0, 1.0);
	dme_rho = new TH1I("PiPlusPiMinus_me", ";#pi^{+}#pi^{-} missing energy (GeV)", 500, -1.0, 1.0);
	dme_omega = new TH1I("PiPlusPiMinusPiZero_me", ";#pi^{+}#pi^{-}#pi^{0} missing energy (GeV)", 500, -1.0, 1.0);
	
	dbeta_vs_p = new TH2I("BetaVsP", "#beta vs. p (best FOM all charged tracks);p (GeV);#beta", 200, 0.0, 2.0, 100, 0.0, 1.2);
	dbeta_vs_p_TOF = new TH2I("BetaVsP_TOF", "#beta vs. p (best FOM all charged tracks in TOF);p (GeV);#beta", 200, 0.0, 2.0, 100, 0.0, 1.2);
	dbeta_vs_p_BCAL = new TH2I("BetaVsP_BCAL", "#beta vs. p (best FOM all charged tracks in BCAL);p (GeV);#beta", 200, 0.0, 2.0, 100, 0.0, 1.2);

	/*************************************************************** F1 TDC - fADC time ***************************************************************/

	// first, fill map of rocid/slot combos that have F1TDC's
	map<int, set<int>> f1tdc_rocid_slot;
	for(int slot=3; slot<=17; slot++){
		if( slot==11 || slot==12) continue;
		if( slot<=16 ) f1tdc_rocid_slot[75].insert(slot); // TAGMH
		if( slot<=4  ) f1tdc_rocid_slot[95].insert(slot); // STPSC1
		if( slot<=13 ) f1tdc_rocid_slot[36].insert(slot); // BCAL6
		if( slot<=13 ) f1tdc_rocid_slot[33].insert(slot); // BCAL3
		if( slot<=13 ) f1tdc_rocid_slot[39].insert(slot); // BCAL9
		if( slot<=13 ) f1tdc_rocid_slot[42].insert(slot); // BCAL12
		if( slot<=16 ) f1tdc_rocid_slot[51].insert(slot); // FDC1
		if( slot<=16 ) f1tdc_rocid_slot[54].insert(slot); // FDC4
		if( slot<=16 ) f1tdc_rocid_slot[63].insert(slot); // FDC13
		if( slot<=16 ) f1tdc_rocid_slot[64].insert(slot); // FDC14
		
	}
	
	// Create map that can be used to find the correct bin given the rocid,slot
	for(auto p : f1tdc_rocid_slot){
		int rocid = p.first;
		for(int slot : p.second){
			f1tdc_bin_map[ pair<int,int>(rocid,slot) ] = (double)f1tdc_bin_map.size();
		}
	}
	
	// Create histogram with bin labels
	dF1TDC_fADC_tdiff = new TH2D("F1TDC_fADC_tdiff", "F1TDC & fADC Time diff vs. rocid / slot", f1tdc_bin_map.size(), 0.5, 0.5+(double)f1tdc_bin_map.size(), 64, -64.0, 64.0);
	dF1TDC_fADC_tdiff->SetStats(0);
	dF1TDC_fADC_tdiff->SetYTitle("TDC - ADC in ns (or TDC/50 time for FDC)");
	for(auto p : f1tdc_bin_map){
		int rocid = p.first.first;
		int slot  = p.first.second;
		int jbin  = p.second;
		char str[256];
		sprintf(str, "R %d - S %d", rocid, slot);
		dF1TDC_fADC_tdiff->GetXaxis()->SetBinLabel(jbin+1, str);
	}
	
	/*************************************************************** Helicity ***************************************************************/

	dHist_heli_asym_gtp = new TH2I("Heli_asym_gtp", "Helicity Asymmetry per Trigger bit from GTP; Trig. bit (1-32); Helicity Asymmetry ", 34, 0.5, 34.5, 2, -0.5, 1.5);

	// back to main dir
	main->cd();
}

//------------------
// BeginRun
//------------------
void JEventProcessor_highlevel_online::BeginRun(const std::shared_ptr<const JEvent>& event)
{
  // This is called whenever the run number changes
	vector<double> locBeamPeriodVector;
	GetCalib(event, "PHOTON_BEAM/RF/beam_period", locBeamPeriodVector);
	dBeamBunchPeriod = locBeamPeriodVector[0];

	dCoherentPeakRange = pair<double, double>(8.4, 9.0);
	map<string, double> photon_beam_param;
	if(DEvent::GetCalib(event, "/PHOTON_BEAM/coherent_energy", photon_beam_param) == false)
		dCoherentPeakRange = pair<double, double>(photon_beam_param["cohmin_energy"], photon_beam_param["cohedge_energy"]);

	fcal_cell_thr  =  65;
	bcal_cell_thr  =  20;
	ecal_cell_thr  =  35;

	ecal_row_mask_min = 15;
	ecal_row_mask_max = 24;
	ecal_col_mask_min = 15;
	ecal_col_mask_max = 24;
	
	if( event->GetRunNumber() < 130000 )
	{
		fcal_row_mask_min = 26;
		fcal_row_mask_max = 32;
		fcal_col_mask_min = 26;
		fcal_col_mask_max = 32;
	}

	if( event->GetRunNumber() < 11127 )
	{
		fcal_row_mask_min = 24;
		fcal_row_mask_max = 34;
		fcal_col_mask_min = 24;
		fcal_col_mask_max = 34;
	}
}

//------------------
// Process
//------------------
void JEventProcessor_highlevel_online::Process(const std::shared_ptr<const JEvent>& locEvent)
{
	vector<const DTrackTimeBased*> locTrackTimeBasedVector;
	locEvent->Get(locTrackTimeBasedVector);

	vector<const DBeamPhoton*> locBeamPhotons;
	locEvent->Get(locBeamPhotons);

	vector<const DPSPair*> locPSPairs;
	locEvent->Get(locPSPairs);

	vector<const DPSCPair*> locPSCPairs;
	locEvent->Get(locPSCPairs);

	vector<const DFCALShower*> locFCALShowers;
	locEvent->Get(locFCALShowers);

	vector<const DBCALShower*> locBCALShowers;
	locEvent->Get(locBCALShowers);

	vector<const DNeutralShower*> locNeutralShowers;
	locEvent->Get(locNeutralShowers);

	vector<const DNeutralParticle*> locNeutralParticles;
	locEvent->Get(locNeutralParticles, "PreSelect");

	vector<const DTOFPoint*> locTOFPoints;
	locEvent->Get(locTOFPoints);

	vector<const DSCHit*> locSCHits;
	locEvent->Get(locSCHits);

	vector<const DTAGHHit*> locTAGHHits;
	locEvent->Get(locTAGHHits);

	vector<const DBCALDigiHit*> locBCALDigiHits;
	locEvent->Get(locBCALDigiHits);
        
	vector<const DECALDigiHit*> locECALDigiHits;
	locEvent->Get(locECALDigiHits);

        // BCAL LED Pseudo Trigger//
        vector<const DBCALHit*> locdbcalhits;
        locEvent->Get(locdbcalhits);

        vector<const DBCALPoint*> locdbcalpoints;
        locEvent->Get(locdbcalpoints);

	vector<const DFCALDigiHit*> locFCALDigiHits;
	locEvent->Get(locFCALDigiHits);

	const DCODAEventInfo* locCODAEventInfo = NULL;
	try {locEvent->GetSingle(locCODAEventInfo);}catch(...){}

	const DEPICSvalue* locEPICSvalue = NULL;
	try {locEvent->GetSingle(locEPICSvalue);}catch(...){}

	const DDetectorMatches* locDetectorMatches = NULL;
	locEvent->GetSingle(locDetectorMatches);

	const DEventRFBunch* locEventRFBunch = NULL;
	locEvent->GetSingle(locEventRFBunch);

	const DVertex* locVertex = NULL;
	locEvent->GetSingle(locVertex);

	vector<const DL1Trigger*> locL1Triggers;
	locEvent->Get(locL1Triggers);
	const DL1Trigger* locL1Trigger = locL1Triggers.empty() ? NULL : locL1Triggers[0];

	vector<const DChargedTrack*> locChargedTracks;
	locEvent->Get(locChargedTracks, "PreSelect");

	std::vector<const DBeamHelicity*> locBeamHelicities;
	locEvent->Get(locBeamHelicities);

	// The following declares containers for all types in F1Types
	// (defined at top of this file) and fills them.
	#define GetVect(A) \
		vector<const A*> v##A; \
		locEvent->Get(v##A);
	#define GetTaggerVect(A) \
		vector<const A*> v##A; \
		locEvent->Get(v##A,"Calib");
	F1Types(GetVect)		
	F1TaggerTypes(GetTaggerVect)		


	/************************************************************** Prepare Timestamp ************************************************************/

	// This is a way for us to pass the unix time of the event to the macros
	// so they can make entries in the HDTSDB (time series DB) based on the
	// time the event was acquired. This turns out to be a lot trickier than
	// you might think due to the unix timestamp only going into the data
	// stream on special event types. The 250MHz system clock is available
	// for physics events but needs the offset to the unix time of the start
	// of the event. It is also not available in the special events (EPICS,
	// scaler, and control).
	// 
	// We handle this in the following way:
	//
	// 1. remember last 250MHz clock time seen
	// 2. when special event with unix time is seen, assume it is
	//    the same time and calculate offset. Remember it so it
	//    can be used to calculate unix time from 250MHz clock on
	//    subsequent events. Always assume 250MHz is exact freq.
	//
	// This should be good to within a couple of seconds. One drawback
	// is that no unix time will be available until a special event
	// is seen. Those should be produced at about 1Hz, but in the
	// online farm with 20 nodes, that would be roughly 1 every 20seconds.
	// Thus, is could conceivably be a couple of minutes before all
	// nodes get initialized.
	//
	// Because RootSpy will automatically add 
	// histograms, we use 2 bins: bin 1 always has "1" so that the macro can
	// know how many numbers were added. bin 2 contains the actual unix time
	// stamp. The macro will have to take an average and that will get skewed
	// if a node drops out mid-run.
	double unix_time = 0;
	if(locL1Trigger){
		// TS scaler event
		unix_time = locL1Trigger->unix_time; // usually will be zero
	}
	if(locEPICSvalue && (unix_time==0)){
		// EPICS event
		unix_time = locEPICSvalue->timestamp;
	}
	
	double timestamp = 0.0;  // in seconds from 250MHz clock
	if(locCODAEventInfo) timestamp = (double)locCODAEventInfo->avg_timestamp / 250.0E6;
	
	if(timestamp != 0.0) last_timestamp = timestamp;
	if(unix_time!=0.0 && last_timestamp!=0.0) unix_offset = (double)unix_time - last_timestamp;
	if( (unix_time==0) && (timestamp!=0.0) && (unix_offset!=0.0) ) unix_time = (unix_offset + timestamp);

	/********************************************************* PREPARE RF ********************************************************/
	
	// Do discrete Fourier transform for selected frequency range
	double *rf_dft = NULL;
	if(dHist_BeamBunchPeriod->GetEntries() > 100){
		
		int NRF_bins      = dHist_BeamBunchPeriod->GetNbinsX();
		int Ndft_bins     = dHist_BeamBunchPeriod_DFT->GetNbinsX();
		double ns_per_bin = dHist_BeamBunchPeriod->GetBinWidth(1);
		double rfdomain   = ns_per_bin*(double)NRF_bins;
		rf_dft = new double[Ndft_bins];
		for(int ibin=1; ibin<=Ndft_bins; ibin++){

			// n.b. the factor of 1000 below is to convert from MHz to GHz
			double f = dHist_BeamBunchPeriod_DFT->GetXaxis()->GetBinCenter(ibin);
			double k = rfdomain/(1000.0/f); // k is number of oscillations in whole x-range for this freq.

			double dphi = TMath::TwoPi()*k/(double)NRF_bins;

			double sumc = 0.0;
			double sums = 0.0;
			for(int jbin=1; jbin<=(int)NRF_bins; jbin++){

				double v = dHist_BeamBunchPeriod->GetBinContent(jbin);
				double theta = (double)jbin*dphi;
				sumc += v*cos(theta);
				sums += v*sin(theta);
			}

			rf_dft[ibin-1] = sqrt(sumc*sumc + sums*sums);
		}
	}

	/********************************************************* PREPARE TAGGER HITS ********************************************************/

	//organize TAGH hits
	map<int, set<double> > locTAGHHitMap; //double: TAGH tdc times (set allows time sorting)
	for(size_t loc_i = 0; loc_i < locTAGHHits.size(); ++loc_i)
	{
		if(locTAGHHits[loc_i]->has_TDC)
			locTAGHHitMap[locTAGHHits[loc_i]->counter_id].insert(locTAGHHits[loc_i]->time_tdc);
	}

	//collect TAGH delta-t's for RF beam bunch histogram
	map<int, set<double> >::iterator locTAGHIterator = locTAGHHitMap.begin();
	vector<double> locTAGHDeltaTs;
	for(; locTAGHIterator != locTAGHHitMap.end(); ++locTAGHIterator)
	{
		set<double>& locCounterHits = locTAGHIterator->second;
		if(locCounterHits.size() < 2)
			continue;
		set<double>::iterator locSetIterator = locCounterHits.begin();
		set<double>::iterator locPreviousSetIterator = locSetIterator;
		for(++locSetIterator; locSetIterator != locCounterHits.end(); ++locSetIterator, ++locPreviousSetIterator)
			locTAGHDeltaTs.push_back(*locSetIterator - *locPreviousSetIterator);
	}

	/********************************************************* PREPARE TRIGGER INFO *******************************************************/

	vector<unsigned int> locgtpTrigBits(32, 0); //bit 1 -> 32 is index 0 -> 31
	vector<unsigned int> locfpTrigBits(32, 0); //bit 1 -> 32 is index 0 -> 31

	if(locL1Trigger != NULL)
	{
		for(unsigned int bit = 0; bit < 32; ++bit){
			locgtpTrigBits[bit] = (locL1Trigger->trig_mask & (1 << bit)) ? 1 : 0;
			locfpTrigBits[bit] = (locL1Trigger->fp_trig_mask & (1 << bit)) ? 1 : 0;
		}
	}

	//Get total FCAL energy
	int fcal_tot_en = 0;
	for( auto fcal_hit : locFCALDigiHits){

	  //		int row = fcal_hit->row;
	  //		int col = fcal_hit->column;
	  
	  //		if( (row >= fcal_row_mask_min) && (row <= fcal_row_mask_max) ){
	  //			if( (col >= fcal_col_mask_min) && (col <= fcal_col_mask_max) ) continue;
	  //		}
	  
	  if( ((int32_t)fcal_hit->pulse_peak-100) <= fcal_cell_thr) continue;
	  
	  // uint32_t adc_time = (fcal_hit->pulse_time >> 6) & 0x1FF; // consider only course time
	  // if((adc_time < 15) || (adc_time > 50)) continue; // changed from 20 and 70 based on run 30284  2/5/2017 DL
	  
	  Int_t pulse_int = fcal_hit->pulse_integral - fcal_hit->nsamples_integral*100;
	  if(pulse_int < 0) continue;
	  fcal_tot_en += pulse_int;
	}
	
	//Get total BCAL energy 
	int bcal_tot_en = 0;
	for(auto bcal_hit : locBCALDigiHits){
		if( ((int32_t)bcal_hit->pulse_peak-100) <= bcal_cell_thr) continue;
		Int_t pulse_int = bcal_hit->pulse_integral - bcal_hit->nsamples_integral*100;
		if(pulse_int < 0) continue;
		bcal_tot_en += pulse_int;
	}

	//Get total ECAL energy
	int ecal_tot_en = 0;
	for(auto ecal_hit : locECALDigiHits){
		int row = ecal_hit->row;
		int col = ecal_hit->column;
		
		if( (row >= ecal_row_mask_min) && (row <= ecal_row_mask_max) ){
			if( (col >= ecal_col_mask_min) && (col <= ecal_col_mask_max) ) continue;
		}
		
		if( ((int32_t)ecal_hit->pulse_peak-100) <= ecal_cell_thr) continue;
		Int_t pulse_int = ecal_hit->pulse_integral - ecal_hit->nsamples_integral*100;
		if(pulse_int < 0) continue;
		ecal_tot_en += pulse_int;
	}

	
	/********************************************************* PREPARE BEAM PHOTONS *******************************************************/

	vector<const DBeamPhoton*> locBeamPhotons_InTime;
	for(auto locBeamPhoton : locBeamPhotons)
	{
		double locDeltaT = locBeamPhoton->time() - locEventRFBunch->dTime;
		if(fabs(locDeltaT) <= 0.5*dBeamBunchPeriod)
			locBeamPhotons_InTime.push_back(locBeamPhoton);
	}
	
	vector<double> Eps; // pair energy in PS
	if(!locPSCPairs.empty()){
		for(auto pspair : locPSPairs){
			auto flhit = pspair->ee.first;
			auto frhit = pspair->ee.second;
			double E = flhit->E + frhit->E;
			Eps.push_back(E);
		}
	}

	/********************************************************* PREPARE HADRONIC TRIGGER *******************************************************/

	bool locIsHadronicEventFlag = false;
	for(auto locTrack : locChargedTracks)
	{
		//make sure at least one track isn't a lepton!! (read: e+/-. assume all muons come from pion decays)
		auto locChargedHypo = locTrack->Get_BestTrackingFOM();

		//timing cut: is it consistent with an e+/-??
		auto locDetector = locChargedHypo->t1_detector();
		double locDeltaT = locChargedHypo->time() - locChargedHypo->t0();
		if(fabs(locDeltaT) > dTimingCutMap[Electron][locDetector])
		{
			locIsHadronicEventFlag = true; //not an electron!!
			break;
		}

		//compute shower-E/p, cut
		double locP = locChargedHypo->momentum().Mag();
		double locShowerEOverP = 0.0;
		auto locFCALShowerMatchParams = locChargedHypo->Get_FCALShowerMatchParams();
		auto locBCALShowerMatchParams = locChargedHypo->Get_BCALShowerMatchParams();
		if(locFCALShowerMatchParams != NULL)
		{
			const DFCALShower* locFCALShower = locFCALShowerMatchParams->dFCALShower;
			locShowerEOverP = locFCALShower->getEnergy()/locP;
		}
		else if(locBCALShowerMatchParams != NULL)
		{
			const DBCALShower* locBCALShower = locBCALShowerMatchParams->dBCALShower;
			locShowerEOverP = locBCALShower->E/locP;
		}
		else //not matched to a shower
		{
			locIsHadronicEventFlag = true; //assume not an electron!!
			break;
		}

		if(locShowerEOverP < dShowerEOverPCut)
		{
			locIsHadronicEventFlag = true; //not an electron!!
			break;
		}
	}

	int locNumHadronicTriggers_CoherentPeak_RFSignal = 0;
	int locNumHadronicTriggers_CoherentPeak_RFSideband = 0;
	double locNumRFSidebandBunches = 2.0*double(dRFSidebandBunchRange.second - dRFSidebandBunchRange.first);
	if(locIsHadronicEventFlag)
	{
		//see if is in coherent peak
		for(auto& locBeamPhoton : locBeamPhotons)
		{
			if(std::isnan(locEventRFBunch->dTime))
				break; //RF sideband background too high: ignore
			if((locBeamPhoton->energy() < dCoherentPeakRange.first) || (locBeamPhoton->energy() > dCoherentPeakRange.second))
				continue; //not in coherent peak

			//delta-t sideband range
			double locSidebandMinDeltaT = dBeamBunchPeriod*(double(dRFSidebandBunchRange.first) - 0.5);
			double locSidebandMaxDeltaT = dBeamBunchPeriod*(double(dRFSidebandBunchRange.first) + 0.5);

			double locBeamRFDeltaT = locBeamPhoton->time() - locEventRFBunch->dTime;
			if(fabs(locBeamRFDeltaT) <= 0.5*dBeamBunchPeriod)
				++locNumHadronicTriggers_CoherentPeak_RFSignal;
			else if((fabs(locBeamRFDeltaT) >= locSidebandMinDeltaT) && (fabs(locBeamRFDeltaT) <= locSidebandMaxDeltaT))
				++locNumHadronicTriggers_CoherentPeak_RFSideband;
		}
	}


	/*************************************************************** F1 TDC - fADC time ***************************************************************/
	lockService->RootFillLock(this); //ACQUIRE ROOT FILL LOCK

	// The following fills the dF1TDC_fADC_tdiff histo for
	// all detectors that use F1TDC modules. See the templates
	// at the top of this file for details.
	#define F1Fill(A) FillF1Hist(v##A);
	F1Types(F1Fill)		
	F1TaggerTypes(F1Fill)		

	/************************************************************** Event Info ************************************************************/

	if( unix_time!=0 ){
		dHist_EventInfo->SetBinContent(1, 1);
		dHist_EventInfo->SetBinContent(2, unix_time);
	}

	/***************************************************************** RF *****************************************************************/
		
	for(size_t loc_i = 0; loc_i < locTAGHDeltaTs.size(); ++loc_i)
		dHist_BeamBunchPeriod->Fill(locTAGHDeltaTs[loc_i]);
	
	if(rf_dft){
		for(int i=0; i<dHist_BeamBunchPeriod_DFT->GetNbinsX(); i++){
			dHist_BeamBunchPeriod_DFT->SetBinContent(i+1, rf_dft[i]);
		}
		delete[] rf_dft;
	}
		
	/*************************************************************** TRIGGER **************************************************************/

	if(locL1Trigger != NULL)
	{
	  
		if(locgtpTrigBits[0] == 1) //bit 1
                  {
		    dHist_BCALVsFCAL2_TrigBit1->Fill(0.4957*Float_t(fcal_tot_en)+Float_t(ecal_tot_en), Float_t(bcal_tot_en));
		    dHist_BCALVsFCAL_TrigBit1->Fill(0.4957*Float_t(fcal_tot_en), Float_t(bcal_tot_en));
		    dHist_BCALVsECAL_TrigBit1->Fill(Float_t(ecal_tot_en), Float_t(bcal_tot_en));
                    dHist_ECALVsFCAL_TrigBit1->Fill(0.4957*Float_t(fcal_tot_en), Float_t(ecal_tot_en));
                  }

		// trigger bits
		for(int bit=1; bit<=32; bit++){
			if(locgtpTrigBits[bit-1]) dHist_L1bits_gtp->Fill(bit);
			if(locfpTrigBits[bit-1] ) dHist_L1bits_fp->Fill(bit);
		}
		
		// Keep counts of events with any physics trigger as bit 33
		if( locgtpTrigBits[1-1] | locgtpTrigBits[2-1] | locgtpTrigBits[3-1] ) dHist_L1bits_gtp->Fill(33);

		// #triggers: total
		if(locL1Trigger->trig_mask > 0)
			dHist_NumTriggers->Fill(33, 1);
		if(locL1Trigger->fp_trig_mask > 0)
			dHist_NumTriggers->Fill(33, 2);

		// #triggers: by bit
		for(int locTriggerBit = 1; locTriggerBit <= 32; ++locTriggerBit)
		{
			if(locgtpTrigBits[locTriggerBit - 1]) //gtp (normal)
				dHist_NumTriggers->Fill(locTriggerBit, 1);
			if(locfpTrigBits[locTriggerBit - 1]) //front panel
				dHist_NumTriggers->Fill(locTriggerBit, 2);
		}

		// #hadronic triggers
		if(locIsHadronicEventFlag)
		{
			//total
			if(locL1Trigger->trig_mask > 0)
			{
				dHist_NumTriggers->Fill(33, 3);

				//coherent peak
				dNumHadronicTriggers_CoherentPeak_RFSignal[32] += double(locNumHadronicTriggers_CoherentPeak_RFSignal);
				dNumHadronicTriggers_CoherentPeak_RFSideband[32] += double(locNumHadronicTriggers_CoherentPeak_RFSideband);
				int locNumHadronicTriggers_CoherentPeak = int(dNumHadronicTriggers_CoherentPeak_RFSignal[32] - dNumHadronicTriggers_CoherentPeak_RFSideband[32]/locNumRFSidebandBunches + 0.5); //+0.5: round
				dHist_NumTriggers->SetBinContent(33, 4, locNumHadronicTriggers_CoherentPeak); //# hadronic triggers
			}

			//by bit
			for(int locTriggerBit = 1; locTriggerBit <= 32; ++locTriggerBit)
			{
				if(!locgtpTrigBits[locTriggerBit - 1])
					continue;

				//all hadronic
				dHist_NumTriggers->Fill(locTriggerBit, 3);

				//coherent peak: must subtract RF sidebands to determine the actual beam energy
				dNumHadronicTriggers_CoherentPeak_RFSignal[locTriggerBit - 1] += double(locNumHadronicTriggers_CoherentPeak_RFSignal);
				dNumHadronicTriggers_CoherentPeak_RFSideband[locTriggerBit - 1] += double(locNumHadronicTriggers_CoherentPeak_RFSideband);
				double locNumHadronicTriggers_CoherentPeak = dNumHadronicTriggers_CoherentPeak_RFSignal[locTriggerBit - 1] - dNumHadronicTriggers_CoherentPeak_RFSideband[locTriggerBit - 1]/locNumRFSidebandBunches;
				dHist_NumTriggers->SetBinContent(locTriggerBit, 4, locNumHadronicTriggers_CoherentPeak); //# hadronic triggers
			}

			//coherent peak rate
			double locHadronicCoherentPeakRate = dHist_NumTriggers->GetBinContent(33, 4)/dHist_NumTriggers->GetBinContent(33, 1);

			ostringstream locHistTitle;
			locHistTitle << "Total Hadronic Coherent Peak Rate = " << locHadronicCoherentPeakRate;
			dHist_NumTriggers->SetTitle(locHistTitle.str().c_str());
		}
	}
        // BCAL LED Pseudo Trigger(1200 hits in BCAL && energy conditions are satisfied) //
       float Eus=0;
       float Eds=0;
     for(unsigned int i=0; i<locdbcalpoints.size(); i++) {
         const DBCALPoint *point = locdbcalpoints[i];
         float pointEus = point->E_US();
               Eus=Eus+pointEus;                 
         float pointEds = point->E_DS();
               Eds=Eds+pointEds;            
        }

        bool LED_US=0, LED_DS=0;

	const DL1Trigger *trig = NULL;
	try {
		locEvent->GetSingle(trig);
	} catch (...) {}
	if (trig) {
        	if (trig->fp_trig_mask & 0x100){//bit=9
			// Upstream LED trigger fired
			//trigUS++;
			LED_US=1;
			
		}
		if (trig->fp_trig_mask & 0x200){//bit=10
			// Downstream LED trigger fired
			//trigDS++;
			LED_DS=1;
		
		}
            }
        int pseudo_triggerbit = 0;
        //Fill pseudo triggerbit histogram if energy conditions are satisfied && number of BCAL hits is > 1200 && FP trigger fails 	
        if (locdbcalhits.size()>1200){
        pseudo_triggerbit = 7;
           }
        if(Eds>Eus && locdbcalhits.size()>1200. && LED_US==0 && LED_DS==0){
        pseudo_triggerbit=9;//Upstream Pseudo triggerbit
        }
        if(Eus>Eds && locdbcalhits.size()>1200.&& LED_US==0 && LED_DS==0){
        pseudo_triggerbit=10; //Downstream Pseudo triggerbit
        }

       dHist_L1bits_fp_twelvehundhits->Fill(pseudo_triggerbit);
	// DON'T DO HIGHER LEVEL PROCESSING FOR FRONT PANEL TRIGGER EVENTS, OR NON-TRIGGER EVENTS
    if(!locL1Trigger || (locL1Trigger && (locL1Trigger->fp_trig_mask>0))) {
        lockService->RootFillUnLock(this); //RELEASE ROOT FILL LOCK
        return;
    }

	/****************************************************** NUM RECONSTRUCTED OBJECTS *****************************************************/

	//# High-Level Objects
	dHist_NumHighLevelObjects->Fill(1, (Double_t)locSCHits.size());
	dHist_NumHighLevelObjects->Fill(2, (Double_t)locTOFPoints.size());
	dHist_NumHighLevelObjects->Fill(3, (Double_t)locBCALShowers.size());
	dHist_NumHighLevelObjects->Fill(4, (Double_t)locFCALShowers.size());
	dHist_NumHighLevelObjects->Fill(5, (Double_t)locTrackTimeBasedVector.size());
	dHist_NumHighLevelObjects->Fill(6, (Double_t)locDetectorMatches->Get_NumTrackSCMatches());
	dHist_NumHighLevelObjects->Fill(7, (Double_t)locDetectorMatches->Get_NumTrackTOFMatches());
	dHist_NumHighLevelObjects->Fill(8, (Double_t)locDetectorMatches->Get_NumTrackBCALMatches());
	dHist_NumHighLevelObjects->Fill(9, (Double_t)locDetectorMatches->Get_NumTrackFCALMatches());
	dHist_NumHighLevelObjects->Fill(10, (Double_t)locBeamPhotons.size());
	dHist_NumHighLevelObjects->Fill(11, (Double_t)locChargedTracks.size());
	dHist_NumHighLevelObjects->Fill(12, (Double_t)locNeutralShowers.size());

	/************************************************************* KINEMATICS *************************************************************/
	
	for(size_t loc_i = 0; loc_i < locBeamPhotons.size(); ++loc_i)
		dHist_BeamEnergy->Fill(locBeamPhotons[loc_i]->energy());
	
	for(auto E : Eps) dHist_PSPairEnergy->Fill(E);

	for(size_t loc_i = 0; loc_i < locChargedTracks.size(); ++loc_i)
	{
		auto locChargedHypo = locChargedTracks[loc_i]->Get_BestTrackingFOM();
		if(locChargedHypo->t1_detector() == SYS_NULL) continue; // skip tracks with artificial betas
		double locP = locChargedHypo->momentum().Mag();
		double locTheta = locChargedHypo->momentum().Theta()*180.0/TMath::Pi();
		double locPhi = locChargedHypo->momentum().Phi()*180.0/TMath::Pi();
		double locBeta = locChargedHypo->measuredBeta();

		auto locBCALShowerMatchParams = locChargedHypo->Get_BCALShowerMatchParams();
		//auto locFCALShowerMatchParams = locChargedHypo->Get_FCALShowerMatchParams();
		auto locTOFHitMatchParams = locChargedHypo->Get_TOFHitMatchParams();
		//auto locSCHitMatchParams = locChargedHypo->Get_SCHitMatchParams();

		dHist_PVsTheta_Tracks->Fill(locTheta, locP);
		dHist_PhiVsTheta_Tracks->Fill(locTheta, locPhi);
		
		dbeta_vs_p->Fill(locP, locBeta);
		if(locTOFHitMatchParams != NULL) {
			dbeta_vs_p_TOF->Fill(locP, locBeta);
		}
		if(locBCALShowerMatchParams != NULL) {
			dbeta_vs_p_BCAL->Fill(locP, locBeta);
		}
	}
	
	/*************************************************************** VERTEX ***************************************************************/
	
	if(locChargedTracks.size() >= 2)
	{
		dEventVertexZ->Fill(locVertex->dSpacetimeVertex.Z());
		dEventVertexYVsX->Fill(locVertex->dSpacetimeVertex.X(), locVertex->dSpacetimeVertex.Y());
	}
	
	/*************************************************************** 2 gamma inv. mass ***************************************************************/

	vector<DLorentzVector> pi0s;
	for(uint32_t i=0; i<locNeutralParticles.size(); i++){

		auto hypoth1 = locNeutralParticles[i]->Get_Hypothesis(Gamma);
		if(!hypoth1) continue;

		//timing cut
		auto detector1 = hypoth1->t1_detector();
		double locDeltaT = hypoth1->time() - hypoth1->t0();
		if(fabs(locDeltaT) > dTimingCutMap[Gamma][detector1])
			continue;

		const DLorentzVector &v1 = hypoth1->lorentzMomentum();

		for(uint32_t j=i+1; j<locNeutralParticles.size(); j++){

			auto hypoth2 = locNeutralParticles[j]->Get_Hypothesis(Gamma);
			if(!hypoth2) continue;

			//timing cut
			auto detector2 = hypoth2->t1_detector();
			locDeltaT = hypoth2->time() - hypoth2->t0();
			if(fabs(locDeltaT) > dTimingCutMap[Gamma][detector2])
				continue;

			const DLorentzVector &v2 = hypoth2->lorentzMomentum();

			DLorentzVector ptot(v1+v2);
			double mass = ptot.M();
			d2gamma->Fill(mass);
			if(mass>0.1 && mass<0.17) pi0s.push_back(ptot);
		}
	}

	/*************************************************************** pi+ pi- pi0 ***************************************************************/

	for(auto t1 : locChargedTracks){
		//look for pi+
		auto hypoth1 = t1->Get_Hypothesis(PiPlus);
		if(!hypoth1) continue;

		//timing cut
		auto detector1 = hypoth1->t1_detector();
		double locDeltaT = hypoth1->time() - hypoth1->t0();
		if(fabs(locDeltaT) > dTimingCutMap[PiPlus][detector1])
			continue;

		const DLorentzVector &pipmom = hypoth1->lorentzMomentum();

		for(auto t2 : locChargedTracks){
			if(t2 == t1) continue;

			//look for pi-
			auto hypoth2 = t2->Get_Hypothesis(PiMinus);
			if(!hypoth2) continue;

			//timing cut
			auto detector2 = hypoth2->t1_detector();
			locDeltaT = hypoth2->time() - hypoth2->t0();
			if(fabs(locDeltaT) > dTimingCutMap[PiMinus][detector2])
				continue;

			const DLorentzVector &pimmom = hypoth2->lorentzMomentum();

			DLorentzVector rhomom(pipmom + pimmom);
			DLorentzVector protonP4;

			if (isExclusive){
			  for(auto t3 : locChargedTracks){
			    if(t3 == t1) continue;
			    if(t3 == t2) continue;

			    //look for proton
			    auto hypoth3 = t3->Get_Hypothesis(Proton);
			    if(!hypoth3) continue;

			    //timing cut
			    auto detector3 = hypoth3->t1_detector();
			    locDeltaT = hypoth3->time() - hypoth3->t0();
			    if(fabs(locDeltaT) > dTimingCutMap[Proton][detector3])
			      continue;

			    const DLorentzVector &protonmom = hypoth3->lorentzMomentum();

			    protonP4 = protonmom;

			    for(auto locBeamPhoton : locBeamPhotons_InTime)
			      {
				// for exclusive rho: require at least one beam photon in time with missing mass squared near 0
				if (isExclusive){
				  DLorentzVector locTargetP4(0.0, 0.0, 0.0, ParticleMass(Proton));
				  DLorentzVector locMissingP4 =  locBeamPhoton->lorentzMomentum() + locTargetP4 - rhomom - protonP4;
				  if(fabs(locMissingP4.M2()) > 0.01)
				    continue;
				  dpip_pim->Fill(rhomom.M());
				  break;
				}
			      }
			  }
			}
			// for inclusive rho: require missing energy to be near 0
			else {
			  for(auto locBeamPhoton : locBeamPhotons_InTime)
			    {
			      DLorentzVector locMissingP4 =  locBeamPhoton->lorentzMomentum() - rhomom;
			      dme_rho->Fill(locMissingP4.E());
			      if(fabs(locMissingP4.E()) > 0.2)
				continue;
			      dpip_pim->Fill(rhomom.M());
			      break;
			    }
			}

			for(uint32_t i=0; i<pi0s.size(); i++){
			  DLorentzVector &pi0mom = pi0s[i];
			  DLorentzVector omegamom(pi0mom + rhomom);

			  for(auto locBeamPhoton : locBeamPhotons_InTime)
			    {
			      //for exclusive omega: require at least one beam photon in time with missing mass squared near 0
			      if (isExclusive){
				DLorentzVector locTargetP4(0.0, 0.0, 0.0, ParticleMass(Proton));
				DLorentzVector locFinalStateP4 = omegamom + protonP4;
				DLorentzVector locMissingP4 = locBeamPhoton->lorentzMomentum() + locTargetP4 - locFinalStateP4;
				if(fabs(locMissingP4.M2()) > 0.01)
				  continue;
				dpip_pim_pi0->Fill(omegamom.M());
				double ptrans = locFinalStateP4.Perp();
				dptrans->Fill(ptrans);
				break;
			      }
			      //for inclusive omega: require missing energy to be near 0
			      else {
				DLorentzVector locMissingP4 = locBeamPhoton->lorentzMomentum() - omegamom;
				dme_omega->Fill(locMissingP4.E());
				if(fabs(locMissingP4.E()) > 0.2)
				  continue;
				dpip_pim_pi0->Fill(omegamom.M());
				break;
			      }
			    }
			}
		}
	}

	/*************************************************************** K+ K- ***************************************************************/

	for(auto t1 : locChargedTracks){
		//look for K+
		auto hypoth1 = t1->Get_Hypothesis(KPlus);
		if(!hypoth1) continue;

		//timing cut
		auto detector1 = hypoth1->t1_detector();
		double locDeltaT = hypoth1->time() - hypoth1->t0();
		if(fabs(locDeltaT) > dTimingCutMap[PiPlus][detector1]) // use same timing cuts as pion
			continue;

		const DLorentzVector &Kpmom = hypoth1->lorentzMomentum();

		for(auto t2 : locChargedTracks){
			if(t2 == t1) continue;

			//look for pi-
			auto hypoth2 = t2->Get_Hypothesis(KMinus);
			if(!hypoth2) continue;

			//timing cut
			auto detector2 = hypoth2->t1_detector();
			locDeltaT = hypoth2->time() - hypoth2->t0();
			if(fabs(locDeltaT) > dTimingCutMap[PiMinus][detector2]) // use same timing cuts as pion
				continue;

			const DLorentzVector &Kmmom = hypoth2->lorentzMomentum();
			DLorentzVector phimom(Kpmom + Kmmom);

			for(auto locBeamPhoton : locBeamPhotons_InTime)
			  {
			    //for exclusive phi: require at least one beam photon in time with missing mass squared near 0
			    if (isExclusive){
			      for(auto t3 : locChargedTracks){
				if(t3 == t1) continue;
				if(t3 == t2) continue;

				//look for proton
				auto hypoth3 = t3->Get_Hypothesis(Proton);
				if(!hypoth3) continue;

				//timing cut
				auto detector3 = hypoth3->t1_detector();
				locDeltaT = hypoth3->time() - hypoth3->t0();
				if(fabs(locDeltaT) > dTimingCutMap[Proton][detector3])
				  continue;

				const DLorentzVector &protonmom = hypoth3->lorentzMomentum();

				DLorentzVector locTargetP4(0.0, 0.0, 0.0, ParticleMass(Proton));
				DLorentzVector locMissingP4 = locBeamPhoton->lorentzMomentum() + locTargetP4 - phimom - protonmom;
				if(fabs(locMissingP4.M2()) > 0.01)
				  continue;
				dKp_Km->Fill(phimom.M());
				break;
			      }
			    }
			    // for inclusive phi: require missing energy to be near 0
			    else {
			      DLorentzVector locMissingP4 =  locBeamPhoton->lorentzMomentum() - phimom;
			      if(fabs(locMissingP4.E()) > 0.1)
				continue;
			      dKp_Km->Fill(phimom.M());
			      break;
			    }
			  }
		}
	}

	/*************************************************************** Helicity ***************************************************************/


	// Save helicity by trigger bit
	for (size_t i=0; i < locBeamHelicities.size(); ++i)
	  {
	    for(int locTriggerBit = 1; locTriggerBit <= 32; ++locTriggerBit)
	      {
		if(locgtpTrigBits[locTriggerBit - 1])
		  if (locBeamHelicities[i]->valid)
		    dHist_heli_asym_gtp->Fill(locTriggerBit, locBeamHelicities[i]->helicity);
	      }
	  }

	lockService->RootFillUnLock(this); //RELEASE ROOT FILL LOCK
}

//------------------
// EndRun
//------------------
void JEventProcessor_highlevel_online::EndRun()
{
  // This is called whenever the run number changes, before it is
  // changed to give you a chance to clean up before processing
  // events from the next run number.
}

//------------------
// Finish
//------------------
void JEventProcessor_highlevel_online::Finish()
{
  // Called before program exit after event processing is finished.
}

