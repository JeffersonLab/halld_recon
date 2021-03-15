#include "MakeAmpToolsFlat_mcthrown_pi0.C"
void call_MakeAmpToolsFlat_mcthrown_pi0 ()
{
// issue the tree->Loop() from the command line.
//
  // gROOT->ProcessLine(".L MakeAmpToolsFlat_mcthrown_pi0.C");
  //gROOT->LoadMacro("MakeAmpToolsFlat_mcthrown_pi0.C");
 MakeAmpToolsFlat_mcthrown_pi0 t;
 t.Loop();
}
