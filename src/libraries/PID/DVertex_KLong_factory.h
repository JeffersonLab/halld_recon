// $Id$
//
//    File: DVertex_KLong_factory.h
// Created: Tue Apr  6 17:01:54 EDT 2010
// Creator: davidl (on Darwin Amelia.local 9.8.0 i386)
//

#ifndef _DVertex_KLong_factory_
#define _DVertex_KLong_factory_

#include <JANA/JFactoryT.h>

#include "TVector3.h"

#include "HDGEOMETRY/DMagneticFieldMap.h"
#include "DANA/DApplication.h"
#include "DVector3.h"

#include "PID/DVertex.h"
#include "PID/DParticleID.h"
#include "PID/DDetectorMatches.h"
#include "TRACKING/DTrackTimeBased.h"

#include "ANALYSIS/DAnalysisUtilities.h"
#include "KINFITTER/DKinFitter.h"
#include "ANALYSIS/DKinFitUtils_GlueX.h"

using namespace std;
using namespace jana;

class DVertex_KLong_factory : public JFactoryT<DVertex>
{
	public:
		DVertex_KLong_factory(void) { 
			SetTag("KLong"); 
		}

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override; 
		void Process(const std::shared_ptr<const JEvent>& event) override; 
// 		void EndRun() override; 
// 		void Finish() override; 

		jerror_t Create_Vertex_NoTracks();
		jerror_t Create_Vertex_OneTrack(const DTrackTimeBased* locTrackTimeBased);
		jerror_t Create_Vertex_Rough(DVector3 locPosition, double locTime);
		jerror_t Create_Vertex_KinFit(vector<const DKinematicData*> &locKinematicDataVector);

		void Set_TrackTime(const std::shared_ptr<const JEvent>& event, DTrackTimeBased* locTrackTimeBased, const DTrackTimeBased* locTrackTimeBased_ToMatch, const DDetectorMatches* locDetectorMatches, const DParticleID* locPIDAlgorithm);
		//double Calc_CrudeVertexTime(const vector<DKinFitParticle*>& locParticles, const DVector3& locCommonVertex) const;

		const DAnalysisUtilities* dAnalysisUtilities;
		DKinFitter* dKinFitter;
		DKinFitUtils_GlueX* dKinFitUtils;

		int dKinFitDebugLevel;
		bool dNoKinematicFitFlag;
		bool dForceTargetCenter;
		bool dUseWeightedAverage;
		bool dUseStartCounterTimesOnly;
		
		double dTargetZCenter;
		double dTargetLength;
		double dTargetRadius;
		double dMinTrackingFOM;
		int dMinTrackingNDF;
		double m_beamSpotX;
		double m_beamSpotY;
};

#endif // _DVertex_KLong_factory_

