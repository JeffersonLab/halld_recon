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
#include "EventViewer.h"
#include "DReferenceTrajectoryHDV.h"

#include <CDC/DCDCWire.h>
#include <CDC/DCDCTrackHit.h>
#include <FCAL/DFCALHit.h>
#include <TRACKING/DTrackFitterKalmanSIMD.h>
#include <TRACKING/DTrackTimeBased.h>
#include <FMWPC/DCTOFDigiHit.h>
#include <FMWPC/DCTOFTDCDigiHit.h>
#include <FMWPC/DCTOFHit.h>
#include <FMWPC/DCTOFPoint.h>

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

// Defined in hdv_mainframe.cc
extern float FCAL_Zlen;
extern float FCAL_Zmin;
extern float FMWPC_width;
extern float FMWPC_Zlen;
extern float FMWPC_Dead_diameter;
extern float FMWPC_WIRE_SPACING;
extern vector<double> FMWPC_Zpos;
extern float CTOF_width;  // 20
extern float CTOF_length; // 120
extern float CTOF_depth;  // 1.27
extern vector<DVector3> CTOF_pos;  // from DGeometry::GetCTOFPositions()


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
					next = new TGTextButton(prevnextframe,	"Next -->");
					prevnextframe->AddFrame(prev, lhints);
					prevnextframe->AddFrame(next, lhints);
				
					next->Connect("Clicked()","hdv_mainframe", hdvmf, "DoNext()");
					prev->Connect("Clicked()","hdv_mainframe", hdvmf, "DoPrev()");
                    prev->SetEnabled(false);
					
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
        checkbuttons["Draw FMWPC"         ] = new TGCheckButton(drawenableframe, "Draw FMWPC");
        checkbuttons["Draw CTOF"          ] = new TGCheckButton(drawenableframe, "Draw CTOF");
        checkbuttons["Draw Tracks"        ] = new TGCheckButton(drawenableframe, "Draw Tracks");
        checkbuttons["Draw Track Hit Projections"] = new TGCheckButton(drawenableframe, "Draw Track Hit Projections");
        checkbuttons["Draw FMWPCHits"     ] = new TGCheckButton(drawenableframe, "Draw DFMWPCHits");
        checkbuttons["Draw FMWPCDigiHits" ] = new TGCheckButton(drawenableframe, "Draw DFMWPCDigiHits");
        checkbuttons["Draw FMWPCClusters" ] = new TGCheckButton(drawenableframe, "Draw DFMWPCClusters");
        checkbuttons["Draw FCALHits"      ] = new TGCheckButton(drawenableframe, "Draw DFCALHits");
        checkbuttons["Draw CTOFDigiHits"  ] = new TGCheckButton(drawenableframe, "Draw CTOFDigiHits");
        checkbuttons["Draw CTOFTDCDigiHits"] = new TGCheckButton(drawenableframe, "Draw CTOFTDCDigiHits");
        checkbuttons["Draw CTOFHits"      ] = new TGCheckButton(drawenableframe, "Draw CTOFHits");
        checkbuttons["Draw CTOFPoints"    ] = new TGCheckButton(drawenableframe, "Draw CTOFPoints");
        checkbuttons["Draw Vertex Momentum Values"] = new TGCheckButton(drawenableframe, "Draw Vertex Momentum Values");
        checkbuttons["Draw Projection Momentum Values"] = new TGCheckButton(drawenableframe, "Draw Projection Momentum Values");
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
		zhi = 985.0;
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

    const auto& current_jevent = gMYPROC->GetCurrentEvent();

    // Update run, event info
    std::stringstream ss;
    ss << current_jevent.GetRunNumber();
    run->SetTitle(ss.str().c_str());
    run->Draw();

    ss.str("");
    ss << current_jevent.GetEventNumber();
    event->SetTitle(ss.str().c_str());
    event->Draw();

    next->SetEnabled(true);
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
    const auto& event = gMYPROC->GetCurrentEvent();

    // Draw chambers
    int wires_into_screen = view=="top"; // true=wires into screen ; false=wires parallel to screen
    if( checkbuttons["Draw FMWPC"]->GetState() == kButtonDown) {
        for (auto z: FMWPC_Zpos) {
            Float_t zmin = z - FMWPC_Zlen / 2.0;
            Float_t zmax = zmin + FMWPC_Zlen;
            Float_t smin = -FMWPC_width / 2.0;
            Float_t smax = +FMWPC_width / 2.0;
            auto box = new TBox(zmin, smin, zmax, smax);
            box->SetLineWidth(1);
            box->SetLineColor(wires_into_screen ? kBlack : kGray);
            box->SetFillStyle(3001);
            box->SetFillColor(wires_into_screen ? TColor::GetColor((Float_t) 0.9, 0.9, 0.9) : kGray);
            graphics.push_back(box);
            graphics_draw_options[box] = "L"; // Draw outline AND fill style

            wires_into_screen = !wires_into_screen;
        }

        // Draw Absorbers
		  if( FMWPC_Zpos.size() < 2 ){
			jout << "FMWPC_Zpos.size()=" << FMWPC_Zpos.size() <<" which is <2. Unable to draw FMWPC." << std::endl;
			jout << "Please make sure the CPP geometry is being used. e,g." << std::endl;
			jout << "  setenv JANA_CALIB_CONTEXT \"variation=mc_cpp\"" << std::endl;
		  }else{
      	  for (uint32_t i = 0; i < FMWPC_Zpos.size() - 2; i++) {
            	Float_t z_upstream = FMWPC_Zpos[i];   // center of upstream chamber
            	Float_t z_dnstream = FMWPC_Zpos[i + 1]; // center of downstream chamber
            	Float_t zmin = z_upstream + FMWPC_Zlen / 2.0 + 1.0;
            	Float_t zmax = z_dnstream - FMWPC_Zlen / 2.0 - 1.0;
            	Float_t smin = -FMWPC_width / 2.0 - 2.0;
            	Float_t smax = +FMWPC_width / 2.0 + 2.0;
            	auto box = new TBox(zmin, smin, zmax, smax);
            	box->SetFillStyle(3001);
            	box->SetFillColor(42);
            	graphics.push_back(box);
      	  }
		  }
    }

    // Draw CTOF
    if( checkbuttons["Draw CTOF"]->GetState() == kButtonDown) {
        for (auto pos: CTOF_pos) {
        
            Float_t zmin = pos.Z() - CTOF_depth/2.0;
            Float_t zmax = zmin + CTOF_depth;
            Float_t smin;
            Float_t smax;
            if( view=="top" ){
                smin = pos.X() - CTOF_width/2.0;
                smax = smin + CTOF_width;
            }else{
                smin = pos.Y() - CTOF_length/2.0;
                smax = smin + CTOF_length;
            }
            auto box = new TBox(zmin, smin, zmax, smax);
            box->SetLineWidth(1);
            box->SetLineColor(kBlack);
            box->SetFillStyle(3001);
            box->SetFillColor(TColor::GetColor((Float_t) 0.9, 0.9, 0.9));
            graphics.push_back(box);
            graphics_draw_options[box] = "L"; // Draw outline AND fill style
        }
    }

    // Get all JANA objects
    vector<const DTrackTimeBased*> tbts;
    vector<const DFCALHit*> fcalhits;
    vector<const DFMWPCDigiHit*> fmwpcdigihits;
    vector<const DFMWPCHit*> fmwpchits;
    vector<const DFMWPCCluster*> fmwpcclusters;
    vector<const DFMWPCMatchedTrack*> fmwpcmatchedtracks;
    vector<const DCTOFDigiHit*> ctofdigihits;
    vector<const DCTOFTDCDigiHit*> ctoftdcdigihits;
    vector<const DCTOFHit*> ctofhits;
    vector<const DCTOFPoint*> ctofpoints;
    event.Get(tbts);
    event.Get(fcalhits);
    event.Get(fmwpcdigihits);
    event.Get(fmwpchits);
    event.Get(fmwpcclusters);
    event.Get(fmwpcmatchedtracks);
    event.Get(ctofdigihits);
    event.Get(ctoftdcdigihits);
    event.Get(ctofhits);
    event.Get(ctofpoints);

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
            //double z_shift = -u*40.0/120.0;
            //z_up += z_shift;
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

    // Draw FMWPCDigiHits
    if( checkbuttons["Draw FMWPCDigiHits"]->GetState() == kButtonDown) {
        for (auto hit: fmwpcdigihits) {

            // layers 1,3,5=vertical   layers 2,4,6=horizontal
            if ((view == "top" ) && ((hit->layer % 2) == 0)) continue; // skip horizontal wires if drawing vertical wires
            if ((view == "side") && ((hit->layer % 2) == 1)) continue; // skip vertical wires if drawing horizontal wires

            double s = (hit->wire * FMWPC_WIRE_SPACING) - 71.5;
            double z = (uint32_t)(hit->layer) <= FMWPC_Zpos.size() ? FMWPC_Zpos[hit->layer - 1] : 0.0;
            auto m = new TMarker(z, s, 22);
            m->SetMarkerColor(kRed+4);
            m->SetMarkerSize(1.0);
            graphics.push_back(m);
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

    // Draw CTOFDigiHits
    if( checkbuttons["Draw CTOFDigiHits"]->GetState() == kButtonDown) {
        for (auto hit: ctofdigihits) {

            if( (hit->bar<1) || (hit->bar>(int)CTOF_pos.size()) ){
                _DBG_<<"CTOF bar out of range (" << hit->bar << " not in range 1-" << CTOF_pos.size() << ")" << endl;
                continue;
            }

            auto pos = CTOF_pos[ hit->bar -1 ];
            double s = (view == "top") ? pos.X():pos.Y();
            double z = pos.Z();
            
            // For top view, draw circle, bot side view draw box
            TObject *m = nullptr;
            auto pedestal = (double)hit->pedestal*(double)hit->nsamples_integral/(double)hit->nsamples_pedestal;
            
            // Some empirical color scaling based on cosmic run 100288
				// Top PMT will be a shade of blue while bottom will be shade of red
            Float_t amp = 0.2 + ((double)hit->pulse_integral - pedestal)/2500.0;
            if( amp<0.2) amp = 0.2;
            if( amp>1.0) amp = 1.0;
            auto color = TColor::GetColor((Float_t) 1.0, 1.0-amp, 1.0-amp); // Default to red-ish for top PMT
				if(hit->end==1) color = TColor::GetColor((Float_t) 1.0-amp, 1.0-amp, 1.0); // blue-ish for bottom PMT

            if( view == "top" ){
                // For top view, shift the marker upstream/downstream for the top/bottom PMT so both can be seen
                if( hit->end == 0 ){
                    // top end
                    z -= 3.0/2.0 + CTOF_depth;
                }else{
                    // bottom end
                    z += 3.0/2.0 + CTOF_depth;
                }
                auto obj = new TEllipse( z, s, 3.0);
                obj->SetFillStyle(1001);
                obj->SetFillColor( color );
                m = obj;
            }else{
                // for side view, shift marker to top or bottom PMT position
                if( hit->end == 0 ){
                    // top end
                    s += CTOF_length/2.0 + 10.0; // 10.0 so tube can be drawn as 20cm long
                }else{
                    // bottom end
                    s -= CTOF_length/2.0 + 10.0; // 10.0 so tube can be drawn as 20cm long
                }
                auto obj = new TBox( z-3.0, s-10.0, z+3.0, s+10.0);
                obj->SetFillStyle(1001);
                obj->SetFillColor( color );
                m = obj;
            }
            
            if( m == nullptr ) continue;
            graphics.push_back(m);
        }
    }

    // Draw CTOFTDCDigiHits
    if( checkbuttons["Draw CTOFTDCDigiHits"]->GetState() == kButtonDown) {
        for (auto hit: ctoftdcdigihits) {

            if( (hit->bar<1) || (hit->bar>(int)CTOF_pos.size()) ){
                _DBG_<<"CTOF bar out of range (" << hit->bar << " not in range 1-" << CTOF_pos.size() << ")" << endl;
                continue;
            }

            auto pos = CTOF_pos[ hit->bar -1 ];
            double s = (view == "top") ? pos.X():pos.Y();
            double z = pos.Z();
            
            // For top view, draw circle, bot side view draw box
            TObject *m = nullptr;
            
            // Color top PMT green and bottom yellow
            auto color = TColor::GetColor((Float_t) 0.0, 1.0, 0.0); // Default to green for top PMT
				if(hit->end==1) color = TColor::GetColor((Float_t) 1.0, 1.0, 0.2); // yellow for bottom PMT

            if( view == "top" ){
                // For top view, shift the marker upstream/downstream for the top/bottom PMT so both can be seen
                if( hit->end == 0 ){
                    // top end
                    z -= 2.5/2.0 + CTOF_depth;
                }else{
                    // bottom end
                    z += 2.5/2.0 + CTOF_depth;
                }
                auto obj = new TEllipse( z, s, 2.0);
                obj->SetFillStyle(1001);
                obj->SetFillColor( color );
                m = obj;
            }else{
                // for side view, shift marker to top or bottom PMT position
                if( hit->end == 0 ){
                    // top end
                    s += CTOF_length/2.0 + 10.0; // 10.0 so tube can be drawn as 20cm long
                }else{
                    // bottom end
                    s -= CTOF_length/2.0 + 10.0; // 10.0 so tube can be drawn as 20cm long
                }
                auto obj = new TBox( z-1.5, s-8.0, z+1.5, s+8.0);
                obj->SetFillStyle(1001);
                obj->SetFillColor( color );
                m = obj;
            }
            
            if( m == nullptr ) continue;
            graphics.push_back(m);
        }
    }

    // Draw CTOFHits
    if( checkbuttons["Draw CTOFHits"]->GetState() == kButtonDown) {
        for (auto hit: ctofhits) {

            if( (hit->bar<1) || (hit->bar>(int)CTOF_pos.size()) ){
                _DBG_<<"CTOF bar out of range (" << hit->bar << " not in range 1-" << CTOF_pos.size() << ")" << endl;
                continue;
            }

            auto pos = CTOF_pos[ hit->bar -1 ];
            double s = (view == "top") ? pos.X():pos.Y();
            double z = pos.Z();
            
            // For top view, draw circle, bot side view draw box
            TObject *m = nullptr;
            
            // Some empirical color scaling
				// Top PMT will be a shade of blue while bottom will be shade of red
            Float_t amp = 0.2 + (double)hit->dE/0.100;
            if( amp<0.2) amp = 0.2;
            if( amp>1.0) amp = 1.0;
            auto color = TColor::GetColor((Float_t) 1.0, 1.0-amp, 1.0-amp); // Default to red-ish for top PMT
				if(hit->end==1) color = TColor::GetColor((Float_t) 1.0-amp, 1.0-amp, 1.0); // blue-ish for bottom PMT

            if( view == "top" ){
                // For top view, shift the marker upstream/downstream for the top/bottom PMT so both can be seen
                if( hit->end == 0 ){
                    // top end
                    z -= 2.5/2.0 + CTOF_depth;
                }else{
                    // bottom end
                    z += 2.5/2.0 + CTOF_depth;
                }
                auto obj = new TEllipse( z, s, 1.0);
                obj->SetFillStyle(1001);
                obj->SetFillColor( color );
                m = obj;
            }else{
                // for side view, shift marker to top or bottom PMT position
                if( hit->end == 0 ){
                    // top end
                    s += CTOF_length/2.0 + 10.0; // 10.0 so tube can be drawn as 20cm long
                }else{
                    // bottom end
                    s -= CTOF_length/2.0 + 10.0; // 10.0 so tube can be drawn as 20cm long
                }
                auto obj = new TBox( z-1.0, s-6.0, z+1.0, s+6.0);
                obj->SetFillStyle(1001);
                obj->SetFillColor( color );
                m = obj;
            }
            
            if( m == nullptr ) continue;
            graphics.push_back(m);
        }
    }

    // Draw CTOFPoints
    if( checkbuttons["Draw CTOFPoints"]->GetState() == kButtonDown) {
        for (auto point: ctofpoints) {


            double s = (view == "top") ? point->pos.X():point->pos.Y();
            double z = point->pos.Z();
            
            // Some empirical color scaling
				// Top PMT will be a shade of blue while bottom will be shade of red
            Float_t amp = 0.4 + (double)point->dE/0.002;
            if( amp<0.4) amp = 0.4;
            if( amp>1.0) amp = 1.0;
            auto color = TColor::GetColor((Float_t) 1.0, 1.0-amp, 1.0); // Default to magenta-ish
				auto m = new TMarker( z, s, 22 );
				m->SetMarkerColor( color );
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
                for( auto proj : fcal_projections ){
                    double s = 0;
                    double z = proj.position.z();
                    if (view == "top") {
                        s = proj.position.x();
                    } else if (view == "side") {
                        s = proj.position.y();
                    }

                    if(checkbuttons["Draw Track Hit Projections"]->GetState() == kButtonDown) {
                        if( wires_into_screen ) {
                            auto m = new TMarker(z, s, 47);
                            m->SetMarkerColor(kBlue + 2);
                            m->SetMarkerSize(2.0);
                            graphics.push_back(m);
                        }
                    }

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
                        wires_into_screen = (layer%2) == 1;
                    } else if (view == "side") {
                        s = proj.position.y();
                        wires_into_screen = (layer%2) == 0;
                    }

                    if( wires_into_screen ) {
                        if(checkbuttons["Draw Track Hit Projections"]->GetState() == kButtonDown) {
                            auto m = new TMarker(z, s, 47);
                            m->SetMarkerColor(kBlue);
                            m->SetMarkerSize(1.0);
                            graphics.push_back(m);
                        }

                        if(checkbuttons["Draw Projection Momentum Values"]->GetState() == kButtonDown) {
                            double x = proj.position.z();
                            double y = s+1;
                            char str[256];
                            sprintf(str, "p=%3.1f GeV/c", proj.momentum.Mag());
                            auto lat = new TLatex(x, y, str);
                            lat->SetTextSize(0.04);
                            lat->SetTextColor(kBlue);
                            lat->SetTextAlign(21);
                            graphics.push_back(lat);
                        }
                    }

                    S.push_back(s);
                    Z.push_back(z);
                }

                if (S.size() > 1) {
                    auto sp = new TSpline3("spline", Z.data(), S.data(), S.size());
                    sp->SetLineColor(kBlue);
                    graphics.push_back(sp);
                    graphics_draw_options[sp] = "same"; // Draw outline AND fill style

                    if(checkbuttons["Draw Vertex Momentum Values"]->GetState() == kButtonDown) {
                        double x = FCAL_Zmin - 4.0 ; //750.0;
                        double y = sp->Eval(x);
                        char str[256];
                        sprintf(str, "p=%3.1f GeV/c", tbt->pmag());
                        auto lat = new TLatex(x,y, str);
                        lat->SetTextColor(kBlue);
                        lat->SetTextAlign(32);
                        graphics.push_back(lat);
                    }
                }
            }
        }
    }
}

void fmwpc_mainframe::EnableControls(bool enabled) {
    if (next != nullptr) {
        next->SetEnabled(enabled);
    }
}

