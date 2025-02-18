#include <sstream>

void build_spill() {
    gInterpreter->AddIncludePath("/storage/gpfs_data/neutrino/users/mt/ANA/spill_slicing/scratch/2x2_sim/run-spill-build/libTG4Event");
    gInterpreter->AddIncludePath("/storage/gpfs_data/neutrino/users/mt/ANA/spill_slicing/scratch/2x2_sim/run-spill-build");
    gSystem->AddDynamicPath("/storage/gpfs_data/neutrino/users/mt/ANA/spill_slicing/scratch/2x2_sim/run-spill-build/libTG4Event");
    gSystem->Load("libTG4Event.so");
    gInterpreter->ProcessLine(".L /storage/gpfs_data/neutrino/users/mt/ANA/spill_slicing/myOverlayWithNuTOF.cpp");
    gInterpreter->ProcessLine(".x /opt/exp_software/neutrino/GENIEv2/Generator/src/scripts/gcint/genie_setup.C");
    for(int i = 0; i < 100; i++) {
        std::ostringstream cmd;
        cmd << "myOverlayWithNuTOF(\"files/spill/sand-spill-events." << i << ".overlay.edep.root\",\"/storage/gpfs_data/neutrino/users/mt/ANA/spill_slicing/productions/SAND/SAND_" << i << "/sand-events." << i << ".edep.root\");";
        gInterpreter->ProcessLine(cmd.str().data());
    }
    return;
}