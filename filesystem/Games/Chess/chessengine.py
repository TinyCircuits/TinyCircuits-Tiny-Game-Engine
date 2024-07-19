piece_values = {"P": 100, "N": 280, "B": 320, "R": 479, "Q": 929, "K": 60000}
pstable = {
    'P': (  829, 829, 829, 829,829, 829, 829, 829,
            78,  83,  86,  73, 102,  82,  85,  90,
            7,  29,  21,  44,  40,  31,  44,   7,
        -17,  16,  -2,  15,  14,   0,  15, -13,
        -26,   3,  10,   9,   6,   1,   0, -23,
        -22,   9,   5, -11, -10,  -2,   3, -19,
        -31,   8,  -7, -37, -36, -14,   3, -31,
            0,   0,   0,   0,   0,   0,   0,   0),
    'N': ( -66, -53, -75, -75, -10, -55, -58, -70,
            -3,  -6, 100, -36,   4,  62,  -4, -14,
            10,  67,   1,  74,  73,  27,  62,  -2,
            24,  24,  45,  37,  33,  41,  25,  17,
            -1,   5,  31,  21,  22,  35,   2,   0,
        -18,  10,  13,  22,  18,  15,  11, -14,
        -23, -15,   2,   0,   2,   0, -23, -20,
        -74, -23, -26, -24, -19, -35, -22, -69),
    'B': ( -59, -78, -82, -76, -23,-107, -37, -50,
        -11,  20,  35, -42, -39,  31,   2, -22,
            -9,  39, -32,  41,  52, -10,  28, -14,
            25,  17,  20,  34,  26,  25,  15,  10,
            13,  10,  17,  23,  17,  16,   0,   7,
            14,  25,  24,  15,   8,  25,  20,  15,
            19,  20,  11,   6,   7,   6,  20,  16,
            -7,   2, -15, -12, -14, -15, -10, -10),
    'R': (  35,  29,  33,   4,  37,  33,  56,  50,
            55,  29,  56,  67,  55,  62,  34,  60,
            19,  35,  28,  33,  45,  27,  25,  15,
            0,   5,  16,  13,  18,  -4,  -9,  -6,
        -28, -35, -16, -21, -13, -29, -46, -30,
        -42, -28, -42, -25, -25, -35, -26, -46,
        -53, -38, -31, -26, -29, -43, -44, -53,
        -30, -24, -18,   5,  -2, -18, -31, -32),
    'Q': (   6,   1,  -8,-104,  69,  24,  88,  26,
            14,  32,  60, -10,  20,  76,  57,  24,
            -2,  43,  32,  60,  72,  63,  43,   2,
            1, -16,  22,  17,  25,  20, -13,  -6,
        -14, -15,  -2,  -5,  -1, -10, -20, -22,
        -30,  -6, -13, -11, -16, -11, -16, -27,
        -36, -18,   0, -19, -15, -15, -21, -38,
        -39, -30, -31, -13, -31, -36, -34, -42),
    'K': (   4,  54,  47, -99, -99,  60,  83, -62,
        -32,  10,  55,  56,  56,  55,  10,   3,
        -62,  12, -57,  44, -67,  28,  37, -31,
        -55,  50,  11,  -4, -19,  13,   0, -49,
        -55, -43, -52, -28, -51, -47,  -8, -50,
        -47, -42, -43, -79, -64, -32, -29, -32,
            -4,   3, -14, -50, -57, -18,  13,   4,
            17,  30,  -3, -14,   6,  -1,  40,  18),
}

