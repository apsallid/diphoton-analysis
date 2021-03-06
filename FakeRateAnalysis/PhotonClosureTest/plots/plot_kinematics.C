// compare MC trugh kinematics against denominator with and without fake rate applied
int plot_fakes(TFile *f_all, TFile *f_fakes, TString name) {
  
  TCanvas *c = new TCanvas("c","",1500,700);
  c->Divide(2,1);

  // EB
  TH1D *h_all_EB = (TH1D *) f_all->Get("photon_"+name+"_denominator_fake_rate_weighted_EB");
  TH1D *h_fakes_EB = (TH1D *) f_fakes->Get("photon_fakes_"+name+"_EB");
  TH1D *h_unweighted_EB = (TH1D *) f_all->Get("photon_"+name+"_denominator_EB");
  TLegend *l_EB = new TLegend(0.55,0.75,0.85,0.85);
  l_EB->SetBorderSize(0);
  l_EB->SetFillColor(0);
  l_EB->AddEntry(h_unweighted_EB,"No fake rate applied","ep");
  l_EB->AddEntry(h_fakes_EB,"MC truth","ep");
  l_EB->AddEntry(h_all_EB,"Fake rate applied","ep");
  c->cd(1);
  h_unweighted_EB->Draw();
  h_unweighted_EB->SetMarkerColor(kBlack);
  h_unweighted_EB->SetLineColor(kBlack);
  h_all_EB->Draw("same");
  h_all_EB->SetMarkerColor(kBlue);
  h_all_EB->SetLineColor(kBlue);
  h_fakes_EB->Draw("same");
  h_fakes_EB->SetMarkerColor(kRed);
  h_fakes_EB->SetLineColor(kRed);
  h_unweighted_EB->SetTitle("EB");
  h_unweighted_EB->SetMaximum(max(h_unweighted_EB->GetMaximum()*1.4,max(h_all_EB->GetMaximum()*1.4,h_fakes_EB->GetMaximum()*1.4)));
  if (name == "pt") {
    h_unweighted_EB->GetXaxis()->SetTitle("p_{T} (GeV)");
    gPad->SetLogy();
  }
  if (name == "eta") h_unweighted_EB->GetXaxis()->SetTitle("#eta");
  if (name == "phi") h_unweighted_EB->GetXaxis()->SetTitle("#phi");
  if (name == "phi" || name == "eta") {
    h_unweighted_EB->GetXaxis()->CenterTitle();
    h_unweighted_EB->SetMinimum(0.);
  }
  l_EB->Draw();
  
  // EE
  TH1D *h_all_EE = (TH1D *) f_all->Get("photon_"+name+"_denominator_fake_rate_weighted_EE");
  TH1D *h_fakes_EE = (TH1D *) f_fakes->Get("photon_fakes_"+name+"_EE");
  TH1D *h_unweighted_EE = (TH1D *) f_all->Get("photon_"+name+"_denominator_EE");
  TLegend *l_EE = new TLegend(0.55,0.75,0.85,0.85);
  l_EE->SetBorderSize(0);
  l_EE->SetFillColor(0);
  l_EE->AddEntry(h_unweighted_EE,"No fake rate applied","ep");
  l_EE->AddEntry(h_fakes_EE,"MC truth","ep");
  l_EE->AddEntry(h_all_EE,"Fake rate applied","ep");
  c->cd(2);
  h_unweighted_EE->Draw();
  h_unweighted_EE->SetMarkerColor(kBlack);
  h_unweighted_EE->SetLineColor(kBlack);
  h_all_EE->Draw("same");
  h_all_EE->SetMarkerColor(kBlue);
  h_all_EE->SetLineColor(kBlue);
  h_fakes_EE->Draw("same");
  h_fakes_EE->SetMarkerColor(kRed);
  h_fakes_EE->SetLineColor(kRed);
  h_unweighted_EE->SetTitle("EE");
  h_unweighted_EE->SetMaximum(max(h_unweighted_EB->GetMaximum()*1.4,max(h_all_EE->GetMaximum()*1.4,h_fakes_EE->GetMaximum()*1.4)));
  if (name == "pt") {
    h_unweighted_EE->GetXaxis()->SetTitle("p_{T} (GeV)");
    gPad->SetLogy();
  }
  if (name == "eta") h_unweighted_EE->GetXaxis()->SetTitle("#eta");
  if (name == "phi") h_unweighted_EE->GetXaxis()->SetTitle("#phi");
  if (name == "phi" || name == "eta") {
    h_unweighted_EE->GetXaxis()->CenterTitle();
    h_unweighted_EE->SetMinimum(0.);
  }
  l_EE->Draw();
  
  c->SaveAs("kinematics_"+name+"_weighted_denom_compared_to_matched_fakes.pdf");
  
  delete c;
  
  return 0;
}

