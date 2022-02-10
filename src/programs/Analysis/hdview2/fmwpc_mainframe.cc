// $Id$
//
//    File: fmwpc_mainframe.cc
// Created: Wed Apr  9 08:11:16 EDT 2008
// Creator: davidl (on Darwin Amelia.local 8.11.1 i386)
//

#include <cmath>
#include <map>
using namespace std;

#include "fmwpc_mainframe.h"
#include "hdv_mainframe.h"
#include "hdview2.h"
#include "MyProcessor.h"
#include "DReferenceTrajectoryHDV.h"

#include <CDC/DCDCWire.h>
#include <CDC/DCDCTrackHit.h>
#include <FCAL/DFCALHit.h>
#include <TRACKING/DTrackFitterKalmanSIMD.h>
#include <TRACKING/DTrackTimeBased.h>

#include <TGButtonGroup.h>
#include <TGTextEntry.h>
#include <TArrow.h>
#include <TBox.h>
#include <TLatex.h>
#include <TGaxis.h>
#include <TColor.h>
#include <TROOT.h>
#include <TF1.h>
#include <TSpline.h>

// Declared in hdview2.cc
extern JEventLoop *eventloop;
extern MyProcessor *myproc;

// Defined in hdv_mainframe.cc
extern float FCAL_Zlen;
extern float FCAL_Zmin;
extern float FMWPC_width;
extern float FMWPC_Zlen;
extern float FMWPC_Dead_diameter;
extern float FMWPC_WIRE_SPACING;
extern vector<double> FMWPC_Zpos;


// We declare this as a global since putting it in the class would
// require defining the DReferenceTrajectory class to ROOT
static vector<DReferenceTrajectory*> REFTRAJ;

// Ditto
static vector<pair<const DCoordinateSystem*,double> > TRACKHITS;
static map<const DCoordinateSystem*,double> S_VALS;


//static int colors[]={kBlue, kRed, kMagenta, kGreen, kCyan};
//static int ncolors=5;