openings = {
    "Ruy Lopez Main Line": ["e4", "e5", "Nf3", "Nc6", "Bb5", "a6", "Ba4", "Nf6", "O-O", "Be7", "Re1", "b5", "Bb3", "d6", "c3", "O-O"],
    "Ruy Lopez Closed Ruy Lopez": ["e4", "e5", "Nf3", "Nc6", "Bb5", "a6", "Ba4", "Nf6", "O-O", "Be7", "Re1", "b5", "Bb3", "d6", "c3", "O-O", "h3", "Nb8", "d4", "Nbd7"],
    "Ruy Lopez Open Ruy Lopez": ["e4", "e5", "Nf3", "Nc6", "Bb5", "a6", "Ba4", "Nf6", "O-O", "Be7", "Re1", "b5", "Bb3", "d6", "c3", "Na5", "Bc2", "c5"],
    "Ruy Lopez Breyer Defense": ["e4", "e5", "Nf3", "Nc6", "Bb5", "a6", "Ba4", "Nf6", "O-O", "Be7", "Re1", "b5", "Bb3", "d6", "c3", "O-O", "h3", "Nb8", "d4", "Nbd7", "Nbd2", "Bb7"],
    "Ruy Lopez Marshall Attack": ["e4", "e5", "Nf3", "Nc6", "Bb5", "a6", "Ba4", "Nf6", "O-O", "Be7", "Re1", "b5", "Bb3", "d6", "c3", "d5", "exd5", "Nxd5", "Nxe5", "Nxe5", "Rxe5", "c6"],
    
    "Italian Game Main Line": ["e4", "e5", "Nf3", "Nc6", "Bc4", "Bc5", "c3", "Nf6", "d3", "d6", "O-O", "O-O", "Re1", "a6", "a4", "Ba7"],
    "Italian Game Evans Gambit": ["e4", "e5", "Nf3", "Nc6", "Bc4", "Bc5", "b4", "Bxb4", "c3", "Ba5"],
    "Italian Game Giuoco Pianissimo": ["e4", "e5", "Nf3", "Nc6", "Bc4", "Bc5", "d3", "d6", "c3", "a6"],
    "Italian Game Greco Attack": ["e4", "e5", "Nf3", "Nc6", "Bc4", "Bc5", "c3", "Nf6", "d4", "exd4", "cxd4", "Bb4+"],
    "Italian Game Two Knights Defense": ["e4", "e5", "Nf3", "Nc6", "Bc4", "Nf6", "Ng5", "d5", "exd5", "Na5"],
    
    "Sicilian Defense Main Line": ["e4", "c5", "Nf3", "d6", "d4", "cxd4", "Nxd4", "Nf6", "Nc3", "a6", "Be3", "e6", "f3", "Be7", "Qd2", "O-O"],
    "Sicilian Defense Najdorf": ["e4", "c5", "Nf3", "d6", "d4", "cxd4", "Nxd4", "Nf6", "Nc3", "a6", "Be3", "e5", "Nb3", "Be6"],
    "Sicilian Defense Dragon": ["e4", "c5", "Nf3", "d6", "d4", "cxd4", "Nxd4", "Nf6", "Nc3", "g6", "Be3", "Bg7", "f3", "O-O", "Qd2", "Nc6"],
    "Sicilian Defense Scheveningen": ["e4", "c5", "Nf3", "d6", "d4", "cxd4", "Nxd4", "Nf6", "Nc3", "a6", "Be3", "e6", "f3", "Be7"],
    "Sicilian Defense Sveshnikov": ["e4", "c5", "Nf3", "d6", "d4", "cxd4", "Nxd4", "Nf6", "Nc3", "e5", "Ndb5", "d6", "Na3", "b5"],
    
    "French Defense Main Line": ["e4", "e6", "d4", "d5", "Nc3", "Nf6", "Bg5", "Be7", "e5", "Nfd7", "Bxe7", "Qxe7", "f4", "O-O", "Nf3", "c5"],
    "French Defense Classical": ["e4", "e6", "d4", "d5", "Nc3", "Nf6", "Bg5", "dxe4", "Nxe4", "Be7"],
    "French Defense Winawer": ["e4", "e6", "d4", "d5", "Nc3", "Bb4", "e5", "c5", "a3", "Bxc3+", "bxc3", "Ne7"],
    "French Defense Tarrasch": ["e4", "e6", "d4", "d5", "Nd2", "Nf6", "e5", "Nfd7", "Bd3", "c5"],
    "French Defense Advance": ["e4", "e6", "d4", "d5", "e5", "c5", "c3", "Nc6", "Nf3", "Qb6"],
    
    "Caro-Kann Defense Main Line": ["e4", "c6", "d4", "d5", "Nc3", "dxe4", "Nxe4", "Bf5", "Ng3", "Bg6", "h4", "h6", "Nf3", "Nd7", "h5", "Bh7", "Bd3", "Bxd3"],
    "Caro-Kann Defense Advance": ["e4", "c6", "d4", "d5", "e5", "Bf5", "Nc3", "e6", "g4", "Bg6"],
    "Caro-Kann Defense Classical": ["e4", "c6", "d4", "d5", "Nc3", "dxe4", "Nxe4", "Bf5", "Ng3", "Bg6", "h4", "h6", "Nf3", "Nd7"],
    "Caro-Kann Defense Panov-Botvinnik Attack": ["e4", "c6", "d4", "d5", "exd5", "cxd5", "c4", "Nf6", "Nc3", "e6", "Nf3", "Bb4"],
    "Caro-Kann Defense Two Knights": ["e4", "c6", "Nf3", "d5", "Nc3", "Bg4"],
    
    "Queen's Gambit Main Line": ["d4", "d5", "c4", "e6", "Nc3", "Nf6", "Bg5", "Be7", "e3", "O-O", "Nf3", "h6", "Bh4", "b6"],
    "Queen's Gambit Accepted": ["d4", "d5", "c4", "dxc4", "Nf3", "Nf6", "e3", "e6", "Bxc4", "c5", "O-O", "a6", "dxc5", "Bxc5", "Qe2", "b5"],
    "Queen's Gambit Declined": ["d4", "d5", "c4", "e6", "Nc3", "Nf6", "Bg5", "Be7"],
    "Slav Defense": ["d4", "d5", "c4", "c6"],
    "Chigorin Defense": ["d4", "d5", "c4", "Nc6"],
    
    "Queen's Gambit Accepted Classical": ["d4", "d5", "c4", "dxc4", "Nf3", "Nf6", "e3", "e6", "Bxc4", "c5", "O-O", "a6"],
    "Queen's Gambit Accepted Modern": ["d4", "d5", "c4", "dxc4", "e4"],
    "Queen's Gambit Accepted Alekhine": ["d4", "d5", "c4", "dxc4", "e4"],
    "Queen's Gambit Accepted Lasker Defense": ["d4", "d5", "c4", "dxc4", "Nc3", "e6", "e4", "c5"],
    
    "King's Indian Defense Main Line": ["d4", "Nf6", "c4", "g6", "Nc3", "Bg7", "e4", "d6", "Nf3", "O-O", "Be2", "e5", "O-O", "Nc6", "d5", "Ne7"],
    "King's Indian Defense Classical": ["d4", "Nf6", "c4", "g6", "Nc3", "Bg7", "e4", "d6", "Nf3", "O-O", "Be2", "Nc6", "d5", "Ne7"],
    "King's Indian Defense Fianchetto": ["d4", "Nf6", "c4", "g6", "Nf3", "Bg7", "g3", "O-O", "Bg2", "d6"],
    "King's Indian Defense Four Pawns Attack": ["d4", "Nf6", "c4", "g6", "e4", "d6", "f4"],
    "King's Indian Defense Saemisch": ["d4", "Nf6", "c4", "g6", "Nc3", "Bg7", "f3"],
    
    "Nimzo-Indian Defense Main Line": ["d4", "Nf6", "c4", "e6", "Nc3", "Bb4", "e3", "O-O", "Bd3", "d5", "Nf3", "c5", "O-O", "Nc6", "a3", "Bxc3"],
    "Nimzo-Indian Defense Rubinstein": ["d4", "Nf6", "c4", "e6", "Nc3", "Bb4", "e3", "O-O", "Bd3", "d5", "Nf3"],
    "Nimzo-Indian Defense Classical": ["d4", "Nf6", "c4", "e6", "Nc3", "Bb4", "Qc2", "O-O", "a3", "Bxc3", "Qxc3", "d5"],
    "Nimzo-Indian Defense Leningrad": ["d4", "Nf6", "c4", "e6", "Nc3", "Bb4", "Bg5", "h6", "Bh4", "c5"],
    "Nimzo-Indian Defense Fischer Variation": ["d4", "Nf6", "c4", "e6", "Nc3", "Bb4", "Qc2", "O-O", "a3", "Bxc3", "Qxc3", "d5"],
    
    "Slav Defense Main Line": ["d4", "d5", "c4", "c6", "Nf3", "Nf6", "Nc3", "dxc4", "a4", "Bf5", "e3", "e6", "Bxc4", "Bb4", "O-O", "O-O"],
    "Slav Defense Exchange": ["d4", "d5", "c4", "c6", "cxd5", "cxd5"],
    "Slav Defense Chebanenko": ["d4", "d5", "c4", "c6", "Nf3", "a6"],
    "Slav Defense Moran Defense": ["d4", "d5", "c4", "c6", "Nf3"],
    "Slav Defense Schlechter": ["d4", "d5", "c4", "c6", "Nf3", "e6", "Nbd2"],
    
    "English Opening Main Line": ["c4", "e5", "Nc3", "Nc6", "g3", "g6", "Bg2", "Bg7", "d3", "d6", "e4", "Be6", "Nge2", "Qd7"],
    "English Opening Symmetrical": ["c4", "c5", "g3", "g6"],
    "English Opening Reversed Sicilian": ["c4", "e5", "Nf3", "Nc6"],
    "English Opening Botvinnik": ["c4", "g3"],
    "English Opening Four Knights": ["c4", "e5", "Nc3", "Nc6", "Nf3"],
    
    "Reti Opening Main Line": ["Nf3", "d5", "c4", "c6", "g3", "Nf6", "Bg2", "Bf5", "O-O", "e6", "d3", "h6", "Nc3", "Be7", "Re1", "O-O"],
    "Reti Opening King's Indian Attack": ["Nf3", "d5", "g3", "Bg4", "d3", "Nd7", "Nbd2", "Ngf6"],
    "Reti Opening Closed": ["Nf3", "c4", "c6", "d4"],
    "Reti Opening English Variation": ["c4", "Nf6", "Nf3", "c5"],
    "Reti Opening Catalan": ["Nf3", "d5", "g3", "e6", "Bg2"],
    
    "London System Main Line": ["d4", "d5", "Nf3", "Nf6", "Bf4", "e6", "e3", "c5", "c3", "Nc6", "Nbd2", "Bd6", "Bg3", "O-O", "Bd3", "b6"],
    "London System Accelerated": ["d4", "Bf4"],
    "London System Jobava": ["d4", "d5", "Nc3", "Nf6", "Bf4"],
    "London System Barry Attack": ["d4", "Nf6", "Nc3", "e6", "Bf4"],
    "London System Colle System": ["d4", "Nf3", "e6", "e3"],
    
    "Scandinavian Defense Main Line": ["e4", "d5", "exd5", "Qxd5", "Nc3", "Qa5", "d4", "c6", "Nf3", "Bg4", "Be2", "e6", "O-O", "Nd7", "h3", "Bh5"],
    "Scandinavian Defense Modern": ["e4", "d5", "d4"],
    "Scandinavian Defense Portuguese": ["e4", "d5", "Nf3"],
    "Scandinavian Defense Classical": ["e4", "d5", "exd5", "Qxd5", "Nc3", "Qa5", "d4", "Nf6"],
    "Scandinavian Defense Patzer": ["e4", "d5", "Qe2"],
    
    "Pirc Defense Main Line": ["e4", "d6", "d4", "Nf6", "Nc3", "g6", "Be2", "Bg7", "Be3", "O-O", "Qd2", "c6", "Bh6", "b5"],
    "Pirc Defense Austrian Attack": ["e4", "d6", "d4", "Nf6", "Nc3", "g6", "f4"],
    "Pirc Defense Classical": ["e4", "d6", "d4", "Nf6", "Nc3", "g6", "Nf3", "Bg7", "Be2", "O-O", "O-O", "c6"],
    "Pirc Defense 150 Attack": ["e4", "d6", "d4", "Nf6", "Nc3", "g6", "Be3", "Bg7", "Qd2", "O-O", "O-O-O"],
    "Pirc Defense Byrne": ["e4", "d6", "d4", "Nf6", "Nc3", "g6", "Bg5", "Bg7", "Qd2", "h6", "Be3"]
}

