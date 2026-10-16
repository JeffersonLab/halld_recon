// Author: Elliott Wolin 19-Mar-2010



#ifndef _DDANAEVIOFactoryGenerator_
#define _DDANAEVIOFactoryGenerator_


#include "JANA/JFactoryGenerator.h"



class DDANAEVIOFactoryGenerator : public JFactoryGenerator {

 public:

  DDANAEVIOFactoryGenerator() {};

  virtual ~DDANAEVIOFactoryGenerator() {};

  virtual const char* className(void) { return static_className(); }
  static const char* static_className(void) { return "DDANAEVIOFactoryGenerator"; }
  

  // this method dose the work
  void GenerateFactories(JFactorySet* fs);

};

#endif // _DDANAEVIOFactoryGenerator_

