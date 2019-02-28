#include "THnSparse.h"
#include "TH1.h"
#include "AlidNdPtTools.h"

class AlidNdPtTools;

using namespace std;

THnSparseD* AlidNdPtTools::fSparseTmp = 0;

//____________________________________________________________________________

Long64_t AlidNdPtTools::FillHist(THnSparseD* s, Double_t x1, Double_t x2, Double_t x3, Double_t x4, Double_t x5, Double_t x6, 
                     Double_t x7 , Double_t x8 , Double_t x9 , Double_t x10 , Double_t x11 , Double_t x12 )
{
    if (s->GetNdimensions() > 12) { return 0; }
    Double_t vals[12]={x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12};
    return s->Fill(vals);
}

//____________________________________________________________________________

Int_t AlidNdPtTools::AddAxis(const char* label, const char* title, Int_t nbins, Double_t xmin, Double_t xmax, const char* option)
{
    Int_t n=1;
    if (fSparseTmp) {
        n += fSparseTmp->GetNdimensions();
    }
    TString s;
    TArrayI bin(n);
    TArrayD min(n);
    TArrayD max(n);
    for (int i=0; i<n-1; i++) {
        bin[i] = fSparseTmp->GetAxis(i)->GetNbins();
        min[i] = fSparseTmp->GetAxis(i)->GetXmin();
        max[i] = fSparseTmp->GetAxis(i)->GetXmax();
        s += fSparseTmp->GetAxis(i)->GetName();
        s += ":";
    }
    bin[n-1] = nbins; 
    min[n-1] = xmin;
    max[n-1] = xmax;
    s += label;
    THnSparseD* h = new THnSparseD("fSparseTmp",s.Data(),n,bin.GetArray(),min.GetArray(),max.GetArray());
    for (int i=0; i<n-1; i++) {
        if (fSparseTmp->GetAxis(i)->GetXbins() && fSparseTmp->GetAxis(i)->GetXbins()->GetSize()) { h->SetBinEdges(i,fSparseTmp->GetAxis(i)->GetXbins()->GetArray()); }
        h->GetAxis(i)->SetTitle(fSparseTmp->GetAxis(i)->GetTitle());
        h->GetAxis(i)->SetName(fSparseTmp->GetAxis(i)->GetName());
    }
    h->GetAxis(n-1)->SetTitle(title);
    h->GetAxis(n-1)->SetName(label);
    if (fSparseTmp) { delete fSparseTmp; }
    fSparseTmp = h;
    return fSparseTmp->GetNdimensions();
}

//____________________________________________________________________________

Int_t AlidNdPtTools::AddAxis(const char* label, Int_t nbins, Double_t xmin, Double_t xmax, const char* option)
{
    return AddAxis(label, label, nbins, xmin, xmax, option);
}

//____________________________________________________________________________

Int_t AlidNdPtTools::AddAxis(const char* label, const char* title, Int_t nbins, Double_t* xbins, const char* option)
{
    Int_t n = AddAxis(label, title, nbins, xbins[0], xbins[nbins], option);
    fSparseTmp->SetBinEdges(n-1,xbins);         
    return n;
}

//____________________________________________________________________________

Int_t AlidNdPtTools::AddAxis(const char* label, Int_t nbins, Double_t* xbins, const char* option)
{
    return AddAxis(label, label, nbins, xbins, option);
}

//____________________________________________________________________________

Int_t AlidNdPtTools::AddAxis(const char* label, const char* title, const char* option)
{
    TString o(option);
    o.ToLower();
    if (o.Contains("ptfew")) {
        const Int_t nbins = 21;
        Double_t xbins[22] = {0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0, 1.1, 1.2, 1.3, 1.4, 1.5, 2.0,  5.0, 10.0,  20.0, 50.0,  200.0};
        return AddAxis(label, title, nbins, xbins);    
    }
    if (o.Contains("ptveryfew")) {
        const Int_t nbins = 5;
        Double_t xbins[6] = {0.0, 0.5, 1.0, 1.5, 2.0, 200.0};
        return AddAxis(label, title, nbins, xbins);    
    }    
    if (o.Contains("pt")) {
        const Int_t nbins = 81;
        Double_t xbins[82] = {0.0, 0.05, 0.1, 0.15, 0.2, 0.25, 0.3, 0.35, 0.4, 0.45, 0.5, 0.55, 0.6, 0.65, 0.7, 0.75, 0.8, 0.85, 0.9, 0.95, 1.0, 1.1, 1.2, 1.3, 1.4, 1.5, 1.6, 1.7, 1.8, 1.9, 2.0, 2.2, 2.4, 2.6, 2.8, 3.0, 3.2, 3.4, 3.6, 3.8, 4.0, 4.5, 5.0, 5.5, 6.0, 6.5, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0, 18.0, 20.0, 22.0, 24.0, 26.0, 28.0, 30.0, 32.0, 34.0, 36.0, 40.0, 45.0, 50.0, 60.0, 70.0, 80.0, 90.0, 100.0, 110.0, 120.0, 130.0, 140.0, 150.0, 160.0, 180.0, 200.0};
        return AddAxis(label, title, nbins, xbins);    
    }   
    if (o.Contains("cent")) {
        const Int_t nbins = 11;
        Double_t xbins[12] = {0.,5.,10.,20.,30.,40.,50.,60.,70.,80.,90.,100.};
        return AddAxis(label, title, nbins, xbins);
    }    
    if (o.Contains("varsig35")) {
        const Int_t nbins = 35;
        Double_t xbins[36] = {-1,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,30,40,50,60,70,80,90,100,200,300,400,500,1000,2000};
        return AddAxis(label, title, nbins, xbins);
    }    
    return 0;
}

//____________________________________________________________________________

Int_t AlidNdPtTools::AddAxis(const char* label, const char* option)
{
    return AddAxis(label, label, option);
}

//____________________________________________________________________________

Int_t AlidNdPtTools::AddAxis(const char* option)
{
    TString o(option);
    o.ToLower();
    if (o.Contains("pt"))   return AddAxis("pt","p_{T} (GeV/c)",option);    
    if (o.Contains("cent"))   return AddAxis("cent","centrality",option);
    return 0;
}

//____________________________________________________________________________

THnSparseD* AlidNdPtTools::CreateHist(const char* name)
{
    if (!fSparseTmp) return 0;
    THnSparseD* h = fSparseTmp;
    h->SetName(name);
    fSparseTmp = 0;    
    return h;
}

//____________________________________________________________________________

TH1D* AlidNdPtTools::CreateLogHist(const char* name, const char* title)
{   
   TH1D *h = 0;
   if (title) { 
       h = new TH1D(name,title,200,0,200);        
    } else {
       h = new TH1D(name,name,200,0,200);
    }                   
   return h;
}   

//____________________________________________________________________________
  
TH1D* AlidNdPtTools::CreateLogHist(const char* name) 
{ 
    return CreateLogHist(name,name);
}

//____________________________________________________________________________
