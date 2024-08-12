// $Id$
//
//    File: DNeutralParticleHypothesis_factory.cc
// Created: Thu Dec  3 17:27:55 EST 2009
// Creator: pmatt (on Linux ifarml6 2.6.18-128.el5 x86_64)
//


#include <iostream>
#include <iomanip>
using namespace std;

#include <TMath.h>

#include "DNeutralParticleHypothesis_factory.h"


//------------------
// Init
//------------------
void DNeutralParticleHypothesis_factory::Init()
{
	auto app = GetApplication();
	//Setting this flag makes it so that JANA does not delete the objects in _data.  This factory will manage this memory. 
	SetFactoryFlag(NOT_OBJECT_OWNER);
	dResourcePool_NeutralParticleHypothesis = new DResourcePool<DNeutralParticleHypothesis>();
	dResourcePool_NeutralParticleHypothesis->Set_ControlParams(50, 20, 400, 4000, 0);
	dResourcePool_TMatrixFSym = std::make_shared<DResourcePool<TMatrixFSym>>();
	dResourcePool_TMatrixFSym->Set_ControlParams(50, 20, 1000, 15000, 0);
}

//------------------
// BeginRun
//------------------
void DNeutralParticleHypothesis_factory::BeginRun(const std::shared_ptr<const JEvent>& event)
{
	auto runnumber = event->GetRunNumber();
	auto app = event->GetJApplication();
	auto geo_manager = app->GetService<DGeometryManager>();
	auto locGeometry = geo_manager->GetDGeometry(runnumber);

	// Get Target parameters from XML
	locGeometry->GetTargetZ(dTargetCenterZ);
	event->GetSingle(dParticleID);
}

//------------------
// Process
//------------------
void DNeutralParticleHypothesis_factory::Process(const std::shared_ptr<const JEvent>& event)
{
	dResourcePool_NeutralParticleHypothesis->Recycle(dCreated);
	dCreated.clear();
	mData.clear();

	vector<const DNeutralShower*> locNeutralShowers;
	event->Get(locNeutralShowers);

	vector<Particle_t> locPIDHypotheses;
	locPIDHypotheses.push_back(Gamma);
	locPIDHypotheses.push_back(Neutron);

	const DEventRFBunch* locEventRFBunch = NULL;
	event->GetSingle(locEventRFBunch);

	const DVertex* locVertex = NULL;
	event->GetSingle(locVertex);

	// Loop over DNeutralShowers
	for(size_t loc_i = 0; loc_i < locNeutralShowers.size(); ++loc_i)
	{
		const DNeutralShower *locNeutralShower = locNeutralShowers[loc_i];
		// Loop over vertices and PID hypotheses & create DNeutralParticleHypotheses for each combination
		for(size_t loc_j = 0; loc_j < locPIDHypotheses.size(); ++loc_j)
		{
			DNeutralParticleHypothesis* locNeutralParticleHypothesis = Create_DNeutralParticleHypothesis(locNeutralShower, locPIDHypotheses[loc_j], locEventRFBunch, locVertex->dSpacetimeVertex, &locVertex->dCovarianceMatrix);
			if(locNeutralParticleHypothesis != nullptr)
				Insert(locNeutralParticleHypothesis);
		}
	}

	dCreated = mData;
}

