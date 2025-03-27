

#ifndef _HDDVIEW_H_
#define _HDDVIEW_H_

#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
using namespace std;

#include <TApplication.h>
#include <TCanvas.h>

#include <JANA/JEvent.h>
#include <DANA/jerror.h>
#include "DANA/DApplication.h"
#include "MyProcessor.h"

extern TCanvas *maincanvas;
extern DApplication *dapp;
extern JEvent *jevent;
extern MyProcessor *myproc;

extern int32_t RUNNUMBER;

enum REQUIRED_CLASSES_LOGIC_t{
	REQUIRED_CLASSES_LOGIC_OR,
	REQUIRED_CLASSES_LOGIC_AND
};
extern std::vector< std::string > REQUIRED_CLASSES_FOR_DRAWING;
extern REQUIRED_CLASSES_LOGIC_t REQUIRED_CLASSES_LOGIC;



jerror_t hdv_getevent(void);
jerror_t hdv_drawevent(void);

#endif //_HDDVIEW_H_