//---------------------------------
// fmwpc_mainframe    (Constructor)
//---------------------------------
fmwpc_mainframe::fmwpc_mainframe(hdv_mainframe *hdvmf, const TGWindow *p, UInt_t w, UInt_t h):TGMainFrame(p,w,h)
{
	this->hdvmf = hdvmf;

	TGLayoutHints *lhints = new TGLayoutHints(kLHintsNormal, 2,2,2,2);
    TGLayoutHints *rhints = new TGLayoutHints(kLHintsCenterY|kLHintsRight, 2,2,2,2);
	TGLayoutHints *bhints = new TGLayoutHints(kLHintsBottom|kLHintsCenterX, 2,2,2,2);
	TGLayoutHints *thints = new TGLayoutHints(kLHintsTop|kLHintsCenterX, 2,2,2,2);
	TGLayoutHints *ahints = new TGLayoutHints(kLHintsLeft|kLHintsTop, 2,2,2,2);
	TGLayoutHints *xhints = new TGLayoutHints(kLHintsNormal|kLHintsExpandX, 2,2,2,2);
	//TGLayoutHints *dhints = new TGLayoutHints(kLHintsExpandY|kLHintsCenterY, 2,2,2,2);
	//TGLayoutHints *ehints = new TGLayoutHints(kLHintsCenterY, 2,2,2,2);

	auto *mainframe = new TGVerticalFrame(this);
	AddFrame(mainframe, ahints);

	//------- Top frame (Controls)
	TGHorizontalFrame *topframe = new TGHorizontalFrame(mainframe);
	mainframe->AddFrame(topframe, ahints);

		//------- Controls
		TGGroupFrame *controlsframe = new TGGroupFrame(topframe, "Controls", kHorizontalFrame);
		topframe->AddFrame(controlsframe, xhints);
		
			//-------- Pan, Zoom, Reset
			TGVerticalFrame *panzoomresetframe = new TGVerticalFrame(controlsframe);
			controlsframe->AddFrame(panzoomresetframe, lhints);
				TGHorizontalFrame *panzoomframe = new TGHorizontalFrame(panzoomresetframe);
				panzoomresetframe->AddFrame(panzoomframe, lhints);
				
//					//------- Pan
//					TGGroupFrame *panframe = new TGGroupFrame(panzoomframe, "Pan", kHorizontalFrame);
//					panzoomframe->AddFrame(panframe, dhints);
//						TGTextButton *panleft = new TGTextButton(panframe, "<");
//						TGVerticalFrame *updownframe = new TGVerticalFrame(panframe);
//							TGTextButton *panup = new TGTextButton(updownframe, "^");
//							TGTextButton *pandown = new TGTextButton(updownframe, "v");
//							updownframe->AddFrame(panup, dhints);
//							updownframe->AddFrame(pandown, dhints);
//						TGTextButton *panright = new TGTextButton(panframe, ">");
//						panframe->AddFrame(panleft, ehints);
//						panframe->AddFrame(updownframe, dhints);
//						panframe->AddFrame(panright, ehints);
//
//					//------- Zoom
//					TGGroupFrame *zoomframe = new TGGroupFrame(panzoomframe, "Zoom", kVerticalFrame);
//					panzoomframe->AddFrame(zoomframe, lhints);
//						TGTextButton *zoomin = new TGTextButton(zoomframe, " + ");
//						TGTextButton *zoomout = new TGTextButton(zoomframe, " - ");
//						zoomframe->AddFrame(zoomin, lhints);
//						zoomframe->AddFrame(zoomout, lhints);
				
				TGTextButton *reset = new TGTextButton(panzoomresetframe, "reset");
				panzoomresetframe->AddFrame(reset, xhints);

			//-------- Event, Info frame
			TGVerticalFrame *eventinfoframe = new TGVerticalFrame(controlsframe);
			controlsframe->AddFrame(eventinfoframe, thints);
		
				//-------- Next, Previous
				TGGroupFrame *prevnextframe = new TGGroupFrame(eventinfoframe, "Event", kHorizontalFrame);
				eventinfoframe->AddFrame(prevnextframe, thints);
					TGTextButton *prev	= new TGTextButton(prevnextframe,	"<-- Prev");
					TGTextButton *next	= new TGTextButton(prevnextframe,	"Next -->");
					prevnextframe->AddFrame(prev, lhints);
					prevnextframe->AddFrame(next, lhints);
				
					next->Connect("Clicked()","hdv_mainframe", hdvmf, "DoNext()");
					prev->Connect("Clicked()","hdv_mainframe", hdvmf, "DoPrev()");
					next->Connect("Clicked()","fmwpc_mainframe", this, "DoNewEvent()");
					prev->Connect("Clicked()","fmwpc_mainframe", this, "DoNewEvent()");
					
			    //-------- Info
                TGGroupFrame *infoframe = new TGGroupFrame(eventinfoframe, "Info", kHorizontalFrame);
                eventinfoframe->AddFrame(infoframe, thints);
                TGVerticalFrame *eventlabs = new TGVerticalFrame(infoframe);
                TGVerticalFrame *eventvals = new TGVerticalFrame(infoframe);
                infoframe->AddFrame(eventlabs, lhints);
                infoframe->AddFrame(eventvals, lhints);

                TGLabel *runlab = new TGLabel(eventlabs, "Run:");
                TGLabel *eventlab = new TGLabel(eventlabs, "Event:");
                run = new TGLabel(eventvals, "--------------");
                event = new TGLabel(eventvals, "--------------");
                eventlabs->AddFrame(runlab, rhints);
                eventlabs->AddFrame(eventlab,rhints);
                eventvals->AddFrame(run, lhints);
                eventvals->AddFrame(event, lhints);

        //-----------  Draw enable checkboxes
        TGGroupFrame *drawenableframe = new TGGroupFrame(topframe, "Draw Options", kVerticalFrame);
        topframe->AddFrame(drawenableframe, lhints);
        checkbuttons["Draw Tracks"        ] = new TGCheckButton(drawenableframe, "Draw Tracks");
        checkbuttons["Draw FMWPCHits"     ] = new TGCheckButton(drawenableframe, "Draw FMWPCHits");
        checkbuttons["Draw FMWPCClusters" ] = new TGCheckButton(drawenableframe, "Draw FMWPCClusters");
        checkbuttons["Draw FCALHits"      ] = new TGCheckButton(drawenableframe, "Draw FCALHits");
        for( auto cb : checkbuttons ){
            cb.second->SetState(kButtonDown);
            drawenableframe->AddFrame( cb.second, lhints );
            cb.second->Connect("Clicked()","fmwpc_mainframe", this, "DoMyRedraw()");
        }

	//------- Middle frame (Canvas)
	TGHorizontalFrame *midframe = new TGHorizontalFrame(mainframe);
	mainframe->AddFrame(midframe, ahints);

		//------- Canvases, top and side views
		int width=900;
		int height=300;

		TGVerticalFrame *canvasframe = new TGVerticalFrame(midframe);
		midframe->AddFrame(canvasframe, bhints);

		topcanvas = new TRootEmbeddedCanvas("FMWPC Top View Canvas", canvasframe, width, height, kSunkenFrame, GetWhitePixel());
		canvasframe->AddFrame(topcanvas, lhints);
		topcanvas->SetScrolling(TGCanvas::kCanvasScrollBoth);

		sidecanvas = new TRootEmbeddedCanvas("FMWPC Side View Canvas", canvasframe, width, height, kSunkenFrame, GetWhitePixel());
		canvasframe->AddFrame(sidecanvas, lhints);
		sidecanvas->SetScrolling(TGCanvas::kCanvasScrollBoth);

		slo = -95.0;
		shi =  95.0;
		zlo = 575.0;
		zhi = 970.0;
        topcanvas->GetCanvas()->cd();
        topcanvas->GetCanvas()->Range(zlo, slo, zhi, shi);
        sidecanvas->GetCanvas()->cd();
        sidecanvas->GetCanvas()->Range(zlo, slo, zhi, shi);

	//------- Bottom frame (Close button)
	TGHorizontalFrame *botframe = new TGHorizontalFrame(mainframe);
	mainframe->AddFrame(botframe, bhints);
    TGTextButton *closebutton = new TGTextButton(botframe,	"Close");
    botframe->AddFrame(closebutton, bhints);
    closebutton->Connect("Clicked()","fmwpc_mainframe", this, "DoClose()");

	// Finish up and map the window
	SetWindowName("Hall-D Event Viewer:FMWPC Inspector");
	SetIconName("HDView:FMWPCInspector");
	MapSubwindows();
	Resize(GetDefaultSize());
	MapWindow();
	
	RequestFocus();
	
	DoNewEvent();
}

