// $Id$
//
//    File: DParticleID_PID1.cc
// Created: Mon Feb 28 15:25:35 EST 2011
// Creator: staylor (on Linux ifarml1 2.6.18-128.el5 x86_64)
//

#include "DParticleID_PID1.h"
#include "DANA/DEvent.h"

//---------------------------------
// DParticleID_PID1    (Constructor)
//---------------------------------
DParticleID_PID1::DParticleID_PID1(const std::shared_ptr<const JEvent>& jevent):DParticleID(jevent)
{
  JCalibration * jcalib = DEvent::GetJCalibration(jevent);
  vector<map<string,double> >vals;
  if (jcalib->Get("CDC/ElectrondEdxMean",vals)==false){
    map<string,double> &row = vals[0];
    ddEdxMeanParams_CDC_Electron.push_back(row["m1"]);
    ddEdxMeanParams_CDC_Electron.push_back(row["m2"]);
    ddEdxMeanParams_CDC_Electron.push_back(row["m3"]);
    ddEdxMeanParams_CDC_Electron.push_back(row["m4"]);
  }
  if (jcalib->Get("CDC/dEdxMean",vals)==false){
    for(unsigned int i=0; i<vals.size(); i++){
      map<string,double> &row = vals[i];
      switch(int(row["PID"])){
      case 8:
	ddEdxMeanParams_CDC_PiPlus.push_back(row["m1"]);
	ddEdxMeanParams_CDC_PiPlus.push_back(row["m2"]);
	ddEdxMeanParams_CDC_PiPlus.push_back(row["m3"]);
	ddEdxMeanParams_CDC_PiPlus.push_back(row["m4"]);
	break;
      case 11:
	ddEdxMeanParams_CDC_KPlus.push_back(row["m1"]);
	ddEdxMeanParams_CDC_KPlus.push_back(row["m2"]);
	ddEdxMeanParams_CDC_KPlus.push_back(row["m3"]);
	ddEdxMeanParams_CDC_KPlus.push_back(row["m4"]);
	break;
      case 14:
	ddEdxMeanParams_CDC_Proton.push_back(row["m1"]);
	ddEdxMeanParams_CDC_Proton.push_back(row["m2"]);
	ddEdxMeanParams_CDC_Proton.push_back(row["m3"]);
	ddEdxMeanParams_CDC_Proton.push_back(row["m4"]);
	break;
      default:
	break;
      }
    }
  }
  if (jcalib->Get("CDC/ElectrondEdxSigma",vals)==false){
    map<string,double> &row = vals[0];
    ddEdxSigmaParams_CDC_Electron.push_back(row["s1"]);
    ddEdxSigmaParams_CDC_Electron.push_back(row["s2"]);
    ddEdxSigmaParams_CDC_Electron.push_back(row["s3"]);
    ddEdxSigmaParams_CDC_Electron.push_back(row["s4"]);
  } 
  if (jcalib->Get("CDC/dEdxNdep",vals)==false){
    map<string,double> &row = vals[0];
    ddEdxSigmaParams_CDC_N_dependence.push_back(row["p1"]);
    ddEdxSigmaParams_CDC_N_dependence.push_back(row["p2"]); 
    ddEdxSigmaParams_CDC_N_dependence.push_back(row["p3"]);
    /*
    cout << "CDC dEdx N dependence:";
    for (unsigned int i=0;i<ddEdxSigmaParams_CDC_N_dependence.size();i++){
      cout << " " << ddEdxSigmaParams_CDC_N_dependence[i];
    }
    cout<< endl;
    */
  }
  if (jcalib->Get("CDC/dEdxSigma",vals)==false){
    for(unsigned int i=0; i<vals.size(); i++){
      map<string,double> &row = vals[i];
      switch(int(row["PID"])){
      case 8:
	ddEdxSigmaParams_CDC_PiPlus.push_back(row["s1"]);
	ddEdxSigmaParams_CDC_PiPlus.push_back(row["s2"]);
	ddEdxSigmaParams_CDC_PiPlus.push_back(row["s3"]);
	ddEdxSigmaParams_CDC_PiPlus.push_back(row["s4"]);
	break;
      case 11:
	ddEdxSigmaParams_CDC_KPlus.push_back(row["s1"]);
	ddEdxSigmaParams_CDC_KPlus.push_back(row["s2"]);
	ddEdxSigmaParams_CDC_KPlus.push_back(row["s3"]);
	ddEdxSigmaParams_CDC_KPlus.push_back(row["s4"]);
	break;
      case 14:
	ddEdxSigmaParams_CDC_Proton.push_back(row["s1"]);
	ddEdxSigmaParams_CDC_Proton.push_back(row["s2"]);
	ddEdxSigmaParams_CDC_Proton.push_back(row["s3"]);
	ddEdxSigmaParams_CDC_Proton.push_back(row["s4"]);
	break;
      default:
	break;
      }
    }
  }
  
   if (jcalib->Get("FDC/ElectrondEdxMean",vals)==false){
    map<string,double> &row = vals[0];
    ddEdxMeanParams_FDC_Electron.push_back(row["m1"]);
    ddEdxMeanParams_FDC_Electron.push_back(row["m2"]);
    ddEdxMeanParams_FDC_Electron.push_back(row["m3"]);
    ddEdxMeanParams_FDC_Electron.push_back(row["m4"]);
  }
  if (jcalib->Get("FDC/dEdxMean",vals)==false){
    for(unsigned int i=0; i<vals.size(); i++){
      map<string,double> &row = vals[i];
      switch(int(row["PID"])){
      case 8:
	ddEdxMeanParams_FDC_PiPlus.push_back(row["m1"]);
	ddEdxMeanParams_FDC_PiPlus.push_back(row["m2"]);
	ddEdxMeanParams_FDC_PiPlus.push_back(row["m3"]);
	ddEdxMeanParams_FDC_PiPlus.push_back(row["m4"]);
	break;
      case 11:
	ddEdxMeanParams_FDC_KPlus.push_back(row["m1"]);
	ddEdxMeanParams_FDC_KPlus.push_back(row["m2"]);
	ddEdxMeanParams_FDC_KPlus.push_back(row["m3"]);
	ddEdxMeanParams_FDC_KPlus.push_back(row["m4"]);
	break;
      case 14:
	ddEdxMeanParams_FDC_Proton.push_back(row["m1"]);
	ddEdxMeanParams_FDC_Proton.push_back(row["m2"]);
	ddEdxMeanParams_FDC_Proton.push_back(row["m3"]);
	ddEdxMeanParams_FDC_Proton.push_back(row["m4"]);
	break;
      default:
	break;
      }
    }
  }
  if (jcalib->Get("FDC/ElectrondEdxSigma",vals)==false){
    map<string,double> &row = vals[0];
    ddEdxSigmaParams_FDC_Electron.push_back(row["s1"]);
    ddEdxSigmaParams_FDC_Electron.push_back(row["s2"]);
    ddEdxSigmaParams_FDC_Electron.push_back(row["s3"]);
    ddEdxSigmaParams_FDC_Electron.push_back(row["s4"]);
  }
  if (jcalib->Get("FDC/dEdxNdep",vals)==false){
    map<string,double> &row = vals[0];
    ddEdxSigmaParams_FDC_N_dependence.push_back(row["p1"]);
    ddEdxSigmaParams_FDC_N_dependence.push_back(row["p2"]); 
    ddEdxSigmaParams_FDC_N_dependence.push_back(row["p3"]);
    /*
    cout << "FDC dEdx N dependence:";
    for (unsigned int i=0;i<ddEdxSigmaParams_FDC_N_dependence.size();i++){
      cout << " " << ddEdxSigmaParams_FDC_N_dependence[i];
    }
    cout<< endl;
    */
  }
  if (jcalib->Get("FDC/dEdxSigma",vals)==false){
    for(unsigned int i=0; i<vals.size(); i++){
      map<string,double> &row = vals[i];
      switch(int(row["PID"])){
      case 8:
	ddEdxSigmaParams_FDC_PiPlus.push_back(row["s1"]);
	ddEdxSigmaParams_FDC_PiPlus.push_back(row["s2"]);
	ddEdxSigmaParams_FDC_PiPlus.push_back(row["s3"]);
	ddEdxSigmaParams_FDC_PiPlus.push_back(row["s4"]);
	break;
      case 11:
	ddEdxSigmaParams_FDC_KPlus.push_back(row["s1"]);
	ddEdxSigmaParams_FDC_KPlus.push_back(row["s2"]);
	ddEdxSigmaParams_FDC_KPlus.push_back(row["s3"]);
	ddEdxSigmaParams_FDC_KPlus.push_back(row["s4"]);
	break;
      case 14:
	ddEdxSigmaParams_FDC_Proton.push_back(row["s1"]);
	ddEdxSigmaParams_FDC_Proton.push_back(row["s2"]);
	ddEdxSigmaParams_FDC_Proton.push_back(row["s3"]);
	ddEdxSigmaParams_FDC_Proton.push_back(row["s4"]);
	break;
      default:
	break;
      }
    }
  }
  if (jcalib->Get("START_COUNTER/dEdxProtonMean",vals)==false){ 
    map<string,double> &row = vals[0];
    ddEdxMeanParams_SC_Proton.push_back(row["m1"]);
    ddEdxMeanParams_SC_Proton.push_back(row["m2"]);
    ddEdxMeanParams_SC_Proton.push_back(row["m3"]);
    ddEdxMeanParams_SC_Proton.push_back(row["m4"]);
  } 
  if (jcalib->Get("START_COUNTER/dEdxProtonSigma",vals)==false){  
    map<string,double> &row = vals[0];
    ddEdxSigmaParams_SC_Proton.push_back(row["s1"]);
    ddEdxSigmaParams_SC_Proton.push_back(row["s2"]);
    ddEdxSigmaParams_SC_Proton.push_back(row["s3"]);
    ddEdxSigmaParams_SC_Proton.push_back(row["s4"]);
  }
  if (jcalib->Get("TOF/dEdxProtonMean",vals)==false){ 
    map<string,double> &row = vals[0];
    ddEdxMeanParams_TOF_Proton.push_back(row["m1"]);
    ddEdxMeanParams_TOF_Proton.push_back(row["m2"]);
    ddEdxMeanParams_TOF_Proton.push_back(row["m3"]);
    ddEdxMeanParams_TOF_Proton.push_back(row["m4"]);
  } 
  if (jcalib->Get("TOF/dEdxProtonSigma",vals)==false){  
    map<string,double> &row = vals[0];
    ddEdxSigmaParams_TOF_Proton.push_back(row["s1"]);
    ddEdxSigmaParams_TOF_Proton.push_back(row["s2"]);
    ddEdxSigmaParams_TOF_Proton.push_back(row["s3"]);
    ddEdxSigmaParams_TOF_Proton.push_back(row["s4"]);
  }


  jevent->GetSingle(dTOFGeometry);
  string locTOFTimeSigmasTable = dTOFGeometry->Get_CCDB_DirectoryName() + "/TimeSigmas";
  if (jcalib->Get(locTOFTimeSigmasTable.c_str(),vals)==false){ 
    for(unsigned int i=0; i<vals.size(); i++){
      map<string,double> &row = vals[i];
      switch(int(row["PID"])){  
      case 2:
	dTimeSigmaParams_TOF_Positron.push_back(row["s1"]);
	dTimeSigmaParams_TOF_Positron.push_back(row["s2"]);
	dTimeSigmaParams_TOF_Positron.push_back(row["s3"]);
	dTimeSigmaParams_TOF_Positron.push_back(row["s4"]);
	break;
      case 8:
	dTimeSigmaParams_TOF_PiPlus.push_back(row["s1"]);
	dTimeSigmaParams_TOF_PiPlus.push_back(row["s2"]);
	dTimeSigmaParams_TOF_PiPlus.push_back(row["s3"]);
	dTimeSigmaParams_TOF_PiPlus.push_back(row["s4"]);
	break;
      case 11:
	dTimeSigmaParams_TOF_KPlus.push_back(row["s1"]);
	dTimeSigmaParams_TOF_KPlus.push_back(row["s2"]);
	dTimeSigmaParams_TOF_KPlus.push_back(row["s3"]);
	dTimeSigmaParams_TOF_KPlus.push_back(row["s4"]);
	break;
      case 14:
	dTimeSigmaParams_TOF_Proton.push_back(row["s1"]);
	dTimeSigmaParams_TOF_Proton.push_back(row["s2"]);
	dTimeSigmaParams_TOF_Proton.push_back(row["s3"]);
	dTimeSigmaParams_TOF_Proton.push_back(row["s4"]);
	break;
      default:
	break;
      }
    }
  }

  if (jcalib->Get("BCAL/TimeSigmas",vals)==false){ 
    for(unsigned int i=0; i<vals.size(); i++){
      map<string,double> &row = vals[i];
      switch(int(row["PID"])){  
      case 2:
	dTimeSigmaParams_BCAL_Positron.push_back(row["s1"]);
	dTimeSigmaParams_BCAL_Positron.push_back(row["s2"]);
	dTimeSigmaParams_BCAL_Positron.push_back(row["s3"]);
	dTimeSigmaParams_BCAL_Positron.push_back(row["s4"]);
	break;
      case 8:
	dTimeSigmaParams_BCAL_PiPlus.push_back(row["s1"]);
	dTimeSigmaParams_BCAL_PiPlus.push_back(row["s2"]);
	dTimeSigmaParams_BCAL_PiPlus.push_back(row["s3"]);
	dTimeSigmaParams_BCAL_PiPlus.push_back(row["s4"]);
	break;
      case 11:
	dTimeSigmaParams_BCAL_KPlus.push_back(row["s1"]);
	dTimeSigmaParams_BCAL_KPlus.push_back(row["s2"]);
	dTimeSigmaParams_BCAL_KPlus.push_back(row["s3"]);
	dTimeSigmaParams_BCAL_KPlus.push_back(row["s4"]);
	break;
      case 14:
	dTimeSigmaParams_BCAL_Proton.push_back(row["s1"]);
	dTimeSigmaParams_BCAL_Proton.push_back(row["s2"]);
	dTimeSigmaParams_BCAL_Proton.push_back(row["s3"]);
	dTimeSigmaParams_BCAL_Proton.push_back(row["s4"]);
	break;
      default:
	break;
      }
    }
  }
  
   if (jcalib->Get("BCAL/TimeMeans",vals)==false){ 
    for(unsigned int i=0; i<vals.size(); i++){
      map<string,double> &row = vals[i];
      switch(int(row["PID"])){  
      case 2:
	dTimeMeanParams_BCAL_Positron.push_back(row["m1"]);
	dTimeMeanParams_BCAL_Positron.push_back(row["m2"]);
	dTimeMeanParams_BCAL_Positron.push_back(row["m3"]);
	dTimeMeanParams_BCAL_Positron.push_back(row["m4"]);
	break;
      case 8:
	dTimeMeanParams_BCAL_PiPlus.push_back(row["m1"]);
	dTimeMeanParams_BCAL_PiPlus.push_back(row["m2"]);
	dTimeMeanParams_BCAL_PiPlus.push_back(row["m3"]);
	dTimeMeanParams_BCAL_PiPlus.push_back(row["m4"]);
	break;
      case 11:
	dTimeMeanParams_BCAL_KPlus.push_back(row["m1"]);
	dTimeMeanParams_BCAL_KPlus.push_back(row["m2"]);
	dTimeMeanParams_BCAL_KPlus.push_back(row["m3"]);
	dTimeMeanParams_BCAL_KPlus.push_back(row["m4"]);
	break;
      case 14:
	dTimeMeanParams_BCAL_Proton.push_back(row["m1"]);
	dTimeMeanParams_BCAL_Proton.push_back(row["m2"]);
	dTimeMeanParams_BCAL_Proton.push_back(row["m3"]);
	dTimeMeanParams_BCAL_Proton.push_back(row["m4"]);
	break;
      default:
	break;
      }
    }
  }

  
  if (jcalib->Get("BCAL/EOverPSigma",vals)==false){  
    map<string,double> &row = vals[0];
    dEOverPSigmaParams_BCAL.push_back(row["s0"]);
    dEOverPSigmaParams_BCAL.push_back(row["s1"]);
    dEOverPSigmaParams_BCAL.push_back(row["s2"]);
  }
  if (jcalib->Get("BCAL/EOverPMean",vals)==false){
    map<string,double> &row = vals[0];
    dEOverPMeanParams_BCAL.push_back(row["m0"]);
    dEOverPMeanParams_BCAL.push_back(row["m1"]);
    dEOverPMeanParams_BCAL.push_back(row["m2"]);
  }


  if (jcalib->Get("FCAL/EOverPSigma",vals)==false){  
    map<string,double> &row = vals[0];
    dEOverPSigmaParams_FCAL.push_back(row["s0"]);
    dEOverPSigmaParams_FCAL.push_back(row["s1"]);
    dEOverPSigmaParams_FCAL.push_back(row["s2"]);
  }
  if (jcalib->Get("FCAL/EOverPMean",vals)==false){
    map<string,double> &row = vals[0];
    dEOverPMeanParams_FCAL.push_back(row["m0"]);
    dEOverPMeanParams_FCAL.push_back(row["m1"]);
    dEOverPMeanParams_FCAL.push_back(row["m2"]);
  }
  
  if (jcalib->Get("FCAL/TimeSigmas",vals)==false){ 
    for(unsigned int i=0; i<vals.size(); i++){
      map<string,double> &row = vals[i];
      switch(int(row["PID"])){  
      case 2:
	dTimeSigmaParams_FCAL_Positron.push_back(row["s1"]);
	dTimeSigmaParams_FCAL_Positron.push_back(row["s2"]);
	dTimeSigmaParams_FCAL_Positron.push_back(row["s3"]);
	dTimeSigmaParams_FCAL_Positron.push_back(row["s4"]);
	break;
      case 8:
	dTimeSigmaParams_FCAL_PiPlus.push_back(row["s1"]);
	dTimeSigmaParams_FCAL_PiPlus.push_back(row["s2"]);
	dTimeSigmaParams_FCAL_PiPlus.push_back(row["s3"]);
	dTimeSigmaParams_FCAL_PiPlus.push_back(row["s4"]);
	break;
      case 11:
	dTimeSigmaParams_FCAL_KPlus.push_back(row["s1"]);
	dTimeSigmaParams_FCAL_KPlus.push_back(row["s2"]);
	dTimeSigmaParams_FCAL_KPlus.push_back(row["s3"]);
	dTimeSigmaParams_FCAL_KPlus.push_back(row["s4"]);
	break;
      case 14:
	dTimeSigmaParams_FCAL_Proton.push_back(row["s1"]);
	dTimeSigmaParams_FCAL_Proton.push_back(row["s2"]);
	dTimeSigmaParams_FCAL_Proton.push_back(row["s3"]);
	dTimeSigmaParams_FCAL_Proton.push_back(row["s4"]);
	break;
      default:
	break;
      }
    }
  }
  
  if (jcalib->Get("FCAL/TimeMeans",vals)==false){ 
    for(unsigned int i=0; i<vals.size(); i++){
      map<string,double> &row = vals[i];
      switch(int(row["PID"])){  
      case 2:
	dTimeMeanParams_FCAL_Positron.push_back(row["m1"]);
	dTimeMeanParams_FCAL_Positron.push_back(row["m2"]);
	dTimeMeanParams_FCAL_Positron.push_back(row["m3"]);
	dTimeMeanParams_FCAL_Positron.push_back(row["m4"]);
	break;
      case 8:
	dTimeMeanParams_FCAL_PiPlus.push_back(row["m1"]);
	dTimeMeanParams_FCAL_PiPlus.push_back(row["m2"]);
	dTimeMeanParams_FCAL_PiPlus.push_back(row["m3"]);
	dTimeMeanParams_FCAL_PiPlus.push_back(row["m4"]);
	break;
      case 11:
	dTimeMeanParams_FCAL_KPlus.push_back(row["m1"]);
	dTimeMeanParams_FCAL_KPlus.push_back(row["m2"]);
	dTimeMeanParams_FCAL_KPlus.push_back(row["m3"]);
	dTimeMeanParams_FCAL_KPlus.push_back(row["m4"]);
	break;
      case 14:
	dTimeMeanParams_FCAL_Proton.push_back(row["m1"]);
	dTimeMeanParams_FCAL_Proton.push_back(row["m2"]);
	dTimeMeanParams_FCAL_Proton.push_back(row["m3"]);
	dTimeMeanParams_FCAL_Proton.push_back(row["m4"]);
	break;
      default:
	break;
      }
      
    }
 }
}

