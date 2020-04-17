/**************************************************************************                                                             
* HallD software                                                          * 
* Copyright(C) 2020       GlueX and PrimEX-D Collaborations               * 
*                                                                         *                                                                
* Author: The GlueX and PrimEX-D Collaborations                           *                                                                
* Contributors: Igal Jaegle                                               *                                                               
*                                                                         *
*                                                                         *   
* This software is provided "as is" without any warranty.                 *
**************************************************************************/

#include "Combination.h"

using namespace std;

Combination::Combination (int npar) {
  n        = npar;
  maxCombi = 0;
  if (n < 2)          return;
  if (n >= MAXCOMBI)  return;
  combi = new unsigned char [n];
  iCombi   = 0;
  maxCombi = calcMaxCombi ();
  list  = new unsigned char [maxCombi * n];
  if (n & 1) oddCombi ();
  else       allCombi (0, 0);
}

void Combination::getCombi (int i) {
  memcpy (combi, list + (i * n), n);
}

void Combination::oddCombi () {
  for (int i = 0; i < n; i++) {
    combi [n-1] = i;
    allCombi (1 << i, 0);
  }
}

void Combination::allCombi (int used, int deep) {
  int nrem       = n - 2 * deep - (n & 1);
  int remain [nrem];
  int irem = 0;
  for (int i = 0; i < n; i++) 
    if ( ! ( (1 << i) & used ) )
      remain [irem++] = i;

  combi [deep*2]   = remain [0];
  for (int j = 1; j < nrem; j++) {
    combi [deep*2+1]  = remain [j];
    int xused   = (1 << remain [0]) | (1 << remain [j]) | used ;
    if (nrem >= 4)
      allCombi (xused, deep + 1);
    else {
      memcpy (list + (iCombi++) * n , combi, n);
    }
  }
}

int Combination::calcMaxCombi () {
  int max = 1;
  int nn  = n;
  if (! (nn & 1)) nn--;
  while (nn > 0) {
    max *= nn;
    nn -= 2;
  }
  return (max);
}
  
