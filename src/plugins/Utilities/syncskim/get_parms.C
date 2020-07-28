


void get_parms(void)
{

	auto chain = new TChain("synctree", "synctree chain");
	chain->AddFile("syncskim_000.root");
	chain->AddFile("syncskim_001.root");
	chain->AddFile("syncskim_002.root");
	chain->AddFile("syncskim_003.root");
	chain->AddFile("syncskim_004.root");
	//chain->Print();

	chain->SetMarkerStyle(20);
	chain->Draw("unix_time-13E8:avg_timestamp/250.0E6");
	auto graph = (TGraph*)gPad->GetPrimitive("Graph");
	graph->Fit("pol1");
	
	double b = graph->GetFunction("pol1")->GetParameter(0);
	double m = graph->GetFunction("pol1")->GetParameter(1);
	
	b += 13.0E8;
	double one_over_m = 250.0E6/m;

	typedef std::numeric_limits< double > dbl;
	cout.precision(dbl::max_digits10);
	
	cout << endl << "timestamp to unix time conversion: tics_per_sec=" << one_over_m << " unix_start_time=" << b << endl << endl;
}

