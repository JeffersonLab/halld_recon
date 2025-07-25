#ifndef _DChargedTrackHypothesis_
#define _DChargedTrackHypothesis_

#include <vector>
#include <string>
#include <memory>
#include <limits>

#include "DResettable.h"
#include <PID/DKinematicData.h>
#include "TRACKING/DTrackTimeBased.h"
#include <PID/DDetectorMatches.h>

using namespace std;

class DChargedTrackHypothesis : public DKinematicData
{
	public:
		JOBJECT_PUBLIC(DChargedTrackHypothesis);

		//CONSTRUCTORS & OPERATORS
		DChargedTrackHypothesis(void);
		DChargedTrackHypothesis(const DChargedTrackHypothesis& locSourceData, bool locShareTrackingFlag = false,
				bool locShareTimingFlag = false, 
					bool locShareEOverPFlg=false, bool locShareKinematicsFlag = false);
		DChargedTrackHypothesis(const DTrackTimeBased* locSourceData);
		DChargedTrackHypothesis& operator=(const DChargedTrackHypothesis& locSourceData);

		void Reset(void) override;
		void Release(void) override;

		//SHARE RESOURCES
		void Share_FromInput(const DChargedTrackHypothesis* locSourceData, bool locShareTrackingFlag, bool locShareTimingFlag, bool locShareEOverPFlag, bool locShareKinematicsFlag);

		//GETTERS

		//Tracking
		unsigned int Get_NDF_DCdEdx(void) const{return dTrackingInfo->dNDF_DCdEdx;}
		double Get_ChiSq_DCdEdx(void) const{return dTrackingInfo->dChiSq_DCdEdx;}
		unsigned int Get_NDF_EoverP(void) const{return dEOverPInfo->dNDF_EoverP;}
		double Get_ChiSq_EoverP(void) const{return dEOverPInfo->dChiSq_EoverP;}
		double Get_dEdx_CDC_amp(void) const{return dTrackingInfo->ddEdx_CDC_amp;}
		double Get_dEdx_CDC_int(void) const{return dTrackingInfo->ddEdx_CDC_int;}
		const DTrackTimeBased* Get_TrackTimeBased(void) const{return dTrackingInfo->dTrackTimeBased;}

		//Timing
		double t0(void) const{return dTimingInfo->dt0;}
		double t0_err(void) const{return dTimingInfo->dt0_err;}
		double t1(void) const;
		double t1_err(void) const;
		DetectorSystem_t t0_detector(void) const{return dTimingInfo->dt0_detector;}
		DetectorSystem_t t1_detector(void) const;
		double Get_TimeAtPOCAToVertex(void) const{return dTimingInfo->dTimeAtPOCAToVertex;}
		unsigned int Get_NDF_Timing(void) const{return dTimingInfo->dNDF_Timing;}
		double Get_ChiSq_Timing(void) const{return dTimingInfo->dChiSq_Timing;}
		double Get_PathLength(void) const;
		double measuredBeta(void) const{return Get_PathLength()/(29.9792458*(t1() - t0()));}

		//totals for overall PID determination
		unsigned int Get_NDF(void) const{return dTimingInfo->dNDF;}
		double Get_ChiSq(void) const{return dTimingInfo->dChiSq;}
		double Get_FOM(void) const{return dTimingInfo->dFOM;}

