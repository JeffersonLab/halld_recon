
#ifndef _HDV_DEBUGERFRAME_H_
#define _HDV_DEBUGERFRAME_H_

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
#include <TG3DLine.h>


//class hdv_mainframe;
#include "hdv_mainframe.h"

class DKinematicData;
class DTrackWireBased;
class DTrackTimeBased;

#if !(defined(__CINT__) || defined(__CLING__))
#include <PID/DKinematicData.h>
#include <TRACKING/DTrackWireBased.h>
#include <TRACKING/DTrackTimeBased.h>
#endif

#define MaxWireTracks 21
#define MaxTimeTracks 21

class hdv_debugerframe:public TGMainFrame {
  
 public:
  hdv_debugerframe(hdv_mainframe *hdvmf, const TGWindow *p, UInt_t w, UInt_t h);
  virtual ~hdv_debugerframe(){};
  
  void DoDone(void);
  void UpdateTrackLabels();
  TGHorizontalFrame *topframe;
  TGHorizontalFrame *mid1frame;
  TGHorizontalFrame *mid2frame;
  TGHorizontalFrame *botframe;
  
  TGGroupFrame *hitdrawopts ;
  TGGroupFrame *hitdrawoptsWB ;
  TGGroupFrame *hitdrawoptsTB ;
  TGGroupFrame *trackinfo;
  TGGroupFrame *trackinfoWB ;
  TGGroupFrame *trackinfoTB ;

  TGTextButton *done ;

  Int_t GetNTrCand(void) {return NTrCand;}
  Int_t GetNTrWB(void) {return NTrWireBased;}
  Int_t GetNTrTB(void) {return NTrTimeBased;}
  void SetNTrCand(Int_t d) { NTrCand = d;}
  void SetNTrTimeBased(Int_t d) { NTrTimeBased = d;}
  void SetNTrWireBased(Int_t d) { NTrWireBased = d;}
  void SetTrackCandidates(std::vector<const DKinematicData*> d) {TrackCandidates=d;}
  void SetTrackWireBased(std::vector<const DTrackWireBased*> d) {subTrackWireBased=d;}
  void SetTrackTimeBased(std::vector<const DTrackTimeBased*> d) {subTrackTimeBased=d;}
  void SetUpMid1Frame();
  void SetUpMid2Frame();

 private:

  Int_t InitMid1Frame;
  Int_t InitMid2Frame;
  Int_t NTrCand;
  Int_t NTrTimeBased;
  Int_t NTrWireBased;
  std::vector<const DKinematicData*> TrackCandidates;
  std::vector<const DTrackWireBased*> subTrackWireBased;
  std::vector<const DTrackTimeBased*> subTrackTimeBased;

  map<string, TGVerticalFrame *> tf;
  map<string, std::vector<TGLabel*> > candlabs;

  map<string, TGVerticalFrame *> tfWB;
  map<string, std::vector<TGLabel*> > wblabs;
	       
  map<string, TGVerticalFrame *> tfTB;
  map<string, std::vector<TGLabel*> > tblabs;
	       
  hdv_mainframe *hdvmf;
  map<string, TGCheckButton*> checkbuttons;
  
  ClassDef(hdv_debugerframe,1)
    };

// The following line is supposed to avoid the warning messages about:
// "dereferencing type-punned pointer will break strict-aliasing rules"
#if (defined(__CINT__) || defined(__CLING__))

#pragma link C++ class hdv_debugerframe+;
#endif



#endif //_HDV_DEBUGERFRAME_H_
