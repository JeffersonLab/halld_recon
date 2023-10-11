// $Id$
//
//    File: DNeutralParticleHypothesis_factory.h
// Created: Tue Aug  9 14:29:24 EST 2011
// Creator: pmatt (on Linux ifarml6 2.6.18-128.el5 x86_64)
//

#ifndef _DNeutralParticleHypothesis_factory_
#define _DNeutralParticleHypothesis_factory_

#include <limits>

#include <TMath.h>
#include <TMatrixFSym.h>

#include <JANA/JFactoryT.h>
#include <HDGEOMETRY/DGeometry.h>
#include <PID/DNeutralParticleHypothesis.h>
#include <PID/DNeutralShower.h>
#include <PID/DEventRFBunch.h>
#include <PID/DParticleID.h>
#include "PID/DVertex.h"
#include "DResourcePool.h"
#include <DVector3.h>
#include <DMatrix.h>

class DNeutralParticleHypothesis_factory : public JFactoryT<DNeutralParticleHypothesis>
{
	public:
		DNeutralParticleHypothesis* Create_DNeutralParticleHypothesis(const DNeutralShower* locNeutralShower, Particle_t locPID, const DEventRFBunch* locEventRFBunch, const DLorentzVector& dSpacetimeVertex, const TMatrixFSym* locVertexCovMatrix, bool locPerfomBetaCut=true);

		void Calc_ParticleCovariance_Photon(const DNeutralShower* locNeutralShower, const TMatrixFSym* locVertexCovMatrix, const DVector3& locMomentum, const DVector3& locPathVector, TMatrixFSym* locParticleCovariance) const;
		void Calc_ParticleCovariance_Massive(const DNeutralShower* locNeutralShower, const TMatrixFSym* locVertexCovMatrix, double locMass, double locDeltaT, const DVector3& locMomentum, const DVector3& locPathVector, TMatrixFSym* locParticleCovariance) const;

		void Recycle_Hypotheses(vector<DNeutralParticleHypothesis*>& locHypos){dResourcePool_NeutralParticleHypothesis->Recycle(locHypos);}
		void Recycle_Hypotheses(vector<const DNeutralParticleHypothesis*>& locHypos){dResourcePool_NeutralParticleHypothesis->Recycle(locHypos);}
		void Recycle_Hypothesis(const DNeutralParticleHypothesis* locHypo){dResourcePool_NeutralParticleHypothesis->Recycle(locHypo);}

		size_t Get_NumObjectsAllThreads(void) const{return dResourcePool_NeutralParticleHypothesis->Get_NumObjectsAllThreads();}
		DNeutralParticleHypothesis* Get_Resource(void)
		{
			auto locHypo = dResourcePool_NeutralParticleHypothesis->Get_Resource();
			return locHypo;
		}

	private:
		double dTargetCenterZ;
		const DParticleID* dParticleID = nullptr;

		//RESOURCE POOL
		vector<DNeutralParticleHypothesis*> dCreated;
		DResourcePool<DNeutralParticleHypothesis>* dResourcePool_NeutralParticleHypothesis = nullptr;
		shared_ptr<DResourcePool<TMatrixFSym>> dResourcePool_TMatrixFSym;

		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void Finish() override
		{
			for(auto locHypo : mData)
				Recycle_Hypothesis(locHypo);
			mData.clear();
			delete dResourcePool_NeutralParticleHypothesis;
		}
};

#endif // _DNeutralParticleHypothesis_factory_

