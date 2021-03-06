#include <TH1D.h>
#include <TH1.h>
#include <TFile.h>
#include <TString.h>
#include <TROOT.h>
#include <TCanvas.h>
#include <THStack.h>
#include <TPad.h>
#include <TMath.h>
#include <TMinuit.h>

#include <iostream>
#include <sstream>

const int NPTBINS=5;
const float PTBINS[NPTBINS+1] = { 50, 70, 90, 130, 200, 600 };

enum hist_t { Real, Fake, Truth, Numerator, Denominator };

const int NEBBINS=9;
const double EBBINS[NEBBINS+1] = { 0., 0.0075, 0.009, 0.0105, 0.0120, 0.0135, 0.0150, 0.0165, 0.0180, 0.03};
const int EBSIEIECUTBIN=4;

const int NEEBINS=8;
const double EEBINS[NEEBINS+1] = { 0., 0.016, 0.020, 0.024, 0.028, 0.032, 0.036, 0.040, 0.08 };
const int EESIEIECUTBIN=5;

TFile* rootfile=0;

// histograms used by the fitter
TH1 *theData, *theBkg1, *theBkg2;


using namespace std;

TH1* GetHist(int ptbin, bool isEB, hist_t type)
{
  rootfile=0;
  if(type==Real)
    rootfile=new TFile("diphoton_fake_rate_real_templates_all_GGJets_GJets_76X_MiniAOD_histograms.root");
  else if(type==Truth)
    rootfile=new TFile("diphoton_fake_rate_closure_test_matching_all_samples_76X_MiniAOD_histograms.root");
  else
    rootfile=new TFile("diphoton_fake_rate_closure_test_all_samples_76X_MiniAOD_histograms.root");
  assert(rootfile);
  rootfile->cd();

  ostringstream oss;
  if(type==Real)
    oss << "sieie" << (isEB ? "EB" : "EE") << "_realtemplate_pt" << PTBINS[ptbin] << "To" << PTBINS[ptbin+1];
  else if(type==Fake)
    oss << "sieie" << (isEB ? "EB" : "EE") << "_faketemplate_pt" << PTBINS[ptbin] << "To" << PTBINS[ptbin+1] << "_chIso5To10";
  else if(type==Numerator)
    oss << "sieie" << (isEB ? "EB" : "EE") << "_numerator_pt" << PTBINS[ptbin] << "To" << PTBINS[ptbin+1];
  else if(type==Denominator)
    oss << "Pt" << (isEB ? "EB" : "EE") << "_denominator_pt" << PTBINS[ptbin] << "To" << PTBINS[ptbin+1];
  else if(type==Truth)
    oss << "sieie" << (isEB ? "EB" : "EE") << "_numerator_fakes_pt" << PTBINS[ptbin] << "To" << PTBINS[ptbin+1];
  
  TH1* h=dynamic_cast<TH1*>( gROOT->FindObject(oss.str().c_str()) );
  if(!h) {
    cout << "Cannot find object " << oss.str().c_str() << " (" << ptbin << ", " << isEB << ", " << type << ")" << endl;
  }
  assert(h);

  if(type!=Denominator) {
    if(isEB)
      h=h->Rebin(NEBBINS, TString(h->GetName())+"_rebin", EBBINS);
    else
      h=h->Rebin(NEEBINS, TString(h->GetName())+"_rebin", EEBINS);
  }
  
  return h;
}


// Negative log likelihood function for the fitting routine
void NLL(int &, double *, double &f, double *par, int)
{
  double frac=par[0];
  double norm1=theData->Integral()/theBkg1->Integral();
  double norm2=theData->Integral()/theBkg2->Integral();
  
  f=0.0;
  for(int i=1; i<=theData->GetNbinsX(); i++) {
    double obs=theData->GetBinContent(i);
    double mu=norm1*frac*theBkg1->GetBinContent(i)+norm2*(1-frac)*theBkg2->GetBinContent(i);
    double mue1=norm1*frac*theBkg1->GetBinError(i);
    double mue2=norm2*(1-frac)*theBkg2->GetBinError(i);
    double mue=sqrt(mue1*mue1+mue2*mue2)*par[i];
    mu+=mue;
    if(mu<=0.0) {
      mu=0.000001;
      std::cout << "Woah!  mu=" << mu << std::endl;
    }
    f += mu-obs*TMath::Log(mu)+par[i]*par[i]/2.;
  }
  return;
}

