
#include <stdint.h>
#include <vector>
#include <iostream>
using namespace std;

#include <Rtypes.h>  // ROOT type definitions (e.g. Int_t)


// FA125 emulation functions cdc_hit, cdc_time etc for NPK=1 (only the first peak is identified)
//
// Peak amplitude and integral are returned without pedestal subtraction
//
// Returned values are unscaled.  (ie IBIT, ABIT, PBIT are not used here)
//
// The pedestal returned is the mean of the NPED2 samples ending PED samples before the sample containing the hit threshold crossing.
//
//
// The ADC data buffer has NW samples in total, comprising
// [unused samples][NPED samples for pedestal][samples WINDOW_START to WINDOW_END][20 or more extra samples] 
//
// WINDOW_START is the sample immediately after the NPED samples in the pedestal window.
// The hit search starts with sample WINDOW_START+PG and ends including sample WINDOW_END. This is for ease of use with older data.
// In the new firmware, there are no unused samples at the start, and 20 samples at the end.
// The first sample is numbered sample 0.
//
//
// This requires that: 
//                     WINDOW_START >= NPED 
//                     WINDOW_END+20 < NW (number of samples supplied) 
//                     NPED2 >= NPED
//


  // cdc_time q_code values:

  // q_code  Time returned       Condition
  // 0       Leading edge time   Good 
  // 1       X*10 - 29           Sample value of 0 found 
  // 1       X*10 - 28           Sample value greater than PED_MAX found in adc[0 to PED]
  // 1       X*10 - 27           Sample values lie below the high timing threshold 
  // 1       TCL*10 + 4          Low timing threshold crossing sample TCL occurs too late to upsample
  // 1       TCL*10 + 5          One or more upsampled values are negative 
  // 1       TCL*10 + 9          The upsampled values are too low


void cdc_hit(Int_t&, Int_t&, Int_t&, Int_t[], Int_t, Int_t, Int_t, Int_t, Int_t, Int_t);   // look for a hit
void cdc_time(Int_t&, Int_t&, Int_t[], Int_t, Int_t, Int_t, Int_t); // find hit time
void cdc_integral(Long_t&, Int_t&, Int_t, Int_t[], Int_t, Int_t); // find integral
void cdc_max(Int_t&, Int_t&, Int_t, Int_t[], Int_t); // find first max amplitude after hit
void upsamplei(Int_t[], Int_t, Int_t[], Int_t);   // upsample



void cdc_hit(Int_t &hitfound, Int_t &hitsample, Int_t &pedestal, Int_t adc[], Int_t WINDOW_START, Int_t WINDOW_END, Int_t HIT_THRES, Int_t NPED, Int_t NPED2, Int_t PG) {


  pedestal=0;  //pedestal
  Int_t threshold=0;

  Int_t i=0;

  // calc pedestal as mean of NPED samples before trigger
  for (i=0; i<NPED; i++) {
    pedestal += adc[WINDOW_START-NPED+i];
  }

  pedestal = ( NPED==0 ? 0:(pedestal/NPED) );   // Integer div is ok as fpga will do 2 rightshifts

  threshold = pedestal + HIT_THRES;

  // look for threshold crossing
  i = WINDOW_START - 1 + PG;
  hitfound = 0;

  while ((hitfound==0) && (i<WINDOW_END-1)) {

    i++;

    if (adc[i] >= threshold) {
      if (adc[i+1] >= threshold) {
        hitfound = 1;
        hitsample = i;
      }
    }
  }

  if (hitfound == 1) {

    //calculate new pedestal ending just before the hit

    pedestal = 0;

    for (i=0; i<NPED2; i++) {
      pedestal += adc[hitsample-PG-i];
    }

    pedestal = ( NPED2==0 ? 0:(pedestal/NPED2) );
  }


}




