// $Id$
//
//    File: DTOFGeometry.h
// Created: Mon Jul 18 11:43:31 EST 2005
// Creator: remitche (on Linux mantrid00 2.4.20-18.8 i686)
//

#ifndef _DTOFGeometry_
#define _DTOFGeometry_

#include <math.h>

#include "JANA/JObject.h"
#include "JANA/JFactory.h"
using namespace jana;

class DTOFGeometry : public JObject {

 public:
  JOBJECT_PUBLIC(DTOFGeometry);

  
  // Get functions
  int Get_NLayers() const { return NLAYERS; };
  int Get_NPlanes() const { return NLAYERS; };
  int Get_NEnds() const { return NENDS; };
  // NOTE THAT Get_NBars() is the number of bars along one end!
  // Therefore Get_NBars() != Get_NLongBars()+Get_NShortBars()
  int Get_NBars() const { return NBARS; }
  int Get_NLongBars() const { return NLONGBARS; }
  int Get_NShortBars() const { return NSHORTBARS; }

  int Get_FirstShortBar() const { return FirstShortBar; }
  int Get_LastShortBar() const { return LastShortBar; }

  bool Is_ShortBar(int paddle) const { return (paddle >= FirstShortBar) && (paddle <= LastShortBar); }

  float Get_CenterVertPlane() const { return CenterVPlane; };  
  float Get_CenterHorizPlane() const { return CenterHPlane; };
  float Get_CenterMidPlane() const { return CenterMPlane; };

  string Get_CCDB_DirectoryName() const {
      return "TOF";
  }

  int NLONGBARS;        ///> number of long scintillator bars
  int NWIDEBARS;        ///> number of long scintillator bars
  int NBARS;            ///> number of long scintillator bars
  int NSHORTBARS;       ///> number of short scintillator bars
  float LONGBARLENGTH;  ///> length of the long scintillators
  float SHORTBARLENGTH; ///> length of the short scintillators
  float BARWIDTH;       ///> width of the scintillator bars
  float YPOS[50];       ///> y position for bar number
  int FirstShortBar;    ///> bar number of first short bar
  int LastShortBar;     ///> bar number of last short bar of same type north

  float CenterVPlane;  /// center z position of Vertical Plane
  float CenterHPlane;  /// center z position of Horizontal Plane
  float CenterMPlane;  /// center z position between the two Plane

  int NLAYERS;         /// number of scintillator layers
  int NENDS;           /// maximum number of ends that are read out (should be 2!)

  float bar2y(int bar, int end=0)  const ///> convert bar number to the
  ///> position of the center of the
  ///> bar in local coordinations
  {
    float y;
    y = YPOS[bar];

    if (bar>=FirstShortBar && bar<=LastShortBar && end != 0) y *= -1.0;

    return y;
  }
  
  
	int y2bar(double y) const   ///> convert local position y to bar number
	{
		if(y < (YPOS[1] - BARWIDTH/2.0))
			return 0;
		if(y > (YPOS[44] + BARWIDTH/2.0))
			return 0;

		int jm=1;
		if (y>YPOS[1])
		{
			int jl=-1;
			int ju=44;
			while(ju - jl > 1)
			{
				jm = (ju + jl) >> 1;
				if (y >= YPOS[jm])
					jl = jm;
				else
					ju = jm;
			}
			if (fabs(y - YPOS[jm - 1]) < fabs(y - YPOS[jm]))
				return jm - 1;
			if (fabs(y - YPOS[jm + 1]) < fabs(y - YPOS[jm]))
				return jm + 1;
		}

		return jm;
	}

		void toStrings(vector<pair<string,string> > &items)const{
			AddString(items, "NBARS", "%d", NBARS);
			AddString(items, "NLONGBARS", "%d", NLONGBARS);
			AddString(items, "NWIDEBARS", "%d", NWIDEBARS);
			AddString(items, "NSHORTBARS", "%d", NSHORTBARS);
			AddString(items, "LONGBARLENGTH", "%6.3f", LONGBARLENGTH);
			AddString(items, "SHORTBARLENGTH", "%6.3f", SHORTBARLENGTH);
			AddString(items, "BARWIDTH", "%6.3f", BARWIDTH);
		}
};

#endif // _DTOFGeometry_

