#include "Analyzer.h"

//// Used to convert Enums to integers
#define ival(x) static_cast<int>(x)
//// BIG_NUM = sqrt(sizeof(int)) so can use diparticle convention of
//// index = BIG_NUM * i1 + i2
//// This insures easy way to extract indices
//// Needs to be changed if go to size_t instead (if want to play safe
#define BIG_NUM 46340

///// Macros defined to shorten code.  Made since lines used A LOT and repeative.  May change to inlines
///// if tests show no loss in speed
#define histAddVal2(val1, val2, name) ihisto.addVal(val1, val2, group, max, name, wgt, syst)
#define histAddVal(val, name) ihisto.addVal(val, group, max, name, wgt, syst)
#define SetBranch(name, variable) BOOM->SetBranchStatus(name, 1);  BOOM->SetBranchAddress(name, &variable);

typedef vector<int>::iterator vec_iter;

//////////////////////////////////////////////////////////////////
///////////////////CONSTANTS DEFINITONS///////////////////////////
//////////////////////////////////////////////////////////////////

//Filespace that has all of the .in files
const string PUSPACE = "Pileup/";


//////////PUBLIC FUNCTIONS////////////////////

const vector<CUTS> Analyzer::genCuts = {
  CUTS::eGTau, CUTS::eNuTau, CUTS::eGTop,
  CUTS::eGElec, CUTS::eGMuon, CUTS::eGZ,
  CUTS::eGW, CUTS::eGHiggs, CUTS::eGJet
};

const vector<CUTS> Analyzer::jetCuts = {
  CUTS::eRJet1,  CUTS::eRJet2,   CUTS::eRCenJet,
  CUTS::eR1stJet, CUTS::eR2ndJet, CUTS::eRBJet
};




const unordered_map<CUTS, vector<CUTS>, EnumHash> Analyzer::adjList = {
  {CUTS::eMuon1Tau1, {CUTS::eRMuon1, CUTS::eRTau1}},
  {CUTS::eMuon1Tau2, {CUTS::eRMuon1, CUTS::eRTau2}},
  {CUTS::eMuon2Tau1, {CUTS::eRMuon2, CUTS::eRTau1}},
  {CUTS::eMuon2Tau2, {CUTS::eRMuon2, CUTS::eRTau2}},

  {CUTS::eElec1Tau1, {CUTS::eRElec1, CUTS::eRTau1}},
  {CUTS::eElec1Tau2, {CUTS::eRElec1, CUTS::eRTau2}},
  {CUTS::eElec2Tau1, {CUTS::eRElec2, CUTS::eRTau1}},
  {CUTS::eMuon2Tau2, {CUTS::eRElec2, CUTS::eRTau2}},

  {CUTS::eMuon1Elec1, {CUTS::eRMuon1, CUTS::eRElec1}},
  {CUTS::eMuon1Elec2, {CUTS::eRMuon1, CUTS::eRElec2}},
  {CUTS::eMuon2Elec1, {CUTS::eRMuon2, CUTS::eRElec1}},
  {CUTS::eMuon2Elec2, {CUTS::eRMuon2, CUTS::eRElec2}},

  {CUTS::eDiElec, {CUTS::eRElec1, CUTS::eRElec2, CUTS::eR1stJet, CUTS::eR2ndJet}},
  {CUTS::eDiMuon, {CUTS::eRMuon1, CUTS::eRMuon2, CUTS::eR1stJet, CUTS::eR2ndJet}},
  {CUTS::eDiTau, {CUTS::eRTau1, CUTS::eRTau2, CUTS::eR1stJet, CUTS::eR2ndJet}},
  {CUTS::eDiJet, {CUTS::eRJet1, CUTS::eRJet2}},
  {CUTS::eSusyCom, {CUTS::eR1stJet, CUTS::eR2ndJet}},
  {CUTS::eGTau, {CUTS::eNuTau}},
  {CUTS::eGen, genCuts}
};


const unordered_map<string, CUTS> Analyzer::cut_num = {
  {"NGenTau", CUTS::eGTau},                             {"NGenTop", CUTS::eGTop},
  {"NGenElectron", CUTS::eGElec},                       {"NGenMuon", CUTS::eGMuon},
  {"NGenZ", CUTS::eGZ},                                 {"NGenW", CUTS::eGW},
  {"NGenHiggs", CUTS::eGHiggs},                         {"NGenJet", CUTS::eGJet},
  {"NRecoMuon1", CUTS::eRMuon1},                        {"NRecoMuon2", CUTS::eRMuon2},
  {"NRecoElectron1", CUTS::eRElec1},                    {"NRecoElectron2",CUTS::eRElec2},
  {"NRecoTau1", CUTS::eRTau1},                          {"NRecoTau2", CUTS::eRTau2},
  {"NRecoJet1", CUTS::eRJet1},                          {"NRecoJet2", CUTS::eRJet2},
  {"NRecoCentralJet", CUTS::eRCenJet},                  {"NRecoBJet", CUTS::eRBJet},
  {"NRecoTriggers1", CUTS::eRTrig1},                    {"NRecoTriggers2", CUTS::eRTrig2},
  {"NRecoFirstLeadingJet", CUTS::eR1stJet},             {"NRecoSecondLeadingJet", CUTS::eR2ndJet},
  {"NDiMuonCombinations", CUTS::eDiMuon},               {"NDiElectronCombinations", CUTS::eDiElec},
  {"NDiTauCombinations", CUTS::eDiTau},                 {"NDiJetCombinations", CUTS::eDiJet},
  {"NMuon1Tau1Combinations", CUTS::eMuon1Tau1},         {"NMuon1Tau2Combinations", CUTS::eMuon1Tau2},
  {"NMuon2Tau1Combinations", CUTS::eMuon2Tau1},         {"NMuon2Tau2Combinations", CUTS::eMuon2Tau2},
  {"NElectron1Tau1Combinations", CUTS::eElec1Tau1},     {"NElectron1Tau2Combinations", CUTS::eElec1Tau2},
  {"NElectron2Tau1Combinations", CUTS::eElec2Tau1},     {"NElectron2Tau2Combinations", CUTS::eElec2Tau2},
  {"NMuon1Electron1Combinations", CUTS::eMuon1Elec1},   {"NMuon1Electron2Combinations", CUTS::eMuon1Elec2},
  {"NMuon2Electron1Combinations", CUTS::eMuon2Elec1},   {"NMuon2Electron2Combinations", CUTS::eMuon2Elec2},
  {"NLeadJetCombinations", CUTS::eSusyCom},             {"METCut", CUTS::eMET},
  {"NRecoWJet", CUTS::eRWjet},                          {"NRecoVertex", CUTS::eRVertex}
};


//////////////////////////////////////////////////////
//////////////////PUBLIC FUNCTIONS////////////////////
//////////////////////////////////////////////////////

///Constructor
Analyzer::Analyzer(vector<string> infiles, string outfile, bool setCR, string configFolder) : goodParts(getArray()) {
  cout << "setup start" << endl;

  BOOM= new TChain("TNT/BOOM");
  infoFile=0;

  //gEnv->SetValue("TFile.Recover", 0);


  for( string infile: infiles){
    //TFile* tmp;
    //tmp = TFile::Open(infile.c_str());
    //if(!tmp) {
      //cout << endl << endl << "File " << infile << " did not open correctly, exiting" <<endl;
      //exit(EXIT_FAILURE);
    //}

    BOOM->AddFile(infile.c_str());
  }


  nentries = (int) BOOM->GetEntries();
  BOOM->SetBranchStatus("*", 0);
  std::cout << "TOTAL EVENTS: " << nentries << std::endl;

  srand(0);

  for(int i=0; i < nTrigReq; i++) {
    vector<int>* tmpi = new vector<int>();
    vector<string>* tmps = new vector<string>();
    trigPlace[i] = tmpi;
    trigName[i] = tmps;
  }

  filespace=configFolder;
  filespace+="/";

  setupGeneral();

  reader.load(calib, BTagEntry::FLAV_B, "comb");


  isData = distats["Run"].bmap.at("isData");

  CalculatePUSystematics = distats["Run"].bmap.at("CalculatePUSystematics");
  initializePileupInfo(distats["Run"].smap.at("MCHistos"), distats["Run"].smap.at("DataHistos"),distats["Run"].smap.at("DataPUHistName"),distats["Run"].smap.at("MCPUHistName"));
  if(!isData) {
    for(auto &it : distats["Systematics"].bmap) {
      if( it.first == "useSystematics")
        doSystematics= it.second;
      else if( it.second and doSystematics) {
        syst_names.push_back(it.first);
        syst_parts.push_back(getArray());
      }
    }
    _Gen = new Generated(BOOM, filespace + "Gen_info.in", syst_names);
  }else{
    doSystematics=false;
  }
  _Electron = new Electron(BOOM, filespace + "Electron_info.in", syst_names);
  _Muon = new Muon(BOOM, filespace + "Muon_info.in", syst_names);
  _Tau = new Taus(BOOM, filespace + "Tau_info.in", syst_names);
  _Jet = new Jet(BOOM, filespace + "Jet_info.in", syst_names);
  _FatJet = new FatJet(BOOM, filespace + "FatJet_info.in", syst_names);
  _MET  = new Met(BOOM,"Met_type1PF", syst_names);

  if(!isData) {
    allParticles= {_Gen,_Electron,_Muon,_Tau,_Jet,_FatJet};
  }else{
    allParticles= {_Electron,_Muon,_Tau,_Jet,_FatJet};
  }

  for(Particle* ipart: allParticles){
    ipart->findExtraCuts();
  }

  vector<string> cr_variables;
  if(setCR) {
    char buf[64];
    read_info(filespace + "Control_Regions.in");
    crbins = pow(2.0, distats["Control_Region"].dmap.size());
    for(auto maper: distats["Control_Region"].dmap) {
      cr_variables.push_back(maper.first);
      sprintf(buf, "%.*G", 16, maper.second);
      cr_variables.push_back(buf);
    }
    if(isData) {
      if(distats["Control_Region"].smap.find("SR") == distats["Control_Region"].smap.end()) {
        cout << "Using Control Regions with data, but no signal region specified can lead to accidentially unblinding a study  before it should be.  Please specify a SR in the file PartDet/Control_Region.in" << endl;
        exit(1);
      } else if(distats["Control_Region"].smap.at("SR").length() != distats["Control_Region"].dmap.size()) {
        cout << "Signal Region specified incorrectly: check signal region variable to make sure the number of variables matches the number of signs in SR" << endl;
        exit(1);
      }
      int factor = 1;
      SignalRegion = 0;
      for(auto gtltSign: distats["Control_Region"].smap["SR"]) {
        if(gtltSign == '>') SignalRegion += factor;
        factor *= 2;
      }
      if(distats["Control_Region"].smap.find("Unblind") != distats["Control_Region"].smap.end()) {

        blinded = distats["Control_Region"].smap["Unblind"] == "false";
	cout << "we have " << blinded << endl;
      }
    }
  }
  //we update the root file if it exist so now we have to delete it:
  std::remove(outfile.c_str()); // delete file
  histo = Histogramer(1, filespace+"Hist_entries.in", filespace+"Cuts.in", outfile, isData, cr_variables);
  if(doSystematics)
    syst_histo=Histogramer(1, filespace+"Hist_syst_entries.in", filespace+"Cuts.in", outfile, isData, cr_variables,syst_names);
  systematics = Systematics(distats);
  jetScaleRes = JetScaleResolution("Pileup/Summer16_23Sep2016V4_MC_Uncertainty_AK4PFchs.txt", "",  "Pileup/Spring16_25nsV6_MC_PtResolution_AK4PFchs.txt", "Pileup/Spring16_25nsV6_MC_SF_AK4PFchs.txt");



  if(setCR) {
    cuts_per.resize(histo.get_folders()->size());
    cuts_cumul.resize(histo.get_folders()->size());
  } else {
    cuts_per.resize(histo.get_cuts()->size());
    cuts_cumul.resize(histo.get_cuts()->size());
  }

  create_fillInfo();
  for(auto maper: distats["Control_Region"].dmap) {

    setupCR(maper.first, maper.second);
  }
  // check if we need to make gen level cuts to cross clean the samples:
  if(infiles[0].find("DY") != string::npos){
    if(infiles[0].find("DYJetsToLL_M-50_HT-") != string::npos){
      gen_selection["DY_noMass_gt_200"]=true;
    //get the DY1Jet DY2Jet ...
    }else if(infiles[0].find("JetsToLL_TuneCUETP8M1_13TeV") != string::npos){
      gen_selection["DY_noMass_gt_200"]=true;
    }else{
      //set it to false!!
      gen_selection["DY_noMass_gt_200"]=false;
    }
    if(infiles[0].find("DYJetsToLL_M-50_TuneCUETP8M1_13TeV") != string::npos){
      gen_selection["DY_noMass_gt_100"]=true;
    }else{
      //set it to false!!
      gen_selection["DY_noMass_gt_100"]=false;
    }
  }else{
    //set it to false!!
    gen_selection["DY_noMass_gt_200"]=false;
    gen_selection["DY_noMass_gt_100"]=false;
  }

  for(auto iselect : gen_selection){
    if(iselect.second){
      cout<<"Waning: The selection "<< iselect.first<< " is active!"<<endl;
    }
  }

  setCutNeeds();

  std::cout << "setup complete" << std::endl << endl;
  start = std::chrono::system_clock::now();
}

unordered_map<CUTS, vector<int>*, EnumHash> Analyzer::getArray() {
  unordered_map<CUTS, vector<int>*, EnumHash> rmap;
  for(auto e: Enum<CUTS>()) {
    rmap[e] = new vector<int>();
  }
  return rmap;
}



