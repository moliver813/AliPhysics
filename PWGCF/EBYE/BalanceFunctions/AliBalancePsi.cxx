/**************************************************************************
 * Author: Panos Christakoglou.                                           *
 * Contributors are mentioned in the code where appropriate.              *
 *                                                                        *
 * Permission to use, copy, modify and distribute this software and its   *
 * documentation strictly for non-commercial purposes is hereby granted   *
 * without fee, provided that the above copyright notice appears in all   *
 * copies and that both the copyright notice and this permission notice   *
 * appear in the supporting documentation. The authors make no claims     *
 * about the suitability of this software for any purpose. It is          *
 * provided "as is" without express or implied warranty.                  *
 **************************************************************************/

/* $Id: AliBalancePsi.cxx 54125 2012-01-24 21:07:41Z miweber $ */

//-----------------------------------------------------------------
//           Balance Function class
//   This is the class to deal with the Balance Function wrt Psi analysis
//   Origin: Panos Christakoglou, Nikhef, Panos.Christakoglou@cern.ch
//-----------------------------------------------------------------


//ROOT
#include <Riostream.h>
#include <TCanvas.h>
#include <TMath.h>
#include <TAxis.h>
#include <TH2D.h>
#include <TH3D.h>
#include <TLorentzVector.h>
#include <TObjArray.h>
#include <TGraphErrors.h>
#include <TString.h>

#include "AliVParticle.h"
#include "AliMCParticle.h"
#include "AliESDtrack.h"
#include "AliAODTrack.h"
#include "AliTHn.h"
#include "AliAnalysisTaskTriggeredBF.h"

#include "AliBalancePsi.h"
using std::cout;
using std::endl;
using std::cerr;

ClassImp(AliBalancePsi)

//____________________________________________________________________//
AliBalancePsi::AliBalancePsi() :
  TObject(), 
  fShuffle(kFALSE),
  fAnalysisLevel("ESD"),
  fAnalyzedEvents(0) ,
  fCentralityId(0) ,
  fCentStart(0.),
  fCentStop(0.),
  fHistP(0),
  fHistN(0),
  fHistPN(0),
  fHistNP(0),
  fHistPP(0),
  fHistNN(0),
  fHistHBTbefore(0),
  fHistHBTafter(0),
  fHistConversionbefore(0),
  fHistConversionafter(0),
  fHistPsiMinusPhi(0),
  fHistResonancesBefore(0),
  fHistResonancesRho(0),
  fHistResonancesK0(0),
  fHistResonancesLambda(0),
  fHistQbefore(0),
  fHistQafter(0),
  fPsiInterval(15.),
  fDeltaEtaMax(2.0),
  fResonancesCut(kFALSE),
  fHBTCut(kFALSE),
  fHBTCutValue(0.02),
  fConversionCut(kFALSE),
  fInvMassCutConversion(0.04),
  fQCut(kFALSE),
  fDeltaPtMin(0.0),
  fVertexBinning(kFALSE),
  fCustomBinning(""),
  fBinningString(""),
  fEventClass("EventPlane"){
  // Default constructor
}

//____________________________________________________________________//
AliBalancePsi::AliBalancePsi(const AliBalancePsi& balance):
  TObject(balance), fShuffle(balance.fShuffle), 
  fAnalysisLevel(balance.fAnalysisLevel),
  fAnalyzedEvents(balance.fAnalyzedEvents), 
  fCentralityId(balance.fCentralityId),
  fCentStart(balance.fCentStart),
  fCentStop(balance.fCentStop),
  fHistP(balance.fHistP),
  fHistN(balance.fHistN),
  fHistPN(balance.fHistPN),
  fHistNP(balance.fHistNP),
  fHistPP(balance.fHistPP),
  fHistNN(balance.fHistNN),
  fHistHBTbefore(balance.fHistHBTbefore),
  fHistHBTafter(balance.fHistHBTafter),
  fHistConversionbefore(balance.fHistConversionbefore),
  fHistConversionafter(balance.fHistConversionafter),
  fHistPsiMinusPhi(balance.fHistPsiMinusPhi),
  fHistResonancesBefore(balance.fHistResonancesBefore),
  fHistResonancesRho(balance.fHistResonancesRho),
  fHistResonancesK0(balance.fHistResonancesK0),
  fHistResonancesLambda(balance.fHistResonancesLambda),
  fHistQbefore(balance.fHistQbefore),
  fHistQafter(balance.fHistQafter),
  fPsiInterval(balance.fPsiInterval),
  fDeltaEtaMax(balance.fDeltaEtaMax),
  fResonancesCut(balance.fResonancesCut),
  fHBTCut(balance.fHBTCut),
  fHBTCutValue(balance.fHBTCutValue),
  fConversionCut(balance.fConversionCut),
  fInvMassCutConversion(balance.fInvMassCutConversion),
  fQCut(balance.fQCut),
  fDeltaPtMin(balance.fDeltaPtMin),
  fVertexBinning(balance.fVertexBinning),
  fCustomBinning(balance.fCustomBinning),
  fBinningString(balance.fBinningString),
  fEventClass("EventPlane"){
  //copy constructor
}

//____________________________________________________________________//
AliBalancePsi::~AliBalancePsi() {
  // Destructor
  delete fHistP;
  delete fHistN;
  delete fHistPN;
  delete fHistNP;
  delete fHistPP;
  delete fHistNN;

  delete fHistHBTbefore;
  delete fHistHBTafter;
  delete fHistConversionbefore;
  delete fHistConversionafter;
  delete fHistPsiMinusPhi;
  delete fHistResonancesBefore;
  delete fHistResonancesRho;
  delete fHistResonancesK0;
  delete fHistResonancesLambda;
  delete fHistQbefore;
  delete fHistQafter;
    
}

//____________________________________________________________________//
void AliBalancePsi::InitHistograms() {
  // single particle histograms

  // global switch disabling the reference 
  // (to avoid "Replacing existing TH1" if several wagons are created in train)
  Bool_t oldStatus = TH1::AddDirectoryStatus();
  TH1::AddDirectory(kFALSE);

  Int_t anaSteps   = 1;       // analysis steps
  Int_t iBinSingle[kTrackVariablesSingle];        // binning for track variables
  Double_t* dBinsSingle[kTrackVariablesSingle];   // bins for track variables  
  TString axisTitleSingle[kTrackVariablesSingle]; // axis titles for track variables
  
  // two particle histograms
  Int_t iBinPair[kTrackVariablesPair];         // binning for track variables
  Double_t* dBinsPair[kTrackVariablesPair];    // bins for track variables  
  TString axisTitlePair[kTrackVariablesPair];  // axis titles for track variables



  // =========================================================
  // The default string (from older versions of AliBalancePsi)
  // =========================================================
  TString defaultBinningStr;
  defaultBinningStr = "multiplicity: 0,10,20,30,40,50,60,70,80,100,100000\n"                // Multiplicity Bins
    "centrality: 0.,5.,10.,20.,30.,40.,50.,60.,70.,80.\n"                                   // Centrality Bins
    "centralityVertex: 0.,5.,10.,15.,20.,25.,30.,35.,40.,45.,50.,55.,60.,65.,70.,75.,80.\n" // Centrality Bins (Vertex Binning)
    "eventPlane: -0.5,0.5,1.5,2.5,3.5\n"                                                    // Event Plane Bins (Psi: -0.5->0.5 (in plane), 0.5->1.5 (intermediate), 1.5->2.5 (out of plane), 2.5->3.5 (rest))
    "deltaEta: -1.6, -1.56, -1.52, -1.48, -1.44, -1.4, -1.36, -1.32, -1.28, -1.24, -1.2, -1.16, -1.12, -1.08, -1.04, -1, -0.96, -0.92, -0.88, -0.84, -0.8, -0.76, -0.72, -0.68, -0.64, -0.6, -0.56, -0.52, -0.48, -0.44, -0.4, -0.36, -0.32, -0.28, -0.24, -0.2, -0.16, -0.12, -0.08, -0.04, 0, 0.04, 0.08, 0.12, 0.16, 0.2, 0.24, 0.28, 0.32, 0.36, 0.4, 0.44, 0.48, 0.52, 0.56, 0.6, 0.64, 0.68, 0.72, 0.76, 0.8, 0.84, 0.88, 0.92, 0.96, 1, 1.04, 1.08, 1.12, 1.16, 1.2, 1.24, 1.28, 1.32, 1.36, 1.4, 1.44, 1.48, 1.52, 1.56, 1.6\n" // Delta Eta Bins
    "deltaEtaVertex: -1.6, -1.52, -1.44, -1.36, -1.28, -1.2, -1.12, -1.04, -0.96, -0.88, -0.8, -0.72, -0.64, -0.56, -0.48, -0.4, -0.32, -0.24, -0.16, -0.08, 0, 0.08, 0.16, 0.24, 0.32, 0.4, 0.48, 0.56, 0.64, 0.72, 0.8, 0.88, 0.96, 1.04, 1.12, 1.2, 1.28, 1.36, 1.44, 1.52, 1.6\n" // Delta Eta Bins (Vertex Binning)
    "deltaPhi: -1.5708, -1.48353, -1.39626, -1.309, -1.22173, -1.13446, -1.0472, -0.959931, -0.872665, -0.785398, -0.698132, -0.610865, -0.523599, -0.436332, -0.349066, -0.261799, -0.174533, -0.0872665, 0, 0.0872665, 0.174533, 0.261799, 0.349066, 0.436332, 0.523599, 0.610865, 0.698132, 0.785398, 0.872665, 0.959931, 1.0472, 1.13446, 1.22173, 1.309, 1.39626, 1.48353, 1.5708, 1.65806, 1.74533, 1.8326, 1.91986, 2.00713, 2.0944, 2.18166, 2.26893, 2.35619, 2.44346, 2.53073, 2.61799, 2.70526, 2.79253, 2.87979, 2.96706, 3.05433, 3.14159, 3.22886, 3.31613, 3.40339, 3.49066, 3.57792, 3.66519, 3.75246, 3.83972, 3.92699, 4.01426, 4.10152, 4.18879, 4.27606, 4.36332, 4.45059, 4.53786, 4.62512, 4.71239\n" // Delta Phi Bins
    "pT: 0.2,0.6,1.0,1.5,2.0,2.5,3.0,3.5,4.0,5.0,6.0,7.0,8.0,10.,12.,15.,20.\n"             // pT Bins
    "pTVertex: 0.2,1.0,2.0,3.0,4.0,8.0,15.0\n"                                              // pT Bins (Vertex Binning)
    "vertex: -10., 10.\n"                                                                   // Vertex Bins
    "vertexVertex: -10., -7., -5., -3., -1., 1., 3., 5., 7., 10.\n"                         // Vertex Bins (Vertex Binning)
    ;
  
  
  // =========================================================
  // Customization (adopted from AliUEHistograms)
  // =========================================================

  TObjArray* lines = defaultBinningStr.Tokenize("\n");
  for (Int_t i=0; i<lines->GetEntriesFast(); i++)
  {
    TString line(lines->At(i)->GetName());
    TString tag = line(0, line.Index(":")+1);
    if (!fCustomBinning.BeginsWith(tag) && !fCustomBinning.Contains(TString("\n") + tag))
      fBinningString += line + "\n";
    else
      AliInfo(Form("Using custom binning for %s", tag.Data()));
  }
  delete lines;
  fBinningString += fCustomBinning;
  
  AliInfo(Form("Used AliTHn Binning:\n%s",fBinningString.Data()));


  // =========================================================
  // Now set the bins
  // =========================================================

  //Depending on fEventClass Variable, do one thing or the other...
  /**********************************************************
   
  ======> Modification: Change Event Classification Scheme
    
  ---> fEventClass == "EventPlane"
   
   Default operation with Event Plane 
   
  ---> fEventClass == "Multiplicity"
   
   Work with reference multiplicity (from GetReferenceMultiplicity, which one is decided in the configuration)
   
  ---> fEventClass == "Centrality" 
   
   Work with Centrality Bins

  ***********************************************************/
  if(fEventClass == "Multiplicity"){
    dBinsSingle[0]     = GetBinning(fBinningString, "multiplicity", iBinSingle[0]);
    dBinsPair[0]       = GetBinning(fBinningString, "multiplicity", iBinPair[0]);
    axisTitleSingle[0] = "reference multiplicity";
    axisTitlePair[0]   = "reference multiplicity";
  }
  if(fEventClass == "Centrality"){
    // fine binning in case of vertex Z binning
    if(fVertexBinning){
      dBinsSingle[0]     = GetBinning(fBinningString, "centralityVertex", iBinSingle[0]);
      dBinsPair[0]       = GetBinning(fBinningString, "centralityVertex", iBinPair[0]);
    }
    else{
      dBinsSingle[0]     = GetBinning(fBinningString, "centrality", iBinSingle[0]);
      dBinsPair[0]       = GetBinning(fBinningString, "centrality", iBinPair[0]);
    }
    axisTitleSingle[0] = "Centrality percentile [%]";
    axisTitlePair[0]   = "Centrality percentile [%]";
  }
  if(fEventClass == "EventPlane"){
    dBinsSingle[0]     = GetBinning(fBinningString, "eventPlane", iBinSingle[0]);
    dBinsPair[0]       = GetBinning(fBinningString, "eventPlane", iBinPair[0]);
    axisTitleSingle[0] = "#varphi - #Psi_{2} (a.u.)";
    axisTitlePair[0]   = "#varphi - #Psi_{2} (a.u.)";
  }
  

  // Delta Eta and Delta Phi
  // (coarse binning in case of vertex Z binning)
  if(fVertexBinning){
    dBinsPair[1]       = GetBinning(fBinningString, "deltaEtaVertex", iBinPair[1]);
  }
  else{
    dBinsPair[1]       = GetBinning(fBinningString, "deltaEta", iBinPair[1]);
  }
  axisTitlePair[1]  = "#Delta#eta"; 
  
  dBinsPair[2]       = GetBinning(fBinningString, "deltaPhi", iBinPair[2]);
  axisTitlePair[2]   = "#Delta#varphi (rad)";  
  

  // pT Trig and pT Assoc
  // (coarse binning in case of vertex Z binning)
  if(fVertexBinning){
    dBinsSingle[1]   = GetBinning(fBinningString, "pTVertex", iBinSingle[1]);
    dBinsPair[3]     = GetBinning(fBinningString, "pTVertex", iBinPair[3]);
    dBinsPair[4]     = GetBinning(fBinningString, "pTVertex", iBinPair[4]);
  }
  else{
    dBinsSingle[1]   = GetBinning(fBinningString, "pT", iBinSingle[1]);
    dBinsPair[3]     = GetBinning(fBinningString, "pT", iBinPair[3]);
    dBinsPair[4]     = GetBinning(fBinningString, "pT", iBinPair[4]);
  }
  
  axisTitleSingle[1]  = "p_{T,trig.} (GeV/c)"; 
  axisTitlePair[3]    = "p_{T,trig.} (GeV/c)"; 
  axisTitlePair[4]    = "p_{T,assoc.} (GeV/c)";  
 

  // vertex Z binning or not
  if(fVertexBinning){
    dBinsSingle[2]   = GetBinning(fBinningString, "vertexVertex", iBinSingle[2]);
    dBinsPair[5]     = GetBinning(fBinningString, "vertexVertex", iBinPair[5]);
  }
  else{
    dBinsSingle[2]   = GetBinning(fBinningString, "vertex", iBinSingle[2]);
    dBinsPair[5]     = GetBinning(fBinningString, "vertex", iBinPair[5]);
  }

  axisTitleSingle[2]  = "v_{Z} (cm)"; 
  axisTitlePair[5]    = "v_{Z} (cm)"; 



  // =========================================================
  // Create the Output objects (AliTHn)
  // =========================================================

  TString histName;
  //+ triggered particles
  histName = "fHistP"; 
  if(fShuffle) histName.Append("_shuffle");
  if(fCentralityId) histName += fCentralityId.Data();
  fHistP = new AliTHn(histName.Data(),histName.Data(),anaSteps,kTrackVariablesSingle,iBinSingle);
  for (Int_t j=0; j<kTrackVariablesSingle; j++) {
    fHistP->SetBinLimits(j, dBinsSingle[j]);
    fHistP->SetVarTitle(j, axisTitleSingle[j]);
  }

  //- triggered particles
  histName = "fHistN"; 
  if(fShuffle) histName.Append("_shuffle");
  if(fCentralityId) histName += fCentralityId.Data();
  fHistN = new AliTHn(histName.Data(),histName.Data(),anaSteps,kTrackVariablesSingle,iBinSingle);
  for (Int_t j=0; j<kTrackVariablesSingle; j++) {
    fHistN->SetBinLimits(j, dBinsSingle[j]);
    fHistN->SetVarTitle(j, axisTitleSingle[j]);
  }
  
  //+- pairs
  histName = "fHistPN";
  if(fShuffle) histName.Append("_shuffle");
  if(fCentralityId) histName += fCentralityId.Data();
  fHistPN = new AliTHn(histName.Data(),histName.Data(),anaSteps, kTrackVariablesPair, iBinPair);
  for (Int_t j=0; j<kTrackVariablesPair; j++) {
    fHistPN->SetBinLimits(j, dBinsPair[j]);
    fHistPN->SetVarTitle(j, axisTitlePair[j]);
  }

  //-+ pairs
  histName = "fHistNP";
  if(fShuffle) histName.Append("_shuffle");
  if(fCentralityId) histName += fCentralityId.Data();
  fHistNP = new AliTHn(histName.Data(),histName.Data(),anaSteps, kTrackVariablesPair, iBinPair);
  for (Int_t j=0; j<kTrackVariablesPair; j++) {
    fHistNP->SetBinLimits(j, dBinsPair[j]);
    fHistNP->SetVarTitle(j, axisTitlePair[j]);
  }

  //++ pairs
  histName = "fHistPP";
  if(fShuffle) histName.Append("_shuffle");
  if(fCentralityId) histName += fCentralityId.Data();
  fHistPP = new AliTHn(histName.Data(),histName.Data(),anaSteps, kTrackVariablesPair, iBinPair);
  for (Int_t j=0; j<kTrackVariablesPair; j++) {
    fHistPP->SetBinLimits(j, dBinsPair[j]);
    fHistPP->SetVarTitle(j, axisTitlePair[j]);
  }

  //-- pairs
  histName = "fHistNN";
  if(fShuffle) histName.Append("_shuffle");
  if(fCentralityId) histName += fCentralityId.Data();
  fHistNN = new AliTHn(histName.Data(),histName.Data(),anaSteps, kTrackVariablesPair, iBinPair);
  for (Int_t j=0; j<kTrackVariablesPair; j++) {
    fHistNN->SetBinLimits(j, dBinsPair[j]);
    fHistNN->SetVarTitle(j, axisTitlePair[j]);
  }

  AliInfo("Finished setting up the AliTHn");

  // QA histograms
  fHistHBTbefore        = new TH2D("fHistHBTbefore","before HBT cut",200,0,2,200,0,2.*TMath::Pi());
  fHistHBTafter         = new TH2D("fHistHBTafter","after HBT cut",200,0,2,200,0,2.*TMath::Pi());
  fHistConversionbefore = new TH3D("fHistConversionbefore","before Conversion cut;#Delta#eta;#Delta#phi;M_{inv}^{2}",50,-2.0,2.0,50,-TMath::Pi()/2.,3.*TMath::Pi()/2.,300,0,1.5);
  fHistConversionafter  = new TH3D("fHistConversionafter","after Conversion cut;#Delta#eta;#Delta#phi;M_{inv}^{2}",50,-2.0,2.0,50,-TMath::Pi()/2.,3.*TMath::Pi()/2.,300,0,1.5);
  fHistPsiMinusPhi      = new TH2D("fHistPsiMinusPhi","",4,-0.5,3.5,100,0,2.*TMath::Pi());
  fHistResonancesBefore = new TH3D("fHistResonancesBefore","before resonance cut;#Delta#eta;#Delta#phi;M_{inv}",50,-2.0,2.0,50,-TMath::Pi()/2.,3.*TMath::Pi()/2.,300,0,1.5);
  fHistResonancesRho    = new TH3D("fHistResonancesRho","after #rho resonance cut;#Delta#eta;#Delta#phi;M_{inv}",50,-2.0,2.0,50,-TMath::Pi()/2.,3.*TMath::Pi()/2.,300,0,1.5);
  fHistResonancesK0     = new TH3D("fHistResonancesK0","after #rho, K0 resonance cut;#Delta#eta;#Delta#phi;M_{inv}",50,-2.0,2.0,50,-TMath::Pi()/2.,3.*TMath::Pi()/2.,300,0,1.5);
  fHistResonancesLambda = new TH3D("fHistResonancesLambda","after #rho, K0, Lambda resonance cut;#Delta#eta;#Delta#phi;M_{inv}",50,-2.0,2.0,50,-TMath::Pi()/2.,3.*TMath::Pi()/2.,300,0,1.5);
  fHistQbefore          = new TH3D("fHistQbefore","before momentum difference cut;#Delta#eta;#Delta#phi;|#Delta p_{T}| (GeV/c)",50,-2.0,2.0,50,-TMath::Pi()/2.,3.*TMath::Pi()/2.,300,0,1.5);
  fHistQafter           = new TH3D("fHistQafter","after momentum difference cut;#Delta#eta;#Delta#phi;|#Delta p_{T}| (GeV/c)",50,-2.0,2.0,50,-TMath::Pi()/2.,3.*TMath::Pi()/2.,300,0,1.5);

  TH1::AddDirectory(oldStatus);

}

