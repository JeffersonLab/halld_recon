// $Id$
//
//    File: DFactoryGeneratorLSLM.h
// Created: Wed Jan 14 11:18:00 EST 2009
// Creator: davidl (on Darwin Harriet.local 9.6.0 i386)
//

#ifndef _DFactoryGeneratorLSLM_
#define _DFactoryGeneratorLSLM_

#include <JANA/JFactoryGenerator.h>

#include "DTrackFitter_factory_LSLM.h"

class DFactoryGeneratorLSLM: public JFactoryGenerator{
	public:
		DFactoryGeneratorLSLM(){}
		virtual ~DFactoryGeneratorLSLM(){}
		virtual const char* className(void){return static_className();}
		static const char* static_className(void){return "DFactoryGeneratorLSLM";}
		
		void GenerateFactories(JFactorySet* fs){
			fs->Add(new DTrackFitter_factory_LSLM());
		}

	protected:
	
	
	private:

};

#endif // _DFactoryGeneratorLSLM_