// compare kinematics against weighted (fake rate applied) denominator and unweighted denominator
int plot_unweighted(TFile *f_all, TFile *f_fakes, TString name) {

  TCanvas *c = new TCanvas("c","",1500,700);
  c->Divide(2,1);

  // EB
  TH1D *h_all_EB = (TH1D *) f_all->Get("photon_"+name+"_denominator_fake_rate_weighted_EB");
  TH1D *h_unweighted_EB = (TH1D *) f_all->Get("photon_"+name+"_denominator_EB");
  TLegend *l_EB = new TLegend(0.55,0.75,0.85,0.85);
  l_EB->SetBorderSize(0);
  l_EB->SetFillColor(0);
  l_EB->AddEntry(h_all_EB,"Fake rate applied","ep");
  l_EB->AddEntry(h_unweighted_EB,"No fake rate applied","ep");
  c->cd(1);
  h_all_EB->Draw();
  h_unweighted_EB->Draw("same");
  h_unweighted_EB->SetMarkerColor(kRed);
  h_all_EB->SetTitle("EB");
  h_all_EB->SetMaximum(max(h_all_EB->GetMaximum()*1.4,h_unweighted_EB->GetMaximum()*1.4));
  if (name == "pt") {
    h_all_EB->GetXaxis()->SetTitle("p_{T} (GeV)");
    gPad->SetLogy();
  }
  if (name == "eta") h_all_EB->GetXaxis()->SetTitle("#eta");
  if (name == "phi") h_all_EB->GetXaxis()->SetTitle("#phi");
  if (name == "phi" || name == "eta") {
    h_all_EB->GetXaxis()->CenterTitle();
    h_all_EB->SetMinimum(0.);
  }
  l_EB->Draw();
  
  // EE
  TH1D *h_all_EE = (TH1D *) f_all->Get("photon_"+name+"_denominator_fake_rate_weighted_EE");
  TH1D *h_unweighted_EE = (TH1D *) f_all->Get("photon_"+name+"_denominator_EE");
  TLegend *l_EE = new TLegend(0.55,0.75,0.85,0.85);
  l_EE->SetBorderSize(0);
  l_EE->SetFillColor(0);
  l_EE->AddEntry(h_all_EE,"Fake rate applied","ep");
  l_EE->AddEntry(h_unweighted_EE,"No fake rate applied","ep");
  c->cd(2);
  h_all_EE->Draw();
  h_unweighted_EE->Draw("same");
  h_unweighted_EE->SetMarkerColor(kRed);
  h_all_EE->SetTitle("EE");
  h_all_EE->SetMaximum(max(h_all_EE->GetMaximum()*1.4,h_unweighted_EE->GetMaximum()*1.4));
  if (name == "pt") {
    h_all_EE->GetXaxis()->SetTitle("p_{T} (GeV)");
    gPad->SetLogy();
  }
  if (name == "eta") h_all_EE->GetXaxis()->SetTitle("#eta");
  if (name == "phi") h_all_EE->GetXaxis()->SetTitle("#phi");
  if (name == "phi" || name == "eta") {
    h_all_EE->GetXaxis()->CenterTitle();
    h_all_EE->SetMinimum(0.);
  }
  l_EE->Draw();
  
  c->SaveAs("kinematics_"+name+"_weighted_denom_compared_unweighted_denom.pdf");
  
  delete c;
  
  return 0;
}

void plot_kinematics() {
  gROOT->SetStyle("Plain");
  gStyle->SetPalette(1,0);
  gStyle->SetNdivisions(505);
  gStyle->SetOptStat(0);

  // input what sample to plot
  TString sample = "";
  cout << "Enter sample to plot (QCD, GJets, GGJets, or all): ";
  cin >> sample;
  if (sample != "QCD" && sample != "GJets" && sample != "GGJets" && sample != "all") {
    cout << "Invalid choice!" << endl;
    return;
  }
  cout << "Using sample: " << sample << endl;
  
  TString filename = "";
  if (sample == "QCD")    filename = "../analysis/diphoton_fake_rate_closure_test_QCD_Pt_all_TuneCUETP8M1_13TeV_pythia8_76X_MiniAOD_histograms.root";
  if (sample == "GJets")  filename = "../analysis/diphoton_fake_rate_closure_test_GJets_HT-all_TuneCUETP8M1_13TeV-madgraphMLM-pythia8_76X_MiniAOD_histograms.root";
  if (sample == "GGJets") filename = "../analysis/diphoton_fake_rate_closure_test_GGJets_M-all_Pt-50_13TeV-sherpa_76X_MiniAOD_histograms.root";
  if (sample == "all")    filename = "../analysis/diphoton_fake_rate_closure_test_all_samples_76X_MiniAOD_histograms.root";
  cout << "filename: " << filename << endl;
  TFile *f_closure_test = TFile::Open(filename);
  
  TString filename_truth = "";
  if (sample == "QCD")    filename_truth = "../analysis/diphoton_fake_rate_closure_test_matching_QCD_Pt_all_TuneCUETP8M1_13TeV_pythia8_76X_MiniAOD_histograms.root";
  if (sample == "GJets")  filename_truth = "../analysis/diphoton_fake_rate_closure_test_matching_GJets_HT-all_TuneCUETP8M1_13TeV-madgraphMLM-pythia8_76X_MiniAOD_histograms.root";
  if (sample == "GGJets") filename_truth = "../analysis/diphoton_fake_rate_closure_test_matching_GGJets_M-all_Pt-50_13TeV-sherpa_76X_MiniAOD_histograms.root";
  if (sample == "all")    filename_truth = "../analysis/diphoton_fake_rate_closure_test_matching_all_samples_76X_MiniAOD_histograms.root";
  cout << "filename for mc truth: " << filename_truth << endl;
  TFile *f_closure_test_matching = TFile::Open(filename_truth);
  
  plot_fakes(f_closure_test,f_closure_test_matching,"pt");
  plot_fakes(f_closure_test,f_closure_test_matching,"eta");
  plot_fakes(f_closure_test,f_closure_test_matching,"phi");
  // plot_unweighted(f_closure_test,f_closure_test_matching,"pt");
  // plot_unweighted(f_closure_test,f_closure_test_matching,"eta");
  // plot_unweighted(f_closure_test,f_closure_test_matching,"phi");
}