//---------------------------------
// ~fmwpc_mainframe    (Destructor)
//---------------------------------
fmwpc_mainframe::~fmwpc_mainframe()
{
	hdvmf->DoClearTrackInspectorPointer();
}

//---------------------------------
// DoNewEvent
//---------------------------------
void fmwpc_mainframe::DoNewEvent(void)
{
	DoUpdateMenus();
	DoMyRedraw();

    // Update run, event info
    if( eventloop ) {
        auto jevent = eventloop->GetJEvent();
        std::stringstream ss;
        ss << jevent.GetRunNumber();
        run->SetTitle(ss.str().c_str());
        run->Draw();

        ss.str("");
        ss << jevent.GetEventNumber();
        event->SetTitle(ss.str().c_str());
        event->Draw();

    }
}

//---------------------------------
// DoMyRedraw
//---------------------------------
void fmwpc_mainframe::DoMyRedraw(void)
{
	// Delete any existing graphics objects
	for( auto g : topgraphics )delete g;
	for( auto g : sidegraphics )delete g;
	topgraphics.clear();
	sidegraphics.clear();
    graphics_draw_options.clear();

	// Draw axes and scales
	topcanvas->GetCanvas()->cd();
	topcanvas->GetCanvas()->Range(zlo, slo, zhi, shi);
	DrawAxes(topcanvas->GetCanvas(), topgraphics, "z(cm)", "x(cm)");

    sidecanvas->GetCanvas()->cd();
    sidecanvas->GetCanvas()->Range(zlo, slo, zhi, shi);
    DrawAxes(sidecanvas->GetCanvas(), sidegraphics, "z(cm)", "y(cm)");

    // Draw detectors
	DrawDetectors(topcanvas->GetCanvas(), topgraphics,  "top");
	DrawDetectors(topcanvas->GetCanvas(), sidegraphics, "side");

    // Draw everything on top view
	topcanvas->GetCanvas()->cd(0);
	for( auto g : topgraphics ){
        const char *draw_options = graphics_draw_options.count(g) ? graphics_draw_options[g].c_str():"";
        g->Draw(draw_options);
    }
	topcanvas->GetCanvas()->Update();

    // Draw everything on side view
    sidecanvas->GetCanvas()->cd(0);
	for( auto g : sidegraphics ){
        const char *draw_options = graphics_draw_options.count(g) ? graphics_draw_options[g].c_str():"";
        g->Draw(draw_options);
    }
    sidecanvas->GetCanvas()->Update();

}

//---------------------------------
// DoHitSelect
//---------------------------------
void fmwpc_mainframe::DoHitSelect(void)
{

}

//---------------------------------
// DoUpdateMenus
//---------------------------------
void fmwpc_mainframe::DoUpdateMenus(void)
{
// 	for(unsigned int i=0; i<factorytag.size(); i++){
// 		FillTrackNumberComboBox(trackno[i], datatype[i], factorytag[i], i!=0);
// 	}
}

//---------------------------------
// DoTagMenuUpdate
//---------------------------------
void fmwpc_mainframe::DoTagMenuUpdate(Int_t widgetId, Int_t id)
{
// 	if(widgetId>=0 && widgetId<(int)datatype.size()){
// 		FillFactoryTagComboBox(factorytag[widgetId], datatype[widgetId], factorytag[widgetId]->GetTextEntry()->GetText());
// 		factorytag[widgetId]->EmitVA("Selected(Int_t, Int_t)", 2, widgetId, id);
// 	}
}