		//Match params (return nullptr if no match)
		shared_ptr<const DSCHitMatchParams> Get_SCHitMatchParams(void) const{return dTrackingInfo->dSCHitMatchParams;}
		shared_ptr<const DTOFHitMatchParams> Get_TOFHitMatchParams(void) const{return dTrackingInfo->dTOFHitMatchParams;}
		shared_ptr<const DBCALShowerMatchParams> Get_BCALShowerMatchParams(void) const{return dTrackingInfo->dBCALShowerMatchParams;}
		shared_ptr<const DFCALShowerMatchParams> Get_FCALShowerMatchParams(void) const{return dTrackingInfo->dFCALShowerMatchParams;}
		shared_ptr<const DFCALSingleHitMatchParams> Get_FCALSingleHitMatchParams(void) const{return dTrackingInfo->dFCALSingleHitMatchParams;}
		shared_ptr<const DDIRCMatchParams> Get_DIRCMatchParams(void) const{return dTrackingInfo->dDIRCMatchParams;}
		shared_ptr<const DCTOFHitMatchParams> Get_CTOFHitMatchParams(void) const{return dTrackingInfo->dCTOFHitMatchParams;}
		shared_ptr<const DFMWPCMatchParams> Get_FMWPCMatchParams(void) const{return dTrackingInfo->dFMWPCMatchParams;}
  		shared_ptr<const DTRDMatchParams> Get_TRDMatchParams(void) const{return dTrackingInfo->dTRDMatchParams;}
  		shared_ptr<const DECALShowerMatchParams> Get_ECALShowerMatchParams(void) const{return dTrackingInfo->dECALShowerMatchParams;}
  shared_ptr<const DECALSingleHitMatchParams> Get_ECALSingleHitMatchParams(void) const{return dTrackingInfo->dECALSingleHitMatchParams;}

		//SETTERS

		//Timing
		void Set_T0(double locT0, double locT0Error, DetectorSystem_t locT0Detector);
		void Set_TimeAtPOCAToVertex(double locTimeAtPOCAToVertex){dTimingInfo->dTimeAtPOCAToVertex = locTimeAtPOCAToVertex;}
		void Set_ChiSq_Timing(double locChiSq, unsigned int locNDF);
		void Set_ChiSq_Overall(double locChiSq, unsigned int locNDF, double locFOM);

		//Tracking
		void Set_TrackTimeBased(const DTrackTimeBased* locTrackTimeBased){dTrackingInfo->dTrackTimeBased = locTrackTimeBased;}
		void Set_ChiSq_DCdEdx(double locChiSq, unsigned int locNDF);
		void Set_ChiSq_EOverP(DetectorSystem_t detector,double locChiSq, unsigned int locNDF);
		void Set_dEdx_CDC_amp(double locCDCdEdx_amp);
		void Set_dEdx_CDC_int(double locCDCdEdx_int);

		//Match params
		void Set_SCHitMatchParams(shared_ptr<const DSCHitMatchParams> locMatchParams){dTrackingInfo->dSCHitMatchParams = locMatchParams;}
		void Set_TOFHitMatchParams(shared_ptr<const DTOFHitMatchParams> locMatchParams){dTrackingInfo->dTOFHitMatchParams = locMatchParams;}
		void Set_BCALShowerMatchParams(shared_ptr<const DBCALShowerMatchParams> locMatchParams){dTrackingInfo->dBCALShowerMatchParams = locMatchParams;}
		void Set_FCALShowerMatchParams(shared_ptr<const DFCALShowerMatchParams> locMatchParams){dTrackingInfo->dFCALShowerMatchParams = locMatchParams;}
		void Set_FCALSingleHitMatchParams(shared_ptr<const DFCALSingleHitMatchParams> locMatchParams){dTrackingInfo->dFCALSingleHitMatchParams = locMatchParams;}
		void Set_DIRCMatchParams(shared_ptr<const DDIRCMatchParams> locMatchParams){dTrackingInfo->dDIRCMatchParams = locMatchParams;}	
		void Set_CTOFHitMatchParams(shared_ptr<const DCTOFHitMatchParams> locMatchParams){dTrackingInfo->dCTOFHitMatchParams = locMatchParams;}
		void Set_FMWPCMatchParams(shared_ptr<const DFMWPCMatchParams> locMatchParams){dTrackingInfo->dFMWPCMatchParams = locMatchParams;}
  		void Set_TRDMatchParams(shared_ptr<const DTRDMatchParams> locMatchParams){dTrackingInfo->dTRDMatchParams = locMatchParams;}
  		void Set_ECALShowerMatchParams(shared_ptr<const DECALShowerMatchParams> locMatchParams){dTrackingInfo->dECALShowerMatchParams = locMatchParams;}
  void Set_ECALSingleHitMatchParams(shared_ptr<const DECALSingleHitMatchParams> locMatchParams){dTrackingInfo->dECALSingleHitMatchParams = locMatchParams;}
  
