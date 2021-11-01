// $Id$
//
//    File: DCTOFPoint.h
// Created: Thu Oct 28 07:48:04 EDT 2021
// Creator: staylor (on Linux ifarm1901.jlab.org 3.10.0-1062.4.1.el7.x86_64 x86_64)
//

#ifndef _DCTOFPoint_
#define _DCTOFPoint_

#include <JANA/JObject.h>
#include <JANA/JFactory.h>

class DCTOFPoint:public jana::JObject{
 public:
  JOBJECT_PUBLIC(DCTOFPoint);

  int bar; ///< bar number
  double dE; ///< Energy depostion in GeV
  double t; ///< Time in ns 
  double y; ///< position along scintillator in cm
		
  // This method is used primarily for pretty printing
  // the second argument to AddString is printf style format
  void toStrings(vector<pair<string,string> > &items)const{
    AddString(items, "bar", "%4d", bar);
    AddString(items, "dE", "%f", dE);
    AddString(items, "t", "%f", t);
    AddString(items, "y", "%f", y);
  }
		
};

#endif // _DCTOFPoint_

