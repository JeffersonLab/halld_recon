// $Id$
//
//    File: JEventProcessor_field_map_writer.h
// Created: Wed Aug 20 05:53:12 PM EDT 2025
// Creator: bgrube (on Linux ifarm2402.jlab.org 5.14.0-570.33.2.el9_6.x86_64 x86_64)
//
// trivial plugin with the sole purpose of writing magnetic field maps into binary JSON files
// there is probably a better way to do this...

/// For more information on the syntax changes between JANA1 and JANA2, visit: https://jeffersonlab.github.io/JANA2/#/jana1to2/jana1-to-jana2

#ifndef _JEventProcessor_field_map_writer_
#define _JEventProcessor_field_map_writer_

#include <JANA/JEventProcessor.h>
#include <string>


class JEventProcessor_field_map_writer : public JEventProcessor
{
	public:
		JEventProcessor_field_map_writer();
		~JEventProcessor_field_map_writer();
		const char* className(void) { return "JEventProcessor_field_map_writer"; }

	private:
		void Init() override;                                                ///< Called once at program start.
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;  ///< Called everytime a new run number is detected.
		void Process (const std::shared_ptr<const JEvent>& event) override;  ///< Called every event.
		void EndRun() override;                                              ///< Called everytime run number changes, provided BeginRun has been called.
		void Finish() override;                                              ///< Called after last event of last event source has been processed.

		std::string OUTPUT_BASE_NAME     = "./field_map";  ///< Base name for output files
		bool        STOP_AFTER_FIRST_RUN = true;           ///< Stop processing after the first run
};

#endif // _JEventProcessor_field_map_writer_