//---------------------------------
// ~DParticleID_PID1    (Destructor)
//---------------------------------
DParticleID_PID1::~DParticleID_PID1()
{

}

double DParticleID_PID1::GetEOverPMean(DetectorSystem_t detector,
				       double p) const{
  double mean=0;
  switch(detector){
  case SYS_FCAL:
    mean=dEOverPMeanParams_FCAL[0]/p+dEOverPMeanParams_FCAL[1]
      +dEOverPMeanParams_FCAL[2]*p;
    break;
  case SYS_BCAL: 
    mean=dEOverPMeanParams_BCAL[0]/p+dEOverPMeanParams_BCAL[1]
      +dEOverPMeanParams_BCAL[2]*p;
    break;
  default:
    break;
  }
  return mean;
}

double DParticleID_PID1::GetEOverPSigma(DetectorSystem_t detector,
				       double p) const{
  double mean=0;
  switch(detector){
  case SYS_FCAL:
    mean=dEOverPSigmaParams_FCAL[0]/p+dEOverPSigmaParams_FCAL[1]
      +dEOverPSigmaParams_FCAL[2]*p;
    break;
  case SYS_BCAL: 
    mean=dEOverPSigmaParams_BCAL[0]/p+dEOverPSigmaParams_BCAL[1]
      +dEOverPSigmaParams_BCAL[2]*p;
    break;
  default:
    break;
  }
  return mean;
}

