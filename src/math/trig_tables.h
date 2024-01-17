#ifndef TRIG_TABLES_H
#define TRIG_TABLES_H

#include <stdint.h>


#ifndef TAN_SIN_TABLE_INCLUDED
#define TAN_SIN_TABLE_INCLUDED

// 16-bit tangent, sine table from 0..pi/2.
static const uint16_t tan_sin_tab[512] = {
    0, 0,
    201, 402,
    402, 804,
    603, 1206,
    804, 1608,
    1005, 2010,
    1207, 2412,
    1408, 2814,
    1609, 3216,
    1810, 3617,
    2011, 4019,
    2213, 4420,
    2414, 4821,
    2615, 5222,
    2817, 5623,
    3018, 6023,
    3220, 6424,
    3421, 6824,
    3623, 7224,
    3825, 7623,
    4026, 8022,
    4228, 8421,
    4430, 8820,
    4632, 9218,
    4834, 9616,
    5036, 10014,
    5239, 10411,
    5441, 10808,
    5644, 11204,
    5846, 11600,
    6049, 11996,
    6252, 12391,
    6455, 12785,
    6658, 13180,
    6861, 13573,
    7064, 13966,
    7268, 14359,
    7471, 14751,
    7675, 15143,
    7879, 15534,
    8083, 15924,
    8287, 16314,
    8492, 16703,
    8696, 17091,
    8901, 17479,
    9106, 17867,
    9311, 18253,
    9516, 18639,
    9721, 19024,
    9927, 19409,
    10133, 19792,
    10339, 20175,
    10545, 20557,
    10751, 20939,
    10958, 21320,
    11165, 21699,
    11372, 22078,
    11579, 22457,
    11786, 22834,
    11994, 23210,
    12202, 23586,
    12410, 23961,
    12618, 24335,
    12827, 24708,
    13036, 25080,
    13245, 25451,
    13454, 25821,
    13664, 26190,
    13874, 26558,
    14084, 26925,
    14295, 27291,
    14506, 27656,
    14717, 28020,
    14928, 28383,
    15140, 28745,
    15352, 29106,
    15564, 29466,
    15776, 29824,
    15989, 30182,
    16202, 30538,
    16416, 30893,
    16630, 31248,
    16844, 31600,
    17058, 31952,
    17273, 32303,
    17489, 32652,
    17704, 33000,
    17920, 33347,
    18136, 33692,
    18353, 34037,
    18570, 34380,
    18787, 34721,
    19005, 35062,
    19223, 35401,
    19442, 35738,
    19661, 36075,
    19880, 36410,
    20100, 36744,
    20320, 37076,
    20541, 37407,
    20762, 37736,
    20983, 38064,
    21205, 38391,
    21427, 38716,
    21650, 39040,
    21873, 39362,
    22097, 39683,
    22321, 40002,
    22546, 40320,
    22771, 40636,
    22997, 40951,
    23223, 41264,
    23449, 41576,
    23676, 41886,
    23904, 42194,
    24132, 42501,
    24360, 42806,
    24590, 43110,
    24819, 43412,
    25049, 43713,
    25280, 44011,
    25511, 44308,
    25743, 44604,
    25975, 44898,
    26208, 45190,
    26442, 45480,
    26676, 45769,
    26911, 46056,
    27146, 46341,
    27382, 46624,
    27618, 46906,
    27855, 47186,
    28093, 47464,
    28331, 47741,
    28570, 48015,
    28810, 48288,
    29050, 48559,
    29291, 48828,
    29533, 49095,
    29775, 49361,
    30018, 49624,
    30261, 49886,
    30506, 50146,
    30751, 50404,
    30996, 50660,
    31243, 50914,
    31490, 51166,
    31738, 51417,
    31986, 51665,
    32236, 51911,
    32486, 52156,
    32736, 52398,
    32988, 52639,
    33240, 52878,
    33494, 53114,
    33748, 53349,
    34002, 53581,
    34258, 53812,
    34514, 54040,
    34772, 54267,
    35030, 54491,
    35289, 54714,
    35548, 54934,
    35809, 55152,
    36071, 55368,
    36333, 55582,
    36596, 55794,
    36861, 56004,
    37126, 56212,
    37392, 56418,
    37659, 56621,
    37927, 56823,
    38196, 57022,
    38465, 57219,
    38736, 57414,
    39008, 57607,
    39281, 57798,
    39555, 57986,
    39829, 58172,
    40105, 58356,
    40382, 58538,
    40660, 58718,
    40939, 58896,
    41219, 59071,
    41500, 59244,
    41782, 59415,
    42066, 59583,
    42350, 59750,
    42636, 59914,
    42923, 60075,
    43210, 60235,
    43500, 60392,
    43790, 60547,
    44081, 60700,
    44374, 60851,
    44668, 60999,
    44963, 61145,
    45259, 61288,
    45557, 61429,
    45856, 61568,
    46156, 61705,
    46457, 61839,
    46760, 61971,
    47064, 62101,
    47369, 62228,
    47676, 62353,
    47984, 62476,
    48294, 62596,
    48605, 62714,
    48917, 62830,
    49231, 62943,
    49546, 63054,
    49863, 63162,
    50181, 63268,
    50501, 63372,
    50822, 63473,
    51145, 63572,
    51469, 63668,
    51795, 63763,
    52122, 63854,
    52451, 63944,
    52782, 64031,
    53114, 64115,
    53448, 64197,
    53784, 64277,
    54121, 64354,
    54460, 64429,
    54801, 64501,
    55144, 64571,
    55488, 64639,
    55834, 64704,
    56182, 64766,
    56532, 64827,
    56883, 64884,
    57237, 64940,
    57592, 64993,
    57950, 65043,
    58309, 65091,
    58670, 65137,
    59033, 65180,
    59398, 65220,
    59766, 65259,
    60135, 65294,
    60506, 65328,
    60880, 65358,
    61255, 65387,
    61633, 65413,
    62013, 65436,
    62395, 65457,
    62780, 65476,
    63167, 65492,
    63556, 65505,
    63947, 65516,
    64341, 65525,
    64737, 65531,
    65135, 65535,
};

