#ifndef _DFactoryGenerator_evio_writer_
#define _DFactoryGenerator_evio_writer_

#include <JANA/Compatibility/jerror.h>
#include <JANA/JFactoryGenerator.h>

#include "DEventWriterEVIO_factory.h"

class DFactoryGenerator_evio_writer : public JFactoryGenerator
{
	public:
		DFactoryGenerator_evio_writer(){}
		virtual ~DFactoryGenerator_evio_writer(){}
		virtual const char* className(void){return static_className();}
		static const char* static_className(void){return "DFactoryGenerator_evio_writer";}
		
		void GenerateFactories(JFactorySet* fs)
		{
			fs->Add(new DEventWriterEVIO_factory());
		}
};

#endif // _DFactoryGenerator_evio_writer_

