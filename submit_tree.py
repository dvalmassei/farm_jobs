import sys
import os
import subprocess
import time

qsimDir = "/w/halla-scshelf2102/moller12gev/dvalmass/QSIM/"
sourceDir = qsimDir + "alma9/qsim-moller/build"
eventFile = qsimDir + "remoll_lam_epelastic_100MEv.root"

config = "LAM_qsimmoller_gamma_2half_6halfdeg_1PMT_06222026_0"
generator = "beam"


runrange= range(int(sys.argv[1]),int(sys.argv[2]))

outDir="/lustre24/expphy/volatile/halla/moller12gev/dvalmass/sim_out/"+config
jobs=outDir+"/jobs"

if not os.path.exists(outDir):
  os.system("mkdir "+outDir)
if not os.path.exists(jobs):
  os.system("mkdir "+jobs)

runjobs = sourceDir + "/jobs"

if not os.path.exists(runjobs):
  os.system("mkdir "+runjobs)

for i in runrange:
  jsubf=open(runjobs + "/test_"+str(i)+".mac","w")
  jsubf.write("#/qsim/fDetMode 2\n")
  jsubf.write("#/qsim/fQMode 0\n")
  jsubf.write("#/qsim/fStandMode 0\n")
  jsubf.write("/qsim/fSourceMode 2\n")  #0=cosmics, 1=beam
  jsubf.write("/qsim/evgenfilename " + eventFile +"\n")
  jsubf.write("#/qsim/fDetAngle 0 deg\n")
  jsubf.write("#/qsim/fDetPosX 0 cm\n")
  jsubf.write("#/qsim/fDetPosY 0 cm\n")
  jsubf.write("/run/initialize\n")
  jsubf.write("/process/optical/boundary/setInvokeSD true\n")
  jsubf.write("/qsim/filename "+outDir+"/test_"+str(i)+".root\n")
  #jsubf.write("/qsim/seed 50\n")
  jsubf.write("/gun/particle gamma\n")
  jsubf.write("/run/beamOn 500000\n")


for i in runrange:
  #outfile=outDir+"/test_"+generator+"_"+str(i)+".root"
  jsubf=open(jobs+"/test_"+str(i)+".sh", "w")
  jsubf.write("#!/bin/bash\n")
  jsubf.write("#SBATCH --account=halla\n")
  jsubf.write("#SBATCH --partition=production\n")
  jsubf.write("#SBATCH --job-name=TEST\n")
  jsubf.write("#SBATCH --time=12:00:00\n")
  jsubf.write("#SBATCH --nodes=1\n")
  jsubf.write("#SBATCH --ntasks=1\n")
  jsubf.write("#SBATCH --cpus-per-task=1\n")
  jsubf.write("#SBATCH --mem=2G\n")
  jsubf.write("#tcsh -c \"source /apps/root/6.18.00/setroot_CUE\"\n")
  jsubf.write("cd "+sourceDir+"\n")
  jsubf.write("echo \"Current working directory is `pwd`\"\n")	
  jsubf.write("./qsim gdmlfiles_2.0/LAM_final.gdml "+runjobs+"/test_"+str(i)+".mac" +"\n")
  print("sbatch "+jobs+"/test_"+str(i)+".sh") 
