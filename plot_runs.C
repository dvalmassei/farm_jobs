
#include <iostream>
#include <string>
#include <regex>
#include <algorithm>

#include "TROOT.h"
#include "TSystem.h"
#include "TSystemDirectory.h"
#include "TSystemFile.h"
#include "TList.h"
#include "TFile.h"
#include "TTree.h"
#include "TCanvas.h"
#include "TH1D.h"
#include "TStyle.h"
#include "TAxis.h"
#include "TString.h"

void plot_runs(const char* directory = ".")
{
    // 1. Find all Int_Run_<runNum>.root files
    TSystemDirectory dir("inputDir", directory);
    TList* files = dir.GetListOfFiles();

    if (!files) {
        std::cerr << "Error reading directory: "
                  << directory << std::endl;
        return;
    }

    std::regex pattern(R"(^Int_Run_([0-9]+)\.root$)");

    TIter next(files);
    TSystemFile* file;

    // 2. Set plotting style
    gStyle->SetOptStat(1111);
    gStyle->SetStatX(0.90);
    gStyle->SetStatY(0.90);
    gStyle->SetStatW(0.20);
    gStyle->SetStatH(0.20);

    TString outputDir = TString(directory) + "/plots";
    gSystem->mkdir(outputDir, true);

    // 3. Process each run separately
    while ((file = (TSystemFile*)next())) {
        std::string filename = file->GetName();

        std::smatch match;
        if (file->IsDirectory() ||
            !std::regex_match(filename, match, pattern)) {
            continue;
        }

        std::string runNum = match[1].str();
        TString inputPath = TString(directory) + "/" +
                            filename.c_str();

        std::cout << "Processing run " << runNum
                  << ": " << inputPath << std::endl;

        TFile* inputFile = TFile::Open(inputPath, "READ");

        if (!inputFile || inputFile->IsZombie()) {
            std::cerr << "Cannot open " << inputPath
                      << std::endl;
            if (inputFile) delete inputFile;
            continue;
        }

        TTree* tree = nullptr;
        inputFile->GetObject("T", tree);

        if (!tree) {
            std::cerr << "Tree T not found in "
                      << inputPath << std::endl;
            inputFile->Close();
            delete inputFile;
            continue;
        }

        // 4. Plot data_0 and data_1
        const char* branches[] = {"data_0", "data_1"};

        for (int ch = 0; ch < 2; ++ch) {
            const char* branch = branches[ch];

            if (!tree->GetBranch(branch)) {
                std::cerr << "Branch " << branch
                          << " not found in run "
                          << runNum << std::endl;
                continue;
            }

            TString histName = TString::Format(
                "h_run%s_ch%d", runNum.c_str(), ch
            );

            TString drawCmd = TString::Format(
                "%s>>%s(200)",
                branch, histName.Data()
            );

            tree->Draw(drawCmd, "", "goff");

            TH1D* h = (TH1D*)gDirectory->Get(
                histName.Data()
            );

            if (!h || h->GetEntries() == 0) {
                std::cerr << "No entries for " << branch
                          << " in run " << runNum << std::endl;
                continue;
            }

            h->SetDirectory(nullptr);

            h->SetTitle(TString::Format(
                "Run %s, %s;Value;Counts",
                runNum.c_str(), branch
            ));

            // 5. X-range: mean +/- 2 sigma
            double mean = h->GetMean();
            double sigma = h->GetStdDev();

            if (sigma > 0) {
                h->GetXaxis()->SetRangeUser(
                    mean - 2.0 * sigma,
                    mean + 2.0 * sigma
                );
            }

            // 6. Draw with logarithmic y-axis
            TString canvasName = TString::Format(
                "c_run%s_ch%d", runNum.c_str(), ch
            );

            TCanvas* c = new TCanvas(
                canvasName, canvasName, 1000, 700
            );

            c->SetLogy();
            h->SetMinimum(0.5);
            h->SetLineWidth(2);
            h->Draw("HIST");

            c->Modified();
            c->Update();

            // 7. Save as <runNum>_data_<chNum>.png
            TString outputFile = TString::Format(
                "%s/%s_data_%d.png",
                outputDir.Data(), runNum.c_str(), ch
            );

            c->SaveAs(outputFile);

            std::cout << "Saved " << outputFile
                      << " | mean = " << mean
                      << ", sigma = " << sigma
                      << ", x range = ["
                      << mean - 2.0 * sigma << ", "
                      << mean + 2.0 * sigma << "]"
                      << std::endl;

            delete c;
            delete h;
        }

        inputFile->Close();
        delete inputFile;
    }

    delete files;
}