void Analyzer::create_fillInfo() {

  fillInfo["FillLeadingJet"] = new FillVals(CUTS::eSusyCom, FILLER::Dipart, _Jet, _Jet);
  fillInfo["FillGen"] =        new FillVals(CUTS::eGen, FILLER::Single, _Gen);
  fillInfo["FillTau1"] =       new FillVals(CUTS::eRTau1, FILLER::Single, _Tau);
  fillInfo["FillTau2"] =       new FillVals(CUTS::eRTau2, FILLER::Single, _Tau);
  fillInfo["FillMuon1"] =      new FillVals(CUTS::eRMuon1, FILLER::Single, _Muon);
  fillInfo["FillMuon2"] =      new FillVals(CUTS::eRMuon2, FILLER::Single, _Muon);
  fillInfo["FillElectron1"] =  new FillVals(CUTS::eRElec1, FILLER::Single, _Electron);
  fillInfo["FillElectron2"] =  new FillVals(CUTS::eRElec2, FILLER::Single, _Electron);

  fillInfo["FillJet1"] =       new FillVals(CUTS::eRJet1, FILLER::Single, _Jet);
  fillInfo["FillJet2"] =       new FillVals(CUTS::eRJet2, FILLER::Single, _Jet);
  fillInfo["FillBJet"] =       new FillVals(CUTS::eRBJet, FILLER::Single, _Jet);
  fillInfo["FillCentralJet"] = new FillVals(CUTS::eRCenJet, FILLER::Single, _Jet);
  fillInfo["FillWJet"] =     new FillVals(CUTS::eRWjet, FILLER::Single, _FatJet);

  fillInfo["FillDiElectron"] = new FillVals(CUTS::eDiElec, FILLER::Dipart, _Electron, _Electron);
  fillInfo["FillDiMuon"] =     new FillVals(CUTS::eDiMuon, FILLER::Dipart, _Muon, _Muon);
  fillInfo["FillDiTau"] =      new FillVals(CUTS::eDiTau, FILLER::Dipart, _Tau, _Tau);
  fillInfo["FillMetCuts"] =    new FillVals();
  fillInfo["FillDiJet"] =      new FillVals(CUTS::eDiJet, FILLER::Dipart, _Jet, _Jet);

  fillInfo["FillMuon1Tau1"] =       new FillVals(CUTS::eMuon1Tau1, FILLER::Dipart, _Muon, _Tau);
  fillInfo["FillMuon1Tau2"] =       new FillVals(CUTS::eMuon1Tau1, FILLER::Dipart, _Muon, _Tau);
  fillInfo["FillMuon2Tau1"] =       new FillVals(CUTS::eMuon2Tau1, FILLER::Dipart, _Muon, _Tau);
  fillInfo["FillMuon2Tau2"] =       new FillVals(CUTS::eMuon2Tau2, FILLER::Dipart, _Muon, _Tau);
  fillInfo["FillElectron1Tau1"] =   new FillVals(CUTS::eElec1Tau1, FILLER::Dipart, _Electron, _Tau);
  fillInfo["FillElectron1Tau2"] =   new FillVals(CUTS::eElec1Tau1, FILLER::Dipart, _Electron, _Tau);
  fillInfo["FillElectron2Tau1"] =   new FillVals(CUTS::eElec2Tau1, FILLER::Dipart, _Electron, _Tau);
  fillInfo["FillElectron2Tau2"] =   new FillVals(CUTS::eElec2Tau2, FILLER::Dipart, _Electron, _Tau);
  fillInfo["FillMuon1Electron1"] =  new FillVals(CUTS::eMuon1Elec1, FILLER::Dipart, _Muon, _Electron);
  fillInfo["FillMuon1Electron2"] =  new FillVals(CUTS::eMuon1Elec1, FILLER::Dipart, _Muon, _Electron);
  fillInfo["FillMuon2Electron1"] =  new FillVals(CUTS::eMuon2Elec1, FILLER::Dipart, _Muon, _Electron);
  fillInfo["FillMuon2Electron2"] =  new FillVals(CUTS::eMuon2Elec2, FILLER::Dipart, _Muon, _Electron);

  //////I hate this solution so much.  Its terrible
  fillInfo["FillElectron1Electron2"] =     new FillVals(CUTS::eDiElec, FILLER::Single, _Electron, _Electron);
  fillInfo["FillMuon1Muon2"] =             new FillVals(CUTS::eDiMuon, FILLER::Single, _Muon, _Muon);
  fillInfo["FillTau1Tau2"] =               new FillVals(CUTS::eDiTau, FILLER::Single, _Tau, _Tau);



  for(auto it: *histo.get_groups()) {
    if(fillInfo[it] == nullptr) fillInfo[it] = new FillVals();
  }

}

void Analyzer::setupCR(string var, double val) {
  smatch m;
  regex part ("^(.+)_(.+)$");
  if(regex_match(var, m, part)) {
    string name = m[1];
    string cut = "Fill" + name;
    if(fillInfo.find(cut) == fillInfo.end()) {
      cout << cut << " not found, put into fillInfo" << endl;
      exit(1);
    }
    cout << cut << " " << m[2] << " " << val << " " << name << endl;
    testVec.push_back(new CRTester(fillInfo.at(cut), m[2], val, name));
  } else {
    cout << "Could not process line: " << var << endl;
    exit(1);
  }

}




////destructor
Analyzer::~Analyzer() {
  clear_values();
  delete BOOM;
  delete _Electron;
  delete _Muon;
  delete _Tau;
  delete _Jet;
  if(!isData) delete _Gen;

  for(auto pair: fillInfo) {
    delete pair.second;
    pair.second=nullptr;
  }

  for(auto e: Enum<CUTS>()) {
    delete goodParts[e];
    goodParts[e]=nullptr;
  }
  //for(auto &it: syst_parts) {
    //for(auto e: Enum<CUTS>()) {
      //if( it[e] != nullptr) {
      //if(it.find(e) != it.end()){
        //delete it[e];
        //it[e]=nullptr;
      //}
      //}
    //}
  //}
  for(auto it: testVec){
    delete it;
    it=nullptr;
  }

  for(int i=0; i < nTrigReq; i++) {
    delete trigPlace[i];
    delete trigName[i];
  }

}


///resets values so analysis can start
void Analyzer::clear_values() {

  for(auto e: Enum<CUTS>()) {
    goodParts[e]->clear();
  }
  //faster!!
  for(auto &it: syst_parts) {
    for(auto e: Enum<CUTS>()) {
      it[e]->clear();
    }
  }

  if(version==1 && infoFile!=BOOM->GetFile()){
    cout<<"New file! Will get the trigger info."<<endl;
    infoFile=BOOM->GetFile();
    BAAM= (TTree*) infoFile->Get("TNT/BAAM");
    initializeTrigger();
    infoFile->Close();
  }


  leadIndex=-1;
  maxCut = 0;
}

///Function that does most of the work.  Calculates the number of each particle
void Analyzer::preprocess(int event) {
  BOOM->GetEntry(event);
  for(Particle* ipart: allParticles){
    ipart->init();
  }

  //call this extra because it is not in the list
  _MET->init();
  active_part = &goodParts;
  if(!select_mc_background()){
    //we will put nothing in good particles
    clear_values();
    return;
  }

  pu_weight = (!isData && CalculatePUSystematics) ? hPU[(int)(nTruePU+1)] : 1.0;


  // SET NUMBER OF GEN PARTICLES
  if(!isData){
    _Gen->cur_P = &_Gen->Reco;
    getGoodGen(_Gen->pstats["Gen"]);
    getGoodTauNu();
  }



  //////Triggers and Vertices
  active_part->at(CUTS::eRVertex)->resize(bestVertices);
  TriggerCuts(*(trigPlace[0]), *(trigName[0]), CUTS::eRTrig1);
  TriggerCuts(*(trigPlace[1]), *(trigName[1]), CUTS::eRTrig2);



  smearLepton(*_Electron, CUTS::eGElec, _Electron->pstats["Smear"]);
  smearLepton(*_Muon, CUTS::eGMuon, _Muon->pstats["Smear"]);
  smearLepton(*_Tau, CUTS::eGTau, _Tau->pstats["Smear"]);

  smearJet(*_Jet, CUTS::eGJet,_Jet->pstats["Smear"]);
  smearJet(*_FatJet, CUTS::eGJet,_FatJet->pstats["Smear"]);

   for(auto name : syst_names) {
     //////Smearing
     smearLepton(*_Electron, CUTS::eGElec, _Electron->pstats["Smear"], name);
     smearLepton(*_Muon, CUTS::eGMuon, _Muon->pstats["Smear"], name);
     smearLepton(*_Tau, CUTS::eGTau, _Tau->pstats["Smear"], name);

     smearJet(*_Jet,CUTS::eGJet,_Jet->pstats["Smear"], name);
     smearJet(*_FatJet,CUTS::eGJet,_FatJet->pstats["Smear"], name);
     updateMet(name);
   }

  //reset all particles to normal:
  for(Particle* ipart: allParticles){
    ipart->setCurrentP("orig");
  }
  //for MET this will be done here:
  updateMet();

  getGoodParticles(-1);

  for(size_t i=0; i < syst_names.size(); i++) {
    string syst=syst_names[i];
    if(syst.find("Met")!=string::npos){
      //does not influence the particle selection:

      for(auto e: Enum<CUTS>()) {
        if(e!=CUTS::eMET){
          continue;
        }
        syst_parts.at(i)[e]=goodParts[e];
      }
      continue;
    }
    getGoodParticles(i);
  }
  active_part = &goodParts;
  if( event < 10 || ( event < 100 && event % 10 == 0 ) ||
    ( event < 1000 && event % 100 == 0 ) ||
    ( event < 10000 && event % 1000 == 0 ) ||
    ( event >= 10000 && event % 10000 == 0 ) ) {
       cout << setprecision(2)<<event << " Events analyzed "<< static_cast<double>(event)/nentries*100. <<"% done"<<endl;
       cout << fixed;
  }
}


void Analyzer::getGoodParticles(int syst_num){

  string syst="";
  if(syst_num != -1){
    active_part=&syst_parts.at(syst_num);
    syst=syst_names[syst_num];
  }

  // // SET NUMBER OF RECO PARTICLES
  // // MUST BE IN ORDER: Muon/Electron, Tau, Jet
  getGoodRecoLeptons(*_Electron, CUTS::eRElec1, CUTS::eGElec, _Electron->pstats["Elec1"],syst);
  getGoodRecoLeptons(*_Electron, CUTS::eRElec2, CUTS::eGElec, _Electron->pstats["Elec2"],syst);
  getGoodRecoLeptons(*_Muon, CUTS::eRMuon1, CUTS::eGMuon, _Muon->pstats["Muon1"],syst);
  getGoodRecoLeptons(*_Muon, CUTS::eRMuon2, CUTS::eGMuon, _Muon->pstats["Muon2"],syst);
  getGoodRecoLeptons(*_Tau, CUTS::eRTau1, CUTS::eGTau, _Tau->pstats["Tau1"],syst);
  getGoodRecoLeptons(*_Tau, CUTS::eRTau2, CUTS::eGTau, _Tau->pstats["Tau2"],syst);

  getGoodRecoJets(CUTS::eRJet1, _Jet->pstats["Jet1"],syst);
  getGoodRecoJets(CUTS::eRJet2, _Jet->pstats["Jet2"],syst);
  getGoodRecoJets(CUTS::eRCenJet, _Jet->pstats["CentralJet"],syst);
  getGoodRecoJets(CUTS::eRBJet, _Jet->pstats["BJet"],syst);
  getGoodRecoJets(CUTS::eR1stJet, _Jet->pstats["FirstLeadingJet"],syst);
  getGoodRecoJets(CUTS::eR2ndJet, _Jet->pstats["SecondLeadingJet"],syst);

  getGoodRecoFatJets(CUTS::eRWjet, _FatJet->pstats["Wjet"],syst);
  treatMuons_Met(syst);

  ///VBF Susy cut on leadin jets
  VBFTopologyCut(distats["VBFSUSY"],syst);

  /////lepton lepton topology cuts
  getGoodLeptonCombos(*_Electron, *_Tau, CUTS::eRElec1,CUTS::eRTau1, CUTS::eElec1Tau1, distats["Electron1Tau1"],syst);
  getGoodLeptonCombos(*_Electron, *_Tau, CUTS::eRElec2, CUTS::eRTau1, CUTS::eElec2Tau1, distats["Electron2Tau1"],syst);
  getGoodLeptonCombos(*_Electron, *_Tau, CUTS::eRElec1, CUTS::eRTau2, CUTS::eElec1Tau2, distats["Electron1Tau2"],syst);
  getGoodLeptonCombos(*_Electron, *_Tau, CUTS::eRElec2, CUTS::eRTau2, CUTS::eElec2Tau2, distats["Electron2Tau2"],syst);

  getGoodLeptonCombos(*_Muon, *_Tau, CUTS::eRMuon1, CUTS::eRTau1, CUTS::eMuon1Tau1, distats["Muon1Tau1"],syst);
  getGoodLeptonCombos(*_Muon, *_Tau, CUTS::eRMuon1, CUTS::eRTau2, CUTS::eMuon1Tau2, distats["Muon1Tau2"],syst);
  getGoodLeptonCombos(*_Muon, *_Tau, CUTS::eRMuon2, CUTS::eRTau1, CUTS::eMuon2Tau1, distats["Muon2Tau1"],syst);
  getGoodLeptonCombos(*_Muon, *_Tau, CUTS::eRMuon2, CUTS::eRTau2, CUTS::eMuon2Tau2, distats["Muon2Tau2"],syst);

  getGoodLeptonCombos(*_Muon, *_Electron, CUTS::eRMuon1, CUTS::eRElec1, CUTS::eMuon1Elec1, distats["Muon1Electron1"],syst);
  getGoodLeptonCombos(*_Muon, *_Electron, CUTS::eRMuon1, CUTS::eRElec2, CUTS::eMuon1Elec2, distats["Muon1Electron2"],syst);
  getGoodLeptonCombos(*_Muon, *_Electron, CUTS::eRMuon2, CUTS::eRElec1, CUTS::eMuon2Elec1, distats["Muon2Electron1"],syst);
  getGoodLeptonCombos(*_Muon, *_Electron, CUTS::eRMuon2, CUTS::eRElec2, CUTS::eMuon2Elec2, distats["Muon2Electron2"],syst);

  ////DIlepton topology cuts
  getGoodLeptonCombos(*_Tau, *_Tau, CUTS::eRTau1, CUTS::eRTau2, CUTS::eDiTau, distats["DiTau"],syst);
  getGoodLeptonCombos(*_Electron, *_Electron, CUTS::eRElec1, CUTS::eRElec2, CUTS::eDiElec, distats["DiElectron"],syst);
  getGoodLeptonCombos(*_Muon, *_Muon, CUTS::eRMuon1, CUTS::eRMuon2, CUTS::eDiMuon, distats["DiMuon"],syst);

  ////Dijet cuts
  getGoodDiJets(distats["DiJet"],syst);
}