//____________________________________________________________________//
void AliBalancePsi::CalculateBalance(Double_t gReactionPlane,
				     TObjArray *particles, 
				     TObjArray *particlesMixed,
				     Float_t bSign,
				     Double_t kMultorCent,
				     Double_t vertexZ) {
  // Calculates the balance function
  fAnalyzedEvents++;
    
  // Initialize histograms if not done yet
  if(!fHistPN){
    AliWarning("Histograms not yet initialized! --> Will be done now");
    AliWarning("This works only in local mode --> Add 'gBalance->InitHistograms()' in your configBalanceFunction");
    InitHistograms();
  }

  Double_t trackVariablesSingle[kTrackVariablesSingle];
  Double_t trackVariablesPair[kTrackVariablesPair];

  if (!particles){
    AliWarning("particles TObjArray is NULL pointer --> return");
    return;
  }
  
  // define end of particle loops
  Int_t iMax = particles->GetEntriesFast();
  Int_t jMax = iMax;
  if (particlesMixed)
    jMax = particlesMixed->GetEntriesFast();

  // Eta() is extremely time consuming, therefore cache it for the inner loop here:
  TObjArray* particlesSecond = (particlesMixed) ? particlesMixed : particles;

  TArrayF secondEta(jMax);
  TArrayF secondPhi(jMax);
  TArrayF secondPt(jMax);
  TArrayS secondCharge(jMax);
  TArrayD secondCorrection(jMax);

  for (Int_t i=0; i<jMax; i++){
    secondEta[i] = ((AliVParticle*) particlesSecond->At(i))->Eta();
    secondPhi[i] = ((AliVParticle*) particlesSecond->At(i))->Phi();
    secondPt[i]  = ((AliVParticle*) particlesSecond->At(i))->Pt();
    secondCharge[i]  = (Short_t)((AliVParticle*) particlesSecond->At(i))->Charge();
    secondCorrection[i]  = (Double_t)((AliBFBasicParticle*) particlesSecond->At(i))->Correction();   //==========================correction
  }
  
  //TLorenzVector implementation for resonances
  TLorentzVector vectorMother, vectorDaughter[2];
  TParticle pPion, pProton, pRho0, pK0s, pLambda;
  pPion.SetPdgCode(211); //pion
  pRho0.SetPdgCode(113); //rho0
  pK0s.SetPdgCode(310); //K0s
  pProton.SetPdgCode(2212); //proton
  pLambda.SetPdgCode(3122); //Lambda
  Double_t gWidthForRho0 = 0.01;
  Double_t gWidthForK0s = 0.01;
  Double_t gWidthForLambda = 0.006;
  Double_t nSigmaRejection = 3.0;

  // 1st particle loop
  for (Int_t i = 0; i < iMax; i++) {
    //AliVParticle* firstParticle = (AliVParticle*) particles->At(i);
    AliBFBasicParticle* firstParticle = (AliBFBasicParticle*) particles->At(i); //==========================correction
    
    // some optimization
    Float_t firstEta = firstParticle->Eta();
    Float_t firstPhi = firstParticle->Phi();
    Float_t firstPt  = firstParticle->Pt();
    Float_t firstCorrection  = firstParticle->Correction();//==========================correction

    // Event plane (determine psi bin)
    Double_t gPsiMinusPhi    =   0.;
    Double_t gPsiMinusPhiBin = -10.;
    gPsiMinusPhi   = TMath::Abs(firstPhi - gReactionPlane);
    //in-plane
    if((gPsiMinusPhi <= 7.5*TMath::DegToRad())||
       ((172.5*TMath::DegToRad() <= gPsiMinusPhi)&&(gPsiMinusPhi <= 187.5*TMath::DegToRad())))
      gPsiMinusPhiBin = 0.0;
    //intermediate
    else if(((37.5*TMath::DegToRad() <= gPsiMinusPhi)&&(gPsiMinusPhi <= 52.5*TMath::DegToRad()))||
	    ((127.5*TMath::DegToRad() <= gPsiMinusPhi)&&(gPsiMinusPhi <= 142.5*TMath::DegToRad()))||
	    ((217.5*TMath::DegToRad() <= gPsiMinusPhi)&&(gPsiMinusPhi <= 232.5*TMath::DegToRad()))||
	    ((307.5*TMath::DegToRad() <= gPsiMinusPhi)&&(gPsiMinusPhi <= 322.5*TMath::DegToRad())))
      gPsiMinusPhiBin = 1.0;
    //out of plane
    else if(((82.5*TMath::DegToRad() <= gPsiMinusPhi)&&(gPsiMinusPhi <= 97.5*TMath::DegToRad()))||
	    ((262.5*TMath::DegToRad() <= gPsiMinusPhi)&&(gPsiMinusPhi <= 277.5*TMath::DegToRad())))
      gPsiMinusPhiBin = 2.0;
    //everything else
    else 
      gPsiMinusPhiBin = 3.0;
    
    fHistPsiMinusPhi->Fill(gPsiMinusPhiBin,gPsiMinusPhi);

    Short_t  charge1 = (Short_t) firstParticle->Charge();
    
    trackVariablesSingle[0]    =  gPsiMinusPhiBin;
    trackVariablesSingle[1]    =  firstPt;
      if(fEventClass=="Multiplicity" || fEventClass == "Centrality" ) trackVariablesSingle[0] = kMultorCent;
    trackVariablesSingle[2]    =  vertexZ;

    
    //fill single particle histograms
    if(charge1 > 0)      fHistP->Fill(trackVariablesSingle,0,firstCorrection); //==========================correction
    else if(charge1 < 0) fHistN->Fill(trackVariablesSingle,0,firstCorrection);  //==========================correction
    
    // 2nd particle loop
    for(Int_t j = 0; j < jMax; j++) {   

      if(!particlesMixed && j == i) continue; // no auto correlations (only for non mixing)

      // pT,Assoc < pT,Trig
      if(firstPt < secondPt[j]) continue;

      Short_t charge2 = secondCharge[j];
      
      trackVariablesPair[0]    =  trackVariablesSingle[0];
      trackVariablesPair[1]    =  firstEta - secondEta[j];  // delta eta
      trackVariablesPair[2]    =  firstPhi - secondPhi[j];  // delta phi
      //if (trackVariablesPair[2] > 180.)   // delta phi between -180 and 180 
      //trackVariablesPair[2] -= 360.;
      //if (trackVariablesPair[2] <  - 180.) 
      //trackVariablesPair[2] += 360.;
      if (trackVariablesPair[2] > TMath::Pi()) // delta phi between -pi and pi 
	trackVariablesPair[2] -= 2.*TMath::Pi();
      if (trackVariablesPair[2] <  - TMath::Pi()) 
	trackVariablesPair[2] += 2.*TMath::Pi();
      if (trackVariablesPair[2] <  - TMath::Pi()/2.) 
      trackVariablesPair[2] += 2.*TMath::Pi();
      
      trackVariablesPair[3]    =  firstPt;      // pt trigger
      trackVariablesPair[4]    =  secondPt[j];  // pt
      trackVariablesPair[5]    =  vertexZ;      // z of the primary vertex
      
      //Exclude resonances for the calculation of pairs by looking 
      //at the invariant mass and not considering the pairs that 
      //fall within 3sigma from the mass peak of: rho0, K0s, Lambda
      if(fResonancesCut) {
	if (charge1 * charge2 < 0) {

	  //rho0
	  vectorDaughter[0].SetPtEtaPhiM(firstPt,firstEta,firstPhi,pPion.GetMass());
	  vectorDaughter[1].SetPtEtaPhiM(secondPt[j],secondEta[j],secondPhi[j],pPion.GetMass());
	  vectorMother = vectorDaughter[0] + vectorDaughter[1];
	  fHistResonancesBefore->Fill(trackVariablesPair[1],trackVariablesPair[2],vectorMother.M());
	  if(TMath::Abs(vectorMother.M() - pRho0.GetMass()) <= nSigmaRejection*gWidthForRho0)
	    continue;
	  fHistResonancesRho->Fill(trackVariablesPair[1],trackVariablesPair[2],vectorMother.M());
	  
	  //K0s
	  if(TMath::Abs(vectorMother.M() - pK0s.GetMass()) <= nSigmaRejection*gWidthForK0s)
	    continue;
	  fHistResonancesK0->Fill(trackVariablesPair[1],trackVariablesPair[2],vectorMother.M());
	  
	  
	  //Lambda
	  vectorDaughter[0].SetPtEtaPhiM(firstPt,firstEta,firstPhi,pPion.GetMass());
	  vectorDaughter[1].SetPtEtaPhiM(secondPt[j],secondEta[j],secondPhi[j],pProton.GetMass());
	  vectorMother = vectorDaughter[0] + vectorDaughter[1];
	  if(TMath::Abs(vectorMother.M() - pLambda.GetMass()) <= nSigmaRejection*gWidthForLambda)
	    continue;
	  
	  vectorDaughter[0].SetPtEtaPhiM(firstPt,firstEta,firstPhi,pProton.GetMass());
	  vectorDaughter[1].SetPtEtaPhiM(secondPt[j],secondEta[j],secondPhi[j],pPion.GetMass());
	  vectorMother = vectorDaughter[0] + vectorDaughter[1];
	  if(TMath::Abs(vectorMother.M() - pLambda.GetMass()) <= nSigmaRejection*gWidthForLambda)
	    continue;
	  fHistResonancesLambda->Fill(trackVariablesPair[1],trackVariablesPair[2],vectorMother.M());
	
	}//unlike-sign only
      }//resonance cut

      // HBT like cut
      if(fHBTCut){ // VERSION 3 (all pairs)
        //if(fHBTCut && charge1 * charge2 > 0){  // VERSION 2 (only for LS)
	//if( dphi < 3 || deta < 0.01 ){   // VERSION 1
	//  continue;
	
	Double_t deta = firstEta - secondEta[j];
	Double_t dphi = firstPhi - secondPhi[j];
	// VERSION 2 (Taken from DPhiCorrelations)
	// the variables & cuthave been developed by the HBT group 
	// see e.g. https://indico.cern.ch/materialDisplay.py?contribId=36&sessionId=6&materialId=slides&confId=142700
	fHistHBTbefore->Fill(deta,dphi);
	
	// optimization
	if (TMath::Abs(deta) < fHBTCutValue * 2.5 * 3) //fHBTCutValue = 0.02 [default for dphicorrelations]
	  {
	    // phi in rad
	    //Float_t phi1rad = firstPhi*TMath::DegToRad();
	    //Float_t phi2rad = secondPhi[j]*TMath::DegToRad();
	    Float_t phi1rad = firstPhi;
	    Float_t phi2rad = secondPhi[j];
	    
	    // check first boundaries to see if is worth to loop and find the minimum
	    Float_t dphistar1 = GetDPhiStar(phi1rad, firstPt, charge1, phi2rad, secondPt[j], charge2, 0.8, bSign);
	    Float_t dphistar2 = GetDPhiStar(phi1rad, firstPt, charge1, phi2rad, secondPt[j], charge2, 2.5, bSign);
	    
	    const Float_t kLimit = fHBTCutValue * 3;
	    
	    Float_t dphistarminabs = 1e5;
	    Float_t dphistarmin = 1e5;
	    
	    if (TMath::Abs(dphistar1) < kLimit || TMath::Abs(dphistar2) < kLimit || dphistar1 * dphistar2 < 0 ) {
	      for (Double_t rad=0.8; rad<2.51; rad+=0.01) {
		Float_t dphistar = GetDPhiStar(phi1rad, firstPt, charge1, phi2rad, secondPt[j], charge2, rad, bSign);
		Float_t dphistarabs = TMath::Abs(dphistar);
		
		if (dphistarabs < dphistarminabs) {
		  dphistarmin = dphistar;
		  dphistarminabs = dphistarabs;
		}
	      }
	      
	      if (dphistarminabs < fHBTCutValue && TMath::Abs(deta) < fHBTCutValue) {
		//AliInfo(Form("HBT: Removed track pair %d %d with [[%f %f]] %f %f %f | %f %f %d %f %f %d %f", i, j, deta, dphi, dphistarminabs, dphistar1, dphistar2, phi1rad, pt1, charge1, phi2rad, pt2, charge2, bSign));
		continue;
	      }
	    }
	  }
	fHistHBTafter->Fill(deta,dphi);
      }//HBT cut
	
      // conversions
      if(fConversionCut) {
	if (charge1 * charge2 < 0) {
	  Double_t deta = firstEta - secondEta[j];
	  Double_t dphi = firstPhi - secondPhi[j];
	  
	  Float_t m0 = 0.510e-3;
	  Float_t tantheta1 = 1e10;
	  
	  // phi in rad
	  //Float_t phi1rad = firstPhi*TMath::DegToRad();
	  //Float_t phi2rad = secondPhi[j]*TMath::DegToRad();
	  Float_t phi1rad = firstPhi;
	  Float_t phi2rad = secondPhi[j];
	  
	  if (firstEta < -1e-10 || firstEta > 1e-10)
	    tantheta1 = 2 * TMath::Exp(-firstEta) / ( 1 - TMath::Exp(-2*firstEta));
	  
	  Float_t tantheta2 = 1e10;
	  if (secondEta[j] < -1e-10 || secondEta[j] > 1e-10)
	    tantheta2 = 2 * TMath::Exp(-secondEta[j]) / ( 1 - TMath::Exp(-2*secondEta[j]));
	  
	  Float_t e1squ = m0 * m0 + firstPt * firstPt * (1.0 + 1.0 / tantheta1 / tantheta1);
	  Float_t e2squ = m0 * m0 + secondPt[j] * secondPt[j] * (1.0 + 1.0 / tantheta2 / tantheta2);
	  
	  Float_t masssqu = 2 * m0 * m0 + 2 * ( TMath::Sqrt(e1squ * e2squ) - ( firstPt * secondPt[j] * ( TMath::Cos(phi1rad - phi2rad) + 1.0 / tantheta1 / tantheta2 ) ) );

	  fHistConversionbefore->Fill(deta,dphi,masssqu);
	  
	  if (masssqu < fInvMassCutConversion*fInvMassCutConversion){
	    //AliInfo(Form("Conversion: Removed track pair %d %d with [[%f %f] %f %f] %d %d <- %f %f  %f %f   %f %f ", i, j, deta, dphi, masssqu, charge1, charge2,eta1,eta2,phi1,phi2,pt1,pt2));
	    continue;
	  }
	  fHistConversionafter->Fill(deta,dphi,masssqu);
	}
      }//conversion cut

      // momentum difference cut - suppress femtoscopic effects
      if(fQCut){ 

	//Double_t ptMin        = 0.1; //const for the time being (should be changeable later on)
	Double_t ptDifference = TMath::Abs( firstPt - secondPt[j]);

	fHistQbefore->Fill(trackVariablesPair[1],trackVariablesPair[2],ptDifference);
	if(ptDifference < fDeltaPtMin) continue;
	fHistQafter->Fill(trackVariablesPair[1],trackVariablesPair[2],ptDifference);

      }

      if( charge1 > 0 && charge2 < 0)  fHistPN->Fill(trackVariablesPair,0,firstCorrection*secondCorrection[j]); //==========================correction
      else if( charge1 < 0 && charge2 > 0)  fHistNP->Fill(trackVariablesPair,0,firstCorrection*secondCorrection[j]);//==========================correction 
      else if( charge1 > 0 && charge2 > 0)  fHistPP->Fill(trackVariablesPair,0,firstCorrection*secondCorrection[j]);//==========================correction 
      else if( charge1 < 0 && charge2 < 0)  fHistNN->Fill(trackVariablesPair,0,firstCorrection*secondCorrection[j]);//==========================correction 
      else {
	//AliWarning(Form("Wrong charge combination: charge1 = %d and charge2 = %d",charge,charge2));
	continue;
      }
    }//end of 2nd particle loop
  }//end of 1st particle loop
}  

//____________________________________________________________________//
TH1D *AliBalancePsi::GetBalanceFunctionHistogram(Int_t iVariableSingle,
						 Int_t iVariablePair,
						 Double_t psiMin, 
						 Double_t psiMax,
						 Double_t vertexZMin,
						 Double_t vertexZMax,
						 Double_t ptTriggerMin,
						 Double_t ptTriggerMax,
						 Double_t ptAssociatedMin,
						 Double_t ptAssociatedMax) {
  //Returns the BF histogram, extracted from the 6 AliTHn objects 
  //(private members) of the AliBalancePsi class.
  //iVariableSingle: 0(phi-Psi), 1(pt-trigger)
  //iVariablePair: 0(phi-Psi) 1(Delta eta), 2(Delta phi), 3(pt-trigger), 4(pt-associated

  // security checks
  if(psiMin > psiMax-0.00001){
    AliError("psiMax <= psiMin");
    return NULL;
  }
  if(vertexZMin > vertexZMax-0.00001){
    AliError("vertexZMax <= vertexZMin");
    return NULL;
  }
  if(ptTriggerMin > ptTriggerMax-0.00001){
    AliError("ptTriggerMax <= ptTriggerMin");
    return NULL;
  }
  if(ptAssociatedMin > ptAssociatedMax-0.00001){
    AliError("ptAssociatedMax <= ptAssociatedMin");
    return NULL;
  }

  // Psi_2
  fHistP->GetGrid(0)->GetGrid()->GetAxis(0)->SetRangeUser(psiMin,psiMax-0.00001); 
  fHistN->GetGrid(0)->GetGrid()->GetAxis(0)->SetRangeUser(psiMin,psiMax-0.00001); 
  fHistPN->GetGrid(0)->GetGrid()->GetAxis(0)->SetRangeUser(psiMin,psiMax-0.00001); 
  fHistNP->GetGrid(0)->GetGrid()->GetAxis(0)->SetRangeUser(psiMin,psiMax-0.00001); 
  fHistPP->GetGrid(0)->GetGrid()->GetAxis(0)->SetRangeUser(psiMin,psiMax-0.00001); 
  fHistNN->GetGrid(0)->GetGrid()->GetAxis(0)->SetRangeUser(psiMin,psiMax-0.00001); 

  // Vz
 if(fVertexBinning){
   fHistP->GetGrid(0)->GetGrid()->GetAxis(2)->SetRangeUser(vertexZMin,vertexZMax-0.00001); 
   fHistN->GetGrid(0)->GetGrid()->GetAxis(2)->SetRangeUser(vertexZMin,vertexZMax-0.00001); 
   fHistPN->GetGrid(0)->GetGrid()->GetAxis(5)->SetRangeUser(vertexZMin,vertexZMax-0.00001); 
   fHistNP->GetGrid(0)->GetGrid()->GetAxis(5)->SetRangeUser(vertexZMin,vertexZMax-0.00001); 
   fHistPP->GetGrid(0)->GetGrid()->GetAxis(5)->SetRangeUser(vertexZMin,vertexZMax-0.00001); 
   fHistNN->GetGrid(0)->GetGrid()->GetAxis(5)->SetRangeUser(vertexZMin,vertexZMax-0.00001); 
 }

  // pt trigger
  if((ptTriggerMin != -1.)&&(ptTriggerMax != -1.)) {
    fHistP->GetGrid(0)->GetGrid()->GetAxis(1)->SetRangeUser(ptTriggerMin,ptTriggerMax-0.00001);
    fHistN->GetGrid(0)->GetGrid()->GetAxis(1)->SetRangeUser(ptTriggerMin,ptTriggerMax-0.00001);
    fHistPN->GetGrid(0)->GetGrid()->GetAxis(3)->SetRangeUser(ptTriggerMin,ptTriggerMax-0.00001);
    fHistNP->GetGrid(0)->GetGrid()->GetAxis(3)->SetRangeUser(ptTriggerMin,ptTriggerMax-0.00001);
    fHistPP->GetGrid(0)->GetGrid()->GetAxis(3)->SetRangeUser(ptTriggerMin,ptTriggerMax-0.00001);
    fHistNN->GetGrid(0)->GetGrid()->GetAxis(3)->SetRangeUser(ptTriggerMin,ptTriggerMax-0.00001);
  }

  // pt associated
  if((ptAssociatedMin != -1.)&&(ptAssociatedMax != -1.)) {
    fHistPN->GetGrid(0)->GetGrid()->GetAxis(4)->SetRangeUser(ptAssociatedMin,ptAssociatedMax-0.00001);
    fHistNP->GetGrid(0)->GetGrid()->GetAxis(4)->SetRangeUser(ptAssociatedMin,ptAssociatedMax-0.00001);
    fHistPP->GetGrid(0)->GetGrid()->GetAxis(4)->SetRangeUser(ptAssociatedMin,ptAssociatedMax-0.00001);
    fHistNN->GetGrid(0)->GetGrid()->GetAxis(4)->SetRangeUser(ptAssociatedMin,ptAssociatedMax-0.00001);
  }

  //Printf("P:%lf - N:%lf - PN:%lf - NP:%lf - PP:%lf - NN:%lf",fHistP->GetEntries(0),fHistN->GetEntries(0),fHistPN->GetEntries(0),fHistNP->GetEntries(0),fHistPP->GetEntries(0),fHistNN->GetEntries(0));

  // Project into the wanted space (1st: analysis step, 2nd: axis)
  TH1D* hTemp1 = (TH1D*)fHistPN->Project(0,iVariablePair); //
  TH1D* hTemp2 = (TH1D*)fHistNP->Project(0,iVariablePair); //
  TH1D* hTemp3 = (TH1D*)fHistPP->Project(0,iVariablePair); //
  TH1D* hTemp4 = (TH1D*)fHistNN->Project(0,iVariablePair); //
  TH1D* hTemp5 = (TH1D*)fHistP->Project(0,iVariableSingle); //
  TH1D* hTemp6 = (TH1D*)fHistN->Project(0,iVariableSingle); //

  TH1D *gHistBalanceFunctionHistogram = 0x0;
  if((hTemp1)&&(hTemp2)&&(hTemp3)&&(hTemp4)&&(hTemp5)&&(hTemp6)) {
    gHistBalanceFunctionHistogram = (TH1D*)hTemp1->Clone();
    gHistBalanceFunctionHistogram->Reset();
    
    switch(iVariablePair) {
    case 1:
      gHistBalanceFunctionHistogram->GetXaxis()->SetTitle("#Delta#eta");
      gHistBalanceFunctionHistogram->GetYaxis()->SetTitle("B(#Delta#eta)");
      break;
    case 2:
      gHistBalanceFunctionHistogram->GetXaxis()->SetTitle("#Delta#varphi (rad)");
      gHistBalanceFunctionHistogram->GetYaxis()->SetTitle("B(#Delta#varphi)");
      break;
    default:
      break;
    }

    hTemp1->Sumw2();
    hTemp2->Sumw2();
    hTemp3->Sumw2();
    hTemp4->Sumw2();
    hTemp1->Add(hTemp3,-1.);
    hTemp1->Scale(1./hTemp5->Integral());
    hTemp2->Add(hTemp4,-1.);
    hTemp2->Scale(1./hTemp6->Integral());
    gHistBalanceFunctionHistogram->Add(hTemp1,hTemp2,1.,1.);
    gHistBalanceFunctionHistogram->Scale(0.5);

    //normalize to bin width
    gHistBalanceFunctionHistogram->Scale(1./((Double_t)gHistBalanceFunctionHistogram->GetXaxis()->GetBinWidth(1)));
  }

  return gHistBalanceFunctionHistogram;
}

