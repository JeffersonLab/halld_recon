#!/usr/bin/env python3

"""Test script for the KINFITTER libary

This script demonstrates creating, configuring, and running a
DKinFitter instance using the standalone KINFITTER shared library via
PyROOT. It constructs particles with bogus data, builds constraints,
runs a kinematic fit, and prints the fit result.
"""


import ROOT


if __name__ == "__main__":
  ROOT.gROOT.SetBatch(True)  # do not start an interactive ROOT session
  # add search paths for includes
  ROOT.gInterpreter.AddIncludePath("$JANA_HOME/include")
  ROOT.gInterpreter.AddIncludePath("$HALLD_RECON_HOME/$BMS_OSNAME/include")
  print(f"{ROOT.gInterpreter.GetIncludePath()=}")
  # set search paths for libraries
  ROOT.gSystem.AddDynamicPath("$JANA_HOME/lib")
  ROOT.gSystem.AddDynamicPath("$HALLD_RECON_HOME/$BMS_OSNAME/lib")
  print(f"{ROOT.gSystem.GetDynamicPath()=}")
  # load the shared library
  ROOT.gInterpreter.Declare("#define _KINFITTER_STANDALONE_")
  ROOT.gInterpreter.ProcessLine('#include "KINFITTER/DKinFitUtils_StandAlone.h"')
  assert ROOT.gSystem.Load("libKINFITTER.so") >= 0, "Could not load 'libKINFITTER.so'"

  # construct utility assuming files for the magnetic field maps are in the same directory
  # to create the field-map files, the `field_map_writer` plugin in `src/plugins/Utilities` has to be run over some data
  kinFitUtils = ROOT.DKinFitUtils_StandAlone("./field_map_coarseMesh.msgpack", "./field_map_fineMesh.msgpack")

  # construct particles and fill in bogus data
  # with these data, the fit will run but not converge
  covMatrix = ROOT.TMatrixFSym(7)
  for i in range(7):
    covMatrix[i][i] = 1
  covMatrixPtr = ROOT.std.make_shared[ROOT.TMatrixFSym](covMatrix)
  covMatrixPtr.Print()
  spaceTimeVertex = ROOT.TLorentzVector(0, 0, 0, 0)
  momentum = ROOT.TVector3(0, 0, 1)
  targetParticle = kinFitUtils.Make_TargetParticle(0, +1, 1)
  beamParticle = kinFitUtils.Make_BeamParticle(0, 0, 0, spaceTimeVertex, momentum, covMatrixPtr)
  detectedParticle = kinFitUtils.Make_DetectedParticle(0, 0, 0, spaceTimeVertex, momentum, 1, covMatrixPtr)
  missingParticle = kinFitUtils.Make_MissingParticle(0, +1, 1)

  # construct sets of particles
  initialParticles = ROOT.std.set[ROOT.std.shared_ptr[ROOT.DKinFitParticle]]()
  initialParticles.insert(targetParticle)
  initialParticles.insert(beamParticle)
  finalParticles = ROOT.std.set[ROOT.std.shared_ptr[ROOT.DKinFitParticle]]()
  finalParticles.insert(detectedParticle)
  finalParticles.insert(missingParticle)
  P4Constraint = kinFitUtils.Make_P4Constraint(initialParticles, finalParticles)
  fullConstraintParticles = ROOT.std.set[ROOT.std.shared_ptr[ROOT.DKinFitParticle]]()
  fullConstraintParticles.insert(detectedParticle)
  fullConstraintParticles.insert(missingParticle)
  noConstraintParticles = ROOT.std.set[ROOT.std.shared_ptr[ROOT.DKinFitParticle]]()
  noConstraintParticles.insert(targetParticle)
  noConstraintParticles.insert(beamParticle)
  vertexConstraint = kinFitUtils.Make_VertexConstraint(fullConstraintParticles ,noConstraintParticles, ROOT.TVector3(0, 0, 0))

  # construct and run fitter
  kinFitter = ROOT.DKinFitter(kinFitUtils)
  kinFitter.Reset_NewEvent()
  kinFitter.Reset_NewFit()
  kinFitter.Add_Constraint(P4Constraint)
  kinFitter.Add_Constraint(vertexConstraint)
  fitSuccess = kinFitter.Fit_Reaction()

  # print fit result
  print(f"{fitSuccess=}")
  print(f"{kinFitter.Get_ConfidenceLevel()=}")
  print(f"{kinFitter.Get_ChiSq()=}")
  print(f"{kinFitter.Get_NDF()=}")
  kinFitParticles = set(kinFitter.Get_KinFitParticles())
  for kinFitParticle in kinFitParticles:
    print(f"{kinFitParticle.Get_KinFitParticleType()=}")
    print(f"{kinFitParticle.Get_PID()=}")
    kinFitParticle.Get_Position().Print()
    kinFitParticle.Get_P4().Print()