double DParticleID_PID1::GetProtondEdxMean_TOF(double locBeta) const{
  double locBetaGammaValue = locBeta/sqrt(1.0 - locBeta*locBeta);
  return 0.001*Function_dEdx(locBetaGammaValue, ddEdxMeanParams_TOF_Proton);  
}
double DParticleID_PID1::GetProtondEdxSigma_TOF(double locBeta) const{
  double locBetaGammaValue = locBeta/sqrt(1.0 - locBeta*locBeta);
  return 0.001*Function_dEdxSigma(locBetaGammaValue, ddEdxSigmaParams_TOF_Proton);  
}

double DParticleID_PID1::GetProtondEdxMean_SC(double locBeta) const{
  double locBetaGammaValue = locBeta/sqrt(1.0 - locBeta*locBeta);
  return 0.001*Function_dEdx(locBetaGammaValue, ddEdxMeanParams_SC_Proton);  
}
double DParticleID_PID1::GetProtondEdxSigma_SC(double locBeta) const{
  double locBetaGammaValue = locBeta/sqrt(1.0 - locBeta*locBeta);
  return 0.001*Function_dEdxSigma(locBetaGammaValue, ddEdxSigmaParams_SC_Proton);  
}

jerror_t DParticleID_PID1::GetdEdxMean_CDC(double locBeta, unsigned int locNumHitsUsedFordEdx, double& locMeandEdx, Particle_t locPIDHypothesis) const
{
  double locBetaGammaValue = locBeta/sqrt(1.0 - locBeta*locBeta); 
  if((locPIDHypothesis == Electron) || (locPIDHypothesis == Positron)){
    locMeandEdx = Function_dEdx(locBetaGammaValue, ddEdxMeanParams_CDC_Electron)/1000000.0;
    return NOERROR;
  }
  if((locPIDHypothesis == Proton) || (locPIDHypothesis == AntiProton)){
    locMeandEdx = Function_dEdx(locBetaGammaValue, ddEdxMeanParams_CDC_Proton)/1000000.0;
    return NOERROR;
  }
  if((locPIDHypothesis == KPlus) || (locPIDHypothesis == KMinus)){
    locMeandEdx = Function_dEdx(locBetaGammaValue, ddEdxMeanParams_CDC_KPlus)/1000000.0;
    return NOERROR;
  }
  if((locPIDHypothesis == PiPlus) || (locPIDHypothesis == PiMinus)){
    locMeandEdx = Function_dEdx(locBetaGammaValue, ddEdxMeanParams_CDC_PiPlus)/1000000.0;
    return NOERROR;
  }
  
  return RESOURCE_UNAVAILABLE;
}