////Reads cuts from Cuts.in file and see if the event has enough particles
void Analyzer::fillCuts() {
  const unordered_map<string,pair<int,int> >* cut_info = histo.get_cuts();
  const vector<string>* cut_order = histo.get_cutorder();

  bool prevTrue = true;
  maxCut=0;


  for(size_t i = 0; i < cut_order->size(); i++) {
    string cut = cut_order->at(i);
    if(isData && cut.find("Gen") != string::npos) continue;

    int min= cut_info->at(cut).first;
    int max= cut_info->at(cut).second;
    int nparticles = active_part->at(cut_num.at(cut))->size();
    if( (nparticles >= min) && (nparticles <= max || max == -1)) {
      if((cut_num.at(cut) == CUTS::eR1stJet || cut_num.at(cut) == CUTS::eR2ndJet) && active_part->at(cut_num.at(cut))->at(0) == -1 ) {
        prevTrue = false;
        continue;  ////dirty dirty hack
      }
      cuts_per[i]++;
      cuts_cumul[i] += (prevTrue) ? 1 : 0;
      maxCut += (prevTrue) ? 1 : 0;
    } else prevTrue = false;
  }

}

/////// maxcut made -1 if doesn't pass all of the cuts
void Analyzer::CRfillCuts() {
  const unordered_map<string,pair<int,int> >* cut_info = histo.get_cuts();
  const vector<string>* cut_order = histo.get_cutorder();

  maxCut=0;

  for(size_t i = 0; i < cut_order->size(); i++) {
    string cut = cut_order->at(i);
    if(isData && cut.find("Gen") != string::npos) continue;

    int min= cut_info->at(cut).first;
    int max= cut_info->at(cut).second;
    int nparticles = active_part->at(cut_num.at(cut))->size();
    if( (nparticles < min) || (nparticles > max && max != -1)) {
      maxCut = -1;
      return;
    } else if((cut_num.at(cut) == CUTS::eR1stJet || cut_num.at(cut) == CUTS::eR2ndJet) && active_part->at(cut_num.at(cut))->at(0) == -1 ) {
      maxCut = -1;
      return;
    }
  }

  int factor = crbins;
  for(auto tester: testVec) {
    factor /= 2;
    /////get variable value from maper.first.
    if(tester->test(this)) { ///pass cut
      maxCut += factor;
    }
  }
  if(isData && blinded && maxCut == SignalRegion) return;
  cuts_per[maxCut]++;
}


///Prints the number of events that passed each cut per event and cumulatively
//done at the end of the analysis
void Analyzer::printCuts() {
  vector<string> cut_order;
  if(crbins > 1) cut_order = *(histo.get_folders());
  else cut_order = *(histo.get_cutorder());
  std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_seconds = end-start;
  double run_time_real=elapsed_seconds.count();


  cout.setf(ios::floatfield,ios::fixed);
  cout<<setprecision(3);
  cout << "\n";
  cout << "Selection Efficiency " << "\n";
  cout << "Total events: " << nentries << "\n";
  cout << "\n";
  cout << "Run Time (real): " <<run_time_real <<" s\n";
  cout << "Time per 1k Events (real): " << run_time_real/(nentries/1000) <<" s\n";
  cout << "Events/s: " << static_cast<double>(nentries)/(run_time_real) <<" 1/s (real) \n";
  cout << "                        Name                  Indiv.";
  if(crbins == 1) cout << "            Cumulative";
  cout << endl << "---------------------------------------------------------------------------\n";
  for(size_t i = 0; i < cut_order.size(); i++) {
    cout << setw(28) << cut_order.at(i) << "    ";
    if(isData && cut_order.at(i).find("Gen") != string::npos) cout << "Skipped" << endl;
    else if(crbins != 1 && blinded && i == (size_t)SignalRegion) cout << "Blinded Signal Region" << endl;
    else {
      cout << setw(10) << cuts_per.at(i) << "  ( " << setw(5) << ((float)cuts_per.at(i)) / nentries << ") ";
      if(crbins == 1) cout << setw(12) << cuts_cumul.at(i) << "  ( " << setw(5) << ((float)cuts_cumul.at(i)) / nentries << ") ";

      cout << endl;
    }
  }
  cout << "---------------------------------------------------------------------------\n";

  //write all the histograms
  //attention this is not the fill_histogram method from the Analyser
  histo.fill_histogram();
  if(doSystematics)
    syst_histo.fill_histogram();
}

/////////////PRIVATE FUNCTIONS////////////////


bool Analyzer::select_mc_background(){
  //will return true if Z* mass is smaller than 200GeV
  if(gen_selection["DY_noMass_gt_200"]){
    TLorentzVector lep1;
    TLorentzVector lep2;
    for(size_t i=0; i<_Gen->size(); i++){
      if(abs(_Gen->pdg_id->at(i))==11 or abs(_Gen->pdg_id->at(i))==13 or abs(_Gen->pdg_id->at(i))==15){
        if(lep1!=TLorentzVector(0,0,0,0)){
          lep2= _Gen->p4(i);
          return (lep1+lep2).M()<200;
        }else{
          lep1= _Gen->p4(i);
        }
      }
    }
  }
  //will return true if Z* mass is smaller than 200GeV
  if(gen_selection["DY_noMass_gt_100"]){
    TLorentzVector lep1;
    TLorentzVector lep2;
    for(size_t i=0; i<_Gen->size(); i++){
      if(abs(_Gen->pdg_id->at(i))==11 or abs(_Gen->pdg_id->at(i))==13 or abs(_Gen->pdg_id->at(i))==15){
        if(lep1!=TLorentzVector(0,0,0,0)){
          lep2= _Gen->p4(i);
          return (lep1+lep2).M()<100;
        }else{
          lep1= _Gen->p4(i);
        }
      }
    }
  }
  //cout<<"Something is rotten in the state of Denmark."<<endl;
  //cout<<"could not find gen selection particle"<<endl;
  return true;
}

///Calculates met from values from each file plus smearing and treating muons as neutrinos
void Analyzer::updateMet(string syst) {
  ///---MHT and HT calculations----////
  int i=0;
  double sumpxForMht=0;
  double sumpyForMht=0;
  double sumptForHt=0;
  for(vector<TLorentzVector>::iterator it=_Jet->begin(); it!=_Jet->end(); it++, i++) {
    if( (it->Pt() > distats["Run"].dmap.at("JetPtForMhtAndHt")) && (fabs(it->Eta()) < distats["Run"].dmap.at("JetEtaForMhtAndHt")) ) {
      if(distats["Run"].bmap.at("ApplyJetLooseIDforMhtAndHt") && !passedLooseJetID(i) ) continue;

      sumpxForMht -= it->Px();
      sumpyForMht -= it->Py();
      sumptForHt  += it->Pt();
    }
  }
  _MET->syst_HT[syst]=sumptForHt;
  _MET->syst_MHT[syst]=sumpyForMht;
  _MET->syst_MHTphi[syst]=atan2(sumpyForMht,sumpxForMht);

  _MET->update(syst);

  /////MET CUTS

  if(!passCutRange("Met", _MET->pt(), distats["Run"])) return;
  if(distats["Run"].bmap.at("DiscrByHT") && sumptForHt < distats["Run"].dmap.at("HtCut")) return;

  active_part->at(CUTS::eMET)->push_back(1);
}

void Analyzer::treatMuons_Met(string syst) {

  //syst not implemented for muon as tau or neutrino yet
  if( syst!="orig" or !(distats["Run"].bmap.at("TreatMuonsAsNeutrinos") || distats["Run"].bmap.at("TreatMuonsAsTaus")) ){
    return;
  }

  //  Neutrino update before calculation
  _MET->addP4Syst(_MET->p4(),"muMET");
  _MET->systdeltaMEx["muMET"]=0;
  _MET->systdeltaMEy["muMET"]=0;

  if(distats["Run"].bmap.at("TreatMuonsAsNeutrinos")) {
    for(auto it : *active_part->at(CUTS::eRMuon1)) {
      if(find(active_part->at(CUTS::eRMuon2)->begin(), active_part->at(CUTS::eRMuon2)->end(), it) != active_part->at(CUTS::eRMuon2)->end() ) continue;
      _MET->systdeltaMEx["muMET"] += _Muon->p4(it).Px();
      _MET->systdeltaMEy["muMET"] += _Muon->p4(it).Py();
    }
    for(auto it : *active_part->at(CUTS::eRMuon2)) {
      _MET->systdeltaMEx["muMET"] += _Muon->p4(it).Px();
      _MET->systdeltaMEy["muMET"] += _Muon->p4(it).Py();
    }
  }
  else if(distats["Run"].bmap.at("TreatMuonsAsTaus")) {

    if(active_part->at(CUTS::eRMuon1)->size() == 1) {

      int muon = (int)active_part->at(CUTS::eRMuon1)->at(0);

      double rand1 = 1;//Tau_HFrac->GetRandom();
      double rand2 = 0;//Tau_Resol->GetRandom();

      double ETau_Pt = _Muon->p4(muon).Pt()*rand1*(rand2+1.0);
      double ETau_Eta = _Muon->p4(muon).Eta();
      double ETau_Phi=normPhi(_Muon->p4(muon).Phi());//+DeltaNu_Phi->GetRandom());
      double ETau_Energy = 0.;


      // double theta = 2.0*TMath::ATan2(1.0,TMath::Exp(_Muon->p4(muon).Eta()));
      // double sin_theta = TMath::Sin(theta);
      // double P_tau = ETau_Pt/sin_theta;

      // //ETau_Energy = sqrt(pow(P_tau, 2) + pow(1.77699, 2));
      // ETau_Energy = sqrt( pow(1.77699, 2) + pow(ETau_Pt, 2) + pow(_Muon->p4(muon).Pz(), 2));

      /*if(ETau_Pt <= 15.0){
        while(ETau_Pt<=15.0){
        rand1 = Tau_HFrac->GetRandom();
        rand2 = Tau_Resol->GetRandom();
        ETau_Pt = _Muon->p4(muon).Pt()*rand1*(rand2+1.0);
        ENu_Pt = _Muon->p4(muon).Pt()-ETau_Pt;
        }
      }
      */

      TLorentzVector Emu_Tau;
      Emu_Tau.SetPtEtaPhiE(ETau_Pt, ETau_Eta, ETau_Phi, ETau_Energy);
      _Muon->cur_P->clear();

      if (ETau_Pt >= _Muon->pstats["Muon1"].pmap.at("PtCut").first ){
        _Muon->cur_P->push_back(Emu_Tau);
        _MET->systdeltaMEy["muMET"] += (_Muon->p4(muon).Px()-Emu_Tau.Px());
        _MET->systdeltaMEy["muMET"] += (_Muon->p4(muon).Py()-Emu_Tau.Py());

      }
    }
  }
  // recalculate MET
  _MET->update("muMET");

  /////MET CUTS
  active_part->at(CUTS::eMET)->clear();

  if(passCutRange("Met", _MET->pt(), distats["Run"])) {
    active_part->at(CUTS::eMET)->push_back(1);
  }
}


/////sets up other values needed for analysis that aren't particle specific
void Analyzer::setupGeneral() {

  SetBranch("nTruePUInteractions", nTruePU);
  SetBranch("bestVertices", bestVertices);
  SetBranch("weightevt", gen_weight);
  //SetBranch("rho", rho);

  read_info(filespace + "ElectronTau_info.in");
  read_info(filespace + "MuonTau_info.in");
  read_info(filespace + "MuonElectron_info.in");
  read_info(filespace + "DiParticle_info.in");
  read_info(filespace + "VBFCuts_info.in");
  read_info(filespace + "Run_info.in");
  read_info(filespace + "Systematics_info.in");

  if( BOOM->GetListOfBranches()->FindObject("Trigger_names") ==0){
    SetBranch("Trigger_decision", Trigger_decisionV1);
    infoFile=BOOM->GetFile();
    BAAM= (TTree*) infoFile->Get("TNT/BAAM");
    initializeTrigger();
    infoFile->Close();
    version=1;
  }else{
    SetBranch("Trigger_names", Trigger_names);
    SetBranch("Trigger_decision", Trigger_decision);
    BOOM->GetEntry(0);
    for(int i = 0; i < nTrigReq; i++) {
      for(int j = 0; j < (int)trigName[i]->size(); j++) {
        for(int k = 0; k < (int)Trigger_names->size(); k++) {
          if(Trigger_names->at(k).find(trigName[i]->at(j)) != string::npos) {
            // structure: i tigger 1 or 2 | j  name of trigger in trigger one or two
            trigPlace[i]->at(j) = k;
            break;
          }
        }
      }
    }
    //cout<<Trigger_names->size()<<endl;
    //for(string itrig : *Trigger_names){
      //cout<<itrig<<endl;
    //}
    BOOM->SetBranchStatus("Trigger_names", 0);
  }
}


//get the correct trigger position:
void Analyzer::initializeTrigger() {
  BAAM->SetBranchStatus("triggernames", 1);
  BAAM->SetBranchAddress("triggernames", &Trigger_names);

  //for(string itrig : *Trigger_names){
    //cout<<itrig<<endl;
  //}

  BAAM->GetEntry(0);
  for(int i = 0; i < nTrigReq; i++) {
    for(int j = 0; j < (int)trigName[i]->size(); j++) {
      for(int k = 0; k < (int)Trigger_names->size(); k++) {
        if(Trigger_names->at(k).find(trigName[i]->at(j)) != string::npos) {
          trigPlace[i]->at(j) = k;
          break;
        }
      }
    }
  }
  BAAM->SetBranchStatus("triggernames", 0);
}


