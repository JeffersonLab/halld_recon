// $Id$
//
//    File: DTrackFitter.h
// Created: Mon Sep  1 10:30:04 EDT 2008
// Creator: davidl (on Darwin Amelia.local 8.11.1 i386)
//

#ifndef _DTrackFitter_
#define _DTrackFitter_

#include <JANA/JObject.h>
#include <JANA/JEvent.h>

#ifdef PROFILE_TRK_TIMES
#include <prof_time.h>
#endif
#include <TRACKING/DTrackingData.h>
#include <HDGEOMETRY/DMagneticFieldMap.h>
#include "HDGEOMETRY/DLorentzMapCalibDB.h"
#include <CDC/DCDCTrackHit.h>
#include <FDC/DFDCPseudo.h>
#include <TRACKING/DReferenceTrajectory.h>
#include <TRD/DTRDPoint.h>

using namespace std;

#define Quiet_NaN std::numeric_limits<double>::quiet_NaN()


class DReferenceTrajectory;
class DGeometry;

//////////////////////////////////////////////////////////////////////////////////
/// The DTrackFitter class is a base class for different charged track
/// fitting algorithms. It does not actually fit the track itself, but
/// provides the interface and some common support features most algorthims
/// will need to implement.
///
/// The reason this is needed (instead of just using the mechanism already
/// built into JANA) is due to the nature of charged track fitting.
/// Specifically, tracks are usually fit first to the wire positions
/// and then to the drift times. The algorithm for both is (at least
/// usually) the same. However, we want to separate the wire-based and
/// time-based fitting into 2 distinct stages allowing easy access to the 
/// wire-based fits.
///
/// There were a few options on how to handle this within the JANA framework
/// but it was decided passing DTrackFitter objects through the framework
/// was the best way to address it. Sub-classes of DTrackFitter will
/// implement the actual algorithms, but JANA will only see these
/// objects as pointers to the DTrackFitter base class. Only one
/// DTrackFitterXXX object will exist for each thread (i.e. each JEventLoop).
/// As such, the state of that object will likely be overwritten
/// many times in a single event and it's internal data never used
/// by anything outside of the TRACKING package. Also, the factories that
/// produce the DTrackFitterXXX objects will make them as persistent
/// and will turn off the the WRITE_TO_OUTPUT bit by default.
//////////////////////////////////////////////////////////////////////////////////

class DTrackFitter: public JObject{
	public:
		JOBJECT_PUBLIC(DTrackFitter);
		
		enum fit_type_t{
			kWireBased,
			kTimeBased
		};
		
		enum fit_status_t{
			kFitNotDone,
			kFitSuccess,
			kFitFailed,
			kFitNoImprovement
		};
		
		class dedx_t{
		public:
		  dedx_t(double dE,double dx, double p):dE(dE),dx(dx),p(p){}
		    double dE; // energy loss in layer
		    double dx; // path length in layer
		    double p;  // momentum at this dE/dx measurement

		};
		class Extrapolation_t{
		public:
		Extrapolation_t(DVector3 position,DVector3 momentum,
				double t,double s,double s_theta_ms_sum=0.,
				double theta2ms_sum=0.):
		  position(position),momentum(momentum),t(t),s(s),s_theta_ms_sum(s_theta_ms_sum),theta2ms_sum(theta2ms_sum){}
		  DVector3 position;
		  DVector3 momentum;
		  double t;
		  double s;
		  double s_theta_ms_sum;
		  double theta2ms_sum;
		};


		class pull_t{
		public:
		pull_t(double resi, double err,double s=0.0,
		       double tdrift=0.0, double d=0.0,
		       const DCDCTrackHit *cdc_hit=NULL,
		       const DFDCPseudo *fdc_hit=NULL, double docaphi=0.0,
		       double z=0.0, double cosThetaRel=0.0,double tcorr=0.0,double resic=0.0,
		       double errc=0.0, int left_right=0):resi(resi),err(err),s(s),tdrift(tdrift),d(d),cdc_hit(cdc_hit),fdc_hit(fdc_hit),docaphi(docaphi),z(z),cosThetaRel(cosThetaRel),tcorr(tcorr),resic(resic),errc(errc),left_right(left_right){}
		    double resi;	// residual of measurement
		    double err;		// estimated error of measurement
		    double s;
		    double tdrift;      // drift time of this measurement
		    double d;  // doca to wire
		    const DCDCTrackHit *cdc_hit;
		    const DFDCPseudo *fdc_hit;
		    double docaphi; // phi of doca in CDC straws
		    double z;// z position at doca
		    double cosThetaRel; // dot product between track and wire directions
		    double tcorr; // drift time with correction for B
		    double resic; // residual for FDC cathode measuremtns
		    double errc;
		    int left_right;  // left-right info. of the wire plane (-1: left or +1: right)
          vector<double> trackDerivatives;
          inline void AddTrackDerivatives(vector<double> d){ trackDerivatives = d;}
             
		};
		