jerror_t DParticleID_PID1::GetdEdxSigma_CDC(double locBeta, unsigned int locNumHitsUsedFordEdx, double& locSigmadEdx, Particle_t locPIDHypothesis) const
{
  double Nscale=ddEdxSigmaParams_CDC_N_dependence[0]
    *pow(double(locNumHitsUsedFordEdx),ddEdxSigmaParams_CDC_N_dependence[1])
    +ddEdxSigmaParams_CDC_N_dependence[2];
  double locBetaGammaValue = locBeta/sqrt(1.0 - locBeta*locBeta);
  if((locPIDHypothesis == Electron) || (locPIDHypothesis == Positron)){
    locSigmadEdx = Nscale*Function_dEdx(locBetaGammaValue, ddEdxSigmaParams_CDC_Electron)/1000000.0;
    return NOERROR;
  }
  if((locPIDHypothesis == Proton) || (locPIDHypothesis == AntiProton)){
    locSigmadEdx = Nscale*Function_dEdxSigma(locBetaGammaValue, ddEdxSigmaParams_CDC_Proton)/1000000.0;
    return NOERROR;
  }
  if((locPIDHypothesis == KPlus) || (locPIDHypothesis == KMinus)){
    locSigmadEdx = Nscale*Function_dEdxSigma(locBetaGammaValue, ddEdxSigmaParams_CDC_KPlus)/1000000.0;
    return NOERROR;
  }
  if((locPIDHypothesis == PiPlus) || (locPIDHypothesis == PiMinus)){
    locSigmadEdx = Nscale*Function_dEdxSigma(locBetaGammaValue, ddEdxSigmaParams_CDC_PiPlus)/1000000.0;
    return NOERROR;
  }
  
  return RESOURCE_UNAVAILABLE;
}

