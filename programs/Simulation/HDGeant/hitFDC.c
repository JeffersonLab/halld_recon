/*
 * hitFDC - registers hits for forward drift chambers
 *
 *        This is a part of the hits package for the
 *        HDGeant simulation program for Hall D.
 *
 *        version 1.0         -Richard Jones July 16, 2001
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <hddm_s.h>
#include <geant3.h>
#include <bintree.h>

const float Tau[] = {0,-45,0,45,15,60,105,-105,-60,-15};

// Drift speed 2.2cm/us is appropriate for a 90/10 Argon/Methane mixture
#define DRIFT_SPEED           .0055
#define WIRE_DEAD_ZONE_RADIUS 3.5
#define STRIP_DEAD_ZONE_RADIUS 5.0
#define ACTIVE_AREA_OUTER_RADIUS 48.5
#define ANODE_CATHODE_SPACING 0.5
#define TWO_HIT_RESOL         250.
#define WIRES_PER_PLANE       96
#define WIRE_SPACING          1.116
#define U_OF_WIRE_ZERO        (-((WIRES_PER_PLANE-1.)*WIRE_SPACING)/2)
#define STRIPS_PER_PLANE      216
#define STRIP_SPACING         0.5
#define U_OF_STRIP_ZERO		  (-((STRIPS_PER_PLANE-1.)*STRIP_SPACING)/2)
#define STRIP_GAP             0.1
#define MAX_HITS             100
#define K2                  1.15
#define STRIP_NODES           3
#define THRESH_KEV           1.
#define THRESH_STRIPS        5.   /* pC */
#define ELECTRON_CHARGE 1.6022e-4 /* fC */
/* The folowing are for interpreting grid of Lorentz deflection data */
#define PACKAGE_Z_POINTS 7
#define LORENTZ_X_POINTS 21
#define LORENTZ_Z_POINTS 4*PACKAGE_Z_POINTS

binTree_t* forwardDCTree = 0;
static int stripCount = 0;
static int wireCount = 0;
static int pointCount = 0;
static int initialized=0;

// Variables for implementing lorentz effect (deflections of avalanche position
// due to the magnetic field).
float lorentz_x[LORENTZ_X_POINTS];
float lorentz_z[LORENTZ_Z_POINTS];
float lorentz_nx[LORENTZ_X_POINTS][LORENTZ_Z_POINTS];
float lorentz_nz[LORENTZ_X_POINTS][LORENTZ_Z_POINTS];

void gpoiss_(float*,int*,const int*); // avoid solaris compiler warnings

// Locate a position in array xx given x
void locate(float *xx,int n,float x,int *j){
  int ju,jm,jl;
  int ascnd;
  
  jl=-1;
  ju=n;
  ascnd=(xx[n-1]>=xx[0]);
  while(ju-jl>1){
    jm=(ju+jl)>>1;
    if (x>=xx[jm]==ascnd)
      jl=jm;
    else
      ju=jm;
  }
  if (x==xx[0]) *j=0;
  else if (x==xx[n-1]) *j=n-2;
  else *j=jl; 
}

// Polynomial interpolation on a grid.
// Adapted from Numerical Recipes in C (2nd Edition), pp. 121-122.
void polint(float *xa, float *ya,int n,float x, float *y,float *dy){
  int i,m,ns=0;
  float den,dif,dift,ho,hp,w;

  float *c=(float *)calloc(n,sizeof(float));
  float *d=(float *)calloc(n,sizeof(float));

  dif=fabs(x-xa[0]);
  for (i=0;i<n;i++){
    if ((dift=fabs(x-xa[i]))<dif){
      ns=i;
      dif=dift;
    }
    c[i]=ya[i];
    d[i]=ya[i];
  }
  *y=ya[ns--];

  for (m=1;m<n;m++){
    for (i=1;i<=n-m;i++){
      ho=xa[i-1]-x;
      hp=xa[i+m-1]-x;
      w=c[i+1-1]-d[i-1];
      if ((den=ho-hp)==0.0) return;
      
      den=w/den;
      d[i-1]=hp*den;
      c[i-1]=ho*den;
      
    }
    
    *y+=(*dy=(2*ns<(n-m) ?c[ns+1]:d[ns--]));
  }
  free(c);
  free(d);
}


/* register hits during tracking (from gustep) */