		// Constructor and destructor
		DTrackFitter(const std::shared_ptr<const JEvent>& event);  // TODO: Let's move this logic somewhere else please
		virtual ~DTrackFitter();
		
		void Reset(void);
		
		// Hit accessor methods
		void AddHit(const DCDCTrackHit* cdchit);
		void AddHits(vector<const DCDCTrackHit*> cdchits);
		void AddHit(const DFDCPseudo* fdchit);
		void AddHits(vector<const DFDCPseudo*> fdchits);
		void AddHit(const DTRDPoint* trdhit);
		void AddHits(vector<const DTRDPoint*> trdhits);
		const vector<const DCDCTrackHit*>& GetCDCInputHits(void) const {return cdchits;}
		const vector<const DFDCPseudo*>&   GetFDCInputHits(void) const {return fdchits;}
		const vector<const DCDCTrackHit*>& GetCDCFitHits(void) const {return cdchits_used_in_fit;}
		const vector<const DFDCPseudo*>&   GetFDCFitHits(void) const {return fdchits_used_in_fit;}
		void ClearExtrapolations(void){	 
		  extrapolations[SYS_NULL].clear();
		  extrapolations[SYS_TOF].clear();
		  extrapolations[SYS_BCAL].clear();
		  extrapolations[SYS_FCAL].clear();
		  extrapolations[SYS_FDC].clear();
		  extrapolations[SYS_CDC].clear();
		  extrapolations[SYS_START].clear();
		  extrapolations[SYS_DIRC].clear();
		  extrapolations[SYS_TRD].clear();
		  extrapolations[SYS_FMWPC].clear();
		  extrapolations[SYS_CTOF].clear();
		  extrapolations[SYS_ECAL].clear();
		};
		
		// Fit parameter accessor methods
		const DKinematicData& GetInputParameters(void) const {return input_params;}
		const DTrackingData& GetFitParameters(void) const {return fit_params;}
		double GetChisq(void) const {return chisq;}
		int GetNdof(void) const {return Ndof;}
		unsigned int GetNumPotentialFDCHits(void) const {return potential_fdc_hits_on_track;}
		unsigned int GetNumPotentialCDCHits(void) const {return potential_cdc_hits_on_track;}
		//virtual unsigned int GetRatioMeasuredPotentialFDCHits(void) const =0;
		//virtual unsigned int GetRatioMeasuredPotentialCDCHits(void) const =0;
      bool GetIsSmoothed(void) const {return IsSmoothed;}
		
      vector<pull_t>& GetPulls(void){return pulls;}
      const map<DetectorSystem_t,vector<Extrapolation_t> >&GetExtrapolations(void) const{
	return extrapolations;
      }

		fit_type_t GetFitType(void) const {return fit_type;}
		const DMagneticFieldMap* GetDMagneticFieldMap(void) const {return bfield;}

		void SetFitType(fit_type_t type){fit_type=type;}
		void SetInputParameters(const DTrackingData &starting_params){input_params=starting_params;}
		
		// Wrappers
		fit_status_t FitTrack(const DVector3 &pos, const DVector3 &mom, double q, double mass,double t0=Quiet_NaN,DetectorSystem_t t0_det=SYS_NULL);
		fit_status_t FitTrack(const DTrackingData &starting_params);
		
