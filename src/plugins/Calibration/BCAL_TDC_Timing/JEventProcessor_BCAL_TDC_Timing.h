// $Id$
//
//    File: JEventProcessor_BCAL_TDC_Timing.h
// Created: Tue Jul 28 10:55:56 EDT 2015
// Creator: mstaib (on Linux egbert 2.6.32-504.30.3.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_BCAL_TDC_Timing_
#define _JEventProcessor_BCAL_TDC_Timing_

#include <JANA/JEventProcessor.h>
#include <JANA/Services/JLockService.h>
#include <BCAL/DBCALGeometry.h>
#include <TRACKING/DTrackFitter.h>

#include <TH1F.h>
#include <TH2F.h>

#include <map>
#include <string>
using namespace std;

class JEventProcessor_BCAL_TDC_Timing:public JEventProcessor{
	public:
		JEventProcessor_BCAL_TDC_Timing();
		~JEventProcessor_BCAL_TDC_Timing();

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;

		std::shared_ptr<JLockService> lockService;

		const int NBCALMODS=48;         ///< number of modules
		const int NBCALLAYERS=4;         ///< number of layers in a module
		const int NBCALSECTORS=4;         ///< number of sectors in a module

		uint32_t VERBOSE;
		uint32_t VERBOSEHISTOGRAMS;

        bool DONT_USE_SC;
        double Z_TARGET;

      //Used as a key for maps
      class readout_channel {
         public:
            readout_channel(int cellId, DBCALGeometry::End end) :
               cellId(cellId), end(end) {}

            int cellId;
            DBCALGeometry::End end;

            bool operator<(const readout_channel &c) const {
               if (cellId<c.cellId) return true;
               if (cellId>c.cellId) return false;
               if (end==DBCALGeometry::kUpstream && c.end==DBCALGeometry::kDownstream) return true;
               return false;
            }
      };

      //For now timewalk corrections are of the form f(ADC) = c0 + c1/(ADC-c3)^c2
      //Store all coefficients in one structure
      class timewalk_coefficients {
         public:
            timewalk_coefficients() :
               a_thresh(0), c0(0), c1(0), c2(0) {}
            timewalk_coefficients(float c0, float c1, float c2, float a_thresh) :
               a_thresh(a_thresh), c0(c0), c1(c1), c2(c2) {}
            float a_thresh,c0,c1,c2;
      };

      map<readout_channel,timewalk_coefficients> tdc_timewalk_map; 


	// histograms
	TH2F *hUpstream_Channel_Deltat_All;
	TH2F *hDownstream_Channel_Deltat_All;
	TH2F *hUpstream_Channel_Deltat_All_Corrected;
	TH2F *hDownstream_Channel_Deltat_All_Corrected;
	
	map<int, TH2F*> hUpstream_TimewalkVsPeak;
	map<int, TH2F*> hDownstream_TimewalkVsPeak;
	map<int, TH2F*> hUpstream_TimewalkVsPeak_Corrected;
	map<int, TH2F*> hDownstream_TimewalkVsPeak_Corrected;
	
	TH1F *hDebug;
	TH2F *hBCALMatch;
	TH2F *hPosShowers_Evst;
	TH2F *hNegShowers_Evst;
	TH2F *hPosPionShowers_Evst;
	TH2F *hPosPionShowers_Pvst;
	TH2F *hPosPionShowers_zvst;
	TH2F *hNegPionShowers_Evst;
	TH2F *hNegPionShowers_Pvst;
	TH2F *hNegPionShowers_zvst;
	TH2F *hNeutShowers_Evst;
	TH2F *hNeutShowers_zvst;
	
	TH2F *hPosShowers_PvsdEdx;
	TH2F *hPosShowers_dEdxvst;
	TH2F *hPosShowers_PvsE;
	TH2F *hPosShowers_PvsEP;
	TH2F *hNegShowers_PvsdEdx;
	TH2F *hNegShowers_dEdxvst;
	TH2F *hNegShowers_PvsE;
	TH2F *hNegShowers_PvsEP;
	TH2F *hMatching_dZvsdPhi;
	
	TH2F *hNpointVsEshower_qpos;
	TH2F *hNpointVsEshower_qneg;
	TH2F *hNpointVsEshower_q0;
	TH2F *hEpointVsEshower_qpos;
	TH2F *hEpointVsEshower_qneg;
	
	map<int, TH2F*> hEpointVsEshower_qpos_layer;
	map<int, TH2F*> hEpointVsEshower_qneg_layer;
	
	TH2F *hAllPointsVsShower_zpos;
	
	TH2F *hZvsDeltat_all;
	TH2F *hZvsDeltat_qpos;
	TH2F *hZvsDeltat_qneg;
	
	map<int, TH2F*> hZvsDeltat_layer;
	map<int, TH2F*> hZvsDeltat_chan;
	map<int, TH2F*> hThetavsDeltat_layer;

	TH1F *hDeltat_corr_all;
	TH2F *hDeltatvscell_corr;
	
	TH2F *hZvsDeltaz_all;
	TH2F *hZvsDeltaz_qpos;
	TH2F *hZvsDeltaz_qneg;
	
	map<int, TH2F*> hZvsDeltaz_layer;
	map<int, TH2F*> hZvsDeltaz_chan;