		void Summarize(JObjectSummary& summary) const override
		{
			summary.add(dTrackingInfo->dTrackTimeBased->candidateid, "candidate", "%d");
			DKinematicData::Summarize(summary);
			summary.add(dTrackingInfo->dTrackTimeBased->chisq, "Track_ChiSq", "%f");
			summary.add(dTrackingInfo->dChiSq_DCdEdx, "dEdx_ChiSq", "%f");
			summary.add(dTimingInfo->dChiSq_Timing, "TOF_ChiSq", "%f");
			summary.add(dEOverPInfo->dChiSq_EoverP, "EOverP_ChiSq", "%f");
			summary.add(dTimingInfo->dChiSq, "PID_ChiSq", "%f");
			summary.add(dTimingInfo->dFOM, "PID_FOM", "%f");
		}

		class DTimingInfo : public DResettable
		{
			public:
				void Reset(void);
				void Release(void){};

				//t0 is RF time at track poca to common vertex
				double dt0 = 0.0;
				double dt0_err = 0.0;
				DetectorSystem_t dt0_detector = SYS_NULL;

				unsigned int dNDF_Timing = 0;
				double dChiSq_Timing = 0.0;

				//technically, these can depend on the tracking chisq also, but no one in their right mind would change the tracking dE/dx info
				unsigned int dNDF = 0; //total NDF used for PID determination
				double dChiSq = 0.0; //total chi-squared used for PID determination
				double dFOM = 0.0; //overall FOM for PID determination

				//problem: how to store timing information accurately?

				//at the comboing stage, we don't want to evaluate the timing at the track POCA to the beamline, because that can be inaccurate
				//e.g. for very-low-theta tracks this position is not well defined
				//instead, we want to evaluate it at the POCA to the reconstructed vertex position on a combo-by-combo basis

				//however, to save memory, we want to share the kinematics (including time!) with the original (non-combo) hypothesis:
					//both are valid points on the track, and we don't want to recompute the covariance matrix (TONS of memory needed), etc.
				//So, that means we need to store the time at the poca to the vertex separately
				double dTimeAtPOCAToVertex = 0.0;
		};

		class DEOverPInfo:public DResettable{
		public:
		  void Reset(void);
		  void Release(void){};
		  
		  DetectorSystem_t dcal_detector = SYS_NULL;

		  unsigned int dNDF_EoverP = 0;
		  double dChiSq_EoverP = 0.0;
		};

		class DTrackingInfo : public DResettable
		{
			public:
				void Reset(void);
				void Release(void){Reset();};

				unsigned int dNDF_DCdEdx = 0;
				double dChiSq_DCdEdx = 0.0;
				unsigned int dNDF_EoverP=0;
				double dChiSq_EoverP=0.0;

				double ddEdx_CDC_amp=0.0;
				double ddEdx_CDC_int=0.0;

				const DTrackTimeBased* dTrackTimeBased = nullptr; //can get candidateid from here

				shared_ptr<const DSCHitMatchParams> dSCHitMatchParams = nullptr;
				shared_ptr<const DTOFHitMatchParams> dTOFHitMatchParams = nullptr;
				shared_ptr<const DBCALShowerMatchParams> dBCALShowerMatchParams = nullptr;
				shared_ptr<const DFCALShowerMatchParams> dFCALShowerMatchParams = nullptr;
				shared_ptr<const DFCALSingleHitMatchParams> dFCALSingleHitMatchParams = nullptr;
				shared_ptr<const DDIRCMatchParams> dDIRCMatchParams = nullptr;	
				shared_ptr<const DCTOFHitMatchParams> dCTOFHitMatchParams = nullptr;
				shared_ptr<const DFMWPCMatchParams> dFMWPCMatchParams=nullptr;
		  		shared_ptr<const DTRDMatchParams> dTRDMatchParams=nullptr;
		  		shared_ptr<const DECALShowerMatchParams> dECALShowerMatchParams = nullptr;
		  shared_ptr<const DECALSingleHitMatchParams> dECALSingleHitMatchParams = nullptr;
		};

	private:

