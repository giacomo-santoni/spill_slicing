#!/bin/bash

# PYTHIA6
export LD_LIBRARY_PATH=/opt/exp_software/neutrino/al9/PYTHIA8/8312/lib:${LD_LIBRARY_PATH}

# ROOT
source /opt/exp_software/neutrino/al9/ROOT/v6.32.06/v6.32.06_install/bin/thisroot.sh

# GEANT4
source /opt/exp_software/neutrino/al9/GEANT4/v10.7.4/v10.7.4_install/bin/geant4.sh

# EDEP-SIM
source /opt/exp_software/neutrino/al9/EDEPSIM/edep-sim/setup.sh

# In this PATH there are the libraries transferred during job submission
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:/home/condor/execute/dir_285013

# GENIE V3
source /opt/exp_software/neutrino/al9/GENIE/R-3_06_00/setup.sh

# Create ROOT geometry file from GDML
echo "TGeoManager::SetVerboseLevel(0); TGeoManager::Import(\"/storage/gpfs_data/neutrino/users/alrugger/Software/dunendggd/EC_yoke_corrected_1212_dev_SAND_complete_opt3_DRIFT1.gdml\"); TFile f(\"/storage/gpfs_data/neutrino/users/gsantoni/productions/SAND_2/SAND_2_0/EC_yoke_corrected_1212_dev_SAND_complete_opt3_DRIFT1.root\",\"RECREATE\"); gGeoManager->Write(\"geo\"); f.Close();" | root -l

# Evaluate max path lengths from ROOT geometry file
gmxpl -f /storage/gpfs_data/neutrino/users/gsantoni/productions/SAND_2/SAND_2_0/EC_yoke_corrected_1212_dev_SAND_complete_opt3_DRIFT1.root -L cm -D g_cm3 -t volWorld -o /storage/gpfs_data/neutrino/users/gsantoni/productions/SAND_2/SAND_2_0/EC_yoke_corrected_1212_dev_SAND_complete_opt3_DRIFT1.xml -seed 30259 --message-thresholds /opt/exp_software/neutrino/al9/GENIE/R-3_04_02/source/config/Messenger_whisper.xml  &> /storage/gpfs_data/neutrino/users/gsantoni/productions/SAND_2/SAND_2_0/gmxpl.log

GHEP_FILE_PREFIX_volSAND="/storage/gpfs_data/neutrino/users/gsantoni/productions/SAND_2/SAND_2_0/sand-events-in-volSAND"
GHEP_FILE_volSAND="${GHEP_FILE_PREFIX_volSAND}.0.ghep.root"
# GENERATING EVENTS IN volSAND
gevgen_fnal -r 0 \
            -f /storage/gpfs_data/neutrino/SAND/LBNF-GSIMPLE-FILES/gsimple_subdetectors/neutrino/gsimple_DUNE_ND_NEAR_FID1_g4lbne_v3r5p4_QGSP_BERT_OptimizedEngineeredNov2017_neutrino_00245_00243.root,DUNE_ND_NEAR_FID1,12,-12,14,-14 \
            -g /storage/gpfs_data/neutrino/users/gsantoni/productions/SAND_2/SAND_2_0/EC_yoke_corrected_1212_dev_SAND_complete_opt3_DRIFT1.root \
            -t volSAND \
            -m /storage/gpfs_data/neutrino/users/gsantoni/productions/SAND_2/SAND_2_0/EC_yoke_corrected_1212_dev_SAND_complete_opt3_DRIFT1.xml \
            -L cm \
            -D g_cm3 \
            -o ${GHEP_FILE_PREFIX_volSAND} \
            -seed 30259 \
            --cross-sections /storage/gpfs_data/neutrino/SAND/PRODUCTIONS/XSEC/GENIE_v3_06_00/SAND_opt2.xsec.xml \
            --message-thresholds /opt/exp_software/neutrino/al9/GENIE/R-3_04_02/source/config/Messenger_whisper.xml \
            -e 1.E15 &> /storage/gpfs_data/neutrino/users/gsantoni/productions/SAND_2/SAND_2_0/gevgen_fnal_volSAND.log

# EVALUATING MEAN NUMBER OF EVENTS IN volSAND
MEAN_volSAND=$(echo "std::cout << gtree->GetEntries()*(7.5E13/gtree->GetWeight()) << std::endl;" | genie -l -b ${GHEP_FILE_volSAND} 2>/dev/null  | tail -1)

# EVALUATING NUMBER OF EVENTS
NEV=$(echo "std::cout << (int)gtree->GetEntries() << std::endl;" | genie -l -b ${GHEP_FILE_volSAND} 2>/dev/null  | tail -1)

# GENIE V3
source /opt/exp_software/neutrino/al9/GENIE/R-3_06_00/setup.sh

# CONVERT IN NUMI_ROOTRACKER FORMAT
gntpc -i ${GHEP_FILE_volSAND} -o /storage/gpfs_data/neutrino/users/gsantoni/productions/SAND_2/SAND_2_0/sand-events.0.gtrac.root -f numi_rootracker &> /storage/gpfs_data/neutrino/users/gsantoni/productions/SAND_2/SAND_2_0/gntpc.log

# Preparing EDEP-SIM macro
sed "s:__INPUT__:/storage/gpfs_data/neutrino/users/gsantoni/productions/SAND_2/SAND_2_0/sand-events.0.gtrac.root:g; s:__NEV__:${NEV}:g; s:/generator/count:#/generator/count:g; s:__RUN__:0:g" macro.template.mac > /storage/gpfs_data/neutrino/users/gsantoni/productions/SAND_2/SAND_2_0/macro.mac

# PROPAGATE PARTICLES WITH EDEP-SIM
edep-sim -C \
  -g /storage/gpfs_data/neutrino/users/alrugger/Software/dunendggd/EC_yoke_corrected_1212_dev_SAND_complete_opt3_DRIFT1.gdml \
  -o /storage/gpfs_data/neutrino/users/gsantoni/productions/SAND_2/SAND_2_0/sand-events.0.edep.root \
  /storage/gpfs_data/neutrino/users/gsantoni/productions/SAND_2/SAND_2_0/macro.mac \
  &> /storage/gpfs_data/neutrino/users/gsantoni/productions/SAND_2/SAND_2_0/edep-sim.log

