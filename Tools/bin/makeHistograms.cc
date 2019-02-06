#include "diphoton-analysis/Tools/interface/sampleList.hh"
#include "diphoton-analysis/Tools/interface/utilities.hh"

#include "TCanvas.h"
#include "TString.h"
#include "TH1.h"
#include "TFile.h"

void allSamples(const std::string &region, const std::string &year, TFile * output);
std::string getSampleBase(const std::string & sampleName, const std::string & year);
std::string getBase(const std::string & sampleName);

int main(int argc, char *argv[])
{
  std::string region, year;

  if(argc!=3) {
    std::cout << "Syntax: makeHistograms.exe [BB/BE] [2016/2017/2018]" << std::endl;
      return -1;
  }
  else {
    region = argv[1];
    if(region!="BB" and region!="BE") {
      std::cout << "Only 'BB' and 'BE' are allowed regions. " << std::endl;
      return -1;
    }
    year = argv[2];
    if(year!="2016" and year!="2017" and year!="2018") {
      std::cout << "Only '2016', 2017' and '2018' are allowed years. " << std::endl;
      return -1;
    }
  }

  init();

  TFile *output = new TFile(Form("datacards/Minv_histos_%s_%s.root", region.c_str(), year.c_str()), "recreate");
  output->mkdir(region.c_str());
  TFile *fakes = new TFile(Form("data/fakes_%s_average.root", year.c_str()));
  fakes->Print("v");
  TH1F *fakeHist = static_cast<TH1F*>(fakes->Get(Form("%s/gj", region.c_str())));
  output->cd(region.c_str());
  fakeHist->Write();
  allSamples(region, year, output);
  output->Write();
  output->Close();

}

void allSamples(const std::string &region, const std::string &year, TFile * output)
{

  int nBins = 120;
  double xMin = 0.0;
  double xMax = 6000.;

  std::map<std::string, std::string> cuts;
  cuts["BB"] = "isGood*(Diphoton.Minv > 500 && Diphoton.deltaR > 0.45 && Photon1.pt>125 && Photon2.pt>125 && Photon1.isEB && Photon2.isEB)";
  cuts["BE"] = "isGood*(Diphoton.Minv > 500 && Diphoton.deltaR > 0.45 && Photon1.pt>125 && Photon2.pt>125 && ( (Photon1.isEB && Photon2.isEE) || (Photon2.isEB &&  Photon1.isEE )))";

  std::vector<std::string> samples = getSampleList();

  // add scale variations
  samples.push_back("gg_R2F2_2016");
  samples.push_back("gg_R2F2_2017");
  samples.push_back("gg_R2F2_2018");
  samples.push_back("gg_R0p5F0p5_2016");
  samples.push_back("gg_R0p5F0p5_2017");
  samples.push_back("gg_R0p5F0p5_2018");


  std::vector<int> stringScales = {3000, 3500, 4000, 4500, 5000, 5500, 6000};

  for(auto isample : samples) {
    std::cout << isample << std::endl;
  }
  std::map<std::string, TH1F*> histograms;
  for(auto isample : samples) {
    std::string sampleCut = cuts[region];
    // skip the Sherpa GEN trees
    if( isample.compare("ggGen") == 0) continue;
    if( isample.find("unskimmed") != std::string::npos ) continue;
    // use data-driven prediction instead
    if( isample.find("gj") != std::string::npos ) continue;
    if( isample.find(year) == std::string::npos && isample.find("ADD") == std::string::npos ) continue;
    // apply weights for all samples except data
    if( isample.find("data") == std::string::npos ) sampleCut+="*weightAll*" + std::to_string(luminosity[year]);
    else if (isample.find("data_2015") != std::string::npos || isample.find("data_2016") != std::string::npos) sampleCut += "*(HLT_DoublePhoton60>0)";
    else sampleCut += "*(HLT_DoublePhoton70>0)";
    // apply k-factor to Sherpa GG sample
    if( isample.find("gg_R2F2_") != std::string::npos) sampleCut += "*" + kfactorString(region, "R2F2");
    else if( isample.find("gg_R0p5F0p5_") != std::string::npos) sampleCut += "*" + kfactorString(region, "R0p5F0p5");
    else if( isample.find("gg_") != std::string::npos) sampleCut += "*" + kfactorString(region, "R1F1");
    std::cout << "Making histograms for sample " << isample << " with cut\n" << sampleCut << std::endl;
    std::string baseName(getSampleBase(isample, year));
    TH1F *hist = new TH1F(baseName.c_str(), baseName.c_str(), nBins, xMin, xMax);
    histograms[baseName] = hist;
    std::cout << "Making histograms for sample " << hist->GetName() << " with cut\n" << sampleCut << std::endl;
    chains[getBase(isample)]->Project(baseName.c_str(), "Diphoton.Minv",  sampleCut.c_str());
    std::cout << "Integral: " << histograms[baseName]->Integral() << std::endl;
    output->cd(region.c_str());
  }

  for(auto histogram : histograms) {
    std::string title(histogram.second->GetTitle());
    if(title.find("ADD") != std::string::npos) histogram.second->Add(histograms["gg70"], -1);
    histogram.second->Write();
  }

}

// remove year
std::string getSampleBase(const std::string & sampleName, const std::string & year)
{
  std::string newString(sampleName);
  if( sampleName.find("_201") != std::string::npos) {
    newString.replace(newString.find("_201"), 5, "");
  }
  // "data_obs" is always the name of the data observation histogram
  std::string data("data_" + year);
  if( sampleName.compare(data) == 0) newString = "data_obs";
  return newString;
}

// ignore variations to get dataset name
std::string getBase(const std::string & sampleName)
{
  if(sampleName.compare("gg_R2F2_2016") == 0 ) return "gg_2016";
  if(sampleName.compare("gg_R0p5F0p5_2016") == 0 ) return "gg_2016";
  if(sampleName.compare("gg_R2F2_2017") == 0 ) return "gg_2017";
  if(sampleName.compare("gg_R0p5F0p5_2017") == 0 ) return "gg_2017";
  if(sampleName.compare("gg_R2F2_2018") == 0 ) return "gg_2018";
  if(sampleName.compare("gg_R0p5F0p5_2018") == 0 ) return "gg_2018";
  return sampleName;
}
