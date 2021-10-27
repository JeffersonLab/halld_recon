// $Id$
//
///    File: DTOFGeometry.h
/// Created: Mon Jul 18 11:43:31 EST 2005
/// Creator: remitche (on Linux mantrid00 2.4.20-18.8 i686)
/// Purpose: Container class object to hold Geometry data for the TOF from CCDB

///\addtogorup TOFDetector

// the following is for doxygen describing this file
/*! \file DTOFGeometry.h 
 * TOF class providing TOF Geometry parameters
 * like number of planes, paddles, short paddles, paddle lengths, ect.
 */


#ifndef _DTOFGeometry_
#define _DTOFGeometry_

#define DTOFGEOMETRY_VERSION 2

#include <HDGEOMETRY/DGeometry.h>
#include <DANA/DApplication.h>

#include <math.h>

#include "JANA/JObject.h"
#include "JANA/JFactory.h"
using namespace jana;
// the follwing is for doxygen describing the class
/*! \class DTOFGeometry provides TOF geometry parameters

 * \fn Get_NLayers(): returns Number of planes
 * \fn Get_NPlanes(): returns Number of planes
 * \fn Get_NEnds(): returns Number of ends of paddle (shold be 2)
 * \fn Get_NBars(): returns Number of paddles in a plane
 * \fn Get_NLongBars(): returns Number of long paddles in one plane
 * \fn Get_NShortBars(): reutrns Number of short paddles in one plane
 * \fn Get_FristShortBar(): returns bar number of first short bar in a plane
 * \fn GetLastShortBar(): return bar number of last short bar in one plane
 * \fn Is_ShortBar(int paddle): returns true if paddle number refers to a short paddle
 * \fn Get_LongBarLength(): returns length or long paddles
 * \fn Get_HalfLongBarLength(): 
 * \fn Get_ShortBarLength(): returns length of short paddles
 * \fn Get_HalfShortBarLength):
 * \fn Get_BarWidth(int bar): returns width of paddle with number "bar", 1,2,3, ....
 * \fn Get_CenterVertPlane():
 * \fn Get_CenterHorizPlane():
 * \fn Get_CenterMidPlane(): 
 * \fn Get_CCDB_DirectoryName(): returns name of Geometry directory, "TOF" or "TOF2"
 * \fn bar2y(int bar, int end=0): converts bar number to position in segemenation
 * \fn y2bar(double y): converts paddle position the direction of segemenation into paddle number

 */
class DTOFGeometry : public JObject {

 public:
  JOBJECT_PUBLIC(DTOFGeometry);
  
  DTOFGeometry(const DGeometry* locGeometry);
  
  // Get functions
  int Get_NLayers() const { return NLAYERS; };
  int Get_NPlanes() const { return NLAYERS; };
  int Get_NEnds() const { return NENDS; };
  // NOTE THAT Get_NBars() is the number of bars along one end!
  // Therefore Get_NBars() != Get_NLongBars()+Get_NShortBars()
  int Get_NBars() const { return NINSTALLBARS; }
  int Get_NLongBars() const { return NLONGBARS; }
  int Get_NShortBars() const { return NSHORTBARS; }

  int Get_FirstShortBar() const { return FirstShortBar; }
  int Get_LastShortBar() const { return LastShortBar; }

  bool Is_ShortBar(int paddle) const { return (paddle >= FirstShortBar) && (paddle <= LastShortBar); }

  float Get_LongBarLength() const { return LONGBARLENGTH; }
  float Get_HalfLongBarLength() const { return HALFLONGBARLENGTH; }
  float Get_ShortBarLength() const { return SHORTBARLENGTH; }
  float Get_HalfShortBarLength() const { return HALFSHORTBARLENGTH; }
  float Get_BarWidth(int bar) const { return YWIDTH[bar]; }

  float Get_CenterVertPlane() const { return CenterVPlane; };  
  float Get_CenterHorizPlane() const { return CenterHPlane; };
  float Get_CenterMidPlane() const { return CenterMPlane; };

  string Get_CCDB_DirectoryName() const {
  	if(Get_NBars() == 46) {
	    return "TOF2";
  	} else {
  		return "TOF";
  	}
  }

  float bar2y(int bar, int end=0) const;  
  int y2bar(double y) const;

  void toStrings(vector<pair<string,string> > &items) const {
		AddString(items, "NBARS", "%d", Get_NBars() );
		AddString(items, "NLONGBARS", "%d",  Get_NLongBars() );
		AddString(items, "NSHORTBARS", "%d", Get_NShortBars() );
		AddString(items, "LONGBARLENGTH", "%6.3f", Get_LongBarLength() );
		AddString(items, "SHORTBARLENGTH", "%6.3f", Get_ShortBarLength() );
		AddString(items, "BARWIDTH", "%6.3f", Get_BarWidth(0) );
  }
  
 private:
  int NLAYERS;         /// number of scintillator layers
  int NENDS;           /// maximum number of ends that are read out (should be 2!)

  int NLONGBARS;        ///> number of long scintillator bars
  //int NWIDEBARS;        ///> number of wide long+short scintillator bars (deprecated)
  int NSHORTBARS;       ///> number of short scintillator bars
  int NBARS;            ///> number of long scintillator bars
  int NINSTALLBARS;     ///> number of bars vertically = NLONGBARS + NSHORTBARS/2

  int FirstShortBar;    ///> bar number of first short bar
  int LastShortBar;     ///> bar number of last short bar of same type north

  float LONGBARLENGTH;  ///> length of the long scintillators
  float HALFLONGBARLENGTH;  ///> middle of the long scintillators
  float SHORTBARLENGTH; ///> length of the short scintillators
  float HALFSHORTBARLENGTH;  ///> middle of the short scintillators
  float BARWIDTH;       ///> width of the scintillator bars

  float CenterVPlane;  /// center z position of Vertical Plane
  float CenterHPlane;  /// center z position of Horizontal Plane
  float CenterMPlane;  /// center z position between the two Plane

  vector<double> YPOS;  ///> y (perpendicular) position for bar number
  vector<double> YWIDTH;  ///> y (perpendicular) bar width per bar number
 
};

#endif // _DTOFGeometry_