DNeutralParticleHypothesis* DNeutralParticleHypothesis_factory::Create_DNeutralParticleHypothesis(const DNeutralShower* locNeutralShower, Particle_t locPID, const DEventRFBunch* locEventRFBunch, const DLorentzVector& dSpacetimeVertex, const TMatrixFSym* locVertexCovMatrix, bool locPerfomBetaCut)
{
	DVector3 locVertexGuess = dSpacetimeVertex.Vect();
	double locStartTime = dSpacetimeVertex.T();

	double locHitTime = locNeutralShower->dSpacetimeVertex.T();
	DVector3 locHitPoint = locNeutralShower->dSpacetimeVertex.Vect();

	// Calculate DNeutralParticleHypothesis Quantities (projected time at vertex for given id, etc.)
	double locMass = ParticleMass(locPID);

	DVector3 locPath = locHitPoint - locVertexGuess;
	double locPathLength = locPath.Mag();
	if(!(locPathLength > 0.0))
		return nullptr; //invalid, will divide by zero when creating error matrix, so skip!

	DVector3 locMomentum(locPath);
	shared_ptr<TMatrixFSym> locParticleCovariance = (locVertexCovMatrix != nullptr) ? dResourcePool_TMatrixFSym->Get_SharedResource() : nullptr;
	if(locParticleCovariance != nullptr)
		locParticleCovariance->ResizeTo(7, 7);

	double locProjectedTime = 0.0, locPMag = 0.0;
	if(locPID != Gamma)
	{
		double locDeltaT = locHitTime - locStartTime;
		double locBeta = locPathLength/(locDeltaT*29.9792458);
		
		if(locPerfomBetaCut) {
			// Make sure that the beta is physically meaningful for this 
			// mass hypothesis
			if (locBeta>=1.0 || locBeta<0) return nullptr;
		}
		
		double locGamma = 1.0/sqrt(1.0 - locBeta*locBeta);
		locPMag = locGamma*locBeta*locMass;
		locMomentum.SetMag(locPMag);
		
//cout << "DNeutralParticleHypothesis_factory: pid, mass, shower-z, vertex-z, path, shower t, rf t, delta-t, beta, pmag = " << locPID << ", " << locMass << ", " << locHitPoint.Z() << ", " << locVertexGuess.Z() << ", " << locPathLength << ", " << locHitTime << ", " << locStartTime << ", " << locDeltaT << ", " << locBeta << ", " << locPMag << endl;

		locProjectedTime = locStartTime + (locVertexGuess.Z() - dTargetCenterZ)/29.9792458;
		if(locVertexCovMatrix != nullptr)
			Calc_ParticleCovariance_Massive(locNeutralShower, locVertexCovMatrix, locMass, locDeltaT, locMomentum, locPath, locParticleCovariance.get());
	}
	else
	{
		locPMag = locNeutralShower->dEnergy;
		double locFlightTime = locPathLength/29.9792458;
		locProjectedTime = locHitTime - locFlightTime;
		locMomentum.SetMag(locPMag);
		if(locVertexCovMatrix != nullptr)
			Calc_ParticleCovariance_Photon(locNeutralShower, locVertexCovMatrix, locMomentum, locPath, locParticleCovariance.get());
	}

	// Build DNeutralParticleHypothesis // dEdx not set
	DNeutralParticleHypothesis* locNeutralParticleHypothesis = Get_Resource();
	locNeutralParticleHypothesis->Set_NeutralShower(locNeutralShower);
	locNeutralParticleHypothesis->setPID(locPID);
	locNeutralParticleHypothesis->setMomentum(locMomentum);
	locNeutralParticleHypothesis->setPosition(locVertexGuess);
	locNeutralParticleHypothesis->setTime(locProjectedTime);
	locNeutralParticleHypothesis->Set_T0(locStartTime, sqrt(locEventRFBunch->dTimeVariance), locEventRFBunch->dTimeSource);
	locNeutralParticleHypothesis->setErrorMatrix(locParticleCovariance);

	// Calculate DNeutralParticleHypothesis FOM
	unsigned int locNDF = 0;
	double locChiSq = 0.0;
	double locFOM = -1.0; //undefined for non-photons
	if(locPID == Gamma)
	{
		double locTimePull = 0.0;
		locChiSq = dParticleID->Calc_TimingChiSq(locNeutralParticleHypothesis, locNDF, locTimePull);
		locFOM = TMath::Prob(locChiSq, locNDF);
	}
	locNeutralParticleHypothesis->Set_ChiSq_Overall(locChiSq, locNDF, locFOM);
	return locNeutralParticleHypothesis;
}

