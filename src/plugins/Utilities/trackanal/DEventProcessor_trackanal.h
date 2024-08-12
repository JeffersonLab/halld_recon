// $Id$
//
//    File: DEventProcessor_tofanal.h
// Created: Wed Jul 14 17:12:56 EDT 2010
// Creator: zihlmann (on Linux chaos.jlab.org 2.6.18-164.2.1.el5 i686)
//

#ifndef _DEventProcessor_trackanal_
#define _DEventProcessor_trackanal_

using namespace std;

#include <TTree.h>
#include <TFile.h>

#include <JANA/JEventProcessor.h>
#define MaxTrThrown 30
#define MaxTrCand 35
#define MaxTrFit 60

class DEventProcessor_trackanal:public JEventProcessor{
 public:
  DEventProcessor_trackanal();
  ~DEventProcessor_trackanal();

  TTree *TrackTree;
  TFile *ROOTFile;
  
  Int_t EventNum;
  Int_t NTrThrown; // total number of thrown tracks
  Int_t MaxT;  // Maximum number of thrown tracks
  Int_t MaxF;  // Maximum number of fits
  Int_t MaxC;  // Maximum number of track candidates considered

  Int_t ThrownPType[MaxTrThrown]; // particle type of thrown tracks
  Float_t ThrownPp[MaxTrThrown];  // particle momentum of thrown tracks
  Float_t ThrownQ[MaxTrThrown];   // electric charge of thrown particle
  Int_t NTrCand;
  Float_t TrCandP[MaxTrCand];   // momentum of track candidate
  Float_t TrCandQ[MaxTrCand];   // charge of track candidate
  Float_t TrCandN[MaxTrCand];   // number of hits of track candidate
  Float_t TrCandM[MaxTrCand];   // number of hits with match found in TruthPoint
  Int_t NTrCandHits;
  Int_t NTrFit;
  Int_t   trlistPtype[MaxTrFit];  // particle type of track candidate with best FOM
  Float_t trlistPp[MaxTrFit];   // particle momentum of track candidate with best FOM
  Float_t trlistFOM[MaxTrFit];  // figure of merrit
  Float_t trlistchisq[MaxTrFit];  // chisq
  Int_t   trlistcand[MaxTrFit];   // track candidate number
  Float_t nh[MaxTrFit*MaxTrFit] ; // number of hits for each track candidate
  Float_t ptypes[MaxTrFit*MaxTrFit];    // for each track candidate the chamber hits for each particle type
    
 private:
    void Init() override;
    void BeginRun(const std::shared_ptr<const JEvent>& event) override;
    void Process(const std::shared_ptr<const JEvent>& event) override;
    void EndRun() override;
    void Finish() override;


  
  pthread_mutex_t mutex;
  
};

#endif // _DEventProcessor_trackanal_