///parsing method that gets info on diparts and basic run info
//put in map called "distats"
void Analyzer::read_info(string filename) {
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  ifstream info_file(filename);
  boost::char_separator<char> sep(", \t");

  if(!info_file) {
    std::cout << "could not open file " << filename <<std::endl;
    exit(1);
  }


  string group, line;
  while(getline(info_file, line)) {
    tokenizer tokens(line, sep);
    vector<string> stemp;

    for(tokenizer::iterator iter = tokens.begin();iter != tokens.end(); iter++) {
      if( ((*iter)[0] == '/' && (*iter)[0] == '/') || ((*iter)[0] == '#') ) break;
      stemp.push_back(*iter);
    }
    if(stemp.size() == 0) continue;
    else if(stemp.size() == 1) {
      group = stemp[0];
      continue;
    } else if(group == "") {
      cout << "error in " << filename << "; no groups specified for data" << endl;
      exit(1);
    } else if(stemp.size() == 2) {
      if(stemp.at(0).find("Trigger") != string::npos) {
        int ntrig = (stemp.at(0).find("1") != string::npos) ? 0 : 1;
        trigName[ntrig]->push_back(stemp.at(1));
        trigPlace[ntrig]->push_back(0);
        continue;
      }

      char* p;
      strtod(stemp[1].c_str(), &p);
      if(group.compare("Control_Region") !=0 ){
        if(stemp[1] == "1" || stemp[1] == "true") distats[group].bmap[stemp[0]]=true;
        else if(stemp[1] == "0" || stemp[1] == "false") distats[group].bmap[stemp[0]]=false;
        else if(*p) distats[group].smap[stemp[0]] = stemp[1];
        else  distats[group].dmap[stemp[0]]=stod(stemp[1]);
      }else{
        if(*p) distats[group].smap[stemp[0]] = stemp[1];
        else  distats[group].dmap[stemp[0]]=stod(stemp[1]);
      }

    } else if(stemp.size() == 3) distats[group].pmap[stemp[0]] = make_pair(stod(stemp[1]), stod(stemp[2]));
  }
  info_file.close();
}


// This code works pretty much (at least in my tests), but dagnabit, its ugly.  They all can't be winners, at least now...
void Analyzer::setCutNeeds() {



  for(auto e: Enum<CUTS>()) {
    need_cut[e] = false;
  }

  for(auto it: *histo.get_groups()) {
    if(fillInfo[it]->type == FILLER::None) continue;
    need_cut[fillInfo[it]->ePos] = true;
    if(adjList.find(fillInfo[it]->ePos) == adjList.end()) continue;
    for(auto e: adjList.at(fillInfo[it]->ePos)) {
      need_cut[e] = true;
    }
  }
  for(auto it: *histo.get_cutorder()) {
    try{
      need_cut[cut_num.at(it)] = true;
    }catch(...){
      cout<<"The following cut is strange: "<<it<<endl;
      exit(2);
    }
    if(adjList.find(cut_num.at(it)) == adjList.end()) continue;
    for(auto e: adjList.at(cut_num.at(it))) {
      need_cut[e] = true;
    }
  }

  for(auto it: testVec) {
    CUTS ePos = it->info->ePos;
    need_cut[ePos] = true;
    if(adjList.find(ePos) == adjList.end()) continue;
    for(auto e: adjList.at(ePos)) {
      need_cut[e] = true;
    }
  }

  for(auto it: _Jet->extraCuts) {
    need_cut[it] = true;
    if(adjList.find(it) == adjList.end()) continue;
    for(auto e: adjList.at(it)) {
      need_cut[e] = true;
    }
  }

  for(auto it: jetCuts) {
    if(need_cut[it]) {
      for(auto it2: _Jet->overlapCuts(it)) {
        need_cut[it2] = true;
        if(adjList.find(it2) == adjList.end()) continue;
        for(auto e: adjList.at(it2)) {
          need_cut[e] = true;
        }
      }
    }
  }

  if( !(need_cut[CUTS::eRTau1] || need_cut[CUTS::eRTau2]) ) {
    cout<<"Taus not needed. They will be deactivated!"<<endl;
    _Tau->unBranch();
  } else {
    for(auto it: _Tau->extraCuts) {
      need_cut[it] = true;
      if(adjList.find(it) == adjList.end()) continue;
      for(auto e: adjList.at(it)) {
        need_cut[e] = true;
      }
    }
  }

  if( !(need_cut[CUTS::eRElec1] || need_cut[CUTS::eRElec2]) ) {
    cout<<"Electrons not needed. They will be deactivated!"<<endl;
    _Electron->unBranch();
  } else {
    for(auto it: _Electron->extraCuts) {
      need_cut[it] = true;
      if(adjList.find(it) == adjList.end()) continue;
      for(auto e: adjList.at(it)) {
        need_cut[e] = true;
      }
    }
  }

  if( !(need_cut[CUTS::eRMuon1] || need_cut[CUTS::eRMuon2]) ) {
    cout<<"Muons not needed. They will be deactivated!"<<endl;
    _Muon->unBranch();
  } else {
    for(auto it: _Muon->extraCuts) {
      need_cut[it] = true;
      if(adjList.find(it) == adjList.end()) continue;
      for(auto e: adjList.at(it)) {
        need_cut[e] = true;
      }
    }
  }

  if(isData) return;

  bool passGen = false;
  for(auto e: genCuts) {
    passGen = passGen || need_cut[e];
  }
  for(auto needed : gen_selection){
    passGen = passGen || needed.second;
  }

  if(!passGen){
    cout<<"Gen particles not needed. They will be deactivated!"<<endl;
    _Gen->unBranch();
  }
  else {
    if(need_cut[CUTS::eGTau]) genMaper[15] = new GenFill(2, CUTS::eGTau);
    if(need_cut[CUTS::eGTop]) genMaper[6] = new GenFill(2, CUTS::eGTop);
    if(need_cut[CUTS::eGJet]) genMaper[5] = new GenFill(2, CUTS::eGJet);
    if(need_cut[CUTS::eGElec]) genMaper[11] = new GenFill(1, CUTS::eGElec);
    if(need_cut[CUTS::eGMuon]) genMaper[13] = new GenFill(1, CUTS::eGMuon);
    if(need_cut[CUTS::eGZ]) genMaper[23] = new GenFill(2, CUTS::eGZ);
    if(need_cut[CUTS::eGW]) genMaper[24] = new GenFill(2, CUTS::eGW);
    if(need_cut[CUTS::eGHiggs]) genMaper[25] = new GenFill(2, CUTS::eGHiggs);
    //  , CUTS::eNuTau
  }

}


///Smears lepton only if specified and not a data file.  Otherwise, just filles up lorentz vectors
//of the data into the vector container smearP with is in each lepton object.
void Analyzer::smearLepton(Lepton& lep, CUTS eGenPos, const PartStats& stats, string syst) {
  if( !isData ) {
    if(syst!="orig"){
      //save time to not rerun stuff
      if( syst.find("Muon")==string::npos && lep.type == PType::Muon){
        return;
      }else if( syst.find("Ele")==string::npos && lep.type == PType::Electron){
        return;
      }else if( syst.find("Tau")==string::npos && lep.type == PType::Tau){
        return;
      }
    }
    //if the orig particle should be smeared this vector needs to be cleared
    double scale=stats.dmap.at("PtScaleOffset");
    double resolution=stats.dmap.at("PtScaleOffset");
    if(!stats.bmap.at("SmearTheParticle")) {
      scale=1.;
      resolution=1.;
    }
    double syst_scale=0.;
    double syst_res=0.;
    if(lep.type == PType::Muon){
      syst_scale=distats["Muon_systematics"].dmap.at("scale");
      syst_res=distats["Muon_systematics"].dmap.at("res");
    }else if(lep.type == PType::Electron){
      syst_scale=distats["Electron_systematics"].dmap.at("scale");
      syst_res=distats["Electron_systematics"].dmap.at("res");
    }else if(lep.type == PType::Tau){
      syst_scale=distats["Tau_systematics"].dmap.at("scale");
      syst_res=distats["Tau_systematics"].dmap.at("res");
    }
    bool dores=(syst.find("_Res_")!=string::npos);
    bool doscale=(syst.find("_Scale_")!=string::npos);


    if(syst.find("_Up")!=string::npos){
      syst_scale=1.+syst_scale;
      syst_res=1.+syst_res;
    }else{
      syst_scale=1.-syst_scale;
      syst_res=1.-syst_res;
    }


    if(syst=="orig" && stats.bmap.at("SmearTheParticle")){
      lep.systVec["orig"]->clear();
    }
    if( (syst=="orig" && stats.bmap.at("SmearTheParticle") ) or syst!="orig"){
      //if(lep.size()>0){
        //cout<<"lmlml  "<<  lep.size()<<endl;
        //cout<<dores<<"  jnjbj "<<doscale<< " nknk "<<syst_scale <<"   "<<syst_res <<"  "<<lep.getName()<<"  "<<syst<<endl;
      //}
      for(size_t i = 0; i < lep.Reco.size(); i++) {
        double smearedPt=1.;
        TLorentzVector genVec =  matchLeptonToGen(lep.Reco.at(i), lep.pstats["Smear"],eGenPos);
        if(genVec != TLorentzVector(0,0,0,0)) {
          if(syst=="orig"){
            smearedPt = ((genVec.Pt()*scale) + (lep.Reco[i].Pt() - genVec.Pt())*(resolution))/lep.Reco[i].Pt();
          }else if(dores){
            smearedPt = ((genVec.Pt()*scale) + (lep.Reco[i].Pt() - genVec.Pt())*(syst_res))/lep.Reco[i].Pt();
          }else if(doscale){
            smearedPt = ((genVec.Pt()*(syst_scale)) + (lep.Reco[i].Pt() - genVec.Pt())*(resolution))/lep.Reco[i].Pt();
          }
          //double smearedEta =(genVec.Eta()*stats.dmap.at("EtaScaleOffset")) + (lep.Reco[i].Eta() - genVec.Eta())*stats.dmap.at("EtaSigmaOffset");
          //double smearedPhi = (genVec.Phi() * stats.dmap.at("PhiScaleOffset")) + (lep.Reco[i].Phi() - genVec.Phi())*stats.dmap.at("PhiSigmaOffset");
          //double smearedEnergy = (genVec.Energy()*stats.dmap.at("EnergyScaleOffset")) + (lep.Reco[i].Energy() - genVec.Energy())*stats.dmap.at("EnergySigmaOffset");

        //}else{
          //cout<<"no gen"<<endl;
        }
        systematics.shiftParticle(lep, lep.Reco[i], smearedPt, _MET->systdeltaMEx[syst], _MET->systdeltaMEy[syst], syst);
      }
    }
  }
    //lepton.setCurrentP(syst);
  //}
}

///Same as smearlepton, just jet specific
void Analyzer::smearJet(Particle& jet, const CUTS eGenPos, const PartStats& stats, string syst) {
  //at the moment
  if(jet.type != PType::Jet){
    return;
  }
  //add energy scale uncertainty
  if( !isData || stats.bmap.at("SmearTheJet") || syst!="orig" ){
  //if( !( isData || !stats.bmap.at("SmearTheJet") || syst!="orig" ) ) {
    if(syst=="orig"){
      //only for jets we want to use smearing
      jet.systVec["orig"]->clear();
    }
    for(size_t i=0; i< jet.Reco.size(); i++) {
      if(JetMatchesLepton(*_Muon, jet.Reco[i], stats.dmap.at("MuonMatchingDeltaR"), CUTS::eGMuon) ||
        JetMatchesLepton(*_Tau, jet.Reco[i], stats.dmap.at("TauMatchingDeltaR"), CUTS::eGTau) ||
        JetMatchesLepton(*_Electron, jet.Reco[i],stats.dmap.at("ElectronMatchingDeltaR"), CUTS::eGElec)){
        jet.addP4Syst(jet.Reco[i],syst);
        continue;
      }

      double sf=1.;
      //only apply corrections for jets not for FatJets
      if(jet.type == PType::Jet){

        TLorentzVector genJet=matchJetToGen(jet.Reco[i], jet.pstats["Smear"],eGenPos);
        if(syst=="orig" and stats.bmap.at("SmearTheJet")){
          sf=jetScaleRes.GetRes(jet.Reco[i],genJet, rho, 0);
        }else if(syst=="Jet_Res_Up"){
          sf=jetScaleRes.GetRes(jet.Reco[i],genJet, rho, 1);
        }else if(syst=="Jet_Res_Down"){
          sf=jetScaleRes.GetRes(jet.Reco[i],genJet, rho, -1);
        }else if(syst=="Jet_Scale_Up"){
          sf = jetScaleRes.GetScale(jet.Reco[i], false, +1.);
        }else if(syst=="Jet_Scale_Down"){
          sf = jetScaleRes.GetScale(jet.Reco[i], false, -1) ;
        }
      }
      //if(abs(sf-1)>0.5){
        //cout<<"before: "<<jet.Reco[i].Pt()<<" sf  "<<sf<<" syst "<<syst <<endl;
      //}
      systematics.shiftParticle(jet, jet.Reco[i], sf, _MET->systdeltaMEx[syst], _MET->systdeltaMEy[syst], syst);
      //if(abs(sf-1)>0.5){
        //cout<<"after: "<<jet.systVec[syst]->at(i).Pt()<<endl;
      //}
    }
  }
  jet.setCurrentP(syst);
}


/////checks if jet is close to a lepton and the lepton is a gen particle, then the jet is a lepton object, so
//this jet isn't smeared
bool Analyzer::JetMatchesLepton(const Lepton& lepton, const TLorentzVector& jetV, double partDeltaR, CUTS eGenPos) {
  for(size_t j = 0; j < lepton.size(); j++) {
    if(jetV.DeltaR(lepton.Reco[j]) < partDeltaR && matchLeptonToGen(lepton.Reco[j], lepton.pstats.at("Smear"), eGenPos) != TLorentzVector(0,0,0,0)) return true;
  }
  return false;
}


////checks if reco object matchs a gen object.  If so, then reco object is for sure a correctly identified particle
TLorentzVector Analyzer::matchLeptonToGen(const TLorentzVector& lvec, const PartStats& stats, CUTS ePos) {
  if(ePos == CUTS::eGTau) {
    return matchTauToGen(lvec, stats.dmap.at("GenMatchingDeltaR"));
  }
  for(auto it : *active_part->at(ePos)) {
    if(lvec.DeltaR(_Gen->p4(it)) <= stats.dmap.at("GenMatchingDeltaR")) {
      if(stats.bmap.at("UseMotherID") && abs(_Gen->motherpdg_id->at(it)) != stats.dmap.at("MotherID")) continue;
      return _Gen->p4(it);
    }
  }
  return TLorentzVector(0,0,0,0);
}