int performFit(TH1* hData, TH1* hBkg1, TH1* hBkg2, double &frac, double &frace)
{
  theData=hData;
  theBkg1=hBkg1;
  theBkg2=hBkg2;

  TMinuit minuit;
  minuit.SetErrorDef(0.5);
  minuit.SetPrintLevel(1);
  minuit.Command("SET STR 2"); // set strategy

  minuit.DefineParameter(0, "FRAC", 0.5, 0.01, 0.0, 1.0);
  for(int i=1; i<=theData->GetNbinsX(); i++) {
    ostringstream oss;
    oss << "BB" << i;
    minuit.DefineParameter(i, oss.str().c_str(), 0, 0.1, -7.0, 7.0);
  }

  minuit.SetFCN(NLL); // set the function
  double tmp[1] = {50000. }; int err;
  minuit.mnexcm("MIGRAD", tmp, 1, err); // execute Minuit with MIGRAD

  // get the parameters
  minuit.GetParameter(0, frac, frace);

  // get the the Minos errors
  double eplus, eminus, eparab, gcc;
  minuit.mnmnos();
  minuit.mnerrs(0, eplus, eminus, eparab, gcc);

  // set the error to the symmetrized minos errors
  frace=(eplus-eminus)/2.0;
  
  return err;
}

