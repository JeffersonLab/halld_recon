// $Id$
//
//    File: DVertex_factory.h
// Created: Tue Apr  6 17:01:54 EDT 2010
// Creator: davidl (on Darwin Amelia.local 9.8.0 i386)
//

#ifndef _DVertex_factory_
#define _DVertex_factory_

#include <JANA/JFactoryT.h>

#include "TVector3.h"

#include "HDGEOMETRY/DMagneticFieldMap.h"
#include "DVector3.h"

#include "PID/DVertex.h"
#include "PID/DDetectorMatches.h"
#include "PID/DEventRFBunch.h"
#include "TRACKING/DTrackTimeBased.h"

#include "ANALYSIS/DAnalysisUtilities.h"
#include "KINFITTER/DKinFitter.h"
#include "ANALYSIS/DKinFitUtils_GlueX.h"

using namespace std;


class DVertex_factory : public JFactoryT<DVertex>
{
	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;

		jerror_t Create_Vertex_NoTracks(const DEventRFBunch* locEventRFBunch);
		jerror_t Create_Vertex_OneTrack(const DTrackTimeBased* locTrackTimeBased, const DEventRFBunch* locEventRFBunch);
		jerror_t Create_Vertex_Rough(DVector3 locPosition, const DEventRFBunch* locEventRFBunch);
		jerror_t Create_Vertex_KinFit(const DEventRFBunch* locEventRFBunch);

		const DAnalysisUtilities* dAnalysisUtilities;
		DKinFitter* dKinFitter;
		DKinFitUtils_GlueX* dKinFitUtils;

		int dKinFitDebugLevel;
		bool dNoKinematicFitFlag;
		bool dForceTargetCenter;
		double dTargetZCenter;
		double dTargetLength;
		double dTargetRadius;
		double dMinTrackingFOM;
		double m_beamSpotX;
		double m_beamSpotY;
};

#endif // _DVertex_factory_

