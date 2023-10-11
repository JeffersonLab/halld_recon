// $Id$
//
//    File: JEventProcessor_FCAL_invmass.h
// Created: Tue May 31 09:44:35 EDT 2016
// Creator: adesh (on Linux ifarm1101 2.6.32-431.el6.x86_64 x86_64)
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

class JEventProcessor_FCAL_invmass : public JEventProcessor
{
	public:
		JEventProcessor_FCAL_invmass(){
			SetTypeName("JEventProcessor_FCAL_invmass");
		};
		~JEventProcessor_FCAL_invmass(){};
		//DVector3 Calc_CrudeVertex(const deque< const DKinematicData* > & locParticles) const;


	private:
		//const DAnalysisUtilities* dAnalysisUtilities;
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
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

		double z_diff;
		double zTarget;
		float qualL;
		float qualH;
		float invM;
		float eL;
		float eH;		

		TMatrixD m_nhits;


};

#endif 
