// $Id$
//
//    File: DCCALGeometry.h
// Created: Tue Nov 30 15:42:41 EST 2010
// Creator: davidl (on Linux ifarml6 2.6.18-128.el5 x86_64)
//

#ifndef _DCCALGeometry_
#define _DCCALGeometry_

#include <JANA/JObject.h>
#include <JANA/JFactory.h>

#include "DVector2.h"
#include "units.h"

class DCCALGeometry:public jana::JObject{

	public:
		JOBJECT_PUBLIC(DCCALGeometry);
		
		DCCALGeometry();
		~DCCALGeometry(){}

		static const int kCCALBlocksWide   = 12;
		static const int kCCALBlocksTall   = 12;
		static const int kCCALMaxChannels  = kCCALBlocksWide * kCCALBlocksTall;
		static const int kCCALMidBlock     = (kCCALBlocksWide)/2;
		static const int kCCALBeamHoleSize = 2;

		static double blockSize()  { return  2.09 * k_cm; }
		static double blockLength(){ return  20.0 * k_cm; }
		static double ccalFaceZ()  { return  1279.376 * k_cm; }
	
		static double ccalMidplane() { return ccalFaceZ() + 0.5 * blockLength() ; } 
	
		bool isBlockActive( int row, int column ) const;
		int  numActiveBlocks() const { return m_numActiveBlocks; }

	
		DVector2 positionOnFace( int row, int column ) const;
		DVector2 positionOnFace( int channel ) const;
	
		int channel( int row, int column ) const;

		int row   ( int channel ) const { return m_row[channel];    }
		int column( int channel ) const { return m_column[channel]; }
	
		// get row and column from x and y positions
		int row   ( float y ) const;
		int column( float x ) const;

		void toStrings(vector<pair<string,string> > &items)const{
		  AddString(items, "kCCALBlocksWide", "%d", (int) kCCALBlocksWide);
		  AddString(items, "kCCALBlocksTall", "%d", (int) kCCALBlocksTall);
		  AddString(items, "kCCALMaxChannels", "%d",(int) kCCALMaxChannels);
		  AddString(items, "kCCALBeamHoleSize", "%2.3f",(int) kCCALBeamHoleSize);
		}
	
	private:

		bool   m_activeBlock[kCCALBlocksTall][kCCALBlocksWide];
		DVector2 m_positionOnFace[kCCALBlocksTall][kCCALBlocksWide];

		int    m_channelNumber[kCCALBlocksTall][kCCALBlocksWide];
		int    m_row[kCCALMaxChannels];
		int    m_column[kCCALMaxChannels];
	
		int    m_numActiveBlocks;
};

#endif // _DCCALGeometry_