void cdc_integral(Long_t& integral, Int_t& overflows, Int_t timesample, Int_t adc[], Int_t WINDOW_END, Int_t INT_END) {

  Int_t i=0;

  integral = 0;
  overflows = 0;

  if (timesample <= WINDOW_END) {

    Int_t lastsample = timesample + INT_END - 1;

    if (lastsample > WINDOW_END) lastsample = WINDOW_END;

    for (i = timesample; i <= lastsample; i++ ) {

      integral += (Long_t)adc[i];
      if (adc[i]==(Long_t)4095) overflows++;   

    }

  }
 

}




void cdc_max(Int_t& maxamp, Int_t &maxsample, Int_t hitsample, Int_t adc[], Int_t WINDOW_END) {

  int i;
  int ndec = 0;  //number of decreasing samples

  //make sure we are on an up-slope

  while ((adc[hitsample] <= adc[hitsample-1]) && (hitsample <= WINDOW_END)) hitsample++;


  maxamp = adc[hitsample];
  maxsample = hitsample;


  for (i=hitsample; i<=WINDOW_END; i++) {

    if (adc[i] > adc[i-1]) {
      maxamp = adc[i];
      maxsample = i;
      ndec = 0;
    }

    if (adc[i] <= adc[i-1]) ndec++;
    if (ndec==2) break;
  }


  if (hitsample >= WINDOW_END) {
    maxamp = adc[WINDOW_END];
    maxsample = WINDOW_END;
  }

}




