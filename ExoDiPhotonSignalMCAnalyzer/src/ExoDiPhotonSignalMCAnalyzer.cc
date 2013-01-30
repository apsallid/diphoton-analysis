// -*- C++ -*-
//
// Package:    ExoDiPhotonSignalMCAnalyzer
// Class:      ExoDiPhotonSignalMCAnalyzer
// 
/**\class ExoDiPhotonSignalMCAnalyzer ExoDiPhotonSignalMCAnalyzer.cc DiPhotonAnalysis/ExoDiPhotonSignalMCAnalyzer/src/ExoDiPhotonSignalMCAnalyzer.cc

Description: [one line class summary]

Implementation:
[Notes on implementation]
*/
//
// Original Author:  Conor Henderson,40 1-B01,+41227671674,
//         Created:  Wed Jun 16 17:06:28 CEST 2010
// $Id: ExoDiPhotonSignalMCAnalyzer.cc,v 1.7 2012/09/11 22:15:17 jcarson Exp $
//
//


// system include files
#include <memory>
#include <iomanip>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "FWCore/Framework/interface/ESHandle.h"

// to use TfileService for histograms and trees
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "TH1.h"
#include "TTree.h"

//for vertex
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"

// for beamspot
#include "DataFormats/BeamSpot/interface/BeamSpot.h"

// for ecal
#include "DataFormats/DetId/interface/DetId.h"
#include "DataFormats/EcalDetId/interface/EBDetId.h"
#include "DataFormats/EcalDetId/interface/EEDetId.h"
#include "DataFormats/EcalDetId/interface/EcalSubdetector.h"
#include "DataFormats/EcalRecHit/interface/EcalRecHitCollections.h"
#include "RecoLocalCalo/EcalRecAlgos/interface/EcalSeverityLevelAlgo.h"
#include "RecoEcal/EgammaCoreTools/interface/EcalClusterLazyTools.h"
#include "CondFormats/DataRecord/interface/EcalChannelStatusRcd.h"
#include "CondFormats/EcalObjects/interface/EcalChannelStatus.h"


// geometry
#include "Geometry/CaloGeometry/interface/CaloGeometry.h"
//#include "Geometry/Records/interface/IdealGeometryRecord.h"
#include "Geometry/Records/interface/CaloGeometryRecord.h"
//#include "Geometry/CaloEventSetup/interface/CaloTopologyRecord.h"
//#include "Geometry/CaloGeometry/interface/CaloCellGeometry.h"
//#include "Geometry/CaloGeometry/interface/CaloSubdetectorGeometry.h"
#include "Geometry/EcalAlgo/interface/EcalPreshowerGeometry.h"


//for photons
#include "DataFormats/EgammaCandidates/interface/Photon.h"
#include "DataFormats/EgammaCandidates/interface/PhotonFwd.h"

#include "DataFormats/Candidate/interface/LeafCandidate.h"
#include "DataFormats/Math/interface/deltaPhi.h"
#include "TMath.h"



//for trigger
#include "DataFormats/Common/interface/TriggerResults.h"
#include "FWCore/Common/interface/TriggerNames.h"
#include "CondFormats/L1TObjects/interface/L1GtTriggerMenuFwd.h"
#include "DataFormats/L1GlobalTrigger/interface/L1GlobalTriggerReadoutRecord.h" 
#include "DataFormats/L1GlobalTrigger/interface/L1GlobalTriggerRecord.h" 
#include "CondFormats/L1TObjects/interface/L1GtTriggerMenu.h"
#include "CondFormats/DataRecord/interface/L1GtTriggerMenuRcd.h"
#include "L1Trigger/GlobalTrigger/interface/L1GlobalTrigger.h"
#include "L1Trigger/GlobalTriggerAnalyzer/interface/L1GtUtils.h"

// for MC
#include "SimDataFormats/GeneratorProducts/interface/HepMCProduct.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"


