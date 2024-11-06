// $Id$
//
//    File: DTrackHitSelector.h
// Created: Thu Feb  5 13:34:58 EST 2009
// Creator: davidl (on Darwin harriet.jlab.org 9.6.0 i386)
//

#ifndef _DTrackHitSelector_
#define _DTrackHitSelector_

#include <JANA/JObject.h>

#include <TRACKING/DTrackFitter.h>



class DReferenceTrajectory;
class DCDCTrackHit;
class DFDCPseudo;

/// The DTrackHitSelector class is a base class for algorithms that 
/// will select hits from the drift chamber systems that are likely
/// to belong to a specified trajectory. This class doesn't actually
/// do the hit selection itself, it just provides a standard API so
/// multiple algorithms can be written. It is done this way since at
/// this point in time, we expect at least a couple of algorithms may
/// be tried.    Feb. 6, 2009  DL

class DTrackHitSelector: public JObject {
 public:
  JOBJECT_PUBLIC(DTrackHitSelector);
  
  DTrackHitSelector(const std::shared_ptr<const JEvent>& event);
  ~DTrackHitSelector() = default;

  enum fit_type_t{
    kWireBased = DTrackFitter::kWireBased, // ensure compatibility with DTrackFitter
    kTimeBased = DTrackFitter::kTimeBased, // ensure compatibility with DTrackFitter
    kHelical
  };

  virtual void GetCDCHits(fit_type_t fit_type, const DReferenceTrajectory *rt, const vector<const DCDCTrackHit*> &cdchits_in, vector<const DCDCTrackHit*> &cdchits_out,int N=20) const =0;
  virtual void GetFDCHits(fit_type_t fit_type, const DReferenceTrajectory *rt, const vector<const DFDCPseudo*> &fdchits_in, vector<const DFDCPseudo*> &fdchits_out, int N=20) const =0;	
  virtual void GetCDCHits(double Bz,double q,const vector<DTrackFitter::Extrapolation_t> &extrapolations, const vector<const DCDCTrackHit*> &cdchits_in, vector<const DCDCTrackHit*> &cdchits_out,int N=20) const =0;
  virtual void GetFDCHits(double Bz,double q,
			  const vector<DTrackFitter::Extrapolation_t> &extrapolations, const vector<const DFDCPseudo*> &fdchits_in, vector<const DFDCPseudo*> &fdchits_out,int N) const=0;
  virtual void GetGEMHits(const vector<DTrackFitter::Extrapolation_t> &extrapolations, const vector<const DGEMPoint*> &gemhits_in, vector<const DGEMPoint*> &gemhits_out) const=0;
  virtual void GetTRDHits(const vector<DTrackFitter::Extrapolation_t> &extrapolations, const vector<const DTRDPoint*> &trdhits_in, vector<const DTRDPoint*> &trdhits_out) const=0;


  void GetTRDHits(const vector<DTrackFitter::Extrapolation_t> &extrapolations, const vector<const DTRDPoint*> &trdhits_in, DTrackFitter *fitter) const;
  void GetGEMHits(const vector<DTrackFitter::Extrapolation_t> &extrapolations, const vector<const DGEMPoint*> &gemhits_in, DTrackFitter *fitter) const;
		void GetCDCHits(fit_type_t fit_type, const DReferenceTrajectory *rt, const vector<const DCDCTrackHit*> &cdchits_in, DTrackFitter *fitter,int N=20) const;
		void GetCDCHits(double Bz,double q,const vector<DTrackFitter::Extrapolation_t> &extrapolations, const vector<const DCDCTrackHit*> &cdchits_in, DTrackFitter *fitter,int N) const;
		void GetFDCHits(fit_type_t fit_type, const DReferenceTrajectory *rt, const vector<const DFDCPseudo*> &fdchits_in, DTrackFitter *fitter,int N=20) const;	
		void GetFDCHits(double Bz,double q,const vector<DTrackFitter::Extrapolation_t> &extrapolations, const vector<const DFDCPseudo*> &fdchits_in, DTrackFitter *fitter,int N=20) const;
		void GetAllHits(fit_type_t fit_type, const DReferenceTrajectory *rt, const vector<const DCDCTrackHit*> &cdchits_in, const vector<const DFDCPseudo*> &fdchits_in, DTrackFitter *fitter,int N=20) const;

	protected:
	
		shared_ptr<const JEvent> event = nullptr;  // TODO: Remove this if possible
};

#endif // _DTrackHitSelector_

