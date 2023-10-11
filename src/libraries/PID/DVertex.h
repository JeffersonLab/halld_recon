// $Id$
//
//    File: DVertex.h
// Created: Tue Apr  6 17:01:54 EDT 2010
// Creator: davidl (on Darwin Amelia.local 9.8.0 i386)
//

#ifndef _DVertex_
#define _DVertex_

#include <map>

#include <JANA/JObject.h>

#include <DLorentzVector.h>

#include "PID/DKinematicData.h"
#include "KINFITTER/DKinFitParticle.h"

using namespace std;

class DVertex: public JObject
{
	public:    
		JOBJECT_PUBLIC(DVertex);

		DLorentzVector dSpacetimeVertex; // vertex position in cm + vertex time in ns
		TMatrixFSym dCovarianceMatrix; //xyzt order

		unsigned int dKinFitNDF;
		double dKinFitChiSq;
		map<const JObject*, map<DKinFitPullType, double> > dKinFitPulls;

		// Objects used to calculate this added as Associated Objects
		void Summarize(JObjectSummary& summary) const override {
		    summary.add(dSpacetimeVertex.X(), "x", "%3.2f");
            summary.add(dSpacetimeVertex.Y(), "y", "%3.2f");
            summary.add(dSpacetimeVertex.Z(), "z", "%3.2f");
            summary.add(dSpacetimeVertex.T(), "t", "%3.2f");
		}
};

#endif // _DVertex_