///Tau specific matching fucntion.  Works by seeing if a tau doesn't decay into a muon/electron and has
//a matching tau neutrino showing that the tau decayed and decayed hadronically
TLorentzVector Analyzer::matchTauToGen(const TLorentzVector& lvec, double lDeltaR) {
  TLorentzVector genVec(0,0,0,0);
  int i = 0;
  for(vec_iter it=active_part->at(CUTS::eGTau)->begin(); it !=active_part->at(CUTS::eGTau)->end();it++, i++) {
    int nu = active_part->at(CUTS::eNuTau)->at(i);
    if(nu == -1) continue;

    genVec = _Gen->p4(*it) - _Gen->p4(nu);
    if(lvec.DeltaR(genVec) <= lDeltaR) {
      return genVec;
    }
  }
  return genVec;
}


////checks if reco object matchs a gen object.  If so, then reco object is for sure a correctly identified particle
TLorentzVector Analyzer::matchJetToGen(const TLorentzVector& lvec, const PartStats& stats, CUTS ePos) {
  //for the future store gen jets
  for(auto it : *active_part->at(ePos)) {
    if(lvec.DeltaR(_Gen->p4(it)) <= stats.dmap.at("GenMatchingDeltaR")) {
      //nothing more than b quark or gluon
      if( !(abs(_Gen->pdg_id->at(it))<5 || _Gen->pdg_id->at(it)==9 ||  _Gen->pdg_id->at(it)==21) ) continue;
      return _Gen->p4(it);
    }
  }
  return TLorentzVector(0,0,0,0);
}



////checks if reco object matchs a gen object.  If so, then reco object is for sure a correctly identified particle
int Analyzer::matchToGenPdg(const TLorentzVector& lvec, double minDR) {
  double _minDR=minDR;
  int found=-1;
  for(size_t i=0; i< _Gen->size(); i++) {

    if(lvec.DeltaR(_Gen->p4(i)) <=_minDR) {
      //only hard interaction

      if( _Gen->status->at(i)<10){
        found=i;
        _minDR=lvec.DeltaR(_Gen->p4(i));
      }
    }
  }
  if (found>=0){
    return _Gen->pdg_id->at(found);
  }
  return 0;
}


////Calculates the number of gen particles.  Based on id number and status of each particle
void Analyzer::getGoodGen(const PartStats& stats) {
  for(size_t j = 0; j < _Gen->size(); j++) {
    //we are not interested in pythia info here!
    if(_Gen->status->at(j)>10){
      continue;
    }
    int id = abs(_Gen->pdg_id->at(j));
    if(genMaper[id] != nullptr && _Gen->status->at(j) == genMaper[id]->status) {
      if(id == 15 && (_Gen->pt(j) < stats.pmap.at("TauPtCut").first || _Gen->pt(j) > stats.pmap.at("TauPtCut").second || abs(_Gen->eta(j)) > stats.dmap.at("TauEtaCut"))) continue;
      active_part->at(genMaper[id]->ePos)->push_back(j);
      //cout<<"mkmlkmlk"<<endl;
    }
    //something special for jet
    if( (id<5 || id==9 ||  id==21) && genMaper[5] != nullptr && _Gen->status->at(j) == genMaper[5]->status) {
      active_part->at(genMaper[5]->ePos)->push_back(j);
      //cout<<id<<"  "<<_Gen->status->at(j)<<endl;
    }
    //if(_Gen->status->at(j)>10){
      //break;
    //}
    //cout<<id<<"  "<<_Gen->status->at(j)<<endl;

  }

}

////Tau neutrino specific function used for calculating the number of hadronic taus
void Analyzer::getGoodTauNu() {
  for(auto it : *active_part->at(CUTS::eGTau)) {
    bool leptonDecay = false;
    int nu = -1;
    for(size_t j = 0; j < _Gen->size(); j++) {
      if(abs(_Gen->BmotherIndex->at(j)) == (it)) {
        if( (abs(_Gen->pdg_id->at(j)) == 16) && (abs(_Gen->motherpdg_id->at(j)) == 15) && (_Gen->status->at(_Gen->BmotherIndex->at(j)) == 2) ) nu = j;
        else if( (abs(_Gen->pdg_id->at(j)) == 12) || (abs(_Gen->pdg_id->at(j)) == 14) ) leptonDecay = true;
      }
    }
    nu = (leptonDecay) ? -1 : nu;
    active_part->at(CUTS::eNuTau)->push_back(nu);
  }

}

///Function used to find the number of reco leptons that pass the various cuts.
///Divided into if blocks for the different lepton requirements.
void Analyzer::getGoodRecoLeptons(const Lepton& lep, const CUTS ePos, const CUTS eGenPos, const PartStats& stats, const string &syst) {
  if(! need_cut[ePos]) return;
  if(syst!=""){
    //save time to not rerun stuff
    if( syst.find("Muon")==string::npos && lep.type == PType::Muon){
      active_part->at(ePos)=goodParts[ePos];
      return;
    }else if( syst.find("Ele")==string::npos && lep.type == PType::Electron){
      active_part->at(ePos)=goodParts[ePos];
      return;
    }else if( syst.find("Tau")==string::npos && lep.type == PType::Tau){
      active_part->at(ePos)=goodParts[ePos];
      return;
    }
  }
  int i = 0;

  for(vector<TLorentzVector>::const_iterator it=lep.begin(); it != lep.end(); it++, i++) {
    TLorentzVector lvec = (*it);

    if (fabs(lvec.Eta()) > stats.dmap.at("EtaCut")) continue;
    if (lvec.Pt() < stats.pmap.at("PtCut").first || lvec.Pt() > stats.pmap.at("PtCut").second) continue;

    if((lep.pstats.at("Smear").bmap.at("MatchToGen")) && (!isData)) {   /////check
      if(matchLeptonToGen(lvec, lep.pstats.at("Smear") ,eGenPos) == TLorentzVector(0,0,0,0)) continue;
    }

    if (stats.bmap.at("DoDiscrByIsolation")) {
      double firstIso = (stats.pmap.find("IsoSumPtCutValue") != stats.pmap.end()) ? stats.pmap.at("IsoSumPtCutValue").first : ival(ePos) - ival(CUTS::eRTau1) + 1;
      double secondIso = (stats.pmap.find("IsoSumPtCutValue") != stats.pmap.end()) ? stats.pmap.at("IsoSumPtCutValue").second : 0;
      if(!lep.get_Iso(i, firstIso, secondIso)) continue;
    }

    if ((lep.type != PType::Tau) && stats.bmap.at("DiscrIfIsZdecay")) {
      if(isZdecay(lvec, lep)) continue;
    }
    if(!passCutRange("MetDphi", absnormPhi(lvec.Phi() - _MET->phi()), stats)) continue;
    if(!passCutRange("MetMt", calculateLeptonMetMt(lvec), stats)) continue;


    if(lep.type == PType::Muon) {      ////////////////MUON CUTS/////////////
      if(stats.bmap.at("DoDiscrByTightID") && (_Muon->tight->at(i) == 0)) continue;
      if(stats.bmap.at("DoDiscrBySoftID") && (_Muon->soft->at(i) == 0)) continue;

    } else if(lep.type == PType::Electron) {    ///////////////ELECTRON CUT///////////

      //----Require electron to pass ID discriminators
      if(stats.bmap.at("DoDiscrByVetoID") && (_Electron->isPassVeto->at(i) == 0)) continue;
      if(stats.bmap.at("DoDiscrByLooseID") && (_Electron->isPassLoose->at(i) == 0)) continue;
      if(stats.bmap.at("DoDiscrByMediumID") && (_Electron->isPassMedium->at(i) == 0)) continue;
      if(stats.bmap.at("DoDiscrByTightID") && (_Electron->isPassTight->at(i) == 0)) continue;
      if(stats.bmap.at("DoDiscrByHEEPID") && (_Electron->isPassHEEPId->at(i) == 0)) continue;


    } else if(lep.type == PType::Tau) {   /////////////TAU CUT/////////////////
      if (stats.bmap.at("DoDiscrByLeadTrack")) {
        if(_Tau->leadChargedCandPt->at(i) < stats.dmap.at("LeadTrackThreshold")) continue;
      }

      // ----Require 1 or 3 prongs
      if(stats.smap.at("DiscrByProngType").find("hps") != string::npos && _Tau->decayModeFindingNewDMs->at(i) == 0) continue;
      if(!passProng(stats.smap.at("DiscrByProngType"), _Tau->nProngs->at(i))) continue;

      // ----Electron and Muon vetos
      vector<int>* against = (ePos == CUTS::eRTau1) ? _Tau->againstElectron.first : _Tau->againstElectron.second;
      if (stats.bmap.at("DoDiscrAgainstElectron") && against->at(i) == 0) continue;
      else if (stats.bmap.at("SelectTausThatAreElectrons") && against->at(i) > 0) continue;

      against = (ePos == CUTS::eRTau1) ? _Tau->againstMuon.first : _Tau->againstMuon.second;
      if (stats.bmap.at("DoDiscrAgainstMuon") && against->at(i) == 0) continue;
      else if (stats.bmap.at("SelectTausThatAreMuons") && against->at(i) > 0) continue;

      if (stats.bmap.at("DoDiscrByCrackCut") && isInTheCracks(lvec.Eta())) continue;

      // ----anti-overlap requirements
      if (stats.bmap.at("RemoveOverlapWithMuon1s") && isOverlaping(lvec, *_Muon, CUTS::eRMuon1, stats.dmap.at("Muon1MatchingDeltaR"))) continue;
      if (stats.bmap.at("RemoveOverlapWithMuon2s") && isOverlaping(lvec, *_Muon, CUTS::eRMuon2, stats.dmap.at("Muon2MatchingDeltaR"))) continue;
      if (stats.bmap.at("RemoveOverlapWithElectron1s") && isOverlaping(lvec, *_Electron, CUTS::eRElec1, stats.dmap.at("Electron1MatchingDeltaR"))) continue;
      if (stats.bmap.at("RemoveOverlapWithElectron2s") && isOverlaping(lvec, *_Electron, CUTS::eRElec2, stats.dmap.at("Electron2MatchingDeltaR"))) continue;
    }
    active_part->at(ePos)->push_back(i);
  }

}

////Jet specific function for finding the number of jets that pass the cuts.
//used to find the nubmer of good jet1, jet2, central jet, 1st and 2nd leading jets and bjet.
void Analyzer::getGoodRecoJets(CUTS ePos, const PartStats& stats, const string &syst) {
  if(! need_cut[ePos]) return;
  if(syst!=""){
    //save time to not rerun stuff
    if( syst.find("Jet")==string::npos){
      active_part->at(ePos)=goodParts[ePos];
      return;
    }
  }
  int i=0;

  for(vector<TLorentzVector>::iterator it=_Jet->begin(); it != _Jet->end(); it++, i++) {
    TLorentzVector lvec = (*it);
    ///if else loop for central jet requirements

    if( ePos == CUTS::eRCenJet) {
      if(fabs(lvec.Eta()) > 2.5) continue;
    } else if (fabs(lvec.Eta()) < stats.pmap.at("EtaCut").first || fabs(lvec.Eta()) > stats.pmap.at("EtaCut").second) continue;

    if (lvec.Pt() < stats.dmap.at("PtCut")) continue;

    /// BJet specific
    if(ePos == CUTS::eRBJet) {
      if(stats.bmap.at("ApplyJetBTagging") && _Jet->bDiscriminator->at(i) <= stats.dmap.at("JetBTaggingCut")) continue;
      if((stats.bmap.at("MatchBToGen")) && !isData && abs(_Jet->partonFlavour->at(i)) != 5) continue;
    } else if (stats.bmap.at("ApplyLooseID") && !passedLooseJetID(i)) continue; //all other Jets

    // ----anti-overlap requirements
    if(stats.bmap.at("RemoveOverlapWithMuon1s") && isOverlaping(lvec, *_Muon, CUTS::eRMuon1, stats.dmap.at("Muon1MatchingDeltaR"))) continue;
    if(stats.bmap.at("RemoveOverlapWithMuon2s") && isOverlaping(lvec, *_Muon, CUTS::eRMuon2, stats.dmap.at("Muon2MatchingDeltaR"))) continue;
    if(stats.bmap.at("RemoveOverlapWithElectron1s") && isOverlaping(lvec, *_Electron, CUTS::eRElec1, stats.dmap.at("Electron1MatchingDeltaR"))) continue;
    if(stats.bmap.at("RemoveOverlapWithElectron2s") && isOverlaping(lvec, *_Electron, CUTS::eRElec2, stats.dmap.at("Electron2MatchingDeltaR"))) continue;
    if(stats.bmap.at("RemoveOverlapWithTau1s") && isOverlaping(lvec, *_Tau, CUTS::eRTau1, stats.dmap.at("Tau1MatchingDeltaR"))) continue;
    if(stats.bmap.at("RemoveOverlapWithTau2s") && isOverlaping(lvec, *_Tau, CUTS::eRTau2, stats.dmap.at("Tau2MatchingDeltaR"))) continue;

    /////fill up array
    if(ePos == CUTS::eRBJet && stats.bmap.at("UseBtagSF") && !isData) {
      double bjet_SF = reader.eval_auto_bounds("central", BTagEntry::FLAV_B, lvec.Eta(), lvec.Pt());
      if(bjet_SF > 1) {
        cout << "didn't pass" << endl;
      }
      if(((double) rand()/(RAND_MAX)) >  bjet_SF) {
        continue;
      }
    }
    active_part->at(ePos)->push_back(i);
  }



  //clean up for first and second jet
  //note the leading jet has to be selected fist!
  if(ePos == CUTS::eR1stJet || ePos == CUTS::eR2ndJet) {
    int potential = -1;
    double prevPt = -1;
    for(auto leadit : *active_part->at(ePos)) {
      if(((ePos == CUTS::eR2ndJet && (leadit) != leadIndex) || ePos == CUTS::eR1stJet) && _Jet->pt(leadit) > prevPt) {
        potential = leadit;
        prevPt = _Jet->pt(leadit);
      }
    }
    active_part->at(ePos)->clear();
    active_part->at(ePos)->push_back(potential);
    if(ePos == CUTS::eR1stJet) leadIndex = active_part->at(CUTS::eR1stJet)->at(0);
  }

}


