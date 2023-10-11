// $Id$
//
//    File: DEventProcessor_FCAL_Shower.h
// Created: Fri Oct 10 16:41:18 EDT 2014
// Creator: adesh (on Linux ifarm1101 2.6.32-220.7.1.el6.x86_64 x86_64)
//

#ifndef _DEventProcessor_FCAL_Shower_
#define _DEventProcessor_FCAL_Shower_

#include <JANA/JEventProcessor.h>
#include <JANA/Compatibility/JLockService.h>

#include <ANALYSIS/DEventWriterROOT.h>
#include <HDDM/DEventWriterREST.h>
#include <ANALYSIS/DHistogramActions.h>
#include "ANALYSIS/DAnalysisUtilities.h"
#include "TRACKING/DTrackFinder.h"

#include "DLorentzVector.h"
#include "TMatrixD.h"


using namespace std;

class JEventProcessor_FCALgains : public JEventProcessor
{
	public:
		JEventProcessor_FCALgains(){
			SetTypeName("JEventProcessor_FCALgains");
		};
		~JEventProcessor_FCALgains(){};
		//DVector3 Calc_CrudeVertex(const deque< const DKinematicData* > & locParticles) const;
		
	       	
	private:
		//const DAnalysisUtilities* dAnalysisUtilities;
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& locEvent) override;
		void Process(const std::shared_ptr<const JEvent>& locEvent) override;
		void EndRun() override;
		void Finish() override;

		std::shared_ptr<JLockService> lockService;

		//jerror_t fillHists();
//double m_x;

//vector<vector<string> > ParseTSV(const char* s);
	 int XYtoAbsNum(int my_x, int my_y);
	 pair<int,int> AbsNumtoXY(int channel);

        DFCALGeometry *m_fcalgeom;
        DFCALGeometry* mygeom;




	// bool read_gains;

        int faredge;
        int beamline;

        int n_channels;

        int Meson2Optimise;
        int m_TotPastCuts;
        int m_event;
        int m_recon;
        int m_nmesons;
        int m_nElements;
        int *m_hits;
        int *m_channel;
        double *m_grad;
        double m_mesonmass;
        double m_pi0mass;
        double m_etamass;
        double scale1;
        double scale2;
        double z_diff;

        double scale_factors[10000];


        //PLUGIN PARAMETERS
       // string GAIN_FACTOR_PATH;
        double SCALE_FACTOR;
        //string outfile;
        bool GET_SCALE_FACTOR;
        double MASS_CUT_HI;
        double MASS_CUT_LO;
        int NHITS_CUT;

	 vector<double> gainfactors;
	 vector<int>    nhits_vec;

        TFile* m_rootFile;

        TMatrixD m_mC;
        TMatrixD m_mD;
        TMatrixD m_mL;
        TMatrixD m_mLt;
        TMatrixD m_mPi0;
        TMatrixD m_massDiff;
        TMatrixD m_nhits;
        double m_massbias;

        TH2F* h2D_mC;
        TH1F* h1D_mL;
        TH1F* h1D_mD;
        TH1F* h1D_massbias;
        TH1F* h1D_mPi0;
        TH1F* h1D_massDiff;
        TH1F* h1D_mPi0cuts;
        TH1F* h1D_nhits;
        TH1F* h1D_nhits_unordered;
        TH1F* h1D_mPi0_window;
	TH2F* hits2D;
	TH2F* hits2D_pi0;
	TH1F* h1D_ebyp;
		
};

#endif // _DEventProcessor_FCAL_Shower_

