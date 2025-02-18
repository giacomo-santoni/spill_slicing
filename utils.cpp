#include <TG4Event.h>

int get_spill_id() {
    static int this_spill_id = 0;
    return this_spill_id++;
}

template<typename T, typename V>
std::vector<V> process_vector(std::vector<T>& v, V (*process_item)(T&)) {
    std::vector<V> r;
    for(auto& item: v) r.push_back(process_item(item));
    return r;
}

template<typename T, typename V>
std::vector<V> my_event_processing(TG4Event& e, std::vector<T>& (*get_vector)(TG4Event&), V (*process_item)(TG4Event&, T&)) {
    std::vector<V> r;
    for(auto& item: get_vector(e)) r.push_back(process_item(e, item));
    return r;
}

std::vector<TG4PrimaryVertex>& get_event_primaries(TG4Event& e) {
    return e.Primaries;
}

std::vector<TG4HitSegment>& get_hit_segments(TG4Event& e, std::string dect) {
    return e.SegmentDetectors[dect];
}

std::vector<TG4HitSegment>& get_ecal_hit_segments(TG4Event& e) {
    return get_hit_segments(e, "EMCalSci");
}

std::vector<TG4HitSegment>& get_drft_hit_segments(TG4Event& e) {
    return get_hit_segments(e, "DriftVolume");
}

std::vector<TG4HitSegment>& get_LArHit_hit_segments(TG4Event& e) {
    return get_hit_segments(e, "LArHit");
}

double get_primary_x(TG4Event& e, TG4PrimaryVertex& v) {
    return v.GetPosition().X();
}

double get_primary_y(TG4Event& e, TG4PrimaryVertex& v) {
    return v.GetPosition().Y();
}

double get_primary_z(TG4Event& e, TG4PrimaryVertex& v) {
    return v.GetPosition().Z();
}

double get_primary_t(TG4Event& e, TG4PrimaryVertex& v) {
    return v.GetPosition().T();
}

std::vector<double> get_dt_between_interactions(TG4Event& e) {
    std::vector<double> r;
    for(unsigned int i = 0; i < e.Primaries.size() - 1; i++) r.push_back(e.Primaries[i+1].GetPosition().T() - e.Primaries[i].GetPosition().T());
    return r;
}

double get_hit_t(TG4Event& e, TG4HitSegment& h) {
    return 0.5 * (h.Start.T() + h.Stop.T());
}

double get_hit_x(TG4Event& e, TG4HitSegment& h) {
    return 0.5 * (h.Start.X() + h.Stop.X());
}

double get_hit_y(TG4Event& e, TG4HitSegment& h) {
    return 0.5 * (h.Start.Y() + h.Stop.Y());
}

double get_hit_z(TG4Event& e, TG4HitSegment& h) {
    return 0.5 * (h.Start.Z() + h.Stop.Z());
}

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

struct myHitSegment {
    TLorentzVector strt;
    TLorentzVector stop;
    int primary_id;
    int event;
    int spill;
    double de;
    int detector_id; // ecal = 0; tracker = 1
};

myHitSegment fill_myHitSegment(TG4HitSegment& h, TG4Event& e, int spill_id, std::string det) {
    myHitSegment my_h;
    my_h.strt = h.Start;
    my_h.stop = h.Stop;
    my_h.primary_id = h.PrimaryId;
    my_h.event = get_eventid_of_hit(e, h);
    my_h.spill = spill_id;
    my_h.de = h.EnergyDeposit;
    
    if(det == "EMCalSci") my_h.detector_id = 0;
    else if(det == "DriftVolume") my_h.detector_id = 1;   
    return my_h;
}

std::vector<myHitSegment> fill_myHitSegments(TG4Event& e, int spill_id) {
    std::vector<myHitSegment> my_h;
    std::vector<std::string> det = {"EMCalSci", "DriftVolume"};
    for(auto& d: det) {
        for(auto& h: e.SegmentDetectors[d]) {
            my_h.push_back(fill_myHitSegment(h, e, spill_id, d));
        }
    }
    return my_h;
}

std::vector<myHitSegment> aggregate(std::vector<myHitSegment>& orig, std::vector<myHitSegment>& toadd) {
    orig.insert(orig.end(), toadd.begin(), toadd.end());
    return orig;
}

struct aggregator {                                                                                                  
    std::vector<myHitSegment> operator()(std::vector<myHitSegment> orig, std::vector<myHitSegment> toadd) {
        orig.insert(orig.end(), toadd.begin(), toadd.end());
        return orig;
    }                                                                             
};

struct aggregator_int {                                                                                                  
    int operator()(int orig, int toadd) {
        return  orig + toadd;
    }                                                                             
};