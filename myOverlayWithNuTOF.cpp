#include "TG4Event.h"
#include "gRooTracker.h"

const double nuSpeed = 299.792458; // mm/ns

// returns a random time for a neutrino interaction to take place within
// a LBNF/NuMI spill, given the beam's micro timing structure
double getProductionTimeLBNF()
{

  // time unit is nanoseconds
  double t;
  bool finding_time = true;

  while (finding_time)
  {
    unsigned int batch = gRandom->Integer(6);  // batch number between 0 and 5 (6 total)
    unsigned int bunch = gRandom->Integer(84); // bunch number between 0 and 83 (84 total)
    if ((bunch == 0 || bunch == 1 || bunch == 82 || bunch == 83) && gRandom->Uniform(1.) < 0.5)
      continue;
    else
    {
      t = gRandom->Uniform(1.) + bunch * 19. + batch * 1680;
      finding_time = false;
    }
  }

  return t;
}

double getInteractionTimeLBNF(double intZ)
{
  return getProductionTimeLBNF() + intZ / nuSpeed;
}

struct eventTime
{
  int evId;
  double intTime;
  eventTime(int evId, double intTime) : evId(evId), intTime(intTime) {}
  eventTime() : evId(0), intTime(0) {}
  bool operator<(eventTime const &other) const
  {
    return intTime < other.intTime;
  }
};

