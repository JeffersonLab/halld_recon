
#ifndef _HDV_FULLLISTFRAME_H_
#define _HDV_FULLLISTFRAME_H_

// This class is made into a ROOT dictionary ala rootcint.
// Therefore, do NOT include anything Hall-D specific here.
// It is OK to do that in the .cc file, just not here in the 
// header.

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

class hdv_mainframe;
class DMCThrown;
class DKinematicData;
#if !(defined(__CINT__) || defined(__CLING__))
#include "hdv_mainframe.h"
#endif

class hdv_fulllistframe:public TGMainFrame {

	public:
		hdv_fulllistframe(hdv_mainframe *hdvmf, const TGWindow *p, UInt_t w, UInt_t h);
		virtual ~hdv_fulllistframe(){};
		
		void DoClose(void);
		void UpdateTrackLabels(std::vector<const DMCThrown*> &throwns, std::vector<const DKinematicData*> &trks);
		
	private:
	
		hdv_mainframe *hdvmf;

		TGComboBox *reconfactory;
		std::map<std::string, std::vector<TGLabel*> > thrownlabs;
		std::map<std::string, std::vector<TGLabel*> > reconlabs;

		TGGroupFrame *throwninfo;
		TGGroupFrame *reconinfo;
		
		std::map<std::string, TGVerticalFrame *> tf;
		std::map<std::string, TGVerticalFrame *> rf;
		
	ClassDef(hdv_fulllistframe,1)
};

// The following line is supposed to avoid the warning messages about:
// "dereferencing type-punned pointer will break strict-aliasing rules"
#if (defined(__CINT__) || defined(__CLING__))

#pragma link C++ class hdv_fulllistframe+;
#endif



#endif //_HDV_FULLLISTFRAME_H_