//---------------------------------
// DoTrackNumberMenuUpdate
//---------------------------------
void fmwpc_mainframe::DoTrackNumberMenuUpdate(Int_t widgetId, Int_t id)
{
// 	if(widgetId>=0 && widgetId<(int)datatype.size()){
// 		FillTrackNumberComboBox(trackno[widgetId], datatype[widgetId], factorytag[widgetId], widgetId!=0);
// 		DoMyRedraw();
// 	}
}

//---------------------------------
// DoRequestFocus
//---------------------------------
void fmwpc_mainframe::DoRequestFocus(Int_t id)
{
	RequestFocus();
}

//---------------------------------
// DoClose
//---------------------------------
void fmwpc_mainframe::DoClose(void)
{
    LowerWindow();
}

//-------------------
// DrawAxes
//-------------------
void fmwpc_mainframe::DrawAxes(TCanvas *c, vector<TObject*> &graphics, const char *xlab, const char *ylab)
{
	/// Create arrows indicating x and y axes with labels on the specified canvas
	/// and add them to the specified container of graphics objects to be draw later.
	double x1 = c->GetX1();
	double x2 = c->GetX2();
	double y1 = c->GetY1();
	double y2 = c->GetY2();
	double deltax = x2-x1;
	double deltay = y2-y1;
	double xlo = x1+0.015*deltax;
	double xhi = xlo + 0.030*deltax;
	double ylo = y1+0.015*deltay;
	double yhi = ylo + 0.10*deltay;
	TArrow *yarrow = new TArrow(xlo, ylo, xlo, yhi, 0.02, ">");
	yarrow->SetLineWidth((Width_t)1.5);
	graphics.push_back(yarrow);
	
	TLatex *ylabel = new TLatex(xlo, yhi+0.005*deltay, ylab);
	ylabel->SetTextAlign(12);
	ylabel->SetTextAngle(90.0);
	ylabel->SetTextSize(0.04);
	graphics.push_back(ylabel);
	
	TArrow *xarrow = new TArrow(xlo, ylo, xhi, ylo, 0.02, ">");
	xarrow->SetLineWidth((Width_t)1.5);
	graphics.push_back(xarrow);
	
	TLatex *xlabel = new TLatex(xhi+0.005*deltax, ylo, xlab);
	xlabel->SetTextAlign(12);
	xlabel->SetTextSize(0.04);
	graphics.push_back(xlabel);
	
	// Left axis (and grid lines)
	xlo = x1+0.08*deltax;
	ylo = y1+0.05*deltay;
	yhi = y1+0.95*deltay;
	TGaxis *axis = new TGaxis(xlo , ylo, xlo, yhi, ylo, yhi, 510, "-RW");
    axis->SetGridLength((yhi-ylo)/deltay - 0.05);
	graphics.push_back(axis);

	// Right axis
	xhi = x1+0.95*deltax;
	axis = new TGaxis(xhi , ylo, xhi, yhi, ylo, yhi, 510, "+U");
	graphics.push_back(axis);

	// Top axis
	axis = new TGaxis(xlo , yhi, xhi, yhi, xlo, xhi, 510, "-R");
	graphics.push_back(axis);

	// Bottom axis (and grid lines)
	axis = new TGaxis(xlo , ylo, xhi, ylo, xlo, xhi, 510, "+UW");
	graphics.push_back(axis);
	axis = new TGaxis(xlo+0.2*deltax, ylo, xhi, ylo, xlo+0.2*deltax, xhi, 507, "+L");
    axis->SetGridLength((xhi-xlo)/deltax + 0.05);
	graphics.push_back(axis);
	
	// Center line
	TLine *l = new TLine(0.0, ylo, 0.0, yhi);
	graphics.push_back(l);

    // Draw Label
    auto x = (xhi-xlo)*0.05 + xlo;
    auto y = (yhi-ylo)*0.9 + ylo;
    auto lab = new TLatex(x, y, std::string(ylab)=="y(cm)" ? "Side View": "Top View");
    lab->SetTextSize(0.08);
    graphics.push_back( lab );
}


