// -----------------------------------------
// DrcEvent.h
//
// Author  : R.Dzhygadlo at gsi.de
// -----------------------------------------

#ifndef DrcEvent_h
#define DrcEvent_h 1

#include <vector>

#include "TObject.h"
#include "TVector3.h"

#include "DrcHit.h"

class DrcEvent: public TObject  {

public:

  DrcEvent(); 	//the default constructor
  ~DrcEvent() {}; 

  void AddHit(DrcHit hit);
  DrcHit GetHit(Int_t ind) { return fHitArray[ind]; }
  std::vector<DrcHit> GetHits() { return fHitArray; }
  
  // Accessors 
  Int_t GetId() const { return fId; }
  Int_t GetType() const { return fType; }
  Double_t GetTime() const { return fTime; }
  Double_t GetInvMass() const { return fInvMass; }
  Double_t GetMissMass() const { return fMissMass; }
  Double_t GetChiSq() const    { return fChiSq; }
  Double_t GetTofTrackDist() const    { return fTofTrackDist; }

  Int_t GetPdg()          const { return fPdg; }
  Int_t GetParent()       const { return fParent; }
  Int_t GetDcHits()       const { return fDcHits; }
  TVector3 GetMomentum()  const { return fMomentum; }
  TVector3 GetPosition()  const { return fPosition; }
  Int_t GetHitSize()      const { return fHitSize; }
  Double_t GetTest1()     const { return fTest1; }
  Double_t GetTest2()     const { return fTest2; }
  
  // Mutators
  void SetId(Int_t val)         { fId=val; }
  void SetType(Int_t val)       { fType=val; }
  void SetTime(Double_t val)    { fTime=val; }
  void SetInvMass(Double_t val) { fInvMass=val; }
  void SetMissMass(Double_t val){ fMissMass=val; }
  void SetChiSq(Double_t val)   { fChiSq=val; }
  void SetTofTrackDist(Double_t val) { fTofTrackDist=val; }

  void SetPdg(Int_t val)        { fPdg = val; }
  void SetParent(Int_t val)     { fParent = val; }
  void SetDcHits(Int_t val)     { fDcHits = val; }
  void SetMomentum(TVector3 val){ fMomentum = val; }
  void SetPosition(TVector3 val){ fPosition = val; }
  void SetTest1(Double_t val)   { fTest1 = val; }
  void SetTest2(Double_t val)   { fTest2 = val; }

private: 
  Int_t fId;
  Int_t fType;
  Int_t fPdg;
  Int_t fParent;
  Int_t fDcHits;
  Double_t fTime;
  Double_t fInvMass;
  Double_t fMissMass;
  Double_t fChiSq;
  Double_t fTofTrackDist;
  
  Int_t fHitSize;
  std::vector<DrcHit> fHitArray;

  TVector3 fMomentum;
  TVector3 fPosition;
  Double_t fTest1;
  Double_t fTest2;
  
  ClassDef(DrcEvent, 3);
};

#endif

// #if defined(__ROOTCLING__)
// #pragma link C++ class DrcEvent+;
// #endif