void cdc_time(Int_t &le_time, Int_t &q_code, Int_t adc[], Int_t NU, Int_t PG, Int_t THRES_HIGH, Int_t THRES_LOW) {


  // adc[NU]     array of samples
  // NU=20       size of array
  // PG          pedestal gap - hit threshold crossing is PG samples after adc[PED] - the single sample to be used as pedestal here
  // THRES_HIGH  high timing threshold (eg 4 x pedestal-width )
  // THRES_LOW   high timing threshold (eg 1 x pedestal-width )
  //
  // le_time     leading edge time as 0.1x number of samples since first sample supplied
  // q_code      quality code, 0=good, >0=rough estimate (firmware returns 0=good, 1=not so good)
  //
  // q_code  Time returned       Condition
  // 0       Leading edge time   Good 
  // 1       X*10 - 29           Sample value of 0 found 
  // 1       X*10 - 28           Sample value greater than PED_MAX found in adc[0 to PED]
  // 1       X*10 - 27           Sample values lie below the high timing threshold 
  // 1       TCL*10 + 4          Low timing threshold crossing sample TCL occurs too late to upsample
  // 1       TCL*10 + 5          One or more upsampled values are negative 
  // 1       TCL*10 + 9          The upsampled values are too low
  //



  const Int_t NUPSAMPLED = 6;       // number of upsampled values to calculate, minimum is 6
  const Int_t SET_ADC_MIN = 20;     // adjust adc values so that the min is at 20
  const Int_t LIMIT_PED_MAX = 511;  // max acceptable value in adc[0 to PED]
  const Int_t PED = 5;              // take local pedestal to be adc[PED]

  const Int_t START_SEARCH = PED+1; // -- start looking for hi threshold xing with this sample

  const Int_t X = PED + PG;         // hit threshold crossing sample is adc[X]
  const Int_t ROUGH_TIME = (X*10)-30; // -- add onto this to return rough time estimates

  Int_t iubuf[NUPSAMPLED] = {0};  // array of upsampled values; iubuf[0] maps to low thres xing sample 

  Int_t adc_thres_hi = 0; // high threshold
  Int_t adc_thres_lo = 0; // low threshold

  //    -- contributions to hit time, these are summed together eventually, units of sample/10
  Int_t itime1 = 0; // which sample
  Int_t itime2 = 0; // which minisample
  Int_t itime3 = 0; // correction from interpolation
    
  //    -- search vars
  Int_t adc_sample_hi = 0; // integer range 0 to NU := 0;  --sample number for adc val at or above hi thres
  Int_t adc_sample_lo = 0; // integer range 0 to NU := 0;  -- sample num for adc val at or below lo thres
  Int_t adc_sample_lo2 = 0; // integer range 0 to 12:= 0;  -- minisample num for adc val at or below lo thres

  Bool_t over_threshold = kFALSE;
  Bool_t below_threshold = kFALSE;

  
  // upsampling checks
  Int_t ups_adjust = 0;

  Int_t i = 0;



  //check all samples are >0
  //check all samples from 0 to pedestal are <= LIMIT_PED_MAX
  //adc=zero and pedestal limit checks are in same fadc clock cycle


  i = 0;
  q_code = 0;
  while (i<NU) {

    if (adc[i] == 0) {
      le_time = ROUGH_TIME + 1;
      q_code = 1;
    }

    if ((i < PED+1) && (adc[i] > LIMIT_PED_MAX)) {
      le_time = ROUGH_TIME + 2;
      q_code = 2;
    }
    i++;
  }
 
  if (q_code>0) return; 


  //  add offset to move min val in subset equal to SET_ADC_MIN
  //  this is to move samples away from 0 to avoid upsampled pts going -ve (on a curve betw 2 samples)

  Int_t adcmin = 4095; 

  i=0; 

  while (i<NU) {

    if (adc[i] < adcmin) {
      adcmin = adc[i];
    }

    i++;
  }

  Int_t adcoffset = SET_ADC_MIN - adcmin;  

  i=0; 

  while (i<NU) {
    adc[i] = adc[i] + adcoffset;
    //    if (adc[i] > 4095) adc[i] = 4095; //DO NOT saturate
    i++;
  }

  // eg if adcmin is 100, setmin is 30, adcoffset = 30 - 100 = -70, move adc down by 70
 

  //////////////////////////////

  // calc thresholds

  adc_thres_hi = adc[PED] + THRES_HIGH;
  adc_thres_lo = adc[PED] + THRES_LOW;

  // search for high threshold crossing

  over_threshold = kFALSE;
  i = START_SEARCH;

  while ((!over_threshold)&&(i<NU)) {

    if (adc[i] >= adc_thres_hi) {
      adc_sample_hi = i;
      over_threshold = kTRUE;
    }

    i++;
  }


  if (!over_threshold) {

    le_time = ROUGH_TIME + 3;
    q_code = 3;
    return;
   
  }


  // search for low threshold crossing

  below_threshold = kFALSE;
  i = adc_sample_hi-1;

  while ((!below_threshold) && (i>=PED)) {  

    if (adc[i] <= adc_thres_lo) {
      adc_sample_lo = i;
      itime1 = i*10;
      below_threshold = kTRUE;
    }

    i--;
  }



  if (adc[adc_sample_lo] == adc_thres_lo) {   // no need to upsample

    le_time = itime1;
    q_code = 0;
    return;

  }
   

  if (adc_sample_lo > NU-7) {   // too late to upsample

    le_time = itime1 + 4;
    q_code = 4;
    return;
   
  }



  //upsample values from adc_sample_lo to adc_sample_lo + 1 

  upsamplei(adc, adc_sample_lo, iubuf, NUPSAMPLED);



  //check upsampled values are >0

  Bool_t negups = kFALSE;
  
  i=0;
  while ((!negups)&&(i<NUPSAMPLED)) {

    if (iubuf[i] < 0 ) {
      negups = kTRUE;
    }

    i++;
  }


  if (negups) {

    le_time = itime1 + 5;   
    q_code = 5;
    return;
   
  }


  // correct errors 
  // iubuf[0] should be equal to adc[adc_sample_lo] and iubuf[5] should equal adc[adc_sample_lo+1]
  // very steep pulse gradients cause errors in upsampling with larger errors in the later values
  // match iubuf[0] to adc[adc_sample_lo] so that the threshold crossing must be at or after iubuf[0]

  ups_adjust = iubuf[0] - adc[adc_sample_lo];

  // move threshold correspondingly instead of correcting upsampled values

  adc_thres_lo = adc_thres_lo + ups_adjust;

  // check that threshold crossing lies within the range of iubuf[0 to 5]

  if (iubuf[NUPSAMPLED-1]<= adc_thres_lo) { //bad upsampling

    le_time = itime1 + 9;   //midway
    q_code = 6;
    return;

  }



  // search through upsampled array

  below_threshold = kFALSE;
  i = NUPSAMPLED-2;

  while ((!below_threshold) && (i>=0)) {

    if (iubuf[i] <= adc_thres_lo) {
      adc_sample_lo2 = i;
      below_threshold = kTRUE;
    }

    i--;
  }



  if (!below_threshold) { //upsampled points did not go below thres

    printf("upsampled points did not go below threshold - should be impossible\n");
    le_time = 0;
    q_code = 9;
    return;
  }




  itime2 = adc_sample_lo2*2;  //  convert from sample/5 to sample/10



  //interpolate

  itime3 = 0;
            

  if (iubuf[adc_sample_lo2] != adc_thres_lo) {
                       
    if (2*adc_thres_lo >= iubuf[adc_sample_lo2] + iubuf[adc_sample_lo2+1]) itime3 = 1;

  }


  le_time = itime1 + itime2 + itime3;  //   -- this is time from first sample point, in 1/10ths of samples
  q_code = 0;


}


