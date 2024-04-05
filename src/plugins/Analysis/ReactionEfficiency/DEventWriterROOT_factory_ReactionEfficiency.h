// $Id$
//
//    File: DEventWriterROOT_factory_ReactionEfficiency.h
// Created: Sat Jun 10 10:20:19 EDT 2023
// Creator: jrsteven (on Linux ifarm1901.jlab.org 3.10.0-1160.90.1.el7.x86_64 x86_64)
//

#ifndef _DEventWriterROOT_factory_ReactionEfficiency_
#define _DEventWriterROOT_factory_ReactionEfficiency_

#include <JANA/JFactoryT.h>

#include "DEventWriterROOT_ReactionEfficiency.h"

class DEventWriterROOT_factory_ReactionEfficiency : public JFactoryT<DEventWriterROOT>
{
	public:
		DEventWriterROOT_factory_ReactionEfficiency(){
                        //use_factory = 1; //prevents JANA from searching the input file for these objects
                        }; 
		~DEventWriterROOT_factory_ReactionEfficiency(){};
		const char* Tag(void){return "ReactionEfficiency";}

		DEventWriterROOT_ReactionEfficiency *dROOTEventWriter = nullptr;

        private:

                //------------------
                // BeginRun
                //------------------
                void BeginRun(const std::shared_ptr<const JEvent>& event) override
                {
                        // (See DTAGHGeometry_factory.h)
                        SetFactoryFlag(NOT_OBJECT_OWNER);
                        ClearFactoryFlag(WRITE_TO_OUTPUT);

                        if( dROOTEventWriter == nullptr ) {
                                dROOTEventWriter = new DEventWriterROOT_ReactionEfficiency();
                                dROOTEventWriter->Initialize(event);
				dROOTEventWriter->Run_Update_Custom(event);
                        } else {
                                dROOTEventWriter->Run_Update(event);
				dROOTEventWriter->Run_Update_Custom(event);
                        }

                        return;
                }

                //------------------
                // Process
                //------------------
                void Process(const std::shared_ptr<const JEvent>& event) override
                 {
                        // Reuse existing DBCALGeometry object.
                        if( dROOTEventWriter ) Insert( dROOTEventWriter );

                        return;
                 }


		 //------------------
                // Finish
                //------------------
                void Finish() override
                {
                        // Delete object: Must be "this" thread so that interfaces deleted properly
                        delete dROOTEventWriter;
                        return;
                }
};

#endif // _DEventWriterROOT_factory_ReactionEfficiency_
