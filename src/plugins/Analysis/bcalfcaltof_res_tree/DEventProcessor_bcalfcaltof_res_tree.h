// $Id$
//
//    File: DEventProcessor_bcalfcaltof_res_tree.h
// Created: Mon Apr  3 11:38:03 EDT 2006
// Creator: davidl (on Darwin swire-b241.jlab.org 8.4.0 powerpc)
//

#ifndef _DEventProcessor_bcalfcaltof_res_tree_
#define _DEventProcessor_bcalfcaltof_res_tree_

#include <JANA/JEventProcessor.h>

#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
#include <TF1.h>
#include <TTree.h>
#include <DVector3.h>
#include <particleType.h>

#include <DANA/DEvent.h>
#include <TRACKING/DMCThrown.h>
#include <BCAL/DBCALShower.h>
#include <FCAL/DFCALShower.h>
#include <TOF/DTOFPoint.h>
#include <TOF/DTOFTruth.h>
#include <TOF/DTOFHit.h>
#include <HDGEOMETRY/DRootGeom.h>
#include <BCALMCComparison.h>
#include <FCALMCComparison.h>
#include <TOFMCComparison.h>
#include <TRACKING/DTrackFitter.h>

class DEventProcessor_bcalfcaltof_res_tree:public JEventProcessor{
	public:
		DEventProcessor_bcalfcaltof_res_tree(){
			SetTypeName("DEventProcessor_bcalfcaltof_res_tree");
		};
		~DEventProcessor_bcalfcaltof_res_tree(){};

		void Convert_Coordinates_BCALToLab(float locBCALR, float locBCALPhi, float locBCALZ, DVector3& locLabVertex);
		void Convert_Coordinates_LabToBCAL(const DVector3& locLabVertex, float& locBCALR, float& locBCALPhi, float& locBCALZ);
		double Calc_MostProbableTOFdE(const DVector3 &locMomentum, double mass);
		void Calc_MostProbableTOFdEdx(double p, double mass, double dx, double &dE, double &dEdx);

		float Calc_BCALPathLengthCorrection(float locEnergy);
		float Calc_BCALPathLengthCorrectionZPostE(float locZ);
		float Calc_FCALPathLengthCorrection(float locEnergy);

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;

		const DRootGeom *dRootGeom;                                 
		bool dBCALStudyFlag;
		bool dFCALStudyFlag;
		bool dTOFStudyFlag;
		bool locSCStudyFlag;
		DTrackFitter *dTrackFitter;

		double dRhoZoverA;
		double dKRhoZoverA;
		double dLnI;

		BCALMCComparison *dBCALMCComparison;
		FCALMCComparison *dFCALMCComparison;
		TOFMCComparison *dTOFMCComparison;

		TTree* dPluginTree_BCALMCComparison;
		TTree* dPluginTree_FCALMCComparison;
		TTree* dPluginTree_TOFMCComparison;
};

#endif // _DEventProcessor_bcalfcaltof_res_tree_