K_MOVES = [
    (770, 0),
    (1797, 0),
    (3594, 0),
    (7188, 0),
    (14376, 0),
    (28752, 0),
    (57504, 0),
    (49216, 0),
    (197123, 0),
    (460039, 0),
    (920078, 0),
    (1840156, 0),
    (3680312, 0),
    (7360624, 0),
    (14721248, 0),
    (12599488, 0),
    (50463488, 0),
    (117769984, 0),
    (235539968, 0),
    (471079936, 0),
    (942159872, 0),
    (1884319744, 0),
    (3768639488, 0),
    (3225468928, 0),
    (33751040, 3),
    (84344832, 7),
    (168689664, 14),
    (337379328, 28),
    (674758656, 56),
    (1349517312, 112),
    (2699034624, 224),
    (1086324736, 192),
    (50331648, 770),
    (117440512, 1797),
    (234881024, 3594),
    (469762048, 7188),
    (939524096, 14376),
    (1879048192, 28752),
    (3758096384, 57504),
    (3221225472, 49216),
    (0, 197123),
    (0, 460039),
    (0, 920078),
    (0, 1840156),
    (0, 3680312),
    (0, 7360624),
    (0, 14721248),
    (0, 12599488),
    (0, 50463488),
    (0, 117769984),
    (0, 235539968),
    (0, 471079936),
    (0, 942159872),
    (0, 1884319744),
    (0, 3768639488),
    (0, 3225468928),
    (0, 33751040),
    (0, 84344832),
    (0, 168689664),
    (0, 337379328),
    (0, 674758656),
    (0, 1349517312),
    (0, 2699034624),
    (0, 1086324736),
]
Q_MOVES = [
    (151323646, 2168529169),
    (302647293, 42091042),
    (605359867, 67404868),
    (1227496695, 134744200),
    (2454993135, 269488145),
    (615018719, 538976546),
    (1213259967, 1078018628),
    (2426454143, 2172814472),
    (84147715, 1092686089),
    (168295687, 2185372178),
    (353303310, 75777060),
    (706541340, 134776905),
    (1413017400, 269488530),
    (2825969520, 539042340),
    (1356906464, 1094861896),
    (2696970176, 2189723792),
    (66978565, 554764549),
    (134022922, 1109529098),
    (251334165, 2219058197),
    (485956650, 143149354),
    (955201620, 269587028),
    (1893691560, 555885736),
    (3770671184, 1111771216),
    (3229597856, 2223542432),
    (4261610761, 285803779),
    (4245096978, 571607559),
    (4212004132, 1143215374),
    (4145818185, 2286496284),
    (4013446290, 294802488),
    (3748702244, 572827760),
    (3219148872, 1145589984),
    (2143330448, 2291179712),
    (50661649, 151323646),
    (118100514, 302647293),
    (236266564, 605359867),
    (472533384, 1227496695),
    (945066513, 2454993135),
    (1890067490, 615018719),
    (3763357764, 1213259967),
    (3231748232, 2426454143),
    (84480289, 84147715),
    (168960578, 168295687),
    (354698372, 353303310),
    (709462024, 706541340),
    (1418858768, 1413017400),
    (2820940321, 2825969520),
    (1346913346, 1356906464),
    (2693826692, 2696970176),
    (152117569, 66978565),
    (304235138, 134022922),
    (608470020, 251334165),
    (1233651720, 485956650),
    (2450591760, 955201620),
    (606216480, 1893691560),
    (1212432961, 3770671184),
    (2424865922, 3229597856),
    (287392129, 4261610761),
    (574784002, 4245096978),
    (1149502468, 4212004132),
    (2282227720, 4145818185),
    (286265360, 4013446290),
    (572596256, 3748702244),
    (1145192768, 3219148872),
    (2290385537, 2143330448),
]
R_MOVES = [
    (16843262, 16843009),
    (33686269, 33686018),
    (67372283, 67372036),
    (134744311, 134744072),
    (269488367, 269488144),
    (538976479, 538976288),
    (1077952703, 1077952576),
    (2155905151, 2155905152),
    (16907777, 16843009),
    (33750274, 33686018),
    (67435268, 67372036),
    (134805256, 134744072),
    (269545232, 269488144),
    (539025184, 538976288),
    (1077985088, 1077952576),
    (2155904896, 2155905152),
    (33423617, 16843009),
    (50135554, 33686018),
    (83559428, 67372036),
    (150407176, 134744072),
    (284102672, 269488144),
    (551493664, 538976288),
    (1086275648, 1077952576),
    (2155839616, 2155905152),
    (4261478657, 16843009),
    (4244767234, 33686018),
    (4211344388, 67372036),
    (4144498696, 134744072),
    (4010807312, 269488144),
    (3743424544, 538976288),
    (3208659008, 1077952576),
    (2139127936, 2155905152),
    (16843009, 16843262),
    (33686018, 33686269),
    (67372036, 67372283),
    (134744072, 134744311),
    (269488144, 269488367),
    (538976288, 538976479),
    (1077952576, 1077952703),
    (2155905152, 2155905151),
    (16843009, 16907777),
    (33686018, 33750274),
    (67372036, 67435268),
    (134744072, 134805256),
    (269488144, 269545232),
    (538976288, 539025184),
    (1077952576, 1077985088),
    (2155905152, 2155904896),
    (16843009, 33423617),
    (33686018, 50135554),
    (67372036, 83559428),
    (134744072, 150407176),
    (269488144, 284102672),
    (538976288, 551493664),
    (1077952576, 1086275648),
    (2155905152, 2155839616),
    (16843009, 4261478657),
    (33686018, 4244767234),
    (67372036, 4211344388),
    (134744072, 4144498696),
    (269488144, 4010807312),
    (538976288, 3743424544),
    (1077952576, 3208659008),
    (2155905152, 2139127936),
]
B_MOVES = [
    (134480384, 2151686160),
    (268961024, 8405024),
    (537987584, 32832),
    (1092752384, 128),
    (2185504768, 1),
    (76042240, 258),
    (135307264, 66052),
    (270548992, 16909320),
    (67239938, 1075843080),
    (134545413, 2151686160),
    (285868042, 8405024),
    (571736084, 32833),
    (1143472168, 386),
    (2286944336, 66052),
    (278921376, 16909320),
    (541065280, 33818640),
    (33554948, 537921540),
    (83887368, 1075843080),
    (167774737, 2151686161),
    (335549474, 8405282),
    (671098948, 98884),
    (1342197896, 16909448),
    (2684395536, 33818640),
    (1073758240, 67637280),
    (132104, 268960770),
    (329744, 537921541),
    (659744, 1075843338),
    (1319489, 2151752212),
    (2638978, 25314344),
    (5277700, 33851472),
    (10489864, 67637408),
    (4202512, 135274560),
    (33818640, 134480384),
    (84414496, 268961024),
    (168894528, 537987584),
    (337789312, 1092752384),
    (675578369, 2185504768),
    (1351091202, 76042240),
    (2685405188, 135307264),
    (1075843080, 270548992),
    (67637280, 67239938),
    (135274560, 134545413),
    (287326336, 285868042),
    (574717952, 571736084),
    (1149370624, 1143472168),
    (2281964033, 2286944336),
    (268960770, 278921376),
    (537921540, 541065280),
    (135274560, 33554948),
    (270549120, 83887368),
    (541097984, 167774737),
    (1098907648, 335549474),
    (2181103616, 671098948),
    (67240192, 1342197896),
    (134480385, 2684395536),
    (268960770, 1073758240),
    (270549120, 132104),
    (541097984, 329744),
    (1082130432, 659744),
    (2147483648, 1319489),
    (16777216, 2638978),
    (33619968, 5277700),
    (67240192, 10489864),
    (134480385, 4202512),
]
N_MOVES = [
    (132096, 0),
    (329856, 0),
    (659712, 0),
    (1319424, 0),
    (2638848, 0),
    (5277696, 0),
    (10555393, 0),
    (4202496, 0),
    (33816580, 0),
    (84443144, 0),
    (168886289, 0),
    (337772578, 0),
    (675545156, 0),
    (1351090312, 0),
    (2702180624, 0),
    (1075839008, 0),
    (67109890, 2),
    (142608517, 5),
    (285217034, 10),
    (570434068, 20),
    (1140868136, 40),
    (2281736272, 80),
    (268505248, 161),
    (536879168, 64),
    (262656, 516),
    (2148041984, 1288),
    (1116672, 2577),
    (2233344, 5154),
    (4466688, 10308),
    (8933376, 20616),
    (17866752, 41232),
    (2113536, 16416),
    (67239936, 132096),
    (142934016, 329856),
    (285868032, 659712),
    (571736064, 1319424),
    (1143472128, 2638848),
    (2286944256, 5277696),
    (278921216, 10555393),
    (541065216, 4202496),
    (33554432, 33816580),
    (2231369728, 84443144),
    (167772160, 168886289),
    (335544320, 337772578),
    (671088640, 675545156),
    (1342177280, 1351090312),
    (2684354560, 2702180624),
    (1073741824, 1075839008),
    (0, 67109890),
    (0, 142608517),
    (0, 285217034),
    (0, 570434068),
    (0, 1140868136),
    (0, 2281736272),
    (0, 268505248),
    (0, 536879168),
    (0, 262656),
    (0, 2148041984),
    (0, 1116672),
    (0, 2233344),
    (0, 4466688),
    (0, 8933376),
    (0, 17866752),
    (0, 2113536),
]
P_MOVES = [
    (896, 0),
    (1792, 0),
    (3584, 0),
    (7168, 0),
    (14336, 0),
    (28672, 0),
    (57344, 0),
    (114688, 0),
    (17006592, 0),
    (34013184, 0),
    (68026368, 0),
    (136052736, 0),
    (272105472, 0),
    (544210944, 0),
    (1088421888, 0),
    (2176843776, 0),
    (58720256, 0),
    (117440512, 0),
    (234881024, 0),
    (469762048, 0),
    (939524096, 0),
    (1879048192, 0),
    (3758096384, 0),
    (3221225472, 1),
    (2147483648, 3),
    (0, 7),
    (0, 14),
    (0, 28),
    (0, 56),
    (0, 112),
    (0, 224),
    (0, 448),
    (0, 896),
    (0, 1792),
    (0, 3584),
    (0, 7168),
    (0, 14336),
    (0, 28672),
    (0, 57344),
    (0, 114688),
    (0, 229376),
    (0, 458752),
    (0, 917504),
    (0, 1835008),
    (0, 3670016),
    (0, 7340032),
    (0, 14680064),
    (0, 29360128),
    (0, 58720256),
    (0, 117440512),
    (0, 234881024),
    (0, 469762048),
    (0, 939524096),
    (0, 1879048192),
    (0, 3758096384),
    (0, 3221225472),
    (0, 2147483648),
    (0, 0),
    (0, 0),
    (0, 0),
    (0, 0),
    (0, 0),
    (0, 0),
    (0, 0),
]
p_MOVES = [
    (0, 0),
    (0, 0),
    (0, 0),
    (0, 0),
    (0, 0),
    (0, 0),
    (0, 0),
    (1, 0),
    (3, 0),
    (7, 0),
    (14, 0),
    (28, 0),
    (56, 0),
    (112, 0),
    (224, 0),
    (448, 0),
    (896, 0),
    (1792, 0),
    (3584, 0),
    (7168, 0),
    (14336, 0),
    (28672, 0),
    (57344, 0),
    (114688, 0),
    (229376, 0),
    (458752, 0),
    (917504, 0),
    (1835008, 0),
    (3670016, 0),
    (7340032, 0),
    (14680064, 0),
    (29360128, 0),
    (58720256, 0),
    (117440512, 0),
    (234881024, 0),
    (469762048, 0),
    (939524096, 0),
    (1879048192, 0),
    (3758096384, 0),
    (3221225472, 1),
    (2147483648, 3),
    (0, 7),
    (0, 14),
    (0, 28),
    (0, 56),
    (0, 112),
    (0, 224),
    (0, 448),
    (0, 897),
    (0, 1794),
    (0, 3588),
    (0, 7176),
    (0, 14352),
    (0, 28704),
    (0, 57408),
    (0, 114816),
    (0, 229376),
    (0, 458752),
    (0, 917504),
    (0, 1835008),
    (0, 3670016),
    (0, 7340032),
    (0, 14680064),
    (0, 29360128),
]