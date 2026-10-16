// $Id$
//
//    File: JEventProcessor_bcal_calib_cosmic_cdc.h
// Created: Tue Jul  1 13:11:51 EDT 2014
// Creator: dalton (on Linux gluon05.jlab.org 2.6.32-358.18.1.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_bcal_calib_cosmic_cdc_
#define _JEventProcessor_bcal_calib_cosmic_cdc_

#include <JANA/JEventProcessor.h>
#include "TTree.h"
#include "BCAL/DBCALGeometry.h"

// Doxygen documentation
/** 
This class is used to project stright lines from cosmic rays in the CDC lines to the BCAL for the purposes of calibrating the BCAL.


*/

class JEventProcessor_bcal_calib_cosmic_cdc:public JEventProcessor{
	public:
		JEventProcessor_bcal_calib_cosmic_cdc();
		~JEventProcessor_bcal_calib_cosmic_cdc();

		TTree *bcal_calib_cosmic_cdc_tree;
		int eventnum;
		int cell; 
		int tlayer;
		int tmodule;
		int tsector;
		int tglobalsect;
		int numcells; ///< Number of BCAL cells intersected by the track
		float tdist;  
		float use;  
		float dse;  
		float track_m;
  		float track_c;
		float chisq;
		int Ndof;

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;

		/// Command Line Parameters
		int VERBOSE;
};

#endif // _JEventProcessor_bcal_calib_cosmic_cdc_