		//memory of object in shared_ptr is managed automatically: deleted automatically when no references are left
		shared_ptr<DTimingInfo> dTimingInfo = nullptr;
		shared_ptr<DTrackingInfo> dTrackingInfo = nullptr;
		shared_ptr<DEOverPInfo> dEOverPInfo = nullptr;

		//RESOURCE POOLS
		static thread_local shared_ptr<DResourcePool<DTimingInfo>> dResourcePool_TimingInfo;
		static thread_local shared_ptr<DResourcePool<DTrackingInfo>> dResourcePool_TrackingInfo;	
		static thread_local shared_ptr<DResourcePool<DEOverPInfo>> dResourcePool_EOverPInfo;
};

/************************************************************** CONSTRUCTORS & OPERATORS ***************************************************************/

inline DChargedTrackHypothesis::DChargedTrackHypothesis(void) :
			       dTimingInfo(dResourcePool_TimingInfo->Get_SharedResource()), 
			       dTrackingInfo(dResourcePool_TrackingInfo->Get_SharedResource()),
			       dEOverPInfo(dResourcePool_EOverPInfo->Get_SharedResource())	       
{}

inline DChargedTrackHypothesis::DChargedTrackHypothesis(const DChargedTrackHypothesis& locSourceData, bool locShareTrackingFlag,
							bool locShareTimingFlag, bool locShareEOverPFlag, bool locShareKinematicsFlag) : DKinematicData(locSourceData, locShareKinematicsFlag)
{
	//Default is NOT to share: create a new, independent copy of the input data (tracked separately from input so it can be modified)
	if(locShareTrackingFlag)
		dTrackingInfo = locSourceData.dTrackingInfo;
	else
	{
		dTrackingInfo = dResourcePool_TrackingInfo->Get_SharedResource();
		*dTrackingInfo = *(locSourceData.dTrackingInfo);
	}

	if(locShareTimingFlag)
		dTimingInfo = locSourceData.dTimingInfo;
	else
	{
		dTimingInfo = dResourcePool_TimingInfo->Get_SharedResource();
		*dTimingInfo = *(locSourceData.dTimingInfo);
	}
	if(locShareEOverPFlag)
		dEOverPInfo = locSourceData.dEOverPInfo;
	else
	{
		dEOverPInfo = dResourcePool_EOverPInfo->Get_SharedResource();
		*dEOverPInfo = *(locSourceData.dEOverPInfo);
	}
}

inline DChargedTrackHypothesis::DChargedTrackHypothesis(const DTrackTimeBased* locSourceData) :
		DKinematicData(*static_cast<const DKinematicData*>(locSourceData), true)
{
	//Default is TO share kinematic data
	dTrackingInfo = dResourcePool_TrackingInfo->Get_SharedResource();
	dTimingInfo = dResourcePool_TimingInfo->Get_SharedResource();
	dEOverPInfo = dResourcePool_EOverPInfo->Get_SharedResource();
	dTrackingInfo->dTrackTimeBased = locSourceData;
}

inline DChargedTrackHypothesis& DChargedTrackHypothesis::operator=(const DChargedTrackHypothesis& locSourceData)
{
	//Replace current data with a new, independent copy of the input data: tracked separately from input so it can be modified
	DKinematicData::operator=(locSourceData);
	if((dTimingInfo == locSourceData.dTimingInfo) 
	   && (dTrackingInfo == locSourceData.dTrackingInfo)
	   && (dEOverPInfo == locSourceData.dEOverPInfo))
		return *this; //guard against self-assignment
	dTimingInfo = dResourcePool_TimingInfo->Get_SharedResource();
	*dTimingInfo = *(locSourceData.dTimingInfo);
	dEOverPInfo = dResourcePool_EOverPInfo->Get_SharedResource();
	*dEOverPInfo = *(locSourceData.dEOverPInfo);
	dTrackingInfo = dResourcePool_TrackingInfo->Get_SharedResource();
	*dTrackingInfo = *(locSourceData.dTrackingInfo);
	return *this;
}

/********************************************************************** GETTERS ************************************************************************/

