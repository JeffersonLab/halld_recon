// $Id$
//
//    File: DEventProcessor_BCAL_gainmatrix.h
// Created: Fri Oct 10 16:41:18 EDT 2014
// Creator: wmcginle (on Linux ifarm1101 2.6.32-220.7.1.el6.x86_64 x86_64)
//

#ifndef _DEventProcessor_BCAL_gainmatrix_
#define _DEventProcessor_BCAL_gainmatrix_

#include <JANA/JEventProcessor.h>
#include <JANA/JApplication.h>

#include <ANALYSIS/DEventWriterROOT.h>
#include <HDDM/DEventWriterREST.h>
#include <ANALYSIS/DHistogramActions.h>
#include "ANALYSIS/DAnalysisUtilities.h"
#include "TRACKING/DTrackFitter.h"

#include "DLorentzVector.h"
#include "TMatrixD.h"


using namespace std;

class DEventProcessor_BCAL_gainmatrix : public JEventProcessor
{
	public:
		DEventProcessor_BCAL_gainmatrix(){
			SetTypeName("DEventProcessor_BCAL_gainmatrix");
		};
		~DEventProcessor_BCAL_gainmatrix(){};
		TTree *BCAL_Neutrals;
	       	uint32_t eventnum  ;
		Float_t E1  ;
		Float_t E1_raw  ;
		Float_t E2  ;
		Float_t E2_raw  ;
		Float_t t1  ;
		Float_t t2  ;
		Float_t z1  ;
		Float_t z2  ;
		Float_t x1  ;
		Float_t x2  ;
		Float_t y1  ;
		Float_t y2  ;
		Float_t psi  ;
		Float_t vertexz ;
		Float_t vertexZ  ;
		Float_t vertexX  ;
		Float_t vertexY  ;
		uint32_t Run_Number  ;
		Int_t num_tracks ;
		Int_t num_showers ;
		Float_t inv_mass  ;
		Float_t inv_mass_raw  ;
		uint32_t logical1;
		uint32_t logical2;
		vector<double> point_energy;
		vector<double> point1_energy_calib;
		vector<double> point2_energy_calib;
		vector<double> point1_channel;
		vector<double> point2_channel;
		vector<pair < double , int > > frac_en;

	private:
		const DAnalysisUtilities* dAnalysisUtilities;
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& locEvent) override;
		void Process(const std::shared_ptr<const JEvent>& locEvent) override;
		void EndRun() override;
		void Finish() override;

		std::shared_ptr<JLockService> lockService;



		double m_massbias;

		TMatrixD m_mC;
		TMatrixD m_mD;
		TMatrixD m_mL;
		TMatrix m_nhits;	
		TH2F* h2D_mC;
		TH1F* h1D_mL;
		TH1F* h1D_mD;
		TH1F* h1D_massbias;
		TH1F* h1D_nhits;

		const DEventWriterROOT* dEventWriterROOT;
		const DEventWriterREST* dEventWriterREST;  

};

#endif // _DEventProcessor_BCAL_gainmatrix_

