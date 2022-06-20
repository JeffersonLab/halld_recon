

#ifndef _HDDVIEW_H_
#define _HDDVIEW_H_

#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
using namespace std;

#include <TApplication.h>
#include <TCanvas.h>

#include <JANA/JEventLoop.h>
#include <JANA/jerror.h>
#include "DANA/DApplication.h"
#include "MyProcessor.h"

extern TCanvas *maincanvas;
extern DApplication *dapp;
extern JEventLoop *eventloop;
extern MyProcessor *myproc;

extern int32_t RUNNUMBER;

extern std::vector< std::string > REQUIRED_CLASSES_FOR_DRAWING;


jerror_t hdv_getevent(void);
jerror_t hdv_drawevent(void);

#endif //_HDDVIEW_H_
