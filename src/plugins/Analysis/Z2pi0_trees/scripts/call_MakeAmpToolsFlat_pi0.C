#include "MakeAmpToolsFlat_pi0.C"
void call_MakeAmpToolsFlat_pi0 (Int_t foption=1)
{
// issue the tree->Loop() from the command line.
//
  // gROOT->ProcessLine(".L MakeAmpToolsFlat_pi0.C");
  //gROOT->LoadMacro("MakeAmpToolsFlat_pi0.C");
 MakeAmpToolsFlat_pi0 t;
 t.Loop(foption);
}
