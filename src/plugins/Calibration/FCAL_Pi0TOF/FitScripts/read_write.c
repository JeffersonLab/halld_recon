#include <iostream>
#include <fstream>
#include <sstream> 
using namespace std;
#include <TFile.h>
#include <TGraphErrors.h>
#include <TMath.h>
#include <TGraph2D.h>
#include <TRandom.h>
#include <TRandom3.h>
#include <TStyle.h>
#include <TText.h>
#include <TCanvas.h>
//#include <TF2.h>
#include <TF1.h>
#include <TH1.h>
#include <TH2.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TROOT.h>
#include <TLegend.h>
#include <TFitResult.h>
#include <TRatioPlot.h>
#include <TProfile2D.h>
#include <TLatex.h>
#include <TLine.h>
#include <TGaxis.h>
#include "TLorentzVector.h"
#include "TVector3.h"
#include "TLorentzRotation.h"
#include <TGenPhaseSpace.h>

#include "/work/halld/home/gxproj2/calibration/ilib/Nicer2D.c"
#include "/work/halld/home/gxproj2/calibration/ilib/Nicer1D.c"
#include "/work/halld/home/gxproj2/calibration/ilib/Nice1D.c"
#include "/work/halld/home/gxproj2/calibration/ilib/Nice3D2D.c"
/*
#include "/home/igal/root/ilib/Nicer2D.c"
#include "/home/igal/root/ilib/Nicer1D.c"
#include "/home/igal/root/ilib/Nice1D.c"
#include "/home/igal/root/ilib/Nice3D2D.c"
*/
//#define ITERATION 7

void read_write () {
  
  TString str_ped = "";
  TString str_com = "";

  int min_range[] = {61321, 61331, 61342, 61351, 61355, 61360, 61372, 61390, 61419, 61427, 61433, 61438, 61466, 61479, 61490, 61502, 61516, 61530, 61548, 61561, 61580, 61592, 61609, 61620, 61630, 61644, 61656, 61670, 61682, 61697, 61707, 61720, 61734, 61746, 61758, 61766, 61780, 61794, 61807, 61820, 61833, 61847, 61857, 61875, 61889, 61908, 61929, 61952};
  int max_range[] = {61330, 61341, 61350, 61354, 61359, 61371, 61389, 61418, 61426, 61431, 61437, 61465, 61478, 61489, 61501, 61515, 61529, 61547, 61560, 61579, 61591, 61608, 61619, 61629, 61643, 61655, 61669, 61681, 61695, 61706, 61719, 61732, 61745, 61757, 61765, 61779, 61793, 61806, 61818, 61832, 61846, 61855, 61874, 61888, 61906, 61928, 61951, 61956};
  /*
  for (int i = 1; i <= 48; i ++) {
    int run = (min_range[i - 1] + max_range[i - 1]) / 2;
    str_ped = Form("period_%d.txt", i);
    // 4 5 6 - 9 10 - 21 22 23 - 28
    if ((1 <= i && i <= 3) || (7 <= i && i <= 8) || (11 <= i && i <= 20) || (24 <= i && i <= 27) || (29 <= i && i <= 48)) {
      str_com = Form("ccdb dump FCAL/pedestals -v primex_cal -r %d > ped-%s", run, str_ped.Data());
      cout << str_com << endl;
      system(str_com);
      str_com = Form("ccdb dump FCAL/gains -v primex_cal -r %d > gai-%s", run, str_ped.Data());
      system(str_com);
      cout << str_com << endl;
      str_com = Form("ccdb add FCAL/pedestals -r %d-%d ped-%s", min_range[i - 1], max_range[i - 1], str_ped.Data());
      cout << str_com << endl;
      system(str_com);
      str_com = Form("ccdb add FCAL/gains -r %d-%d gai-%s", min_range[i - 1], max_range[i - 1], str_ped.Data());
      cout << str_com << endl;
      system(str_com);
    }
  }
  */
  for (int i = 1; i <= 48; i ++) {
    str_com = "";
    if (i == 4) str_com = Form("ccdb add FCAL/pedestals -r %d-%d ped-period_3.txt", min_range[2], max_range[2]);
    if (i == 5) str_com = Form("ccdb add FCAL/pedestals -r %d-%d ped-period_3.txt", min_range[2], max_range[2]);
    if (i == 6) str_com = Form("ccdb add FCAL/pedestals -r %d-%d ped-period_7.txt", min_range[6], max_range[6]);
    if (i == 9) str_com = Form("ccdb add FCAL/pedestals -r %d-%d ped-period_8.txt", min_range[7], max_range[7]);
    if (i == 10) str_com = Form("ccdb add FCAL/pedestals -r %d-%d ped-period_11.txt", min_range[10], max_range[10]);
    if (i == 21) str_com = Form("ccdb add FCAL/pedestals -r %d-%d ped-period_20.txt", min_range[19], max_range[19]);
    if (i == 22) str_com = Form("ccdb add FCAL/pedestals -r %d-%d ped-period_20.txt", min_range[19], max_range[19]);
    if (i == 23) str_com = Form("ccdb add FCAL/pedestals -r %d-%d ped-period_24.txt", min_range[23], max_range[23]);
    if (i == 28) str_com = Form("ccdb add FCAL/pedestals -r %d-%d ped-period_27.txt", min_range[26], max_range[26]);
    system(str_com);
    if (i == 4) str_com = Form("ccdb add FCAL/gains -r %d-%d gai-period_3.txt", min_range[2], max_range[2]);
    if (i == 5) str_com = Form("ccdb add FCAL/gains -r %d-%d gai-period_3.txt", min_range[2], max_range[2]);
    if (i == 6) str_com = Form("ccdb add FCAL/gains -r %d-%d gai-period_7.txt", min_range[6], max_range[6]);
    if (i == 9) str_com = Form("ccdb add FCAL/gains -r %d-%d gai-period_8.txt", min_range[7], max_range[7]);
    if (i == 10) str_com = Form("ccdb add FCAL/gains -r %d-%d gai-period_11.txt", min_range[10], max_range[10]);
    if (i == 21) str_com = Form("ccdb add FCAL/gains -r %d-%d gai-period_20.txt", min_range[19], max_range[19]);
    if (i == 22) str_com = Form("ccdb add FCAL/gains -r %d-%d gai-period_20.txt", min_range[19], max_range[19]);
    if (i == 23) str_com = Form("ccdb add FCAL/gains -r %d-%d gai-period_24.txt", min_range[23], max_range[23]);
    if (i == 28) str_com = Form("ccdb add FCAL/gains -r %d-%d gai-period_27.txt", min_range[26], max_range[26]);
    system(str_com);
  }
  
}