//-------------------
// DrawDetectors
//-------------------
void fmwpc_mainframe::DrawDetectors(TCanvas *c, vector<TObject*> &graphics, std::string view)
{

    // Draw chambers
    for( auto z : FMWPC_Zpos ){
        Float_t zmin = z - FMWPC_Zlen/2.0;
        Float_t zmax = zmin + FMWPC_Zlen;
        Float_t smin = -FMWPC_width/2.0;
        Float_t smax = +FMWPC_width/2.0;
        auto box = new TBox(zmin, smin, zmax, smax);
        box->SetLineWidth(1);
        box->SetLineColor(kBlack);
        box->SetFillStyle(3001);
        box->SetFillColor(TColor::GetColor((Float_t)0.9, 0.9, 0.9));
        graphics.push_back( box );
        graphics_draw_options[box] = "L"; // Draw outline AND fill style
    }

    // Draw Absorbers
    for(uint32_t i=0; i<FMWPC_Zpos.size()-2; i++){
        Float_t z_upstream = FMWPC_Zpos[i];   // center of upstream chamber
        Float_t z_dnstream = FMWPC_Zpos[i+1]; // center of downstream chamber
        Float_t zmin = z_upstream + FMWPC_Zlen/2.0 + 1.0;
        Float_t zmax = z_dnstream - FMWPC_Zlen/2.0 - 1.0;
        Float_t smin = -FMWPC_width/2.0 - 2.0;
        Float_t smax = +FMWPC_width/2.0 + 2.0;        
        auto box = new TBox(zmin, smin, zmax, smax);
        box->SetFillStyle(3001);
        box->SetFillColor(42);
        graphics.push_back( box );
    }

    // Get all JANA objects
    vector<const DTrackTimeBased*> tbts;
    vector<const DFCALHit*> fcalhits;
    vector<const DFMWPCHit*> fmwpchits;
    vector<const DFMWPCCluster*> fmwpcclusters;
    vector<const DFMWPCMatchedTrack*> fmwpcmatchedtracks;
    if( eventloop != NULL ) {
        eventloop->Get(tbts);
        eventloop->Get(fcalhits);
        eventloop->Get(fmwpchits);
        eventloop->Get(fmwpcclusters);
        eventloop->Get(fmwpcmatchedtracks);
    }

    // Draw FCALHits
    if( checkbuttons["Draw FCALHits"]->GetState() == kButtonDown) {

        // We want to draw the FCAL objects in order from lowest
        // energy to highest so that the highest energy hit is
        // drawn last.
        std::multimap<Double_t, TObject*> objs;

        for (auto hit : fcalhits) {

            auto z_up = FCAL_Zmin;
            auto z_dn = FCAL_Zmin + FCAL_Zlen;
            auto s = (view == "top" ) ? hit->x:hit->y;
            auto u = (view == "top" ) ? hit->y:hit->x;
            auto s_min = s - 2.0;     // assume FCAL blocks are 4cmx4cm
            auto s_max = s_min + 4.0; // assume FCAL blocks are 4cmx4cm

            // In order to draw some FCAL info for blocks on the same
            // row/column we use the dimension going into the screen
            // "u" to shift the block forward or backward slightly
            // in z.
            double z_shift = -u*40.0/120.0;
            z_up += z_shift;
            //z_dn += z_shift;

            auto b = new TBox(z_up, s_min, z_dn, s_max);
            // Get number for log of energy deposited that is scaled to be
            // 0.0 at E=0 and 1.0 at E>=1.5GeV
            double color = log(hit->E/0.01)/log(0.5/0.01);  // color=0 for E=10MeV; color=1 for E=1.0GeV
            color = color<0.0 ? 0.0:(color>1.0 ? 1.0:color);  // clip values to keep color in 0-1 range

            // For 0.0<E<0.5 use color to set G,B so color is light red
            // For 0.5<E<1.0 use color to set R so color is dark red
            float R = 1.0;
            float G = 1.0 - 2.0*color;
            float B = G;
            if( color>0.5 ){
                R = 2.0*color;
                G = 0.0;
                B = G;
            }
            b->SetLineColor(kRed+4);
            b->SetFillColor(TColor::GetColor( R, G, B ));
            b->SetFillStyle(1001);
            objs.insert( std::pair<Double_t, TObject*>(-u, b) );
            graphics_draw_options[b] = "L";
        }

        // Add objects in energy order
        for( auto &p : objs ) graphics.push_back( p.second );
    }

    // Draw FMWPCClusters
    if( checkbuttons["Draw FMWPCClusters"]->GetState() == kButtonDown) {
        for (auto cluster: fmwpcclusters) {

            // layers 1,3,5=vertical   layers 2,4,6=horizontal
            if ((view == "top" ) && ((cluster->layer % 2) == 0)) continue; // skip horizontal wires if drawing vertical wires
            if ((view == "side") && ((cluster->layer % 2) == 1)) continue; // skip vertical wires if drawing horizontal wires

            int first_wire = cluster->first_wire;
            int last_wire  = cluster->last_wire;
            double s_first = (first_wire * FMWPC_WIRE_SPACING) - 71.5 - 0.5*FMWPC_WIRE_SPACING;
            double s_last  = (last_wire  * FMWPC_WIRE_SPACING) - 71.5 + 0.5*FMWPC_WIRE_SPACING;
            double z = (uint32_t)(cluster->layer) <= FMWPC_Zpos.size() ? FMWPC_Zpos[cluster->layer - 1] : 0.0;
            double z_up = z - FMWPC_Zlen/2.0 - 1.0;
            double z_dn = z + FMWPC_Zlen/2.0 + 1.0;
            auto b = new TBox(z_up, s_first, z_dn, s_last);
            b->SetLineColor(kMagenta);
            graphics.push_back(b);
            graphics_draw_options[b] = "L";
        }
    }

    // Draw FMWPCHits
    if( checkbuttons["Draw FMWPCHits"]->GetState() == kButtonDown) {
        for (auto hit: fmwpchits) {

            // layers 1,3,5=vertical   layers 2,4,6=horizontal
            if ((view == "top" ) && ((hit->layer % 2) == 0)) continue; // skip horizontal wires if drawing vertical wires
            if ((view == "side") && ((hit->layer % 2) == 1)) continue; // skip vertical wires if drawing horizontal wires

            double s = (hit->wire * FMWPC_WIRE_SPACING) - 71.5;
            double z = (uint32_t)(hit->layer) <= FMWPC_Zpos.size() ? FMWPC_Zpos[hit->layer - 1] : 0.0;
            auto m = new TMarker(z, s, 8);
            m->SetMarkerColor(kRed);
            m->SetMarkerSize(1.0);
            graphics.push_back(m);
        }
    }

    // Draw Tracks
    if( checkbuttons["Draw Tracks"]->GetState() == kButtonDown) {
        for (auto tbt: tbts) {

            // Vectors to hold values of markers drawn so we can draw a spline
            vector <Double_t> Z;
            vector <Double_t> S;

            if ((tbt->PID() == 8) || (tbt->PID() == 9)) {
                auto fcal_projections = tbt->extrapolations.at(SYS_FCAL);
 //             if( ! fcal_projections.empty() ){
                for( auto proj : fcal_projections ){
//                    auto proj = fcal_projections[fcal_projections.size()-1];
                    double s = 0;
                    double z = proj.position.z();
                    if (view == "top") {
                        s = proj.position.x();
                    } else if (view == "side") {
                        s = proj.position.y();
                    }
                    auto m = new TMarker(z, s, 47);
                    m->SetMarkerColor(kBlue+2);
                    m->SetMarkerSize(2.0);
                    graphics.push_back(m);

                    S.push_back(s);
                    Z.push_back(z);
                }

                auto fmwpc_projections = tbt->extrapolations.at(SYS_FMWPC);
                for (int layer = 1; layer <= (int) fmwpc_projections.size(); layer++) {
                    auto idx = layer - 1;
                    auto proj = fmwpc_projections[idx];
                    double s = 0;
                    double z = proj.position.z();
                    if (view == "top") {
                        s = proj.position.x();
                    } else if (view == "side") {
                        s = proj.position.y();
                    }
                    auto m = new TMarker(z, s, 47);
                    m->SetMarkerColor(kBlue);
                    m->SetMarkerSize(1.0);
                    graphics.push_back(m);

                    S.push_back(s);
                    Z.push_back(z);
                }

                if (S.size() > 1) {
                    auto sp = new TSpline3("spline", Z.data(), S.data(), S.size());
                    sp->SetLineColor(kBlue);
                    graphics.push_back(sp);
                    graphics_draw_options[sp] = "same"; // Draw outline AND fill style
                }
            }
        }
    }
}

