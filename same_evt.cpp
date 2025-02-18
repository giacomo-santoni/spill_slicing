void same_evt() {
    TFile f1("files/spill/sand-spill-events.12.overlay.edep.root");
    TFile f2("files/spill/sand-spill-events.13.overlay.edep.root");
    auto t1 = static_cast<TTree*>(f1.Get("EDepSimEvents"));
    auto t2 = static_cast<TTree*>(f2.Get("EDepSimEvents"));
    TG4Event* e1 = new TG4Event();
    TG4Event* e2 = new TG4Event();
    t1->SetBranchAddress("Event",&e1);
    t2->SetBranchAddress("Event",&e2);
    t1->GetEntry(167 - 155);
    t2->GetEntry(181 - 169);
    std::cout << e1->Primaries[45].GetPosition().X() << " " << e2->Primaries[54].GetPosition().X() << std::endl;
    std::cout << e1->Primaries[45].GetPosition().Y() << " " << e2->Primaries[54].GetPosition().Y() << std::endl;
    std::cout << e1->Primaries[45].GetPosition().Z() << " " << e2->Primaries[54].GetPosition().Z() << std::endl;
    std::cout << e1->Primaries[45].GetPosition().T() << " " << e2->Primaries[54].GetPosition().T() << std::endl;

    TFile f3("/storage/gpfs_data/neutrino/users/mt/ANA/spill_slicing/productions/SAND/SAND_12/sand-events.12.edep.root");
    TFile f4("/storage/gpfs_data/neutrino/users/mt/ANA/spill_slicing/productions/SAND/SAND_13/sand-events.13.edep.root");
    auto t3 = static_cast<TTree*>(f3.Get("EDepSimEvents"));
    auto t4 = static_cast<TTree*>(f4.Get("EDepSimEvents"));

    t3->Scan("Entry$:Primaries.Position.X():Primaries.Position.Y():Primaries.Position.Z()","fabs(Primaries.Position.X() - 1800.72) < 0.01");
    t4->Scan("Entry$:Primaries.Position.X():Primaries.Position.Y():Primaries.Position.Z()","fabs(Primaries.Position.X() - 1800.72) < 0.01");

    t3->Scan("Entry$:Primaries.Position.X():Primaries.Position.Y():Primaries.Position.Z()","Entry$ < 2");
    t4->Scan("Entry$:Primaries.Position.X():Primaries.Position.Y():Primaries.Position.Z()","Entry$ < 2");

    
    TFile f5("/storage/gpfs_data/neutrino/users/mt/ANA/spill_slicing/productions/SAND/SAND_12/sand-events.12.gtrac.root");
    TFile f6("/storage/gpfs_data/neutrino/users/mt/ANA/spill_slicing/productions/SAND/SAND_13/sand-events.13.gtrac.root");
    auto t5 = static_cast<TTree*>(f5.Get("gRooTracker"));
    auto t6 = static_cast<TTree*>(f6.Get("gRooTracker"));

    t5->Scan("Entry$:EvtVtx[0]:EvtVtx[1]:EvtVtx[2]:EvtVtx[3]","fabs(EvtVtx[0] - 1.80072) < 0.00001");
    t6->Scan("Entry$:EvtVtx[0]:EvtVtx[1]:EvtVtx[2]:EvtVtx[3]","fabs(EvtVtx[0] - 1.80072) < 0.00001");

    t5->Scan("Entry$:EvtVtx[0]:EvtVtx[1]:EvtVtx[2]:EvtVtx[3]","Entry$ < 2");
    t6->Scan("Entry$:EvtVtx[0]:EvtVtx[1]:EvtVtx[2]:EvtVtx[3]","Entry$ < 2");
}

// /storage/gpfs_data/neutrino/users/mt/ANA/spill_slicing/productions/SAND/SAND_13/run.sh 
// 4853
// /storage/gpfs_data/neutrino/SAND/LBNF-GSIMPLE-FILES/gsimple_subdetectors/neutrino/gsimple_DUNE_ND_NEAR_FID1_g4lbne_v3r5p4_QGSP_BERT_OptimizedEngineeredNov2017_neutrino_00035_00034.root

// /storage/gpfs_data/neutrino/users/mt/ANA/spill_slicing/productions/SAND/SAND_12/run.sh 
// 9069
// /storage/gpfs_data/neutrino/SAND/LBNF-GSIMPLE-FILES/gsimple_subdetectors/neutrino/gsimple_DUNE_ND_NEAR_FID1_g4lbne_v3r5p4_QGSP_BERT_OptimizedEngineeredNov2017_neutrino_00035_00034.root

/*
41	7	TRUE
52	17	TRUE
13	35	TRUE
28	65	TRUE
91	77	TRUE
73	85	TRUE
39	102	TRUE
70	102	TRUE
68	110	TRUE
77	114	TRUE
85	165	TRUE
40	213	TRUE
93	226	TRUE
32	236	TRUE
81	241	TRUE

29	4	FALSE
0	7	FALSE
41	7	TRUE
99	10	FALSE
61	16	FALSE
48	17	FALSE
52	17	TRUE
56	18	FALSE
98	26	FALSE
63	27	FALSE
97	28	FALSE
14	29	FALSE
64	31	FALSE
19	32	FALSE
54	33	FALSE
49	34	FALSE
12	35	FALSE
13	35	TRUE
1	36	FALSE
2	39	FALSE
18	41	FALSE
60	44	FALSE
75	45	FALSE
76	48	FALSE
16	50	FALSE
50	52	FALSE
88	60	FALSE
3	61	FALSE
11	65	FALSE
28	65	TRUE
66	67	FALSE
6	75	FALSE
82	77	FALSE
91	77	TRUE
83	78	FALSE
62	82	FALSE
35	85	FALSE
73	85	TRUE
47	94	FALSE
22	100	FALSE
26	102	FALSE
39	102	TRUE
70	102	TRUE
90	103	FALSE
89	108	FALSE
8	110	FALSE
68	110	TRUE
4	113	FALSE
30	114	FALSE
77	114	TRUE
20	117	FALSE
37	118	FALSE
38	119	FALSE
84	123	FALSE
71	125	FALSE
9	127	FALSE
55	133	FALSE
53	135	FALSE
44	138	FALSE
94	144	FALSE
51	147	FALSE
36	164	FALSE
33	165	FALSE
85	165	TRUE
59	172	FALSE
96	179	FALSE
87	182	FALSE
17	183	FALSE
78	185	FALSE
5	187	FALSE
80	190	FALSE
92	191	FALSE
46	195	FALSE
72	199	FALSE
95	205	FALSE
67	206	FALSE
24	208	FALSE
42	209	FALSE
65	211	FALSE
7	213	FALSE
40	213	TRUE
69	214	FALSE
57	217	FALSE
10	219	FALSE
31	222	FALSE
15	223	FALSE
34	225	FALSE
25	226	FALSE
93	226	TRUE
79	233	FALSE
21	234	FALSE
23	236	FALSE
32	236	TRUE
58	238	FALSE
43	241	FALSE
81	241	TRUE
45	244	FALSE
74	246	FALSE
27	250	FALSE
*/