#endif


// From 0..pi/2
static const uint16_t tan_table[256] = {
    0,
    201,
    402,
    603,
    804,
    1005,
    1207,
    1408,
    1609,
    1810,
    2011,
    2213,
    2414,
    2615,
    2817,
    3018,
    3220,
    3421,
    3623,
    3825,
    4026,
    4228,
    4430,
    4632,
    4834,
    5036,
    5239,
    5441,
    5644,
    5846,
    6049,
    6252,
    6455,
    6658,
    6861,
    7064,
    7268,
    7471,
    7675,
    7879,
    8083,
    8287,
    8492,
    8696,
    8901,
    9106,
    9311,
    9516,
    9721,
    9927,
    10133,
    10339,
    10545,
    10751,
    10958,
    11165,
    11372,
    11579,
    11786,
    11994,
    12202,
    12410,
    12618,
    12827,
    13036,
    13245,
    13454,
    13664,
    13874,
    14084,
    14295,
    14506,
    14717,
    14928,
    15140,
    15352,
    15564,
    15776,
    15989,
    16202,
    16416,
    16630,
    16844,
    17058,
    17273,
    17489,
    17704,
    17920,
    18136,
    18353,
    18570,
    18787,
    19005,
    19223,
    19442,
    19661,
    19880,
    20100,
    20320,
    20541,
    20762,
    20983,
    21205,
    21427,
    21650,
    21873,
    22097,
    22321,
    22546,
    22771,
    22997,
    23223,
    23449,
    23676,
    23904,
    24132,
    24360,
    24590,
    24819,
    25049,
    25280,
    25511,
    25743,
    25975,
    26208,
    26442,
    26676,
    26911,
    27146,
    27382,
    27618,
    27855,
    28093,
    28331,
    28570,
    28810,
    29050,
    29291,
    29533,
    29775,
    30018,
    30261,
    30506,
    30751,
    30996,
    31243,
    31490,
    31738,
    31986,
    32236,
    32486,
    32736,
    32988,
    33240,
    33494,
    33748,
    34002,
    34258,
    34514,
    34772,
    35030,
    35289,
    35548,
    35809,
    36071,
    36333,
    36596,
    36861,
    37126,
    37392,
    37659,
    37927,
    38196,
    38465,
    38736,
    39008,
    39281,
    39555,
    39829,
    40105,
    40382,
    40660,
    40939,
    41219,
    41500,
    41782,
    42066,
    42350,
    42636,
    42923,
    43210,
    43500,
    43790,
    44081,
    44374,
    44668,
    44963,
    45259,
    45557,
    45856,
    46156,
    46457,
    46760,
    47064,
    47369,
    47676,
    47984,
    48294,
    48605,
    48917,
    49231,
    49546,
    49863,
    50181,
    50501,
    50822,
    51145,
    51469,
    51795,
    52122,
    52451,
    52782,
    53114,
    53448,
    53784,
    54121,
    54460,
    54801,
    55144,
    55488,
    55834,
    56182,
    56532,
    56883,
    57237,
    57592,
    57950,
    58309,
    58670,
    59033,
    59398,
    59766,
    60135,
    60506,
    60880,
    61255,
    61633,
    62013,
    62395,
    62780,
    63167,
    63556,
    63947,
    64341,
    64737,
    65135
};


