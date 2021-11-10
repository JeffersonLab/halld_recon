// $Id$
//
//    File: trk_mainframe.h
// Created: Wed Apr  9 08:11:16 EDT 2008
// Creator: davidl (on Darwin Amelia.local 8.11.1 i386)
//

// This class is made into a ROOT dictionary ala rootcint.
// Therefore, do NOT include anything Hall-D specific here.
// It is OK to do that in the .cc file, just not here in the 
// header.

#ifndef _trk_mainframe_
#define _trk_mainframe_

#include <iostream>
#include <cmath>
#include <string>
#include <vector>
#include <map>

#include <TGClient.h>
#include <TGButton.h>
#include <TCanvas.h>
#include <TText.h>
#include <TRootEmbeddedCanvas.h>
#include <TTUBE.h>
#include <TNode.h>
#include <TGComboBox.h>
#include <TPolyLine.h>
#include <TEllipse.h>
#include <TMarker.h>
#include <TVector3.h>
#include <TGLabel.h>
#include <TTimer.h>
#include <TH1.h>
#include <TLatex.h>


#if !(defined(__CINT__) || defined(__CLING__))

#include "hdv_mainframe.h"

#else

// This ugliness is due to rootcling incorrectly flagging the
// foward declaration of hdv_mainframe as an error. Since only
// pointers to the class with no internals being referenced here,
// a forward declaration should be allowed. This empty class
// definition is just to appease rootcling but should not be
// seen by the actual C++ compiler.
class hdv_mainframe{};

#endif


class DCoordinateSystem;
class DReferenceTrajectory;
class DCDCTrackHit;

class trk_mainframe:public TGMainFrame{
	public:
		trk_mainframe(hdv_mainframe *hdvmf, const TGWindow *p, UInt_t w, UInt_t h);
		virtual ~trk_mainframe();
		
		void DoNewEvent(void);
		void DoMyRedraw(void);
		void DoHitSelect(void);
		void DoUpdateMenus(void);
		void DoTagMenuUpdate(Int_t widgetId, Int_t id);
		void DoTrackNumberMenuUpdate(Int_t widgetId, Int_t id);
		void DoRequestFocus(Int_t id);
		
		void FillDataTypeComboBox(TGComboBox* cb, const std::string &def="DTrack");
		void FillFactoryTagComboBox(TGComboBox* cb, TGComboBox* datanamecb, const std::string &def);
		void FillTrackNumberComboBox(TGComboBox* cb, TGComboBox* datanamecb, TGComboBox* tagcb, bool add_best_match_option);
		
	protected:
	
	
	private:
		hdv_mainframe *hdvmf;
		
		TRootEmbeddedCanvas *canvas;
		TRootEmbeddedCanvas *histocanvas;
		TH1D *resi;
		TLatex *resi_lab;
		std::vector<TGComboBox*> datatype;
		std::vector<TGComboBox*> factorytag;
		std::vector<TGComboBox*> trackno;
		TGCheckButton *slock;

		double slo, shi, resilo, resihi;
		
		std::vector<TObject*> graphics;
		
		void DrawAxes(TCanvas *c, std::vector<TObject*> &graphics, const char *xlab, const char *ylab);
		void DrawHits(std::vector<TObject*> &graphics);
		void DrawHitsForOneTrack(TVector3 &pos, TVector3 &mom, std::vector<TObject*> &graphics);
		void DrawHitsForOneTrack(std::vector<TObject*> &graphics,
										std::vector<std::pair<const DCoordinateSystem*,double> > &allhits,
										DReferenceTrajectory *rt,
										int index,
										std::vector<const DCDCTrackHit*> &cdctrackhits);

		bool WireInList(const DCoordinateSystem *wire, std::vector<const DCDCTrackHit*> &cdctrackhits);

	ClassDef(trk_mainframe,1)
};

#endif // _trk_mainframe_

