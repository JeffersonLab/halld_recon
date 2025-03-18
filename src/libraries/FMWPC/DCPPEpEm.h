// $Id$
//
//    File: DCPPEpEm.h
// Created: Thu Mar 17 14:49:42 EDT 2022
// Creator: staylor (on Linux ifarm1801.jlab.org 3.10.0-1160.11.1.el7.x86_64 x86_64)
//

#ifndef _DCPPEpEm_
#define _DCPPEpEm_

#include <JANA/JObject.h>
#include <FCAL/DFCALShower.h>

class DCPPEpEm:public JObject{
 public:
  JOBJECT_PUBLIC(DCPPEpEm);
  
  double Ebeam; // Photon beam energy 
  double weight;  // event weight (+: prompt, -: accidental)
  double pippim_chisq,epem_chisq,kpkm_chisq; // fit qualities
  double pimu_ML_classifier;    // ~0=mu event , ~1=pi event -1=info not available
  double pipep_ML_classifier,pimem_ML_classifier;// ~1 = lepton
  const DFCALShower *ElectronShower,*PositronShower; // pointers to FCAL showers to which the tracks point
  DLorentzVector pim_v4,pip_v4; // four vectors for pion hypothesis pairs
  DLorentzVector em_v4,ep_v4; // four vectors for e+/e- hypothesis pairs
  DLorentzVector km_v4,kp_v4; // four vectors for K+/K- hypothesis pairs

  // This method is used primarily for pretty printing
  // the second argument to AddString is printf style format
  void Summarize(JObjectSummary& summary) const override {

    summary.add(Ebeam, "Ebeam", "%f");
    summary.add(weight, "weight", "%f");
    summary.add((ElectronShower!=NULL)?ElectronShower->getEnergy():0., "Electron E", "%f");
    summary.add((PositronShower!=NULL)?PositronShower->getEnergy():0., "Positron E", "%f");
    summary.add(kpkm_chisq, "kpkm_chisq", "%f");
    summary.add(pimu_ML_classifier, "pimu_ML_classifier", "%f"); 
    summary.add(pimem_ML_classifier, "pimem_ML_classifier", "%f");
    summary.add(pipep_ML_classifier, "pipep_ML_classifier", "%f");
    summary.add(kp_v4.P(), "K+ momentum", "%f");
    summary.add(km_v4.P(), "K- momentum", "%f");
    summary.add(pippim_chisq, "pippim_chisq", "%f");
    summary.add(pip_v4.P(), "pi+ momentum", "%f");
    summary.add(pim_v4.P(), "pi- momentum", "%f");
    summary.add(epem_chisq, "epem_chisq", "%f");
    summary.add(ep_v4.P(), "e+ momentum", "%f");
    summary.add(em_v4.P(), "e- momentum", "%f");
  }
  
};

#endif // _DCPPEpEm_