// From 0..pi/2
static const uint16_t sin_table[256] = {
    0,
    402,
    804,
    1206,
    1608,
    2010,
    2412,
    2814,
    3216,
    3617,
    4019,
    4420,
    4821,
    5222,
    5623,
    6023,
    6424,
    6824,
    7224,
    7623,
    8022,
    8421,
    8820,
    9218,
    9616,
    10014,
    10411,
    10808,
    11204,
    11600,
    11996,
    12391,
    12785,
    13180,
    13573,
    13966,
    14359,
    14751,
    15143,
    15534,
    15924,
    16314,
    16703,
    17091,
    17479,
    17867,
    18253,
    18639,
    19024,
    19409,
    19792,
    20175,
    20557,
    20939,
    21320,
    21699,
    22078,
    22457,
    22834,
    23210,
    23586,
    23961,
    24335,
    24708,
    25080,
    25451,
    25821,
    26190,
    26558,
    26925,
    27291,
    27656,
    28020,
    28383,
    28745,
    29106,
    29466,
    29824,
    30182,
    30538,
    30893,
    31248,
    31600,
    31952,
    32303,
    32652,
    33000,
    33347,
    33692,
    34037,
    34380,
    34721,
    35062,
    35401,
    35738,
    36075,
    36410,
    36744,
    37076,
    37407,
    37736,
    38064,
    38391,
    38716,
    39040,
    39362,
    39683,
    40002,
    40320,
    40636,
    40951,
    41264,
    41576,
    41886,
    42194,
    42501,
    42806,
    43110,
    43412,
    43713,
    44011,
    44308,
    44604,
    44898,
    45190,
    45480,
    45769,
    46056,
    46341,
    46624,
    46906,
    47186,
    47464,
    47741,
    48015,
    48288,
    48559,
    48828,
    49095,
    49361,
    49624,
    49886,
    50146,
    50404,
    50660,
    50914,
    51166,
    51417,
    51665,
    51911,
    52156,
    52398,
    52639,
    52878,
    53114,
    53349,
    53581,
    53812,
    54040,
    54267,
    54491,
    54714,
    54934,
    55152,
    55368,
    55582,
    55794,
    56004,
    56212,
    56418,
    56621,
    56823,
    57022,
    57219,
    57414,
    57607,
    57798,
    57986,
    58172,
    58356,
    58538,
    58718,
    58896,
    59071,
    59244,
    59415,
    59583,
    59750,
    59914,
    60075,
    60235,
    60392,
    60547,
    60700,
    60851,
    60999,
    61145,
    61288,
    61429,
    61568,
    61705,
    61839,
    61971,
    62101,
    62228,
    62353,
    62476,
    62596,
    62714,
    62830,
    62943,
    63054,
    63162,
    63268,
    63372,
    63473,
    63572,
    63668,
    63763,
    63854,
    63944,
    64031,
    64115,
    64197,
    64277,
    64354,
    64429,
    64501,
    64571,
    64639,
    64704,
    64766,
    64827,
    64884,
    64940,
    64993,
    65043,
    65091,
    65137,
    65180,
    65220,
    65259,
    65294,
    65328,
    65358,
    65387,
    65413,
    65436,
    65457,
    65476,
    65492,
    65505,
    65516,
    65525,
    65531,
    65535
};

#endif  // TRIG_TABLES_H