void fit(void)
{
  TCanvas *cEB=new TCanvas("EB","EB");
  TCanvas *cEE=new TCanvas("EE","EE");
  TCanvas *cEBTruth=new TCanvas("EBTruth","EB Truth");
  TCanvas *cEETruth=new TCanvas("EETruth","EE Truth");
  cEB->Divide(2,3);
  cEE->Divide(2,3);
  cEBTruth->Divide(2,3);
  cEETruth->Divide(2,3);

  double EBFake[NPTBINS], EBFakeE[NPTBINS];
  double EEFake[NPTBINS], EEFakeE[NPTBINS];
  int EBFakeStatus[NPTBINS], EEFakeStatus[NPTBINS];

  double EBTruth[NPTBINS], EBTruthE[NPTBINS];
  double EETruth[NPTBINS], EETruthE[NPTBINS];
  int EBTruthStatus[NPTBINS], EETruthStatus[NPTBINS];

  
  for(int isTruth=0; isTruth<2; isTruth++) {
    for(int isEB=0; isEB<2; isEB++) {
      for(int ptbin=0; ptbin<NPTBINS; ptbin++) {
	
	TH1* hReal=GetHist(ptbin, isEB, Real);
	TH1* hFake= isTruth ? GetHist(ptbin, isEB, Truth) : GetHist(ptbin, isEB, Fake);
	TH1* hNum=GetHist(ptbin, isEB, Numerator);
	TH1* hDem=GetHist(ptbin, isEB, Denominator);
	
	double frac, frace;
	int err=performFit(hNum, hReal, hFake, frac, frace);
	
	double norm1=hNum->Integral()/hReal->Integral();
	double norm2=hNum->Integral()/hFake->Integral();
	hReal->Scale(norm1*frac);
	hFake->Scale(norm2*(1-frac));
	hReal->SetFillColor(kBlue);
	hReal->SetFillStyle(3001);
	hFake->SetFillColor(kRed);
	hFake->SetFillStyle(3002);
	
	ostringstream oss;
	oss << "h" << PTBINS[ptbin] << "To" << PTBINS[ptbin+1] << "_" << (isEB ? "EB" : "EE") << "_" << (isTruth ? "Truth" : "Fake");
	THStack *hs=new THStack(oss.str().c_str(), "");
	hs->Add(hFake, "HIST");
	hs->Add(hReal, "HIST");
	
	if(isTruth) {
	  if(isEB) cEBTruth->cd(ptbin+1);
	  else     cEETruth->cd(ptbin+1);
	} else {
	  if(isEB) cEB->cd(ptbin+1);
	  else     cEE->cd(ptbin+1);
	}
	hs->Draw();
	hNum->Draw("same");
	gPad->SetLogy(1);
	hs->GetXaxis()->SetTitle("#sigma_{i#etai#eta}");
	
	double num=0.0, nume2=0.0;
	int nbins=isEB ? EBSIEIECUTBIN-1 : EESIEIECUTBIN-1;
	for(int i=1; i<=nbins; i++) {
	  num += hFake->GetBinContent(i);
	  nume2 += hFake->GetBinError(i)*hFake->GetBinError(i);
	}
	double error=sqrt(nume2/num/num+frace*frace/frac/frac)*num;
	double den=hDem->Integral();

	if(!isTruth) {
	  if(isEB) { EBFake[ptbin]=num/den; EBFakeE[ptbin]=error/den; EBFakeStatus[ptbin]=err; }
	  else     { EEFake[ptbin]=num/den; EEFakeE[ptbin]=error/den; EEFakeStatus[ptbin]=err; }
	} else {
	  if(isEB) { EBTruth[ptbin]=num/den; EBTruthE[ptbin]=error/den; EBTruthStatus[ptbin]=err; }
	  else     { EETruth[ptbin]=num/den; EETruthE[ptbin]=error/den; EETruthStatus[ptbin]=err; }
	}
      }
    }
  }

  for(int i=0; i<NPTBINS; i++)
    std::cout << "Fake EB " << i << ": " << EBFake[i] << " +/- " << EBFakeE[i] << " (" << EBFakeE[i]/EBFake[i]*100 << "%; status=" << EBFakeStatus[i] << ")" << std::endl;
  for(int i=0; i<NPTBINS; i++)
    std::cout << "Fake EE " << i << ": " << EEFake[i] << " +/- " << EEFakeE[i] << " (" << EEFakeE[i]/EEFake[i]*100 << "%; status=" << EEFakeStatus[i] << ")" << std::endl;
  for(int i=0; i<NPTBINS; i++)
    std::cout << "Truth EB " << i << ": " << EBTruth[i] << " +/- " << EBTruthE[i] << " (" << EBTruthE[i]/EBTruth[i]*100 << "%; status=" << EBTruthStatus[i] << ")" << std::endl;
  for(int i=0; i<NPTBINS; i++)
    std::cout << "Truth EE " << i << ": " << EETruth[i] << " +/- " << EETruthE[i] << " (" << EETruthE[i]/EETruth[i]*100 << "%; status=" << EETruthStatus[i] << ")" << std::endl;

  TCanvas *c1=new TCanvas("cEBRatio","EBRatio");
  TCanvas *c2=new TCanvas("cEERatio","EERatio");
  TH1D* hEBRatio=new TH1D("hEBRatio","EB Ratio", NPTBINS, PTBINS);
  TH1D* hEERatio=new TH1D("hEERatio","EE Ratio", NPTBINS, PTBINS);

  for(int i=0; i<NPTBINS; i++) {
    double r=EBFake[i]/EBTruth[i];
    double e1=EBFakeE[i]/EBFake[i];
    double e2=EBTruthE[i]/EBTruth[i];
    double e=r*sqrt(e1*e1+e2*e2);
    std::cout << "EB Ratio " << i << ": " << r  << " +/- " << e << std::endl;
    hEBRatio->SetBinContent(i+1, r);
    hEBRatio->SetBinError(i+1, e);
  }
  for(int i=0; i<NPTBINS; i++) {
    double r=EEFake[i]/EETruth[i];
    double e1=EEFakeE[i]/EEFake[i];
    double e2=EETruthE[i]/EETruth[i];
    double e=r*sqrt(e1*e1+e2*e2);
    std::cout << "EE Ratio " << i << ": " << r  << " +/- " << e << std::endl;
    hEERatio->SetBinContent(i+1, r);
    hEERatio->SetBinError(i+1, e);
  }

  c1->cd(); hEBRatio->Draw();
  c2->cd(); hEERatio->Draw();

  hEBRatio->SetMinimum(0);  hEERatio->SetMinimum(0);
  hEBRatio->SetMaximum(2.0);  hEERatio->SetMaximum(2.0);
  return;
}