void DNeutralParticleHypothesis_factory::Calc_ParticleCovariance_Photon(const DNeutralShower* locNeutralShower, const TMatrixFSym* locVertexCovMatrix, const DVector3& locMomentum, const DVector3& locPathVector, TMatrixFSym* locParticleCovariance) const
{
	//build 8x8 matrix: 5x5 shower, 3x3 vertex position
	TMatrixFSym locShowerPlusVertCovariance(8);
	for(unsigned int loc_l = 0; loc_l < 5; ++loc_l) //shower: e, x, y, z, t
	{
		for(unsigned int loc_m = 0; loc_m < 5; ++loc_m)
			locShowerPlusVertCovariance(loc_l, loc_m) = (*(locNeutralShower->dCovarianceMatrix))(loc_l, loc_m);
	}
	for(unsigned int loc_l = 0; loc_l < 3; ++loc_l) //vertex xyz
	{
		for(unsigned int loc_m = 0; loc_m < 3; ++loc_m)
			locShowerPlusVertCovariance(5 + loc_l, 5 + loc_m) = (*locVertexCovMatrix)(loc_l, loc_m);
	}

	//the input delta X is defined as "hit - start"
	//however, the documentation and derivations define delta x as "start - hit"
	//so, reverse the sign of the inputs to match the documentation
	//and then the rest will follow the documentation
	DVector3 locDeltaX = -1.0*locPathVector;
	DVector3 locDeltaXOverDeltaXSq = (1.0/locDeltaX.Mag2())*locDeltaX;
	DVector3 locUnitP = (1.0/locNeutralShower->dEnergy)*locMomentum;
	DVector3 locUnitDeltaXOverC = (1.0/(29.9792458*locDeltaX.Mag()))*locDeltaX;

	//build transform matrix
	TMatrix locTransformMatrix(7, 8);

	locTransformMatrix(0, 0) = locUnitP.X(); //partial deriv of px wrst shower-e
	locTransformMatrix(0, 1) = locMomentum.Px()*(locDeltaXOverDeltaXSq.X() - 1.0/locDeltaX.X()); //partial deriv of px wrst shower-x
	locTransformMatrix(0, 2) = locMomentum.Px()*locDeltaX.Y()/locDeltaX.Mag2(); //partial deriv of px wrst shower-y
	locTransformMatrix(0, 3) = locMomentum.Px()*locDeltaX.Z()/locDeltaX.Mag2(); //partial deriv of px wrst shower-z
	locTransformMatrix(0, 5) = -1.0*locTransformMatrix(0, 1); //partial deriv of px wrst vert-x
	locTransformMatrix(0, 6) = -1.0*locTransformMatrix(0, 2); //partial deriv of px wrst vert-y
	locTransformMatrix(0, 7) = -1.0*locTransformMatrix(0, 3); //partial deriv of px wrst vert-z

	locTransformMatrix(1, 0) = locUnitP.Y(); //partial deriv of py wrst shower-e
	locTransformMatrix(1, 1) = locMomentum.Py()*locDeltaX.X()/locDeltaX.Mag2(); //partial deriv of py wrst shower-x
	locTransformMatrix(1, 2) = locMomentum.Py()*(locDeltaXOverDeltaXSq.Y() - 1.0/locDeltaX.Y()); //partial deriv of py wrst shower-y
	locTransformMatrix(1, 3) = locMomentum.Py()*locDeltaX.Z()/locDeltaX.Mag2(); //partial deriv of py wrst shower-z
	locTransformMatrix(1, 5) = -1.0*locTransformMatrix(1, 1); //partial deriv of py wrst vert-x
	locTransformMatrix(1, 6) = -1.0*locTransformMatrix(1, 2); //partial deriv of py wrst vert-y
	locTransformMatrix(1, 7) = -1.0*locTransformMatrix(1, 3); //partial deriv of py wrst vert-z

	locTransformMatrix(2, 0) = locUnitP.Z(); //partial deriv of pz wrst shower-e
	locTransformMatrix(2, 1) = locMomentum.Pz()*locDeltaX.X()/locDeltaX.Mag2(); //partial deriv of pz wrst shower-x
	locTransformMatrix(2, 2) = locMomentum.Pz()*locDeltaX.Y()/locDeltaX.Mag2(); //partial deriv of pz wrst shower-y
	locTransformMatrix(2, 3) = locMomentum.Pz()*(locDeltaXOverDeltaXSq.Z() - 1.0/locDeltaX.Z()); //partial deriv of pz wrst shower-z
	locTransformMatrix(2, 5) = -1.0*locTransformMatrix(2, 1); //partial deriv of pz wrst vert-x
	locTransformMatrix(2, 6) = -1.0*locTransformMatrix(2, 2); //partial deriv of pz wrst vert-y
	locTransformMatrix(2, 7) = -1.0*locTransformMatrix(2, 3); //partial deriv of pz wrst vert-z

	locTransformMatrix(3, 5) = 1.0; //partial deriv of x wrst vertex-x
	locTransformMatrix(4, 6) = 1.0; //partial deriv of y wrst vertex-y
	locTransformMatrix(5, 7) = 1.0; //partial deriv of z wrst vertex-z

	locTransformMatrix(6, 0) = 0.0; //partial deriv of t wrst shower-e //beta defined = 1
	locTransformMatrix(6, 1) = locUnitDeltaXOverC.X(); //partial deriv of t wrst shower-x
	locTransformMatrix(6, 2) = locUnitDeltaXOverC.Y(); //partial deriv of t wrst shower-y
	locTransformMatrix(6, 3) = locUnitDeltaXOverC.Z(); //partial deriv of t wrst shower-z
	locTransformMatrix(6, 4) = 1.0; //partial deriv of t wrst shower-t
	locTransformMatrix(6, 5) = -1.0*locTransformMatrix(6, 1); //partial deriv of t wrst vert-x
	locTransformMatrix(6, 6) = -1.0*locTransformMatrix(6, 2); //partial deriv of t wrst vert-y
	locTransformMatrix(6, 7) = -1.0*locTransformMatrix(6, 3); //partial deriv of t wrst vert-z

	//convert
	*locParticleCovariance = locShowerPlusVertCovariance.Similarity(locTransformMatrix);
}