void cdc_time2(Double_t &le_time, Int_t &q_code, Int_t pedestal, Int_t adc[], Int_t NU, Int_t PG, Int_t THRES_HIGH, Int_t THRES_LOW) {

  // DIFFERENT TIME CALC!!

  // adc[NU]     array of samples
  // NU=20       size of array
  // PG          pedestal gap - hit threshold crossing is PG samples after adc[PED] - the single sample to be used as pedestal here
  // THRES_HIGH  high timing threshold (eg 4 x pedestal-width )
  // THRES_LOW   high timing threshold (eg 1 x pedestal-width )
  //
  // le_time     leading edge time as 0.1x number of samples since first sample supplied
  // q_code      quality code, 0=good, >0=rough estimate (firmware returns 0=good, 1=not so good)
  //
  // q_code  Time returned       Condition
  // 0       Leading edge time   Good 
  // 1       X*10 - 29           Sample value of 0 found 
  // 1       X*10 - 28           Sample value greater than PED_MAX found in adc[0 to PED]
  // 1       X*10 - 27           Sample values lie below the high timing threshold 
  // 1       TCL*10 + 4          Low timing threshold crossing sample TCL occurs too late to upsample
  // 1       TCL*10 + 5          One or more upsampled values are negative 
  // 1       TCL*10 + 9          The upsampled values are too low
  //



  const Int_t NUPSAMPLED = 6;       // number of upsampled values to calculate, minimum is 6
  const Int_t SET_ADC_MIN = 20;     // adjust adc values so that the min is at 20
  const Int_t LIMIT_PED_MAX = 511;  // max acceptable value in adc[0 to PED]
  Int_t PED = 5;              // take local pedestal to be adc[PED]

  const Int_t START_SEARCH = PED+1; // -- start looking for hi threshold xing with this sample

  const Int_t X = PED + PG;         // hit threshold crossing sample is adc[X]
  const Int_t ROUGH_TIME = (X*10)-30; // -- add onto this to return rough time estimates

  Int_t iubuf[NUPSAMPLED] = {0};  // array of upsampled values; iubuf[0] maps to low thres xing sample 

  Int_t adc_thres_hi = 0; // high threshold
  Int_t adc_thres_lo = 0; // low threshold

  //    -- contributions to hit time, these are summed together eventually, units of sample/10
  Int_t itime1 = 0; // which sample
  Int_t itime2 = 0; // which minisample
  Int_t itime3 = 0; // correction from interpolation
    
  //    -- search vars
  Int_t adc_sample_hi = 0; // integer range 0 to NU := 0;  --sample number for adc val at or above hi thres
  Int_t adc_sample_hi2 = 0; // integer range 0 to NU := 0;  --sample number for adc val at or above hi thres
  Int_t adc_sample_lo = 0; // integer range 0 to NU := 0;  -- sample num for adc val at or below lo thres
  Int_t adc_sample_lo2 = 0; // integer range 0 to 12:= 0;  -- minisample num for adc val at or below lo thres

  Bool_t over_threshold = kFALSE;
  Bool_t below_threshold = kFALSE;

  
  // upsampling checks
  Int_t ups_adjust = 0;

  Int_t i = 0;

  int VERBOSE = 0;



  le_time = 0;

  //check all samples are >0
  //check all samples from 0 to pedestal are <= LIMIT_PED_MAX
  //adc=zero and pedestal limit checks are in same fadc clock cycle


  i = 0;
  while (i<NU) {

    if (adc[i] == 0) {
      le_time = ROUGH_TIME + 1;
      q_code = 1;
    }

    if ((i < PED+1) && (adc[i] > LIMIT_PED_MAX)) {
      le_time = ROUGH_TIME + 2;
      q_code = 2;
    }
    i++;
  }
 
  if (q_code>0) return; 


  //  add offset to move min val in subset equal to SET_ADC_MIN
  //  this is to move samples away from 0 to avoid upsampled pts going -ve (on a curve betw 2 samples)

  Int_t adcmin = 4095; 

  i=0; 

  while (i<NU) {

    if (adc[i] < adcmin) {
      adcmin = adc[i];
    }

    i++;
  }

  Int_t adcoffset = SET_ADC_MIN - adcmin;  

  i=0; 

  while (i<NU) {
    adc[i] = adc[i] + adcoffset;
    if (adc[i] > 4095) adc[i] = 4095; //saturate
    i++;
  }

  // eg if adcmin is 100, setmin is 30, adcoffset = 30 - 100 = -70, move adc down by 70
 

  // find max adc value

  int maxamp = 0;
  i = 0;

  while (i<NU) {
    if (adc[i]>maxamp) maxamp = adc[i];
    i++;
  }

  //*** set pedestal according to maxamp.....

  if (VERBOSE) printf("\nfound maxamp %i\n",maxamp);

  /* PED = 4; */
  /* if (maxamp>200) PED = 5;    */
  /* if (maxamp>500) PED = 6;    */
  /* if (maxamp>3000) PED = 7;  */

  //  pedestal = adc[PED];


  adc_thres_hi = pedestal + THRES_HIGH;
  adc_thres_lo = pedestal + THRES_LOW;


  //////////////////////////////

  // calc thresholds

  //adc_thres_hi = adc[PED] + THRES_HIGH;
  //adc_thres_lo = adc[PED] + THRES_LOW;

  if (VERBOSE) printf("thresholds %i  %i\n",adc_thres_hi,adc_thres_lo);

  // search for high threshold crossing

  over_threshold = kFALSE;
  i = START_SEARCH;

  while ((!over_threshold)&&(i<NU)) {

    if (adc[i] >= adc_thres_hi) {
      adc_sample_hi = i;
      over_threshold = kTRUE;
    }

    i++;
  }


  if (!over_threshold) {

    le_time = ROUGH_TIME + 3;
    q_code = 3;
    return;
   
  }

  if (VERBOSE) printf("found high threshold upward xing in sample %i val %i\n",adc_sample_hi,adc[adc_sample_hi]);

  // search for low threshold crossing

  below_threshold = kFALSE;
  i = adc_sample_hi-1;

  while ((!below_threshold) && (i>=PED)) {  

    if (adc[i] <= adc_thres_lo) {
      adc_sample_lo = i;
      itime1 = i*10;
      below_threshold = kTRUE;
    }

    i--;
  }

  if (VERBOSE) printf("found low threshold downward xing in sample %i val %i\n",adc_sample_lo,adc[adc_sample_lo]);


  if (adc[adc_sample_lo] == adc_thres_lo) {   // no need to upsample */

     le_time = itime1; 
     q_code = 0; 
     // return; 

  } 
   

  if (adc_sample_lo > NU-7) {   // too late to upsample

    le_time = itime1 + 4;
    q_code = 4;
    return;
   
  }



  if (le_time == 0) {

    //upsample values from adc_sample_lo to adc_sample_lo + 1 
  
    if (VERBOSE) printf("upsampling sample %i val %i to next val %i \n",adc_sample_lo,adc[adc_sample_lo],adc[adc_sample_lo+1]);

    upsamplei(adc, adc_sample_lo, iubuf, NUPSAMPLED);

    for (i=0;i<NUPSAMPLED; i++) if (VERBOSE) printf("iubuf %i\n",iubuf[i]);


    //check upsampled values are >0

    Bool_t negups = kFALSE;
  
    i=0;
    while ((!negups)&&(i<NUPSAMPLED)) {

      if (iubuf[i] < 0 ) {
        negups = kTRUE;
      }

      i++;
    }


    if (negups) {

      le_time = itime1 + 5;   
      q_code = 5;
      return;
   
    }


    // correct errors 
    // iubuf[0] should be equal to adc[adc_sample_lo] and iubuf[5] should equal adc[adc_sample_lo+1]
    // very steep pulse gradients cause errors in upsampling with larger errors in the later values
    // match iubuf[0] to adc[adc_sample_lo] so that the threshold crossing must be at or after iubuf[0]

    ups_adjust = iubuf[0] - adc[adc_sample_lo];

    // move threshold correspondingly instead of correcting upsampled values

    adc_thres_lo = adc_thres_lo + ups_adjust;

    // check that threshold crossing lies within the range of iubuf[0 to 5]

    if (iubuf[NUPSAMPLED-1]<= adc_thres_lo) { //bad upsampling
      le_time = itime1 + 9;   //midway
      q_code = 6;
      return;
    }

    // search through upsampled array

    below_threshold = kFALSE;
    i = NUPSAMPLED-2;

    while ((!below_threshold) && (i>=0)) {

      if (iubuf[i] <= adc_thres_lo) {
        adc_sample_lo2 = i;
        below_threshold = kTRUE;
      }

      i--;
    }

    if (!below_threshold) { //upsampled points did not go below thres
      if (VERBOSE) printf("upsampled points did not go below threshold - should be impossible\n");
      le_time = 0;
      q_code = 9;
      return;
    }

    itime2 = adc_sample_lo2*2;  //  convert from sample/5 to sample/10



  //interpolate

    itime3 = 0;
            
    if (iubuf[adc_sample_lo2] != adc_thres_lo) {
      if (2*adc_thres_lo >= iubuf[adc_sample_lo2] + iubuf[adc_sample_lo2+1]) itime3 = 1;
    }


    le_time = itime1 + itime2 + itime3;  //   -- this is time from first sample point, in 1/10ths of samples
    q_code = 0;

  }


  // extra part below here. 


  double tl_time = adc_sample_lo*10 + adc_sample_lo2*2;  

  if ( adc_thres_lo > iubuf[adc_sample_lo2] ) {

    if ( iubuf[adc_sample_lo2] == iubuf[adc_sample_lo2 + 1] ) {
      if (VERBOSE) printf("iubuf plateau \n");
    } else {
      tl_time += 2*(adc_thres_lo - iubuf[adc_sample_lo2])/double(iubuf[adc_sample_lo2 + 1] - iubuf[adc_sample_lo2]);
    }
  } else {
    if (VERBOSE) printf("adc_thres_lo %i = iubuf[adc_sample_lo2]\n",adc_thres_lo);
  }

  if (VERBOSE) printf("le_time %.0f tl_time %.1f \n",le_time,tl_time);   //should be similar



 //******************** upsample high time threshold sample

  double th_time = 0;

  adc_sample_hi--;   //move it back one so that thres is between this and the next

  if (VERBOSE) printf("upsampling sample %i val %i to next val %i \n",adc_sample_hi,adc[adc_sample_hi],adc[adc_sample_hi+1]);

  upsamplei(adc, adc_sample_hi, iubuf, NUPSAMPLED);

  for (i=0;i<NUPSAMPLED; i++) if (VERBOSE) printf("iubuf %i\n",iubuf[i]);

  ups_adjust = iubuf[0] - adc[adc_sample_hi];

  // move threshold correspondingly instead of correcting upsampled values

  adc_thres_hi = adc_thres_hi + ups_adjust;

  // check that threshold crossing lies within the range of iubuf[0 to 5]

  if (iubuf[NUPSAMPLED-1]<= adc_thres_hi) { //bad upsampling
    if (VERBOSE) printf("bad upsampling\n");
    q_code = 16;
    th_time = 0;
    return;
  }


  // search through upsampled array

  below_threshold = kFALSE;
  i = NUPSAMPLED-2;

  while ((!below_threshold) && (i>=0)) {

    if (iubuf[i] <= adc_thres_hi) {
      adc_sample_hi2 = i;
      below_threshold = kTRUE;
    }

    i--;
  }



  if (!below_threshold) { //upsampled points did not go below thres
    if (VERBOSE) printf("upsampled points did not go below threshold - should be impossible\n");
    th_time = 0;
    q_code = 19;
    return;
  }




  itime2 = adc_sample_hi2*2;  //  convert from sample/5 to sample/10


  th_time = adc_sample_hi*10 + adc_sample_hi2*2;  

  th_time += 2*(adc_thres_hi - iubuf[adc_sample_hi2])/double(iubuf[adc_sample_hi2 + 1] - iubuf[adc_sample_hi2]);

  if (VERBOSE) printf("th_time %.1f \n",th_time); 





  // calc thresholds again - upsadjust moved them.

  adc_thres_hi = pedestal + THRES_HIGH;
  adc_thres_lo = pedestal + THRES_LOW;



  //  if (VERBOSE) printf("TH sample %i value %i   LE sample %i value %i   pedestal %i\n",adc_sample_hi,adc[adc_sample_hi],adc_sample_lo,le_time,adc[PED]);


  //interpolate through th_time and le_time to reach pedestal.  

  le_time = th_time - (th_time - tl_time)*THRES_HIGH/(THRES_HIGH-THRES_LOW);

  if (VERBOSE) printf("new le_time %.1f\n",le_time);

  q_code = 0;
  return;



}





