void call_DSelector2 (TString file)
{
// issue the tree->Process, so that it can be run from the command line
//
cout << "call_DSelector2: file=" << file << endl;
gROOT->LoadMacro("$ROOT_ANALYSIS_HOME/scripts/Load_DSelector.C");
pi0pi0misspb208__B2_Tree->Process("DSelector_Z2pi0_trees2.C+");
}
