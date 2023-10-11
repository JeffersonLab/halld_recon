// $Id$
//
//    File: DTrackHitSelector.cc
// Created: Thu Feb  5 13:34:58 EST 2009
// Creator: davidl (on Darwin harriet.jlab.org 9.6.0 i386)
//

#include "DTrackHitSelector.h"


// The methods here are just convenience methods that delegate
// to the virtual methods that get implemented in the subclass.

// Note that we sort the hits in the same way as is done in 
// DTrack_factory just so it is easier to compare the lists
// of hits from that and other factories using this hit
// selector. These routines are defined in DTrack_factory.cc
extern bool CDCSortByRincreasing(const DCDCTrackHit* const &hit1, const DCDCTrackHit* const &hit2);
extern bool FDCSortByZincreasing(const DFDCPseudo* const &hit1, const DFDCPseudo* const &hit2);
extern bool GEMSortByZincreasing(const DGEMPoint* const &hit1, const DGEMPoint* const &hit2);
extern bool TRDSortByZincreasing(const DTRDPoint* const &hit1, const DTRDPoint* const &hit2);


//---------------------
// Constructor
//---------------------
DTrackHitSelector::DTrackHitSelector(const std::shared_ptr<const JEvent>& event){
	this->event = event;
}

//---------------------
// GetCDCHits
//---------------------
void DTrackHitSelector::GetCDCHits(fit_type_t fit_type, const DReferenceTrajectory *rt, const vector<const DCDCTrackHit*> &cdchits_in, DTrackFitter *fitter,int N) const
{
	/// Get all hits from the CDC and add them to the specified DTrackFitter object

	vector<const DCDCTrackHit*> cdchits_out;
	GetCDCHits(fit_type, rt, cdchits_in, cdchits_out,N);
	sort(cdchits_out.begin(), cdchits_out.end(), CDCSortByRincreasing);
	for(unsigned int i=0; i<cdchits_out.size(); i++)fitter->AddHit(cdchits_out[i]);
}

//---------------------
// GetCDCHits
//---------------------
void DTrackHitSelector::GetCDCHits(double Bz, double q,const vector<DTrackFitter::Extrapolation_t> &extrapolations, const vector<const DCDCTrackHit*> &cdchits_in, DTrackFitter *fitter,int N) const
{
	/// Get all hits from the CDC and add them to the specified DTrackFitter object

	vector<const DCDCTrackHit*> cdchits_out;
	GetCDCHits(Bz,q,extrapolations, cdchits_in, cdchits_out,N);
	sort(cdchits_out.begin(), cdchits_out.end(), CDCSortByRincreasing);
	for(unsigned int i=0; i<cdchits_out.size(); i++)fitter->AddHit(cdchits_out[i]);
}

//---------------------
// GetFDCHits
//---------------------
void DTrackHitSelector::GetFDCHits(fit_type_t fit_type, const DReferenceTrajectory *rt, const vector<const DFDCPseudo*> &fdchits_in, DTrackFitter *fitter,int N) const
{
	/// Get all hits from the FDC and add them to the specified DTrackFitter object

	vector<const DFDCPseudo*> fdchits_out;
	GetFDCHits(fit_type, rt, fdchits_in, fdchits_out,N);
	sort(fdchits_out.begin(), fdchits_out.end(), FDCSortByZincreasing);
	for(unsigned int i=0; i<fdchits_out.size(); i++)fitter->AddHit(fdchits_out[i]);
}

//---------------------
// GetFDCHits
//---------------------
void DTrackHitSelector::GetFDCHits(double Bz,double q,const vector<DTrackFitter::Extrapolation_t> &extrapolations, const vector<const DFDCPseudo*> &fdchits_in, DTrackFitter *fitter,int N) const
{
	/// Get all hits from the FDC and add them to the specified DTrackFitter object

	vector<const DFDCPseudo*> fdchits_out;
	GetFDCHits(Bz,q,extrapolations, fdchits_in, fdchits_out,N);
	sort(fdchits_out.begin(), fdchits_out.end(), FDCSortByZincreasing);
	for(unsigned int i=0; i<fdchits_out.size(); i++)fitter->AddHit(fdchits_out[i]);
}

//---------------------
// GetTRDHits
//---------------------
void DTrackHitSelector::GetTRDHits(const vector<DTrackFitter::Extrapolation_t> &extrapolations, const vector<const DTRDPoint*> &trdhits_in, DTrackFitter *fitter) const
{
  /// Get all hits from the TRD and add them to the specified DTrackFitter object

  vector<const DTRDPoint*> trdhits_out;
  GetTRDHits(extrapolations, trdhits_in, trdhits_out);
  sort(trdhits_out.begin(), trdhits_out.end(), TRDSortByZincreasing);
  for(unsigned int i=0; i<trdhits_out.size(); i++)fitter->AddHit(trdhits_out[i]);
}

//---------------------
// GetGEMHits
//---------------------
void DTrackHitSelector::GetGEMHits(const vector<DTrackFitter::Extrapolation_t> &extrapolations, const vector<const DGEMPoint*> &gemhits_in, DTrackFitter *fitter) const
{
  /// Get all hits from the GEM and add them to the specified DTrackFitter object

  vector<const DGEMPoint*> gemhits_out;
  GetGEMHits(extrapolations, gemhits_in, gemhits_out);
  sort(gemhits_out.begin(), gemhits_out.end(), GEMSortByZincreasing);
  for(unsigned int i=0; i<gemhits_out.size(); i++)fitter->AddHit(gemhits_out[i]);
}

//---------------------
// GetAllHits
//---------------------
void DTrackHitSelector::GetAllHits(fit_type_t fit_type, const DReferenceTrajectory *rt, const vector<const DCDCTrackHit*> &cdchits_in, const vector<const DFDCPseudo*> &fdchits_in, DTrackFitter *fitter,int N) const
{

  /// Get all hits from both CDC and FDC and add them to the specified DTrackFitter object
  if (cdchits_in.size()>0) GetCDCHits(fit_type, rt, cdchits_in, fitter, N);
  if (fdchits_in.size()>0) GetFDCHits(fit_type, rt, fdchits_in, fitter, N);
}


