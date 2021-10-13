void checking() {

  ifstream in;

  TString str_run_list1[10000];
  TString str_run_list2[10000];
  TString str_evio_list[10000];

  int run_list1[10000];
  int run_list2[10000];
  int evio_list[10000];

  int run_done[10000];
  int evio_done[10000];

  int i = 0;
  in.open("all-batches.txt");
  while (in.good()) {
    int run1 = 0, run2 = 0, evio = 0;
    in >> run1 >> run2 >> evio;
    run_list1[i] = run1;
    run_list2[i] = run2;
    evio_list[i] = evio;
    i ++;
  }
  in.close();

  i = 0;
  in.open("all-batches.txt");
  while (in.good()) {
    in >> str_run_list1[i] >> str_run_list2[i] >> str_evio_list[i];
    i ++;
  }
  in.close();

  int j = 0;
  in.open("batch-done.txt");
  while (in.good()) {
    int run = 0, evio = 0;
    in >> run >> evio;
    run_done[j] = run;
    evio_done[j] = evio;
    j ++;
  }
  in.close();
  
  for (int k = 0; k < i; k ++) {
    bool todo = false;
    for (int l = 0; l < j; l ++) {
      if (run_list1[k] == run_done[l]) todo = true;
    }
    TString file = Form("/cache/halld/RunPeriod-2019-01/rawdata/Run%s/hd_rawdata_%s_%s.evio", str_run_list1[k].Data(), str_run_list2[k].Data(), str_evio_list[k].Data());
    if (!todo) cout << file << endl;
  }

}