////FatJet specific function for finding the number of V-jets that pass the cuts.
void Analyzer::getGoodRecoFatJets(CUTS ePos, const PartStats& stats, const string &syst) {
  if(! need_cut[ePos]) return;
  if(syst!=""){
    //save time to not rerun stuff
    if( syst.find("Jet")==string::npos){
      active_part->at(ePos)=goodParts[ePos];
      return;
    }
  }
  int i=0;

  for(vector<TLorentzVector>::iterator it=_FatJet->begin(); it != _FatJet->end(); it++, i++) {
    TLorentzVector lvec = (*it);
    ///if else loop for central jet requirements

    if (fabs(lvec.Eta()) < stats.pmap.at("EtaCut").first || fabs(lvec.Eta()) > stats.pmap.at("EtaCut").second) continue;

    if (lvec.Pt() < stats.dmap.at("PtCut")) continue;

    /// WJet specific
    if(ePos == CUTS::eRWjet) { //W Tagging
      if(stats.bmap.at("ApplyJetWTagging") &&
        not (_FatJet->tau2->at(i)/_FatJet->tau1->at(i)> stats.pmap.at("JetTau2Tau1Ratio").first &&
          _FatJet->tau2->at(i)/_FatJet->tau1->at(i)< stats.pmap.at("JetTau2Tau1Ratio").second &&
          _FatJet->PrunedMass->at(i) > stats.pmap.at("JetWmassCut").first &&
          _FatJet->PrunedMass->at(i) < stats.pmap.at("JetWmassCut").second)) continue;
    }

    // ----anti-overlap requirements
    if(stats.bmap.at("RemoveOverlapWithMuon1s") && isOverlaping(lvec, *_Muon, CUTS::eRMuon1, stats.dmap.at("Muon1MatchingDeltaR"))) continue;
    if(stats.bmap.at("RemoveOverlapWithMuon2s") && isOverlaping(lvec, *_Muon, CUTS::eRMuon2, stats.dmap.at("Muon2MatchingDeltaR"))) continue;
    if(stats.bmap.at("RemoveOverlapWithElectron1s") && isOverlaping(lvec, *_Electron, CUTS::eRElec1, stats.dmap.at("Electron1MatchingDeltaR"))) continue;
    if(stats.bmap.at("RemoveOverlapWithElectron2s") && isOverlaping(lvec, *_Electron, CUTS::eRElec2, stats.dmap.at("Electron2MatchingDeltaR"))) continue;
    if(stats.bmap.at("RemoveOverlapWithTau1s") && isOverlaping(lvec, *_Tau, CUTS::eRTau1, stats.dmap.at("Tau1MatchingDeltaR"))) continue;
    if(stats.bmap.at("RemoveOverlapWithTau2s") && isOverlaping(lvec, *_Tau, CUTS::eRTau2, stats.dmap.at("Tau2MatchingDeltaR"))) continue;

    /////fill up array
    active_part->at(ePos)->push_back(i);
  }
}

///function to see if a lepton is overlapping with another particle.  Used to tell if jet or tau
//came ro decayed into those leptons
bool Analyzer::isOverlaping(const TLorentzVector& lvec, Lepton& overlapper, CUTS ePos, double MatchingDeltaR) {
  for(auto it : *active_part->at(ePos)) {
    if(lvec.DeltaR(overlapper.p4(it)) < MatchingDeltaR) return true;
  }
  return false;
}

///Tests if tau decays into the specified number of jet prongs.
bool Analyzer::passProng(string prong, int value) {
  return ( (prong.find("1") != string::npos && value == 1) ||
  (prong.find("2") != string::npos && value == 2) ||
  (prong.find("3") != string::npos && value == 3) );
}


////Tests if tau is within the cracks of the detector (the specified eta ranges)
bool Analyzer::isInTheCracks(float etaValue){
  return (fabs(etaValue) < 0.018 ||
  (fabs(etaValue)>0.423 && fabs(etaValue)<0.461) ||
  (fabs(etaValue)>0.770 && fabs(etaValue)<0.806) ||
  (fabs(etaValue)>1.127 && fabs(etaValue)<1.163) ||
  (fabs(etaValue)>1.460 && fabs(etaValue)<1.558));
}


//Tests if a jet meets a litany of different tests
bool Analyzer::passedLooseJetID(int nobj) {
  if (_Jet->neutralHadEnergyFraction->at(nobj) >= 0.99) return false;
  if (_Jet->neutralEmEmEnergyFraction->at(nobj) >= 0.99) return false;
  if (_Jet->numberOfConstituents->at(nobj) <= 1) return false;
  if (_Jet->muonEnergyFraction->at(nobj) >= 0.80) return false;
  if ( (fabs(_Jet->p4(nobj).Eta()) < 2.4) &&
    ((_Jet->chargedHadronEnergyFraction->at(nobj) <= 0.0) ||
    (_Jet->chargedMultiplicity->at(nobj) <= 0.0) ||
    (_Jet->chargedEmEnergyFraction->at(nobj) >= 0.99) )) return false;
  return true;
}


///sees if the event passed one of the two cuts provided
void Analyzer::TriggerCuts(vector<int>& prevTrig, const vector<string>& trigvec, CUTS ePos) {
  if(! need_cut[ePos]) return;
  //cout<<" trigger "<<Trigger_decision->size()<<endl;
  if(version==1){
    for(size_t i = 0; i < trigvec.size(); i++) {
      for(size_t j =0; j<Trigger_decisionV1->size();  j++){
        //cout<<"i:  "<<prevTrig.at(i)<<" j:  "<<j<<" dec(j):  "<<Trigger_decisionV1->at(j)<<endl;
        if(prevTrig.at(i)==Trigger_decisionV1->at(j)){
          active_part->at(ePos)->push_back(0);
          return;
        }
      }
    }
  }else{
    for(int i = 0; i < (int)trigvec.size(); i++) {
      if(Trigger_decision->at(prevTrig.at(i)) == 1) {
        active_part->at(ePos)->push_back(0);
        return;
      }
    }
  }
}


////VBF specific cuts dealing with the leading jets.
void Analyzer::VBFTopologyCut(const PartStats& stats, const string &syst) {
  if(! need_cut[CUTS::eSusyCom]) return;
  if(syst!=""){
    //only jet stuff is affected
    //save time to not rerun stuff
    if( syst.find("Jet")==string::npos){
      active_part->at(CUTS::eSusyCom)=goodParts[CUTS::eSusyCom];
      return;
    }
  }

  if(active_part->at(CUTS::eR1stJet)->at(0) == -1 || active_part->at(CUTS::eR2ndJet)->at(0) == -1) return;

  TLorentzVector ljet1 = _Jet->p4(active_part->at(CUTS::eR1stJet)->at(0));
  TLorentzVector ljet2 = _Jet->p4(active_part->at(CUTS::eR2ndJet)->at(0));

  if(!passCutRange("Mass", (ljet1 + ljet2).M(), stats)) return;
  if(!passCutRange("Pt", (ljet1 + ljet2).Pt(), stats)) return;
  if(!passCutRange("DeltaEta", abs(ljet1.Eta() - ljet2.Eta()), stats)) return;
  if(!passCutRange("DeltaEta", absnormPhi(ljet1.Phi() - ljet2.Phi()), stats)) return;

  if(stats.bmap.at("DiscrByOSEta")) {
    if((ljet1.Eta() * ljet2.Eta()) >= 0) return;
  }

  double dphi1 = normPhi(ljet1.Phi() - _MET->phi());
  double dphi2 = normPhi(ljet2.Phi() - _MET->phi());
  double r1, r2, alpha;

  if(stats.bmap.at("DiscrByR1")) {
    r1 = sqrt( pow(dphi1,2.0) + pow((TMath::Pi() - dphi2),2.0) );
    if(r1 < stats.pmap.at("R1Cut").first || r1 > stats.pmap.at("R1Cut").second) return;

  }
  if(stats.bmap.at("DiscrByR2")) {
    r2 = sqrt( pow(dphi2,2.0) + pow((TMath::Pi() - dphi1),2.0) );
    if(r2 < stats.pmap.at("R2Cut").first || r2 > stats.pmap.at("R2Cut").second) return;
  }
  if(stats.bmap.at("DiscrByAlpha")) {
    TLorentzVector addVec = ljet1 + ljet2;
    alpha = (addVec.M() > 0) ? ljet2.Pt() / addVec.M() : -1;
    if(alpha < stats.pmap.at("AlphaCut").first || alpha > stats.pmap.at("AlphaCut").second) return;
  }
  if( !passCutRange("Dphi1", abs(dphi1), stats)) return;
  if( !passCutRange("Dphi2", abs(dphi2), stats)) return;

  active_part->at(CUTS::eSusyCom)->push_back(0);
}

inline bool Analyzer::passCutRange(string CutName, double value, const PartStats& stats) {
  return ( !(stats.bmap.at("DiscrBy" + CutName)) || (value > stats.pmap.at(CutName + "Cut").first && value < stats.pmap.at(CutName + "Cut").second) );

}


//-----Calculate lepton+met transverse mass
double Analyzer::calculateLeptonMetMt(const TLorentzVector& Tobj) {
  double px = Tobj.Px() + _MET->px();
  double py = Tobj.Py() + _MET->py();
  double et = Tobj.Et() + _MET->energy();
  double mt2 = et*et - (px*px + py*py);
  return (mt2 >= 0) ? sqrt(mt2) : -1;
}


/////Calculate the diparticle mass based on how to calculate it
///can use Collinear Approximation, which can fail (number failed available in a histogram)
///can use VectorSumOfVisProductAndMet which is sum of particles and met
///Other which is adding without met
double Analyzer::diParticleMass(const TLorentzVector& Tobj1, const TLorentzVector& Tobj2, string howCalc) {
  bool ratioNotInRange = false;
  TLorentzVector The_LorentzVect;


  //////check this equation/////
  if(howCalc == "CollinearApprox") {
    double denominator = (Tobj1.Px() * Tobj2.Py()) - (Tobj2.Px() * Tobj1.Py());
    double x1 = (Tobj2.Py()*_MET->px() - Tobj2.Px()*_MET->py())/denominator;
    double x2 = (Tobj1.Px()*_MET->py() - Tobj1.Py()*_MET->px())/denominator;
    ratioNotInRange=!((x1 < 0.) && (x2 < 0.));
    if (!ratioNotInRange) {
      The_LorentzVect.SetPxPyPzE( (Tobj1.Px()*(1 + x1) + Tobj2.Px()*(1+x2)), (Tobj1.Py()*(1+x1) + Tobj2.Py()*(1+x2)), (Tobj1.Pz()*(1+x1) + Tobj2.Pz()*(1+x2)), (Tobj1.Energy()*(1+x1) + Tobj2.Energy()*(1+x2)) );
      return The_LorentzVect.M();
    }
  }

  if(howCalc == "VectorSumOfVisProductsAndMet" || ratioNotInRange) {
    return (Tobj1 + Tobj2 + _MET->p4()).M();
  }

  return (Tobj1 + Tobj2).M();
}

////Tests if the CollinearApproximation works for finding the mass of teh particles
bool Analyzer::passDiParticleApprox(const TLorentzVector& Tobj1, const TLorentzVector& Tobj2, string howCalc) {
  if(howCalc == "CollinearApprox") {
    double x1_numerator = (Tobj1.Px() * Tobj2.Py()) - (Tobj2.Px() * Tobj1.Py());
    double x1_denominator = (Tobj2.Py() * (Tobj1.Px() + _MET->px())) - (Tobj2.Px() * (Tobj1.Py() + _MET->py()));
    double x1 = ( x1_denominator != 0. ) ? x1_numerator/x1_denominator : -1.;
    double x2_numerator = x1_numerator;
    double x2_denominator = (Tobj1.Px() * (Tobj2.Py() + _MET->py())) - (Tobj1.Py() * (Tobj2.Px() + _MET->px()));
    double x2 = ( x2_denominator != 0. ) ? x2_numerator/x2_denominator : -1.;
    return (x1 > 0. && x1 < 1.) && (x2 > 0. && x2 < 1.);
  } else {
    return true;
  }
}


/////abs for values
///Find the number of lepton combos that pass the dilepton cuts
void Analyzer::getGoodLeptonCombos(Lepton& lep1, Lepton& lep2, CUTS ePos1, CUTS ePos2, CUTS ePosFin, const PartStats& stats, const string & syst) {
  if(! need_cut[ePosFin]) return;
  if(syst!=""){
    //save time to not rerun stuff
    if( syst.find("Muon")==string::npos && !(lep1.type == PType::Muon || lep2.type == PType::Muon) ){
      active_part->at(ePosFin)=goodParts[ePosFin];
      return;
    }else if( syst.find("Ele")==string::npos && !(lep1.type == PType::Electron || lep2.type == PType::Electron) ){
      active_part->at(ePosFin)=goodParts[ePosFin];
      return;
    }else if( syst.find("Tau")==string::npos && !(lep1.type == PType::Tau || lep2.type == PType::Tau) ){
      active_part->at(ePosFin)=goodParts[ePosFin];
      return;
    }
  }

  bool sameParticle = (&lep1 == &lep2);
  TLorentzVector part1, part2;

  for(auto i1 : *active_part->at(ePos1)) {
    for(auto i2 : *active_part->at(ePos2)) {
      if(sameParticle && i2 <= i1) continue;
      part1 = lep1.p4(i1);
      part2 = lep2.p4(i2);

      if(stats.bmap.at("DiscrByDeltaR") && (part1.DeltaR(part2)) < stats.dmap.at("DeltaRCut")) continue;



      if (stats.bmap.find("DiscrByOSLSType") != stats.bmap.end() ){
        //if it is 1 or 0 it will end up in the bool map!!
        if(stats.bmap.at("DiscrByOSLSType") && (lep1.charge->at(i1) * lep2.charge->at(i2) <= 0)) continue;
      }else if (stats.dmap.find("DiscrByOSLSType") != stats.dmap.end() ){
        if(lep1.charge->at(i1) * lep2.charge->at(i2) > 0) continue;
      }else if (stats.smap.find("DiscrByOSLSType") != stats.smap.end() ){
        if(stats.smap.at("DiscrByOSLSType") == "LS" && (lep1.charge->at(i1) * lep2.charge->at(i2) <= 0)) continue;
        else if(stats.smap.at("DiscrByOSLSType") == "OS" && (lep1.charge->at(i1) * lep2.charge->at(i2) >= 0)) continue;
      }

      if( !passCutRange("CosDphi", cos(absnormPhi( part1.Phi() - part2.Phi())), stats)) continue;

      // ----Mass window requirement

      if (stats.bmap.at("DiscrByMassReco")) {
        double diMass = diParticleMass(part1,part2, stats.smap.at("HowCalculateMassReco"));
        if( diMass < stats.pmap.at("MassCut").first || diMass > stats.pmap.at("MassCut").second) continue;
      }

      if (stats.bmap.at("DiscrByCDFzeta2D")) {
        double CDFzeta = stats.dmap.at("PZetaCutCoefficient") * getPZeta(part1, part2).first
                + stats.dmap.at("PZetaVisCutCoefficient") * getPZeta(part1, part2).second;
        if( CDFzeta < stats.pmap.at("CDFzeta2DCutValue").first || CDFzeta > stats.pmap.at("CDFzeta2DCutValue").second ) continue;
      }


      if (stats.bmap.at("DiscrByDeltaPtDivSumPt")) {
        double ptDiv = (part1.Pt() - part2.Pt()) / (part1.Pt() + part2.Pt());
        if( ptDiv < stats.pmap.at("DeltaPtDivSumPtCutValue").first || ptDiv > stats.pmap.at("DeltaPtDivSumPtCutValue").second) continue;
      }

      if (stats.bmap.at("DiscrByDeltaPt")) {
        double deltaPt = part1.Pt() - part2.Pt();
        if(deltaPt < stats.pmap.at("DeltaPtCutValue").first || deltaPt > stats.pmap.at("DeltaPtCutValue").second) continue;
      }

      ///Particlesp that lead to good combo are nGen * part1 + part2
      /// final / nGen = part1 (make sure is integer)
      /// final % nGen = part2
      active_part->at(ePosFin)->push_back(i1*BIG_NUM + i2);
    }
  }
}