jerror_t DParticleID_PID1::GetdEdxMean_FDC(double locBeta, unsigned int locNumHitsUsedFordEdx, double& locMeandEdx, Particle_t locPIDHypothesis) const
{
  double locBetaGammaValue = locBeta/sqrt(1.0 - locBeta*locBeta);
  if((locPIDHypothesis == Electron) || (locPIDHypothesis == Positron)){
    locMeandEdx = Function_dEdx(locBetaGammaValue, ddEdxMeanParams_FDC_Electron)/1000000.0;
    return NOERROR;
  }
  if((locPIDHypothesis == Proton) || (locPIDHypothesis == AntiProton)){
    locMeandEdx = Function_dEdx(locBetaGammaValue, ddEdxMeanParams_FDC_Proton)/1000000.0;
    return NOERROR;
  }
  if((locPIDHypothesis == KPlus) || (locPIDHypothesis == KMinus)){
    locMeandEdx = Function_dEdx(locBetaGammaValue, ddEdxMeanParams_FDC_KPlus)/1000000.0;
    return NOERROR;
  }
  if((locPIDHypothesis == PiPlus) || (locPIDHypothesis == PiMinus)){
    locMeandEdx = Function_dEdx(locBetaGammaValue, ddEdxMeanParams_FDC_PiPlus)/1000000.0;
    return NOERROR;
  }
    
  return RESOURCE_UNAVAILABLE;;
}

