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
// $Id: ExoDiPhotonSignalMCAnalyzer.cc,v 1.10 2013/02/12 14:01:15 scooper Exp $
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
#include "L1Trigger/GlobalTrigger/plugins/L1GlobalTrigger.h"
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
//new for PF ID definition
#include "DiPhotonAnalysis/CommonClasses/interface/PFPhotonID.h"

//new for PU gen
#include "SimDataFormats/PileupSummaryInfo/interface/PileupSummaryInfo.h"

// new for LumiReweighting
#include "PhysicsTools/Utilities/interface/LumiReWeighting.h"

//for conversion safe electron veto
#include "RecoEgamma/EgammaTools/interface/ConversionTools.h"

//new for PFIsolation code
//#include "EGamma/EGammaAnalysisTools/src/PFIsolationEstimator.cc"

//-----------------taken from Ilya-----------------
#include "RecoEgamma/EgammaTools/interface/EffectiveAreas.h"
#include "Math/VectorUtil.h"
#include "DataFormats/Common/interface/ValueMap.h"
#include "DataFormats/Candidate/interface/Candidate.h"
#include "DataFormats/Candidate/interface/CandidateFwd.h"
//-----------------taken from Ilya-----------------

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
  edm::InputTag           fPhotonTag;       // select photon collection 
  double                  fMin_pt;          // min pt cut (photons)
  edm::InputTag           fHltInputTag;     // hltResults
  edm::InputTag           fRho25Tag;
  edm::InputTag           pileupCollectionTag;
  edm::LumiReWeighting    LumiWeights;
  
  bool                    fkRemoveSpikes;         // option to remove spikes before filling tree
  bool                    fkRequireGenEventInfo;  // generated information for RS graviton files
  string                  PUMCFileName;
  string                  PUDataFileName;
  string                  PUDataHistName;
  string                  PUMCHistName;
  string                  fPFIDCategory;
  string                  fIDMethod;
  
  // tools for clusters
  //std::auto_ptr<EcalClusterLazyTools> lazyTools_;
  std::auto_ptr<noZS::EcalClusterLazyTools> lazyTools_;
  edm::InputTag recHitsEBTag_;
  edm::InputTag recHitsEETag_;
  edm::EDGetTokenT<EcalRecHitCollection> recHitsEBToken;
  edm::EDGetTokenT<EcalRecHitCollection> recHitsEEToken;

  // my Tree
  TTree *fTree;
  
  ExoDiPhotons::eventInfo_t fEventInfo;
  ExoDiPhotons::vtxInfo_t fVtxInfo;
  ExoDiPhotons::beamSpotInfo_t fBeamSpotInfo;
  
  ExoDiPhotons::hltTrigInfo_t fHLTInfo;
  
  ExoDiPhotons::mcTrueObjectInfo_t fSignalPhoton1Info; // leading signal photon (from Grav. decay)
  ExoDiPhotons::mcTrueObjectInfo_t fSignalPhoton2Info;

  ExoDiPhotons::mcTrueObjectInfo_t fGenPhoton1Info; // leading gen photon (final state)
  ExoDiPhotons::mcTrueObjectInfo_t fGenPhoton2Info;

  ExoDiPhotons::recoPhotonInfo_t fRecoPhoton1Info; // leading matched reco photon 
  ExoDiPhotons::recoPhotonInfo_t fRecoPhoton2Info; // second photon
  
  ExoDiPhotons::diphotonInfo_t fDiphotonSignalInfo;
  ExoDiPhotons::diphotonInfo_t fDiphotonGenInfo;
  ExoDiPhotons::diphotonInfo_t fDiphotonRecoInfo;
 
  // Store PileUp Info
  double fRho25;
  int fpu_n;
  int fBC;
  double fMCPUWeight;

  // Events before selectoin
  TH1F* fpu_n_BeforeCuts;
  TH1F* fpu_n_BeforeCutsAfterReWeight;

  // SIC add
  // for PFIsolation Code
  //PFIsolationEstimator isolator04;
  //PFIsolationEstimator isolator03;
  //PFIsolationEstimator isolator02;

  //-----------------taken from Ilya-----------------
  // Format-independent data members
  edm::EDGetTokenT<double> rhoToken_;
  
  // AOD case data members
  edm::EDGetToken photonsToken_;
  //edm::EDGetTokenT<edm::View<reco::GenParticle> > genParticlesToken_;
  
  // MiniAOD case data members
  edm::EDGetToken photonsMiniAODToken_;
  //edm::EDGetTokenT<edm::View<reco::GenParticle> > genParticlesMiniAODToken_;
  
  // Photon variables computed upstream in a special producer
  edm::EDGetTokenT<edm::ValueMap<float> > full5x5SigmaIEtaIEtaMapToken_; 
  edm::EDGetTokenT<edm::ValueMap<float> > phoChargedIsolationToken_; 
  edm::EDGetTokenT<edm::ValueMap<float> > phoNeutralHadronIsolationToken_; 
  edm::EDGetTokenT<edm::ValueMap<float> > phoPhotonIsolationToken_; 

  // ID decision objects
  edm::EDGetTokenT<edm::ValueMap<bool> > phoLooseIdMapToken_;
  edm::EDGetTokenT<edm::ValueMap<bool> > phoMediumIdMapToken_;
  edm::EDGetTokenT<edm::ValueMap<bool> > phoTightIdMapToken_;

  Float_t rho_;      // the rho variable

  // Effective area constants for all isolation types
  EffectiveAreas effAreaChHadrons_;
  EffectiveAreas effAreaNeuHadrons_;
  EffectiveAreas effAreaPhotons_;

  std::vector<Int_t> passLooseId_;
  std::vector<Int_t> passMediumId_;
  std::vector<Int_t> passTightId_;
  //-----------------taken from Ilya-----------------

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
    fkRequireGenEventInfo(iConfig.getUntrackedParameter<bool>("requireGenEventInfo")),
    PUMCFileName(iConfig.getUntrackedParameter<string>("PUMCFileName")),
    PUDataFileName(iConfig.getUntrackedParameter<string>("PUDataFileName")),
    PUDataHistName(iConfig.getUntrackedParameter<string>("PUDataHistName")),
    PUMCHistName(iConfig.getUntrackedParameter<string>("PUMCHistName")),
    fPFIDCategory(iConfig.getUntrackedParameter<string>("PFIDCategory")),
    fIDMethod(iConfig.getUntrackedParameter<string>("IDMethod")),
    //-----------------taken from Ilya-----------------
    rhoToken_(consumes<double> (iConfig.getParameter<edm::InputTag>("rho"))),
    // Cluster shapes
    full5x5SigmaIEtaIEtaMapToken_(consumes <edm::ValueMap<float> > (iConfig.getParameter<edm::InputTag>("full5x5SigmaIEtaIEtaMap"))),
    // Isolations
    phoChargedIsolationToken_(consumes <edm::ValueMap<float> > (iConfig.getParameter<edm::InputTag>("phoChargedIsolation"))),
    phoNeutralHadronIsolationToken_(consumes <edm::ValueMap<float> > (iConfig.getParameter<edm::InputTag>("phoNeutralHadronIsolation"))),
    phoPhotonIsolationToken_(consumes <edm::ValueMap<float> > (iConfig.getParameter<edm::InputTag>("phoPhotonIsolation"))),
    phoLooseIdMapToken_(consumes<edm::ValueMap<bool> >(iConfig.getParameter<edm::InputTag>("phoLooseIdMap"))),
    phoMediumIdMapToken_(consumes<edm::ValueMap<bool> >(iConfig.getParameter<edm::InputTag>("phoMediumIdMap"))),
    phoTightIdMapToken_(consumes<edm::ValueMap<bool> >(iConfig.getParameter<edm::InputTag>("phoTightIdMap"))),
    // Objects containing effective area constants
    effAreaChHadrons_( (iConfig.getParameter<edm::FileInPath>("effAreaChHadFile")).fullPath() ),
    effAreaNeuHadrons_( (iConfig.getParameter<edm::FileInPath>("effAreaNeuHadFile")).fullPath() ),
    effAreaPhotons_( (iConfig.getParameter<edm::FileInPath>("effAreaPhoFile")).fullPath() )
    //-----------------taken from Ilya-----------------
{
   //now do what ever initialization is needed

  std::cout << "ExoDiPhotonAnalyzer: ID Method used " << fIDMethod.c_str()
	    << "PF ID Category " << fPFIDCategory.c_str()
	    << std::endl;

  //-----------------taken from Ilya-----------------
  //
  // Prepare tokens for all input collections and objects
  //
  // AOD tokens
  photonsToken_ = mayConsume<edm::View<reco::Photon> >
    (iConfig.getParameter<edm::InputTag>
     ("photons"));
  
  //  genParticlesToken_ = mayConsume<edm::View<reco::GenParticle> >
  //    (iConfig.getParameter<edm::InputTag>
  //     ("genParticles"));
  
  // MiniAOD tokens
  photonsMiniAODToken_ = mayConsume<edm::View<reco::Photon> >
    (iConfig.getParameter<edm::InputTag>
     ("photonsMiniAOD"));
  
  //  genParticlesMiniAODToken_ = mayConsume<edm::View<reco::GenParticle> >
  //    (iConfig.getParameter<edm::InputTag>
  //     ("genParticlesMiniAOD"));
  //-----------------taken from Ilya-----------------

  edm::Service<TFileService> fs;
  fTree = fs->make<TTree>("fTree","PhotonTree");
  fpu_n_BeforeCuts = fs->make<TH1F>("fpu_n_BeforeCuts","PileUpBeforeCuts",300,0,300);
  fpu_n_BeforeCutsAfterReWeight = fs->make<TH1F>("fpu_n_BeforeCutsAfterReWeight","PileUpBeforeCuts",300,0,300);
 
  // now with CommonClasses, use fthe string defined in the header

  fTree->Branch("Event",&fEventInfo,ExoDiPhotons::eventInfoBranchDefString.c_str());
  fTree->Branch("Vtx",&fVtxInfo,ExoDiPhotons::vtxInfoBranchDefString.c_str());
  fTree->Branch("BeamSpot",&fBeamSpotInfo,ExoDiPhotons::beamSpotInfoBranchDefString.c_str());
  fTree->Branch("TrigHLT",&fHLTInfo,ExoDiPhotons::hltTrigBranchDefString.c_str());

  fTree->Branch("SignalPhoton1",&fSignalPhoton1Info,ExoDiPhotons::mcTrueObjectInfoBranchDefString.c_str());
  fTree->Branch("SignalPhoton2",&fSignalPhoton2Info,ExoDiPhotons::mcTrueObjectInfoBranchDefString.c_str());

  fTree->Branch("GenPhoton1",&fGenPhoton1Info,ExoDiPhotons::mcTrueObjectInfoBranchDefString.c_str());
  fTree->Branch("GenPhoton2",&fGenPhoton2Info,ExoDiPhotons::mcTrueObjectInfoBranchDefString.c_str());

  fTree->Branch("Photon1",&fRecoPhoton1Info,ExoDiPhotons::recoPhotonBranchDefString.c_str());
  fTree->Branch("Photon2",&fRecoPhoton2Info,ExoDiPhotons::recoPhotonBranchDefString.c_str());
  
  // signal diphoton info? eg to probe true MC width?
  // reco diphoton info?
  
  fTree->Branch("DiphotonSignal",&fDiphotonSignalInfo,ExoDiPhotons::diphotonInfoBranchDefString.c_str());
  fTree->Branch("DiphotonGen",&fDiphotonGenInfo,ExoDiPhotons::diphotonInfoBranchDefString.c_str());
  fTree->Branch("Diphoton",&fDiphotonRecoInfo,ExoDiPhotons::diphotonInfoBranchDefString.c_str());
   
  
  //Pileup Info
  fTree->Branch("rho25",&fRho25,"rho25/D");
  fTree->Branch("pu_n", &fpu_n, "pu_n/I");
  fTree->Branch("MCPUWeight",&fMCPUWeight,"MCPUWeight/D");
  

  // SIC add
  //new PFIsolation code
  //isolator04.initializePhotonIsolation(kTRUE);
  //isolator04.setConeSize(0.4);
  //isolator03.initializePhotonIsolation(kTRUE);
  //isolator03.setConeSize(0.3);
  //isolator02.initializePhotonIsolation(kTRUE);
  //isolator02.setConeSize(0.2);
   
  recHitsEBTag_ = iConfig.getUntrackedParameter<edm::InputTag>("RecHitsEBTag",edm::InputTag("reducedEcalRecHitsEB"));
  recHitsEETag_ = iConfig.getUntrackedParameter<edm::InputTag>("RecHitsEETag",edm::InputTag("reducedEcalRecHitsEE"));
  recHitsEBToken = consumes <edm::SortedCollection<EcalRecHit> > (recHitsEBTag_);
  recHitsEEToken = consumes <edm::SortedCollection<EcalRecHit> > (recHitsEETag_);
  // recHitsEBTag_ = iConfig.getUntrackedParameter<edm::InputTag>("RecHitsEBTag",edm::InputTag("reducedEgamma:reducedEBRecHits"));
  // recHitsEETag_ = iConfig.getUntrackedParameter<edm::InputTag>("RecHitsEETag",edm::InputTag("reducedEgamma:reducedEERecHits"));
  // recHitsEBToken = consumes < EcalRecHitCollection > (recHitsEBTag_);
  // recHitsEEToken = consumes < EcalRecHitCollection > (recHitsEETag_);
  //TO DISENTANGLE BETWEEN MINIAOD AND AOD
 
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
   using namespace std;
   using namespace reco;

   //cout <<  iEvent.id().run() << " " <<  iEvent.id().luminosityBlock() << " " << iEvent.id().event() << endl;


   //cout<<"event"<<endl;
   //initialize
   fpu_n = -99999.99;
   fBC = -99999.99;
   fMCPUWeight = -99999.99;

   // basic event info
   ExoDiPhotons::FillEventInfo(fEventInfo,iEvent);

   //-----------------taken from Ilya-----------------
  // Retrieve the collection of photons from the event.
  // If we fail to retrieve the collection with the standard AOD
  // name, we next look for the one with the stndard miniAOD name. 
  //   We use exactly the same handle for AOD and miniAOD formats
  // since pat::Photon objects can be recast as reco::Photon objects.
  edm::Handle<edm::View<reco::Photon> > photons;
  //bool isAOD = true;
  iEvent.getByToken(photonsToken_, photons);
  if( !photons.isValid() ){
    //isAOD = false;
    iEvent.getByToken(photonsMiniAODToken_,photons);
  }
  //-----------------taken from Ilya-----------------
  // Get generator level info
  // edm::Handle<edm::View<reco::GenParticle> > genParticles;
  // if( isAOD )
  //  iEvent.getByToken(genParticlesToken_,genParticles);
  //  else
  //  iEvent.getByToken(genParticlesMiniAODToken_,genParticles);
  
  //Get rho
  edm::Handle< double > rhoH;
  iEvent.getByToken(rhoToken_,rhoH);
  rho_ = *rhoH;
  
  // Get the full5x5 map
  edm::Handle<edm::ValueMap<float> > full5x5SigmaIEtaIEtaMap;
  iEvent.getByToken(full5x5SigmaIEtaIEtaMapToken_, full5x5SigmaIEtaIEtaMap);

  // Get the isolation maps
  edm::Handle<edm::ValueMap<float> > phoChargedIsolationMap;
  iEvent.getByToken(phoChargedIsolationToken_, phoChargedIsolationMap);
  edm::Handle<edm::ValueMap<float> > phoNeutralHadronIsolationMap;
  iEvent.getByToken(phoNeutralHadronIsolationToken_, phoNeutralHadronIsolationMap);
  edm::Handle<edm::ValueMap<float> > phoPhotonIsolationMap;
  iEvent.getByToken(phoPhotonIsolationToken_, phoPhotonIsolationMap);

  // Get the photon ID data from the event stream.
  // Note: this implies that the VID ID modules have been run upstream.
  // If you need more info, check with the EGM group.
  edm::Handle<edm::ValueMap<bool> > loose_id_decisions;
  edm::Handle<edm::ValueMap<bool> > medium_id_decisions;
  edm::Handle<edm::ValueMap<bool> > tight_id_decisions;
  iEvent.getByToken(phoLooseIdMapToken_ ,loose_id_decisions);
  iEvent.getByToken(phoMediumIdMapToken_,medium_id_decisions);
  iEvent.getByToken(phoTightIdMapToken_ ,tight_id_decisions);
  //-----------------taken from Ilya-----------------

  edm::Handle<GenEventInfoProduct> GenInfoHandle;
  if(fkRequireGenEventInfo){
    iEvent.getByLabel("generator",GenInfoHandle);
    if(!GenInfoHandle.isValid()) {
      cout << "Gen Event Info Product collection empty! Bailing out!" <<endl;
      return;
    }
  }

  if(fkRequireGenEventInfo) {
    fEventInfo.pthat = GenInfoHandle->hasBinningValues() ? (GenInfoHandle->binningValues())[0] : 0.0 ;
    fEventInfo.alphaqcd = GenInfoHandle->alphaQCD();
    fEventInfo.alphaqed = GenInfoHandle->alphaQED();
    fEventInfo.qscale = GenInfoHandle->qScale();
    fEventInfo.processid = GenInfoHandle->signalProcessID();
    fEventInfo.weight = GenInfoHandle->weights()[0];
  }

  //fNumTotalEvents->Fill(1.);
  //fNumTotalWeightedEvents->Fill(1.,fEventInfo.weight);

  //cout <<  iEvent.id().run() << " " <<  iEvent.id().luminosityBlock() << " " << iEvent.id().event() << endl;

  // get the vertex collection
  Handle<reco::VertexCollection> vertexColl;
  iEvent.getByLabel("offlinePrimaryVertices",vertexColl);
  //iEvent.getByLabel("offlineSlimmedPrimaryVertices",vertexColl);
  //TO DISENTANGLE BETWEEN MINIAOD AND AOD

  if(!vertexColl.isValid()) {
    cout << "Vertex collection empty! Bailing out!" <<endl;
    return;
  }
  //   cout << "N vertices = " << vertexColl->size() <<endl;
  // fVtxInfo.Nvtx = vertexColl->size();

  fVtxInfo.vx = -99999.99;
  fVtxInfo.vy = -99999.99;
  fVtxInfo.vz = -99999.99;
  fVtxInfo.isFake = -99;   
  fVtxInfo.Ntracks = -99;
  fVtxInfo.sumPtTracks = -99999.99;
  fVtxInfo.ndof = -99999.99;
  fVtxInfo.d0 = -99999.99;
  
  //get the reference to 1st vertex for use in fGetIsolation
  //for PFIsolation calculation
  reco::VertexRef firstVtx(vertexColl,0);

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

   // beam spot
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
   
  //for PFIsolation code
  //Handle<PFCandidateCollection> pfCandidatesColl;
  //iEvent.getByLabel("particleFlow",pfCandidatesColl);
  //const PFCandidateCollection * pfCandidates = pfCandidatesColl.product();

  //for conversion safe electron veto
  edm::Handle<reco::ConversionCollection> hConversions;
  iEvent.getByLabel("allConversions", hConversions);
  
  edm::Handle<reco::GsfElectronCollection> hElectrons;
  //iEvent.getByLabel("gsfElectrons", hElectrons);
  iEvent.getByLabel("gedGsfElectrons", hElectrons);
  //edm::Handle<pat::ElectronCollection> hElectrons;
  //iEvent.getByLabel(edm::InputTag("slimmedElectrons"), hElectrons);
  //patElectrons_slimmedElectrons__PAT.obj.embeddedSuperCluster_
  //TO DISENTANGLE BETWEEN MINIAOD AND AOD
  if(!hElectrons.isValid()) {
    cout<<"no ged gsf electrons "<<endl;
    return;
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
  // edm::Handle<double> rho25Handle;
  // iEvent.getByLabel(fRho25Tag, rho25Handle);

  //if (!rho25Handle.isValid()){
  //cout<<"rho25 not found"<<endl;
  //return;
  // }
  
  //fRho25 = *(rho25Handle.product());
  fRho25 = *rhoH;

  // ecal information
  // lazyTools_ = std::auto_ptr<EcalClusterLazyTools>( new  EcalClusterLazyTools(iEvent,iSetup,edm::InputTag("reducedEcalRecHitsEB"),edm::InputTag("reducedEcalRecHitsEE")));
  // //lazyTools_ = std::auto_ptr<EcalClusterLazyTools>( new EcalClusterLazyTools(iEvent,iSetup,edm::InputTag("ecalRecHit:EcalRecHitsEB"),edm::InputTag("ecalRecHit:EcalRecHitsEE")) );
  lazyTools_ = std::auto_ptr<noZS::EcalClusterLazyTools>( new noZS::EcalClusterLazyTools(iEvent, iSetup, recHitsEBToken, recHitsEEToken));

   // get ecal barrel recHits for spike rejection
   edm::Handle<EcalRecHitCollection> recHitsEB_h;
   iEvent.getByLabel(edm::InputTag("reducedEcalRecHitsEB"), recHitsEB_h );
   const EcalRecHitCollection * recHitsEB = 0;
   if ( ! recHitsEB_h.isValid() ) {
     LogError("ExoDiPhotonAnalyzer") << " ECAL Barrel RecHit Collection not available !"; return;
   } else {
     recHitsEB = recHitsEB_h.product();
   }
   //TO DISENTANGLE BETWEEN MINIAOD AND AOD

   edm::Handle<EcalRecHitCollection> recHitsEE_h;
   iEvent.getByLabel(edm::InputTag("reducedEcalRecHitsEE"), recHitsEE_h );
   const EcalRecHitCollection * recHitsEE = 0;
   if ( ! recHitsEE_h.isValid() ) {
    LogError("ExoDiPhotonAnalyzer") << " ECAL Endcap RecHit Collection not available !"; return;
   } else {
    recHitsEE = recHitsEE_h.product();
   }
   //TO DISENTANGLE BETWEEN MINIAOD AND AOD

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

   //   const reco::PhotonCollection *myPhotonColl = photonColl.product();
   //cout<<"photoncoll size "<<myPhotonColl->size()<<endl;
   //   cout << "N reco photons = " << photonColl->size() <<endl;


   //cout << "N photons = " << photonColl->size() <<endl;

   //      //   photon loop
   //    for(reco::PhotonCollection::const_iterator recoPhoton = photonColl->begin(); recoPhoton!=photonColl->end(); recoPhoton++) {
   //
   //   //  cout << "Reco photon et, eta, phi = " << recoPhoton->et() <<", "<<recoPhoton->eta()<< ", "<< recoPhoton->phi();
   //   //  cout << "; eMax/e3x3 = " << recoPhoton->maxEnergyXtal()/recoPhoton->e3x3();
   //   //   cout << "; hadOverEm = " << recoPhoton->hadronicOverEm();
   //   //   cout << "; trkIso = " << recoPhoton->trkSumPtHollowConeDR04();
   //   //   cout << "; ecalIso = " << recoPhoton->ecalRecHitSumEtConeDR04();
   //   //   cout << "; hcalIso = " << recoPhoton->hcalTowerSumEtConeDR04();
   //   //   cout << "; pixelSeed = " << recoPhoton->hasPixelSeed();
   ////      cout << endl;
   //
   //
   //    } // end reco photon loop
   
   // need these??
   TString CategoryPFID(fPFIDCategory.c_str());
   TString MethodID(fIDMethod.c_str());
   
   //cout <<  iEvent.id().run() << " " <<  iEvent.id().luminosityBlock() << " " << iEvent.id().event() << endl;

   /*        
     // now add selected photons to vector if:
     // tight ID
     // not in gap
     // not a spike
     // min pt cut  (same for all photons)
     // in EB only (use detector eta)

     // we will check both tight and fakeable photons
     // some cuts are common to both - EB and pt
     //apr 2011, remove EB cut - what should we do about the increased combinatorics?


     //Unfortunately, we have to compute PF isolation variables here
     //to check if the photon is tight or fakeable
     //But then, we have to recompute them later for each tight or fakeable photon
     //because once again the PF isolation variables are not accessible 
     //in the Photon class
     //We need to compute only 03 isol variables so far
     //because they are the official ones

     //we retrieve the effective areas
     //Remember effareaCH = 1st, effareaNH = 2nd, effareaPH = 3rd
     //std::vector<double> effareas = ExoDiPhotons::EffectiveAreas(&(*recoPhoton));
     // double rhocorPFIsoCH = isoChargedHadronsWithEA;
     //double rhocorPFIsoNH = isoNeutralHadronsWithEA;
     //double rhocorPFIsoPH = isoPhotonsWithEA;
     //double pfisoall = rhocorPFIsoCH + rhocorPFIsoNH + rhocorPFIsoPH;
     //and we also have to test the conversion safe electron veto
     //bool passelecveto = !ConversionTools::hasMatchedPromptElectron(recoPhoton->superCluster(), hElectrons, hConversions, beamSpot.position());
     //bool passelecveto = true;
     //TO DISENTANGLE BETWEEN MINIAOD AND AOD

     // cout << "Photon et, eta, phi = " << recoPhoton->et() <<", "<<recoPhoton->eta()<< ", "<< recoPhoton->phi();
     // cout << "; calo position eta = " << recoPhoton->caloPosition().eta();
     // cout << "; eMax/e3x3 = " << recoPhoton->maxEnergyXtal()/recoPhoton->e3x3();
     // cout << "; hadOverEm = " << recoPhoton->hadronicOverEm();
     // cout << "; pixelSeed = " << recoPhoton->hasPixelSeed();
     // cout << "; sigmaietaieta = " << recoPhoton->sigmaIetaIeta();
     // cout << "; CHiso = " << rhocorPFIsoCH;
     // cout << "; NHiso = " << rhocorPFIsoNH;
     // cout << "; PHiso = " << rhocorPFIsoPH;
     // cout<<""<<endl;

   
     //Now we choose which ID to use (PF or Det)
     if(MethodID.Contains("Detector")){
       if(ExoDiPhotons::isTightPhoton(&(*recoPhoton),fRho25) && !ExoDiPhotons::isGapPhoton(&(*recoPhoton)) && !ExoDiPhotons::isASpike(&(*recoPhoton))  ) {
	 //	    if( !ExoDiPhotons::isASpike(&(*recoPhoton))  ) {   
	 selectedPhotons.push_back(*recoPhoton);
       }
     }
     else if(MethodID.Contains("ParticleFlow")){
       //CAREFUL UNCOMMENT THAT WHEN DONE
       // if(!(ExoDiPhotons::isPFTightPhoton(&(*recoPhoton),rhocorPFIsoCH,rhocorPFIsoNH,rhocorPFIsoPH,full5x5sigmaIetaIeta,passelecveto,CategoryPFID))) continue;
       if(!isPassLoose) continue;
       if(ExoDiPhotons::isGapPhoton(&(*recoPhoton))) continue;
       if(ExoDiPhotons::isASpike(&(*recoPhoton))) continue;
       selectedPhotons.push_back(*recoPhoton);
     }

   
   */




   //   Handle<HepMCProduct> mcProduct;
   //   iEvent.getByLabel("generator",mcProduct);
   //   const HepMC::GenEvent *mcEvent = mcProduct->GetEvent();
   
   //     mcEvent->print( std::cout );

   // I used to use HepMCProdfuct, and access particles as HepMC::GenParticles
   // I now use the reco::GenParticles collection
   Handle<reco::GenParticleCollection> genParticles;
   iEvent.getByLabel("genParticles",genParticles);

   if(!genParticles.isValid()) {
     cout << "No Gen Particles collection!" << endl;
     return;
   }
   /*
   fSignalPhoton1Info.status = -999999;
   fSignalPhoton1Info.PdgId = -999999;
   fSignalPhoton1Info.MotherPdgId = -999999;
   fSignalPhoton1Info.GrandmotherPdgId = -999999;
   fSignalPhoton1Info.pt = -999999.99;
   fSignalPhoton1Info.eta = -999999.99;
   fSignalPhoton1Info.phi = -999999.99;
   fSignalPhoton1Info.isol04 = -999999.99;
   fSignalPhoton1Info.isol04ratio = -999999.99;
   fSignalPhoton1Info.isol03 = -999999.99;
   fSignalPhoton1Info.isol03ratio = -999999.99;
   fSignalPhoton1Info.isol02 = -999999.99;
   fSignalPhoton1Info.isol02ratio = -999999.99;

   fSignalPhoton2Info.status = -999999;
   fSignalPhoton2Info.PdgId = -999999;
   fSignalPhoton2Info.MotherPdgId = -999999;
   fSignalPhoton2Info.GrandmotherPdgId = -999999;
   fSignalPhoton2Info.pt = -999999.99;
   fSignalPhoton2Info.eta = -999999.99;
   fSignalPhoton2Info.phi = -999999.99;
   fSignalPhoton2Info.isol04 = -999999.99;
   fSignalPhoton2Info.isol04ratio = -999999.99;
   fSignalPhoton2Info.isol03 = -999999.99;
   fSignalPhoton2Info.isol03ratio = -999999.99;
   fSignalPhoton2Info.isol02 = -999999.99;
   fSignalPhoton2Info.isol02ratio = -999999.99;
   */

   ExoDiPhotons::InitMCTrueObjectInfo(fSignalPhoton1Info);
   ExoDiPhotons::InitMCTrueObjectInfo(fSignalPhoton2Info);

   ExoDiPhotons::InitMCTrueObjectInfo(fGenPhoton1Info);
   ExoDiPhotons::InitMCTrueObjectInfo(fGenPhoton2Info);

   const reco::Candidate *signalPhoton1 = NULL;
   const reco::Candidate *signalPhoton2 = NULL;

   const reco::Candidate *genPhoton1 = NULL;
   const reco::Candidate *genPhoton2 = NULL;

   using namespace reco;

   for(unsigned i = 0; i < genParticles->size(); ++ i) {
     
     const GenParticle & p = (*genParticles)[i];
     int id = p.pdgId();
     int st = p.status();  
     
     //const Candidate * mom = p.mother();
     //double pt = p.pt(), eta = p.eta(), phi = p.phi(), mass = p.mass();
     //double vx = p.vx(), vy = p.vy(), vz = p.vz();
     //int charge = p.charge();

     unsigned nDau = p.numberOfDaughters();
     
     if(id == 5100039 && p.isLastCopy()){     
       cout << "---RSG---" << endl;
       cout << "numberOfDaughters: " << nDau << endl;
       cout << "Status: " << st << endl;
       cout << "isLastCopy(): " << p.isLastCopy() << endl;
       cout << "GenParticle Pt, Eta, Phi: " << p.pt() << ", " << p.eta() << ", " << p.phi() << endl;
       for(unsigned j = 0; j < nDau; ++ j) {
	 
	 const Candidate * dau = p.daughter( j );
	 unsigned nDauDau = dau->numberOfDaughters();
	 int dauId = dau->pdgId();
	 
	 // since p is last copy, should be true, but will require it anyway
	 if(nDau == 2 && dauId == 22) {
	   //	   const GenParticle  *pho = dau;
	   cout << " -dauID: " << dauId << endl;
	   //cout << "   isLastCopy: " << dau->isLastCopy() << endl;
	   cout << "   numberOfDaughters: " << nDauDau << endl;
	   cout << "   dauStatus: " << dau->status() << endl;
	   cout << "   dauPt, Eta, Phi: " << dau->pt() << ", " << dau->eta() << ", " << dau->phi() << endl;
	   
	   // assign signal photons
	   if (!signalPhoton1) signalPhoton1 = dau;
	   else signalPhoton2 = dau;
	   
	   // assign gen photons CASE 1
	   if (nDauDau == 0 && dau->status() == 1) {
	     if (!genPhoton1) genPhoton1 = dau;
	     else genPhoton2 = dau;
	   }
	   // assign gen photons CASE 2
	   // check if one photon pair produces (the other has a final state photon dau)
	   else if (nDauDau == 1 && dau->daughter(0)->pdgId() == 22) {
	     if (dau->daughter(0)->status() == 1) {
	       const Candidate *dauDau = dau->daughter(0);
	       if (!genPhoton1) genPhoton1 = dauDau;
	       else genPhoton2 = dauDau;
	     }
	     else if (dau->daughter(0)->numberOfDaughters()==2
		      && fabs(dau->daughter(0)->daughter(0)->pdgId()) == fabs(dau->daughter(0)->daughter(1)->pdgId())) {
	       const Candidate *dauDauDau = dau->daughter(0)->daughter(0); // take either, choose 1st
	       cout << "  *Signal photon pair produced.*" << endl;
	       if (!genPhoton1) genPhoton1 = dauDauDau;
	       else genPhoton2 = dauDauDau;
	     }
	   }
	   else if (nDauDau == 2 && fabs(dau->daughter(0)->pdgId()) == fabs(dau->daughter(1)->pdgId())) {
	     const Candidate *dauDau = dau->daughter(0); // take either, choose 1st
	     cout << "  *Signal photon pair produced.*" << endl;
	     if (!genPhoton1) genPhoton1 = dauDau;
	     else genPhoton2 = dauDau;
	   }
	   else {
	     cout << "Didnt find Gen Photon!" << endl;
	   }
	   
	   // print dauDau info
	   for (unsigned j=0; j<nDauDau; ++j) {
	     const Candidate *dauDau = dau->daughter( j );
	     //	       const GenParticle *phoD = dauDau;
	     cout << "  --dauID: " << dauDau->pdgId() << endl;
	     //cout << "     isLastCopy: " << dauDau->isLastCopy() << endl;
	     cout << "     numberOfDaughters: " << dauDau->numberOfDaughters() << endl;
	     cout << "     dauStatus: " << dauDau->status() << endl;
	     cout << "     dauPt, Eta, Phi: " << dauDau->pt() << ", " << dauDau->eta() << ", " << dauDau->phi() << endl;
	     
	     //print dauDauDau info
	     unsigned nDauDauDau = dauDau->numberOfDaughters();
	     for (unsigned j=0; j<nDauDauDau; ++j) {
	       const Candidate *dauDauDau = dauDau->daughter( j );
	       cout << "   ---dauID: " << dauDauDau->pdgId() << endl;
	       cout << "       numberOfDaughters: " << dauDauDau->numberOfDaughters() << endl;
	       cout << "       dauStatus: " << dauDauDau->status() << endl;
	       cout << "       dauPt, Eta, Phi: " << dauDauDau->pt() << ", " << dauDauDau->eta() << ", " << dauDauDau->phi() << endl;
	     } // end dauDauDau loop
	   } // end dauDau loop
	 
	 } // end 2 photon check
	 else cout << "Didnt find 2 photons from RSG decay!" << endl;
       } // end dau loop
     } // end RSG check
   } // end genParticle loop


   // what if some of the signal photons arent found? 
   if(!signalPhoton1) {
     cout << "Couldnt find signal Photon1 !" <<endl;
     fSignalPhoton1Info.status = -99;
     fTree->Fill();
     return;
   }
   if(!signalPhoton2) {
     cout << "Couldnt find signal Photon2 !" <<endl;
     fSignalPhoton2Info.status = -99;
     fTree->Fill();
     return;
   }

   // what if some of the gen photons arent found? 
   if(!genPhoton1) {
     cout << "Couldnt find gen Photon1 !" <<endl;
     fGenPhoton1Info.status = -888.888;
     //fTree->Fill();
     //return;
   }
   if(!genPhoton2) {
     cout << "Couldnt find gen Photon2 !" <<endl;
     fGenPhoton2Info.status = -888.888;
     //fTree->Fill();
     //return;
   }

   // reorder the signal photons by pt
   if(signalPhoton1 && signalPhoton2 && signalPhoton2->pt()>signalPhoton1->pt()) {
     const reco::Candidate *tempSignalPhoton = signalPhoton1;
     signalPhoton1 = signalPhoton2;
     signalPhoton2 = tempSignalPhoton;
   }
  
   // reorder the gen photons by pt
   if(genPhoton1 && genPhoton2 && genPhoton2->pt()>genPhoton1->pt()) {
     const reco::Candidate *tempGenPhoton = genPhoton1;
     genPhoton1 = genPhoton2;
     genPhoton2 = tempGenPhoton;
   }

   // if genPhoton1 pair produces, swap with genPhoton2 (which may also have pair produced)
   // careful: if genPhoton1 pair produces and we dont have genPhoton2
   if(genPhoton1 && genPhoton1->pdgId() != 22) {
     const reco::Candidate *tempGenPhoton = genPhoton1;
     genPhoton1 = genPhoton2;
     genPhoton2 = tempGenPhoton;
   }
   if(genPhoton1 && genPhoton1->pdgId() != 22) {
     cout << "GenPhoton1 is part of pair production." << endl;
     fGenPhoton1Info.status = -77;
     fGenPhoton1Info.PdgId = fabs(genPhoton1->pdgId());
     fGenPhoton1Info.pt= -77.77;
     fGenPhoton1Info.eta= -77.77;
     fGenPhoton1Info.phi= -77.77;
   }
   if(genPhoton2 && genPhoton2->pdgId() != 22) {
     cout << "GenPhoton2 is part of pair production." << endl;
     fGenPhoton2Info.status = -77;
     fGenPhoton2Info.PdgId = fabs(genPhoton2->pdgId());
     fGenPhoton2Info.pt= -77.77;
     fGenPhoton2Info.eta= -77.77;
     fGenPhoton2Info.phi= -77.77;
   }

   if(signalPhoton1) {
     ExoDiPhotons::FillMCTrueObjectInfo(fSignalPhoton1Info,signalPhoton1);
     //      fSignalPhoton1Info.status = signalPhoton1->status();
     //      fSignalPhoton1Info.PdgId = signalPhoton1->pdgId();
     //      fSignalPhoton1Info.MotherPdgId = signalPhoton1->mother()->pdgId();
     //      fSignalPhoton1Info.GrandmotherPdgId = signalPhoton1->mother()->mother()->pdgId();
     //      fSignalPhoton1Info.pt = signalPhoton1->pt();
     //      fSignalPhoton1Info.eta = signalPhoton1->eta();
     //      fSignalPhoton1Info.phi = signalPhoton1->phi();
   }
   if(signalPhoton2) {
     ExoDiPhotons::FillMCTrueObjectInfo(fSignalPhoton2Info,signalPhoton2);
     //      fSignalPhoton2Info.status = signalPhoton2->status();
     //      fSignalPhoton2Info.PdgId = signalPhoton2->pdgId();
     //      fSignalPhoton2Info.MotherPdgId = signalPhoton2->mother()->pdgId();
     //      fSignalPhoton2Info.GrandmotherPdgId = signalPhoton2->mother()->mother()->pdgId();
     //      fSignalPhoton2Info.pt = signalPhoton2->pt();
     //      fSignalPhoton2Info.eta = signalPhoton2->eta();
     //      fSignalPhoton2Info.phi = signalPhoton2->phi();		 
   }

   if(genPhoton1 && genPhoton1->pdgId() == 22) ExoDiPhotons::FillMCTrueObjectInfo(fGenPhoton1Info,genPhoton1);
   if(genPhoton2 && genPhoton2->pdgId() == 22) ExoDiPhotons::FillMCTrueObjectInfo(fGenPhoton2Info,genPhoton2);


   cout << endl;
   cout << "SignalPhoton1 status, pdgId, pt: " << fSignalPhoton1Info.status << ", " << fSignalPhoton1Info.PdgId << ", " << fSignalPhoton1Info.pt << endl;
   cout << "SignalPhoton2 status, pdgId, pt: " << fSignalPhoton2Info.status << ", " << fSignalPhoton2Info.PdgId << ", " << fSignalPhoton2Info.pt << endl;
   cout << endl;
   cout << "GenPhoton1 status, pdgId, pt: " << fGenPhoton1Info.status << ", " << fGenPhoton1Info.PdgId << ", " << fGenPhoton1Info.pt << endl;
   cout << "GenPhoton2 status, pdgId, pt: " << fGenPhoton2Info.status << ", " << fGenPhoton2Info.PdgId << ", " << fGenPhoton2Info.pt << endl;   
   cout << endl;


   // match gen photon to best reco photon

   ExoDiPhotons::InitRecoPhotonInfo(fRecoPhoton1Info);
   ExoDiPhotons::InitRecoPhotonInfo(fRecoPhoton2Info);

   // for matching signal to reco photons
   const reco::Photon *matchPhoton1 = NULL;
   const reco::Photon *matchPhoton2 = NULL;
    
   //const reco::Photon *tempMatchPhoton1 = NULL;
   //const reco::Photon *tempMatchPhoton2 = NULL;   

   // is 1.0 small enough?
   double minDeltaR1 = 1.0;
   double minDeltaR2 = 1.0;

   // index needed to retrieve reco info 
   int matchPhoton1Index = -1;
   int matchPhoton2Index = -1;
   int phoIndex = -1;
   
   for (reco::PhotonCollection::const_iterator recoPhoton = photonColl->begin(); recoPhoton!=photonColl->end(); recoPhoton++) {	 
     phoIndex++;
     
     cout << "Reco Photons: " << endl;
     cout << "pt, eta, phi: " << recoPhoton->pt() << ", " << recoPhoton->eta() << ", " << recoPhoton->phi() << endl;

     
     // we have the gen photons
     // lets match each separately
     if (genPhoton1 && genPhoton1->pdgId()==22) {
       // there's a CMS function for deltaPhi in DataFormats/Math
       double deltaPhi = reco::deltaPhi(genPhoton1->phi(),recoPhoton->phi());
       double deltaEta = genPhoton1->eta()-recoPhoton->eta();
       double deltaR = TMath::Sqrt(deltaPhi*deltaPhi+deltaEta*deltaEta);
	
       cout << "GenPhoton1 deltaR   : " << deltaR << endl;
       cout << "GenPhoton1 minDeltaR: " << minDeltaR1 << endl;

       if (deltaR < minDeltaR1) {
	 // then this is the best match so far
	 minDeltaR1 = deltaR;
	 matchPhoton1 = &(*recoPhoton); //deref the iter to get what it points to
	 matchPhoton1Index = phoIndex;
       }       
     }
     if (genPhoton2 && genPhoton2->pdgId()==22) {
       // there's a CMS function for deltaPhi in DataFormats/Math
       double deltaPhi = reco::deltaPhi(genPhoton2->phi(),recoPhoton->phi());
       double deltaEta = genPhoton2->eta()-recoPhoton->eta();
       double deltaR = TMath::Sqrt(deltaPhi*deltaPhi+deltaEta*deltaEta);
       
       cout << "GenPhoton2 deltaR   : " << deltaR << endl;
       cout << "GenPhoton2 minDeltaR: " << minDeltaR2 << endl;
		 
       if (deltaR < minDeltaR2) {
	 // then this is the best match so far
	 minDeltaR2 = deltaR;
	 matchPhoton2 = &(*recoPhoton); //deref the iter to get what it points to
	 matchPhoton2Index = phoIndex;
       }       
     }
     
   } //end recoPhoton loop to match to the present signal photon
	       


   // reorder match photons by pt
   // (lose direct genPhoton  matchPhoton correspondance)
   if (matchPhoton1 && matchPhoton2) {
     if (matchPhoton2->pt() > matchPhoton1->pt()) {
       const reco::Photon *tempMatchPhoton = matchPhoton1;
       int tempMatchPhotonIndex = matchPhoton1Index;
       matchPhoton1 = matchPhoton2;
       matchPhoton1Index = matchPhoton2Index;
       matchPhoton2 = tempMatchPhoton;
       matchPhoton2Index = tempMatchPhotonIndex;
     }
   }
   if (!matchPhoton1 && matchPhoton2) {
     const reco::Photon *tempMatchPhoton = matchPhoton2;
     int tempMatchPhotonIndex = matchPhoton2Index;
     matchPhoton2 = matchPhoton1; // = null
     matchPhoton2Index = matchPhoton1Index; // = -1
     matchPhoton1 = tempMatchPhoton;
     matchPhoton1Index = tempMatchPhotonIndex;
   }
   
   cout << endl;



   /*
     for(reco::GenParticleCollection::const_iterator genParticle = genParticles->begin(); genParticle != genParticles->end(); ++genParticle) {
        
     // identify the status 1 (ie no further decay) photons
     // which came from the hard-scatt photons (status 3)
     
     if (genParticle->pdgId()==5100039) {
       //cout << "RS Graviton status: " << genParticle->status() << endl;
       //       cout << "\t Daughter: " << genParticle->daughter() << endl;
     }
     
     if(genParticle->status()==1 && genParticle->pdgId()==22 && genParticle->mother()->pdgId() == 22) {
       //cout << "Status 1 photon fromHardProcessFinalState()." << endl;
       cout << "Status 1 photon with mother photon." << endl;
       cout << "isPromptFinalState(), fromHardProcessFinalState(), isLastCopy(): " << genParticle->isPromptFinalState() << ", "
	    << genParticle->fromHardProcessFinalState() << ", " << genParticle->isLastCopy() << endl;
       cout << "============================================================================================================" << endl;
       
           
       if (genParticle->numberOfMothers()>0) {
	 cout << "\tNumber of mothers : " << genParticle->numberOfMothers() << endl;
	 cout << "\tMother: Status, PDGID: " << genParticle->mother()->status() << ", " << genParticle->mother()->pdgId() << endl;
	 if (genParticle->mother()->numberOfMothers()>0) {
	   cout << "\t\tNumber of mothers : " << genParticle->mother()->numberOfMothers() << endl;
	   cout << "\t\tMother: Status, PDGID: " << genParticle->mother()->mother()->status() << ", " << genParticle->mother()->mother()->pdgId() << endl;
	   if (genParticle->mother()->mother()->numberOfMothers()>0) {
	     cout << "\t\t\tNumber of mothers : " << genParticle->mother()->mother()->numberOfMothers() << endl;
	     cout << "\t\t\tMother: Status, PDGID: " << genParticle->mother()->mother()->mother()->status() << ", " << genParticle->mother()->mother()->mother()->pdgId() << endl;
	     if (genParticle->mother()->mother()->mother()->numberOfMothers()>0) {
	       cout << "\t\t\t\tNumber of mothers : " << genParticle->mother()->mother()->mother()->numberOfMothers() << endl;
	       cout << "\t\t\t\tMother: Status, PDGID: " << genParticle->mother()->mother()->mother()->mother()->status() << ", " << genParticle->mother()->mother()->mother()->mother()->pdgId() << endl;
	     }
	   }
	 }
       }
      

       
	 // now match this signal photon to best recoPhoton
	 const reco::Photon *tempMatchPhoton = NULL;
	 double minDeltaR = 1.0;
	       
	 int phoIndex = -1;
	       
	 for(reco::PhotonCollection::const_iterator recoPhoton = photonColl->begin(); recoPhoton!=photonColl->end(); recoPhoton++) {	 
		 
	   phoIndex++;
		 
	   // there's a CMS function for deltaPhi in DataFormats/Math
	   double deltaPhi = reco::deltaPhi(genParticle->phi(),recoPhoton->phi());
	   double deltaEta = genParticle->eta()-recoPhoton->eta();
	   double deltaR = TMath::Sqrt(deltaPhi*deltaPhi+deltaEta*deltaEta);
		 
	   if(deltaR<minDeltaR) {
	     // then this is the best match so far
	     minDeltaR = deltaR;
	     tempMatchPhoton = &(*recoPhoton); //deref the iter to get what it points to
		   
	   }
	 } //end recoPhoton loop to match to the present signal photon
	       
	 // now assign our signal and matched photons to 1 or 2
	 if(!signalPhoton1) {
	   // then we havent found the first one yet, so this is it
	   //signalPhoton1 = &(*genParticle);
	   //cout<<signalPhoton1<<endl;
	   matchPhoton1 = tempMatchPhoton;
	   matchPhoton1Index = phoIndex;
	 }
	 else {
	   // we have already found one, so this is the second
	   //signalPhoton2 = &(*genParticle);
	   matchPhoton2 = tempMatchPhoton;
	   matchPhoton2Index = phoIndex;
	 }
	       
      
     } //end status 1 req for  photons from RS graviton
     
     // identify other real photons in event
     // what about ISR photons? 
     
     //cout << "MC particle: Status = "<< genParticle->status() << "; pdg id = "<< genParticle->pdgId() << "; pt, eta, phi = " << genParticle->pt() << ", "<< genParticle->eta() << ", " << genParticle->phi() << endl;	   
     
     // what about a photon which converts late in detector?
     // (or, similarly, an electron which brems a photon)
     // how does this, which happens after pythia, get saved in event record?
     
     // or what if it is not even a photon at all, but say a jet?
     // try printing all final state particles above pt cut
     // but remember that jets can have lots of particles!
     // so maybe best not to cut on pt of each particle?
     //      if(genParticle->status()==1) {
     
     //        cout << "MC particle: Status = "<< genParticle->status() << "; pdg id = "<< genParticle->pdgId() << "; pt, eta, phi = " << genParticle->pt() << ", "<< genParticle->eta() << ", " << genParticle->phi();	   
     //        if(genParticle->numberOfMothers()>0) {
     // 	 cout << "; Mother pdg ID = " << genParticle->mother()->pdgId();
     //        }
     //        cout << endl;
     //      }

     
   } //end loop over gen particles
   */
   
   
   // if no match found, then the match photon pointers are certainly NULL
   if(matchPhoton1) {		 
     //      cout << "Matched to signal photon!" <<endl;
     //      cout << "Reco photon et, eta, phi = " << matchPhoton1->et() <<", "<<matchPhoton1->eta()<< ", "<< matchPhoton1->phi();
     //      cout << "; eMax/e3x3 = " << matchPhoton1->maxEnergyXtal()/matchPhoton1->e3x3();
     //if(iEvent.id().event()==19486)
     //{
     //  cout << "MatchPhoton1: Event number: " << iEvent.id().event();
     // cout << "; et = " << matchPhoton1->et() << endl;
     // cout << "; hadOverEm = " << matchPhoton1->hadronicOverEm() << endl;
     // cout << "; trkIso = " << matchPhoton1->trkSumPtHollowConeDR04() << endl;
     // cout << "; trkIsoCut = " << (2.0+0.001*matchPhoton1->pt()+0.0167*fRho25) << endl;
     // cout << "; ecalIso = " << matchPhoton1->ecalRecHitSumEtConeDR04() << endl;
     // cout << "; ecalIsoCut = " << (4.2 + 0.006*matchPhoton1->pt()+0.183*fRho25) << endl;
     // cout << "; hcalIso = " << matchPhoton1->hcalTowerSumEtConeDR04() << endl;
     // cout << "; hcalIsoCut = " << (2.2 + 0.0025*matchPhoton1->pt()+0.062*fRho25) << endl;
     // cout << "; pixelSeed = " << matchPhoton1->hasPixelSeed() << endl;
     // cout << "; sigmaIetaIeta = " << matchPhoton1->sigmaIetaIeta() << endl;
     // cout << "; isGap = " <<  ExoDiPhotons::isGapPhoton(&(*matchPhoton1)) << endl;
     // cout << "; isSpike = " <<  ExoDiPhotons::isASpike(&(*matchPhoton1)) << endl; 
     // cout << "; passesHadOverEm = " << ExoDiPhotons::passesHadOverEmCut(matchPhoton1) << endl;
     // cout << ": passesTrkIso = " << ExoDiPhotons::passesTrkIsoCut(matchPhoton1,fRho25) << endl;
     // cout << "; passesEcalIsoCut = " << ExoDiPhotons::passesEcalIsoCut(matchPhoton1,fRho25) << endl;
     // cout << "; passesHcalIsoCut = " << ExoDiPhotons::passesHcalIsoCut(matchPhoton1,fRho25) << endl;
     // cout << "; passesSigmaIetaIetaCut = " << ExoDiPhotons::passesSigmaIetaIetaCut(matchPhoton1) << endl;
     // cout << endl << endl;
     //}
     
     
     // fill info into tree
     ExoDiPhotons::FillRecoPhotonInfo(fRecoPhoton1Info,matchPhoton1,lazyTools_.get(),recHitsEB,recHitsEE,ch_status,iEvent,iSetup);
     // SIC add all this to the function above?
     fRecoPhoton1Info.hasMatchedPromptElec = ConversionTools::hasMatchedPromptElectron(matchPhoton1->superCluster(), hElectrons, hConversions, beamSpot.position());


     edm::Ptr<reco::Photon> matchPho1Ptr(photonColl,matchPhoton1Index);

     fRecoPhoton1Info.sigmaIetaIeta = (*full5x5SigmaIEtaIEtaMap)[matchPho1Ptr];
     fRecoPhoton1Info.PFIsoCharged03 = (*phoChargedIsolationMap)[matchPho1Ptr];
     fRecoPhoton1Info.PFIsoNeutral03 = (*phoNeutralHadronIsolationMap)[matchPho1Ptr];
     fRecoPhoton1Info.PFIsoPhoton03 = (*phoPhotonIsolationMap)[matchPho1Ptr];
     fRecoPhoton1Info.PFIsoAll03 = fRecoPhoton1Info.PFIsoCharged03 + fRecoPhoton1Info.PFIsoNeutral03 + fRecoPhoton1Info.PFIsoPhoton03;

     
     float matchPho1Eta = abs(matchPho1Ptr->superCluster()->eta());
     
     //now the corrected PF isolation variables
     fRecoPhoton1Info.rhocorPFIsoCharged03 = std::max((float)0.0,(float)fRecoPhoton1Info.PFIsoCharged03-rho_*effAreaChHadrons_.getEffectiveArea(matchPho1Eta));
     fRecoPhoton1Info.rhocorPFIsoNeutral03 = std::max((float)0.0,(float)fRecoPhoton1Info.PFIsoNeutral03-rho_*effAreaChHadrons_.getEffectiveArea(matchPho1Eta));
     fRecoPhoton1Info.rhocorPFIsoPhoton03 = std::max((float)0.0,(float)fRecoPhoton1Info.PFIsoPhoton03-rho_*effAreaChHadrons_.getEffectiveArea(matchPho1Eta));
     fRecoPhoton1Info.rhocorPFIsoAll03 = fRecoPhoton1Info.rhocorPFIsoCharged03 + fRecoPhoton1Info.rhocorPFIsoNeutral03 + fRecoPhoton1Info.rhocorPFIsoPhoton03;


     //fill 02 and 04 cones?



     //we retrieve the effective areas
     //Remember effareaCH = 1st, effareaNH = 2nd, effareaPH = 3rd
     // std::vector<double> photon1TorFEffAreas = ExoDiPhotons::EffectiveAreas(matchPhoton1);
     // double pfisoall = isolator03.fGetIsolation(matchPhoton1,pfCandidates,firstVtx,vertexColl);
     // double rhocorPFIsoCH = max(isolator03.getIsolationCharged()-fRho25*photon1TorFEffAreas[0],0.);
     //double rhocorPFIsoNH = max(isolator03.getIsolationNeutral()-fRho25*photon1TorFEffAreas[1],0.);
     //double rhocorPFIsoPH = max(isolator03.getIsolationPhoton()-fRho25*photon1TorFEffAreas[2],0.);
     
     //fRecoPhoton1Info.PFIsoAll03 = pfisoall;
     //fRecoPhoton1Info.PFIsoCharged03 = isolator03.getIsolationCharged();
     //fRecoPhoton1Info.PFIsoNeutral03 = isolator03.getIsolationNeutral();
     //fRecoPhoton1Info.PFIsoPhoton03 = isolator03.getIsolationPhoton();      
/*
     fRecoPhoton1Info.PFIsoAll04 = isolator04.fGetIsolation(matchPhoton1,pfCandidates,firstVtx,vertexColl);
     fRecoPhoton1Info.PFIsoCharged04 = isolator04.getIsolationCharged();
     fRecoPhoton1Info.PFIsoNeutral04 = isolator04.getIsolationNeutral();
     fRecoPhoton1Info.PFIsoPhoton04 = isolator04.getIsolationPhoton();      

     fRecoPhoton1Info.PFIsoAll02 = isolator02.fGetIsolation(matchPhoton1,pfCandidates,firstVtx,vertexColl);
     fRecoPhoton1Info.PFIsoCharged02 = isolator02.getIsolationCharged();
     fRecoPhoton1Info.PFIsoNeutral02 = isolator02.getIsolationNeutral();
     fRecoPhoton1Info.PFIsoPhoton02 = isolator02.getIsolationPhoton();      

     //now the corrected PF isolation variables
     fRecoPhoton1Info.rhocorPFIsoCharged04 = max(fRecoPhoton1Info.PFIsoCharged04-fRho25*photon1TorFEffAreas[0],0.);
     fRecoPhoton1Info.rhocorPFIsoNeutral04 = max(fRecoPhoton1Info.PFIsoNeutral04-fRho25*photon1TorFEffAreas[1],0.);
     fRecoPhoton1Info.rhocorPFIsoPhoton04 = max(fRecoPhoton1Info.PFIsoPhoton04-fRho25*photon1TorFEffAreas[2],0.);
     fRecoPhoton1Info.rhocorPFIsoAll04 = fRecoPhoton1Info.rhocorPFIsoCharged04 + fRecoPhoton1Info.rhocorPFIsoNeutral04 + fRecoPhoton1Info.rhocorPFIsoPhoton04;
 */
     //fRecoPhoton1Info.rhocorPFIsoCharged03 = rhocorPFIsoCH;
     //fRecoPhoton1Info.rhocorPFIsoNeutral03 = rhocorPFIsoNH;
     //fRecoPhoton1Info.rhocorPFIsoPhoton03 = rhocorPFIsoPH;
     //fRecoPhoton1Info.rhocorPFIsoAll03 = fRecoPhoton1Info.rhocorPFIsoCharged03 + fRecoPhoton1Info.rhocorPFIsoNeutral03 + fRecoPhoton1Info.rhocorPFIsoPhoton03;
     /*
     fRecoPhoton1Info.rhocorPFIsoCharged02 = max(fRecoPhoton1Info.PFIsoCharged02-fRho25*photon1TorFEffAreas[0],0.);
     fRecoPhoton1Info.rhocorPFIsoNeutral02 = max(fRecoPhoton1Info.PFIsoNeutral02-fRho25*photon1TorFEffAreas[1],0.);
     fRecoPhoton1Info.rhocorPFIsoPhoton02 = max(fRecoPhoton1Info.PFIsoPhoton02-fRho25*photon1TorFEffAreas[2],0.);
     fRecoPhoton1Info.rhocorPFIsoAll02 = fRecoPhoton1Info.rhocorPFIsoCharged02 + fRecoPhoton1Info.rhocorPFIsoNeutral02 + fRecoPhoton1Info.rhocorPFIsoPhoton02;
     */

     
     fRecoPhoton1Info.isTightPFPhoton = (*tight_id_decisions)[matchPho1Ptr];
     fRecoPhoton1Info.isMediumPFPhoton = (*medium_id_decisions)[matchPho1Ptr];
     fRecoPhoton1Info.isLoosePFPhoton = (*loose_id_decisions)[matchPho1Ptr]; 


     // fill DET info?

     /*
     // now we fill the ID variables for det and PFID
     if(ExoDiPhotons::isTightPhoton(&(*matchPhoton1),fRho25) &&
        !ExoDiPhotons::isGapPhoton(&(*matchPhoton1)) &&
        !ExoDiPhotons::isASpike(&(*matchPhoton1))  )
       fRecoPhoton1Info.isTightDetPhoton = true;
     else
       fRecoPhoton1Info.isTightDetPhoton = false;
     */

     /*    
     // test the conversion safe electron veto
     bool passElecVeto = !ConversionTools::hasMatchedPromptElectron(matchPhoton1->superCluster(), hElectrons, hConversions, beamSpot.position());
     if(ExoDiPhotons::isPFTightPhoton(&(*matchPhoton1),rhocorPFIsoCH,rhocorPFIsoNH,rhocorPFIsoPH,passElecVeto,TString("Tight")) && 
	!ExoDiPhotons::isGapPhoton(&(*matchPhoton1)) && 
	!ExoDiPhotons::isASpike(&(*matchPhoton1))  )
       fRecoPhoton1Info.isTightPFPhoton = true;
     else
       fRecoPhoton1Info.isTightPFPhoton = false;
     if(ExoDiPhotons::isPFTightPhoton(&(*matchPhoton1),rhocorPFIsoCH,rhocorPFIsoNH,rhocorPFIsoPH,passElecVeto,TString("Medium")) && 
	!ExoDiPhotons::isGapPhoton(&(*matchPhoton1)) && 
	!ExoDiPhotons::isASpike(&(*matchPhoton1))  )
       fRecoPhoton1Info.isMediumPFPhoton = true;
     else
       fRecoPhoton1Info.isMediumPFPhoton = false;
     if(ExoDiPhotons::isPFTightPhoton(&(*matchPhoton1),rhocorPFIsoCH,rhocorPFIsoNH,rhocorPFIsoPH,passElecVeto,TString("Loose")) && 
	!ExoDiPhotons::isGapPhoton(&(*matchPhoton1)) && 
	!ExoDiPhotons::isASpike(&(*matchPhoton1))  )
       fRecoPhoton1Info.isLoosePFPhoton = true;
     else
       fRecoPhoton1Info.isLoosePFPhoton = false;
     */  
   }
   else {
     //     cout << "No match to signal photon1!" <<endl;
     // as short cut for indicating this in tree
     // make sure the pt value is crazy
     fRecoPhoton1Info.pt = -777.777;
   }
   
   if(matchPhoton2) {		 
     //      cout << "Matched to signal photon!" <<endl;
     //      cout << "Reco photon et, eta, phi = " << matchPhoton2->et() <<", "<<matchPhoton2->eta()<< ", "<< matchPhoton2->phi();
     //      cout << "; eMax/e3x3 = " << matchPhoton2->maxEnergyXtal()/matchPhoton2->e3x3();
     //if(iEvent.id().event()==19486)
     //{
     //  cout << "MatchPhoton2: Event number: " << iEvent.id().event();
     // cout << "; et = " << matchPhoton2->et() << endl;
     // cout << "; hadOverEm = " << matchPhoton2->hadronicOverEm() << endl;
     // cout << "; trkIso = " << matchPhoton2->trkSumPtHollowConeDR04() << endl;
     // cout << "; trkIsoCut = " << (2.0+0.001*matchPhoton2->pt()+0.0167*fRho25) << endl;
     // cout << "; ecalIso = " << matchPhoton2->ecalRecHitSumEtConeDR04() << endl;
     // cout << "; ecalIsoCut = " << (4.2 + 0.006*matchPhoton2->pt()+0.183*fRho25) << endl;
     // cout << "; hcalIso = " << matchPhoton2->hcalTowerSumEtConeDR04() << endl;
     // cout << "; hcalIsoCut = " << (2.2 + 0.0025*matchPhoton2->pt()+0.062*fRho25) << endl;
     // cout << "; pixelSeed = " << matchPhoton2->hasPixelSeed() << endl;
     // cout << "; sigmaIetaIeta = " << matchPhoton2->sigmaIetaIeta() << endl;
     // cout << "; isGap = " <<  ExoDiPhotons::isGapPhoton(&(*matchPhoton2)) << endl;
     // cout << "; isSpike = " <<  ExoDiPhotons::isASpike(&(*matchPhoton2)) << endl; 
     // cout << "; passesHadOverEm = " << ExoDiPhotons::passesHadOverEmCut(matchPhoton2) << endl;
     // cout << ": passesTrkIso = " << ExoDiPhotons::passesTrkIsoCut(matchPhoton2,fRho25) << endl;
     // cout << "; passesEcalIsoCut = " << ExoDiPhotons::passesEcalIsoCut(matchPhoton2,fRho25) << endl;
     // cout << "; passesHcalIsoCut = " << ExoDiPhotons::passesHcalIsoCut(matchPhoton2,fRho25) << endl;
     // cout << "; passesSigmaIetaIetaCut = " << ExoDiPhotons::passesSigmaIetaIetaCut(matchPhoton2) << endl;
     // cout << endl << endl;
     //}


     // fill info into tree
     ExoDiPhotons::FillRecoPhotonInfo(fRecoPhoton2Info,matchPhoton2,lazyTools_.get(),recHitsEB,recHitsEE,ch_status,iEvent, iSetup);
     // SIC add all this to the function above?
     fRecoPhoton2Info.hasMatchedPromptElec = ConversionTools::hasMatchedPromptElectron(matchPhoton2->superCluster(), hElectrons, hConversions, beamSpot.position());



     edm::Ptr<reco::Photon> matchPho2Ptr(photonColl,matchPhoton2Index);
     

     fRecoPhoton2Info.sigmaIetaIeta = (*full5x5SigmaIEtaIEtaMap)[matchPho2Ptr];
     fRecoPhoton2Info.PFIsoCharged03 = (*phoChargedIsolationMap)[matchPho2Ptr];
     fRecoPhoton2Info.PFIsoNeutral03 = (*phoNeutralHadronIsolationMap)[matchPho2Ptr];
     fRecoPhoton2Info.PFIsoPhoton03 = (*phoPhotonIsolationMap)[matchPho2Ptr];
     fRecoPhoton2Info.PFIsoAll03 = fRecoPhoton2Info.PFIsoCharged03 + fRecoPhoton2Info.PFIsoNeutral03 + fRecoPhoton2Info.PFIsoPhoton03;

     
     float matchPho2Eta = abs(matchPho2Ptr->superCluster()->eta());
     
     //now the corrected PF isolation variables
     fRecoPhoton2Info.rhocorPFIsoCharged03 = std::max((float)0.0,(float)fRecoPhoton2Info.PFIsoCharged03-rho_*effAreaChHadrons_.getEffectiveArea(matchPho2Eta));
     fRecoPhoton2Info.rhocorPFIsoNeutral03 = std::max((float)0.0,(float)fRecoPhoton2Info.PFIsoNeutral03-rho_*effAreaChHadrons_.getEffectiveArea(matchPho2Eta));
     fRecoPhoton2Info.rhocorPFIsoPhoton03 = std::max((float)0.0,(float)fRecoPhoton2Info.PFIsoPhoton03-rho_*effAreaChHadrons_.getEffectiveArea(matchPho2Eta));
     fRecoPhoton2Info.rhocorPFIsoAll03 = fRecoPhoton2Info.rhocorPFIsoCharged03 + fRecoPhoton2Info.rhocorPFIsoNeutral03 + fRecoPhoton2Info.rhocorPFIsoPhoton03;


     //fill 02 and 04 cones?


     /*
     //we retrieve the effective areas
     //Remember effareaCH = 1st, effareaNH = 2nd, effareaPH = 3rd
     //Now we store all PF isolation variables for the 2nd photon
     std::vector<double> photon2TorFEffAreas = ExoDiPhotons::EffectiveAreas(matchPhoton2);
     double pfisoall = isolator03.fGetIsolation(matchPhoton2,pfCandidates,firstVtx,vertexColl);
     double rhocorPFIsoCH = max(isolator03.getIsolationCharged()-fRho25*photon2TorFEffAreas[0],0.);
     double rhocorPFIsoNH = max(isolator03.getIsolationNeutral()-fRho25*photon2TorFEffAreas[1],0.);
     double rhocorPFIsoPH = max(isolator03.getIsolationPhoton()-fRho25*photon2TorFEffAreas[2],0.);
     
     fRecoPhoton2Info.PFIsoAll04 = isolator04.fGetIsolation(matchPhoton2,pfCandidates,firstVtx,vertexColl);
     fRecoPhoton2Info.PFIsoCharged04 = isolator04.getIsolationCharged();
     fRecoPhoton2Info.PFIsoNeutral04 = isolator04.getIsolationNeutral();
     fRecoPhoton2Info.PFIsoPhoton04 = isolator04.getIsolationPhoton();      

     fRecoPhoton2Info.PFIsoAll03 = pfisoall;
     fRecoPhoton2Info.PFIsoCharged03 = isolator03.getIsolationCharged();
     fRecoPhoton2Info.PFIsoNeutral03 = isolator03.getIsolationNeutral();
     fRecoPhoton2Info.PFIsoPhoton03 = isolator03.getIsolationPhoton();      
    
     fRecoPhoton2Info.PFIsoAll02 = isolator02.fGetIsolation(matchPhoton2,pfCandidates,firstVtx,vertexColl);
     fRecoPhoton2Info.PFIsoCharged02 = isolator02.getIsolationCharged();
     fRecoPhoton2Info.PFIsoNeutral02 = isolator02.getIsolationNeutral();
     fRecoPhoton2Info.PFIsoPhoton02 = isolator02.getIsolationPhoton();      

     //now the corrected PF isolation variables
     fRecoPhoton2Info.rhocorPFIsoCharged04 = max(fRecoPhoton2Info.PFIsoCharged04-fRho25*photon2TorFEffAreas[0],0.);
     fRecoPhoton2Info.rhocorPFIsoNeutral04 = max(fRecoPhoton2Info.PFIsoNeutral04-fRho25*photon2TorFEffAreas[1],0.);
     fRecoPhoton2Info.rhocorPFIsoPhoton04 = max(fRecoPhoton2Info.PFIsoPhoton04-fRho25*photon2TorFEffAreas[2],0.);
     fRecoPhoton2Info.rhocorPFIsoAll04 = fRecoPhoton2Info.rhocorPFIsoCharged04 + fRecoPhoton2Info.rhocorPFIsoNeutral04 + fRecoPhoton2Info.rhocorPFIsoPhoton04;

     fRecoPhoton2Info.rhocorPFIsoCharged03 = rhocorPFIsoCH;
     fRecoPhoton2Info.rhocorPFIsoNeutral03 = rhocorPFIsoNH;
     fRecoPhoton2Info.rhocorPFIsoPhoton03 = rhocorPFIsoPH;
     fRecoPhoton2Info.rhocorPFIsoAll03 = fRecoPhoton2Info.rhocorPFIsoCharged03 + fRecoPhoton2Info.rhocorPFIsoNeutral03 + fRecoPhoton2Info.rhocorPFIsoPhoton03;

     fRecoPhoton2Info.rhocorPFIsoCharged02 = max(fRecoPhoton2Info.PFIsoCharged02-fRho25*photon2TorFEffAreas[0],0.);
     fRecoPhoton2Info.rhocorPFIsoNeutral02 = max(fRecoPhoton2Info.PFIsoNeutral02-fRho25*photon2TorFEffAreas[1],0.);
     fRecoPhoton2Info.rhocorPFIsoPhoton02 = max(fRecoPhoton2Info.PFIsoPhoton02-fRho25*photon2TorFEffAreas[2],0.);
     fRecoPhoton2Info.rhocorPFIsoAll02 = fRecoPhoton2Info.rhocorPFIsoCharged02 + fRecoPhoton2Info.rhocorPFIsoNeutral02 + fRecoPhoton2Info.rhocorPFIsoPhoton02;
     */


     fRecoPhoton2Info.isTightPFPhoton = (*tight_id_decisions)[matchPho2Ptr];
     fRecoPhoton2Info.isMediumPFPhoton = (*medium_id_decisions)[matchPho2Ptr];
     fRecoPhoton2Info.isLoosePFPhoton = (*loose_id_decisions)[matchPho2Ptr]; 


     // fill DET info?

     /*
     // now we fill the ID variables for det and PFID
     if(ExoDiPhotons::isTightPhoton(matchPhoton2,fRho25) &&
        !ExoDiPhotons::isGapPhoton(matchPhoton2) &&
        !ExoDiPhotons::isASpike(matchPhoton2)  )
       fRecoPhoton2Info.isTightDetPhoton = true;
     else
       fRecoPhoton2Info.isTightDetPhoton = false;
    
     // test the conversion safe electron veto
     bool passElecVeto = !ConversionTools::hasMatchedPromptElectron(matchPhoton2->superCluster(), hElectrons, hConversions, beamSpot.position());
     if(ExoDiPhotons::isPFTightPhoton(&(*matchPhoton2),rhocorPFIsoCH,rhocorPFIsoNH,rhocorPFIsoPH,passElecVeto,TString("Tight")) && 
	!ExoDiPhotons::isGapPhoton(&(*matchPhoton2)) && 
	!ExoDiPhotons::isASpike(&(*matchPhoton2))  )
       fRecoPhoton2Info.isTightPFPhoton = true;
     else
       fRecoPhoton2Info.isTightPFPhoton = false;
     if(ExoDiPhotons::isPFTightPhoton(&(*matchPhoton2),rhocorPFIsoCH,rhocorPFIsoNH,rhocorPFIsoPH,passElecVeto,TString("Medium")) && 
	!ExoDiPhotons::isGapPhoton(&(*matchPhoton2)) && 
	!ExoDiPhotons::isASpike(&(*matchPhoton2))  )
       fRecoPhoton2Info.isMediumPFPhoton = true;
     else
       fRecoPhoton2Info.isMediumPFPhoton = false;
     if(ExoDiPhotons::isPFTightPhoton(&(*matchPhoton2),rhocorPFIsoCH,rhocorPFIsoNH,rhocorPFIsoPH,passElecVeto,TString("Loose")) && 
	!ExoDiPhotons::isGapPhoton(&(*matchPhoton2)) && 
	!ExoDiPhotons::isASpike(&(*matchPhoton2))  )
       fRecoPhoton2Info.isLoosePFPhoton = true;
     else
       fRecoPhoton2Info.isLoosePFPhoton = false;
     */
   }
   else {
     //     cout << "No match to signal photon2!" <<endl;
     // as short cut for indicating this in tree
     // make sure the pt value is crazy
     fRecoPhoton2Info.pt = -777.777;
   }

   
   cout << "RecoPhoton1 pt, eta, phi: " << fRecoPhoton1Info.pt << ", " << fRecoPhoton1Info.eta << ", " << fRecoPhoton1Info.phi << endl;
   cout << "RecoPhoton2 pt, eta, phi: " << fRecoPhoton2Info.pt << ", " << fRecoPhoton2Info.eta << ", " << fRecoPhoton2Info.phi << endl;
   cout << endl;
   cout << endl;

   // put in diphoton info?
   // (both for signal and reco photons?)

   // fill diphoton info

   // start with silly values
   ExoDiPhotons::InitDiphotonInfo(fDiphotonSignalInfo);
   ExoDiPhotons::InitDiphotonInfo(fDiphotonGenInfo);
   ExoDiPhotons::InitDiphotonInfo(fDiphotonRecoInfo);
   
   if (signalPhoton1 && signalPhoton2) {
     ExoDiPhotons::FillDiphotonInfo(fDiphotonSignalInfo,signalPhoton1,signalPhoton2);
   }

   if (genPhoton1 && genPhoton2 && genPhoton1->pdgId()==22 && genPhoton2->pdgId()==22) {
     ExoDiPhotons::FillDiphotonInfo(fDiphotonGenInfo,genPhoton1,genPhoton2);
   }

   if (matchPhoton1 && matchPhoton2) {
     ExoDiPhotons::FillDiphotonInfo(fDiphotonRecoInfo,matchPhoton1,matchPhoton2);
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
