// $Id$
//
//    File: DFCALTruthShower.h
// Created: Wed Jan  4 14:43:05 EST 2006
// Creator: davidl (on Linux jlabl1.jlab.org 2.4.21-37.ELsmp i686)
//

#ifndef _DFCALTruthShower_
#define _DFCALTruthShower_

#include "JANA/JObject.h"
#include "DANA/DObjectID.h"

class DFCALTruthShower: public JObject{
	public:
		JOBJECT_PUBLIC(DFCALTruthShower);
		DFCALTruthShower(){}
		
		DFCALTruthShower( oid_t id,
				  float x, float y, float z,
				  float px, float py, float pz,
				  float E, float t,
				  bool primary, int track, int type,
				  int itrack=0) :
		  m_x( x ),
		  m_y( y ),
		  m_z( z ),
		  m_px( px ),
		  m_py( py ),
		  m_pz( pz ),
		  m_E( E ),
		  m_t( t ),
		  m_primary(primary),
		  m_track(track),
		  m_type(type),
		  m_itrack(itrack){}
	
		float x() const { return m_x; }
		float y() const { return m_y; }
		float z() const { return m_z; }
		float t() const { return m_t; }
		float px() const { return m_px; }
		float py() const { return m_py; }
		float pz() const { return m_pz; }
		float E() const { return m_E; }
		bool primary() const { return m_primary; }
		int track() const { return m_track; }
		int type() const { return m_type; }
		int itrack() const { return m_itrack; }

		void Summarize(JObjectSummary& summary) const override {
			summary.add(x(), "x(cm)", "%3.1f");
			summary.add(y(), "y(cm)", "%3.1f");
			summary.add(z(), "z(cm)", "%3.1f");
			summary.add(px()*1000, "px(MeV/c)", "%3.1f");
			summary.add(py()*1000, "py(MeV/c)", "%3.1f");
			summary.add(pz()*1000, "pz(MeV/c)", "%3.1f");
			summary.add(E()*1000.0, "E(MeV)", "%3.3f");
			summary.add(t(), "t(ns)", "%3.1f");
			summary.add(primary(), "primary", "%d");
			summary.add(track(), "track", "%d");
			summary.add(itrack(), "itrack", "%d");
			summary.add(type(), "type", "%d");
		}

	private:
		
		float m_x;
		float m_y;
		float m_z;
		float m_px;
		float m_py;
		float m_pz;
		float m_E;
		float m_t;
		bool m_primary;
		int m_track;
		int m_type;
		int m_itrack;
};

#endif // _DFCALTruthShower_