//____________________________________________________________________//
TH1D *AliBalancePsi::GetBalanceFunctionHistogram2pMethod(Int_t iVariableSingle,
							 Int_t iVariablePair,
							 Double_t psiMin, 
							 Double_t psiMax,
							 Double_t vertexZMin,
							 Double_t vertexZMax,
							 Double_t ptTriggerMin,
							 Double_t ptTriggerMax,
							 Double_t ptAssociatedMin,
							 Double_t ptAssociatedMax,
							 AliBalancePsi *bfMix) {
  //Returns the BF histogram, extracted from the 6 AliTHn objects 
  //after dividing each correlation function by the Event Mixing one 
  //(private members) of the AliBalancePsi class.
  //iVariableSingle: 0(phi-Psi), 1(pt-trigger)
  //iVariablePair: 0(phi-Psi) 1(Delta eta), 2(Delta phi), 3(pt-trigger), 4(pt-associated

  // security checks
  if(psiMin > psiMax-0.00001){
    AliError("psiMax <= psiMin");
    return NULL;
  }
  if(vertexZMin > vertexZMax-0.00001){
    AliError("vertexZMax <= vertexZMin");
    return NULL;
  }
  if(ptTriggerMin > ptTriggerMax-0.00001){
    AliError("ptTriggerMax <= ptTriggerMin");
    return NULL;
  }
  if(ptAssociatedMin > ptAssociatedMax-0.00001){
    AliError("ptAssociatedMax <= ptAssociatedMin");
    return NULL;
  }

  // pt trigger (fixed for all vertices/psi/centralities)
  if((ptTriggerMin != -1.)&&(ptTriggerMax != -1.)) {
    fHistP->GetGrid(0)->GetGrid()->GetAxis(1)->SetRangeUser(ptTriggerMin,ptTriggerMax-0.00001);
    fHistN->GetGrid(0)->GetGrid()->GetAxis(1)->SetRangeUser(ptTriggerMin,ptTriggerMax-0.00001);
    fHistPN->GetGrid(0)->GetGrid()->GetAxis(3)->SetRangeUser(ptTriggerMin,ptTriggerMax-0.00001);
    fHistNP->GetGrid(0)->GetGrid()->GetAxis(3)->SetRangeUser(ptTriggerMin,ptTriggerMax-0.00001);
    fHistPP->GetGrid(0)->GetGrid()->GetAxis(3)->SetRangeUser(ptTriggerMin,ptTriggerMax-0.00001);
    fHistNN->GetGrid(0)->GetGrid()->GetAxis(3)->SetRangeUser(ptTriggerMin,ptTriggerMax-0.00001);
  }

  // pt associated (fixed for all vertices/psi/centralities)
  if((ptAssociatedMin != -1.)&&(ptAssociatedMax != -1.)) {
    fHistPN->GetGrid(0)->GetGrid()->GetAxis(4)->SetRangeUser(ptAssociatedMin,ptAssociatedMax-0.00001);
    fHistNP->GetGrid(0)->GetGrid()->GetAxis(4)->SetRangeUser(ptAssociatedMin,ptAssociatedMax-0.00001);
    fHistPP->GetGrid(0)->GetGrid()->GetAxis(4)->SetRangeUser(ptAssociatedMin,ptAssociatedMax-0.00001);
    fHistNN->GetGrid(0)->GetGrid()->GetAxis(4)->SetRangeUser(ptAssociatedMin,ptAssociatedMax-0.00001);
  }

  // ============================================================================================
  // the same for event mixing
  AliTHn *fHistPMix = bfMix->GetHistNp();
  AliTHn *fHistNMix = bfMix->GetHistNn();
  AliTHn *fHistPNMix = bfMix->GetHistNpn();
  AliTHn *fHistNPMix = bfMix->GetHistNnp();
  AliTHn *fHistPPMix = bfMix->GetHistNpp();
  AliTHn *fHistNNMix = bfMix->GetHistNnn();

  // pt trigger (fixed for all vertices/psi/centralities)
  if((ptTriggerMin != -1.)&&(ptTriggerMax != -1.)) {
    fHistPMix->GetGrid(0)->GetGrid()->GetAxis(1)->SetRangeUser(ptTriggerMin,ptTriggerMax-0.00001);
    fHistNMix->GetGrid(0)->GetGrid()->GetAxis(1)->SetRangeUser(ptTriggerMin,ptTriggerMax-0.00001);
    fHistPNMix->GetGrid(0)->GetGrid()->GetAxis(3)->SetRangeUser(ptTriggerMin,ptTriggerMax-0.00001);
    fHistNPMix->GetGrid(0)->GetGrid()->GetAxis(3)->SetRangeUser(ptTriggerMin,ptTriggerMax-0.00001);
    fHistPPMix->GetGrid(0)->GetGrid()->GetAxis(3)->SetRangeUser(ptTriggerMin,ptTriggerMax-0.00001);
    fHistNNMix->GetGrid(0)->GetGrid()->GetAxis(3)->SetRangeUser(ptTriggerMin,ptTriggerMax-0.00001);
  }

  // pt associated (fixed for all vertices/psi/centralities)
  if((ptAssociatedMin != -1.)&&(ptAssociatedMax != -1.)) {
    fHistPNMix->GetGrid(0)->GetGrid()->GetAxis(4)->SetRangeUser(ptAssociatedMin,ptAssociatedMax-0.00001);
    fHistNPMix->GetGrid(0)->GetGrid()->GetAxis(4)->SetRangeUser(ptAssociatedMin,ptAssociatedMax-0.00001);
    fHistPPMix->GetGrid(0)->GetGrid()->GetAxis(4)->SetRangeUser(ptAssociatedMin,ptAssociatedMax-0.00001);
    fHistNNMix->GetGrid(0)->GetGrid()->GetAxis(4)->SetRangeUser(ptAssociatedMin,ptAssociatedMax-0.00001);
  }

  // ============================================================================================
  // ranges (in bins) for vertexZ and centrality (psi)
  Int_t binPsiMin    = fHistPN->GetGrid(0)->GetGrid()->GetAxis(0)->FindBin(psiMin);
  Int_t binPsiMax    = fHistPN->GetGrid(0)->GetGrid()->GetAxis(0)->FindBin(psiMax-0.00001);
  Int_t binVertexMin = 0;
  Int_t binVertexMax = 0;
  if(fVertexBinning){
    binVertexMin = fHistPN->GetGrid(0)->GetGrid()->GetAxis(5)->FindBin(vertexZMin);
    binVertexMax = fHistPN->GetGrid(0)->GetGrid()->GetAxis(5)->FindBin(vertexZMax-0.00001);
  }  
  Double_t binPsiLowEdge    = 0.;
  Double_t binPsiUpEdge     = 1000.;
  Double_t binVertexLowEdge = 0.;
  Double_t binVertexUpEdge  = 1000.;

  TH1D* h1 = NULL;
  TH1D* h2 = NULL;
  TH1D* h3 = NULL;
  TH1D* h4 = NULL;

  // loop over all bins
  for(Int_t iBinPsi = binPsiMin; iBinPsi <= binPsiMax; iBinPsi++){
    for(Int_t iBinVertex = binVertexMin; iBinVertex <= binVertexMax; iBinVertex++){
  
      cout<<"In the balance function (1D) loop: "<<iBinPsi<<" (psiBin), "<<iBinVertex<<" (vertexBin)  "<<endl;

      // determine the bin edges for this bin
      binPsiLowEdge    = fHistPN->GetGrid(0)->GetGrid()->GetAxis(0)->GetBinLowEdge(iBinPsi);
      binPsiUpEdge     = fHistPN->GetGrid(0)->GetGrid()->GetAxis(0)->GetBinUpEdge(iBinPsi);
      if(fVertexBinning){
	binVertexLowEdge = fHistPN->GetGrid(0)->GetGrid()->GetAxis(5)->GetBinLowEdge(iBinVertex);
	binVertexUpEdge  = fHistPN->GetGrid(0)->GetGrid()->GetAxis(5)->GetBinUpEdge(iBinVertex);
      }
      
      // Psi_2
      fHistP->GetGrid(0)->GetGrid()->GetAxis(0)->SetRangeUser(binPsiLowEdge,binPsiUpEdge-0.00001); 
      fHistN->GetGrid(0)->GetGrid()->GetAxis(0)->SetRangeUser(binPsiLowEdge,binPsiUpEdge-0.00001); 
      fHistPN->GetGrid(0)->GetGrid()->GetAxis(0)->SetRangeUser(binPsiLowEdge,binPsiUpEdge-0.00001); 
      fHistNP->GetGrid(0)->GetGrid()->GetAxis(0)->SetRangeUser(binPsiLowEdge,binPsiUpEdge-0.00001); 
      fHistPP->GetGrid(0)->GetGrid()->GetAxis(0)->SetRangeUser(binPsiLowEdge,binPsiUpEdge-0.00001); 
      fHistNN->GetGrid(0)->GetGrid()->GetAxis(0)->SetRangeUser(binPsiLowEdge,binPsiUpEdge-0.00001); 

      // Vz
      if(fVertexBinning){
	fHistP->GetGrid(0)->GetGrid()->GetAxis(2)->SetRangeUser(binVertexLowEdge,binVertexUpEdge-0.00001); 
	fHistN->GetGrid(0)->GetGrid()->GetAxis(2)->SetRangeUser(binVertexLowEdge,binVertexUpEdge-0.00001); 
	fHistPN->GetGrid(0)->GetGrid()->GetAxis(5)->SetRangeUser(binVertexLowEdge,binVertexUpEdge-0.00001); 
	fHistNP->GetGrid(0)->GetGrid()->GetAxis(5)->SetRangeUser(binVertexLowEdge,binVertexUpEdge-0.00001); 
	fHistPP->GetGrid(0)->GetGrid()->GetAxis(5)->SetRangeUser(binVertexLowEdge,binVertexUpEdge-0.00001); 
	fHistNN->GetGrid(0)->GetGrid()->GetAxis(5)->SetRangeUser(binVertexLowEdge,binVertexUpEdge-0.00001); 
      }

      // ============================================================================================
      // the same for event mixing
      
      // Psi_2
      fHistPMix->GetGrid(0)->GetGrid()->GetAxis(0)->SetRangeUser(binPsiLowEdge,binPsiUpEdge-0.00001); 
      fHistNMix->GetGrid(0)->GetGrid()->GetAxis(0)->SetRangeUser(binPsiLowEdge,binPsiUpEdge-0.00001); 
      fHistPNMix->GetGrid(0)->GetGrid()->GetAxis(0)->SetRangeUser(binPsiLowEdge,binPsiUpEdge-0.00001); 
      fHistNPMix->GetGrid(0)->GetGrid()->GetAxis(0)->SetRangeUser(binPsiLowEdge,binPsiUpEdge-0.00001); 
      fHistPPMix->GetGrid(0)->GetGrid()->GetAxis(0)->SetRangeUser(binPsiLowEdge,binPsiUpEdge-0.00001); 
      fHistNNMix->GetGrid(0)->GetGrid()->GetAxis(0)->SetRangeUser(binPsiLowEdge,binPsiUpEdge-0.00001); 
      
      // Vz
      if(fVertexBinning){
	fHistPMix->GetGrid(0)->GetGrid()->GetAxis(2)->SetRangeUser(binVertexLowEdge,binVertexUpEdge-0.00001); 
	fHistNMix->GetGrid(0)->GetGrid()->GetAxis(2)->SetRangeUser(binVertexLowEdge,binVertexUpEdge-0.00001); 
	fHistPNMix->GetGrid(0)->GetGrid()->GetAxis(5)->SetRangeUser(binVertexLowEdge,binVertexUpEdge-0.00001); 
	fHistNPMix->GetGrid(0)->GetGrid()->GetAxis(5)->SetRangeUser(binVertexLowEdge,binVertexUpEdge-0.00001); 
	fHistPPMix->GetGrid(0)->GetGrid()->GetAxis(5)->SetRangeUser(binVertexLowEdge,binVertexUpEdge-0.00001); 
	fHistNNMix->GetGrid(0)->GetGrid()->GetAxis(5)->SetRangeUser(binVertexLowEdge,binVertexUpEdge-0.00001); 
      }
      
      // ============================================================================================

      //Printf("P:%lf - N:%lf - PN:%lf - NP:%lf - PP:%lf - NN:%lf",fHistP->GetEntries(0),fHistN->GetEntries(0),fHistPN->GetEntries(0),fHistNP->GetEntries(0),fHistPP->GetEntries(0),fHistNN->GetEntries(0));
      
      // Project into the wanted space (1st: analysis step, 2nd: axis)
      TH1D* hTempHelper1 = (TH1D*)fHistPN->Project(0,iVariablePair);
      TH1D* hTempHelper2 = (TH1D*)fHistNP->Project(0,iVariablePair);
      TH1D* hTempHelper3 = (TH1D*)fHistPP->Project(0,iVariablePair);
      TH1D* hTempHelper4 = (TH1D*)fHistNN->Project(0,iVariablePair);
      TH1D* hTemp5 = (TH1D*)fHistP->Project(0,iVariableSingle);
      TH1D* hTemp6 = (TH1D*)fHistN->Project(0,iVariableSingle);
      
      // ============================================================================================
      // the same for event mixing
      TH1D* hTempHelper1Mix = (TH1D*)fHistPNMix->Project(0,iVariablePair);
      TH1D* hTempHelper2Mix = (TH1D*)fHistNPMix->Project(0,iVariablePair);
      TH1D* hTempHelper3Mix = (TH1D*)fHistPPMix->Project(0,iVariablePair);
      TH1D* hTempHelper4Mix = (TH1D*)fHistNNMix->Project(0,iVariablePair);
      TH1D* hTemp5Mix = (TH1D*)fHistPMix->Project(0,iVariableSingle);
      TH1D* hTemp6Mix = (TH1D*)fHistNMix->Project(0,iVariableSingle);
      // ============================================================================================

      hTempHelper1->Sumw2();
      hTempHelper2->Sumw2();
      hTempHelper3->Sumw2();
      hTempHelper4->Sumw2();
      hTemp5->Sumw2();
      hTemp6->Sumw2();
      hTempHelper1Mix->Sumw2();
      hTempHelper2Mix->Sumw2();
      hTempHelper3Mix->Sumw2();
      hTempHelper4Mix->Sumw2();
      hTemp5Mix->Sumw2();
      hTemp6Mix->Sumw2();

      // first put everything on positive x - axis (to be comparable with published data) --> ONLY IN THIS OPTION!

      Double_t helperEndBin = 1.6;
      if(iVariablePair==2) helperEndBin = TMath::Pi();

      TH1D* hTempPos1 = new TH1D(Form("hTempPos1_%d_%d",iBinPsi,iBinVertex),Form("hTempPos1_%d_%d",iBinPsi,iBinVertex),hTempHelper1->GetNbinsX()/2,0,helperEndBin);
      TH1D* hTempPos2 = new TH1D(Form("hTempPos2_%d_%d",iBinPsi,iBinVertex),Form("hTempPos2_%d_%d",iBinPsi,iBinVertex),hTempHelper2->GetNbinsX()/2,0,helperEndBin);
      TH1D* hTempPos3 = new TH1D(Form("hTempPos3_%d_%d",iBinPsi,iBinVertex),Form("hTempPos3_%d_%d",iBinPsi,iBinVertex),hTempHelper3->GetNbinsX()/2,0,helperEndBin);
      TH1D* hTempPos4 = new TH1D(Form("hTempPos4_%d_%d",iBinPsi,iBinVertex),Form("hTempPos4_%d_%d",iBinPsi,iBinVertex),hTempHelper4->GetNbinsX()/2,0,helperEndBin);
      TH1D* hTempPos1Mix = new TH1D(Form("hTempPos1Mix_%d_%d",iBinPsi,iBinVertex),Form("hTempPos1Mix_%d_%d",iBinPsi,iBinVertex),hTempHelper1Mix->GetNbinsX()/2,0,helperEndBin);
      TH1D* hTempPos2Mix = new TH1D(Form("hTempPos2Mix_%d_%d",iBinPsi,iBinVertex),Form("hTempPos2Mix_%d_%d",iBinPsi,iBinVertex),hTempHelper2Mix->GetNbinsX()/2,0,helperEndBin);
      TH1D* hTempPos3Mix = new TH1D(Form("hTempPos3Mix_%d_%d",iBinPsi,iBinVertex),Form("hTempPos3Mix_%d_%d",iBinPsi,iBinVertex),hTempHelper3Mix->GetNbinsX()/2,0,helperEndBin);
      TH1D* hTempPos4Mix = new TH1D(Form("hTempPos4Mix_%d_%d",iBinPsi,iBinVertex),Form("hTempPos4Mix_%d_%d",iBinPsi,iBinVertex),hTempHelper4Mix->GetNbinsX()/2,0,helperEndBin);

      TH1D* hTemp1 = new TH1D(Form("hTemp1_%d_%d",iBinPsi,iBinVertex),Form("hTemp1_%d_%d",iBinPsi,iBinVertex),hTempHelper1->GetNbinsX()/2,0,helperEndBin);
      TH1D* hTemp2 = new TH1D(Form("hTemp2_%d_%d",iBinPsi,iBinVertex),Form("hTemp2_%d_%d",iBinPsi,iBinVertex),hTempHelper2->GetNbinsX()/2,0,helperEndBin);
      TH1D* hTemp3 = new TH1D(Form("hTemp3_%d_%d",iBinPsi,iBinVertex),Form("hTemp3_%d_%d",iBinPsi,iBinVertex),hTempHelper3->GetNbinsX()/2,0,helperEndBin);
      TH1D* hTemp4 = new TH1D(Form("hTemp4_%d_%d",iBinPsi,iBinVertex),Form("hTemp4_%d_%d",iBinPsi,iBinVertex),hTempHelper4->GetNbinsX()/2,0,helperEndBin);
      TH1D* hTemp1Mix = new TH1D(Form("hTemp1Mix_%d_%d",iBinPsi,iBinVertex),Form("hTemp1Mix_%d_%d",iBinPsi,iBinVertex),hTempHelper1Mix->GetNbinsX()/2,0,helperEndBin);
      TH1D* hTemp2Mix = new TH1D(Form("hTemp2Mix_%d_%d",iBinPsi,iBinVertex),Form("hTemp2Mix_%d_%d",iBinPsi,iBinVertex),hTempHelper2Mix->GetNbinsX()/2,0,helperEndBin);
      TH1D* hTemp3Mix = new TH1D(Form("hTemp3Mix_%d_%d",iBinPsi,iBinVertex),Form("hTemp3Mix_%d_%d",iBinPsi,iBinVertex),hTempHelper3Mix->GetNbinsX()/2,0,helperEndBin);
      TH1D* hTemp4Mix = new TH1D(Form("hTemp4Mix_%d_%d",iBinPsi,iBinVertex),Form("hTemp4Mix_%d_%d",iBinPsi,iBinVertex),hTempHelper4Mix->GetNbinsX()/2,0,helperEndBin);


      hTempPos1->Sumw2();
      hTempPos2->Sumw2();
      hTempPos3->Sumw2();
      hTempPos4->Sumw2();
      hTempPos1Mix->Sumw2();
      hTempPos2Mix->Sumw2();
      hTempPos3Mix->Sumw2();
      hTempPos4Mix->Sumw2();

      hTemp1->Sumw2();
      hTemp2->Sumw2();
      hTemp3->Sumw2();
      hTemp4->Sumw2();
      hTemp1Mix->Sumw2();
      hTemp2Mix->Sumw2();
      hTemp3Mix->Sumw2();
      hTemp4Mix->Sumw2();


      for(Int_t i=0;i<hTempHelper1->GetNbinsX();i++){

	Double_t binCenter  = hTempHelper1->GetXaxis()->GetBinCenter(i+1);

	if(iVariablePair==1){
	  if(binCenter>0){
	    hTempPos1->SetBinContent(hTempPos1->FindBin(binCenter),hTempHelper1->GetBinContent(i+1));
	    hTempPos2->SetBinContent(hTempPos2->FindBin(binCenter),hTempHelper2->GetBinContent(i+1));
	    hTempPos3->SetBinContent(hTempPos3->FindBin(binCenter),hTempHelper3->GetBinContent(i+1));
	    hTempPos4->SetBinContent(hTempPos4->FindBin(binCenter),hTempHelper4->GetBinContent(i+1));
	    hTempPos1Mix->SetBinContent(hTempPos1Mix->FindBin(binCenter),hTempHelper1Mix->GetBinContent(i+1));
	    hTempPos2Mix->SetBinContent(hTempPos2Mix->FindBin(binCenter),hTempHelper2Mix->GetBinContent(i+1));
	    hTempPos3Mix->SetBinContent(hTempPos3Mix->FindBin(binCenter),hTempHelper3Mix->GetBinContent(i+1));
	    hTempPos4Mix->SetBinContent(hTempPos4Mix->FindBin(binCenter),hTempHelper4Mix->GetBinContent(i+1));

	    hTempPos1->SetBinError(hTempPos1->FindBin(binCenter),hTempHelper1->GetBinError(i+1));
	    hTempPos2->SetBinError(hTempPos2->FindBin(binCenter),hTempHelper2->GetBinError(i+1));
	    hTempPos3->SetBinError(hTempPos3->FindBin(binCenter),hTempHelper3->GetBinError(i+1));
	    hTempPos4->SetBinError(hTempPos4->FindBin(binCenter),hTempHelper4->GetBinError(i+1));
	    hTempPos1Mix->SetBinError(hTempPos1Mix->FindBin(binCenter),hTempHelper1Mix->GetBinError(i+1));
	    hTempPos2Mix->SetBinError(hTempPos2Mix->FindBin(binCenter),hTempHelper2Mix->GetBinError(i+1));
	    hTempPos3Mix->SetBinError(hTempPos3Mix->FindBin(binCenter),hTempHelper3Mix->GetBinError(i+1));
	    hTempPos4Mix->SetBinError(hTempPos4Mix->FindBin(binCenter),hTempHelper4Mix->GetBinError(i+1));
	  }
	  else{
	    hTemp1->SetBinContent(hTemp1->FindBin(-binCenter),hTempHelper1->GetBinContent(i+1));
	    hTemp2->SetBinContent(hTemp2->FindBin(-binCenter),hTempHelper2->GetBinContent(i+1));
	    hTemp3->SetBinContent(hTemp3->FindBin(-binCenter),hTempHelper3->GetBinContent(i+1));
	    hTemp4->SetBinContent(hTemp4->FindBin(-binCenter),hTempHelper4->GetBinContent(i+1));
	    hTemp1Mix->SetBinContent(hTemp1Mix->FindBin(-binCenter),hTempHelper1Mix->GetBinContent(i+1));
	    hTemp2Mix->SetBinContent(hTemp2Mix->FindBin(-binCenter),hTempHelper2Mix->GetBinContent(i+1));
	    hTemp3Mix->SetBinContent(hTemp3Mix->FindBin(-binCenter),hTempHelper3Mix->GetBinContent(i+1));
	    hTemp4Mix->SetBinContent(hTemp4Mix->FindBin(-binCenter),hTempHelper4Mix->GetBinContent(i+1));

	    hTemp1->SetBinError(hTemp1->FindBin(-binCenter),hTempHelper1->GetBinError(i+1));
	    hTemp2->SetBinError(hTemp2->FindBin(-binCenter),hTempHelper2->GetBinError(i+1));
	    hTemp3->SetBinError(hTemp3->FindBin(-binCenter),hTempHelper3->GetBinError(i+1));
	    hTemp4->SetBinError(hTemp4->FindBin(-binCenter),hTempHelper4->GetBinError(i+1));
	    hTemp1Mix->SetBinError(hTemp1Mix->FindBin(-binCenter),hTempHelper1Mix->GetBinError(i+1));
	    hTemp2Mix->SetBinError(hTemp2Mix->FindBin(-binCenter),hTempHelper2Mix->GetBinError(i+1));
	    hTemp3Mix->SetBinError(hTemp3Mix->FindBin(-binCenter),hTempHelper3Mix->GetBinError(i+1));
	    hTemp4Mix->SetBinError(hTemp4Mix->FindBin(-binCenter),hTempHelper4Mix->GetBinError(i+1));
	  }
	}
	else if(iVariablePair==2){
	  if(binCenter>0 && binCenter<TMath::Pi()){
	    hTempPos1->SetBinContent(hTempPos1->FindBin(binCenter),hTempHelper1->GetBinContent(i+1));
	    hTempPos2->SetBinContent(hTempPos2->FindBin(binCenter),hTempHelper2->GetBinContent(i+1));
	    hTempPos3->SetBinContent(hTempPos3->FindBin(binCenter),hTempHelper3->GetBinContent(i+1));
	    hTempPos4->SetBinContent(hTempPos4->FindBin(binCenter),hTempHelper4->GetBinContent(i+1));
	    hTempPos1Mix->SetBinContent(hTempPos1Mix->FindBin(binCenter),hTempHelper1Mix->GetBinContent(i+1));
	    hTempPos2Mix->SetBinContent(hTempPos2Mix->FindBin(binCenter),hTempHelper2Mix->GetBinContent(i+1));
	    hTempPos3Mix->SetBinContent(hTempPos3Mix->FindBin(binCenter),hTempHelper3Mix->GetBinContent(i+1));
	    hTempPos4Mix->SetBinContent(hTempPos4Mix->FindBin(binCenter),hTempHelper4Mix->GetBinContent(i+1));

	    hTempPos1->SetBinError(hTempPos1->FindBin(binCenter),hTempHelper1->GetBinError(i+1));
	    hTempPos2->SetBinError(hTempPos2->FindBin(binCenter),hTempHelper2->GetBinError(i+1));
	    hTempPos3->SetBinError(hTempPos3->FindBin(binCenter),hTempHelper3->GetBinError(i+1));
	    hTempPos4->SetBinError(hTempPos4->FindBin(binCenter),hTempHelper4->GetBinError(i+1));
	    hTempPos1Mix->SetBinError(hTempPos1Mix->FindBin(binCenter),hTempHelper1Mix->GetBinError(i+1));
	    hTempPos2Mix->SetBinError(hTempPos2Mix->FindBin(binCenter),hTempHelper2Mix->GetBinError(i+1));
	    hTempPos3Mix->SetBinError(hTempPos3Mix->FindBin(binCenter),hTempHelper3Mix->GetBinError(i+1));
	    hTempPos4Mix->SetBinError(hTempPos4Mix->FindBin(binCenter),hTempHelper4Mix->GetBinError(i+1));
	  }
	  else if(binCenter<0){
	    hTemp1->SetBinContent(hTemp1->FindBin(-binCenter),hTempHelper1->GetBinContent(i+1));
	    hTemp2->SetBinContent(hTemp2->FindBin(-binCenter),hTempHelper2->GetBinContent(i+1));
	    hTemp3->SetBinContent(hTemp3->FindBin(-binCenter),hTempHelper3->GetBinContent(i+1));
	    hTemp4->SetBinContent(hTemp4->FindBin(-binCenter),hTempHelper4->GetBinContent(i+1));
	    hTemp1Mix->SetBinContent(hTemp1Mix->FindBin(-binCenter),hTempHelper1Mix->GetBinContent(i+1));
	    hTemp2Mix->SetBinContent(hTemp2Mix->FindBin(-binCenter),hTempHelper2Mix->GetBinContent(i+1));
	    hTemp3Mix->SetBinContent(hTemp3Mix->FindBin(-binCenter),hTempHelper3Mix->GetBinContent(i+1));
	    hTemp4Mix->SetBinContent(hTemp4Mix->FindBin(-binCenter),hTempHelper4Mix->GetBinContent(i+1));

	    hTemp1->SetBinError(hTemp1->FindBin(-binCenter),hTempHelper1->GetBinError(i+1));
	    hTemp2->SetBinError(hTemp2->FindBin(-binCenter),hTempHelper2->GetBinError(i+1));
	    hTemp3->SetBinError(hTemp3->FindBin(-binCenter),hTempHelper3->GetBinError(i+1));
	    hTemp4->SetBinError(hTemp4->FindBin(-binCenter),hTempHelper4->GetBinError(i+1));
	    hTemp1Mix->SetBinError(hTemp1Mix->FindBin(-binCenter),hTempHelper1Mix->GetBinError(i+1));
	    hTemp2Mix->SetBinError(hTemp2Mix->FindBin(-binCenter),hTempHelper2Mix->GetBinError(i+1));
	    hTemp3Mix->SetBinError(hTemp3Mix->FindBin(-binCenter),hTempHelper3Mix->GetBinError(i+1));
	    hTemp4Mix->SetBinError(hTemp4Mix->FindBin(-binCenter),hTempHelper4Mix->GetBinError(i+1));
	  }
	  else{
	    hTemp1->SetBinContent(hTemp1->FindBin(TMath::Pi()-binCenter),hTempHelper1->GetBinContent(i+1));
	    hTemp2->SetBinContent(hTemp2->FindBin(TMath::Pi()-binCenter),hTempHelper2->GetBinContent(i+1));
	    hTemp3->SetBinContent(hTemp3->FindBin(TMath::Pi()-binCenter),hTempHelper3->GetBinContent(i+1));
	    hTemp4->SetBinContent(hTemp4->FindBin(TMath::Pi()-binCenter),hTempHelper4->GetBinContent(i+1));
	    hTemp1Mix->SetBinContent(hTemp1Mix->FindBin(TMath::Pi()-binCenter),hTempHelper1Mix->GetBinContent(i+1));
	    hTemp2Mix->SetBinContent(hTemp2Mix->FindBin(TMath::Pi()-binCenter),hTempHelper2Mix->GetBinContent(i+1));
	    hTemp3Mix->SetBinContent(hTemp3Mix->FindBin(TMath::Pi()-binCenter),hTempHelper3Mix->GetBinContent(i+1));
	    hTemp4Mix->SetBinContent(hTemp4Mix->FindBin(TMath::Pi()-binCenter),hTempHelper4Mix->GetBinContent(i+1));

	    hTemp1->SetBinError(hTemp1->FindBin(TMath::Pi()-binCenter),hTempHelper1->GetBinError(i+1));
	    hTemp2->SetBinError(hTemp2->FindBin(TMath::Pi()-binCenter),hTempHelper2->GetBinError(i+1));
	    hTemp3->SetBinError(hTemp3->FindBin(TMath::Pi()-binCenter),hTempHelper3->GetBinError(i+1));
	    hTemp4->SetBinError(hTemp4->FindBin(TMath::Pi()-binCenter),hTempHelper4->GetBinError(i+1));
	    hTemp1Mix->SetBinError(hTemp1Mix->FindBin(TMath::Pi()-binCenter),hTempHelper1Mix->GetBinError(i+1));
	    hTemp2Mix->SetBinError(hTemp2Mix->FindBin(TMath::Pi()-binCenter),hTempHelper2Mix->GetBinError(i+1));
	    hTemp3Mix->SetBinError(hTemp3Mix->FindBin(TMath::Pi()-binCenter),hTempHelper3Mix->GetBinError(i+1));
	    hTemp4Mix->SetBinError(hTemp4Mix->FindBin(TMath::Pi()-binCenter),hTempHelper4Mix->GetBinError(i+1));
	  }
	}
      }

      hTemp1->Add(hTempPos1);
      hTemp2->Add(hTempPos2);
      hTemp3->Add(hTempPos3);
      hTemp4->Add(hTempPos4);
      hTemp1Mix->Add(hTempPos1Mix);
      hTemp2Mix->Add(hTempPos2Mix);
      hTemp3Mix->Add(hTempPos3Mix);
      hTemp4Mix->Add(hTempPos4Mix);

      hTemp1->Scale(1./hTemp5->Integral());
      hTemp3->Scale(1./hTemp5->Integral());
      hTemp2->Scale(1./hTemp6->Integral());
      hTemp4->Scale(1./hTemp6->Integral());

      // normalization of Event mixing to 1 at (0,0) --> Jan Fietes method
      // does not work here, so normalize also to trigger particles 
      // --> careful: gives different integrals then as with full 2D method 
      hTemp1Mix->Scale(1./hTemp5Mix->Integral());
      hTemp3Mix->Scale(1./hTemp5Mix->Integral());
      hTemp2Mix->Scale(1./hTemp6Mix->Integral());
      hTemp4Mix->Scale(1./hTemp6Mix->Integral());

      hTemp1->Divide(hTemp1Mix);
      hTemp2->Divide(hTemp2Mix);
      hTemp3->Divide(hTemp3Mix);
      hTemp4->Divide(hTemp4Mix);

      // for the first: clone
      if(iBinPsi == binPsiMin && iBinVertex == binVertexMin ){
	h1 = (TH1D*)hTemp1->Clone();
	h2 = (TH1D*)hTemp2->Clone();
	h3 = (TH1D*)hTemp3->Clone();
	h4 = (TH1D*)hTemp4->Clone();
      }
      else{  // otherwise: add for averaging
	h1->Add(hTemp1);
	h2->Add(hTemp2);
	h3->Add(hTemp3);
	h4->Add(hTemp4);
      }

      delete hTemp1;
      delete hTemp2;
      delete hTemp3;
      delete hTemp4;
      delete hTemp1Mix;
      delete hTemp2Mix;
      delete hTemp3Mix;
      delete hTemp4Mix;

      delete hTempPos1;
      delete hTempPos2;
      delete hTempPos3;
      delete hTempPos4;
      delete hTempPos1Mix;
      delete hTempPos2Mix;
      delete hTempPos3Mix;
      delete hTempPos4Mix;
    }
  }

  TH1D *gHistBalanceFunctionHistogram = 0x0;
  if((h1)&&(h2)&&(h3)&&(h4)) {

    // average over number of bins nbinsVertex * nbinsPsi
    h1->Scale(1./((Double_t)(binPsiMax-binPsiMin+1)*(binVertexMax-binVertexMin+1)));
    h2->Scale(1./((Double_t)(binPsiMax-binPsiMin+1)*(binVertexMax-binVertexMin+1)));
    h3->Scale(1./((Double_t)(binPsiMax-binPsiMin+1)*(binVertexMax-binVertexMin+1)));
    h4->Scale(1./((Double_t)(binPsiMax-binPsiMin+1)*(binVertexMax-binVertexMin+1)));

    gHistBalanceFunctionHistogram = (TH1D*)h1->Clone();
    gHistBalanceFunctionHistogram->Reset();
    
    switch(iVariablePair) {
    case 1:
      gHistBalanceFunctionHistogram->GetXaxis()->SetTitle("#Delta#eta");
      gHistBalanceFunctionHistogram->GetYaxis()->SetTitle("B(#Delta#eta)");
      break;
    case 2:
      gHistBalanceFunctionHistogram->GetXaxis()->SetTitle("#Delta#varphi (rad)");
      gHistBalanceFunctionHistogram->GetYaxis()->SetTitle("B(#Delta#varphi)");
      break;
    default:
      break;
    }

    h1->Add(h3,-1.);
    h2->Add(h4,-1.);

    gHistBalanceFunctionHistogram->Add(h1,h2,1.,1.);
    gHistBalanceFunctionHistogram->Scale(0.5);

    //normalize to bin width
    gHistBalanceFunctionHistogram->Scale(1./((Double_t)gHistBalanceFunctionHistogram->GetXaxis()->GetBinWidth(1)));
  }

  return gHistBalanceFunctionHistogram;
}

