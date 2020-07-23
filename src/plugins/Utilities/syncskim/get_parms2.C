
// This is an alternate method to print the parameters used to
// convert timestamps to unix time. This gets the values from
// the conversion_tree in the file synskim.root. You can run it
// like this (with the syncskim.root file in the same directory.
//
// root -l syncskim.root


void get_parms2(void)
{

	auto chain = new TChain("conversion_tree", "conversion params chain");
	chain->AddFile("syncskim.root");
	
	ULong_t tics_per_sec;
	ULong_t unix_start_time;
	chain->SetBranchAddress("tics_per_sec", &tics_per_sec);
	chain->SetBranchAddress("unix_start_time", &unix_start_time);
	chain->GetEntry(0);
	
	cout << endl << "timestamp to unix time conversion: tics_per_sec=" << tics_per_sec << " unix_start_time=" << unix_start_time << endl << endl;
}

