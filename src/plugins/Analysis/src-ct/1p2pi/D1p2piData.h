
#ifndef _D1P2PIDATA_
#define _D1P2PIDATA_


#include <JANA/JObject.h>
#include <JANA/JFactory.h>
using namespace jana;

#include <vector>

class D1p2piData : public JObject{
	
public:
	
  JOBJECT_PUBLIC(D1p2piData);
	
  D1p2piData(){}
    
  int nHyp;
  int nHyp_fitted;
  int nPhotonCandidates;
  
  vector<double> bmE;
  vector<double> bmtime;
  
  int eventNumber;
  Double_t L1TriggerBits;
  
  vector<double> T_vertex;
  vector<double> X_vertex;
  vector<double> Y_vertex;
  vector<double> Z_vertex;
  vector<double> CLKinFit;
  vector<double> NDF;
  vector<double> ChiSqFit;
  vector<double> Common_Time;
  
  vector<double> pX_piminus;
  vector<double> pY_piminus;
  vector<double> pZ_piminus;
  vector<double> E_piminus;
  vector<double> t_piminus;

  vector<double> pX_piplus;
  vector<double> pY_piplus;
  vector<double> pZ_piplus;
  vector<double> E_piplus;
  vector<double> t_piplus;

  vector<double> pX_proton;
  vector<double> pY_proton;
  vector<double> pZ_proton;
  vector<double> E_proton;
  vector<double> t_proton;

};



#endif  // _D1P2PIDATA_