//____________________________________________________________________//
TH2D *AliBalancePsi::GetBalanceFunctionDeltaEtaDeltaPhi(Double_t psiMin, 
							Double_t psiMax,
							Double_t vertexZMin,
							Double_t vertexZMax,
							Double_t ptTriggerMin,
							Double_t ptTriggerMax,
							Double_t ptAssociatedMin,
							Double_t ptAssociatedMax) {
  //Returns the BF histogram in Delta eta vs Delta phi, 
  //extracted from the 6 AliTHn objects 
  //(private members) of the AliBalancePsi class.
  //iVariableSingle: 0(phi-Psi), 1(pt-trigger)
  //iVariablePair: 0(phi-Psi) 1(Delta eta), 2(Delta phi), 3(pt-trigger), 4(pt-associated

  // security checks
  if(psiMin > psiMax-0.00001){
    AliError("psiMax <= psiMin");
    return NULL;
  }
  if(vertexZMin > vertexZMax-0.00001){
    AliError("vertexZMax <= vertexZMin");
    return NULL;
  }
  if(ptTriggerMin > ptTriggerMax-0.00001){
    AliError("ptTriggerMax <= ptTriggerMin");
    return NULL;
  }
  if(ptAssociatedMin > ptAssociatedMax-0.00001){
    AliError("ptAssociatedMax <= ptAssociatedMin");
    return NULL;
  }

  TString histName = "gHistBalanceFunctionHistogram2D";

  // Psi_2
  fHistP->GetGrid(0)->GetGrid()->GetAxis(0)->SetRangeUser(psiMin,psiMax-0.00001); 
  fHistN->GetGrid(0)->GetGrid()->GetAxis(0)->SetRangeUser(psiMin,psiMax-0.00001); 
  fHistPN->GetGrid(0)->GetGrid()->GetAxis(0)->SetRangeUser(psiMin,psiMax-0.00001); 
  fHistNP->GetGrid(0)->GetGrid()->GetAxis(0)->SetRangeUser(psiMin,psiMax-0.00001); 
  fHistPP->GetGrid(0)->GetGrid()->GetAxis(0)->SetRangeUser(psiMin,psiMax-0.00001); 
  fHistNN->GetGrid(0)->GetGrid()->GetAxis(0)->SetRangeUser(psiMin,psiMax-0.00001); 

  // Vz
  if(fVertexBinning){
    fHistP->GetGrid(0)->GetGrid()->GetAxis(2)->SetRangeUser(vertexZMin,vertexZMax-0.00001); 
    fHistN->GetGrid(0)->GetGrid()->GetAxis(2)->SetRangeUser(vertexZMin,vertexZMax-0.00001); 
    fHistPN->GetGrid(0)->GetGrid()->GetAxis(5)->SetRangeUser(vertexZMin,vertexZMax-0.00001); 
    fHistNP->GetGrid(0)->GetGrid()->GetAxis(5)->SetRangeUser(vertexZMin,vertexZMax-0.00001); 
    fHistPP->GetGrid(0)->GetGrid()->GetAxis(5)->SetRangeUser(vertexZMin,vertexZMax-0.00001); 
    fHistNN->GetGrid(0)->GetGrid()->GetAxis(5)->SetRangeUser(vertexZMin,vertexZMax-0.00001); 
  }

  // pt trigger
  if((ptTriggerMin != -1.)&&(ptTriggerMax != -1.)) {
    fHistP->GetGrid(0)->GetGrid()->GetAxis(1)->SetRangeUser(ptTriggerMin,ptTriggerMax-0.00001);
    fHistN->GetGrid(0)->GetGrid()->GetAxis(1)->SetRangeUser(ptTriggerMin,ptTriggerMax-0.00001);
    fHistPN->GetGrid(0)->GetGrid()->GetAxis(3)->SetRangeUser(ptTriggerMin,ptTriggerMax-0.00001);
    fHistNP->GetGrid(0)->GetGrid()->GetAxis(3)->SetRangeUser(ptTriggerMin,ptTriggerMax-0.00001);
    fHistPP->GetGrid(0)->GetGrid()->GetAxis(3)->SetRangeUser(ptTriggerMin,ptTriggerMax-0.00001);
    fHistNN->GetGrid(0)->GetGrid()->GetAxis(3)->SetRangeUser(ptTriggerMin,ptTriggerMax-0.00001);
  }

  // pt associated
  if((ptAssociatedMin != -1.)&&(ptAssociatedMax != -1.)) {
    fHistPN->GetGrid(0)->GetGrid()->GetAxis(4)->SetRangeUser(ptAssociatedMin,ptAssociatedMax-0.00001);
    fHistNP->GetGrid(0)->GetGrid()->GetAxis(4)->SetRangeUser(ptAssociatedMin,ptAssociatedMax-0.00001);
    fHistPP->GetGrid(0)->GetGrid()->GetAxis(4)->SetRangeUser(ptAssociatedMin,ptAssociatedMax-0.00001);
    fHistNN->GetGrid(0)->GetGrid()->GetAxis(4)->SetRangeUser(ptAssociatedMin,ptAssociatedMax-0.00001);
  }

  //AliInfo(Form("P:%lf - N:%lf - PN:%lf - NP:%lf - PP:%lf - NN:%lf",fHistP->GetEntries(0),fHistN->GetEntries(0),fHistPN->GetEntries(0),fHistNP->GetEntries(0),fHistPP->GetEntries(0),fHistNN->GetEntries(0)));

  // Project into the wanted space (1st: analysis step, 2nd: axis)
  TH2D* hTemp1 = (TH2D*)fHistPN->Project(0,1,2);
  TH2D* hTemp2 = (TH2D*)fHistNP->Project(0,1,2);
  TH2D* hTemp3 = (TH2D*)fHistPP->Project(0,1,2);
  TH2D* hTemp4 = (TH2D*)fHistNN->Project(0,1,2);
  TH1D* hTemp5 = (TH1D*)fHistP->Project(0,1);
  TH1D* hTemp6 = (TH1D*)fHistN->Project(0,1);

  TH2D *gHistBalanceFunctionHistogram = 0x0;
  if((hTemp1)&&(hTemp2)&&(hTemp3)&&(hTemp4)&&(hTemp5)&&(hTemp6)) {
    gHistBalanceFunctionHistogram = (TH2D*)hTemp1->Clone();
    gHistBalanceFunctionHistogram->Reset();
    gHistBalanceFunctionHistogram->GetXaxis()->SetTitle("#Delta#eta");   
    gHistBalanceFunctionHistogram->GetYaxis()->SetTitle("#Delta#varphi (rad)");
    gHistBalanceFunctionHistogram->GetZaxis()->SetTitle("B(#Delta#eta,#Delta#varphi)");   
    
    hTemp1->Sumw2();
    hTemp2->Sumw2();
    hTemp3->Sumw2();
    hTemp4->Sumw2();
    hTemp1->Add(hTemp3,-1.);
    hTemp1->Scale(1./hTemp5->Integral());
    hTemp2->Add(hTemp4,-1.);
    hTemp2->Scale(1./hTemp6->Integral());
    gHistBalanceFunctionHistogram->Add(hTemp1,hTemp2,1.,1.);
    gHistBalanceFunctionHistogram->Scale(0.5);

    //normalize to bin width
    gHistBalanceFunctionHistogram->Scale(1./((Double_t)gHistBalanceFunctionHistogram->GetXaxis()->GetBinWidth(1)*(Double_t)gHistBalanceFunctionHistogram->GetYaxis()->GetBinWidth(1)));
  }

  return gHistBalanceFunctionHistogram;
}