// new CommonClasses approach
// these objects are all in the namespace 'ExoDiPhotons'
#include "DiPhotonAnalysis/CommonClasses/interface/RecoPhotonInfo.h"
#include "DiPhotonAnalysis/CommonClasses/interface/MCTrueObjectInfo.h"
#include "DiPhotonAnalysis/CommonClasses/interface/TriggerInfo.h"
#include "DiPhotonAnalysis/CommonClasses/interface/PhotonID.h"
#include "DiPhotonAnalysis/CommonClasses/interface/EventAndVertexInfo.h"
#include "DiPhotonAnalysis/CommonClasses/interface/DiphotonInfo.h"

//new for PU gen
#include "SimDataFormats/PileupSummaryInfo/interface/PileupSummaryInfo.h"

// new for LumiReweighting
#include "PhysicsTools/Utilities/interface/LumiReWeighting.h"


using namespace std;

//
// class declaration
//


class ExoDiPhotonSignalMCAnalyzer : public edm::EDAnalyzer {
public:
  explicit ExoDiPhotonSignalMCAnalyzer(const edm::ParameterSet&);
  ~ExoDiPhotonSignalMCAnalyzer();


private:
  virtual void beginJob() ;
  virtual void analyze(const edm::Event&, const edm::EventSetup&);
  virtual void endJob() ;

  // ----------member data ---------------------------
  
  // input tags and parameters
  edm::InputTag      fPhotonTag;       //select photon collection 
  double             fMin_pt;          // min pt cut (photons)
  edm::InputTag      fHltInputTag;     // hltResults
  edm::InputTag      fRho25Tag;
  edm::InputTag      pileupCollectionTag;
  edm::LumiReWeighting    LumiWeights;

  bool               fkRemoveSpikes;   // option to remove spikes before filling tree
  bool               fkRequireTightPhotons;  // option to require tight photon id in tree
  string             PUMCFileName;
  string             PUDataFileName;
  string             PUDataHistName;
  string             PUMCHistName;

  // tools for clusters
  std::auto_ptr<EcalClusterLazyTools> lazyTools_;

  // my Tree
  TTree *fTree;

  ExoDiPhotons::eventInfo_t fEventInfo;
  ExoDiPhotons::vtxInfo_t fVtxInfo;
  ExoDiPhotons::beamSpotInfo_t fBeamSpotInfo;
  ExoDiPhotons::hltTrigInfo_t fHLTInfo;
  ExoDiPhotons::recoPhotonInfo_t fRecoPhotonInfo1; // leading matched reco photon 
  ExoDiPhotons::recoPhotonInfo_t fRecoPhotonInfo2; // second photon
  ExoDiPhotons::diphotonInfo_t fDiphotonRecoInfo;
 
  //Store PileUp Info
  double fRho25;
  int fpu_n;
  int fBC;
  double fMCPUWeight;

  //Events before selectoin
  TH1F* fpu_n_BeforeCuts;
  TH1F* fpu_n_BeforeCutsAfterReWeight;



};

//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
ExoDiPhotonSignalMCAnalyzer::ExoDiPhotonSignalMCAnalyzer(const edm::ParameterSet& iConfig)
  : fPhotonTag(iConfig.getUntrackedParameter<edm::InputTag>("photonCollection")),
    fMin_pt(iConfig.getUntrackedParameter<double>("ptMin")),
    // note that the HLT process name can vary for different MC samples
    // so be sure to adjsut correctly in cfg
    fHltInputTag(iConfig.getUntrackedParameter<edm::InputTag>("hltResults")),
    fRho25Tag(iConfig.getParameter<edm::InputTag>("rho25Correction")),
    pileupCollectionTag(iConfig.getUntrackedParameter<edm::InputTag>("pileupCorrection")),
    fkRemoveSpikes(iConfig.getUntrackedParameter<bool>("removeSpikes")),
    fkRequireTightPhotons(iConfig.getUntrackedParameter<bool>("requireTightPhotons")),
    PUMCFileName(iConfig.getUntrackedParameter<string>("PUMCFileName")),
    PUDataFileName(iConfig.getUntrackedParameter<string>("PUDataFileName")),
    PUDataHistName(iConfig.getUntrackedParameter<string>("PUDataHistName")),
    PUMCHistName(iConfig.getUntrackedParameter<string>("PUMCHistName"))