void hitForwardDC (float xin[4], float xout[4],
                   float pin[5], float pout[5], float dEsum,
                   int track, int stack, int history)
{
  float x[3], t;
  float dx[3], dr;
  float dEdx;
  float xlocal[3];
  float xinlocal[3];
  float xoutlocal[3];
  float dradius;
  float alpha;
  int i,j;

  /* Get chamber information */
  int module = getmodule_();
  int layer = getlayer_();
  int chamber = (module*10)+layer;
  int wire1,wire2;
  int wire,dwire;

  // Initialize arrays of deflection data from the Lorentz effect
  if (!initialized){
    FILE *fp=fopen("fdc_deflections.dat","r");
    if (fp){
      char dummy[80];
      float fdummy;
      size_t len=0;
      getline(dummy,&len,fp); // Skip header line
   
      // Read deflection plane data
      for (i=0;i<LORENTZ_X_POINTS;i++){
	for (j=0;j<LORENTZ_Z_POINTS;j++){
	  fscanf(fp,"%f %f %f %f %f %f\n",&lorentz_x[i],&lorentz_z[j],
		 &fdummy, &fdummy,
		 &lorentz_nx[i][j],&lorentz_nz[i][j]);
	}
      }
      initialized=1;
    }
    else{
      fprintf(stderr,"HDGeant error in hitFDC.c: ");
      fprintf(stderr,"no deflection parameterization file found.\n");
      exit(1);
    }
  }  

  transformCoord(xin,"global",xinlocal,"local");
  wire1 = ceil((xinlocal[0] - U_OF_WIRE_ZERO)/WIRE_SPACING +0.5);
  transformCoord(xout,"global",xoutlocal,"local");
  wire2 = ceil((xoutlocal[0] - U_OF_WIRE_ZERO)/WIRE_SPACING +0.5);
  dwire = (wire1 < wire2)? 1 : -1;
  alpha = atan2(xoutlocal[0]-xinlocal[0],xoutlocal[2]-xinlocal[2]);
  xlocal[0] = (xinlocal[0] + xoutlocal[0])/2;
  xlocal[1] = (xinlocal[1] + xoutlocal[1])/2;
  xlocal[2] = (xinlocal[2] + xoutlocal[2])/2;
  wire = ceil((xlocal[0] - U_OF_WIRE_ZERO)/WIRE_SPACING +0.5);
  x[0] = (xin[0] + xout[0])/2;
  x[1] = (xin[1] + xout[1])/2;
  x[2] = (xin[2] + xout[2])/2;
  t = (xin[3] + xout[3])/2 * 1e9;
  dx[0] = xin[0] - xout[0];
  dx[1] = xin[1] - xout[1];
  dx[2] = xin[2] - xout[2];
  dr = sqrt(dx[0]*dx[0] + dx[1]*dx[1] + dx[2]*dx[2]);
  if (dr > 1e-3)
  {
    dEdx = dEsum/dr;
  }
  else
  {
    dEdx = 0;
  }

  /* Make a fuzzy boundary around the forward dead region 
   * by killing any track segment whose midpoint is within the boundary */

  if (sqrt(xlocal[0]*xlocal[0]+xlocal[1]*xlocal[1]) < WIRE_DEAD_ZONE_RADIUS)
  {
    return;
  }
  
  /* post the hit to the truth tree */
 
  if (history == 0)
  {
    int mark = (1<<16) + (chamber<<20) + pointCount;
    void** twig = getTwig(&forwardDCTree, mark);
    if (*twig == 0)
    {
      s_ForwardDC_t* fdc = *twig = make_s_ForwardDC();
      s_FdcChambers_t* chambers = make_s_FdcChambers(1);
      s_FdcTruthPoints_t* points = make_s_FdcTruthPoints(1);
      float xwire = U_OF_WIRE_ZERO + (wire-1)*WIRE_SPACING;
      float u[2];
      u[0] = xinlocal[2];
      u[1] = xinlocal[0]-xwire;
      dradius = fabs(u[1]*cos(alpha)-u[0]*sin(alpha));
      points->mult = 1;
      points->in[0].primary = (stack == 0);
      points->in[0].track = track;
      points->in[0].x = x[0];
      points->in[0].y = x[1];
      points->in[0].z = x[2];
      points->in[0].dradius = dradius;
      points->in[0].dEdx = dEdx;
      chambers->mult = 1;
      chambers->in[0].module = module;
      chambers->in[0].layer = layer;
      chambers->in[0].fdcTruthPoints = points;
      fdc->fdcChambers = chambers;
      pointCount++;
    }
  }

  /* post the hit to the hits tree, mark cell as hit */

  if (dEsum > 0)
  {
    int nhit;
    s_FdcAnodeHits_t* ahits;    
    s_FdcCathodeHits_t* chits;    
    float tdrift;
    for (wire=wire1; wire-dwire != wire2; wire+=dwire)
    {
      int valid_hit=1;
      float dE;
      float u[2];
      float x0[3],x1[3];
      float avalanche_y;
      float xwire = U_OF_WIRE_ZERO + (wire-1)*WIRE_SPACING;
      x0[0] = xwire-0.5*dwire*WIRE_SPACING;
      x0[1] = xinlocal[1] + (x0[0]-xinlocal[0]+1e-20)*
             (xoutlocal[1]-xinlocal[1])/(xoutlocal[0]-xinlocal[0]+1e-20);
      x0[2] = xinlocal[2] + (x0[0]-xinlocal[0]+1e-20)*
             (xoutlocal[2]-xinlocal[2])/(xoutlocal[0]-xinlocal[0]+1e-20);
      if (fabs(x0[2]-xoutlocal[2]) > fabs(xinlocal[2]-xoutlocal[2]))
      {
        x0[0] = xinlocal[0];
        x0[1] = xinlocal[1];
        x0[2] = xinlocal[2];
      }
      x1[0] = xwire+0.5*dwire*WIRE_SPACING;
      x1[1] = xinlocal[1] + (x1[0]-xinlocal[0]+1e-20)*
             (xoutlocal[1]-xinlocal[1])/(xoutlocal[0]-xinlocal[0]+1e-20);
      x1[2] = xinlocal[2] + (x1[0]-xinlocal[0]+1e-20)*
             (xoutlocal[2]-xinlocal[2])/(xoutlocal[0]-xinlocal[0]+1e-20);
      if (fabs(x1[2]-xinlocal[2]) > fabs(xoutlocal[2]-xinlocal[2]))
      {
        x1[0] = xoutlocal[0];
        x1[1] = xoutlocal[1];
        x1[2] = xoutlocal[2];
      }
      u[0] = xinlocal[2];
      u[1] = xinlocal[0]-xwire;
      dradius = fabs(u[1]*cos(alpha)-u[0]*sin(alpha));
      tdrift = t + dradius/DRIFT_SPEED;
      dE = dEsum*(x1[2]-x0[2])/(xoutlocal[2]-xinlocal[2]);

       /* Compute Lorentz deflection of avalanche position */
      /* Checks to see if deflection would push avalanche out of active area */
      if (dE > 0){
	float tanr,tanz,r;
	float dist_to_wire=xlocal[0]-xwire;
	float phi=atan2(x0[1]+x1[1],x0[0]+x1[0]);
	float ytemp[LORENTZ_X_POINTS],ytemp2[LORENTZ_X_POINTS],dy;
	int imin,imax,ind,ind2;
	
	avalanche_y = (x0[1]+x1[1])/2;
	r=sqrt((x0[0]+x1[0])*(x0[0]+x1[0])+(x0[1]+x1[1])*(x0[1]+x1[1]))/2.;

	// Locate positions in x and z arrays given r and z=x[2]
	locate(lorentz_x,LORENTZ_X_POINTS,r,&ind);
	locate(lorentz_z,25,x[2],&ind2);
	
	// First do interpolation in z direction 
	imin=PACKAGE_Z_POINTS*(ind2/PACKAGE_Z_POINTS); // Integer division...
	for (j=0;j<LORENTZ_X_POINTS;j++){
	  polint(&lorentz_z[imin],&lorentz_nx[j][imin],PACKAGE_Z_POINTS,x[2],
		 &ytemp[j],&dy);
	  polint(&lorentz_z[imin],&lorentz_nz[j][imin],PACKAGE_Z_POINTS,x[2],
		 &ytemp2[j],&dy);
	}
	// Then do final interpolation in x direction 
	imin=(ind>0)?(ind-1):0;
	imax=(ind<LORENTZ_X_POINTS-2)?(ind+2):(LORENTZ_X_POINTS-1);
	polint(&lorentz_x[imin],ytemp,imax-imin+1,r,&tanr,&dy);
	polint(&lorentz_x[imin],ytemp2,imax-imin+1,r,&tanz,&dy);

	// Correct avalanche position with deflection along wire	
	avalanche_y+=-tanz*dist_to_wire*sin(alpha)*cos(phi)
	  +tanr*dist_to_wire*cos(alpha);
	
	/* If the Lorentz effect would deflect the avalanche out of the active 
	   region, mark as invalid hit */
	r=sqrt(avalanche_y*avalanche_y+xwire*xwire);
	if (r>ACTIVE_AREA_OUTER_RADIUS || r<WIRE_DEAD_ZONE_RADIUS){
	  valid_hit=0;
	}
      }

    /* first record the anode wire hit */

      if (dE > 0 && valid_hit)
      {
        int mark = (chamber<<20) + (2<<10) + wire;
        void** twig = getTwig(&forwardDCTree, mark);

        if (*twig == 0)
        {
          s_ForwardDC_t* fdc = *twig = make_s_ForwardDC();
          s_FdcChambers_t* chambers = make_s_FdcChambers(1);
          s_FdcAnodeWires_t* wires = make_s_FdcAnodeWires(1);
          wires->mult = 1;
          wires->in[0].wire = wire;
          wires->in[0].fdcAnodeHits = ahits = make_s_FdcAnodeHits(MAX_HITS);
          chambers->mult = 1;
          chambers->in[0].module = module;
          chambers->in[0].layer = layer;
          chambers->in[0].fdcAnodeWires = wires;
          fdc->fdcChambers = chambers;
          wireCount++;          
        }
        else
        {
           s_ForwardDC_t* fdc = *twig;
           ahits = fdc->fdcChambers->in[0].fdcAnodeWires->in[0].fdcAnodeHits;
        }

        for (nhit = 0; nhit < ahits->mult; nhit++)
        {
          if (fabs(ahits->in[nhit].t - tdrift) < TWO_HIT_RESOL)
          {
            break;
          }
        }
        if (nhit < ahits->mult)                 /* merge with former hit */
        {
          ahits->in[nhit].t = 
              (ahits->in[nhit].t * ahits->in[nhit].dE + tdrift * dE)
              / (ahits->in[nhit].dE += dE);
        }
        else if (nhit < MAX_HITS)              /* create new hit */
        {
          ahits->in[nhit].t = tdrift;
          ahits->in[nhit].dE = dE;
          ahits->mult++;
        }
        else
        {
          fprintf(stderr,"HDGeant error in hitForwardDC: ");
          fprintf(stderr,"max hit count %d exceeded, truncating!\n",MAX_HITS);
        }
      }

    /* then generate hits in the two surrounding cathode planes */

      if (dE > 0 && valid_hit)
      {
        float avalanche_x = xwire;
  
        /* Mock-up of cathode strip charge distribution */
  
        int plane, node;
        for (plane=1; plane<4; plane+=2)
        {
          float theta = (plane == 1)? -M_PI/4 : +M_PI/4;
          float cathode_u = avalanche_x*cos(theta)+avalanche_y*sin(theta);
          int strip1 = ceil((cathode_u - U_OF_STRIP_ZERO)/STRIP_SPACING +0.5);
          float cathode_u1 = (strip1-1)*STRIP_SPACING + U_OF_STRIP_ZERO;
          float delta = cathode_u-cathode_u1;
  
          /* Variables for approximating number of ion pairs  */ 
          const float w_eff=26.0; // eV, average energy needed to produce an 
                                  // ion pair.  For simplicity use pure argon
          const float n_s_per_p=2.2; // average number of secondary ion pairs
                                     // per primary ionization
          float n_p_mean,n_s_mean;
          int n_p,n_t,n_s;
          const int one=1;
      
          /* variables for gain approximation */
          const float k=1.81;   // empirical constant related to first
                                // Townsend coefficient of argon gas
          const float N=269.*273/293;  // proportional to number of gas 
                                       // molecules/cm^3 
          const float V=1800;   // V, operation voltage 
          const float V_t=703.5; // V, threshold voltage
          const float amp_gain=46; // 2.3 mV/uA into 50 Ohms
          const float C=0.946;  // capacitance in units of episilon_0 
          const float a=0.001;  // cm, radius of sense wires 
  
          /* Sauli eq. 31: */
          float gain = exp(sqrt(2.*k*V*C*N*a/M_PI)*(sqrt(V/V_t)-1.));
          float q_anode;
  
          /* Total number of ion pairs.  On average for each primary ion 
             pair produced there are n_s secondary ion pairs produced.  The
             probability distribution is a compound poisson distribution
             that requires generating two Poisson variables.
           */
          n_p_mean = dE/w_eff/(1.+n_s_per_p)*1e9;
          gpoiss_(&n_p_mean,&n_p,&one);
          n_s_mean = ((float)n_p)*n_s_per_p;
          gpoiss_(&n_s_mean,&n_s,&one);
          n_t = n_s+n_p;
          q_anode=((float)n_t)*gain*ELECTRON_CHARGE;
  
          for (node=-STRIP_NODES; node<=STRIP_NODES; node++)
          {
          /* Induce charge on the strips according to the Mathieson 
             function tuned to results from FDC prototype
           */
            float lambda1=(((float)node-0.5)*STRIP_SPACING+STRIP_GAP/2.
                          -delta)/ANODE_CATHODE_SPACING;
            float lambda2=(((float)node+0.5)*STRIP_SPACING-STRIP_GAP/2.
                           -delta)/ANODE_CATHODE_SPACING;
            float q = q_anode*amp_gain/1000.
                      *(tanh(M_PI*K2*lambda2/4.)-tanh(M_PI*K2*lambda1/4.))/4.;
  
            int strip = strip1+node;
	    /* Throw away hits on strips falling within a certain dead-zone
	       radius */
	    float strip_outer_u=cathode_u1
	      +(STRIP_SPACING+STRIP_GAP/2.)*(int)node;
	    float cathode_v=-avalanche_x*sin(theta)+avalanche_y*cos(theta);
	    float check_radius=sqrt(strip_outer_u*strip_outer_u
				    +cathode_v*cathode_v);

            if ((strip > 0) && (check_radius>STRIP_DEAD_ZONE_RADIUS) 
		&& (strip <= STRIPS_PER_PLANE))
            {
  	      int mark = (chamber<<20) + (plane<<10) + strip;
              void** twig = getTwig(&forwardDCTree, mark);
              if (*twig == 0)
              {
                s_ForwardDC_t* fdc = *twig = make_s_ForwardDC();
                s_FdcChambers_t* chambers = make_s_FdcChambers(1);
                s_FdcCathodeStrips_t* strips = make_s_FdcCathodeStrips(1);
                strips->mult = 1;
                strips->in[0].plane = plane;
                strips->in[0].strip = strip;
                strips->in[0].fdcCathodeHits = chits
                                             = make_s_FdcCathodeHits(MAX_HITS);
                chambers->mult = 1;
                chambers->in[0].module = module;
                chambers->in[0].layer = layer;
                chambers->in[0].fdcCathodeStrips = strips;
                fdc->fdcChambers = chambers;
                stripCount++;
              }
              else
              {
                s_ForwardDC_t* fdc = *twig;
                chits = fdc->fdcChambers->in[0].fdcCathodeStrips
                                        ->in[0].fdcCathodeHits;
              }
          
              for (nhit = 0; nhit < chits->mult; nhit++)
              {
                if (fabs(chits->in[nhit].t - tdrift) < TWO_HIT_RESOL)
                {
                  break;
                }
              }
              if (nhit < chits->mult)          /* merge with former hit */
              {
                chits->in[nhit].t = 
                    (chits->in[nhit].t * chits->in[nhit].q + tdrift * q)
                    / (chits->in[nhit].q += q);
              }
              else if (nhit < MAX_HITS)        /* create new hit */
              {
                chits->in[nhit].t = tdrift;
                chits->in[nhit].q = q;
                chits->mult++;
              }
              else
              {
                fprintf(stderr,"HDGeant error in hitForwardDC: ");
                fprintf(stderr,"max hit count %d exceeded, truncating!\n",
                        MAX_HITS);
              }
            }
          }
        }
      }
    }
  }
}