//____________________________________________________________________//
TH2D *AliBalancePsi::GetBalanceFunctionDeltaEtaDeltaPhi2pMethod(Double_t psiMin, 
								Double_t psiMax,
								Double_t vertexZMin,
								Double_t vertexZMax,
								Double_t ptTriggerMin,
								Double_t ptTriggerMax,
								Double_t ptAssociatedMin,
								Double_t ptAssociatedMax,
								AliBalancePsi *bfMix) {
  //Returns the BF histogram in Delta eta vs Delta phi,
  //after dividing each correlation function by the Event Mixing one 
  //extracted from the 6 AliTHn objects 
  //(private members) of the AliBalancePsi class.
  //iVariableSingle: 0(phi-Psi), 1(pt-trigger)
  //iVariablePair: 0(phi-Psi) 1(Delta eta), 2(Delta phi), 3(pt-trigger), 4(pt-associated
  TString histName = "gHistBalanceFunctionHistogram2D";

  if(!bfMix){
    AliError("balance function object for event mixing not available");
    return NULL;
  }

  // security checks
  if(psiMin > psiMax-0.00001){
    AliError("psiMax <= psiMin");
    return NULL;
  }
  if(vertexZMin > vertexZMax-0.00001){
    AliError("vertexZMax <= vertexZMin");
    return NULL;
  }
  if(ptTriggerMin > ptTriggerMax-0.00001){
    AliError("ptTriggerMax <= ptTriggerMin");
    return NULL;
  }
  if(ptAssociatedMin > ptAssociatedMax-0.00001){
    AliError("ptAssociatedMax <= ptAssociatedMin");
    return NULL;
  }

  // pt trigger (fixed for all vertices/psi/centralities)
  if((ptTriggerMin != -1.)&&(ptTriggerMax != -1.)) {
    fHistP->GetGrid(0)->GetGrid()->GetAxis(1)->SetRangeUser(ptTriggerMin,ptTriggerMax-0.00001);
    fHistN->GetGrid(0)->GetGrid()->GetAxis(1)->SetRangeUser(ptTriggerMin,ptTriggerMax-0.00001);
    fHistPN->GetGrid(0)->GetGrid()->GetAxis(3)->SetRangeUser(ptTriggerMin,ptTriggerMax-0.00001);
    fHistNP->GetGrid(0)->GetGrid()->GetAxis(3)->SetRangeUser(ptTriggerMin,ptTriggerMax-0.00001);
    fHistPP->GetGrid(0)->GetGrid()->GetAxis(3)->SetRangeUser(ptTriggerMin,ptTriggerMax-0.00001);
    fHistNN->GetGrid(0)->GetGrid()->GetAxis(3)->SetRangeUser(ptTriggerMin,ptTriggerMax-0.00001);
  }

  // pt associated (fixed for all vertices/psi/centralities)
  if((ptAssociatedMin != -1.)&&(ptAssociatedMax != -1.)) {
    fHistPN->GetGrid(0)->GetGrid()->GetAxis(4)->SetRangeUser(ptAssociatedMin,ptAssociatedMax-0.00001);
    fHistNP->GetGrid(0)->GetGrid()->GetAxis(4)->SetRangeUser(ptAssociatedMin,ptAssociatedMax-0.00001);
    fHistPP->GetGrid(0)->GetGrid()->GetAxis(4)->SetRangeUser(ptAssociatedMin,ptAssociatedMax-0.00001);
    fHistNN->GetGrid(0)->GetGrid()->GetAxis(4)->SetRangeUser(ptAssociatedMin,ptAssociatedMax-0.00001);
  }

  // ============================================================================================
  // the same for event mixing
  AliTHn *fHistPMix = bfMix->GetHistNp();
  AliTHn *fHistNMix = bfMix->GetHistNn();
  AliTHn *fHistPNMix = bfMix->GetHistNpn();
  AliTHn *fHistNPMix = bfMix->GetHistNnp();
  AliTHn *fHistPPMix = bfMix->GetHistNpp();
  AliTHn *fHistNNMix = bfMix->GetHistNnn();

  // pt trigger (fixed for all vertices/psi/centralities)
  if((ptTriggerMin != -1.)&&(ptTriggerMax != -1.)) {
    fHistPMix->GetGrid(0)->GetGrid()->GetAxis(1)->SetRangeUser(ptTriggerMin,ptTriggerMax-0.00001);
    fHistNMix->GetGrid(0)->GetGrid()->GetAxis(1)->SetRangeUser(ptTriggerMin,ptTriggerMax-0.00001);
    fHistPNMix->GetGrid(0)->GetGrid()->GetAxis(3)->SetRangeUser(ptTriggerMin,ptTriggerMax-0.00001);
    fHistNPMix->GetGrid(0)->GetGrid()->GetAxis(3)->SetRangeUser(ptTriggerMin,ptTriggerMax-0.00001);
    fHistPPMix->GetGrid(0)->GetGrid()->GetAxis(3)->SetRangeUser(ptTriggerMin,ptTriggerMax-0.00001);
    fHistNNMix->GetGrid(0)->GetGrid()->GetAxis(3)->SetRangeUser(ptTriggerMin,ptTriggerMax-0.00001);
  }

  // pt associated (fixed for all vertices/psi/centralities)
  if((ptAssociatedMin != -1.)&&(ptAssociatedMax != -1.)) {
    fHistPNMix->GetGrid(0)->GetGrid()->GetAxis(4)->SetRangeUser(ptAssociatedMin,ptAssociatedMax-0.00001);
    fHistNPMix->GetGrid(0)->GetGrid()->GetAxis(4)->SetRangeUser(ptAssociatedMin,ptAssociatedMax-0.00001);
    fHistPPMix->GetGrid(0)->GetGrid()->GetAxis(4)->SetRangeUser(ptAssociatedMin,ptAssociatedMax-0.00001);
    fHistNNMix->GetGrid(0)->GetGrid()->GetAxis(4)->SetRangeUser(ptAssociatedMin,ptAssociatedMax-0.00001);
  }

  // ============================================================================================
  // ranges (in bins) for vertexZ and centrality (psi)
  Int_t binPsiMin    = fHistPN->GetGrid(0)->GetGrid()->GetAxis(0)->FindBin(psiMin);
  Int_t binPsiMax    = fHistPN->GetGrid(0)->GetGrid()->GetAxis(0)->FindBin(psiMax-0.00001);
  Int_t binVertexMin = 0;
  Int_t binVertexMax = 0;
  if(fVertexBinning){
    binVertexMin = fHistPN->GetGrid(0)->GetGrid()->GetAxis(5)->FindBin(vertexZMin);
    binVertexMax = fHistPN->GetGrid(0)->GetGrid()->GetAxis(5)->FindBin(vertexZMax-0.00001);
  }  
  Double_t binPsiLowEdge    = 0.;
  Double_t binPsiUpEdge     = 0.;
  Double_t binVertexLowEdge = 0.;
  Double_t binVertexUpEdge  = 0.;

  TH2D* h1 = NULL;
  TH2D* h2 = NULL;
  TH2D* h3 = NULL;
  TH2D* h4 = NULL;

  // loop over all bins
  for(Int_t iBinPsi = binPsiMin; iBinPsi <= binPsiMax; iBinPsi++){
    for(Int_t iBinVertex = binVertexMin; iBinVertex <= binVertexMax; iBinVertex++){
  
      cout<<"In the balance function (2D) loop: "<<iBinPsi<<" (psiBin), "<<iBinVertex<<" (vertexBin)  "<<endl;

      // determine the bin edges for this bin
      binPsiLowEdge    = fHistPN->GetGrid(0)->GetGrid()->GetAxis(0)->GetBinLowEdge(iBinPsi);
      binPsiUpEdge     = fHistPN->GetGrid(0)->GetGrid()->GetAxis(0)->GetBinUpEdge(iBinPsi);
      if(fVertexBinning){
	binVertexLowEdge = fHistPN->GetGrid(0)->GetGrid()->GetAxis(5)->GetBinLowEdge(iBinVertex);
	binVertexUpEdge  = fHistPN->GetGrid(0)->GetGrid()->GetAxis(5)->GetBinUpEdge(iBinVertex);
      }

      // Psi_2
      fHistP->GetGrid(0)->GetGrid()->GetAxis(0)->SetRangeUser(binPsiLowEdge,binPsiUpEdge-0.00001); 
      fHistN->GetGrid(0)->GetGrid()->GetAxis(0)->SetRangeUser(binPsiLowEdge,binPsiUpEdge-0.00001); 
      fHistPN->GetGrid(0)->GetGrid()->GetAxis(0)->SetRangeUser(binPsiLowEdge,binPsiUpEdge-0.00001); 
      fHistNP->GetGrid(0)->GetGrid()->GetAxis(0)->SetRangeUser(binPsiLowEdge,binPsiUpEdge-0.00001); 
      fHistPP->GetGrid(0)->GetGrid()->GetAxis(0)->SetRangeUser(binPsiLowEdge,binPsiUpEdge-0.00001); 
      fHistNN->GetGrid(0)->GetGrid()->GetAxis(0)->SetRangeUser(binPsiLowEdge,binPsiUpEdge-0.00001); 
  
      // Vz
      if(fVertexBinning){
	fHistP->GetGrid(0)->GetGrid()->GetAxis(2)->SetRangeUser(binVertexLowEdge,binVertexUpEdge-0.00001); 
	fHistN->GetGrid(0)->GetGrid()->GetAxis(2)->SetRangeUser(binVertexLowEdge,binVertexUpEdge-0.00001); 
	fHistPN->GetGrid(0)->GetGrid()->GetAxis(5)->SetRangeUser(binVertexLowEdge,binVertexUpEdge-0.00001); 
	fHistNP->GetGrid(0)->GetGrid()->GetAxis(5)->SetRangeUser(binVertexLowEdge,binVertexUpEdge-0.00001); 
	fHistPP->GetGrid(0)->GetGrid()->GetAxis(5)->SetRangeUser(binVertexLowEdge,binVertexUpEdge-0.00001); 
	fHistNN->GetGrid(0)->GetGrid()->GetAxis(5)->SetRangeUser(binVertexLowEdge,binVertexUpEdge-0.00001); 
      }



      // ============================================================================================
      // the same for event mixing
      
      // Psi_2
      fHistPMix->GetGrid(0)->GetGrid()->GetAxis(0)->SetRangeUser(binPsiLowEdge,binPsiUpEdge-0.00001); 
      fHistNMix->GetGrid(0)->GetGrid()->GetAxis(0)->SetRangeUser(binPsiLowEdge,binPsiUpEdge-0.00001); 
      fHistPNMix->GetGrid(0)->GetGrid()->GetAxis(0)->SetRangeUser(binPsiLowEdge,binPsiUpEdge-0.00001); 
      fHistNPMix->GetGrid(0)->GetGrid()->GetAxis(0)->SetRangeUser(binPsiLowEdge,binPsiUpEdge-0.00001); 
      fHistPPMix->GetGrid(0)->GetGrid()->GetAxis(0)->SetRangeUser(binPsiLowEdge,binPsiUpEdge-0.00001); 
      fHistNNMix->GetGrid(0)->GetGrid()->GetAxis(0)->SetRangeUser(binPsiLowEdge,binPsiUpEdge-0.00001); 
      
      // Vz
      if(fVertexBinning){
	fHistPMix->GetGrid(0)->GetGrid()->GetAxis(2)->SetRangeUser(binVertexLowEdge,binVertexUpEdge-0.00001); 
	fHistNMix->GetGrid(0)->GetGrid()->GetAxis(2)->SetRangeUser(binVertexLowEdge,binVertexUpEdge-0.00001); 
	fHistPNMix->GetGrid(0)->GetGrid()->GetAxis(5)->SetRangeUser(binVertexLowEdge,binVertexUpEdge-0.00001); 
	fHistNPMix->GetGrid(0)->GetGrid()->GetAxis(5)->SetRangeUser(binVertexLowEdge,binVertexUpEdge-0.00001); 
	fHistPPMix->GetGrid(0)->GetGrid()->GetAxis(5)->SetRangeUser(binVertexLowEdge,binVertexUpEdge-0.00001); 
	fHistNNMix->GetGrid(0)->GetGrid()->GetAxis(5)->SetRangeUser(binVertexLowEdge,binVertexUpEdge-0.00001); 
      }
      
      // ============================================================================================
      
      
      //AliInfo(Form("P:%lf - N:%lf - PN:%lf - NP:%lf - PP:%lf - NN:%lf",fHistP->GetEntries(0),fHistN->GetEntries(0),fHistPN->GetEntries(0),fHistNP->GetEntries(0),fHistPP->GetEntries(0),fHistNN->GetEntries(0)));

      // Project into the wanted space (1st: analysis step, 2nd: axis)
      TH2D* hTemp1 = (TH2D*)fHistPN->Project(0,1,2);
      TH2D* hTemp2 = (TH2D*)fHistNP->Project(0,1,2);
      TH2D* hTemp3 = (TH2D*)fHistPP->Project(0,1,2);
      TH2D* hTemp4 = (TH2D*)fHistNN->Project(0,1,2);
      TH1D* hTemp5 = (TH1D*)fHistP->Project(0,1);
      TH1D* hTemp6 = (TH1D*)fHistN->Project(0,1);
      
      // ============================================================================================
      // the same for event mixing
      TH2D* hTemp1Mix = (TH2D*)fHistPNMix->Project(0,1,2);
      TH2D* hTemp2Mix = (TH2D*)fHistNPMix->Project(0,1,2);
      TH2D* hTemp3Mix = (TH2D*)fHistPPMix->Project(0,1,2);
      TH2D* hTemp4Mix = (TH2D*)fHistNNMix->Project(0,1,2);
      // TH1D* hTemp5Mix = (TH1D*)fHistPMix->Project(0,1);
      // TH1D* hTemp6Mix = (TH1D*)fHistNMix->Project(0,1);
      // ============================================================================================
      
      hTemp1->Sumw2();
      hTemp2->Sumw2();
      hTemp3->Sumw2();
      hTemp4->Sumw2();
      hTemp1Mix->Sumw2();
      hTemp2Mix->Sumw2();
      hTemp3Mix->Sumw2();
      hTemp4Mix->Sumw2();
      
      hTemp1->Scale(1./hTemp5->Integral());
      hTemp3->Scale(1./hTemp5->Integral());
      hTemp2->Scale(1./hTemp6->Integral());
      hTemp4->Scale(1./hTemp6->Integral());

      // normalization of Event mixing to 1 at (0,0) --> Jan Fietes method
      Double_t mixedNorm1 = hTemp1Mix->Integral(hTemp1Mix->GetXaxis()->FindBin(0-10e-5),hTemp1Mix->GetXaxis()->FindBin(0+10e-5),1,hTemp1Mix->GetNbinsX());
      mixedNorm1 /= hTemp1Mix->GetNbinsY()*(hTemp1Mix->GetXaxis()->FindBin(0.01) - hTemp1Mix->GetXaxis()->FindBin(-0.01) + 1);
      hTemp1Mix->Scale(1./mixedNorm1);
      Double_t mixedNorm2 = hTemp2Mix->Integral(hTemp2Mix->GetXaxis()->FindBin(0-10e-5),hTemp2Mix->GetXaxis()->FindBin(0+10e-5),1,hTemp2Mix->GetNbinsX());
      mixedNorm2 /= hTemp2Mix->GetNbinsY()*(hTemp2Mix->GetXaxis()->FindBin(0.01) - hTemp2Mix->GetXaxis()->FindBin(-0.01) + 1);
      hTemp2Mix->Scale(1./mixedNorm2);
      Double_t mixedNorm3 = hTemp3Mix->Integral(hTemp3Mix->GetXaxis()->FindBin(0-10e-5),hTemp3Mix->GetXaxis()->FindBin(0+10e-5),1,hTemp3Mix->GetNbinsX());
      mixedNorm3 /= hTemp3Mix->GetNbinsY()*(hTemp3Mix->GetXaxis()->FindBin(0.01) - hTemp3Mix->GetXaxis()->FindBin(-0.01) + 1);
      hTemp3Mix->Scale(1./mixedNorm3);
      Double_t mixedNorm4 = hTemp4Mix->Integral(hTemp4Mix->GetXaxis()->FindBin(0-10e-5),hTemp4Mix->GetXaxis()->FindBin(0+10e-5),1,hTemp4Mix->GetNbinsX());
      mixedNorm4 /= hTemp4Mix->GetNbinsY()*(hTemp4Mix->GetXaxis()->FindBin(0.01) - hTemp4Mix->GetXaxis()->FindBin(-0.01) + 1);
      hTemp4Mix->Scale(1./mixedNorm4);
      
      hTemp1->Divide(hTemp1Mix);
      hTemp2->Divide(hTemp2Mix);
      hTemp3->Divide(hTemp3Mix);
      hTemp4->Divide(hTemp4Mix);
      
      // for the first: clone
      if(iBinPsi == binPsiMin && iBinVertex == binVertexMin ){
	h1 = (TH2D*)hTemp1->Clone();
	h2 = (TH2D*)hTemp2->Clone();
	h3 = (TH2D*)hTemp3->Clone();
	h4 = (TH2D*)hTemp4->Clone();
      }
      else{  // otherwise: add for averaging
	h1->Add(hTemp1);
	h2->Add(hTemp2);
	h3->Add(hTemp3);
	h4->Add(hTemp4);
      } 
    }
  }
   
  TH2D *gHistBalanceFunctionHistogram = 0x0;
  if((h1)&&(h2)&&(h3)&&(h4)) {

    // average over number of bins nbinsVertex * nbinsPsi
    h1->Scale(1./((Double_t)(binPsiMax-binPsiMin+1)*(binVertexMax-binVertexMin+1)));
    h2->Scale(1./((Double_t)(binPsiMax-binPsiMin+1)*(binVertexMax-binVertexMin+1)));
    h3->Scale(1./((Double_t)(binPsiMax-binPsiMin+1)*(binVertexMax-binVertexMin+1)));
    h4->Scale(1./((Double_t)(binPsiMax-binPsiMin+1)*(binVertexMax-binVertexMin+1)));

    gHistBalanceFunctionHistogram = (TH2D*)h1->Clone();
    gHistBalanceFunctionHistogram->Reset();
    gHistBalanceFunctionHistogram->GetXaxis()->SetTitle("#Delta#eta");   
    gHistBalanceFunctionHistogram->GetYaxis()->SetTitle("#Delta#varphi (rad)");
    gHistBalanceFunctionHistogram->GetZaxis()->SetTitle("B(#Delta#eta,#Delta#varphi)");   

    h1->Add(h3,-1.);
    h2->Add(h4,-1.);
    
    gHistBalanceFunctionHistogram->Add(h1,h2,1.,1.);
    gHistBalanceFunctionHistogram->Scale(0.5);
    
    //normalize to bin width
    gHistBalanceFunctionHistogram->Scale(1./((Double_t)gHistBalanceFunctionHistogram->GetXaxis()->GetBinWidth(1)*(Double_t)gHistBalanceFunctionHistogram->GetYaxis()->GetBinWidth(1)));
  }
  
  return gHistBalanceFunctionHistogram;
}

