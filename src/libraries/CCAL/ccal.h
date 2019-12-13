#ifndef _CCAL_H_
#define _CCAL_H_



#define MCOL 12
#define MROW 12

#define MADCGAM 50 // maximum allowed showers
#define MAX_CC 60
		
#define xsize 2.09
#define ysize 2.09

#define MIN_ENERGY 0.

#define COUNT_ZERO 1
#define TEST_P 1



enum ClusterType_t{ 
	SinglePeak=0, 
	MultiPeak 
};

enum PeakType_t{ 
	OneGamma=0, 
	TwoGammas 
};



typedef struct {

  int  id[MAX_CC];    /* ID of ith block, where i runs from 0 to 8 */
  double E[MAX_CC];   /* Energy of ith block */
  double x[MAX_CC];   /* Center of ith block x-coord */
  double y[MAX_CC];   /* Center of ith block y-coord */
  double t[MAX_CC];

} cluster_t;
		
		
typedef struct {

  int type;       /* cluster types: 0,1,2,10,11,12 */
  int nhits;      /* Number of hits in cluster */
  int id;         /* Indicates multiple gammas in same peak */
  int idmax;      /* Cluster's central cell's ID */
  
  double E;       /* Cluster's energy [GeV] */
  double Esum;    /* Cluster energy calculated as sum of blocks' energies [GeV] */
  double x;       /* Cluster's x-position [cm] */
  double y;       /* Cluster's y-position [cm] */
  double x1;      /* Cluster's x1-position [cm] */
  double y1;      /* Cluster's y1-position [cm] */
  double chi2;    /* Cluster's profile fit to single shower profile */
  double time;    /* Cluster's time [ns] */
  double sigma_E; 
  double emax;
  
} ccalcluster_t;
		
		
typedef struct {
  
  int type;
  int dime;
  int id;
		
  double chi2;
  double energy;
  double x;
  double y;
  double xc;
  double yc;
		  
  int icl_in[MAX_CC];
  int icl_en[MAX_CC];
  
} gamma_t;



#endif