//////////////LOOK INTO DIJET PICKING
///////HOW TO GET RID OF REDUNCENCIES??

/////Same as gooddilepton, just jet specific
void Analyzer::getGoodDiJets(const PartStats& stats, const string & syst) {
  if(! need_cut[CUTS::eDiJet]) return;
  if(syst!=""){
    //save time to not rerun stuff
    if( syst.find("Jet")==string::npos){
      active_part->at(CUTS::eDiJet)=goodParts[CUTS::eDiJet];
      return;
    }
  }
  TLorentzVector jet1, jet2;
  // ----Separation cut between jets (remove overlaps)
  for(auto ij2 : *active_part->at(CUTS::eRJet2)) {
    jet2 = _Jet->p4(ij2);
    for(auto ij1 : *active_part->at(CUTS::eRJet1)) {
      jet1 = _Jet->p4(ij1);

      if (stats.bmap.at("DiscrByDeltaR")) {
        if(jet1.DeltaR(jet2) < stats.dmap.at("DeltaRCut")) continue;
      }

      if( !passCutRange("DeltaEta", abs(jet1.Eta() - jet2.Eta()), stats) ) continue;
      if( !passCutRange("DeltaPhi", abs(jet1.Phi() - jet2.Phi()), stats) ) continue;

      if (stats.bmap.at("DiscrByOSEta")) {
        if((jet1.Eta() * jet2.Eta()) >= 0) continue;
      }
      // ----Require both legs to be almost back-to-back in phi
      if( !passCutRange("CosDphi", cos(absnormPhi(jet1.Phi() - jet2.Phi())), stats) ) continue;

      // ----Mass window requirement
      if (stats.bmap.at("DiscrByMassReco")) {
        if( ((jet1 + jet2).M() < stats.pmap.at("MassCut").first) || ((jet1 + jet2).M() > stats.pmap.at("MassCut").second) ) continue;
      }
      ///Particlesp that lead to good combo are totjet * part1 + part2
      /// final / totjet = part1 (make sure is integer)
      /// final % totjet = part2
      active_part->at(CUTS::eDiJet)->push_back(ij1*_Jet->size() + ij2);
    }
  }
}

///////Only tested for if is Zdecay, can include massptasymmpair later?
/////Tests to see if a light lepton came form a zdecay
bool Analyzer::isZdecay(const TLorentzVector& theObject, const Lepton& lep) {
  bool eventIsZdecay = false;
  const float zMass = 90.1876;
  const float zWidth = 2.4952;
  float zmmPtAsymmetry = -10.;

  // if mass is within 3 sigmas of z or pt asymmetry is small set to true.
  for(vector<TLorentzVector>::const_iterator lepit= lep.begin(); lepit != lep.end(); lepit++) {
    if(theObject.DeltaR(*lepit) < 0.3) continue;
    if(theObject == (*lepit)) continue;

    TLorentzVector The_LorentzVect = theObject + (*lepit);
    zmmPtAsymmetry = (theObject.Pt() - lepit->Pt()) / (theObject.Pt() + lepit->Pt());

    if( (abs(The_LorentzVect.M() - zMass) < 3.*zWidth) || (fabs(zmmPtAsymmetry) < 0.20) ) {
      eventIsZdecay = true;
      break;
    }
  }

  return eventIsZdecay;
}


///Calculates the Pzeta value
pair<double, double> Analyzer::getPZeta(const TLorentzVector& Tobj1, const TLorentzVector& Tobj2) {
  double zetaX = cos(Tobj1.Phi()) + cos(Tobj2.Phi());
  double zetaY = sin(Tobj1.Phi()) + sin(Tobj2.Phi());
  double zetaR = TMath::Sqrt(zetaX*zetaX + zetaY*zetaY);
  if ( zetaR > 0. ) { zetaX /= zetaR; zetaY /= zetaR; }
  double visPx = Tobj1.Px() + Tobj2.Px();
  double visPy = Tobj1.Py() + Tobj2.Py();
  double px = visPx + _MET->px();
  double py = visPy + _MET->py();
  return make_pair(px*zetaX + py*zetaY, visPx*zetaX + visPy*zetaY);
}



////Grabs a list of the groups of histograms to be filled and asked Fill_folder to fill up the histograms
void Analyzer::fill_histogram() {
  if(distats["Run"].bmap["ApplyGenWeight"] && gen_weight == 0.0) return;

  if(crbins != 1) CRfillCuts();
  else fillCuts();

  if(isData && blinded && maxCut == SignalRegion) return;
  const vector<string>* groups = histo.get_groups();
  if(!isData){
    wgt = pu_weight;
    if(distats["Run"].bmap["ApplyGenWeight"]) wgt *= (gen_weight > 0) ? 1.0 : -1.0;
    //backup current weight
    backup_wgt=wgt;
  }else{
    wgt=1.;
    backup_wgt=wgt;
  }

  for(auto it: *groups) {
    fill_Folder(it, maxCut, histo);
  }

  if(doSystematics){
    const vector<string>* syst_groups = syst_histo.get_groups();
    int isyst=0;
    for(auto name : syst_names) {
      //switch the systematics:
      for(Particle* ipart: allParticles){
        if(ipart->systVec[name]->size()>0)
          ipart->setCurrentP(name);
      }
      _MET->setCurrentP(name);
      active_part =&syst_parts.at(isyst);

      //get the systematics for the last folder:
      for(auto it: *syst_groups) {
        fill_Folder(it, maxCut, syst_histo, isyst);
      }
      isyst++;
    }
    for(Particle* ipart: allParticles){
      ipart->setCurrentP("orig");
    }
    _MET->setCurrentP("orig");
    active_part = &goodParts;
  }

}

