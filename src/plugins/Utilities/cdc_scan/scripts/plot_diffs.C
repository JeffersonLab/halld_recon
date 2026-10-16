{

  // script to find differences between reported and emulated fa125 output in a root tree produced by cdc_scan

  int rocs[] = {25,26,27,28,52,53,55,56,57,58,59,60,61,62};
  int nrocs = 14;

  int slots[] = {3,4,5,6,7,8,9,10,13,14,15,16,17,18,19,20};
  int nslots = 15;
    
  P->Draw("rocid:slot","","colz");
  gPad->SaveAs("roc_slot.png");
  
  P->Draw("rocid:slot","diffs","colz");
  gPad->SaveAs("roc_slot_diffs.png");
  
  
  for (int i = 0; i< nrocs; i++) {
    int roc = rocs[i];

    if (P->GetEntries(Form("diffs && rocid==%i",roc)) == 0) continue;

    P->Draw("channel:slot",Form("rocid==%i",roc),"colz");
    gPad->SaveAs(Form("roc_%i.png",roc));

    P->Draw("channel:slot",Form("diffs&&rocid==%i",roc),"colz");
    gPad->SaveAs(Form("roc_%i_diffs.png",roc));
    
    for (int j = 0; j < nslots; j++) {

      int slot = slots[j];

      if (P->GetEntries(Form("diffs && rocid==%i && slot==%i",roc,slot)) == 0) continue;

      P->Draw("channel",Form("diffs&&rocid==%i&&slot==%i",roc,slot));
      gPad->SaveAs(Form("roc_%i_slot_%i_diffs.png",roc,slot));

      for (int k=0; k<72; k++) {

	int ndiffs = P->GetEntries(Form("diffs && rocid==%i && slot==%i && channel==%i",roc,slot,k));
	int nentries = P->GetEntries(Form("rocid==%i && slot==%i && channel==%i",roc,slot,k));

	if (ndiffs>0) {

          printf("rocid %i slot %i channel %i diffs %i entries %i rate %.3f%%\n",roc,slot,k,ndiffs,nentries,100*(float)ndiffs/(float)nentries);
	}  
      }

      P->Scan("rocid:slot:channel:d_time:d_q:d_pedestal:d_integral:d_amp:d_overflows:d_pktime",Form("diffs&&rocid==%i&&slot==%i",roc,slot));

      printf("\n");
      
    }

  }

}