//-------------------
// DrawHits
//-------------------
void fmwpc_mainframe::DrawHits(vector<TObject*> &graphics)
{
// 	// Find the factory name, tag, and track number for the prime track
// 	string dataname = datatype[0]->GetTextEntry()->GetText();
// 	string tag = factorytag[0]->GetTextEntry()->GetText();
// 	string track = trackno[0]->GetTextEntry()->GetText();
// 
// 	// Reset residual histogram
// 	this->resi->Reset();
// 
// 	if(track==""){_DBG_<<"No prime tracks!"<<endl;return;}
// 	if(tag=="<default>")tag="";
// 	unsigned int index = atoi(track.c_str());
// 	
// 	// Clear out any existing reference trajectories
// 	for(unsigned int i=0; i<REFTRAJ.size(); i++)delete REFTRAJ[i];
// 	REFTRAJ.clear();
// 	
// 	// Get the reference trajectory for the prime track
// 	DReferenceTrajectoryHDV *rt=NULL;
// 	vector<const DCDCTrackHit*> cdctrackhits;
// 	gMYPROC->GetDReferenceTrajectory(dataname, tag, index, rt, cdctrackhits);
// 	if(rt==NULL){
// 		_DBG_<<"Reference trajectory unavailable for "<<dataname<<":"<<tag<<" #"<<index<<endl;
// 		return;
// 	}
// 	
// 	REFTRAJ.push_back(rt);
// 
// 	// Get a list of ALL wire hits for this event
// 	vector<pair<const DCoordinateSystem*,double> > allhits;
// 	gMYPROC->GetAllWireHits(allhits);
// 	
// 	// Draw prime track
// 	DrawHitsForOneTrack(graphics, allhits, rt, 0, cdctrackhits);
// 	
// 	// Draw other tracks
// 	for(unsigned int i=1; i<datatype.size(); i++){
// 		dataname = datatype[i]->GetTextEntry()->GetText();
// 		tag = factorytag[i]->GetTextEntry()->GetText();
// 		track = trackno[i]->GetTextEntry()->GetText();
// 		if(track=="")continue;
// 		if(tag=="<default>")tag="";
// 		unsigned int index = atoi(track.c_str());
// 		if(track=="Best Match"){
// 			// Need to implement algorithm to find the best match
// 			index=0;
// 		}
// 		
// 		// Get reference trajectory for this track
// 		DReferenceTrajectoryHDV *myrt=NULL;
// 		gMYPROC->GetDReferenceTrajectory(dataname, tag, index, myrt, cdctrackhits);
// 		if(myrt){
// 			REFTRAJ.push_back(myrt);
// 			DrawHitsForOneTrack(graphics, allhits, myrt, i, cdctrackhits);
// 		}
// 	}
}

