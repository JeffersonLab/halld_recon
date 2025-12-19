#include "DrcEvent.h"

ClassImp(DrcEvent)

// // -----   Default constructor   -------------------------------------------
DrcEvent::DrcEvent(): fId(-1),fType(0),fPdg(0),fParent(0),fDcHits(0),
  fMomentum(TVector3(0, 0, 0)),fPosition(TVector3(0, 0, 0)),
  fMomentum_Truth(TVector3(0, 0, 0)),fPosition_Truth(TVector3(0, 0, 0)),
  fTime(-100),fInvMass(-100),fMissMass(-100),fChiSq(-100),fTofTrackDist(-100),fTofTrackDeltaT(-100),fHitSize(0),fTest1(0),fTest2(0),fNPhotons(0),fLikelihoodElectron(-999),fLikelihoodPion(-9999),fLikelihoodKaon(-99999),fLikelihoodProton(-999999){
}

void DrcEvent::AddHit(DrcHit hit){
  fHitArray.push_back(hit);
  fHitSize++;
}