void myOverlayWithNuTOF(std::string outFileName,
                        std::string nuIntFileName,
                        int runId = 0,
                        double spillPOT = 7.5E13)
{

  // the script takes the events in the input files
  // and overlays them to create a beam spill
  // The arguments are:
  //  - outFile      : output file
  //  - nuIntFile    : edep-sim simulated SAND events
  //  - spillPOT     : number of POT per spill               [OPTIONAL]

  // constraints:
  // - in Trajectories first are all the primary
  //   particles of all the interactions and the secondary ones
  // - what about event id?
  // - what about the mapping of the events between GENIE and EDEPSIM

  // create TG4Event pointer
  TG4Event *event = 0;
  TG4Event *spill = 0;

  // open neutrino event file
  std::unique_ptr<TFile> nuIntFile(TFile::Open(nuIntFileName.data()));
  if (!nuIntFile || nuIntFile->IsZombie())
  {
    std::cerr << "[ERROR]: Error opening file: " << nuIntFileName.data() << endl;
    exit(-1);
  }

  // get neutrino edepsim event tree
  std::unique_ptr<TTree> nuIntEdepTree(nuIntFile->Get<TTree>("EDepSimEvents"));
  if (!nuIntEdepTree)
  {
    std::cerr << "[ERROR]: Error opening neutrino interaction tree: EDepSimEvents" << endl;
    exit(-1);
  }
  nuIntEdepTree->SetBranchAddress("Event", &event);

  // get neutrino gRooTracker event tree
  std::unique_ptr<TTree> nuIntGRtrkTree(nuIntFile->Get<TTree>("DetSimPassThru/gRooTracker"));
  if (!nuIntGRtrkTree)
  {
    std::cerr << "[ERROR]: Error opening neutrino interaction tree: gRooTracker" << endl;
    exit(-1);
  }
  gRooTracker nuIntGRtrk(nuIntGRtrkTree.get());

  // creating output file
  std::unique_ptr<TFile> outFile(TFile::Open(outFileName.data(), "RECREATE"));
  auto outEdepTree = nuIntEdepTree->CloneTree(0);
  auto outGRtrkTree = nuIntGRtrkTree->CloneTree(0);
  outEdepTree->SetBranchAddress("Event", &spill);

  gRooTracker outGRtrk(outGRtrkTree);

  auto nuIntFilePOT = nuIntGRtrkTree->GetWeight();
  auto nuIntFileEvt = nuIntGRtrkTree->GetEntries();
  auto evtsPerSpill = ((double)nuIntFileEvt) / (nuIntFilePOT / spillPOT);

  std::cout << "File: " << nuIntFileName.data() << std::endl;
  std::cout << "    Number of POTs  : " << nuIntFilePOT << std::endl;
  std::cout << "    Number of spills: " << std::floor((double)nuIntFileEvt / evtsPerSpill) << std::endl;
  std::cout << "    Events per spill: " << evtsPerSpill << std::endl;

  int evtIt = 0;
  int nEvtsThisSpill = 0;

  for (int spillId = 0;; ++spillId)
  {
    nEvtsThisSpill = gRandom->Poisson(evtsPerSpill);

    if (evtIt + nEvtsThisSpill > nuIntFileEvt)
      break;

    std::cout << "working on spill # " << spillId << std::endl;

    spill = new TG4Event();
    spill->RunId = runId;
    spill->EventId = spillId;

    // std::vector<double> times(nEvtsThisSpill);
    // std::generate(times.begin(),
    //               times.begin() + nEvtsThisSpill,
    //               []()
    //               { return getProductionTimeLBNF(); });
    // std::sort(times.begin(),
    //           times.end());

    std::vector<eventTime> times;

    int nPrimaryPart = 0;
    int nTrajectories = 0;
    for (int i = 0; i < nEvtsThisSpill; i++)
    {
      nuIntEdepTree->GetEntry(evtIt + i);
      // here (and afterwards) we assume there is just one primary
      assert(event->Primaries.size() != 0u && "Multiple interaction vertices in the same event not supported!!");

      auto v = event->Primaries[0];
      nPrimaryPart += v.Particles.size();
      times.push_back(eventTime(evtIt + i, getInteractionTimeLBNF(v.Position.Z())));
      nTrajectories += event->Trajectories.size();
    }

    std::sort(times.begin(), times.end());

    // std::cout << "This spill: " << std::endl;
    // std::cout << "  - Primary particle: " << nPrimaryPart << std::endl;
    // std::cout << "  - Trajectories    : " << nTrajectories << std::endl;

    int lastPriTrajId = 0;
    int lastSecTrajId = nPrimaryPart;
    spill->Trajectories.resize(nTrajectories);

    std::map<std::string, std::vector<TG4HitSegment>> SegmentDetectors;

    for (const auto &evTime : times)
    {
      nuIntEdepTree->GetEntry(evTime.evId);
      nuIntGRtrkTree->GetEntry(evTime.evId);

      outGRtrk.CopyFrom(nuIntGRtrk);
      outGRtrk.EvtVtx[3] = evTime.intTime;

      int nPrimaryPartThisEvent = 0;
      for (auto &v : event->Primaries)
        nPrimaryPartThisEvent += v.Particles.size();
      int nTracjectoriesThisEvent = event->Trajectories.size();
      int nSecondaryPartThisEvent = nTracjectoriesThisEvent - nPrimaryPartThisEvent;

      auto updateTrackId = [lastPriTrajId, lastSecTrajId, nPrimaryPartThisEvent](int &trkId)
      { trkId = trkId < nPrimaryPartThisEvent ? lastPriTrajId + trkId : lastSecTrajId + trkId - nPrimaryPartThisEvent; };

      // ... interaction vertex
      auto& v = event->Primaries[0];
      auto tOffset = evTime.intTime - v.Position.T();
      v.Position.SetT(evTime.intTime);
      v.InteractionNumber = evtIt;
      for (auto &p : v.Particles)
        updateTrackId(p.TrackId);
      spill->Primaries.push_back(v);

      // std::cout << "This event: " << evtIt << " of " << nEvtsThisSpill << std::endl;
      // std::cout << "  - Primary particle  : " << nPrimaryPartThisEvent << std::endl;
      // std::cout << "  - Secondary particle: " << nSecondaryPartThisEvent << std::endl;
      // std::cout << "  - Trajectories      : " << nTracjectoriesThisEvent << std::endl;
      // std::cout << "  - Last Pri. Part. Id: " << lastPriTrajId << std::endl;
      // std::cout << "  - Last Sec. Part. Id: " << lastSecTrajId << std::endl;

      // ... trajectories
      for (auto &t : event->Trajectories)
      {
        // loop over all points in the trajectory
        for (auto &p : t.Points)
        {
          p.Position.SetT(tOffset + p.Position.T());
        }
        updateTrackId(t.TrackId);
        if (t.ParentId != -1)
          updateTrackId(t.ParentId);
        spill->Trajectories[t.TrackId] = t;
      }

      // ... and, finally, energy depositions
      for (auto &d : event->SegmentDetectors)
      {
        for (auto &h : d.second)
        {
          h.Start.SetT(tOffset + h.Start.T());
          h.Stop.SetT(tOffset + h.Stop.T());
          updateTrackId(h.PrimaryId);
          for (auto &trkId : h.Contrib)
            updateTrackId(trkId);
          SegmentDetectors[d.first].push_back(h);
        }
      }
      lastPriTrajId += nPrimaryPartThisEvent;
      lastSecTrajId += nSecondaryPartThisEvent;
      outGRtrkTree->Fill();
      evtIt++;
    }
    spill->SegmentDetectors = std::vector<std::pair<std::string, std::vector<TG4HitSegment>>>(SegmentDetectors.begin(), SegmentDetectors.end());
    outEdepTree->Fill();
    delete spill;
  }
  outGRtrk.fChain = 0;
  nuIntGRtrk.fChain = 0;

  outEdepTree->Write();
  outGRtrkTree->Write();
}