// //-------------------
// // DrawHitsForOneTrack
// //-------------------
// void fmwpc_mainframe::DrawHitsForOneTrack(
// 	vector<TObject*> &graphics,
// 	vector<pair<const DCoordinateSystem*,double> > &allhits,
// 	DReferenceTrajectory *rt,
// 	int index,
// 	vector<const DCDCTrackHit*> &cdctrackhits)
// {
// 	// Clear current hits list
// 	if(index==0){
// 		TRACKHITS.clear();
// 		S_VALS.clear();
// 		slo = shi = 20.0;
// 	}
// 	
// 	// Get state of s-lock checkbutton
// 	bool lock_s_coordinate = slock->GetState();
// 
//     // Get fdc drift time - distance function
//     vector<const DTrackFitter*> fitters;
//     eventloop->Get(fitters, "KalmanSIMD");
//     const DTrackFitterKalmanSIMD *fitter=0;
//     if (fitters.size() > 0)
//         fitter =  dynamic_cast<const DTrackFitterKalmanSIMD *>(fitters[0]);
// 
// 
// 	vector<pair<const DCoordinateSystem*,double> > &hits = index==0 ? allhits:TRACKHITS;
// 	
// 	// Loop over all hits and create graphics objects for each
// 	for(unsigned int i=0; i<hits.size(); i++){
// 		const DCoordinateSystem *wire = hits[i].first;
// 		double dist = hits[i].second;
// 		DVector3 pos_doca, mom_doca;
// 		double s;
// 		if(wire==NULL){_DBG_<<"wire==NULL!!"<<endl; continue;}
// 		if(rt==NULL){_DBG_<<"rt==NULL!!"<<endl; continue;}
// 		double doca = rt->DistToRT(wire, &s);
// 		rt->GetLastDOCAPoint(pos_doca, mom_doca);
// 		DVector3 shift = wire->udir.Cross(mom_doca);
// 		
// 		// The magnitude of "dist" is based on the drift time
// 		// which does not yet subtract out the TOF. This can add
// 		// 50-100 microns to the resolution.
// 		//
// 		// What is really needed here is to try different hypotheses
// 		// as to the particle type. For now, we just assume its a pion
// 		double mass = 0.13957;
// 		double beta = 1.0/sqrt(1.0 + pow(mass/mom_doca.Mag(), 2.0))*2.998E10;
// 		double tof = s/beta/1.0E-9; // in ns
//         if (fitter) {
//             double Bz = rt->FindClosestSwimStep(wire)->B[2];
//             dist = fitter->GetFDCDriftDistance(dist / 55.0e-4 - tof, Bz);
// #if PRINT_DRIFT_DISTANCE_MAP
//             static bool print_the_map=true;
//             if (print_the_map) {
//                print_the_map = false;
//                for (double t=0; t < 2.5; t += 0.001) {
//                   double d = fitter->GetFDCDriftDistance(t, Bz);
//                   std::cout << d << " " << t << std::endl;
//                }
//             }
// #endif
//         }
//         else {
// 		    dist -= tof*55.0E-4;
//         }
// 		shift.SetMag(dist);
// 
// 		// See comments in DTrack_factory_ALT1.cc::LeastSquaresB
// 		double u = rt->GetLastDistAlongWire();
// 		DVector3 pos_wire = wire->origin + u*wire->udir;
// 		DVector3 pos_diff = pos_doca-pos_wire;
// 		double sdist = pos_diff.Mag();
// 		if(shift.Dot(pos_diff)<0.0){
// 			shift = -shift;
// 			sdist = -sdist;
// 		}
// 		
// 		// OK. Finally, we can decide on a sign for the residual.
// 		// We do this by taking the dot product of the shift with
// 		// the vector pointing to the center of the wire.
// 		//double sign = (shift.Dot(pos_wire)<0.0) ? -1.0:+1.0;
// 		double resi = fabs(doca)-fabs(dist);
// 		if(!isfinite(resi))continue;
// #if VERBOSE_PRINT_FDC_HIT_INFO
//          std::cout
//            << "hit " << i << ": "
//            << "dist=" << dist
//            << " (" << shift[0] << "," << shift[1] << "," << shift[2] << ")"
//            << ", sdist=" << sdist
//            << " (" << pos_diff[0] << "," << pos_diff[1] << "," << pos_diff[2] << ")"
//            << ", tof=" << tof
//            << " at (" << pos_doca[0] << "," << pos_doca[1] << "," << pos_doca[2] << ")"
//            << std::endl;
// #endif
// 		// If the residual is reasonably small, consider this hit to be
// 		// on this track and record it (if this is the prime track)
// 		if(index==0)TRACKHITS.push_back(hits[i]);
// 		
// //_DBG_<<"resi="<<resi<<"  s="<<s<<"   resi*10E4="<<resi*1.0E4<<endl;
// 		
// 		if(index==0){
// 			TMarker *m = new TMarker(sdist, s, 20);
// 			m->SetMarkerSize(1.6);
// 			m->SetMarkerColor(kYellow);
// 			graphics.push_back(m);
// 			this->resi->Fill(resi);
// 			
// 			// Record limits for s.
// 			// NOTE: We calculate zlo and zhi from these later
// 			// in DoMyRedraw().
// 			if(s<slo)slo=s;
// 			if(s>shi)shi=s;
// 		}
// 		
// 		// Check if this is a CDC wire.
// 		int marker_style = 20;
// 		double ellipse_width = 0.8;
// 		int ellipse_color = colors[index%ncolors];
// 		const DCDCWire *cdcwire = dynamic_cast<const DCDCWire*>(wire);
// 		if(cdcwire!=NULL && cdcwire->stereo!=0.0){
// 			ellipse_width = 3.0;
// 			ellipse_color += cdcwire->stereo>0.0 ? 4:-2;
// 			marker_style = 5;
// 		}
// 		
// 		// Check if this is wire is in the list of wires associated with this track
// 		int ellipse_style = 1;
// 		if(!WireInList(wire, cdctrackhits)){
// 			ellipse_style=2;
// 			ellipse_width=2.0;
// 		}
// 		
// 		// If the lock_s_coordinate flag is set and this is not the prime track
// 		// then try and replace the s-value for this hit by the one from the 
// 		// prime track. If this is the prime track, then record the s-value.
// 		if(lock_s_coordinate){
// 			if(index==0){
// 				// This is prime track. Record s-value for this wire
// 				S_VALS[wire] = s;
// 			}else{
// 				map<const DCoordinateSystem*,double>::iterator iter = S_VALS.find(wire);
// 				if(iter!=S_VALS.end()){
// 					s = iter->second;
// 				}
// 			}
// 		}
// 
// 		// Create ellipse for distance from wire
// 		TEllipse *e = new TEllipse(sdist, s, dist, dist);
// 		e->SetLineWidth((Width_t)ellipse_width);
// 		e->SetLineColor(ellipse_color);
// 		e->SetLineStyle(ellipse_style);
// 		e->SetFillColor(19);
// 		graphics.push_back(e);
// 
// 		// Create marker for wire
// 		TMarker *m = new TMarker(sdist, s, marker_style);
// 		m->SetMarkerSize(1.5);
// 		m->SetMarkerColor(colors[index%ncolors]);
// 		graphics.push_back(m);
// 		
// 	}
// }

// //-------------------
// // WireInList
// //-------------------
// bool fmwpc_mainframe::WireInList(const DCoordinateSystem *wire, vector<const DCDCTrackHit*> &cdctrackhits)
// {
// 	for(unsigned int i=0; i<cdctrackhits.size(); i++){
// 		if(cdctrackhits[i]->wire == wire)return true;
// 	}
// 
// 	return false;
// }
