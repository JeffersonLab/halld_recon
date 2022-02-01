
#include <TRACKING/DReferenceTrajectory.h>

#ifndef _DREFERENCETRAJECTORY_H_
#define _DREFERENCETRAJECTORY_H_

class DReferenceTrajectoryHDV: public DReferenceTrajectory{

    public:
        DReferenceTrajectoryHDV(const DMagneticFieldMap *bfield
                                        , double q=1.0
                                        , swim_step_t *swim_steps=NULL
                                        , int max_swim_steps=0
                                        , double step_size=-1.0):  DReferenceTrajectory(bfield, q, swim_steps,max_swim_steps,step_size){}
        //DReferenceTrajectoryHDV(const DMagneticFieldMap *bfield) :  DReferenceTrajectory(bfield){}
        DReferenceTrajectoryHDV(const DReferenceTrajectory& rt) :  DReferenceTrajectory(rt){}

        void SetZmaxTrackBoundary(double zmax_track_boundary){ this->zmax_track_boundary = zmax_track_boundary; }
};

#endif //_DREFERENCETRAJECTORY_H_


