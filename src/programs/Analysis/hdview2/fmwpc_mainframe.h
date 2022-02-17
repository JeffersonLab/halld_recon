// $Id$
//
//    File: fmwpc_mainframe.h
// Created: Wed Apr  9 08:11:16 EDT 2008
// Creator: davidl (on Darwin Amelia.local 8.11.1 i386)
//

// This class is made into a ROOT dictionary ala rootcint.
// Therefore, do NOT include anything Hall-D specific here.
// It is OK to do that in the .cc file, just not here in the 
// header.

#ifndef _fmwpc_mainframe_
#define _fmwpc_mainframe_

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

#include "hdv_mainframe.h"


class DCoordinateSystem;
class DReferenceTrajectory;
class DCDCTrackHit;

class fmwpc_mainframe:public TGMainFrame{
	public:
		fmwpc_mainframe(hdv_mainframe *hdvmf, const TGWindow *p, UInt_t w, UInt_t h);
		virtual ~fmwpc_mainframe();
		
		void DoNewEvent(void);
		void DoMyRedraw(void);
		void DoHitSelect(void);
		void DoUpdateMenus(void);
		void DoTagMenuUpdate(Int_t widgetId, Int_t id);
		void DoTrackNumberMenuUpdate(Int_t widgetId, Int_t id);
		void DoRequestFocus(Int_t id);
        void DoClose(void);
		
		
	protected:
	
	
	private:
		hdv_mainframe *hdvmf;
		
		TRootEmbeddedCanvas *topcanvas;
        TRootEmbeddedCanvas *sidecanvas;
		TRootEmbeddedCanvas *histocanvas;
		TH1D *resi;
		TLatex *resi_lab;
		std::vector<TGComboBox*> datatype;
		std::vector<TGComboBox*> factorytag;
		std::vector<TGComboBox*> trackno;
        std::map<std::string, TGCheckButton*> checkbuttons;

        TGLabel *event, *run;

		double slo, shi, zlo, zhi;
		
		std::vector<TObject*> topgraphics;
		std::vector<TObject*> sidegraphics;
        std::map<TObject*, std::string> graphics_draw_options; // for special draw options for those graphics that need them
		
		void DrawAxes(TCanvas *c, std::vector<TObject*> &graphics, const char *xlab, const char *ylab);
        void DrawDetectors(TCanvas *c, std::vector<TObject*> &graphics, std::string view);
		void DrawHits(std::vector<TObject*> &graphics);
		void DrawHitsForOneTrack(TVector3 &pos, TVector3 &mom, std::vector<TObject*> &graphics);
		void DrawHitsForOneTrack(std::vector<TObject*> &graphics,
										std::vector<std::pair<const DCoordinateSystem*,double> > &allhits,
										DReferenceTrajectory *rt,
										int index,
										std::vector<const DCDCTrackHit*> &cdctrackhits);

		bool WireInList(const DCoordinateSystem *wire, std::vector<const DCDCTrackHit*> &cdctrackhits);



	ClassDef(fmwpc_mainframe,1)

};

#endif // _fmwpc_mainframe_

