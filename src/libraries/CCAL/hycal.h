#ifndef _HYCAL_H_
#define _HYCAL_H_

#define MSECT 5
#define MCOL 12
#define MROW 12

// #define  READ_MC_FILE

#ifdef  READ_MC_FILE
#define ADMIX_CLOCK         // Whether to admix clock-ev. pattern to mc
#define HEGEN(N) read_mcfile_com_.hegen[N-1]
#endif

#define CRYSTAL_BLOCKS 144 // 12x12 array (2x2 hole in the center)
#define T_BLOCKS 144 

#define MAX_HHITS 144 // For Hycal
#define MAX_CLUSTERS 60 // 1728/2 ?
#define MAX_CC 60 // Maximum Cluster Cells per cluster

#define nint_phot_cell  5
#define ncoef_phot_cell 3
#define dcorr_phot_cell 16

extern int status_global[MSECT][MCOL][MROW];

extern float Nonlin_en1[T_BLOCKS];          //  Hycal nonlin: term1
extern float Nonlin_en2[T_BLOCKS];          //  Hycal nonlin: term2
extern int event_num;
extern int run_num;
extern int clock_num;
extern float beam_energy;
extern float tac_energy;
extern float beam_time;
extern int n_h_clusters;
extern int n_h_hits;
extern int tagger_id;
extern int tagger_time;
extern int iftac;

typedef struct {
  int id;                 // ID of block
  float x;                // Center of block x-coord
  float y;                // Center of block y-coord
//  int cc;                 // Number of ClusterMembers including central member
//  int member[MAX_CC];     // Array of ints to hold the ClusterMembers
  int sector;             // 0 for W, 1 - 4 for Glass (clockwise starting at noon)
  int row;                // row number within sector
  int col;                // column number within sector
} blockINFO_t;

extern blockINFO_t blockINFO[T_BLOCKS];

#define CRYS_ROWS 12
#define CRYS_SIZE_X 2.077   // real X-size of crystal
#define CRYS_SIZE_Y 2.075   // real Y-size of crystal

typedef struct {
  int  id[MAX_CC];   // ID of ith block, where i runs from 0 to 8
  float E[MAX_CC];   // Energy of ith block
  float x[MAX_CC];   // Center of ith block x-coord
  float y[MAX_CC];   // Center of ith block y-coord
  float t[MAX_CC];
} cluster_t;
/*----- start of ccalcluster bank -----*/
/* ccalcluster: CCAL cluster bank */

typedef struct {
  int type; /* cluster types: 0,1,2,3,4;-1 */
  int nhits; /* Number of hits in cluster */
  int id; /* Cluster's central cell's ID */
  float E; /* Cluster's energy (GeV) */
  float E1; /* Cluster's energy (GeV) */
  float E2; /* Cluster's energy (GeV) */
  float time; /* Cluster's time (ns) */
  float x; /* Cluster's x-position (cm) */
  float y; /* Cluster's y-position (cm) */
  float chi2; /* Cluster's profile fit to single shower profile */
  float x1; /* Cluster's x1-position (cm) */
  float y1; /* Cluster's y1-position (cm) */
  float sigma_E;
  float emax;
  int status;
} ccalcluster_t;

typedef struct {
  int  id;   // ID of ADC
  float e;   // Energy of ADC
} ccalhit_t;

extern ccalhit_t ccalhit[T_BLOCKS];
extern cluster_t cluster_storage[MAX_CLUSTERS];
extern ccalcluster_t ccalcluster[MAX_CLUSTERS];

float energy_correct(float c_energy, int central_id);
float shower_depth(float energy);

extern "C"  void main_island_();

extern "C"  void init_island_(char filename[1000], int *name_length);
//extern "C"  void init_island_(float new_acell[2][501][501], float new_ad2c[2][501][501]);

extern "C" {extern struct {int mcrun, mcevent; float hegen[64];
              int nout, iout[MAX_HHITS], aout[MAX_HHITS];} read_mcfile_com_;}
extern "C" {extern struct {int mcread_flag, mc_eof_flag, nreadmc;} mcread_stat_com_;}

#define ECH(M,N) ech_common_.ech[N-1][M-1]
extern "C" {extern struct {int ech[MROW][MCOL];} ech_common_;}
//#define TIME(M,N) tch_common_.time[N-1][M-1]
//extern "C" {extern struct {int time[MROW][MCOL];} tch_common_;}
#define STAT_CH(M,N) stat_ch_common_.stat_ch[N-1][M-1]
extern "C" {extern struct {int stat_ch[MROW][MCOL];} stat_ch_common_;}

#define ICL_INDEX(M,N) icl_common_.icl_index[N][M]
#define ICL_IENER(M,N) icl_common_.icl_iener[N][M]
extern "C" {extern struct {int icl_index[MAX_CC][200], icl_iener[MAX_CC][200];} icl_common_;}

#define HEGEN(N) read_mcfile_com_.hegen[N-1]

#define SET_XSIZE set_common_.xsize
#define SET_YSIZE set_common_.ysize
#define SET_EMIN  set_common_.mine
#define SET_EMAX  set_common_.maxe
#define SET_HMIN  set_common_.min_dime
#define SET_MINM  set_common_.minm
#define NCOL      set_common_.ncol
#define NROW      set_common_.nrow
#define ZHYCAL    set_common_.zhycal
#define ISECT     set_common_.isect

extern "C" {extern struct {float xsize, ysize, mine, maxe; int min_dime; float minm;
                              int ncol, nrow; float zhycal; int isect;} set_common_;}

extern "C" {extern struct {int nadcgam;
            union {int iadcgam[50][12]; float fadcgam[50][12];} u;} adcgam_cbk_;}

extern "C" {extern struct {float fa[100];} hbk_common_;}
#define FA(N) hbk_common_.fa[N-1]

#endif
