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

//#if !defined(COMBINATION)
//#define COMBINATION

#ifndef _Combination_
#define _Combination_

#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <string.h>

using namespace std;

class Combination
{
#define MAXCOMBI 32
protected:
  int n;
  int maxCombi, iCombi;
  unsigned char * list;
  
public:
  unsigned char * combi;

  Combination (int npar);
  void oddCombi ();
  void allCombi (int used, int deep);
  void getCombi (int i);
  int getMaxCombi () { return (maxCombi);  }
  int getN        () { return (n);         }
  int calcMaxCombi ();
};

#endif // _combination_
//#endif // _COMBINATION_H_