		// Methods that actually do something
		fit_status_t 
		  FindHitsAndFitTrack(const DKinematicData &starting_params, 
				      const DReferenceTrajectory *rt, 
				      const std::shared_ptr<const JEvent> &loop, double mass=-1.0,
				      int N=0,
				      double t0=Quiet_NaN,
				      DetectorSystem_t t0_det=SYS_NULL
				      ); ///< mass<0 means get it from starting_params
		fit_status_t 
		  FindHitsAndFitTrack(const DKinematicData &starting_params, 
				      const map<DetectorSystem_t,vector<DTrackFitter::Extrapolation_t> >&extrapolations,
				      const std::shared_ptr<const JEvent>& loop,
				      double mass,int N,double t0,
				      DetectorSystem_t t0_det);
		
		jerror_t CorrectForELoss(const DKinematicData &starting_params, DReferenceTrajectory *rt, DVector3 &pos, DVector3 &mom, double mass);
		double CalcDensityEffect(double p,double mass,double density,
					 double Z_over_A,double I);  
		double CalcDensityEffect(double p,double mass,
					 double rho_Z_over_A,double LnI);
		double CalcDensityEffect(double betagamma,
					 double rho_Z_over_A,double LnI);
#ifdef PROFILE_TRK_TIMES
		void GetProfilingTimes(std::map<std::string, prof_time::time_diffs> &my_prof_times) const;
#endif		
		bool ExtrapolateToRadius(double R,
					 const vector<Extrapolation_t>&extraps,
					 DVector3 &pos,DVector3 &mom,double &t,
					 double &s) const;
		bool ExtrapolateToRadius(double R,
					 const vector<Extrapolation_t>&extraps,
					 DVector3 &pos) const;
		double DistToWire(const DCoordinateSystem *wire,
				  const vector<Extrapolation_t>&extrapolations,
				  DVector3 *pos=NULL,DVector3 *mom=NULL,
				  DVector3 *position_along_wire=NULL) const;
	      
		//---- The following need to be supplied by the subclass ----
		virtual string Name(void) const =0;
		virtual fit_status_t FitTrack(void)=0;
		virtual double ChiSq(fit_type_t fit_type, DReferenceTrajectory *rt, double *chisq_ptr=NULL, int *dof_ptr=NULL, vector<pull_t> *pulls_ptr=NULL)=0;

	protected:
	

		// The following should be used as inputs by FitTrack(void)
		vector<const DCDCTrackHit*> cdchits;	//< Hits in the CDC
		vector<const DFDCPseudo*> fdchits;		//< Hits in the FDC
		vector<const DTRDPoint*> trdhits;

	DTrackingData input_params;				//< Starting parameters for the fit
		fit_type_t fit_type;							//< kWireBased or kTimeBased
		const DMagneticFieldMap *bfield;			//< Magnetic field map for current event (acquired through loop)
		const DGeometry *geom;						//< DGeometry pointer used to access materials through calibDB maps for eloss
		const DRootGeom *RootGeom;					//< ROOT geometry used for accessing material for MULS, energy loss
		std::shared_ptr<const JEvent> event;	    //< Pointer to JEventLoop object handling the current event
													// TODO: Delete me completely if at all possible!

		// The following should be set as outputs by FitTrack(void)
		DTrackingData fit_params;									//< Results of last fit
		double chisq;													//< Chi-sq of final track fit (not the chisq/dof!)
		int Ndof;														//< Number of degrees of freedom for final track
		vector<pull_t> pulls;										//< pull_t objects for each contribution to chisq (assuming no correlations)
		map<DetectorSystem_t,vector<Extrapolation_t> > extrapolations;

		fit_status_t fit_status;									//< Status of values in fit_params (kFitSuccess, kFitFailed, ...)
		vector<const DCDCTrackHit*> cdchits_used_in_fit;	//< The CDC hits actually used in the fit
		vector<const DFDCPseudo*> fdchits_used_in_fit;		//< The FDC hits actually used in the fit

      bool IsSmoothed;                                   //< Indicates if the smoother routine finished successfully

		unsigned int potential_fdc_hits_on_track;
		unsigned int potential_cdc_hits_on_track;


		bool CORRECT_FOR_ELOSS;

	private:
		int DEBUG_LEVEL;
		string MATERIAL_MAP_MODEL;			

		// Prohibit default constructor
		DTrackFitter();
		
		// gas material properties
		double mKRhoZoverAGas,mRhoZoverAGas,mLnIGas;
		
};

#endif // _DTrackFitter_

