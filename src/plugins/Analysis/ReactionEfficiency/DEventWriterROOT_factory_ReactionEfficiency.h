// $Id$
//
//    File: DEventWriterROOT_factory_ReactionEfficiency.h
// Created: Sat Jun 10 10:20:19 EDT 2023
// Creator: jrsteven (on Linux ifarm1901.jlab.org 3.10.0-1160.90.1.el7.x86_64 x86_64)
//

#ifndef _DEventWriterROOT_factory_ReactionEfficiency_
#define _DEventWriterROOT_factory_ReactionEfficiency_

#include <JANA/JFactory.h>
#include <JANA/JEventLoop.h>

#include "DEventWriterROOT_ReactionEfficiency.h"

class DEventWriterROOT_factory_ReactionEfficiency : public jana::JFactory<DEventWriterROOT>
{
	public:
		DEventWriterROOT_factory_ReactionEfficiency(){use_factory = 1;}; //prevents JANA from searching the input file for these objects
		~DEventWriterROOT_factory_ReactionEfficiency(){};
		const char* Tag(void){return "ReactionEfficiency";}

		DEventWriterROOT_ReactionEfficiency *dROOTEventWriter = nullptr;

        private:

                //------------------
                // brun
                //------------------
                jerror_t brun(JEventLoop *loop, int32_t runnumber)
                {
                        // (See DTAGHGeometry_factory.h)
                        SetFactoryFlag(NOT_OBJECT_OWNER);
                        ClearFactoryFlag(WRITE_TO_OUTPUT);

                        if( dROOTEventWriter == nullptr ) {
                                dROOTEventWriter = new DEventWriterROOT_ReactionEfficiency();
                                dROOTEventWriter->Initialize(loop);
				dROOTEventWriter->Run_Update_Custom(loop);
                        } else {
                                dROOTEventWriter->Run_Update(loop);
				dROOTEventWriter->Run_Update_Custom(loop);
                        }

                        return NOERROR;
                }

                //------------------
                // evnt
                //------------------
                 jerror_t evnt(JEventLoop *loop, uint64_t eventnumber)
                 {
                        // Reuse existing DBCALGeometry object.
                        if( dROOTEventWriter ) _data.push_back( dROOTEventWriter );

                        return NOERROR;
                 }


		 //------------------
                // fini
                //------------------
                jerror_t fini(void)
                {
                        // Delete object: Must be "this" thread so that interfaces deleted properly
                        delete dROOTEventWriter;
                        return NOERROR;
                }
};

#endif // _DEventWriterROOT_factory_ReactionEfficiency_