{
  //now do what ever initialization is needed

  edm::Service<TFileService> fs;
  fTree = fs->make<TTree>("fTree","PhotonTree");
  fpu_n_BeforeCuts = fs->make<TH1F>("fpu_n_BeforeCuts","PileUpBeforeCuts",300,0,300);
  fpu_n_BeforeCutsAfterReWeight = fs->make<TH1F>("fpu_n_BeforeCutsAfterReWeight","PileUpBeforeCuts",300,0,300);
 
  // now with CommonClasses, use the string defined in the header

  fTree->Branch("Event",&fEventInfo,ExoDiPhotons::eventInfoBranchDefString.c_str());
  fTree->Branch("Vtx",&fVtxInfo,ExoDiPhotons::vtxInfoBranchDefString.c_str());
  fTree->Branch("BeamSpot",&fBeamSpotInfo,ExoDiPhotons::beamSpotInfoBranchDefString.c_str());
  fTree->Branch("TrigHLT",&fHLTInfo,ExoDiPhotons::hltTrigBranchDefString.c_str());

  fTree->Branch("Photon1",&fRecoPhotonInfo1,ExoDiPhotons::recoPhotonBranchDefString.c_str());
  fTree->Branch("Photon2",&fRecoPhotonInfo2,ExoDiPhotons::recoPhotonBranchDefString.c_str());
  fTree->Branch("Diphoton",&fDiphotonRecoInfo,ExoDiPhotons::diphotonInfoBranchDefString.c_str());
   
  //Pileup Info
  fTree->Branch("rho25",&fRho25,"rho25/D");
  fTree->Branch("pu_n", &fpu_n, "pu_n/I");
  fTree->Branch("MCPUWeight",&fMCPUWeight,"MCPUWeight/D");
  
}


ExoDiPhotonSignalMCAnalyzer::~ExoDiPhotonSignalMCAnalyzer()
{
 
  // do anything here that needs to be done at desctruction time
  // (e.g. close files, deallocate resources etc.)

}


//
// member functions
//




