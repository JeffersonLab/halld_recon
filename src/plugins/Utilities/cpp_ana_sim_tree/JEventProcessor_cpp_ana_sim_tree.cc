// $Id$
//
//    File: JEventProcessor_cpp_ana_sim_tree.cc
// Created: Tue Jul 28 01:42:19 PM EDT 2026
// Creator: alfab (on Linux ifarm2401.jlab.org 5.14.0-611.55.1.el9_7.x86_64 x86_64)
//

/// For more information on the syntax changes between JANA1 and JANA2, visit: https://jeffersonlab.github.io/JANA2/#/jana1to2/jana1-to-jana2

#include "JEventProcessor_cpp_ana_sim_tree.h"


// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
extern "C"{
void InitPlugin(JApplication *app){
    InitJANAPlugin(app);
    app->Add(new JEventProcessor_cpp_ana_sim_tree());
}
} // "C"


//------------------
// JEventProcessor_cpp_ana_sim_tree (Constructor)
//------------------
JEventProcessor_cpp_ana_sim_tree::JEventProcessor_cpp_ana_sim_tree()
{
	SetTypeName(NAME_OF_THIS); // Provide JANA with this class's name

    // Parameters and Services should be accessed from Init() instead of here!
}

//------------------
// ~JEventProcessor_cpp_ana_sim_tree (Destructor)
//------------------
JEventProcessor_cpp_ana_sim_tree::~JEventProcessor_cpp_ana_sim_tree()
{
}