jerror_t DParticleID_PID1::GetdEdxSigma_FDC(double locBeta, unsigned int locNumHitsUsedFordEdx, double& locSigmadEdx, Particle_t locPIDHypothesis) const
{
  double Nscale=ddEdxSigmaParams_FDC_N_dependence[0]
    *pow(double(locNumHitsUsedFordEdx),ddEdxSigmaParams_FDC_N_dependence[1])
    +ddEdxSigmaParams_FDC_N_dependence[2];
  double locBetaGammaValue = locBeta/sqrt(1.0 - locBeta*locBeta);
  if((locPIDHypothesis == Electron) || (locPIDHypothesis == Positron)){
    locSigmadEdx = Nscale*Function_dEdx(locBetaGammaValue, ddEdxSigmaParams_FDC_Electron)/1000000.0;
    return NOERROR;
  }
  if((locPIDHypothesis == Proton) || (locPIDHypothesis == AntiProton)){
    locSigmadEdx = Nscale*Function_dEdxSigma(locBetaGammaValue, ddEdxSigmaParams_FDC_Proton)/1000000.0;
    return NOERROR;
  }
  if((locPIDHypothesis == KPlus) || (locPIDHypothesis == KMinus)){
    locSigmadEdx = Nscale*Function_dEdxSigma(locBetaGammaValue, ddEdxSigmaParams_FDC_KPlus)/1000000.0;
    return NOERROR;
  }
  if((locPIDHypothesis == PiPlus) || (locPIDHypothesis == PiMinus)){
    locSigmadEdx = Nscale*Function_dEdxSigma(locBetaGammaValue, ddEdxSigmaParams_FDC_PiPlus)/1000000.0;
    return NOERROR;
  }

  return RESOURCE_UNAVAILABLE;
}