void DNeutralParticleHypothesis_factory::Calc_ParticleCovariance_Massive(const DNeutralShower* locNeutralShower, const TMatrixFSym* locVertexCovMatrix, double locMass, double locDeltaT, const DVector3& locMomentum, const DVector3& locPathVector, TMatrixFSym* locParticleCovariance) const
{
  double ct=locDeltaT*SPEED_OF_LIGHT;
  double x=locPathVector.x();
  double y=locPathVector.y();
  double z=locPathVector.z();
  double s=locPathVector.Mag();
  double fac=1./sqrt(ct*ct-s*s);
  double fac2=fac*fac;
  double fac3=fac2*fac;
  double dpx_dx=locMass*fac*(1.+x*x*fac2);
  double dpx_dy=locMass*x*y*fac3;
  double dpx_dz=locMass*x*z*fac3;
  double dpx_dt=-locMass*fac3*x*ct*SPEED_OF_LIGHT;        
  double dpy_dx=dpx_dy;
  double dpy_dz=locMass*z*y*fac3;
  double dpy_dy=locMass*fac*(1.+y*y*fac2);
  double dpy_dt=-locMass*fac3*y*ct*SPEED_OF_LIGHT;
  double dpz_dx=dpx_dz;
  double dpz_dy=dpy_dz;
  double dpz_dz=locMass*fac*(1.+z*z*fac2);
  double dpz_dt=-locMass*fac3*z*ct*SPEED_OF_LIGHT;
  double varx=25.,vary=25.,varz=25.,vart=0.09;

  locParticleCovariance->Zero();
  locParticleCovariance->operator()(0,0)=dpx_dx*dpx_dx*varx
    +dpx_dy*dpx_dy*vary+dpx_dz*dpx_dz*varz
    +dpx_dt*dpx_dt*vart;
  locParticleCovariance->operator()(1,1)=dpy_dx*dpy_dx*varx
    +dpy_dy*dpy_dy*vary+dpy_dz*dpy_dz*varz
    +dpy_dt*dpy_dt*vart;  
  locParticleCovariance->operator()(2,2)=dpz_dx*dpz_dx*varx
    +dpz_dy*dpz_dy*vary+dpz_dz*dpz_dz*varz
    +dpz_dt*dpz_dt*vart;

  for(unsigned int loc_l = 0; loc_l < 3; ++loc_l){ //vertex xyz
    for(unsigned int loc_m = 0; loc_m < 3; ++loc_m)
      locParticleCovariance->operator()(3 + loc_l, 3 + loc_m) = (*locVertexCovMatrix)(loc_l, loc_m);
  }
  locParticleCovariance->operator()(6,6)=vart;
}
