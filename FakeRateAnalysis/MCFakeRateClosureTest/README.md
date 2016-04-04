# MC Fake Rate Closure Test

## Setup env

export SCRAM_ARCH=slc6_amd64_gcc493 (bash)  
setenv SCRAM_ARCH slc6_amd64_gcc493 (tcsh)

cmsrel CMSSW_7_6_3_patch2  
cd CMSSW_7_6_3_patch2/src  
cmsenv  
git clone git@github.com:cms-exotica-diphotons/diphoton-analysis  
cd diphoton-analysis   
cd FakeRateAnalysis/MCFakeRateClosureTest  

## Scripts

Merge ntuples from scripts directory:  
./hadd_script.sh

Make class from analysis directory:  
root ../scripts/diphoton_makeClass.C