jerror_t DParticleID_PID1::CalcDCdEdxChiSq(DChargedTrackHypothesis *locChargedTrackHypothesis) const
{
	locChargedTrackHypothesis->Set_ChiSq_DCdEdx(0.0, 0);
	unsigned int locMinimumNumberUsedHitsForConfidence = 3; //dE/dx is landau-distributed, so to approximate Gaussian must remove hits with largest dE/dx //3 means 6 or more hits originally
	Particle_t locPID = locChargedTrackHypothesis->PID();
	double locChiSq=0.;
	unsigned int locNDF=0;

	auto locTrackTimeBased = locChargedTrackHypothesis->Get_TrackTimeBased();
	unsigned int locNumHitsUsedFordEdx_CDC = locTrackTimeBased->dNumHitsUsedFordEdx_CDC;
	unsigned int locNumHitsUsedFordEdx_FDC = locTrackTimeBased->dNumHitsUsedFordEdx_FDC;

	bool locUseCDCHitsFlag = (locNumHitsUsedFordEdx_CDC >= locMinimumNumberUsedHitsForConfidence) ? true : false;
	bool locUseFDCHitsFlag = (locNumHitsUsedFordEdx_FDC >= locMinimumNumberUsedHitsForConfidence) ? true : false;

	if((locUseCDCHitsFlag == false) && (locUseFDCHitsFlag == false))
		return RESOURCE_UNAVAILABLE; //not enough hits, use other sources of information for PID
	
	// Get the dEdx values for each detector group
	double locDCdEdx_FDC=locTrackTimeBased->ddEdx_FDC;
	double locDCdEdx_CDC=locChargedTrackHypothesis->Get_dEdx_CDC_amp(); //use space-charge corrected value

	double locMeandEdx_FDC, locMeandEdx_CDC, locSigmadEdx_FDC, locSigmadEdx_CDC;
	double locDeltadEdx_CDC = 0.0, locDeltadEdx_FDC = 0.0;

	double locBeta = locChargedTrackHypothesis->momentum().Mag()/locChargedTrackHypothesis->energy();
	
	if(locUseCDCHitsFlag && GetdEdxMean_CDC(locBeta, locNumHitsUsedFordEdx_CDC, locMeandEdx_CDC, locPID) == NOERROR)
	{
		if(GetdEdxSigma_CDC(locBeta, locNumHitsUsedFordEdx_CDC, locSigmadEdx_CDC, locPID) == NOERROR)
		{
			locDeltadEdx_CDC = locDCdEdx_CDC - locMeandEdx_CDC;
			double locNormalizedDifference = locDeltadEdx_CDC/locSigmadEdx_CDC;
			locChiSq += locNormalizedDifference*locNormalizedDifference;
			locNDF++;
		}
	}

	if(locUseFDCHitsFlag && GetdEdxMean_FDC(locBeta, locNumHitsUsedFordEdx_FDC, locMeandEdx_FDC, locPID) == NOERROR)
	{
		if(GetdEdxSigma_FDC(locBeta, locNumHitsUsedFordEdx_FDC, locSigmadEdx_FDC, locPID) == NOERROR)
		{
			locDeltadEdx_FDC = locDCdEdx_FDC - locMeandEdx_FDC;
			double locNormalizedDifference = locDeltadEdx_FDC/locSigmadEdx_FDC;
			locChiSq += locNormalizedDifference*locNormalizedDifference;
			locNDF++;
		}
	}

	if (locNDF>0)
		locChargedTrackHypothesis->Set_ChiSq_DCdEdx(locChiSq, locNDF);

	return NOERROR;
}

double DParticleID_PID1::GetTimeVariance(DetectorSystem_t detector,Particle_t particle,double p) const {
  double locSigma=0.;
	  
  if (particle==Proton || particle==AntiProton){
    switch(detector){
    case SYS_TOF:
      locSigma=dTimeSigmaParams_TOF_Proton[0]/(p*p) 
	+ dTimeSigmaParams_TOF_Proton[1]/p
	+ dTimeSigmaParams_TOF_Proton[2]
	+ dTimeSigmaParams_TOF_Proton[3]*p;
      break;  
    case SYS_BCAL:
      locSigma=dTimeSigmaParams_BCAL_Proton[0]/(p*p) 
	+ dTimeSigmaParams_BCAL_Proton[1]/p
	+ dTimeSigmaParams_BCAL_Proton[2]
	+ dTimeSigmaParams_BCAL_Proton[3]*p;
      break;  
    case SYS_FCAL:
      locSigma=dTimeSigmaParams_FCAL_Proton[0]/(p*p) 
	+ dTimeSigmaParams_FCAL_Proton[1]/p
	+ dTimeSigmaParams_FCAL_Proton[2]
	+ dTimeSigmaParams_FCAL_Proton[3]*p;
      break;
    default:
      break;    
    }
  }  
  else if (particle==KPlus || particle==KMinus){
    switch(detector){
    case SYS_TOF:
      locSigma=dTimeSigmaParams_TOF_KPlus[0]/(p*p) 
	+ dTimeSigmaParams_TOF_KPlus[1]/p
	+ dTimeSigmaParams_TOF_KPlus[2]
	+ dTimeSigmaParams_TOF_KPlus[3]*p;
      break;  
    case SYS_BCAL:
      locSigma=dTimeSigmaParams_BCAL_KPlus[0]/(p*p) 
	+ dTimeSigmaParams_BCAL_KPlus[1]/p
	+ dTimeSigmaParams_BCAL_KPlus[2]
	+ dTimeSigmaParams_BCAL_KPlus[3]*p;
      break;  
    case SYS_FCAL:
      locSigma=dTimeSigmaParams_FCAL_KPlus[0]/(p*p) 
	+ dTimeSigmaParams_FCAL_KPlus[1]/p
	+ dTimeSigmaParams_FCAL_KPlus[2]
	+ dTimeSigmaParams_FCAL_KPlus[3]*p;
      break;
    default:
      break;    
    }
  }
  else if (particle==PiPlus || particle==PiMinus){
    switch(detector){
    case SYS_TOF:
      locSigma=dTimeSigmaParams_TOF_PiPlus[0]/(p*p) 
	+ dTimeSigmaParams_TOF_PiPlus[1]/p
	+ dTimeSigmaParams_TOF_PiPlus[2]
	+ dTimeSigmaParams_TOF_PiPlus[3]*p;
      break;  
    case SYS_BCAL:
      locSigma=dTimeSigmaParams_BCAL_PiPlus[0]/(p*p) 
	+ dTimeSigmaParams_BCAL_PiPlus[1]/p
	+ dTimeSigmaParams_BCAL_PiPlus[2]
	+ dTimeSigmaParams_BCAL_PiPlus[3]*p;
      break;  
    case SYS_FCAL:
      locSigma=dTimeSigmaParams_FCAL_PiPlus[0]/(p*p) 
	+ dTimeSigmaParams_FCAL_PiPlus[1]/p
	+ dTimeSigmaParams_FCAL_PiPlus[2]
	+ dTimeSigmaParams_FCAL_PiPlus[3]*p;
      break;
    default:
      break;    
    }
  } 
  else if (particle==Electron || particle==Positron){
    switch(detector){
    case SYS_TOF:
      locSigma=dTimeSigmaParams_TOF_Positron[0]/(p*p) 
	+ dTimeSigmaParams_TOF_Positron[1]/p
	+ dTimeSigmaParams_TOF_Positron[2]
	+ dTimeSigmaParams_TOF_Positron[3]*p;
      break;  
    case SYS_BCAL:
      locSigma=dTimeSigmaParams_BCAL_Positron[0]/(p*p) 
	+ dTimeSigmaParams_BCAL_Positron[1]/p
	+ dTimeSigmaParams_BCAL_Positron[2]
	+ dTimeSigmaParams_BCAL_Positron[3]*p;
      break;  
    case SYS_FCAL:
      locSigma=dTimeSigmaParams_FCAL_Positron[0]/(p*p) 
	+ dTimeSigmaParams_FCAL_Positron[1]/p
	+ dTimeSigmaParams_FCAL_Positron[2]
	+ dTimeSigmaParams_FCAL_Positron[3]*p;
      break;
    default:
      break;    
    }
  }

  return locSigma*locSigma;	  
}
  