//____________________________________________________________________//
TH1D *AliBalancePsi::GetBalanceFunction1DFrom2D2pMethod(Bool_t bPhi,
                                                        Double_t psiMin,
                                                        Double_t psiMax,
							Double_t vertexZMin,
							Double_t vertexZMax,
                                                        Double_t ptTriggerMin,
                                                        Double_t ptTriggerMax,
                                                        Double_t ptAssociatedMin,
                                                        Double_t ptAssociatedMax,
                                                        AliBalancePsi *bfMix) {
  //Returns the BF histogram in Delta eta OR Delta phi,
  //after dividing each correlation function by the Event Mixing one
  // (But the division is done here in 2D, this was basically done to check the Integral)
  //extracted from the 6 AliTHn objects
  //(private members) of the AliBalancePsi class.
  //iVariableSingle: 0(phi-Psi), 1(pt-trigger)
  //iVariablePair: 0(phi-Psi) 1(Delta eta), 2(Delta phi), 3(pt-trigger), 4(pt-associated
  TString histName = "gHistBalanceFunctionHistogram1D";

  if(!bfMix){
    AliError("balance function object for event mixing not available");
    return NULL;
  }

  // security checks
  if(psiMin > psiMax-0.00001){
    AliError("psiMax <= psiMin");
    return NULL;
  }
  if(vertexZMin > vertexZMax-0.00001){
    AliError("vertexZMax <= vertexZMin");
    return NULL;
  }
  if(ptTriggerMin > ptTriggerMax-0.00001){
    AliError("ptTriggerMax <= ptTriggerMin");
    return NULL;
  }
  if(ptAssociatedMin > ptAssociatedMax-0.00001){
    AliError("ptAssociatedMax <= ptAssociatedMin");
    return NULL;
  }

  // pt trigger (fixed for all vertices/psi/centralities)
  if((ptTriggerMin != -1.)&&(ptTriggerMax != -1.)) {
    fHistP->GetGrid(0)->GetGrid()->GetAxis(1)->SetRangeUser(ptTriggerMin,ptTriggerMax-0.00001);
    fHistN->GetGrid(0)->GetGrid()->GetAxis(1)->SetRangeUser(ptTriggerMin,ptTriggerMax-0.00001);
    fHistPN->GetGrid(0)->GetGrid()->GetAxis(3)->SetRangeUser(ptTriggerMin,ptTriggerMax-0.00001);
    fHistNP->GetGrid(0)->GetGrid()->GetAxis(3)->SetRangeUser(ptTriggerMin,ptTriggerMax-0.00001);
    fHistPP->GetGrid(0)->GetGrid()->GetAxis(3)->SetRangeUser(ptTriggerMin,ptTriggerMax-0.00001);
    fHistNN->GetGrid(0)->GetGrid()->GetAxis(3)->SetRangeUser(ptTriggerMin,ptTriggerMax-0.00001);
  }

  // pt associated (fixed for all vertices/psi/centralities)
  if((ptAssociatedMin != -1.)&&(ptAssociatedMax != -1.)) {
    fHistPN->GetGrid(0)->GetGrid()->GetAxis(4)->SetRangeUser(ptAssociatedMin,ptAssociatedMax-0.00001);
    fHistNP->GetGrid(0)->GetGrid()->GetAxis(4)->SetRangeUser(ptAssociatedMin,ptAssociatedMax-0.00001);
    fHistPP->GetGrid(0)->GetGrid()->GetAxis(4)->SetRangeUser(ptAssociatedMin,ptAssociatedMax-0.00001);
    fHistNN->GetGrid(0)->GetGrid()->GetAxis(4)->SetRangeUser(ptAssociatedMin,ptAssociatedMax-0.00001);
  }

  // ============================================================================================
  // the same for event mixing
  AliTHn *fHistPMix = bfMix->GetHistNp();
  AliTHn *fHistNMix = bfMix->GetHistNn();
  AliTHn *fHistPNMix = bfMix->GetHistNpn();
  AliTHn *fHistNPMix = bfMix->GetHistNnp();
  AliTHn *fHistPPMix = bfMix->GetHistNpp();
  AliTHn *fHistNNMix = bfMix->GetHistNnn();

  // pt trigger (fixed for all vertices/psi/centralities)
  if((ptTriggerMin != -1.)&&(ptTriggerMax != -1.)) {
    fHistPMix->GetGrid(0)->GetGrid()->GetAxis(1)->SetRangeUser(ptTriggerMin,ptTriggerMax-0.00001);
    fHistNMix->GetGrid(0)->GetGrid()->GetAxis(1)->SetRangeUser(ptTriggerMin,ptTriggerMax-0.00001);
    fHistPNMix->GetGrid(0)->GetGrid()->GetAxis(3)->SetRangeUser(ptTriggerMin,ptTriggerMax-0.00001);
    fHistNPMix->GetGrid(0)->GetGrid()->GetAxis(3)->SetRangeUser(ptTriggerMin,ptTriggerMax-0.00001);
    fHistPPMix->GetGrid(0)->GetGrid()->GetAxis(3)->SetRangeUser(ptTriggerMin,ptTriggerMax-0.00001);
    fHistNNMix->GetGrid(0)->GetGrid()->GetAxis(3)->SetRangeUser(ptTriggerMin,ptTriggerMax-0.00001);
  }

  // pt associated (fixed for all vertices/psi/centralities)
  if((ptAssociatedMin != -1.)&&(ptAssociatedMax != -1.)) {
    fHistPNMix->GetGrid(0)->GetGrid()->GetAxis(4)->SetRangeUser(ptAssociatedMin,ptAssociatedMax-0.00001);
    fHistNPMix->GetGrid(0)->GetGrid()->GetAxis(4)->SetRangeUser(ptAssociatedMin,ptAssociatedMax-0.00001);
    fHistPPMix->GetGrid(0)->GetGrid()->GetAxis(4)->SetRangeUser(ptAssociatedMin,ptAssociatedMax-0.00001);
    fHistNNMix->GetGrid(0)->GetGrid()->GetAxis(4)->SetRangeUser(ptAssociatedMin,ptAssociatedMax-0.00001);
  }

  // ============================================================================================
  // ranges (in bins) for vertexZ and centrality (psi)
  Int_t binPsiMin    = fHistPN->GetGrid(0)->GetGrid()->GetAxis(0)->FindBin(psiMin);
  Int_t binPsiMax    = fHistPN->GetGrid(0)->GetGrid()->GetAxis(0)->FindBin(psiMax-0.00001);
  Int_t binVertexMin = 0;
  Int_t binVertexMax = 0;
  if(fVertexBinning){
    binVertexMin = fHistPN->GetGrid(0)->GetGrid()->GetAxis(5)->FindBin(vertexZMin);
    binVertexMax = fHistPN->GetGrid(0)->GetGrid()->GetAxis(5)->FindBin(vertexZMax-0.00001);
  }  
  Double_t binPsiLowEdge    = 0.;
  Double_t binPsiUpEdge     = 0.;
  Double_t binVertexLowEdge = 0.;
  Double_t binVertexUpEdge  = 0.;

  TH2D* h1 = NULL;
  TH2D* h2 = NULL;
  TH2D* h3 = NULL;
  TH2D* h4 = NULL;

  // loop over all bins
  for(Int_t iBinPsi = binPsiMin; iBinPsi <= binPsiMax; iBinPsi++){
    for(Int_t iBinVertex = binVertexMin; iBinVertex <= binVertexMax; iBinVertex++){
  
      cout<<"In the balance function (2D->1D) loop: "<<iBinPsi<<" (psiBin), "<<iBinVertex<<" (vertexBin)  "<<endl;

      // determine the bin edges for this bin
      binPsiLowEdge    = fHistPN->GetGrid(0)->GetGrid()->GetAxis(0)->GetBinLowEdge(iBinPsi);
      binPsiUpEdge     = fHistPN->GetGrid(0)->GetGrid()->GetAxis(0)->GetBinUpEdge(iBinPsi);
      if(fVertexBinning){
	binVertexLowEdge = fHistPN->GetGrid(0)->GetGrid()->GetAxis(5)->GetBinLowEdge(iBinVertex);
	binVertexUpEdge  = fHistPN->GetGrid(0)->GetGrid()->GetAxis(5)->GetBinUpEdge(iBinVertex);
      }

      // Psi_2
      fHistP->GetGrid(0)->GetGrid()->GetAxis(0)->SetRangeUser(psiMin,psiMax-0.00001);
      fHistN->GetGrid(0)->GetGrid()->GetAxis(0)->SetRangeUser(psiMin,psiMax-0.00001);
      fHistPN->GetGrid(0)->GetGrid()->GetAxis(0)->SetRangeUser(psiMin,psiMax-0.00001);
      fHistNP->GetGrid(0)->GetGrid()->GetAxis(0)->SetRangeUser(psiMin,psiMax-0.00001);
      fHistPP->GetGrid(0)->GetGrid()->GetAxis(0)->SetRangeUser(psiMin,psiMax-0.00001);
      fHistNN->GetGrid(0)->GetGrid()->GetAxis(0)->SetRangeUser(psiMin,psiMax-0.00001);
  
      // Vz
      if(fVertexBinning){
	fHistP->GetGrid(0)->GetGrid()->GetAxis(2)->SetRangeUser(vertexZMin,vertexZMax-0.00001); 
	fHistN->GetGrid(0)->GetGrid()->GetAxis(2)->SetRangeUser(vertexZMin,vertexZMax-0.00001); 
	fHistPN->GetGrid(0)->GetGrid()->GetAxis(5)->SetRangeUser(vertexZMin,vertexZMax-0.00001); 
	fHistNP->GetGrid(0)->GetGrid()->GetAxis(5)->SetRangeUser(vertexZMin,vertexZMax-0.00001); 
	fHistPP->GetGrid(0)->GetGrid()->GetAxis(5)->SetRangeUser(vertexZMin,vertexZMax-0.00001); 
	fHistNN->GetGrid(0)->GetGrid()->GetAxis(5)->SetRangeUser(vertexZMin,vertexZMax-0.00001); 
      }

      // ============================================================================================
      // the same for event mixing
      
      // Psi_2
      fHistPMix->GetGrid(0)->GetGrid()->GetAxis(0)->SetRangeUser(psiMin,psiMax-0.00001);
      fHistNMix->GetGrid(0)->GetGrid()->GetAxis(0)->SetRangeUser(psiMin,psiMax-0.00001);
      fHistPNMix->GetGrid(0)->GetGrid()->GetAxis(0)->SetRangeUser(psiMin,psiMax-0.00001);
      fHistNPMix->GetGrid(0)->GetGrid()->GetAxis(0)->SetRangeUser(psiMin,psiMax-0.00001);
      fHistPPMix->GetGrid(0)->GetGrid()->GetAxis(0)->SetRangeUser(psiMin,psiMax-0.00001);
      fHistNNMix->GetGrid(0)->GetGrid()->GetAxis(0)->SetRangeUser(psiMin,psiMax-0.00001);

      // Vz
      if(fVertexBinning){
	fHistPMix->GetGrid(0)->GetGrid()->GetAxis(2)->SetRangeUser(vertexZMin,vertexZMax-0.00001); 
	fHistNMix->GetGrid(0)->GetGrid()->GetAxis(2)->SetRangeUser(vertexZMin,vertexZMax-0.00001); 
	fHistPNMix->GetGrid(0)->GetGrid()->GetAxis(5)->SetRangeUser(vertexZMin,vertexZMax-0.00001); 
	fHistNPMix->GetGrid(0)->GetGrid()->GetAxis(5)->SetRangeUser(vertexZMin,vertexZMax-0.00001); 
	fHistPPMix->GetGrid(0)->GetGrid()->GetAxis(5)->SetRangeUser(vertexZMin,vertexZMax-0.00001); 
	fHistNNMix->GetGrid(0)->GetGrid()->GetAxis(5)->SetRangeUser(vertexZMin,vertexZMax-0.00001); 
      }
      // ============================================================================================

      //AliInfo(Form("P:%lf - N:%lf - PN:%lf - NP:%lf - PP:%lf - NN:%lf",fHistP->GetEntries(0),fHistN->GetEntries(0),fHistPN->GetEntries(0),fHistNP->GetEntries(0),fHistPP->GetEntries(0),fHistNN->GetEntries(0)));
      
      // Project into the wanted space (1st: analysis step, 2nd: axis)
      TH2D* hTemp1 = (TH2D*)fHistPN->Project(0,1,2);
      TH2D* hTemp2 = (TH2D*)fHistNP->Project(0,1,2);
      TH2D* hTemp3 = (TH2D*)fHistPP->Project(0,1,2);
      TH2D* hTemp4 = (TH2D*)fHistNN->Project(0,1,2);
      TH1D* hTemp5 = (TH1D*)fHistP->Project(0,1);
      TH1D* hTemp6 = (TH1D*)fHistN->Project(0,1);

      // ============================================================================================
      // the same for event mixing
      TH2D* hTemp1Mix = (TH2D*)fHistPNMix->Project(0,1,2);
      TH2D* hTemp2Mix = (TH2D*)fHistNPMix->Project(0,1,2);
      TH2D* hTemp3Mix = (TH2D*)fHistPPMix->Project(0,1,2);
      TH2D* hTemp4Mix = (TH2D*)fHistNNMix->Project(0,1,2);
      // TH1D* hTemp5Mix = (TH1D*)fHistPMix->Project(0,1);
      // TH1D* hTemp6Mix = (TH1D*)fHistNMix->Project(0,1);
      // ============================================================================================
      
      hTemp1->Sumw2();
      hTemp2->Sumw2();
      hTemp3->Sumw2();
      hTemp4->Sumw2();
      hTemp1Mix->Sumw2();
      hTemp2Mix->Sumw2();
      hTemp3Mix->Sumw2();
      hTemp4Mix->Sumw2();
      
      hTemp1->Scale(1./hTemp5->Integral());
      hTemp3->Scale(1./hTemp5->Integral());
      hTemp2->Scale(1./hTemp6->Integral());
      hTemp4->Scale(1./hTemp6->Integral());

      // normalization of Event mixing to 1 at (0,0) --> Jan Fietes method
      Double_t mixedNorm1 = hTemp1Mix->Integral(hTemp1Mix->GetXaxis()->FindBin(0-10e-5),hTemp1Mix->GetXaxis()->FindBin(0+10e-5),1,hTemp1Mix->GetNbinsX());
      mixedNorm1 /= hTemp1Mix->GetNbinsY()*(hTemp1Mix->GetXaxis()->FindBin(0.01) - hTemp1Mix->GetXaxis()->FindBin(-0.01) + 1);
      hTemp1Mix->Scale(1./mixedNorm1);
      Double_t mixedNorm2 = hTemp2Mix->Integral(hTemp2Mix->GetXaxis()->FindBin(0-10e-5),hTemp2Mix->GetXaxis()->FindBin(0+10e-5),1,hTemp2Mix->GetNbinsX());
      mixedNorm2 /= hTemp2Mix->GetNbinsY()*(hTemp2Mix->GetXaxis()->FindBin(0.01) - hTemp2Mix->GetXaxis()->FindBin(-0.01) + 1);
      hTemp2Mix->Scale(1./mixedNorm2);
      Double_t mixedNorm3 = hTemp3Mix->Integral(hTemp3Mix->GetXaxis()->FindBin(0-10e-5),hTemp3Mix->GetXaxis()->FindBin(0+10e-5),1,hTemp3Mix->GetNbinsX());
      mixedNorm3 /= hTemp3Mix->GetNbinsY()*(hTemp3Mix->GetXaxis()->FindBin(0.01) - hTemp3Mix->GetXaxis()->FindBin(-0.01) + 1);
      hTemp3Mix->Scale(1./mixedNorm3);
      Double_t mixedNorm4 = hTemp4Mix->Integral(hTemp4Mix->GetXaxis()->FindBin(0-10e-5),hTemp4Mix->GetXaxis()->FindBin(0+10e-5),1,hTemp4Mix->GetNbinsX());
      mixedNorm4 /= hTemp4Mix->GetNbinsY()*(hTemp4Mix->GetXaxis()->FindBin(0.01) - hTemp4Mix->GetXaxis()->FindBin(-0.01) + 1);
      hTemp4Mix->Scale(1./mixedNorm4);

      hTemp1->Divide(hTemp1Mix);
      hTemp2->Divide(hTemp2Mix);
      hTemp3->Divide(hTemp3Mix);
      hTemp4->Divide(hTemp4Mix);

      // for the first: clone
      if(iBinPsi == binPsiMin && iBinVertex == binVertexMin ){
	h1 = (TH2D*)hTemp1->Clone();
	h2 = (TH2D*)hTemp2->Clone();
	h3 = (TH2D*)hTemp3->Clone();
	h4 = (TH2D*)hTemp4->Clone();
      }
      else{  // otherwise: add for averaging
	h1->Add(hTemp1);
	h2->Add(hTemp2);
	h3->Add(hTemp3);
	h4->Add(hTemp4);
      }

    }
  }

  TH1D *gHistBalanceFunctionHistogram = 0x0;
  if((h1)&&(h2)&&(h3)&&(h4)) {

    // average over number of bins nbinsVertex * nbinsPsi
    h1->Scale(1./((Double_t)(binPsiMax-binPsiMin+1)*(binVertexMax-binVertexMin+1)));
    h2->Scale(1./((Double_t)(binPsiMax-binPsiMin+1)*(binVertexMax-binVertexMin+1)));
    h3->Scale(1./((Double_t)(binPsiMax-binPsiMin+1)*(binVertexMax-binVertexMin+1)));
    h4->Scale(1./((Double_t)(binPsiMax-binPsiMin+1)*(binVertexMax-binVertexMin+1)));

    // now only project on one axis
    TH1D *h1DTemp1 = NULL;
    TH1D *h1DTemp2 = NULL;
    TH1D *h1DTemp3 = NULL;
    TH1D *h1DTemp4 = NULL;
    if(!bPhi){
      h1DTemp1 = (TH1D*)h1->ProjectionX(Form("%s_projX",h1->GetName()));
      h1DTemp2 = (TH1D*)h2->ProjectionX(Form("%s_projX",h2->GetName()));
      h1DTemp3 = (TH1D*)h3->ProjectionX(Form("%s_projX",h3->GetName()));
      h1DTemp4 = (TH1D*)h4->ProjectionX(Form("%s_projX",h4->GetName()));
    }
    else{
      h1DTemp1 = (TH1D*)h1->ProjectionY(Form("%s_projY",h1->GetName()));
      h1DTemp2 = (TH1D*)h2->ProjectionY(Form("%s_projY",h2->GetName()));
      h1DTemp3 = (TH1D*)h3->ProjectionY(Form("%s_projY",h3->GetName()));
      h1DTemp4 = (TH1D*)h4->ProjectionY(Form("%s_projY",h4->GetName()));
    }

    gHistBalanceFunctionHistogram = (TH1D*)h1DTemp1->Clone(Form("%s_clone",h1DTemp1->GetName()));
    gHistBalanceFunctionHistogram->Reset();
    if(!bPhi){
      gHistBalanceFunctionHistogram->GetXaxis()->SetTitle("#Delta#eta");  
      gHistBalanceFunctionHistogram->GetYaxis()->SetTitle("B(#Delta#eta)");  
    }
    else{
      gHistBalanceFunctionHistogram->GetXaxis()->SetTitle("#Delta#varphi (rad)");
      gHistBalanceFunctionHistogram->GetYaxis()->SetTitle("B(#Delta#varphi)");  
    }

    h1DTemp1->Add(h1DTemp3,-1.);
    h1DTemp2->Add(h1DTemp4,-1.);

    gHistBalanceFunctionHistogram->Add(h1DTemp1,h1DTemp2,1.,1.);
    gHistBalanceFunctionHistogram->Scale(0.5);

    //normalize to bin width
    gHistBalanceFunctionHistogram->Scale(1./((Double_t)gHistBalanceFunctionHistogram->GetXaxis()->GetBinWidth(1)));
  }

  return gHistBalanceFunctionHistogram;
}