//------------------
// Init
//------------------
void JEventProcessor_cpp_ana_sim_tree::Init()
{
    // This is called once at program startup. 
    
    auto app = GetApplication();
    // lockService should be initialized here like this
    lockService = app->GetService<JLockService>();
    vector< string > varsMinus( inputVarsMinus, inputVarsMinus + sizeof( inputVarsMinus )/sizeof( char* ) );
    dEPIClassifierMinus = new ReadMLPMinus( varsMinus );
    vector< string > varsPlus( inputVarsPlus, inputVarsPlus + sizeof( inputVarsPlus )/sizeof( char* ) );
    dEPIClassifierPlus = new ReadMLPPlus( varsPlus );

    tof_count = 0;
    fcal_count = 0;
    
    SPLIT_CUT=0.5;
    FCAL_THRESHOLD=0.1;
    BCAL_THRESHOLD=0.05;
    GAMMA_DT_CUT=2.0;

    h2_vertex_x_y = new TH2D("h2_vertex_x_y","Vertex location",100,-10,10,100,-10,10);
    h1_vertex_z = new TH1D("h1_vertex_z","Vertex Z Position",150,-100,500);
    h1_t0_rf = new TH1D("h1_t0_rf","t0_rf",150,-50,50);

    h1_tof_dx = new TH1D("h1_tof_dx","TOF dx",100,-20,20);
    h1_tof_dy = new TH1D("h1_tof_dy","TOF dy",100,-20,20);
    
    h_u = new TH1D("h_u", "Cut Progress", 30, -0.5, 29.5);
    h_zvtx = new TH1D("h_zvtx","Vertex Z Position",100,-100,500);
    h_xyvtx = new TH2D("h_xyvtx", "Vertex x and y position",100,-10,10,100,-10,10);

    h1_muplus_chisq = new TH1D("h1_muplus_chisq","#mu^{+} #chi^{2}/dof",100,0,10);
    h1_muminus_chisq = new TH1D("h1_muminus_chisq","#mu^{-} #chi^{2}/dof",100,0,10);

    h1_muplus_energy = new TH1D("h1_muplus_energy","#mu^{+} energy",100,0,12);
    h1_muminus_energy = new TH1D("h1_muminus_energy","#mu^{-} energy",100,0,12);
    h1_2c_energy = new TH1D("h1_2c_energy","2#mu energy",100,0,12);

    h1_muplus_energy_nocut = new TH1D("h1_muplus_energy_nocut","#mu^{+} Energy No Cut",100,0,12);
    h1_muminus_energy_nocut = new TH1D("h1_muminus_energy_nocut","#mu^{-} Energy No Cut",100,0,12);
    h1_2c_energy_nocut = new TH1D("h1_2c_energy_nocut","2#mu energy",100,0,12);

    h1_energy_combos = new TH1I("h1_energy_combos","Energy Combos",6,0,6);

    h1_fcal_event_type = new TH1I("h1_fcal_event_type","FCAL Event Type",3,0,3);

    h1_bar1_track_multiplicity = new TH1I("h1_bar1_track_multiplicity","Bar 1 Track Multiplicity",145,0,145);
    h1_bar2_track_multiplicity = new TH1I("h1_bar2_track_multiplicity","Bar 2 Track Multiplicity",145,0,145);
    h1_bar3_track_multiplicity = new TH1I("h1_bar3_track_multiplicity","Bar 3 Track Multiplicity",145,0,145);
    h1_bar4_track_multiplicity = new TH1I("h1_bar4_track_multiplicity","Bar 4 Track Multiplicity",145,0,145);

    h1_bar1_tm_plus = new TH1I("h1_bar1_tm_plus","Bar 1 #mu^{+} Track Multiplicity",145,0,145);
    h1_bar2_tm_plus = new TH1I("h1_bar2_tm_plus","Bar 2 #mu^{+} Track Multiplicity",145,0,145);
    h1_bar3_tm_plus = new TH1I("h1_bar3_tm_plus","Bar 3 #mu^{+} Track Multiplicity",145,0,145);
    h1_bar4_tm_plus = new TH1I("h1_bar4_tm_plus","Bar 4 #mu^{+} Track Multiplicity",145,0,145);

    h1_bar1_tm_minus = new TH1I("h1_bar1_tm_plus","Bar 1 #mu^{-} Track Multiplicity",145,0,145);
    h1_bar2_tm_minus = new TH1I("h1_bar2_tm_plus","Bar 2 #mu^{-} Track Multiplicity",145,0,145);
    h1_bar3_tm_minus = new TH1I("h1_bar3_tm_plus","Bar 3 #mu^{-} Track Multiplicity",145,0,145);
    h1_bar4_tm_minus = new TH1I("h1_bar4_tm_plus","Bar 4 #mu^{-} Track Multiplicity",145,0,145);
    

    h1_mlp_score_plus = new TH1D("h1_mlp_score_plus","MLP Response",100,-0.1,1.1);
    h1_mlp_score_minus = new TH1D("h1_mlp_score_minus","MLP Response",100,-0.1,1.1);

    ofile = new TFile("cpp_sim_to_classify.root","RECREATE");
    ptree = new TTree("ptree","Training Features");

    nChargedTracks = ptree->Branch("nChargedTracks", &nCT, "nCT/F");
    nFCALShowers = ptree->Branch("nFCALShowers", &nFS, "nFS/F");
    nFCALhits = ptree->Branch("nFCALhits", &nFh,"nFh/F");
    nMWPChits = ptree->Branch("nMWPChits",&nMh,"nMh/F");
    nFMWPCMatchedTracks = ptree->Branch("nFMWPCMatchedTracks",&nFMT,"nFMT/F");
    FCAL_E_center_8 = ptree->Branch("FCAL_E_center_8",&FEc8,"FEc8/F");
    FCAL_E_3x3_8 = ptree->Branch("FCAL_E_3x3_8",&FE38,"FE38/F");
    FCAL_E_5x5_8 = ptree->Branch("FCAL_E_5x5_8",&FE58,"FE58/F");
    FMWPC_closest_wire1_8 = ptree->Branch("FMWPC_closest_wire1_8",&Fcw18,"Fcw18/F");
    FMWPC_dist_closest_wire1_8 = ptree->Branch("FMWPC_dist_closest_wire1_8",&Fdcw18,"Fdcw18/F");
    FMWPC_Nhits_cluster1_8 = ptree->Branch("FMWPC_Nhits_cluster1_8",&FNc18,"FNc18/F");
    FMWPC_closest_wire2_8 = ptree->Branch("FMWPC_closest_wire2_8",&Fcw28,"Fcw28/F");
    FMWPC_dist_closest_wire2_8 =ptree->Branch("FMWPC_dist_closest_wire2_8",&Fdcw28,"Fdcw28/F");
    FMWPC_Nhits_cluster2_8 = ptree->Branch("FMWPC_Nhits_cluster2_8",&FNc28,"FNc28/F");
    FMWPC_closest_wire3_8 = ptree->Branch("FMWPC_closest_wire3_8",&Fcw38,"Fcw38/F");
    FMWPC_dist_closest_wire3_8 =ptree->Branch("FMWPC_dist_closest_wire3_8",&Fdcw38,"Fdcw38/F");
    FMWPC_Nhits_cluster3_8 = ptree->Branch("FMWPC_Nhits_cluster3_8",&FNc38,"FNc38/F");
    FMWPC_closest_wire4_8 = ptree->Branch("FMWPC_closest_wire4_8",&Fcw48,"Fcw48/F");
    FMWPC_dist_closest_wire4_8 =ptree->Branch("FMWPC_dist_closest_wire4_8",&Fdcw48,"Fdcw48/F");
    FMWPC_Nhits_cluster4_8 = ptree->Branch("FMWPC_Nhits_cluster4_8",&FNc48,"FNc48/F");
    FMWPC_closest_wire5_8 = ptree->Branch("FMWPC_closest_wire5_8",&Fcw58,"Fcw58/F");
    FMWPC_dist_closest_wire5_8 =ptree->Branch("FMWPC_dist_closest_wire5_8",&Fdcw58,"Fdcw58/F");
    FMWPC_Nhits_cluster5_8 = ptree->Branch("FMWPC_Nhits_cluster5_8",&FNc58,"FNc58/F");
    FMWPC_closest_wire6_8 = ptree->Branch("FMWPC_closest_wire6_8",&Fcw68,"Fcw68/F");
    FMWPC_dist_closest_wire6_8 =ptree->Branch("FMWPC_dist_closest_wire6_8",&Fdcw68,"Fdcw68/F");
    FMWPC_Nhits_cluster6_8 = ptree->Branch("FMWPC_Nhits_cluster6_8",&FNc68,"FNc68/F");
    FCAL_E_center_9 = ptree->Branch("FCAL_E_center_9",&FEc9,"FEc9/F");
    FCAL_E_3x3_9 = ptree->Branch("FCAL_E_3x3_9",&FE39,"FE39/F");
    FCAL_E_5x5_9 = ptree->Branch("FCAL_E_5x5_9",&FE59,"FE59/F");
    FMWPC_closest_wire1_9 = ptree->Branch("FMWPC_closest_wire1_9",&Fcw19,"Fcw19/F");
    FMWPC_dist_closest_wire1_9 = ptree->Branch("FMWPC_dist_closest_wire1_9",&Fdcw19,"Fdcw19/F");
    FMWPC_Nhits_cluster1_9 = ptree->Branch("FMWPC_Nhits_cluster1_9",&FNc19,"FNc19/F");
    FMWPC_closest_wire2_9 = ptree->Branch("FMWPC_closest_wire2_9",&Fcw29,"Fcw29/F");
    FMWPC_dist_closest_wire2_9 =ptree->Branch("FMWPC_dist_closest_wire2_9",&Fdcw29,"Fdcw29/F");
    FMWPC_Nhits_cluster2_9 = ptree->Branch("FMWPC_Nhits_cluster2_9",&FNc29,"FNc29/F");
    FMWPC_closest_wire3_9 = ptree->Branch("FMWPC_closest_wire3_9",&Fcw39,"Fcw39/F");
    FMWPC_dist_closest_wire3_9 =ptree->Branch("FMWPC_dist_closest_wire3_9",&Fdcw39,"Fdcw39/F");
    FMWPC_Nhits_cluster3_9 = ptree->Branch("FMWPC_Nhits_cluster3_9",&FNc39,"FNc39/F");
    FMWPC_closest_wire4_9 = ptree->Branch("FMWPC_closest_wire4_9",&Fcw49,"Fcw49/F");
    FMWPC_dist_closest_wire4_9 =ptree->Branch("FMWPC_dist_closest_wire4_9",&Fdcw49,"Fdcw49/F");
    FMWPC_Nhits_cluster4_9 = ptree->Branch("FMWPC_Nhits_cluster4_9",&FNc49,"FNc49/F");
    FMWPC_closest_wire5_9 = ptree->Branch("FMWPC_closest_wire5_9",&Fcw59,"Fcw59/F");
    FMWPC_dist_closest_wire5_9 =ptree->Branch("FMWPC_dist_closest_wire5_9",&Fdcw59,"Fdcw59/F");
    FMWPC_Nhits_cluster5_9 = ptree->Branch("FMWPC_Nhits_cluster5_9",&FNc59,"FNc59/F");
    FMWPC_closest_wire6_9 = ptree->Branch("FMWPC_closest_wire6_9",&Fcw69,"Fcw69/F");
    FMWPC_dist_closest_wire6_9 =ptree->Branch("FMWPC_dist_closest_wire6_9",&Fdcw69,"Fdcw69/F");
    FMWPC_Nhits_cluster6_9 = ptree->Branch("FMWPC_Nhits_cluster6_9",&FNc69,"FNc69/F");

    //New Variables
    TRACK_PLUS_MOM = ptree->Branch("TRACK_PLUS_MOM",&TPm,"TPm/F");
    TRACK_MINUS_MOM = ptree->Branch("TRACK_MINUS_MOM",&TMm,"TMm/F");
    FCAL_e_8 = ptree->Branch("FCAL_e_8",&fcale8,"fcale8/F");
    FCAL_eoverp_8 = ptree->Branch("FCAL_eoverp_8",&fcalep8,"fcalep8/F");
    FCAL_doca_8 = ptree->Branch("FCAL_doca_8",&fcaldoca8,"fcaldoca8/F");
    FCAL_e9e25_8 = ptree->Branch("FCAL_e9e25_8",&fcale9e258,"fcale9e258/F");
    FCAL_blocks_n_8 = ptree->Branch("FCAL_blocks_n_8",&fcalblocksn8,"fcalblocksn8/F");
    FCAL_e_9 = ptree->Branch("FCAL_e_9",&fcale9,"fcale9/F");
    FCAL_eoverp_9 = ptree->Branch("FCAL_eoverp_9",&fcalep9,"fcalep9/F");
    FCAL_doca_9 = ptree->Branch("FCAL_doca_9",&fcaldoca9,"fcaldoca9/F");
    FCAL_e9e25_9 = ptree->Branch("FCAL_e9e25_9",&fcale9e259,"fcale9e259/F");
    FCAL_blocks_n_9 = ptree->Branch("FCAL_blocks_n_9",&fcalblocksn9,"fcalblocksn9/F");
    FCAL_E1E9_8 = ptree->Branch("FCALShower_E1E9_8",&fcale1e98,"fcale1e98/F");
    FCAL_E1E9_9 = ptree->Branch("FCALShower_E1E9_9",&fcale1e99,"fcale1e99/F");
    FCALSumU8 = ptree->Branch("FCALSumU8",&sumu8,"sumu8/F");
    FCALSumU9 = ptree->Branch("FCALSumU9",&sumu9,"sumu9/F");
    FCALSumV8 = ptree->Branch("FCALSumV8",&sumv8,"sumv8/F");
    FCALSumV9 = ptree->Branch("FCALSumV9",&sumv9,"sumv9/F");
    FMWPC_chamber1 = ptree->Branch("FMWPC_chamber1",&fmwpc1n,"fmwpc1n/F");
    FMWPC_chamber2 = ptree->Branch("FMWPC_chamber2",&fmwpc2n,"fmwpc2n/F");
    FMWPC_chamber3 = ptree->Branch("FMWPC_chamber3",&fmwpc3n,"fmwpc3n/F");
    FMWPC_chamber4 = ptree->Branch("FMWPC_chamber4",&fmwpc4n,"fmwpc4n/F");
    FMWPC_chamber5 = ptree->Branch("FMWPC_chamber5",&fmwpc5n,"fmwpc5n/F");
    FMWPC_chamber6 = ptree->Branch("FMWPC_chamber6",&fmwpc6n,"fmwpc6n/F");
    chisq_8 = ptree->Branch("CHI_squared_ndof_8",&cn8,"cn8/F");
    chisq_9 = ptree->Branch("CHI_squared_ndof_9",&cn9,"cn9/F");

    InvariantMass = ptree->Branch("InvariantMass",&w_2pi,"w_2pi/F");
    Pion1_px = ptree->Branch("Pion1_px",&p1x,"p1x/F");
    Pion1_py = ptree->Branch("Pion1_py",&p1y,"p1y/F");
    Pion1_pz = ptree->Branch("Pion1_pz",&p1z,"p1z/F");
    Pion1_pe = ptree->Branch("Pion1_pe",&p1e,"p1e/F");
    Pion2_px = ptree->Branch("Pion2_px",&p2x,"p2x/F");
    Pion2_py = ptree->Branch("Pion2_py",&p2y,"p2y/F");
    Pion2_pz = ptree->Branch("Pion2_pz",&p2z,"p2z/F");
    Pion2_pe = ptree->Branch("Pion2_pe",&p2e,"p2e/F");

    EPI_MLP_Response_plus = ptree->Branch("EPI_MLP_Response_plus",&epimlp_plus,"epimlp_plus/F");
    EPI_MLP_Response_minus = ptree->Branch("EPI_MLP_Response_minus",&epimlp_minus,"epimlp_minus/F");

    Beam_Energy = ptree->Branch("Beam_Energy",&bE,"be/F");
    Beam_Time = ptree->Branch("Beam_Time",&bT,"bT/F");
    Track1_Time = ptree->Branch("Track1_Time",&t1t,"t1t/F");
    Track2_Time = ptree->Branch("Track2_time",&t2t,"t2t/F");
    Track1_Energy = ptree->Branch("Track1_Energy",&t1e,"t1e/F");
    Track2_Energy = ptree->Branch("Track2_Energy",&t2e,"t2e/F");
    Elasticity = ptree->Branch("Elasticity",&elas,"elas/F");
    Weight = ptree->Branch("Weight",&wght,"wght/F");
    Tagger_ID = ptree->Branch("Tagger_ID",&tagid,"tagid/F");
    KinFit_Chisq = ptree->Branch("KinFit_Chisq",&mm_chi,"mm_chi/F");
    Track_Mom_8_KinFit = ptree->Branch("Track_Mom_8_KinFit",&mp_8_kf,"mp_8_kf/F");
    Track_Mom_9_KinFit = ptree->Branch("Track_Mom_9_KinFit",&mp_9_kf,"mp_9_kf/F");
    FCAL_eoverp_kf_8 = ptree->Branch("FCAL_eoverp_kf_8",&fcalepkf8,"fcalepkf8/F");
    FCAL_eoverp_kf_9 = ptree->Branch("FCAL_eoverp_kf_9",&fcalepkf9,"fcalepkf9/F");
    EPI_MLP_Response_KF8 = ptree->Branch("EPI_MLP_Response_KF8",&mlp_kf_8,"mlp_kf_8/F");
    EPI_MLP_Response_KF9 = ptree->Branch("EPI_MLP_Response_KF9",&mlp_kf_9,"mlp_kf_9/F");
    InvariantMass_KF = ptree->Branch("InvariantMass_KF",&w_2mu_kf,"w_2mu_kf/F");
    Pion1_px_KF = ptree->Branch("Pion1_px_KF",&p1x_kf,"p1x_kf/F");
    Pion1_py_KF = ptree->Branch("Pion1_py_KF",&p1y_kf,"p1y_kf/F");
    Pion1_pz_KF = ptree->Branch("Pion1_pz_KF",&p1z_kf,"p1z_kf/F");
    Pion1_pe_KF = ptree->Branch("Pion1_pe_KF",&p1e_kf,"p1e_kf/F");
    Pion2_px_KF = ptree->Branch("Pion2_px_KF",&p2x_kf,"p2x_kf/F");
    Pion2_py_KF = ptree->Branch("Pion2_py_KF",&p2y_kf,"p2y_kf/F");
    Pion2_pz_KF = ptree->Branch("Pion2_pz_KF",&p2z_kf,"p2z_kf/F");
    Pion2_pe_KF = ptree->Branch("Pion2_pe_KF",&p2e_kf,"p2e_kf/F");
    KinFit_NDF = ptree->Branch("KinFit_ndf",&kf_ndf,"kf_ndf/F");
    Track1_NDF = ptree->Branch("Track1_NDF",&t1_ndf,"t1_ndf/F");
    Track2_NDF = ptree->Branch("Track2_NDF",&t2_ndf,"t2_ndf/F");

    Event_No = ptree->Branch("Event_No",&eventno,"eventno/F");

    FinalChamber8 = ptree->Branch("FinalChamber8",&pfinalChamber,"&pfinalChamber/F");
    FinalChamber9 = ptree->Branch("FinalChamber9",&nfinalChamber,"&nfinalChamber/F");

    TriggerFinalChamber = ptree->Branch("TriggerFinalChamber",&tfinalChamber,"&tfinalChamber/F");

    FinalChamber8_exclusive = ptree->Branch("FinalChamber8_exclusive",&pfinalChamber_exclusive,"&pfinalChamber_exclusive/F");
    FinalChamber9_exclusive = ptree->Branch("FinalChamber9_exclusive",&nfinalChamber_exclusive,"&nfinalChamber_exclusive/F");

    TriggerFinalChamber_exclusive = ptree->Branch("TriggerFinalChamber_exclusive",&tfinalChamber_exclusive,"&tfinalChamber_exclusive/F");


    FinalChamber81to5 = ptree->Branch("FinalChamber81to5",&pfinalChamber1to5,"&pfinalChamber1to5/F");
    FinalChamber91to5 = ptree->Branch("FinalChamber91to5",&nfinalChamber1to5,"&nfinalChamber1to5/F");

    TriggerFinalChamber1to5 = ptree->Branch("TriggerFinalChamber1to5",&tfinalChamber1to5,"&tfinalChamber1to5/F");

    FinalChamber81to5_exclusive = ptree->Branch("FinalChamber81to5_exclusive",&pfinalChamber1to5_exclusive,"&pfinalChamber1to5_exclusive/F");
    FinalChamber91to5_exclusive = ptree->Branch("FinalChamber91to5_exclusive",&nfinalChamber1to5_exclusive,"&nfinalChamber1to5_exclusive/F");

    TriggerFinalChamber1to5_exclusive = ptree->Branch("TriggerFinalChamber1to5_exclusive",&tfinalChamber1to5_exclusive,"&tfinalChamber1to5_exclusive/F");

    Chamber1_diff_8 = ptree->Branch("Chamber1_diff_8",&pdiff1,"pdiff1/F");
    Chamber2_diff_8 = ptree->Branch("Chamber2_diff_8",&pdiff2,"pdiff2/F");
    Chamber3_diff_8 = ptree->Branch("Chamber3_diff_8",&pdiff3,"pdiff3/F");
    Chamber4_diff_8 = ptree->Branch("Chamber4_diff_8",&pdiff4,"pdiff4/F");
    Chamber5_diff_8 = ptree->Branch("Chamber5_diff_8",&pdiff5,"pdiff5/F");
    Chamber6_diff_8 = ptree->Branch("Chamber6_diff_8",&pdiff6,"pdiff6/F");

    Chamber1_diff_9 = ptree->Branch("Chamber1_diff_9",&ndiff1,"ndiff1/F");
    Chamber2_diff_9 = ptree->Branch("Chamber2_diff_9",&ndiff2,"ndiff2/F");
    Chamber3_diff_9 = ptree->Branch("Chamber3_diff_9",&ndiff3,"ndiff3/F");
    Chamber4_diff_9 = ptree->Branch("Chamber4_diff_9",&ndiff4,"ndiff4/F");
    Chamber5_diff_9 = ptree->Branch("Chamber5_diff_9",&ndiff5,"ndiff5/F");
    Chamber6_diff_9 = ptree->Branch("Chamber6_diff_9",&ndiff6,"ndiff6/F");

    Chamber1MatchedHits8 = ptree->Branch("Chamber1MatchedHits8",&fmwpc1m8,"fmwpc1m8/F");
    Chamber2MatchedHits8 = ptree->Branch("Chamber2MatchedHits8",&fmwpc2m8,"fmwpc2m8/F");
    Chamber3MatchedHits8 = ptree->Branch("Chamber3MatchedHits8",&fmwpc3m8,"fmwpc3m8/F");
    Chamber4MatchedHits8 = ptree->Branch("Chamber4MatchedHits8",&fmwpc4m8,"fmwpc4m8/F");
    Chamber5MatchedHits8 = ptree->Branch("Chamber5MatchedHits8",&fmwpc5m8,"fmwpc5m8/F");
    Chamber6MatchedHits8 = ptree->Branch("Chamber6MatchedHits8",&fmwpc6m8,"fmwpc6m8/F");

    Chamber1MatchedHits9 = ptree->Branch("Chamber1MatchedHits9",&fmwpc1m9,"fmwpc1m9/F");
    Chamber2MatchedHits9 = ptree->Branch("Chamber2MatchedHits9",&fmwpc2m9,"fmwpc2m9/F");
    Chamber3MatchedHits9 = ptree->Branch("Chamber3MatchedHits9",&fmwpc3m9,"fmwpc3m9/F");
    Chamber4MatchedHits9 = ptree->Branch("Chamber4MatchedHits9",&fmwpc4m9,"fmwpc4m9/F");
    Chamber5MatchedHits9 = ptree->Branch("Chamber5MatchedHits9",&fmwpc5m9,"fmwpc5m9/F");
    Chamber6MatchedHits9 = ptree->Branch("Chamber6MatchedHits9",&fmwpc6m9,"fmwpc6m9/F");

    Chamber1TotalMatchedHits = ptree->Branch("Chamber1TotalMatchedHits",&tfmwpc1m,"tfmwpc1m/F");
    Chamber2TotalMatchedHits = ptree->Branch("Chamber2TotalMatchedHits",&tfmwpc2m,"tfmwpc2m/F");
    Chamber3TotalMatchedHits = ptree->Branch("Chamber3TotalMatchedHits",&tfmwpc3m,"tfmwpc3m/F");
    Chamber4TotalMatchedHits = ptree->Branch("Chamber4TotalMatchedHits",&tfmwpc4m,"tfmwpc4m/F");
    Chamber5TotalMatchedHits = ptree->Branch("Chamber5TotalMatchedHits",&tfmwpc5m,"tfmwpc5m/F");
    Chamber6TotalMatchedHits = ptree->Branch("Chamber6TotalMatchedHits",&tfmwpc6m,"tfmwpc6m/F");

    NoChamber_matches_8 = ptree->Branch("NoChamber_matches_8",&nFMh8,"nFMh8/F");
    NoChamber_matches_9 = ptree->Branch("NoChamber_matches_9",&nFMh9,"nFMh9/F");

    NoFCALShowersRefined = ptree->Branch("NoFCALShowersRefined",&nFSr,"nFSr/F");
    NoFCALHitsRefined = ptree->Branch("NoFCALHitsRefined",&nFHr,"nFHr/F");

    Thrown_P4X1 = ptree->Branch("Thrown_P4X1",&thrownpx1,"thrownpx1/F");
    Thrown_P4Y1 = ptree->Branch("Thrown_P4Y1",&thrownpy1,"thrownpy1/F");
    Thrown_P4Z1 = ptree->Branch("Thrown_P4Z1",&thrownpz1,"thrownpz1/F");
    Thrown_P4E1 = ptree->Branch("Thrown_P4E1",&thrownpe1,"thrownpe1/F");

    Thrown_P4X2 = ptree->Branch("Thrown_P4X2",&thrownpx2,"thrownpx2/F");
    Thrown_P4Y2 = ptree->Branch("Thrown_P4Y2",&thrownpy2,"thrownpy2/F");
    Thrown_P4Z2 = ptree->Branch("Thrown_P4Z2",&thrownpz2,"thrownpz2/F");
    Thrown_P4E2 = ptree->Branch("Thrown_P4E2",&thrownpe2,"thrownpe2/F");

    FCAL_SHOWER_COUNT8 = ptree->Branch("FCAL_SHOWER_COUNT8",&nFS8r,"nFS8r/F");
    FCAL_SHOWER_COUNT9 = ptree->Branch("FCAL_SHOWER_COUNT9",&nFS9r,"nFS9r/F");

    FCAL_HIT_COUNT8 = ptree->Branch("FCAL_HIT_COUNT8",&nFH8r,"nFH8r/F");
    FCAL_HIT_COUNT9 = ptree->Branch("FCAL_HIT_COUNT9",&nFH9r,"nFH9r/F");

    FCALTime8 = ptree->Branch("FCALTime8",&fcaltime8,"fcaltime8/F");
    FCALTime9 = ptree->Branch("FCALTime9",&fcaltime9,"fcaltime9/F");

    VertexZpos8 = ptree->Branch("VertexZpos8",&vtxpos8,"vtxpos8/F");
    VertexZpos9 = ptree->Branch("VertexZpos9",&vtxpos9,"vtxpos9/F");

    VertexT8 = ptree->Branch("VertexT8",&vtxt8,"vtxt8/F");
    VertexT9 = ptree->Branch("VertexT9",&vtxt9,"vtxt9/F");

    TrackZFCAL8 = ptree->Branch("TrackZFCAL8",&trackatfcalz8,"trackatfcalz8/F");
    TrackZFCAL9 = ptree->Branch("TrackZFCAL9",&trackatfcalz9,"trackatfcalz9/F");

    TrackTFCAL8 = ptree->Branch("TrackTFCAL8",&trackatfcalt8,"trackatfcalt8/F");
    TrackTFCAL9 = ptree->Branch("TrackTFCAL9",&trackatfcalt9,"trackatfcalt9/F");

    CTOF_Bar1 = ptree->Branch("CTOF_Bar1",&ctofbar1,"ctofbar1/F");
    CTOF_Bar2 = ptree->Branch("CTOF_Bar2",&ctofbar2,"ctofbar2/F");
    CTOF_Bar3 = ptree->Branch("CTOF_Bar3",&ctofbar3,"ctofbar3/F");
    CTOF_Bar4 = ptree->Branch("CTOF_Bar4",&ctofbar4,"ctofbar4/F");

    Chamber1_Extrap_Exists_Plus = ptree->Branch("Chamber1_Extrap_Exists_Plus",&c1extrap_plus,"c1extrap_plus/F");
    Chamber2_Extrap_Exists_Plus = ptree->Branch("Chamber2_Extrap_Exists_Plus",&c2extrap_plus,"c2extrap_plus/F");
    Chamber3_Extrap_Exists_Plus = ptree->Branch("Chamber3_Extrap_Exists_Plus",&c3extrap_plus,"c3extrap_plus/F");
    Chamber4_Extrap_Exists_Plus = ptree->Branch("Chamber4_Extrap_Exists_Plus",&c4extrap_plus,"c4extrap_plus/F");
    Chamber5_Extrap_Exists_Plus = ptree->Branch("Chamber5_Extrap_Exists_Plus",&c5extrap_plus,"c5extrap_plus/F");
    Chamber6_Extrap_Exists_Plus = ptree->Branch("Chamber6_Extrap_Exists_Plus",&c6extrap_plus,"c6extrap_plus/F");

    Chamber1_Extrap_Exists_Minus = ptree->Branch("Chamber1_Extrap_Exists_Minus",&c1extrap_minus,"c1extrap_minus/F");
    Chamber2_Extrap_Exists_Minus = ptree->Branch("Chamber2_Extrap_Exists_Minus",&c2extrap_minus,"c2extrap_minus/F");
    Chamber3_Extrap_Exists_Minus = ptree->Branch("Chamber3_Extrap_Exists_Minus",&c3extrap_minus,"c3extrap_minus/F");
    Chamber4_Extrap_Exists_Minus = ptree->Branch("Chamber4_Extrap_Exists_Minus",&c4extrap_minus,"c4extrap_minus/F");
    Chamber5_Extrap_Exists_Minus = ptree->Branch("Chamber5_Extrap_Exists_Minus",&c5extrap_minus,"c5extrap_minus/F");
    Chamber6_Extrap_Exists_Minus = ptree->Branch("Chamber6_Extrap_Exists_Minus",&c6extrap_minus,"c6extrap_minus/F");

    Chamber6_Fiducial_Plus = ptree->Branch("Chamber6_Fiducial_Plus",&c6fid_plus,"c6fid_plus/F");
    Chamber6_Fiducial_Minus = ptree->Branch("Chamber6_Fiducial_Minus",&c6fid_minus,"c6fid_minus/F");

    BShowersExist = ptree->Branch("BShowersExist",&bshowerexist,"bshowerexist/F");
    BHitsExist = ptree->Branch("BHitsExist",&bhitexist,"bhitexist/F");
    BCALPlusEnergy = ptree->Branch("BCALPlusEnergy",&plusenergy_bcal,"plusenergy_bcal/F");
    BCALMinusEnergy = ptree->Branch("BCALMinusEnergy",&minusenergy_bcal,"minusenergy_bcal/F");
    
}