double DParticleID_PID1::GetTimeMean(DetectorSystem_t detector,Particle_t particle,double p) const {
  double locMean=0.;

  if (particle==Electron || particle==Positron){
    switch(detector){
    case SYS_BCAL:
      locMean=dTimeMeanParams_BCAL_Positron[0]/(p*p) 
	+ dTimeMeanParams_BCAL_Positron[1]/p
	+ dTimeMeanParams_BCAL_Positron[2]
	+ dTimeMeanParams_BCAL_Positron[3]*p;
      break;  
    case SYS_FCAL:
      locMean=dTimeMeanParams_FCAL_Positron[0]/(p*p) 
	+ dTimeMeanParams_FCAL_Positron[1]/p
	+ dTimeMeanParams_FCAL_Positron[2]
	+ dTimeMeanParams_FCAL_Positron[3]*p;
      break;
    default:
      break;    
    }
  }
  else if (particle==PiPlus || particle==PiMinus){
    switch(detector){
    case SYS_BCAL:
      locMean=dTimeMeanParams_BCAL_PiPlus[0]/(p*p) 
	+ dTimeMeanParams_BCAL_PiPlus[1]/p
	+ dTimeMeanParams_BCAL_PiPlus[2]
	+ dTimeMeanParams_BCAL_PiPlus[3]*p;
      break;  
    case SYS_FCAL:
      locMean=dTimeMeanParams_FCAL_PiPlus[0]/(p*p) 
	+ dTimeMeanParams_FCAL_PiPlus[1]/p
	+ dTimeMeanParams_FCAL_PiPlus[2]
	+ dTimeMeanParams_FCAL_PiPlus[3]*p;
      break;
    default:
      break;    
    }
  } 
  else if (particle==KPlus || particle==KMinus){
    switch(detector){
    case SYS_BCAL:
      locMean=dTimeMeanParams_BCAL_KPlus[0]/(p*p) 
	+ dTimeMeanParams_BCAL_KPlus[1]/p
	+ dTimeMeanParams_BCAL_KPlus[2]
	+ dTimeMeanParams_BCAL_KPlus[3]*p;
      break;  
    case SYS_FCAL:
      locMean=dTimeMeanParams_FCAL_KPlus[0]/(p*p) 
	+ dTimeMeanParams_FCAL_KPlus[1]/p
	+ dTimeMeanParams_FCAL_KPlus[2]
	+ dTimeMeanParams_FCAL_KPlus[3]*p;
      break;
    default:
      break;    
    }
  }
 else if (particle==Proton || particle==AntiProton){
    switch(detector){
    case SYS_BCAL:
      locMean=dTimeMeanParams_BCAL_Proton[0]/(p*p) 
	+ dTimeMeanParams_BCAL_Proton[1]/p
	+ dTimeMeanParams_BCAL_Proton[2]
	+ dTimeMeanParams_BCAL_Proton[3]*p;
      break;  
    case SYS_FCAL:
      locMean=dTimeMeanParams_FCAL_Proton[0]/(p*p) 
	+ dTimeMeanParams_FCAL_Proton[1]/p
	+ dTimeMeanParams_FCAL_Proton[2]
	+ dTimeMeanParams_FCAL_Proton[3]*p;
      break;
    default:
      break;    
    }
  }


  return locMean;
}
  

