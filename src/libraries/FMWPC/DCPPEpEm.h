// $Id$
//
//    File: DCPPEpEm.h
// Created: Thu Mar 17 14:49:42 EDT 2022
// Creator: staylor (on Linux ifarm1801.jlab.org 3.10.0-1160.11.1.el7.x86_64 x86_64)
//

#ifndef _DCPPEpEm_
#define _DCPPEpEm_

#include <JANA/JObject.h>
#include <JANA/JFactory.h>
#include <FCAL/DFCALShower.h>

class DCPPEpEm:public jana::JObject{
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
  DVector3 pippim_pos; // common vertex position

  // This method is used primarily for pretty printing
  // the second argument to AddString is printf style format
  void toStrings(vector<pair<string,string> > &items)const{
    AddString(items, "Ebeam", "%f", Ebeam);
    AddString(items, "weight", "%f", weight);
    AddString(items, "Electron E", "%f", 
	      (ElectronShower!=NULL)?ElectronShower->getEnergy():0.);
    AddString(items, "Positron E", "%f", 
	      (PositronShower!=NULL)?PositronShower->getEnergy():0.);
    AddString(items, "pimu_ML_classifier", "%f", pimu_ML_classifier); 
    AddString(items, "pimem_ML_classifier", "%f", pimem_ML_classifier);
    AddString(items, "pipep_ML_classifier", "%f", pipep_ML_classifier);
    AddString(items, "kpkm_chisq", "%f", kpkm_chisq);
    AddString(items, "K+ momentum", "%f", kp_v4.P());
    AddString(items, "K- momentum", "%f", km_v4.P());
    AddString(items, "pippim_chisq", "%f", pippim_chisq);
    AddString(items, "pi+ momentum", "%f", pip_v4.P());
    AddString(items, "pi- momentum", "%f", pim_v4.P());
    AddString(items, "epem_chisq", "%f", epem_chisq);
    AddString(items, "e+ momentum", "%f", ep_v4.P());
    AddString(items, "e- momentum", "%f", em_v4.P());
    AddString(items, "pi+pi- vertex","%f", pippim_pos.z());
  }
  
};

#endif // _DCPPEpEm_