inline DetectorSystem_t DChargedTrackHypothesis::t1_detector(void) const
{
	if(Get_BCALShowerMatchParams() != nullptr)
		return SYS_BCAL;
	else if(Get_TOFHitMatchParams() != nullptr)
		return SYS_TOF;
	else if(Get_FCALShowerMatchParams() != nullptr)
		return SYS_FCAL;
	else if(Get_ECALShowerMatchParams() != nullptr)
		return SYS_ECAL;
	else if(Get_SCHitMatchParams() != nullptr)
		return SYS_START;
	return SYS_NULL;
}

inline double DChargedTrackHypothesis::Get_PathLength(void) const
{
	auto locPathLength = 0.0;
	if(Get_BCALShowerMatchParams() != nullptr)
		locPathLength = Get_BCALShowerMatchParams()->dPathLength;
	else if(Get_TOFHitMatchParams() != nullptr)
		locPathLength = Get_TOFHitMatchParams()->dPathLength;
	else if(Get_FCALShowerMatchParams() != nullptr)
	  locPathLength = Get_FCALShowerMatchParams()->dPathLength;
	else if(Get_ECALShowerMatchParams() != nullptr)
		locPathLength = Get_ECALShowerMatchParams()->dPathLength;
	else if(Get_SCHitMatchParams() != nullptr)
		locPathLength = Get_SCHitMatchParams()->dPathLength;
	else
		return std::numeric_limits<double>::quiet_NaN();

	//correct for the fact that t0 is reported at the poca to the vertex, and the path length above is to the POCA to the beamline
	return locPathLength + (time() - Get_TimeAtPOCAToVertex())/(29.9792458*lorentzMomentum().Beta());
}

inline double DChargedTrackHypothesis::t1(void) const
{
	auto locDetector = t1_detector();
	if(locDetector == SYS_BCAL)
		return Get_BCALShowerMatchParams()->dBCALShower->t;
	else if(locDetector == SYS_TOF)
		return Get_TOFHitMatchParams()->dTOFPoint->t;
	else if(locDetector == SYS_FCAL)
		return Get_FCALShowerMatchParams()->dFCALShower->getTime();
	else if(locDetector == SYS_ECAL)
		return Get_ECALShowerMatchParams()->dECALShower->t;
	else if(locDetector == SYS_START)
		return Get_SCHitMatchParams()->dHitTime;
	return std::numeric_limits<double>::quiet_NaN();
}

inline double DChargedTrackHypothesis::t1_err(void) const
{
	auto locDetector = t1_detector();
	if(locDetector == SYS_BCAL)
		return sqrt(Get_BCALShowerMatchParams()->dBCALShower->ExyztCovariance(4, 4));
	else if(locDetector == SYS_TOF)
		return Get_TOFHitMatchParams()->dTOFPoint->tErr;
	else if(locDetector == SYS_FCAL)
		return sqrt(Get_FCALShowerMatchParams()->dFCALShower->ExyztCovariance(4, 4));
	else if(locDetector == SYS_ECAL)
		return sqrt(Get_ECALShowerMatchParams()->dECALShower->ExyztCovariance(4, 4));
	else if(locDetector == SYS_START)
		return sqrt(Get_SCHitMatchParams()->dHitTimeVariance);
	return std::numeric_limits<double>::quiet_NaN();
}

/********************************************************************** SETTERS ************************************************************************/

inline void DChargedTrackHypothesis::Share_FromInput(const DChargedTrackHypothesis* locSourceData, bool locShareTrackingFlag, bool locShareTimingFlag, bool locShareEOverPFlag, bool locShareKinematicsFlag)
{
	if(locShareTrackingFlag)
		dTrackingInfo = const_cast<DChargedTrackHypothesis*>(locSourceData)->dTrackingInfo;
	if(locShareTimingFlag)
		dTimingInfo = const_cast<DChargedTrackHypothesis*>(locSourceData)->dTimingInfo;
	if(locShareEOverPFlag)
	  dEOverPInfo = const_cast<DChargedTrackHypothesis*>(locSourceData)->dEOverPInfo;
	if(locShareKinematicsFlag)
		Share_FromInput_Kinematics(static_cast<const DKinematicData*>(locSourceData));
}

