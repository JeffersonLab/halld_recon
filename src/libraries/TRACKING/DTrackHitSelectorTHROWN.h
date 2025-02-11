// $Id$
//
//    File: DTrackHitSelectorTHROWN.h
// Created: Mon Mar  9 09:03:03 EDT 2009
// Creator: davidl (on Darwin harriet.jlab.org 9.6.0 i386)
//

#ifndef _DTrackHitSelectorTHROWN_
#define _DTrackHitSelectorTHROWN_

#include <JANA/Compatibility/jerror.h>

#include <TRACKING/DTrackHitSelector.h>

class DMCTrackHit;

class DTrackHitSelectorTHROWN:public DTrackHitSelector{
	public:
		DTrackHitSelectorTHROWN(const std::shared_ptr<const JEvent>& loop);
		virtual ~DTrackHitSelectorTHROWN();
		
		void GetCDCHits(fit_type_t fit_type, const DReferenceTrajectory *rt, const vector<const DCDCTrackHit*> &cdchits_in, vector<const DCDCTrackHit*> &cdchits_out,int N=0) const;
		void GetFDCHits(fit_type_t fit_type, const DReferenceTrajectory *rt, const vector<const DFDCPseudo*> &fdchits_in, vector<const DFDCPseudo*> &fdchits_out, int N=0) const;
		void GetCDCHits(double Bz,double q,const vector<DTrackFitter::Extrapolation_t> &extrapolations, const vector<const DCDCTrackHit*> &cdchits_in, vector<const DCDCTrackHit*> &cdchits_out, int N=0) const {}; 
		void GetFDCHits(double Bz,double q,
				const vector<DTrackFitter::Extrapolation_t> &extrapolations, const vector<const DFDCPseudo*> &fdchits_in, vector<const DFDCPseudo*> &fdchits_out,int N) const {};
		void GetGEMHits(const vector<DTrackFitter::Extrapolation_t> &extrapolations, const vector<const DGEMPoint*> &gemhits_in, vector<const DGEMPoint*> &gemhits_out) const {};
		void GetTRDHits(const vector<DTrackFitter::Extrapolation_t> &extrapolations, const vector<const DTRDPoint*> &trdhits_in, vector<const DTRDPoint*> &trdhits_out) const {};

		int FindTrackNumber(const DReferenceTrajectory *rt) const;
		static const DMCTrackHit* GetMCTrackHit(const DCoordinateSystem *wire, double rdrift, vector<const DMCTrackHit*> &mctrackhits, int trackno_filter=-1);

	private:
		int HS_DEBUG_LEVEL;
		
};

#endif // _DTrackHitSelectorTHROWN_