///Function that fills up the histograms
void Analyzer::fill_Folder(string group, const int max, Histogramer &ihisto, int syst) {
  /*be aware in this function
   * the following definition is used:
   * histAddVal(val, name) histo.addVal(val, group, max, name, wgt)
   * so each histogram knows the group, max and weight!
   */
  if(group == "FillRun" and syst==-1) {
    if(crbins != 1) {
      for(int i = 0; i < crbins; i++) {
        ihisto.addVal(false, group, i, "Events", 1);
        if(distats["Run"].bmap["ApplyGenWeight"]) {
          //put the weighted events in bin 3
          ihisto.addVal(2, group,i, "Events", (gen_weight > 0) ? 1.0 : -1.0);
        }
      }
    }
    else{
      ihisto.addVal(false, group,ihisto.get_maxfolder(), "Events", 1);
      if(distats["Run"].bmap["ApplyGenWeight"]) {
        //put the weighted events in bin 3
        ihisto.addVal(2, group,ihisto.get_maxfolder(), "Events", (gen_weight > 0) ? 1.0 : -1.0);
      }
    }
    histAddVal(true, "Events");
    histAddVal(bestVertices, "NVertices");
  } else if(!isData && group == "FillGen") {

    int nhadtau = 0;
    TLorentzVector genVec;
    int i = 0;
    for(vec_iter it=active_part->at(CUTS::eGTau)->begin(); it!=active_part->at(CUTS::eGTau)->end(); it++, i++) {

      int nu = active_part->at(CUTS::eNuTau)->at(i);
      if(nu != -1) {
        genVec = _Gen->p4(*it) - _Gen->p4(nu);
        histAddVal(genVec.Pt(), "HadTauPt");
        histAddVal(genVec.Eta(), "HadTauEta");
        nhadtau++;

      }
      histAddVal(_Gen->energy(*it), "TauEnergy");
      histAddVal(_Gen->pt(*it), "TauPt");
      histAddVal(_Gen->eta(*it), "TauEta");
      histAddVal(_Gen->phi(*it), "TauPhi");
      for(vec_iter it2=it+1; it2!=active_part->at(CUTS::eGTau)->end(); it2++) {
        histAddVal(diParticleMass(_Gen->p4(*it),_Gen->p4(*it2), "none"), "DiTauMass");
      }
    }
    histAddVal(active_part->at(CUTS::eGTau)->size(), "NTau");
    histAddVal(nhadtau, "NHadTau");

    for(auto it : *active_part->at(CUTS::eGMuon)) {
      histAddVal(_Gen->energy(it), "MuonEnergy");
      histAddVal(_Gen->pt(it), "MuonPt");
      histAddVal(_Gen->eta(it), "MuonEta");
      histAddVal(_Gen->phi(it), "MuonPhi");
    }
    histAddVal(active_part->at(CUTS::eGMuon)->size(), "NMuon");

    double mass=0;
    TLorentzVector lep1;
    TLorentzVector lep2;
    for(size_t i=0; i<_Gen->size(); i++){
      //if a Z boson is explicitly there
      if(abs(_Gen->pdg_id->at(i))==11 or abs(_Gen->pdg_id->at(i))==13 or abs(_Gen->pdg_id->at(i))==15){
        if(lep1!=TLorentzVector(0,0,0,0)){
          lep2= _Gen->p4(i);
          mass=(lep1+lep2).M();
          //cout<<"mass  leptons "<<mass<<endl;
          break;
        }else{
          lep1= _Gen->p4(i);
        }
      }
    }
    histAddVal(mass, "LeptonMass");
  } else if(fillInfo[group]->type == FILLER::Single) {
    Particle* part = fillInfo[group]->part;
    CUTS ePos = fillInfo[group]->ePos;

    for(auto it : *active_part->at(ePos)) {
      histAddVal(part->p4(it).Energy(), "Energy");
      histAddVal(part->p4(it).Pt(), "Pt");
      histAddVal(part->p4(it).Eta(), "Eta");
      histAddVal(part->p4(it).Phi(), "Phi");
      if(part->type == PType::Tau) {
        histAddVal(_Tau->nProngs->at(it), "NumSignalTracks");
        histAddVal(_Tau->charge->at(it), "Charge");
        histAddVal(_Tau->leadChargedCandPt->at(it), "SeedTrackPt");
      }
      if(part->type != PType::Jet) {
        histAddVal(calculateLeptonMetMt(part->p4(it)), "MetMt");
      }
      if(part->type == PType::FatJet ) {
        histAddVal(_FatJet->PrunedMass->at(it), "PrunedMass");
        histAddVal(_FatJet->SoftDropMass->at(it), "SoftDropMass");
        histAddVal(_FatJet->tau1->at(it), "tau1");
        histAddVal(_FatJet->tau2->at(it), "tau2");
        histAddVal(_FatJet->tau2->at(it)/_FatJet->tau1->at(it), "tau2Overtau1");
      }
    }
    if((part->type != PType::Jet ) && active_part->at(ePos)->size() > 0) {
      double leadpt = 0;
      double leadeta = 0;
      for(auto it : *active_part->at(ePos)) {
        if(part->p4(it).Pt() >= leadpt) {
          leadpt = part->p4(it).Pt();
          leadeta = part->p4(it).Eta();
        }
      }

      histAddVal(leadpt, "FirstLeadingPt");
      histAddVal(leadeta, "FirstLeadingEta");
    }


    histAddVal(active_part->at(ePos)->size(), "N");


  } else if(group == "FillMetCuts") {
    histAddVal(_MET->MHT(), "MHT");
    histAddVal(_MET->HT(), "HT");
    histAddVal(_MET->HT() + _MET->MHT(), "Meff");
    histAddVal(_MET->pt(), "Met");

  } else if(group == "FillLeadingJet" && active_part->at(CUTS::eSusyCom)->size() == 0) {

    if(active_part->at(CUTS::eR1stJet)->at(0) != -1) {
      histAddVal(_Jet->p4(active_part->at(CUTS::eR1stJet)->at(0)).Pt(), "FirstPt");
      histAddVal(_Jet->p4(active_part->at(CUTS::eR1stJet)->at(0)).Eta(), "FirstEta");
    }
    if(active_part->at(CUTS::eR2ndJet)->at(0) != -1) {
      histAddVal(_Jet->p4(active_part->at(CUTS::eR2ndJet)->at(0)).Pt(), "SecondPt");
      histAddVal(_Jet->p4(active_part->at(CUTS::eR2ndJet)->at(0)).Eta(), "SecondEta");
    }


  } else if(group == "FillLeadingJet" && active_part->at(CUTS::eSusyCom)->size() != 0) {

    TLorentzVector first = _Jet->p4(active_part->at(CUTS::eR1stJet)->at(0));
    TLorentzVector second = _Jet->p4(active_part->at(CUTS::eR2ndJet)->at(0));

    histAddVal(first.Pt(), "FirstPt");
    histAddVal(second.Pt(), "SecondPt");

    histAddVal(first.Eta(), "FirstEta");
    histAddVal(second.Eta(), "SecondEta");

    TLorentzVector LeadDiJet = first + second;

    histAddVal(LeadDiJet.M(), "Mass");
    histAddVal(LeadDiJet.Pt(), "Pt");
    histAddVal(fabs(first.Eta() - second.Eta()), "DeltaEta");
    histAddVal(first.DeltaR(second), "DeltaR");

    double dphiDijets = absnormPhi(first.Phi() - second.Phi());
    double dphi1 = normPhi(first.Phi() - _MET->phi());
    double dphi2 = normPhi(second.Phi() - _MET->phi());
    double alpha = (LeadDiJet.M() > 0) ? second.Pt() / LeadDiJet.M() : 999999999.0;

    histAddVal(dphiDijets, "LeadSublDijetDphi");
    histAddVal2(_MET->pt(),dphiDijets, "MetVsDiJetDeltaPhiLeadSubl");
    histAddVal2(fabs(first.Eta()-second.Eta()), dphiDijets, "DeltaEtaVsDeltaPhiLeadSubl");

    histAddVal(absnormPhi(_MET->phi() - LeadDiJet.Phi()), "MetDeltaPhi");



    histAddVal(sqrt( pow(dphi1,2.0) + pow((TMath::Pi() - dphi2),2.0) ), "R1");
    histAddVal(sqrt( pow(dphi2,2.0) + pow((TMath::Pi() - dphi1),2.0)), "R2");
    histAddVal(normPhi(first.Phi() - _MET->MHTphi()), "Dphi1MHT");
    histAddVal(normPhi(second.Phi() - _MET->MHTphi()), "Dphi2MHT");
    histAddVal(dphi1, "Dphi1");
    histAddVal(dphi2, "Dphi2");
    histAddVal2(dphi1,dphi2, "Dphi1VsDphi2");
    histAddVal(alpha, "Alpha");


    //dijet info
  } else if(group == "FillDiJet") {
    double leaddijetmass = 0;
    double leaddijetpt = 0;
    double leaddijetdeltaR = 0;
    double leaddijetdeltaEta = 0;
    double etaproduct = 0;
    for(auto it : *active_part->at(CUTS::eDiJet)) {
      int p1 = (it) / _Jet->size();
      int p2 = (it) % _Jet->size();
      TLorentzVector jet1 = _Jet->p4(p1);
      TLorentzVector jet2 = _Jet->p4(p2);
      TLorentzVector DiJet = jet1 + jet2;

      if(DiJet.M() > leaddijetmass) {
        leaddijetmass = DiJet.M();
        etaproduct = (jet1.Eta() * jet2.Eta() > 0) ? 1 : -1;
      }
      if(DiJet.Pt() > leaddijetpt) leaddijetpt = DiJet.Pt();
      if(fabs(jet1.Eta() - jet2.Eta()) > leaddijetdeltaEta) leaddijetdeltaEta = fabs(jet1.Eta() - jet2.Eta());
      if(jet1.DeltaR(jet2) > leaddijetdeltaR) leaddijetdeltaR = jet1.DeltaR(jet2);

      histAddVal(DiJet.M(), "Mass");
      histAddVal(DiJet.Pt(), "Pt");
      histAddVal(fabs(jet1.Eta() - jet2.Eta()), "DeltaEta");
      histAddVal(absnormPhi(jet1.Phi() - jet2.Phi()), "DeltaPhi");
      histAddVal(jet1.DeltaR(jet2), "DeltaR");
    }


    histAddVal(leaddijetmass, "LargestMass");
    histAddVal(leaddijetpt, "LargestPt");
    histAddVal(leaddijetdeltaEta, "LargestDeltaEta");
    histAddVal(leaddijetdeltaR, "LargestDeltaR");
    histAddVal(etaproduct, "LargestMassEtaProduct");


    ////diparticle stuff
  } else if(fillInfo[group]->type == FILLER::Dipart) {
    Lepton* lep1 = static_cast<Lepton*>(fillInfo[group]->part);
    Lepton* lep2 = static_cast<Lepton*>(fillInfo[group]->part2);
    CUTS ePos = fillInfo[group]->ePos;
    string digroup = group;
    digroup.erase(0,4);

    TLorentzVector part1;
    TLorentzVector part2;

    for(auto it : *active_part->at(ePos)) {

      int p1= (it) / BIG_NUM;
      int p2= (it) % BIG_NUM;

      part1 = lep1->p4(p1);
      part2 = lep2->p4(p2);

      histAddVal2(part1.Pt(),part2.Pt(), "Part1PtVsPart2Pt");
      histAddVal(part1.DeltaR(part2), "DeltaR");
      if(group.find("Di") != string::npos) {
        histAddVal((part1.Pt() - part2.Pt()) / (part1.Pt() + part2.Pt()), "DeltaPtDivSumPt");
        histAddVal(part1.Pt() - part2.Pt(), "DeltaPt");
      } else {
        histAddVal((part2.Pt() - part1.Pt()) / (part1.Pt() + part2.Pt()), "DeltaPtDivSumPt");
        histAddVal(part2.Pt() - part1.Pt(), "DeltaPt");
      }
      histAddVal(cos(absnormPhi(part2.Phi() - part1.Phi())), "CosDphi");
      histAddVal(absnormPhi(part1.Phi() - _MET->phi()), "Part1MetDeltaPhi");
      histAddVal2(absnormPhi(part1.Phi() - _MET->phi()), cos(absnormPhi(part2.Phi() - part1.Phi())), "Part1MetDeltaPhiVsCosDphi");
      histAddVal(absnormPhi(part2.Phi() - _MET->phi()), "Part2MetDeltaPhi");
      histAddVal(cos(absnormPhi(atan2(part1.Py() - part2.Py(), part1.Px() - part2.Px()) - _MET->phi())), "CosDphi_DeltaPtAndMet");

      double diMass = diParticleMass(part1,part2, distats[digroup].smap.at("HowCalculateMassReco"));
      if(passDiParticleApprox(part1,part2, distats[digroup].smap.at("HowCalculateMassReco"))) {
        histAddVal(diMass, "ReconstructableMass");
      } else {
        histAddVal(diMass, "NotReconstructableMass");
      }

      double PZeta = getPZeta(part1,part2).first;
      double PZetaVis = getPZeta(part1,part2).second;
      histAddVal(calculateLeptonMetMt(part1), "Part1MetMt");
      histAddVal(calculateLeptonMetMt(part2), "Part2MetMt");
      histAddVal(lep2->charge->at(p2) * lep1->charge->at(p1), "OSLS");
      histAddVal(PZeta, "PZeta");
      histAddVal(PZetaVis, "PZetaVis");
      histAddVal2(PZetaVis,PZeta, "Zeta2D");
      histAddVal((distats.at(digroup).dmap.at("PZetaCutCoefficient") * PZeta) + (distats.at(digroup).dmap.at("PZetaVisCutCoefficient") * PZetaVis), "Zeta1D");

      if ((active_part->at(CUTS::eR1stJet)->size()>0 && active_part->at(CUTS::eR1stJet)->at(0) != -1) && (active_part->at(CUTS::eR2ndJet)->size()>0 && active_part->at(CUTS::eR2ndJet)->at(0) != -1)) {
        TLorentzVector TheLeadDiJetVect = _Jet->p4(active_part->at(CUTS::eR1stJet)->at(0)) + _Jet->p4(active_part->at(CUTS::eR2ndJet)->at(0));

        histAddVal(absnormPhi(part1.Phi() - TheLeadDiJetVect.Phi()), "Part1DiJetDeltaPhi");
        histAddVal(absnormPhi(part2.Phi() - TheLeadDiJetVect.Phi()), "Part2DiJetDeltaPhi");
        histAddVal(diParticleMass(TheLeadDiJetVect, part1+part2, "VectorSumOfVisProductsAndMet"), "DiJetReconstructableMass");
      }

      if(lep1->type != PType::Tau) {
        histAddVal(isZdecay(part1, *lep1), "Part1IsZdecay");
      }
      if(lep2->type != PType::Tau){
        histAddVal(isZdecay(part2, *lep2), "Part2IsZdecay");
      }


      //electron tau stuff:
      if(lep1->type == PType::Electron && lep2->type == PType::Electron){
        //loop over taus to find a match in the unisolated taus:
        int matchedTauInd=-1;
        TLorentzVector matchedEle;
        TLorentzVector unmatchedEle;
        for( size_t itau =0; itau< _Tau->size(); itau++){
          if(part2.DeltaR(_Tau->p4(itau))<0.3){
            //we are sure that part1 passes the tight id
            matchedTauInd=itau;
            matchedEle=part2;
            unmatchedEle=part1;
          }else if(part1.DeltaR(_Tau->p4(itau))<0.3){
            //check if part2 passes the tight id:
            if(find(active_part->at(CUTS::eRElec1)->begin(),active_part->at(CUTS::eRElec1)->end(),p2)!=active_part->at(CUTS::eRElec1)->end()){
              matchedTauInd=itau;
              matchedEle=part1;
              unmatchedEle=part2;
            }
          }
        }
        if(matchedTauInd>=0){
          if(find(active_part->at(CUTS::eRTau1)->begin(),active_part->at(CUTS::eRTau1)->end(),matchedTauInd)!=active_part->at(CUTS::eRTau1)->end()){
            histAddVal(_Tau->p4(matchedTauInd).Pt(), "DiEleGoodTauMatchPt");
            histAddVal(_Tau->p4(matchedTauInd).Pt()-matchedEle.Pt(), "DiEleGoodTauMatchDeltaPt");
            histAddVal((_Tau->p4(matchedTauInd)+unmatchedEle).M(), "DiEleGoodTauMatchMass");
            histAddVal((matchedEle+unmatchedEle).M(), "DiEleEleGoodMatchMass");
            histAddVal(matchedEle.Pt(), "DiEleEleGoodMatchPt");
            histAddVal(_Tau->leadChargedCandPtError->at(matchedTauInd),"DiEleleadChargedCandPtErrorGoodMatched");
            histAddVal(_Tau->leadChargedCandValidHits->at(matchedTauInd),"DiEleleadChargedCandValidHitGoodMatched");
            histAddVal2( matchedEle.Pt(),   (_Tau->p4(matchedTauInd).Pt()-matchedEle.Pt())/matchedEle.Pt(), "DiEleTauGoodMatchPt_vs_DeltaPt");
            histAddVal2( matchedEle.Pt(),   matchedEle.Eta(), "DiEleTauGoodMatchPt_vs_eta");
            histAddVal2( _Tau->pt(matchedTauInd),   _Tau->decayMode->at(matchedTauInd), "DiEleTauGoodMatchPt_vs_Decay");
          }else{
            histAddVal(_Tau->p4(matchedTauInd).Pt(), "DiEleTauMatchPt");
            histAddVal(_Tau->p4(matchedTauInd).Pt()-matchedEle.Pt(), "DiEleTauMatchDeltaPt");
            histAddVal((_Tau->p4(matchedTauInd)+unmatchedEle).M(), "DiEleTauMatchMass");
            histAddVal((matchedEle+unmatchedEle).M(), "DiEleEleMatchMass");
            histAddVal(matchedEle.Pt(), "DiEleEleMatchPt");
            histAddVal(_Tau->leadChargedCandPtError->at(matchedTauInd),"DiEleleadChargedCandPtErrorMatched");
            histAddVal(_Tau->leadChargedCandValidHits->at(matchedTauInd),"DiEleleadChargedCandValidHitsMatched");
            histAddVal2( matchedEle.Pt(),   (_Tau->p4(matchedTauInd).Pt()-matchedEle.Pt())/matchedEle.Pt(), "DiEleTauMatchPt_vs_DeltaPt");
            histAddVal2( matchedEle.Pt(),   matchedEle.Eta(), "DiEleTauMatchPt_vs_eta");
            histAddVal2( _Tau->pt(matchedTauInd),   _Tau->decayMode->at(matchedTauInd), "DiEleTauMatchPt_vs_Decay");
          }
        }else{
          histAddVal((part1+part2).M(), "DiEleEleUnMatchMass");
          histAddVal(part2.Pt(), "DiEleEleUnMatchPt");
          histAddVal2( part2.Pt(),   part2.Eta(), "DiEleUnMatchPt_vs_eta");
          if(!isData){
            histAddVal(part2.Pt(), "DiEleEleUnMatchPt_gen_"+to_string(abs(matchToGenPdg(part2,0.3))));
          }
          int found=-1;
          for(size_t i=0; i< _Jet->size(); i++) {
            if(part2.DeltaR(_Jet->p4(i)) <=0.4) {
              found=i;
            }
          }
          if (found>=0){
            histAddVal(_Jet->chargedMultiplicity->at(found), "DiEleEleUnMatchJetMultiplicity");
          }else{
            histAddVal(-1, "DiEleEleUnMatchJetMultiplicity");
          }

        }
      }
    }
  }
}


void Analyzer::initializePileupInfo(string MCHisto, string DataHisto, string DataHistoName, string MCHistoName) {

  TFile *file1 = new TFile((PUSPACE+MCHisto).c_str());
  TH1D* histmc = (TH1D*)file1->FindObjectAny(MCHistoName.c_str());
  if(!histmc) throw std::runtime_error("failed to extract histogram");

  TFile* file2 = new TFile((PUSPACE+DataHisto).c_str());
  TH1D* histdata = (TH1D*)file2->FindObjectAny(DataHistoName.c_str());
  if(!histdata) throw std::runtime_error("failed to extract histogram");

  double factor = histmc->Integral() / histdata->Integral();
  double value;
  for(int bin=0; bin < 100; bin++) {
    if(histmc->GetBinContent(bin) == 0) value = 1;

    else value = factor*histdata->GetBinContent(bin) / histmc->GetBinContent(bin);
    hPU[bin] = value;
  }

  file1->Close();
  file2->Close();

}

///Normalizes phi to be between -PI and PI
double normPhi(double phi) {
  static double const TWO_PI = TMath::Pi() * 2;
  while ( phi <= -TMath::Pi() ) phi += TWO_PI;
  while ( phi >  TMath::Pi() ) phi -= TWO_PI;
  return phi;
}


///Takes the absolute value of of normPhi (made because constant use)
double absnormPhi(double phi) {
  return abs(normPhi(phi));
}