//____________________________________________________________________//
TH2D *AliBalancePsi::GetCorrelationFunction(TString type,
					    Double_t psiMin, 
					    Double_t psiMax,
					    Double_t vertexZMin,
					    Double_t vertexZMax,
					    Double_t ptTriggerMin,
					    Double_t ptTriggerMax,
					    Double_t ptAssociatedMin,
					    Double_t ptAssociatedMax,
					    AliBalancePsi *bMixed,
					    Bool_t normToTrig) {

  // Returns the 2D correlation function for "type"(PN,NP,PP,NN) pairs,
  // does the division by event mixing inside,
  // and averaging over several vertexZ and centrality bins

  // security checks
  if(type != "PN" && type != "NP" && type != "PP" && type != "NN" && type != "ALL"){
    AliError("Only types allowed: PN,NP,PP,NN,ALL");
    return NULL;
  }
  if(!bMixed){
    AliError("No Event Mixing AliTHn");
    return NULL;
  }

  TH2D *gHist  = NULL;
  TH2D *fSame  = NULL;
  TH2D *fMixed = NULL;

  // ranges (in bins) for vertexZ and centrality (psi)
  Int_t binPsiMin    = fHistPN->GetGrid(0)->GetGrid()->GetAxis(0)->FindBin(psiMin);
  Int_t binPsiMax    = fHistPN->GetGrid(0)->GetGrid()->GetAxis(0)->FindBin(psiMax-0.00001);
  Int_t binVertexMin = 0;
  Int_t binVertexMax = 0;
  if(fVertexBinning){
    binVertexMin = fHistPN->GetGrid(0)->GetGrid()->GetAxis(5)->FindBin(vertexZMin);
    binVertexMax = fHistPN->GetGrid(0)->GetGrid()->GetAxis(5)->FindBin(vertexZMax-0.00001);
  }  
  Double_t binPsiLowEdge    = 0.;
  Double_t binPsiUpEdge     = 1000.;
  Double_t binVertexLowEdge = 0.;
  Double_t binVertexUpEdge  = 1000.;

  // loop over all bins
  for(Int_t iBinPsi = binPsiMin; iBinPsi <= binPsiMax; iBinPsi++){
    for(Int_t iBinVertex = binVertexMin; iBinVertex <= binVertexMax; iBinVertex++){

      cout<<"In the correlation function loop: "<<iBinPsi<<" (psiBin), "<<iBinVertex<<" (vertexBin)  "<<endl;

      // determine the bin edges for this bin
      binPsiLowEdge    = fHistPN->GetGrid(0)->GetGrid()->GetAxis(0)->GetBinLowEdge(iBinPsi);
      binPsiUpEdge     = fHistPN->GetGrid(0)->GetGrid()->GetAxis(0)->GetBinUpEdge(iBinPsi);
      if(fVertexBinning){
	binVertexLowEdge = fHistPN->GetGrid(0)->GetGrid()->GetAxis(5)->GetBinLowEdge(iBinVertex);
	binVertexUpEdge  = fHistPN->GetGrid(0)->GetGrid()->GetAxis(5)->GetBinUpEdge(iBinVertex);
      }

      // get the 2D histograms for the correct type
      if(type=="PN"){
	fSame  = GetCorrelationFunctionPN(binPsiLowEdge,binPsiUpEdge,binVertexLowEdge,binVertexUpEdge,ptTriggerMin,ptTriggerMax,ptAssociatedMin,ptAssociatedMax);
	fMixed = bMixed->GetCorrelationFunctionPN(binPsiLowEdge,binPsiUpEdge,binVertexLowEdge,binVertexUpEdge,ptTriggerMin,ptTriggerMax,ptAssociatedMin,ptAssociatedMax);
      }
      else if(type=="NP"){
	fSame  = GetCorrelationFunctionNP(binPsiLowEdge,binPsiUpEdge,binVertexLowEdge,binVertexUpEdge,ptTriggerMin,ptTriggerMax,ptAssociatedMin,ptAssociatedMax);
	fMixed = bMixed->GetCorrelationFunctionNP(binPsiLowEdge,binPsiUpEdge,binVertexLowEdge,binVertexUpEdge,ptTriggerMin,ptTriggerMax,ptAssociatedMin,ptAssociatedMax);
      }
      else if(type=="PP"){
	fSame  = GetCorrelationFunctionPP(binPsiLowEdge,binPsiUpEdge,binVertexLowEdge,binVertexUpEdge,ptTriggerMin,ptTriggerMax,ptAssociatedMin,ptAssociatedMax);
	fMixed = bMixed->GetCorrelationFunctionPP(binPsiLowEdge,binPsiUpEdge,binVertexLowEdge,binVertexUpEdge,ptTriggerMin,ptTriggerMax,ptAssociatedMin,ptAssociatedMax);
      }
      else if(type=="NN"){
	fSame  = GetCorrelationFunctionNN(binPsiLowEdge,binPsiUpEdge,binVertexLowEdge,binVertexUpEdge,ptTriggerMin,ptTriggerMax,ptAssociatedMin,ptAssociatedMax);
	fMixed = bMixed->GetCorrelationFunctionNN(binPsiLowEdge,binPsiUpEdge,binVertexLowEdge,binVertexUpEdge,ptTriggerMin,ptTriggerMax,ptAssociatedMin,ptAssociatedMax);
      }
      else if(type=="ALL"){
	fSame  = GetCorrelationFunctionChargeIndependent(binPsiLowEdge,binPsiUpEdge,binVertexLowEdge,binVertexUpEdge,ptTriggerMin,ptTriggerMax,ptAssociatedMin,ptAssociatedMax);
	fMixed = bMixed->GetCorrelationFunctionChargeIndependent(binPsiLowEdge,binPsiUpEdge,binVertexLowEdge,binVertexUpEdge,ptTriggerMin,ptTriggerMax,ptAssociatedMin,ptAssociatedMax);
      }

      if(fMixed && normToTrig){
	
	// normalization of Event mixing to 1 at (0,0) --> Jan Fietes method
	// do it only on away-side (due to two-track cuts)
	Double_t mixedNorm = fMixed->Integral(fMixed->GetXaxis()->FindBin(0-10e-5),fMixed->GetXaxis()->FindBin(0+10e-5),fMixed->GetNbinsY()/2+1,fMixed->GetNbinsY());
	mixedNorm /= 0.5 * fMixed->GetNbinsY() *(fMixed->GetXaxis()->FindBin(0.01) - fMixed->GetXaxis()->FindBin(-0.01) + 1);

	// finite bin correction
	Double_t binWidthEta = fMixed->GetXaxis()->GetBinWidth(fMixed->GetNbinsX());
	Double_t maxEta      = fMixed->GetXaxis()->GetBinUpEdge(fMixed->GetNbinsX());
	
	Double_t finiteBinCorrection = -1.0 / (2*maxEta) * binWidthEta / 2 + 1;
	//Printf("Finite bin correction: %f", finiteBinCorrection);
	mixedNorm /= finiteBinCorrection;
	
	fMixed->Scale(1./mixedNorm);
      }

      if(fSame && fMixed){
	// then get the correlation function (divide fSame/fmixed)
	fSame->Divide(fMixed);
	
	// NEW averaging:
	// average over number of triggers in each sub-bin
	Double_t NTrigSubBin = 0;
	if(type=="PN" || type=="PP")
	  NTrigSubBin = (Double_t)(fHistP->Project(0,1)->Integral());
	else if(type=="NP" || type=="NN")
	  NTrigSubBin = (Double_t)(fHistN->Project(0,1)->Integral());
	fSame->Scale(NTrigSubBin);
	
	// for the first: clone
	if( (iBinPsi == binPsiMin && iBinVertex == binVertexMin) || !gHist ){
	  gHist = (TH2D*)fSame->Clone();
	}
	else{  // otherwise: add for averaging
	  gHist->Add(fSame);
	}
      }
    }
  }

  if(gHist){
    
    // OLD averaging:
    // average over number of bins nbinsVertex * nbinsPsi
    // gHist->Scale(1./((Double_t)(binPsiMax-binPsiMin+1)*(binVertexMax-binVertexMin+1)));

    // NEW averaging:
    // average over number of triggers in each sub-bin
    // first set to full range and then obtain number of all triggers 
    Double_t NTrigAll = 0;
    if(type=="PN" || type=="PP"){
      fHistP->GetGrid(0)->GetGrid()->GetAxis(0)->SetRangeUser(psiMin,psiMax-0.00001); 
      fHistP->GetGrid(0)->GetGrid()->GetAxis(2)->SetRangeUser(vertexZMin,vertexZMax-0.00001); 
      fHistP->GetGrid(0)->GetGrid()->GetAxis(1)->SetRangeUser(ptTriggerMin,ptTriggerMax-0.00001);
      NTrigAll = (Double_t)(fHistP->Project(0,1)->Integral());
    }
    else if(type=="NP" || type=="NN"){
      fHistN->GetGrid(0)->GetGrid()->GetAxis(0)->SetRangeUser(psiMin,psiMax-0.00001); 
      fHistN->GetGrid(0)->GetGrid()->GetAxis(2)->SetRangeUser(vertexZMin,vertexZMax-0.00001); 
      fHistN->GetGrid(0)->GetGrid()->GetAxis(1)->SetRangeUser(ptTriggerMin,ptTriggerMax-0.00001);
      NTrigAll = (Double_t)(fHistN->Project(0,1)->Integral());
    }
    gHist->Scale(1./NTrigAll);
    
  }
  
  return gHist;
}


//____________________________________________________________________//
TH2D *AliBalancePsi::GetCorrelationFunctionPN(Double_t psiMin, 
					      Double_t psiMax,
					      Double_t vertexZMin,
					      Double_t vertexZMax,
					      Double_t ptTriggerMin,
					      Double_t ptTriggerMax,
					      Double_t ptAssociatedMin,
					      Double_t ptAssociatedMax) {
  //Returns the 2D correlation function for (+-) pairs

  // security checks
  if(psiMin > psiMax-0.00001){
    AliError("psiMax <= psiMin");
    return NULL;
  }
  if(vertexZMin > vertexZMax-0.00001){
    AliError("vertexZMax <= vertexZMin");
    return NULL;
  }
  if(ptTriggerMin > ptTriggerMax-0.00001){
    AliError("ptTriggerMax <= ptTriggerMin");
    return NULL;
  }
  if(ptAssociatedMin > ptAssociatedMax-0.00001){
    AliError("ptAssociatedMax <= ptAssociatedMin");
    return NULL;
  }

  // Psi_2: axis 0
  fHistP->GetGrid(0)->GetGrid()->GetAxis(0)->SetRangeUser(psiMin,psiMax-0.00001); 
  fHistPN->GetGrid(0)->GetGrid()->GetAxis(0)->SetRangeUser(psiMin,psiMax-0.00001); 

  // Vz
  if(fVertexBinning){
    //Printf("Cutting on Vz...");
    fHistP->GetGrid(0)->GetGrid()->GetAxis(2)->SetRangeUser(vertexZMin,vertexZMax-0.00001); 
    fHistPN->GetGrid(0)->GetGrid()->GetAxis(5)->SetRangeUser(vertexZMin,vertexZMax-0.00001); 
  }

  // pt trigger
  if((ptTriggerMin != -1.)&&(ptTriggerMax != -1.)) {
    fHistP->GetGrid(0)->GetGrid()->GetAxis(1)->SetRangeUser(ptTriggerMin,ptTriggerMax-0.00001);
    fHistPN->GetGrid(0)->GetGrid()->GetAxis(3)->SetRangeUser(ptTriggerMin,ptTriggerMax-0.00001);
  }

  // pt associated
  if((ptAssociatedMin != -1.)&&(ptAssociatedMax != -1.))
    fHistPN->GetGrid(0)->GetGrid()->GetAxis(4)->SetRangeUser(ptAssociatedMin,ptAssociatedMax-0.00001);

  //fHistP->GetGrid(0)->GetGrid()->GetAxis(0)->SetRangeUser(-0.5,2.5); 
  //fHistPN->GetGrid(0)->GetGrid()->GetAxis(0)->SetRangeUser(-0.5,2.5); 

  //TH2D *gHistTest = dynamic_cast<TH2D *>(fHistP->Project(0,0,1));
  //TCanvas *c1 = new TCanvas("c1","");
  //c1->cd();
  //if(!gHistTest){
  //AliError("Projection of fHistP = NULL");
  //return gHistTest;
  //}
  //else{
  //gHistTest->DrawCopy("colz");
  //}

  //0:step, 1: Delta eta, 2: Delta phi
  TH2D *gHist = dynamic_cast<TH2D *>(fHistPN->Project(0,1,2));
  if(!gHist){
    AliError("Projection of fHistPN = NULL");
    return gHist;
  }

  //AliInfo(Form("Entries (test): %lf",(Double_t)(gHistTest->GetEntries())));
  //AliInfo(Form("Entries (1D): %lf",(Double_t)(fHistP->Project(0,1)->GetEntries())));
  //AliInfo(Form("Entries (2D): %lf",(Double_t)(fHistPN->Project(0,1,2)->GetEntries())));
  //AliInfo(Form("Integral (1D): %lf",(Double_t)(fHistP->Project(0,1)->Integral())));
  //AliInfo(Form("Integral (2D): %lf",(Double_t)(fHistPN->Project(0,1,2)->Integral())));
  
  //TCanvas *c2 = new TCanvas("c2","");
  //c2->cd();
  //fHistPN->Project(0,1,2)->DrawCopy("colz");

  if((Double_t)(fHistP->Project(0,1)->Integral())>0)
    gHist->Scale(1./(Double_t)(fHistP->Project(0,1)->Integral()));

  //normalize to bin width
  gHist->Scale(1./((Double_t)gHist->GetXaxis()->GetBinWidth(1)*(Double_t)gHist->GetYaxis()->GetBinWidth(1)));
    
  return gHist;
}

//____________________________________________________________________//
TH2D *AliBalancePsi::GetCorrelationFunctionNP(Double_t psiMin, 
					      Double_t psiMax,
					      Double_t vertexZMin,
					      Double_t vertexZMax,
					      Double_t ptTriggerMin,
					      Double_t ptTriggerMax,
					      Double_t ptAssociatedMin,
					      Double_t ptAssociatedMax) {
  //Returns the 2D correlation function for (+-) pairs

  // security checks
  if(psiMin > psiMax-0.00001){
    AliError("psiMax <= psiMin");
    return NULL;
  }
  if(vertexZMin > vertexZMax-0.00001){
    AliError("vertexZMax <= vertexZMin");
    return NULL;
  }
  if(ptTriggerMin > ptTriggerMax-0.00001){
    AliError("ptTriggerMax <= ptTriggerMin");
    return NULL;
  }
  if(ptAssociatedMin > ptAssociatedMax-0.00001){
    AliError("ptAssociatedMax <= ptAssociatedMin");
    return NULL;
  }

  // Psi_2: axis 0
  fHistN->GetGrid(0)->GetGrid()->GetAxis(0)->SetRangeUser(psiMin,psiMax-0.00001); 
  fHistNP->GetGrid(0)->GetGrid()->GetAxis(0)->SetRangeUser(psiMin,psiMax-0.00001); 

  // Vz
  if(fVertexBinning){
    fHistN->GetGrid(0)->GetGrid()->GetAxis(2)->SetRangeUser(vertexZMin,vertexZMax-0.00001); 
    fHistNP->GetGrid(0)->GetGrid()->GetAxis(5)->SetRangeUser(vertexZMin,vertexZMax-0.00001); 
  }
    
  // pt trigger
  if((ptTriggerMin != -1.)&&(ptTriggerMax != -1.)) {
    fHistN->GetGrid(0)->GetGrid()->GetAxis(1)->SetRangeUser(ptTriggerMin,ptTriggerMax-0.00001);
    fHistNP->GetGrid(0)->GetGrid()->GetAxis(3)->SetRangeUser(ptTriggerMin,ptTriggerMax-0.00001);
  }

  // pt associated
  if((ptAssociatedMin != -1.)&&(ptAssociatedMax != -1.))
    fHistNP->GetGrid(0)->GetGrid()->GetAxis(4)->SetRangeUser(ptAssociatedMin,ptAssociatedMax-0.00001);

  //0:step, 1: Delta eta, 2: Delta phi
  TH2D *gHist = dynamic_cast<TH2D *>(fHistNP->Project(0,1,2));
  if(!gHist){
    AliError("Projection of fHistPN = NULL");
    return gHist;
  }

  //Printf("Entries (1D): %lf",(Double_t)(fHistN->Project(0,2)->GetEntries()));
  //Printf("Entries (2D): %lf",(Double_t)(fHistNP->Project(0,2,3)->GetEntries()));
  if((Double_t)(fHistN->Project(0,1)->Integral())>0)
    gHist->Scale(1./(Double_t)(fHistN->Project(0,1)->Integral()));

  //normalize to bin width
  gHist->Scale(1./((Double_t)gHist->GetXaxis()->GetBinWidth(1)*(Double_t)gHist->GetYaxis()->GetBinWidth(1)));
    
  return gHist;
}

//____________________________________________________________________//
TH2D *AliBalancePsi::GetCorrelationFunctionPP(Double_t psiMin, 
					      Double_t psiMax,
					      Double_t vertexZMin,
					      Double_t vertexZMax,
					      Double_t ptTriggerMin,
					      Double_t ptTriggerMax,
					      Double_t ptAssociatedMin,
					      Double_t ptAssociatedMax) {
  //Returns the 2D correlation function for (+-) pairs

  // security checks
  if(psiMin > psiMax-0.00001){
    AliError("psiMax <= psiMin");
    return NULL;
  }
  if(vertexZMin > vertexZMax-0.00001){
    AliError("vertexZMax <= vertexZMin");
    return NULL;
  }
  if(ptTriggerMin > ptTriggerMax-0.00001){
    AliError("ptTriggerMax <= ptTriggerMin");
    return NULL;
  }
  if(ptAssociatedMin > ptAssociatedMax-0.00001){
    AliError("ptAssociatedMax <= ptAssociatedMin");
    return NULL;
  }

  // Psi_2: axis 0
  fHistP->GetGrid(0)->GetGrid()->GetAxis(0)->SetRangeUser(psiMin,psiMax-0.00001); 
  fHistPP->GetGrid(0)->GetGrid()->GetAxis(0)->SetRangeUser(psiMin,psiMax-0.00001); 

  // Vz
  if(fVertexBinning){
    fHistP->GetGrid(0)->GetGrid()->GetAxis(2)->SetRangeUser(vertexZMin,vertexZMax-0.00001); 
    fHistPP->GetGrid(0)->GetGrid()->GetAxis(5)->SetRangeUser(vertexZMin,vertexZMax-0.00001); 
  }

  // pt trigger
  if((ptTriggerMin != -1.)&&(ptTriggerMax != -1.)) {
    fHistP->GetGrid(0)->GetGrid()->GetAxis(1)->SetRangeUser(ptTriggerMin,ptTriggerMax-0.00001);
    fHistPP->GetGrid(0)->GetGrid()->GetAxis(3)->SetRangeUser(ptTriggerMin,ptTriggerMax-0.00001);
  }

  // pt associated
  if((ptAssociatedMin != -1.)&&(ptAssociatedMax != -1.))
    fHistPP->GetGrid(0)->GetGrid()->GetAxis(4)->SetRangeUser(ptAssociatedMin,ptAssociatedMax-0.00001);
      
  //0:step, 1: Delta eta, 2: Delta phi
  TH2D *gHist = dynamic_cast<TH2D *>(fHistPP->Project(0,1,2));
  if(!gHist){
    AliError("Projection of fHistPN = NULL");
    return gHist;
  }

  //Printf("Entries (1D): %lf",(Double_t)(fHistP->Project(0,2)->GetEntries()));
  //Printf("Entries (2D): %lf",(Double_t)(fHistPP->Project(0,2,3)->GetEntries()));
  if((Double_t)(fHistP->Project(0,1)->Integral())>0)
    gHist->Scale(1./(Double_t)(fHistP->Project(0,1)->Integral()));

  //normalize to bin width
  gHist->Scale(1./((Double_t)gHist->GetXaxis()->GetBinWidth(1)*(Double_t)gHist->GetYaxis()->GetBinWidth(1)));
  
  return gHist;
}