//------------------
// BeginRun
//------------------
void JEventProcessor_cpp_ana_sim_tree::BeginRun(const std::shared_ptr<const JEvent> &event)
{
    // This is called whenever the run number changes
    auto runnumber = event->GetRunNumber();
    auto app = event->GetJApplication();
    
    auto geo_manager = app->GetService<DGeometryManager>();
    bfield = geo_manager->GetBfield(runnumber);
    
    auto geom = geo_manager->GetDGeometry(runnumber);
    geom->GetFCALZ(fcalfrontfaceZ);

    geom->GetFCALPosition(m_FCALdX,m_FCALdY,m_FCALfront);

    vector<double>  tof_face;
    geom->Get("//section/composition/posXYZ[@volume='ForwardTOF']/@X_Y_Z",tof_face);
    m_TOFdX = tof_face[0]; m_TOFdY = tof_face[1]; m_TOFfront = tof_face[2];
}

//------------------
// Process
//------------------
void JEventProcessor_cpp_ana_sim_tree::Process(const std::shared_ptr<const JEvent> &event)
{
    // This is called for every event. Use of common resources like writing
    // to a file or filling a histogram should be mutex protected. Using
    // event->Get(...) to get reconstructed objects (and thereby activating the
    // reconstruction algorithm) should be done outside of any mutex lock
    // since multiple threads may call this method at the same time.
    // Here's an example:
    //
    // vector<const MyDataClass*> mydataclasses;
    // event->Get(mydataclasses);
    //
    // If you have lockService initialized in Init() then you can acquire locks like this
    // lockService->RootFillLock(this);
    //  ... fill historgrams or trees ...
    // lockService->RootFillUnLock(this);

    vector<const DCPPSelect*> pimu_samples;
    event->Get(pimu_samples);

    if(pimu_samples.empty()) return;
    
    vector<const DChargedTrack*>tracks;
    event->Get(tracks);
    if (tracks.size()!=2) return;

    vector<const DEventRFBunch*> rf;
    event->Get(rf);

    if(rf.size() == 0) return;

    double locRFTime = rf[0]->dTime;
    if(isnan(locRFTime)) return;

    vector<const DL1MCTrigger*> cpp_trigs;
    event->Get(cpp_trigs,"CPP");
    if(cpp_trigs.empty()) return;
    const uint32_t TOF_mask = (1u << (2-1));
    if(!(cpp_trigs[0]->trig_mask & TOF_mask)) return;

    vector<const DMCThrown*> mcThrownTracks;
    event->Get(mcThrownTracks);


    for(size_t i = 0; i < mcThrownTracks.size();i++){
        const DMCThrown *thrown = mcThrownTracks[i];
        if(thrown->PID() == 8){
            thrownpx1 = thrown->px();
            thrownpy1 = thrown->py();
            thrownpz1 = thrown->pz();

            DVector3 thrown_piplus(thrownpx1,thrownpy1,thrownpz1);
            thrownpe1 = calculateTrackEnergy(thrown_piplus,mpic);

        }
        if(thrown->PID() == 9){
            thrownpx2 = thrown->px();
            thrownpy2 = thrown->py();
            thrownpz2 = thrown->pz();

            DVector3 thrown_piminus(thrownpx2,thrownpy2,thrownpz2);
            thrownpe2 = calculateTrackEnergy(thrown_piminus,mpic);
        }
    }


    for(size_t i = 0; i<pimu_samples.size(); i++){
        const DCPPSelect *pimu = pimu_samples[i];
        
        TLorentzVector rec_piplus, rec_piminus; 
        rec_piplus.SetVectM(pimu->piplus3mom,mpic);
        rec_piminus.SetVectM(pimu->piminus3mom,mpic);

        DVector3 piplus3_mom = rec_piplus.Vect();
        DVector3 piminus3_mom = rec_piminus.Vect();

        if(piplus3_mom.Mag() > 10.0 || piplus3_mom.Mag() < 1.0) continue;
        if(piminus3_mom.Mag() > 10.0 || piminus3_mom.Mag() < 1.0) continue;

        if(pimu->IS_TrackDOCAGood == false) continue;

        double piplus_e = calculateTrackEnergy(piplus3_mom,mpic);
        double piminus_e = calculateTrackEnergy(piminus3_mom,mpic); 
        //if(piplus_e + piminus_e < 4.0) continue;

        if(!(pimu->IS_PlusTrackInTOF && pimu->IS_MinusTrackInTOF)) continue;

        lockService->RootFillLock(this);

         eventno = event->GetEventNumber();

        wght = pimu->beam_weight;
        bE = pimu->beam_energy;
            
        mm_chi = pimu->kinfit_chisq;
        kf_ndf = pimu->kinfit_ndf;

        fcale8 = pimu->fcal_energy_piplus;
        fcale9 = pimu->fcal_energy_piminus;
        fcalep8 = pimu->fcal_eoverp_piplus;
        fcalep9 = pimu->fcal_eoverp_piminus;
        fcaldoca8 = pimu->fcal_doca_piplus;
        fcaldoca9 = pimu->fcal_doca_piminus;
        fcale1e98 = pimu->fcal_e1e9_piplus;
        fcale1e99 = pimu->fcal_e1e9_piminus;
        fcale9e258 = pimu->fcal_e9e25_piplus;
        fcale9e259 = pimu->fcal_e9e25_piminus;
        
        fcalblocksn8 = pimu->fcal_nblocks_piplus;
        fcalblocksn9 = pimu->fcal_nblocks_piminus;
        
        fcale9e258 = pimu->fcal_e9e25_piplus;
        fcale9e259 = pimu->fcal_e9e25_piminus;


        fmwpc1m8 = pimu->fmwpc1n_piplus;
        fmwpc1m9 = pimu->fmwpc1n_piminus;

        fmwpc2m8 = pimu->fmwpc2n_piplus;
        fmwpc2m9 = pimu->fmwpc2n_piminus;

        fmwpc3m8 = pimu->fmwpc3n_piplus;
        fmwpc3m9 = pimu->fmwpc3n_piminus;

        fmwpc4m8 = pimu->fmwpc4n_piplus;
        fmwpc4m9 = pimu->fmwpc4n_piminus;

        fmwpc5m8 = pimu->fmwpc5n_piplus;
        fmwpc5m9 = pimu->fmwpc5n_piminus;

        fmwpc6m8 = pimu->fmwpc6n_piplus;
        fmwpc6m9 = pimu->fmwpc6n_piminus;

        p1x = rec_piplus.X();
        p1y = rec_piplus.Y();
        p1z = rec_piplus.Z();
        p1e = rec_piplus.E();

        p2x = rec_piminus.X();
        p2y = rec_piminus.Y();
        p2z = rec_piminus.Z();
        p2e = rec_piminus.E();

        p1x_kf = pimu->piplus3mom_kf.X();
        p1y_kf = pimu->piplus3mom_kf.Y();
        p1z_kf = pimu->piplus3mom_kf.Z();

        p2x_kf = pimu->piminus3mom_kf.X();
        p2y_kf = pimu->piminus3mom_kf.Y();
        p2z_kf = pimu->piminus3mom_kf.Z();

        ctofbar1 = pimu->ctof_bar1;
        ctofbar2 = pimu->ctof_bar2;
        ctofbar3 = pimu->ctof_bar3;
        ctofbar4 = pimu->ctof_bar4;

        c1extrap_plus = pimu->IS_Chamber1ExtrapReal_plus;
        c2extrap_plus = pimu->IS_Chamber2ExtrapReal_plus;
        c3extrap_plus = pimu->IS_Chamber3ExtrapReal_plus;
        c4extrap_plus = pimu->IS_Chamber4ExtrapReal_plus;
        c5extrap_plus = pimu->IS_Chamber5ExtrapReal_plus;
        c6extrap_plus = pimu->IS_Chamber6ExtrapReal_plus;

        c1extrap_minus = pimu->IS_Chamber1ExtrapReal_minus;
        c2extrap_minus = pimu->IS_Chamber2ExtrapReal_minus;
        c3extrap_minus = pimu->IS_Chamber3ExtrapReal_minus;
        c4extrap_minus = pimu->IS_Chamber4ExtrapReal_minus;
        c5extrap_minus = pimu->IS_Chamber5ExtrapReal_minus;
        c6extrap_minus = pimu->IS_Chamber6ExtrapReal_minus;

        c6fid_plus = pimu->IS_PlusTrackINChamber6;
        c6fid_minus = pimu->IS_MinusTrackINChamber6;

        bshowerexist = pimu->BCALShowersExist;
        bhitexist = pimu->BCALHitsExist;

        plusenergy_bcal = pimu->PlusTrack_EnergyInBCAL;
        minusenergy_bcal = pimu->MinusTrack_EnergyInBCAL;
        
        lockService->RootFillUnLock(this);

        double plus_mlp_score, minus_mlp_score;
        plus_mlp_score = getEPIClassifierPlus(fcalep8,fcaldoca8,fcale9e258);
        minus_mlp_score = getEPIClassifierMinus(fcalep9,fcaldoca9,fcale9e259);

        lockService->RootFillLock(this);
        epimlp_plus = plus_mlp_score;
        epimlp_minus = minus_mlp_score;
        lockService->RootFillUnLock(this);

        lockService->RootFillLock(this);
        ptree->Fill();
        lockService->RootFillUnLock(this);

    }

}

