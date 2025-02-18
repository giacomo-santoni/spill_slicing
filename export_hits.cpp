// run:
//   source /opt/exp_software/neutrino/env.sh
// and then:
//   root -l -b export_hits.cpp\(\"productions/SAND_SPILL/SAND_SPILL_0/sand-spill-events.0.edep.root\",\"ofile.test.root\"\)

// STDLIB
#include <iostream>
#include <sstream>
// ROOT
#include <TFile.h>
#include <TTree.h>
// EDEPSIM
#include "/opt/exp_software/neutrino/EDEPSIM/include/EDepSim/TG4Event.h"

int get_trackid_of_primary_parent(TG4Event& e, int trkid) {
    // assumption [checked]: trackId and index in the vector are the same
    auto trk = e.Trajectories[trkid];
    while (trk.GetParentId() != -1) {
        trkid = trk.GetParentId();
        trk = e.Trajectories[trkid];
    }
    return trkid;
}

int get_eventid_from_primary_particle(TG4Event& e, int trkid) {
    // assumption [checked]: trackId are sorted among Primaries
    int eid = 0;
    while (e.Primaries[eid].Particles.back().GetTrackId() < trkid) eid++;
    return eid;
}

int get_eventid_of_hit(TG4Event& e, TG4HitSegment& h) {
    return get_eventid_from_primary_particle(e, get_trackid_of_primary_parent(e, h.GetPrimaryId()));
}

void export_hits(const char* ifname, const char* ofname, int& first_spill_id) {


    // Read input file
    TFile ifile(ifname);

    // read input tree
    TTree* itree = (TTree*) ifile.Get("EDepSimEvents");
    TG4Event* event = new TG4Event();
    itree->SetBranchAddress("Event",&event);


    // Read output file
    TFile ofile(ofname,"RECREATE");

    // create output tree
    TTree otree("thits","Tree of hits");

    // setup output tree
    int spl_id;
    int evt_id;
    double evt_x, evt_y, evt_z, evt_t;                        // event position and time
    std::vector<double> hit_x, hit_y, hit_z, hit_t, hit_de;   // hits position, time and energy
    std::vector<int> hit_pdg;                                 // pdg of the particle creating the hit
    std::vector<std::string> hit_det;                         // detector type
    otree.Branch("spl_id",&spl_id);
    otree.Branch("evt_id",&evt_id);
    otree.Branch("evt_x",&evt_x);
    otree.Branch("evt_y",&evt_y);
    otree.Branch("evt_z",&evt_z);
    otree.Branch("evt_t",&evt_t);
    otree.Branch("hit_x",&hit_x);
    otree.Branch("hit_y",&hit_y);
    otree.Branch("hit_z",&hit_z);
    otree.Branch("hit_t",&hit_t);
    otree.Branch("hit_de",&hit_de);
    otree.Branch("hit_pdg",&hit_pdg);
    otree.Branch("hit_det",&hit_det);

    // detectors
    std::vector<std::string> detectors {"EMCalSci", "DriftVolume"};

    // loop over spills
    auto n_spill = itree->GetEntries();
    for(Long64_t spill_id = 0; spill_id < n_spill; spill_id++) {
        itree->GetEntry(spill_id);

        // group hits by event
        std::vector<std::map<std::string,std::vector<int>>> hits_by_event(event->Primaries.size());

        // loop over detectors
        for(auto& det: detectors) {
            // loop over hits
            // and get their event id
            for(unsigned int ihit = 0; ihit < event->SegmentDetectors[det].size(); ihit++) {
                hits_by_event[get_eventid_of_hit(*event, event->SegmentDetectors[det][ihit])][det].push_back(ihit);
            }
        }
        
        spl_id = first_spill_id + spill_id;
        // loop over events
        for(unsigned int iev = 0; iev < event->Primaries.size(); iev++) {
            // reset vector
            hit_x.clear();
            hit_y.clear();
            hit_z.clear();
            hit_t.clear();
            hit_de.clear();
            hit_pdg.clear();
            hit_det.clear();

            // event info
            evt_id = iev;
            evt_x = event->Primaries[iev].Position.X();
            evt_y = event->Primaries[iev].Position.Y();
            evt_z = event->Primaries[iev].Position.Z();
            evt_t = event->Primaries[iev].Position.T();
            
            // loop over detectors
            for(auto& det: detectors) {
                // loop over hits
                for(unsigned int ihit = 0; ihit < hits_by_event[iev][det].size(); ihit++) {
                    auto& hit = event->SegmentDetectors[det][hits_by_event[iev][det][ihit]];
                    // hit info
                    hit_x.push_back(0.5 * (hit.GetStart().X() + hit.GetStop().X()));
                    hit_y.push_back(0.5 * (hit.GetStart().Y() + hit.GetStop().Y()));
                    hit_z.push_back(0.5 * (hit.GetStart().Z() + hit.GetStop().Z()));
                    hit_t.push_back(0.5 * (hit.GetStart().T() + hit.GetStop().T()));
                    hit_de.push_back(hit.GetEnergyDeposit());
                    hit_pdg.push_back(event->Trajectories[hit.GetPrimaryId()].GetPDGCode());
                    hit_det.push_back(det);
                }
            }
            otree.Fill();
        }
    }
    first_spill_id += n_spill;

    // write tree and close ooutput file
    otree.Write();
    ofile.Close();

    // close input file
    ifile.Close();

}

void export_hits() {
    int files = 100;
    int first_spill_id = 0;
    for(int i = 0; i < files; i++)  {
        std::cout << "processing ..." << i << std::endl;
        std::ostringstream ifile;
        std::ostringstream ofile;
        // ifile << "productions/SAND_SPILL/SAND_SPILL_" << i << "/sand-spill-events." << i << ".edep.root";
        ifile << "files/spill/sand-spill-events." << i << ".overlay.edep.root";
        ofile << "files/hits/sand-spill-events." << i << ".hits.root";
        
        export_hits(ifile.str().data(),ofile.str().data(), first_spill_id);
    }
} 