void upsamplei(Int_t x[], Int_t startpos, Int_t z[], const Int_t NUPSAMPLED) {

  // x is array of samples
  // z is array of upsampled data
  // startpos is where to start upsampling in array x, only need to upsample a small region


  const Int_t nz = NUPSAMPLED;

  //  const Int_t Kscale = 32768;
  //  const Int_t K[43]={-8,-18,-27,-21,10,75,165,249,279,205,-2,-323,-673,-911,-873,-425,482,1773,3247,4618,5591,5943,5591,4618,3247,1773,482,-425,-873,-911,-673,-323,-2,205,279,249,165,75,10,-21,-27,-18,-8}; //32768

  Int_t k,j,dk;


  const Int_t Kscale = 16384;
  const Int_t K[43] = {-4, -9, -13, -10, 5, 37, 82, 124, 139, 102, -1, -161, -336, -455, -436, -212, 241, 886, 1623, 2309, 2795, 2971, 2795, 2309, 1623, 886, 241, -212, -436, -455, -336, -161, -1, 102, 139, 124, 82, 37, 5, -10, -13, -9, -4};                           


  //don't need to calculate whole range of k possible
  //earliest value k=42 corresponds to sample 4.2
  //               k=43                sample 4.4
  //               k=46                sample 5.0
  

  // sample 4 (if possible) would be at k=41
  // sample 4.2                         k=42
  // sample 5                           k=46

  // sample x                           k=41 + (x-4)*5
  // sample x-0.2                       k=40 + (x-4)*5


  Int_t firstk = 41 + (startpos-4)*5;


  for (k=firstk; k<firstk+nz; k++) {

    dk = k - firstk;    

    z[dk]=0.0;

    for (j=k%5;j<43;j+=5) {

      z[dk] += x[(k-j)/5]*K[j]; 

    }

    //    printf("dk %i z %i 5z %i  5z/scale %i\n",dk,z[dk],5.0*z[dk],5.0*z[dk]/Kscale);

    z[dk] = (Int_t)(5*z[dk])/Kscale;

  }

}