// ------------ method called to for each event  ------------
void
ExoDiPhotonSignalMCAnalyzer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  using namespace edm;
  //cout<<"event"<<endl;
  //initialize
  fpu_n = -99999.99;
  fBC = -99999.99;
  fMCPUWeight = -99999.99;

  // basic event info
  ExoDiPhotons::FillEventInfo(fEventInfo,iEvent);

  // get the vertex collection
  Handle<reco::VertexCollection> vertexColl;
  iEvent.getByLabel("offlinePrimaryVertices",vertexColl);

  if(!vertexColl.isValid()) {
    cout << "Vertex collection empty! Bailing out!" <<endl;
    return;
  }
  //   cout << "N vertices = " << vertexColl->size() <<endl;
  fVtxInfo.Nvtx = vertexColl->size();

  fVtxInfo.vx = -99999.99;
  fVtxInfo.vy = -99999.99;
  fVtxInfo.vz = -99999.99;
  fVtxInfo.isFake = -99;   
  fVtxInfo.Ntracks = -99;
  fVtxInfo.sumPtTracks = -99999.99;
  fVtxInfo.ndof = -99999.99;
  fVtxInfo.d0 = -99999.99;

  const reco::Vertex *vertex1 = NULL; // best vertex (by trk SumPt)
  // note for higher lumi, may want to also store second vertex, for pileup studies
   
  // even if the vertex has sumpt=0, its still enough to be the 'highest'
  double highestSumPtTracks1 = -1.0; 

  for(reco::VertexCollection::const_iterator vtx=vertexColl->begin(); vtx!=vertexColl->end(); vtx++) {

    // loop over assoc tracks to get sum pt
    //     fVtxInfo.sumPtTracks = 0.0;
    double sumPtTracks = 0.0;
     
    for(reco::Vertex::trackRef_iterator vtxTracks=vtx->tracks_begin(); vtxTracks!=vtx->tracks_end();vtxTracks++) {
      //       fVtxInfo.sumPtTracks += (**vtxTracks).pt();
      sumPtTracks += (**vtxTracks).pt();
    }

    //     cout << "Vtx x = "<< vtx->x()<<", y= "<< vtx->y()<<", z = " << vtx->z() << ";  N tracks = " << vtx->tracksSize() << "; isFake = " << vtx->isFake() <<", sumPt(tracks) = "<<sumPtTracks << "; ndof = " << vtx->ndof()<< "; d0 = " << vtx->position().rho() << endl;
     
    // and note that this vertex collection can contain vertices with Ntracks = 0
    // watch out for these!

    if(sumPtTracks > highestSumPtTracks1) {
      // then this new best
      vertex1 = &(*vtx);
      highestSumPtTracks1 = sumPtTracks;

    }

   
  }// end vertex loop

  if(vertex1) {

    ExoDiPhotons::FillVertexInfo(fVtxInfo,vertex1);
    // fill the SumPt Tracks separately for now
    fVtxInfo.sumPtTracks = highestSumPtTracks1;
     
  }

  //beam spot

  reco::BeamSpot beamSpot;
  edm::Handle<reco::BeamSpot> beamSpotHandle;
  iEvent.getByLabel("offlineBeamSpot", beamSpotHandle);

  fBeamSpotInfo.x0 = -99999999.99;
  fBeamSpotInfo.y0 = -99999999.99;
  fBeamSpotInfo.z0 = -99999999.99;
  fBeamSpotInfo.sigmaZ = -99999999.99;
  fBeamSpotInfo.x0error = -99999999.99;
  fBeamSpotInfo.y0error = -99999999.99;
  fBeamSpotInfo.z0error = -99999999.99;
  fBeamSpotInfo.sigmaZ0error = -99999999.99;

  if(beamSpotHandle.isValid()) {
    beamSpot = *beamSpotHandle;
    ExoDiPhotons::FillBeamSpotInfo(fBeamSpotInfo,beamSpot);
  }
   

  // L1 info

  // HLT info

  Handle<TriggerResults> hltResultsHandle;
  iEvent.getByLabel(fHltInputTag,hltResultsHandle);

  if(!hltResultsHandle.isValid()) {
    cout << "HLT results not valid!" <<endl;
    cout << "Couldnt find TriggerResults with input tag " << fHltInputTag << endl;
    return;
  }

  const TriggerResults *hltResults = hltResultsHandle.product();
  //   cout << *hltResults <<endl;
  // this way of getting triggerNames should work, even when the 
  // trigger menu changes from one run to the next
  // alternatively, one could also use the HLTConfigPovider
  const TriggerNames & hltNames = iEvent.triggerNames(*hltResults);

  // now we just use the FillHLTInfo() function from TrigInfo.h:
  ExoDiPhotons::FillHLTInfo(fHLTInfo,hltResults,hltNames);
  
  //Add PileUp Information
  edm::Handle<std::vector<PileupSummaryInfo> > pileupHandle;
  iEvent.getByLabel(pileupCollectionTag, pileupHandle);
  std::vector<PileupSummaryInfo>::const_iterator PUI;

  if (pileupHandle.isValid()){

    for (PUI = pileupHandle->begin();PUI != pileupHandle->end(); ++PUI){
      
      fBC = PUI->getBunchCrossing() ;
      if(fBC==0){
	//Select only the in time bunch crossing with bunch crossing=0
	fpu_n = PUI->getTrueNumInteractions();
	fpu_n_BeforeCuts->Fill(fpu_n);
      }
    }

    fMCPUWeight = LumiWeights.weight(fpu_n);
    fpu_n_BeforeCutsAfterReWeight->Fill(fpu_n,fMCPUWeight);

  }
  
  //add rho correction

  //      double rho;

  edm::Handle<double> rho25Handle;
  iEvent.getByLabel(fRho25Tag, rho25Handle);

  if (!rho25Handle.isValid()){
    cout<<"rho25 not found"<<endl;
    return;
  }

  fRho25 = *(rho25Handle.product());


  // ecal information
  lazyTools_ = std::auto_ptr<EcalClusterLazyTools>( new  EcalClusterLazyTools(iEvent,iSetup,edm::InputTag("reducedEcalRecHitsEB"),edm::InputTag("reducedEcalRecHitsEE")));

  //lazyTools_ = std::auto_ptr<EcalClusterLazyTools>( new EcalClusterLazyTools(iEvent,iSetup,edm::InputTag("ecalRecHit:EcalRecHitsEB"),edm::InputTag("ecalRecHit:EcalRecHitsEE")) );

  // get ecal barrel recHits for spike rejection
  edm::Handle<EcalRecHitCollection> recHitsEB_h;
  iEvent.getByLabel(edm::InputTag("reducedEcalRecHitsEB"), recHitsEB_h );
  const EcalRecHitCollection * recHitsEB = 0;
  if ( ! recHitsEB_h.isValid() ) {
    LogError("ExoDiPhotonAnalyzer") << " ECAL Barrel RecHit Collection not available !"; return;
  } else {
    recHitsEB = recHitsEB_h.product();
  }


  edm::Handle<EcalRecHitCollection> recHitsEE_h;
  iEvent.getByLabel(edm::InputTag("reducedEcalRecHitsEE"), recHitsEE_h );
  const EcalRecHitCollection * recHitsEE = 0;
  if ( ! recHitsEE_h.isValid() ) {
    LogError("ExoDiPhotonAnalyzer") << " ECAL Endcap RecHit Collection not available !"; return;
  } else {
    recHitsEE = recHitsEE_h.product();
  }

  edm::ESHandle<EcalChannelStatus> chStatus;
  iSetup.get<EcalChannelStatusRcd>().get(chStatus);
  const EcalChannelStatus *ch_status = chStatus.product(); 



  // get the photon collection
  Handle<reco::PhotonCollection> photonColl;
  iEvent.getByLabel(fPhotonTag,photonColl);

  // If photon collection is empty, exit
  if (!photonColl.isValid()) {
    cout << "No Photons! Move along, there's nothing to see here .." <<endl;
    return;
  }
   
  cout << "N photons = " << photonColl->size() <<endl;

  //      //   photon loop
  for(reco::PhotonCollection::const_iterator recoPhoton = photonColl->begin(); recoPhoton!=photonColl->end(); recoPhoton++) {

    //  cout << "Reco photon et, eta, phi = " << recoPhoton->et() <<", "<<recoPhoton->eta()<< ", "<< recoPhoton->phi();
    cout << "; eMax/e3x3 = " << recoPhoton->maxEnergyXtal()/recoPhoton->e3x3();
    cout << "; hadOverEm = " << recoPhoton->hadronicOverEm();
    cout << "; trkIso = " << recoPhoton->trkSumPtHollowConeDR04();
    cout << "; ecalIso = " << recoPhoton->ecalRecHitSumEtConeDR04();
    cout << "; hcalIso = " << recoPhoton->hcalTowerSumEtConeDR04();
    cout << "; pixelSeed = " << recoPhoton->hasPixelSeed();
    //      cout << endl;

    ExoDiPhotons::FillRecoPhotonInfo(fRecoPhotonInfo1,matchPhoton1,lazyTools_.get(),recHitsEB,recHitsEE,ch_status,iEvent, iSetup);
    ExoDiPhotons::FillRecoPhotonInfo(fRecoPhotonInfo2,matchPhoton2,lazyTools_.get(),recHitsEB,recHitsEE,ch_status,iEvent, iSetup);
    //ExoDiPhotons::FillDiphotonInfo(fDiphotonRecoInfo,matchPhoton1,matchPhoton2);

  }

  // for this signal MC, want to fill the tree every event
  fTree->Fill();
   


#ifdef THIS_IS_AN_EVENT_EXAMPLE
  Handle<ExampleData> pIn;
  iEvent.getByLabel("example",pIn);
#endif
   
#ifdef THIS_IS_AN_EVENTSETUP_EXAMPLE
  ESHandle<SetupData> pSetup;
  iSetup.get<SetupRecord>().get(pSetup);
#endif
}


// ------------ method called once each job just before starting event loop  ------------
void 
ExoDiPhotonSignalMCAnalyzer::beginJob()
{
  LumiWeights = edm::LumiReWeighting(PUMCFileName,PUDataFileName,PUMCHistName,PUDataHistName);
}

// ------------ method called once each job just after ending the event loop  ------------
void 
ExoDiPhotonSignalMCAnalyzer::endJob() {
}

//define this as a plug-in
DEFINE_FWK_MODULE(ExoDiPhotonSignalMCAnalyzer);
