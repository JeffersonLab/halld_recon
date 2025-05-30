
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cmath>
#include <fstream>
using namespace std;

#include <TRACKING/DMCThrown.h>
#include "hdv_endviewBframe.h"
#include "FDC/DFDCGeometry.h"
#include "FCAL/DFCALGeometry.h"
#include "DVector2.h"
#include "HDGEOMETRY/DGeometry.h"
#include <PID/DNeutralParticle.h>

#include <TPolyMarker.h>
#include <TLine.h>
#include <TMarker.h>
#include <TBox.h>
#include <TVector3.h>
#include <TGeoVolume.h>
#include <TGeoManager.h>
#include <TGLabel.h>
#include <TGComboBox.h>
#include <TGButton.h>
#include <TGButtonGroup.h>
#include <TGTextEntry.h>
#include <TArrow.h>
#include <TLatex.h>
#include <TColor.h>
#include <TG3DLine.h>


//-------------------
// Constructor
//-------------------
hdv_endviewBframe::hdv_endviewBframe(hdv_mainframe *hdvmf, const TGWindow *p, UInt_t w, UInt_t h):TGMainFrame(p,w,h)
{
	this->hdvmf = hdvmf;

	// First, define all of the of the graphics objects. Below that, make all
	// of the connections to the methods so these things will work!

	// The main GUI window is divided into three sections, top, middle, and bottom.
	// Create those frames here.
	TGLayoutHints *lhints = new TGLayoutHints(kLHintsNormal, 2,2,2,2);
	TGLayoutHints *chints = new TGLayoutHints(kLHintsCenterY|kLHintsCenterX, 2,2,2,2);
	TGLayoutHints *xhints = new TGLayoutHints(kLHintsNormal|kLHintsExpandX, 2,2,2,2);
	TGLayoutHints *yhints = new TGLayoutHints(kLHintsNormal|kLHintsExpandY|kLHintsCenterY, 2,2,2,2);
	TGLayoutHints *thints = new TGLayoutHints(kLHintsTop|kLHintsCenterX|kLHintsExpandX, 2,2,0,0);
	TGHorizontalFrame *topframe = new TGHorizontalFrame(this, w, h);
	TGHorizontalFrame *botframe = new TGHorizontalFrame(this, w, h);
	AddFrame(topframe, lhints);
	AddFrame(botframe, chints);

	TGGroupFrame *canvasframe = new TGGroupFrame(topframe, "FCAL", kVerticalFrame);
	TGGroupFrame *controls = new TGGroupFrame(topframe, "", kVerticalFrame);
	topframe->AddFrame(canvasframe, lhints);
	topframe->AddFrame(controls, lhints);

	TGGroupFrame *viewcontrols = new TGGroupFrame(controls, "Controls", kVerticalFrame);
	controls->AddFrame(viewcontrols, lhints);
	TGGroupFrame *fcalColorCodes = new TGGroupFrame(controls, "Color code", kVerticalFrame);
	controls->AddFrame(fcalColorCodes, lhints);
	
		//------------- View canvas
		ecanvas = new TRootEmbeddedCanvas("endviewB Large Canvas", canvasframe, w, h, kSunkenFrame, GetWhitePixel());
		canvasframe->AddFrame(ecanvas, chints);

		//------------- Pan buttons
		TGGroupFrame *panframe = new TGGroupFrame(viewcontrols, "Pan", kHorizontalFrame);
			TGTextButton *panxneg	= new TGTextButton(panframe,	"-X");
			TGVerticalFrame *panupdn = new TGVerticalFrame(panframe);
				TGTextButton *panypos	= new TGTextButton(panupdn,	"+Y");
				TGTextButton *panyneg	= new TGTextButton(panupdn,	"-Y");
			TGTextButton *panxpos	= new TGTextButton(panframe,	"X+");
			
		viewcontrols->AddFrame(panframe,	yhints);
			panframe->AddFrame(panxneg,	chints);
			panframe->AddFrame(panupdn,	chints);
				panupdn->AddFrame(panypos,	chints);
				panupdn->AddFrame(panyneg,	chints);
			panframe->AddFrame(panxpos,	chints);

		panxneg->Connect("Clicked()","hdv_mainframe",hdvmf,"DoPanXneg()");
		panyneg->Connect("Clicked()","hdv_mainframe",hdvmf,"DoPanYneg()");
		panxpos->Connect("Clicked()","hdv_mainframe",hdvmf,"DoPanXpos()");
		panypos->Connect("Clicked()","hdv_mainframe",hdvmf,"DoPanYpos()");
		
		//------------- Zoom buttons
		TGGroupFrame *zoomframe = new TGGroupFrame(viewcontrols, "ZOOM", kHorizontalFrame);
		viewcontrols->AddFrame(zoomframe,	thints);
			TGTextButton *zoomout = new TGTextButton(zoomframe,	" - ");
			TGTextButton *zoomin	= new TGTextButton(zoomframe,	" + ");
			zoomframe->AddFrame(zoomout,	xhints);
			zoomframe->AddFrame(zoomin,	xhints);

		zoomout->Connect("Clicked()","hdv_mainframe",hdvmf,"DoZoomOut()");
		zoomin->Connect("Clicked()","hdv_mainframe",hdvmf,"DoZoomIn()");

	//------------- Reset button
	TGTextButton *reset	= new TGTextButton(viewcontrols,	"Reset");
	viewcontrols->AddFrame(reset, chints);
	reset->Connect("Clicked()","hdv_mainframe", hdvmf, "DoReset()");

	//------------- SaveAs message
	stringstream ss;
	ss<<"To save the canvas to\n";
	ss<<"a file, right click\n";
	ss<<"and select \"SaveAs\"\n";
	ss<<"from the menu. File type\n";
	ss<<"will be determined by\n";
	ss<<"the suffix of the file\n";
	ss<<"name.\n";
	TGLabel *saveas	= new TGLabel(viewcontrols, ss.str().c_str());
	viewcontrols->AddFrame(saveas, chints);

	TGLabel* FCCLables[9]; 
	unsigned int FCccodes[9] = {0x0000FF,0x7700FF,0xFF00FF,0xFF0077,0xFF0000,0xFF7700,0xFFFF00,0xFFFF77,0xFFFFFF};
	for (int i=0;i<9;i++) {
	  double e = pow(10,((8-(double)i)/2.0));
	  char str1[128];
	  if (e >= 1000) {
	    sprintf(str1,"%7.2f GeV",e/1000.);
	  } else {
	    sprintf(str1,"%7.1f MeV",e);
	  }
	  FCCLables[i] =  new TGLabel(fcalColorCodes, (const char*)str1);
	  FCCLables[i]->SetBackgroundColor(FCccodes[i]);
	  fcalColorCodes->AddFrame(FCCLables[i],lhints);
	}


	//========== Dismiss Button ===========
	TGTextButton *dismiss = new TGTextButton(botframe,	"dismiss");
	botframe->AddFrame(dismiss, chints);

	//&&&&&&&&&&&&&&&& Connections
	dismiss->Connect("Clicked()","hdv_endviewBframe", this, "DoDismiss()");
	this->Connect("CloseWindow()", "hdv_endviewBframe", this, "DoDismiss()");
        this->DontCallClose();

	// Finish up and map the window
	SetWindowName("Hall-D Event Viewer FCAL and TOF View");
	SetIconName("HDView");


	MapSubwindows();
	Resize(GetDefaultSize());
	panframe->Resize();
	saveas->Resize();
	viewcontrols->Resize();
}

//-------------------
// DoDismiss
//-------------------
void hdv_endviewBframe::DoDismiss(void)
{
	UnmapWindow();
}

//-------------------
// SetRange
//-------------------
void hdv_endviewBframe::SetRange(double xlo, double ylo, double xhi, double yhi)
{
	ecanvas->GetCanvas()->cd();
	ecanvas->GetCanvas()->Range(xlo, ylo, xhi, yhi);
}

//-------------------
// DrawObjects
//-------------------
void hdv_endviewBframe::DrawObjects(vector<TObject*> &graphics_endB)
{
	if(!IsMapped())return;

	ecanvas->GetCanvas()->cd(0);
	for(unsigned int i=0; i<graphics_endB.size(); i++)graphics_endB[i]->Draw("f");
	for(unsigned int i=0; i<graphics_endB.size(); i++)graphics_endB[i]->Draw();
	ecanvas->GetCanvas()->Update();
}