//____________________________________________________________________//
TH2D *AliBalancePsi::GetCorrelationFunctionNN(Double_t psiMin, 
					      Double_t psiMax,
					      Double_t vertexZMin,
					      Double_t vertexZMax,
					      Double_t ptTriggerMin,
					      Double_t ptTriggerMax,
					      Double_t ptAssociatedMin,
					      Double_t ptAssociatedMax) {
  //Returns the 2D correlation function for (+-) pairs

  // security checks
  if(psiMin > psiMax-0.00001){
    AliError("psiMax <= psiMin");
    return NULL;
  }
  if(vertexZMin > vertexZMax-0.00001){
    AliError("vertexZMax <= vertexZMin");
    return NULL;
  }
  if(ptTriggerMin > ptTriggerMax-0.00001){
    AliError("ptTriggerMax <= ptTriggerMin");
    return NULL;
  }
  if(ptAssociatedMin > ptAssociatedMax-0.00001){
    AliError("ptAssociatedMax <= ptAssociatedMin");
    return NULL;
  }

  // Psi_2: axis 0
  fHistN->GetGrid(0)->GetGrid()->GetAxis(0)->SetRangeUser(psiMin,psiMax-0.00001); 
  fHistNN->GetGrid(0)->GetGrid()->GetAxis(0)->SetRangeUser(psiMin,psiMax-0.00001); 

  // Vz
  if(fVertexBinning){
    fHistN->GetGrid(0)->GetGrid()->GetAxis(2)->SetRangeUser(vertexZMin,vertexZMax-0.00001); 
    fHistNN->GetGrid(0)->GetGrid()->GetAxis(5)->SetRangeUser(vertexZMin,vertexZMax-0.00001); 
  }

  // pt trigger
  if((ptTriggerMin != -1.)&&(ptTriggerMax != -1.)) {
    fHistN->GetGrid(0)->GetGrid()->GetAxis(1)->SetRangeUser(ptTriggerMin,ptTriggerMax-0.00001);
    fHistNN->GetGrid(0)->GetGrid()->GetAxis(3)->SetRangeUser(ptTriggerMin,ptTriggerMax-0.00001);
  }

  // pt associated
  if((ptAssociatedMin != -1.)&&(ptAssociatedMax != -1.))
    fHistNN->GetGrid(0)->GetGrid()->GetAxis(4)->SetRangeUser(ptAssociatedMin,ptAssociatedMax-0.00001);
    
  //0:step, 1: Delta eta, 2: Delta phi
  TH2D *gHist = dynamic_cast<TH2D *>(fHistNN->Project(0,1,2));
  if(!gHist){
    AliError("Projection of fHistPN = NULL");
    return gHist;
  }

  //Printf("Entries (1D): %lf",(Double_t)(fHistN->Project(0,2)->GetEntries()));
  //Printf("Entries (2D): %lf",(Double_t)(fHistNN->Project(0,2,3)->GetEntries()));
  if((Double_t)(fHistN->Project(0,1)->Integral())>0)
    gHist->Scale(1./(Double_t)(fHistN->Project(0,1)->Integral()));

  //normalize to bin width
  gHist->Scale(1./((Double_t)gHist->GetXaxis()->GetBinWidth(1)*(Double_t)gHist->GetYaxis()->GetBinWidth(1)));
    
  return gHist;
}

//____________________________________________________________________//
TH2D *AliBalancePsi::GetCorrelationFunctionChargeIndependent(Double_t psiMin, 
							     Double_t psiMax,
							     Double_t vertexZMin,
							     Double_t vertexZMax,
							     Double_t ptTriggerMin,
							     Double_t ptTriggerMax,
							     Double_t ptAssociatedMin,
							     Double_t ptAssociatedMax) {
  //Returns the 2D correlation function for the sum of all charge combination pairs

  // security checks
  if(psiMin > psiMax-0.00001){
    AliError("psiMax <= psiMin");
    return NULL;
  }
  if(vertexZMin > vertexZMax-0.00001){
    AliError("vertexZMax <= vertexZMin");
    return NULL;
  }
  if(ptTriggerMin > ptTriggerMax-0.00001){
    AliError("ptTriggerMax <= ptTriggerMin");
    return NULL;
  }
  if(ptAssociatedMin > ptAssociatedMax-0.00001){
    AliError("ptAssociatedMax <= ptAssociatedMin");
    return NULL;
  }

  // Psi_2: axis 0
  fHistN->GetGrid(0)->GetGrid()->GetAxis(0)->SetRangeUser(psiMin,psiMax-0.00001); 
  fHistP->GetGrid(0)->GetGrid()->GetAxis(0)->SetRangeUser(psiMin,psiMax-0.00001); 
  fHistNN->GetGrid(0)->GetGrid()->GetAxis(0)->SetRangeUser(psiMin,psiMax-0.00001); 
  fHistPP->GetGrid(0)->GetGrid()->GetAxis(0)->SetRangeUser(psiMin,psiMax-0.00001); 
  fHistNP->GetGrid(0)->GetGrid()->GetAxis(0)->SetRangeUser(psiMin,psiMax-0.00001); 
  fHistPN->GetGrid(0)->GetGrid()->GetAxis(0)->SetRangeUser(psiMin,psiMax-0.00001); 
 
  // Vz
  if(fVertexBinning){
    fHistP->GetGrid(0)->GetGrid()->GetAxis(2)->SetRangeUser(vertexZMin,vertexZMax-0.00001); 
    fHistN->GetGrid(0)->GetGrid()->GetAxis(2)->SetRangeUser(vertexZMin,vertexZMax-0.00001); 
    fHistPN->GetGrid(0)->GetGrid()->GetAxis(5)->SetRangeUser(vertexZMin,vertexZMax-0.00001); 
    fHistNP->GetGrid(0)->GetGrid()->GetAxis(5)->SetRangeUser(vertexZMin,vertexZMax-0.00001); 
    fHistPP->GetGrid(0)->GetGrid()->GetAxis(5)->SetRangeUser(vertexZMin,vertexZMax-0.00001); 
    fHistNN->GetGrid(0)->GetGrid()->GetAxis(5)->SetRangeUser(vertexZMin,vertexZMax-0.00001); 
  }

  // pt trigger
  if((ptTriggerMin != -1.)&&(ptTriggerMax != -1.)) {
    fHistN->GetGrid(0)->GetGrid()->GetAxis(1)->SetRangeUser(ptTriggerMin,ptTriggerMax-0.00001);
    fHistP->GetGrid(0)->GetGrid()->GetAxis(1)->SetRangeUser(ptTriggerMin,ptTriggerMax-0.00001);
    fHistNN->GetGrid(0)->GetGrid()->GetAxis(3)->SetRangeUser(ptTriggerMin,ptTriggerMax-0.00001);
    fHistPP->GetGrid(0)->GetGrid()->GetAxis(3)->SetRangeUser(ptTriggerMin,ptTriggerMax-0.00001);
    fHistNP->GetGrid(0)->GetGrid()->GetAxis(3)->SetRangeUser(ptTriggerMin,ptTriggerMax-0.00001);
    fHistPN->GetGrid(0)->GetGrid()->GetAxis(3)->SetRangeUser(ptTriggerMin,ptTriggerMax-0.00001);
  }

  // pt associated
  if((ptAssociatedMin != -1.)&&(ptAssociatedMax != -1.))
    fHistNN->GetGrid(0)->GetGrid()->GetAxis(4)->SetRangeUser(ptAssociatedMin,ptAssociatedMax-0.00001);
    fHistPP->GetGrid(0)->GetGrid()->GetAxis(4)->SetRangeUser(ptAssociatedMin,ptAssociatedMax-0.00001);
    fHistNP->GetGrid(0)->GetGrid()->GetAxis(4)->SetRangeUser(ptAssociatedMin,ptAssociatedMax-0.00001);
    fHistPN->GetGrid(0)->GetGrid()->GetAxis(4)->SetRangeUser(ptAssociatedMin,ptAssociatedMax-0.00001);
    
  //0:step, 1: Delta eta, 2: Delta phi
  TH2D *gHistNN = dynamic_cast<TH2D *>(fHistNN->Project(0,1,2));
  if(!gHistNN){
    AliError("Projection of fHistNN = NULL");
    return gHistNN;
  }
  TH2D *gHistPP = dynamic_cast<TH2D *>(fHistPP->Project(0,1,2));
  if(!gHistPP){
    AliError("Projection of fHistPP = NULL");
    return gHistPP;
  }
  TH2D *gHistNP = dynamic_cast<TH2D *>(fHistNP->Project(0,1,2));
  if(!gHistNP){
    AliError("Projection of fHistNP = NULL");
    return gHistNP;
  }
  TH2D *gHistPN = dynamic_cast<TH2D *>(fHistPN->Project(0,1,2));
  if(!gHistPN){
    AliError("Projection of fHistPN = NULL");
    return gHistPN;
  }

  // sum all 2 particle histograms
  gHistNN->Add(gHistPP);
  gHistNN->Add(gHistNP);
  gHistNN->Add(gHistPN);

  // divide by sum of + and - triggers
  if((Double_t)(fHistN->Project(0,1)->Integral())>0 && (Double_t)(fHistP->Project(0,1)->Integral())>0)
    gHistNN->Scale(1./(Double_t)(fHistN->Project(0,1)->Integral() + fHistN->Project(0,1)->Integral()));

  //normalize to bin width
  gHistNN->Scale(1./((Double_t)gHistNN->GetXaxis()->GetBinWidth(1)*(Double_t)gHistNN->GetYaxis()->GetBinWidth(1)));
  
  return gHistNN;
}

//____________________________________________________________________//

Bool_t AliBalancePsi::GetMomentsAnalytical(Int_t fVariable, TH1D* gHist, Bool_t kUseZYAM,
					   Double_t &mean, Double_t &meanError,
					   Double_t &sigma, Double_t &sigmaError,
					   Double_t &skewness, Double_t &skewnessError,
					   Double_t &kurtosis, Double_t &kurtosisError) {
  //
  // helper method to calculate the moments and errors of a TH1D anlytically
  // fVariable = 1 for Delta eta (moments in full range)
  //           = 2 for Delta phi (moments only on near side -pi/2 < dphi < pi/2)
  //
  
  Bool_t success = kFALSE;
  mean          = -1.;
  meanError     = -1.;
  sigma         = -1.;
  sigmaError    = -1.;
  skewness      = -1.;
  skewnessError = -1.;
  kurtosis      = -1.;
  kurtosisError = -1.;

  if(gHist){

    // ----------------------------------------------------------------------
    // basic parameters of histogram

    Int_t fNumberOfBins = gHist->GetNbinsX();
    //    Int_t fBinWidth     = gHist->GetBinWidth(1); // assume equal binning


    // ----------------------------------------------------------------------
    // ZYAM (for partially negative distributions)
    // --> we subtract always the minimum value
    Double_t zeroYield    = 0.;
    Double_t zeroYieldCur = -FLT_MAX;
    if(kUseZYAM){
      for(Int_t iMin = 0; iMin<2; iMin++){
	zeroYieldCur = gHist->GetMinimum(zeroYieldCur);
	zeroYield   += zeroYieldCur;
      }
      zeroYield /= 2.;
      //zeroYield = gHist->GetMinimum();
    }
    // ----------------------------------------------------------------------
    // first calculate the mean

    Double_t fWeightedAverage   = 0.;
    Double_t fNormalization     = 0.;

    for(Int_t i = 1; i <= fNumberOfBins; i++) {

      // for Delta phi: moments only on near side -pi/2 < dphi < pi/2
      if(fVariable == 2 && 
	 (gHist->GetBinCenter(i) < - TMath::Pi()/2 ||
	  gHist->GetBinCenter(i) > TMath::Pi()/2)){
	continue;
      }

      fWeightedAverage   += (gHist->GetBinContent(i)-zeroYield) * gHist->GetBinCenter(i);
      fNormalization     += (gHist->GetBinContent(i)-zeroYield);
    }  
    
    mean = fWeightedAverage / fNormalization;

    // ----------------------------------------------------------------------
    // then calculate the higher moments

    Double_t fMu  = 0.;
    Double_t fMu2 = 0.;
    Double_t fMu3 = 0.;
    Double_t fMu4 = 0.;
    Double_t fMu5 = 0.;
    Double_t fMu6 = 0.;
    Double_t fMu7 = 0.;
    Double_t fMu8 = 0.;

    for(Int_t i = 1; i <= fNumberOfBins; i++) {

      // for Delta phi: moments only on near side -pi/2 < dphi < pi/2
      if(fVariable == 2 && 
	 (gHist->GetBinCenter(i) < - TMath::Pi()/2 ||
	  gHist->GetBinCenter(i) > TMath::Pi()/2)){
	continue;
      }

      fMu  += (gHist->GetBinContent(i)-zeroYield) * (gHist->GetBinCenter(i) - mean);
      fMu2 += (gHist->GetBinContent(i)-zeroYield) * TMath::Power((gHist->GetBinCenter(i) - mean),2);
      fMu3 += (gHist->GetBinContent(i)-zeroYield) * TMath::Power((gHist->GetBinCenter(i) - mean),3);
      fMu4 += (gHist->GetBinContent(i)-zeroYield) * TMath::Power((gHist->GetBinCenter(i) - mean),4);
      fMu5 += (gHist->GetBinContent(i)-zeroYield) * TMath::Power((gHist->GetBinCenter(i) - mean),5);
      fMu6 += (gHist->GetBinContent(i)-zeroYield) * TMath::Power((gHist->GetBinCenter(i) - mean),6);
      fMu7 += (gHist->GetBinContent(i)-zeroYield) * TMath::Power((gHist->GetBinCenter(i) - mean),7);
      fMu8 += (gHist->GetBinContent(i)-zeroYield) * TMath::Power((gHist->GetBinCenter(i) - mean),8);
    }

    // normalize to bin entries!
    fMu  /= fNormalization;    
    fMu2 /= fNormalization;    
    fMu3 /= fNormalization;    
    fMu4 /= fNormalization;    
    fMu5 /= fNormalization;    
    fMu6 /= fNormalization;    
    fMu7 /= fNormalization;    
    fMu8 /= fNormalization;    

    sigma    = TMath::Sqrt(fMu2);
    skewness = fMu3 / TMath::Power(sigma,3);
    kurtosis = fMu4 / TMath::Power(sigma,4) - 3;

    // normalize with sigma^r
    fMu  /= TMath::Power(sigma,1);    
    fMu2 /= TMath::Power(sigma,2);    
    fMu3 /= TMath::Power(sigma,3);    
    fMu4 /= TMath::Power(sigma,4);    
    fMu5 /= TMath::Power(sigma,5);    
    fMu6 /= TMath::Power(sigma,6);    
    fMu7 /= TMath::Power(sigma,7);    
    fMu8 /= TMath::Power(sigma,8);    
  
    // ----------------------------------------------------------------------
    // then calculate the higher moment errors
    // cout<<fNormalization<<" "<<gHist->GetEffectiveEntries()<<" "<<gHist->Integral()<<endl;
    // cout<<gHist->GetMean()<<" "<<gHist->GetRMS()<<" "<<gHist->GetSkewness(1)<<" "<<gHist->GetKurtosis(1)<<endl;
    // cout<<gHist->GetMeanError()<<" "<<gHist->GetRMSError()<<" "<<gHist->GetSkewness(11)<<" "<<gHist->GetKurtosis(11)<<endl;

    Double_t normError = gHist->GetEffectiveEntries(); //gives the "ROOT" meanError

    // // assuming normal distribution (as done in ROOT)
    // meanError     = sigma / TMath::Sqrt(normError); 
    // sigmaError    = TMath::Sqrt(sigma*sigma/(2*normError));
    // skewnessError = TMath::Sqrt(6./(normError));
    // kurtosisError = TMath::Sqrt(24./(normError));

    // use delta theorem paper (Luo - arXiv:1109.0593v1)
    Double_t Lambda11 = TMath::Abs((fMu4-1)*sigma*sigma/(4*normError));
    Double_t Lambda22 = TMath::Abs((9-6*fMu4+fMu3*fMu3*(35+9*fMu4)/4-3*fMu3*fMu5+fMu6)/normError);
    Double_t Lambda33 = TMath::Abs((-fMu4*fMu4+4*fMu4*fMu4*fMu4+16*fMu3*fMu3*(1+fMu4)-8*fMu3*fMu5-4*fMu4*fMu6+fMu8)/normError);
    //Double_t Lambda12 = -(fMu3*(5+3*fMu4)-2*fMu5)*sigma/(4*normError);
    //Double_t Lambda13 = ((-4*fMu3*fMu3+fMu4-2*fMu4*fMu4+fMu6)*sigma)/(2*normError);
    //Double_t Lambda23 = (6*fMu3*fMu3*fMu3-(3+2*fMu4)*fMu5+3*fMu3*(8+fMu4+2*fMu4*fMu4-fMu6)/2+fMu7)/normError;

    // cout<<Lambda11<<" "<<Lambda22<<" "<<Lambda33<<" "<<endl;
    // cout<<Lambda12<<" "<<Lambda13<<" "<<Lambda23<<" "<<endl;

    if (TMath::Sqrt(normError) != 0){
      meanError        = sigma / TMath::Sqrt(normError); 
      sigmaError       = TMath::Sqrt(Lambda11);
      skewnessError    = TMath::Sqrt(Lambda22);
      kurtosisError    = TMath::Sqrt(Lambda33);

      success = kTRUE;    
	  
    }
    else success = kFALSE;
  
  }
  return success;
}


//____________________________________________________________________//
Float_t AliBalancePsi::GetDPhiStar(Float_t phi1, Float_t pt1, Float_t charge1, Float_t phi2, Float_t pt2, Float_t charge2, Float_t radius, Float_t bSign) { 
  //
  // calculates dphistar
  //
  Float_t dphistar = phi1 - phi2 - charge1 * bSign * TMath::ASin(0.075 * radius / pt1) + charge2 * bSign * TMath::ASin(0.075 * radius / pt2);
  
  static const Double_t kPi = TMath::Pi();
  
  // circularity
//   if (dphistar > 2 * kPi)
//     dphistar -= 2 * kPi;
//   if (dphistar < -2 * kPi)
//     dphistar += 2 * kPi;
  
  if (dphistar > kPi)
    dphistar = kPi * 2 - dphistar;
  if (dphistar < -kPi)
    dphistar = -kPi * 2 - dphistar;
  if (dphistar > kPi) // might look funny but is needed
    dphistar = kPi * 2 - dphistar;
  
  return dphistar;
}

//____________________________________________________________________//
Double_t* AliBalancePsi::GetBinning(const char* configuration, const char* tag, Int_t& nBins)
{
  // This method is a copy from AliUEHist::GetBinning
  // takes the binning from <configuration> identified by <tag>
  // configuration syntax example:
  // eta: 2.4, -2.3, -2.2, -2.1, -2.0, -1.9, -1.8, -1.7, -1.6, -1.5, -1.4, -1.3, -1.2, -1.1, -1.0, -0.9, -0.8, -0.7, -0.6, -0.5, -0.4, -0.3, -0.2, -0.1, 0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0, 1.1, 1.2, 1.3, 1.4, 1.5, 1.6, 1.7, 1.8, 1.9, 2.0, 2.1, 2.2, 2.3, 2.4
  // phi: .....
  //
  // returns bin edges which have to be deleted by the caller
  
  TString config(configuration);
  TObjArray* lines = config.Tokenize("\n");
  for (Int_t i=0; i<lines->GetEntriesFast(); i++)
  {
    TString line(lines->At(i)->GetName());
    if (line.BeginsWith(TString(tag) + ":"))
    {
      line.Remove(0, strlen(tag) + 1);
      line.ReplaceAll(" ", "");
      TObjArray* binning = line.Tokenize(",");
      Double_t* bins = new Double_t[binning->GetEntriesFast()];
      for (Int_t j=0; j<binning->GetEntriesFast(); j++)
	bins[j] = TString(binning->At(j)->GetName()).Atof();
      
      nBins = binning->GetEntriesFast() - 1;

      delete binning;
      delete lines;
      return bins;
    }
  }
  
  delete lines;
  AliFatal(Form("Tag %s not found in %s", tag, configuration));
  return 0;
}