//------------------
// EndRun
//------------------
void JEventProcessor_cpp_ana_sim_tree::EndRun()
{
    // This is called whenever the run number changes, before it is
    // changed to give you a chance to clean up before processing
    // events from the next run number.
}

//------------------
// Finish
//------------------
void JEventProcessor_cpp_ana_sim_tree::Finish()
{
    // Called before program exit after event processing is finished.

    lockService->RootWriteLock();
    ofile->Write();
    ofile->Close();
    lockService->RootUnLock();
}

double JEventProcessor_cpp_ana_sim_tree::calculateTrackEnergy(DVector3 ptrack_mom, double pmass){
  double px = ptrack_mom.X();
  double py = ptrack_mom.Y();
  double pz = ptrack_mom.Z();

  double energy = sqrt(px*px+py*py+pz*pz + pmass*pmass);

  return energy;

}


double JEventProcessor_cpp_ana_sim_tree::getEPIClassifierMinus(double EoverP_minus, double FCAL_DOCA_minus, double FCAL_E9E25_minus){
  vector<double> mvaInputsMinus(3);
  mvaInputsMinus[0] = EoverP_minus;
  mvaInputsMinus[1] = FCAL_DOCA_minus;
  mvaInputsMinus[2] = FCAL_E9E25_minus;
  double epiMVAminus = dEPIClassifierMinus->GetMvaValue( mvaInputsMinus );
  return epiMVAminus;
}


double JEventProcessor_cpp_ana_sim_tree::getEPIClassifierPlus(double EoverP_plus, double FCAL_DOCA_plus, double FCAL_E9E25_plus){
  vector<double> mvaInputsPlus(3);
  mvaInputsPlus[0] = EoverP_plus;
  mvaInputsPlus[1] = FCAL_DOCA_plus;
  mvaInputsPlus[2] = FCAL_E9E25_plus;
  double epiMVAplus = dEPIClassifierPlus->GetMvaValue( mvaInputsPlus );
  return epiMVAplus;
}