inline void DChargedTrackHypothesis::Set_T0(double locT0, double locT0Error, DetectorSystem_t locT0Detector)
{
	dTimingInfo->dt0 = locT0;
	dTimingInfo->dt0_err = locT0Error;
	dTimingInfo->dt0_detector = locT0Detector;
}

inline void DChargedTrackHypothesis::Set_ChiSq_Timing(double locChiSq, unsigned int locNDF)
{
	dTimingInfo->dChiSq_Timing = locChiSq;
	dTimingInfo->dNDF_Timing = locNDF;
}

inline void DChargedTrackHypothesis::Set_ChiSq_EOverP(DetectorSystem_t detector,double locChiSq, unsigned int locNDF)
{
  dEOverPInfo->dcal_detector=detector;
  dEOverPInfo->dChiSq_EoverP = locChiSq;
  dEOverPInfo->dNDF_EoverP = locNDF;
}

inline void DChargedTrackHypothesis::Set_ChiSq_DCdEdx(double locChiSq, unsigned int locNDF)
{
	dTrackingInfo->dChiSq_DCdEdx = locChiSq;
	dTrackingInfo->dNDF_DCdEdx = locNDF;
}

inline void DChargedTrackHypothesis::Set_dEdx_CDC_amp(double locCDCdEdx_amp)
{
	dTrackingInfo->ddEdx_CDC_amp = locCDCdEdx_amp;
}

inline void DChargedTrackHypothesis::Set_dEdx_CDC_int(double locCDCdEdx_int)
{
	dTrackingInfo->ddEdx_CDC_int = locCDCdEdx_int;
}

inline void DChargedTrackHypothesis::Set_ChiSq_Overall(double locChiSq, unsigned int locNDF, double locFOM)
{
	dTimingInfo->dChiSq = locChiSq;
	dTimingInfo->dNDF = locNDF;
	dTimingInfo->dFOM = locFOM;
}

inline void DChargedTrackHypothesis::Reset(void)
{
	DKinematicData::Reset();
	dTimingInfo = dResourcePool_TimingInfo->Get_SharedResource(); //not safe to reset individually, since you don't know what it's shared with
	dTrackingInfo = dResourcePool_TrackingInfo->Get_SharedResource(); //not safe to reset individually, since you don't know what it's shared with
	dEOverPInfo = dResourcePool_EOverPInfo->Get_SharedResource(); //not safe to reset individually, since you don't know what it's shared with
}

inline void DChargedTrackHypothesis::Release(void)
{
	DKinematicData::Release();
	dTimingInfo = nullptr;
	dTrackingInfo = nullptr;
	dEOverPInfo = nullptr;
}

inline void DChargedTrackHypothesis::DTimingInfo::Reset(void)
{
	dt0 = 0.0;
	dt0_err = 0.0;
	dt0_detector = SYS_NULL;
	dNDF_Timing = 0;
	dChiSq_Timing = 0.0;
	dNDF = 0;
	dChiSq = 0.0;
	dFOM = 0.0;
	dTimeAtPOCAToVertex = 0.0;
}

inline void DChargedTrackHypothesis::DTrackingInfo::Reset(void)
{
	dNDF_DCdEdx = 0;
	dChiSq_DCdEdx = 0.0;
	ddEdx_CDC_amp = 0.0;
	ddEdx_CDC_int = 0.0;
	dTrackTimeBased = nullptr;
	dSCHitMatchParams = nullptr;
	dTOFHitMatchParams = nullptr;
	dCTOFHitMatchParams = nullptr; 
	dBCALShowerMatchParams = nullptr;
	dFCALShowerMatchParams = nullptr;
	dFCALSingleHitMatchParams = nullptr;
	dDIRCMatchParams = nullptr;
	dFMWPCMatchParams = nullptr;
	dTRDMatchParams = nullptr;
	dECALShowerMatchParams = nullptr;
	dECALSingleHitMatchParams = nullptr;
}

inline void DChargedTrackHypothesis::DEOverPInfo::Reset(void)
{	
  dcal_detector = SYS_NULL;
  dNDF_EoverP=0;
  dChiSq_EoverP=0.0;
}



#endif // _DChargedTrackHypothesis_