	TH2F *htrackZvsBCALZ_all;
	TH2F *htrackZvsBCALZ_qpos;
	TH2F *htrackZvsBCALZ_qneg;
	
	map<int, TH2F*> htrackZvsBCALZ_layer;
	map<int, TH2F*> htrackZvsBCALZ_chan;

	TH1F *hDeltat_raw_all;
	TH2F *hDeltat_raw_chan;
	
	TH2F *hdeltaTVsCell_all;
	TH2F *hdeltaTVsCell_qpos;
	TH2F *hdeltaTVsCell_qneg;
	TH2F *hdeltaTVsCell_q0;
	TH2F *hdeltaTVsCell_qpos_Eweight;
	TH2F *hdeltaTVsCell_qneg_Eweight;
	TH2F *hdeltaTVsCell_q0_Eweight;
	TH2F *hdeltaTVsCell_qpos_E2weight;
	TH2F *hdeltaTVsCell_qneg_E2weight;
	TH2F *hdeltaTVsCell_q0_E2weight;
	TH2F *hdeltaTVsLayer_qpos;
	TH2F *hdeltaTVsLayer_qneg;
	TH2F *hHitDeltaTVsChannel;
	TH2F *hHittimediff;
	
	TH2F *hHits_deltaTVsE_ADC_qpos;
	TH2F *hHits_deltaTVsE_TDC_qpos;
	TH2F *hHits_deltaTVsE_Mixed_qpos;
	TH2F *hHits_deltaTVsE_ADC_qneg;
	TH2F *hHits_deltaTVsE_TDC_qneg;
	TH2F *hHits_deltaTVsE_Mixed_qneg;
	
	TH2F *hHits_deltaTVsPPmax_ADC_qpos;
	TH2F *hHits_deltaTVsPPmax_TDC_qpos;
	TH2F *hHits_deltaTVsPPmax_Mixed_qpos;
	TH2F *hHits_deltaTVsPPmax_ADC_qneg;
	TH2F *hHits_deltaTVsPPmax_TDC_qneg;
	TH2F *hHits_deltaTVsPPmax_Mixed_qneg;
	
	TH2F *hHits_deltaTVsPPmin_ADC_qpos;
	TH2F *hHits_deltaTVsPPmin_TDC_qpos;
	TH2F *hHits_deltaTVsPPmin_Mixed_qpos;
	TH2F *hHits_deltaTVsPPmin_ADC_qneg;
	TH2F *hHits_deltaTVsPPmin_TDC_qneg;
	TH2F *hHits_deltaTVsPPmin_Mixed_qneg;
	
	TH2F *hPoints_deltaTVsEnergy_qpos;
	TH2F *hPoints_deltaTVsEnergy_qneg;
	TH2F *hPoints_deltaTVsEnergy_q0;
	TH2F *hPoints_altdeltaTVsEnergy_q0;
	TH2F *hPoints_deltaTVsEnergy_ADC_qpos;
	TH2F *hPoints_deltaTVsEnergy_ADC_qneg;
	TH2F *hPoints_deltaTVsEnergy_TDC_qpos;
	TH2F *hPoints_deltaTVsEnergy_TDC_qneg;
	TH2F *hPoints_deltaTVsEnergy_Mixed_qpos;
	TH2F *hPoints_deltaTVsEnergy_Mixed_qneg;
	
	map<int, TH2F*> hPoints_deltaTVsEnergy_qpos_layer;
	map<int, TH2F*> hPoints_deltaTVsEnergy_qneg_layer;
	map<int, TH2F*> hPoints_deltaTVsEnergy_q0_layer;
	map<int, TH2F*> hPoints_deltaTVsEnergy_ADC_qpos_layer;
	map<int, TH2F*> hPoints_deltaTVsEnergy_ADC_qneg_layer;
	map<int, TH2F*> hPoints_deltaTVsEnergy_TDC_qpos_layer;
	map<int, TH2F*> hPoints_deltaTVsEnergy_TDC_qneg_layer;
	map<int, TH2F*> hPoints_deltaTVsEnergy_Mixed_qpos_layer;
	map<int, TH2F*> hPoints_deltaTVsEnergy_Mixed_qneg_layer;
	map<int, TH2F*> hPoints_altdeltaTVsEnergy_q0_layer;

	
	TH2F *hPoints_deltaTVsShowerEnergy_qpos;
	TH2F *hPoints_deltaTVsShowerEnergy_qneg;
	TH2F *hPoints_deltaTVsShowerEnergy_q0;
	
	map<int, TH2F*> hPoints_deltaTVsShowerEnergy_qpos_layer;
	map<int, TH2F*> hPoints_deltaTVsShowerEnergy_qneg_layer;
	map<int, TH2F*> hPoints_deltaTVsShowerEnergy_q0_layer;
	map<int, TH2F*> hPoints_altdeltaTVsShowerEnergy_q0_layer;
	
	TH2F *hPoints_altdeltaTVsShowerEnergy_q0;
	TH2F *hlogintratiovsZtrack_all;

	map<int, TH2F*> hlogintratiovsZtrack_chan;


	//TH2F *;
	
	

};

#endif // _JEventProcessor_BCAL_TDC_Timing_