/* entry points from fortran */

void hitforwarddc_(float* xin, float* xout,
                   float* pin, float* pout, float* dEsum,
                   int* track, int* stack, int* history)
{
   hitForwardDC(xin,xout,pin,pout,*dEsum,*track,*stack,*history);
}


/* pick and package the hits for shipping */

s_ForwardDC_t* pickForwardDC ()
{
   s_ForwardDC_t* box;
   s_ForwardDC_t* item;

   if ((stripCount == 0) && (wireCount == 0) && (pointCount == 0))
   {
      return HDDM_NULL;
   }

   box = make_s_ForwardDC();
   box->fdcChambers = make_s_FdcChambers(32);
   box->fdcChambers->mult = 0;
   while (item = (s_ForwardDC_t*) pickTwig(&forwardDCTree))
   {
      s_FdcChambers_t* chambers = item->fdcChambers;
      int module = chambers->in[0].module;
      int layer = chambers->in[0].layer;
      int m = box->fdcChambers->mult;

      /* compress out the hits below threshold */
      s_FdcAnodeWires_t* wires = chambers->in[0].fdcAnodeWires;
      int wire;
      s_FdcCathodeStrips_t* strips = chambers->in[0].fdcCathodeStrips;
      int strip;
      s_FdcTruthPoints_t* points = chambers->in[0].fdcTruthPoints;
      int point;
      int mok=0;
      for (wire=0; wire < wires->mult; wire++)
      {
         s_FdcAnodeHits_t* ahits = wires->in[wire].fdcAnodeHits;

         int i,iok;
         for (iok=i=0; i < ahits->mult; i++)
         {
            if (ahits->in[i].dE >= THRESH_KEV/1e6)
            {
               if (iok < i)
               {
                  ahits->in[iok] = ahits->in[i];
               }
               ++iok;
               ++mok;
            }
         }
         if (iok)
         {
            ahits->mult = iok;
         }
         else if (ahits != HDDM_NULL)
         {
            FREE(ahits);
         }
      }
      if ((wires != HDDM_NULL) && (mok == 0))
      {
         FREE(wires);
         wires = HDDM_NULL;
      }

      mok = 0;
      for (strip=0; strip < strips->mult; strip++)
      {
         s_FdcCathodeHits_t* chits = strips->in[strip].fdcCathodeHits;
         int i,iok;
         for (iok=i=0; i < chits->mult; i++)
         {
            if (chits->in[i].q >= THRESH_STRIPS)
            {
               if (iok < i)
               {
                  chits->in[iok] = chits->in[i];
               }
               ++iok;
               ++mok;
            }
         }
         if (iok)
         {
            chits->mult = iok;
         }
         else if (chits != HDDM_NULL)
         {
           FREE(chits);
         }
      }
      if ((strips != HDDM_NULL) && (mok == 0))
      {
         FREE(strips);
         strips = HDDM_NULL;
      }

      if ((wires != HDDM_NULL) || 
          (strips != HDDM_NULL) ||
          (points != HDDM_NULL))
      {
        if ((m == 0) || (module > box->fdcChambers->in[m-1].module)
                     || (layer  > box->fdcChambers->in[m-1].layer))
        {
          box->fdcChambers->in[m] = chambers->in[0];
          box->fdcChambers->in[m].fdcCathodeStrips = 
                              make_s_FdcCathodeStrips(stripCount);
          box->fdcChambers->in[m].fdcAnodeWires =
                              make_s_FdcAnodeWires(wireCount);
          box->fdcChambers->in[m].fdcTruthPoints =
                              make_s_FdcTruthPoints(pointCount);
          box->fdcChambers->mult++;
        }
        else
        {
          m--;
        }
        for (strip=0; strip < strips->mult; ++strip)
        {
           int mm = box->fdcChambers->in[m].fdcCathodeStrips->mult++;
           box->fdcChambers->in[m].fdcCathodeStrips->in[mm] = strips->in[strip];
        }
        if (strips != HDDM_NULL)
        {
           FREE(strips);
        }
        for (wire=0; wire < wires->mult; ++wire)
        {
           int mm = box->fdcChambers->in[m].fdcAnodeWires->mult++;
           box->fdcChambers->in[m].fdcAnodeWires->in[mm] = wires->in[wire];
        }
        if (wires != HDDM_NULL)
        {
           FREE(wires);
        }
        for (point=0; point < points->mult; ++point)
        {
           int mm = box->fdcChambers->in[m].fdcTruthPoints->mult++;
           box->fdcChambers->in[m].fdcTruthPoints->in[mm] = points->in[point];
        }
        if (points != HDDM_NULL)
        {
           FREE(points);
        }
     }
     FREE(chambers);
     FREE(item);
   }

   stripCount = wireCount = pointCount = 0;

   if ((box->fdcChambers != HDDM_NULL) &&
       (box->fdcChambers->mult == 0))
   {
      FREE(box->fdcChambers);
      box->fdcChambers = HDDM_NULL;
   }
   if (box->fdcChambers->mult == 0)
   {
      FREE(box);
      box = HDDM_NULL;
   }
   return box;
}
