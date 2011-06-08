/*							shichi.c
 *
 *	Hyperbolic sine and cosine integrals
 *
 *
 *
 * SYNOPSIS:
 *
 * double x, Chi, Shi, shichi();
 *
 * shichi( x, &Chi, &Shi );
 *
 *
 * DESCRIPTION:
 *
 * Approximates the integrals
 *
 *                            x
 *                            -
 *                           | |   cosh t - 1
 *   Chi(x) = eul + ln x +   |    -----------  dt,
 *                         | |          t
 *                          -
 *                          0
 *
 *               x
 *               -
 *              | |  sinh t
 *   Shi(x) =   |    ------  dt
 *            | |       t
 *             -
 *             0
 *
 * where eul = 0.57721566490153286061 is Euler's constant.
 * The integrals are evaluated by power series for x < 8
 * and by Chebyshev expansions for x between 8 and 88.
 * For large x, both functions approach exp(x)/2x.
 * Arguments greater than 88 in magnitude return MAXNUM.
 *
 *
 * ACCURACY:
 *
 * Test interval 0 to 88.
 *                      Relative error:
 * arithmetic   function  # trials      peak         rms
 *    DEC          Shi       3000       9.1e-17
 *    IEEE         Shi      30000       6.9e-16     1.6e-16
 *        Absolute error, except relative when |Chi| > 1:
 *    DEC          Chi       2500       9.3e-17
 *    IEEE         Chi      30000       8.4e-16     1.4e-16
 */

/*
Cephes Math Library Release 2.0:  April, 1987
Copyright 1984, 1987 by Stephen L. Moshier
Direct inquiries to 30 Frost Street, Cambridge, MA 02140
*/


#include "mconf.h"
#include "cephes.h"

#ifdef UNK
/* x exp(-x) shi(x), inverted interval 8 to 18 */
static double S1[] = {
 1.83889230173399459482E-17,
-9.55485532279655569575E-17,
 2.04326105980879882648E-16,
 1.09896949074905343022E-15,
-1.31313534344092599234E-14,
 5.93976226264314278932E-14,
-3.47197010497749154755E-14,
-1.40059764613117131000E-12,
 9.49044626224223543299E-12,
-1.61596181145435454033E-11,
-1.77899784436430310321E-10,
 1.35455469767246947469E-9,
-1.03257121792819495123E-9,
-3.56699611114982536845E-8,
 1.44818877384267342057E-7,
 7.82018215184051295296E-7,
-5.39919118403805073710E-6,
-3.12458202168959833422E-5,
 8.90136741950727517826E-5,
 2.02558474743846862168E-3,
 2.96064440855633256972E-2,
 1.11847751047257036625E0
};

/* x exp(-x) shi(x), inverted interval 18 to 88 */
static double S2[] = {
-1.05311574154850938805E-17,
 2.62446095596355225821E-17,
 8.82090135625368160657E-17,
-3.38459811878103047136E-16,
-8.30608026366935789136E-16,
 3.93397875437050071776E-15,
 1.01765565969729044505E-14,
-4.21128170307640802703E-14,
-1.60818204519802480035E-13,
 3.34714954175994481761E-13,
 2.72600352129153073807E-12,
 1.66894954752839083608E-12,
-3.49278141024730899554E-11,
-1.58580661666482709598E-10,
-1.79289437183355633342E-10,
 1.76281629144264523277E-9,
 1.69050228879421288846E-8,
 1.25391771228487041649E-7,
 1.16229947068677338732E-6,
 1.61038260117376323993E-5,
 3.49810375601053973070E-4,
 1.28478065259647610779E-2,
 1.03665722588798326712E0
};
#endif

#ifdef DEC
static unsigned short S1[] = {
0022251,0115635,0165120,0006574,
0122734,0050751,0020305,0101356,
0023153,0111154,0011103,0177462,
0023636,0060321,0060253,0124246,
0124554,0106655,0152525,0166400,
0025205,0140145,0171006,0106556,
0125034,0056427,0004205,0176022,
0126305,0016731,0025011,0134453,
0027046,0172453,0112604,0116235,
0127216,0022071,0116600,0137667,
0130103,0115126,0071104,0052535,
0030672,0025450,0010071,0141414,
0130615,0165136,0132137,0177737,
0132031,0031611,0074436,0175407,
0032433,0077602,0104345,0060076,
0033121,0165741,0167177,0172433,
0133665,0025262,0174621,0022612,
0134403,0006761,0124566,0145405,
0034672,0126332,0034737,0116744,
0036004,0137654,0037332,0131766,
0036762,0104466,0121445,0124326,
0040217,0025105,0062145,0042640
};

static unsigned short S2[] = {
0122102,0041774,0016051,0055137,
0022362,0010125,0007651,0015773,
0022713,0062551,0040227,0071645,
0123303,0015732,0025731,0146570,
0123557,0064016,0002067,0067711,
0024215,0136214,0132374,0124234,
0024467,0051425,0071066,0064210,
0125075,0124305,0135123,0024170,
0125465,0010261,0005560,0034232,
0025674,0066602,0030724,0174557,
0026477,0151520,0051510,0067250,
0026352,0161076,0113154,0116271,
0127431,0116470,0177465,0127274,
0130056,0056174,0170315,0013321,
0130105,0020575,0075327,0036710,
0030762,0043625,0113046,0125035,
0031621,0033211,0154354,0022077,
0032406,0121555,0074270,0041141,
0033234,0000116,0041611,0173743,
0034207,0013263,0174715,0115563,
0035267,0063300,0175753,0117266,
0036522,0077633,0033255,0136200,
0040204,0130457,0014454,0166254
};
#endif

#ifdef IBMPC
static unsigned short S1[] = {
0x01b0,0xbd4a,0x3373,0x3c75,
0xb05e,0x2418,0x8a3d,0xbc9b,
0x7fe6,0x8248,0x724d,0x3cad,
0x7515,0x2c15,0xcc1a,0x3cd3,
0xbda0,0xbaaa,0x91b5,0xbd0d,
0xd1ae,0xbe40,0xb80c,0x3d30,
0xbf82,0xe110,0x8ba2,0xbd23,
0x3725,0x2541,0xa3bb,0xbd78,
0x9394,0x72b0,0xdea5,0x3da4,
0x17f7,0x33b0,0xc487,0xbdb1,
0x8aac,0xce48,0x734a,0xbde8,
0x3862,0x0207,0x4565,0x3e17,
0xfffc,0xd68b,0xbd4b,0xbe11,
0xdf61,0x2f23,0x2671,0xbe63,
0xac08,0x511c,0x6ff0,0x3e83,
0xfea3,0x3dcf,0x3d7c,0x3eaa,
0x24b1,0x5f32,0xa556,0xbed6,
0xd961,0x352e,0x61be,0xbf00,
0xf3bd,0x473b,0x559b,0x3f17,
0x567f,0x87db,0x97f5,0x3f60,
0xb51b,0xd464,0x5126,0x3f9e,
0xa8b4,0xac8c,0xe548,0x3ff1
};

static unsigned short S2[] = {
0x2b4c,0x8385,0x487f,0xbc68,
0x237f,0xa1f5,0x420a,0x3c7e,
0xee75,0x2812,0x6cad,0x3c99,
0x39af,0x457b,0x637b,0xbcb8,
0xedf9,0xc086,0xed01,0xbccd,
0x9513,0x969f,0xb791,0x3cf1,
0xcd11,0xae46,0xea62,0x3d06,
0x650f,0xb74a,0xb518,0xbd27,
0x0713,0x216e,0xa216,0xbd46,
0x9f2e,0x463a,0x8db0,0x3d57,
0x0dd5,0x0a69,0xfa6a,0x3d87,
0x9397,0xd2cd,0x5c47,0x3d7d,
0xb5d8,0x1fe6,0x33a7,0xbdc3,
0xa2da,0x9e19,0xcb8f,0xbde5,
0xe7b9,0xaf5a,0xa42f,0xbde8,
0xd544,0xb2c4,0x48f2,0x3e1e,
0x8488,0x3b1d,0x26d1,0x3e52,
0x084c,0xaf17,0xd46d,0x3e80,
0x3efc,0xc871,0x8009,0x3eb3,
0xb36e,0x7f39,0xe2d6,0x3ef0,
0x73d7,0x1f7d,0xecd8,0x3f36,
0xb790,0x66d5,0x4ff3,0x3f8a,
0x9d96,0xe325,0x9625,0x3ff0
};
#endif

#ifdef MIEEE
static unsigned short S1[] = {
0x3c75,0x3373,0xbd4a,0x01b0,
0xbc9b,0x8a3d,0x2418,0xb05e,
0x3cad,0x724d,0x8248,0x7fe6,
0x3cd3,0xcc1a,0x2c15,0x7515,
0xbd0d,0x91b5,0xbaaa,0xbda0,
0x3d30,0xb80c,0xbe40,0xd1ae,
0xbd23,0x8ba2,0xe110,0xbf82,
0xbd78,0xa3bb,0x2541,0x3725,
0x3da4,0xdea5,0x72b0,0x9394,
0xbdb1,0xc487,0x33b0,0x17f7,
0xbde8,0x734a,0xce48,0x8aac,
0x3e17,0x4565,0x0207,0x3862,
0xbe11,0xbd4b,0xd68b,0xfffc,
0xbe63,0x2671,0x2f23,0xdf61,
0x3e83,0x6ff0,0x511c,0xac08,
0x3eaa,0x3d7c,0x3dcf,0xfea3,
0xbed6,0xa556,0x5f32,0x24b1,
0xbf00,0x61be,0x352e,0xd961,
0x3f17,0x559b,0x473b,0xf3bd,
0x3f60,0x97f5,0x87db,0x567f,
0x3f9e,0x5126,0xd464,0xb51b,
0x3ff1,0xe548,0xac8c,0xa8b4
};

static unsigned short S2[] = {
0xbc68,0x487f,0x8385,0x2b4c,
0x3c7e,0x420a,0xa1f5,0x237f,
0x3c99,0x6cad,0x2812,0xee75,
0xbcb8,0x637b,0x457b,0x39af,
0xbccd,0xed01,0xc086,0xedf9,
0x3cf1,0xb791,0x969f,0x9513,
0x3d06,0xea62,0xae46,0xcd11,
0xbd27,0xb518,0xb74a,0x650f,
0xbd46,0xa216,0x216e,0x0713,
0x3d57,0x8db0,0x463a,0x9f2e,
0x3d87,0xfa6a,0x0a69,0x0dd5,
0x3d7d,0x5c47,0xd2cd,0x9397,
0xbdc3,0x33a7,0x1fe6,0xb5d8,
0xbde5,0xcb8f,0x9e19,0xa2da,
0xbde8,0xa42f,0xaf5a,0xe7b9,
0x3e1e,0x48f2,0xb2c4,0xd544,
0x3e52,0x26d1,0x3b1d,0x8488,
0x3e80,0xd46d,0xaf17,0x084c,
0x3eb3,0x8009,0xc871,0x3efc,
0x3ef0,0xe2d6,0x7f39,0xb36e,
0x3f36,0xecd8,0x1f7d,0x73d7,
0x3f8a,0x4ff3,0x66d5,0xb790,
0x3ff0,0x9625,0xe325,0x9d96
};
#endif


#ifdef UNK
/* x exp(-x) chin(x), inverted interval 8 to 18 */
static double C1[] = {
-8.12435385225864036372E-18,
 2.17586413290339214377E-17,
 5.22624394924072204667E-17,
-9.48812110591690559363E-16,
 5.35546311647465209166E-15,
-1.21009970113732918701E-14,
-6.00865178553447437951E-14,
 7.16339649156028587775E-13,
-2.93496072607599856104E-12,
-1.40359438136491256904E-12,
 8.76302288609054966081E-11,
-4.40092476213282340617E-10,
-1.87992075640569295479E-10,
 1.31458150989474594064E-8,
-4.75513930924765465590E-8,
-2.21775018801848880741E-7,
 1.94635531373272490962E-6,
 4.33505889257316408893E-6,
-6.13387001076494349496E-5,
-3.13085477492997465138E-4,
 4.97164789823116062801E-4,
 2.64347496031374526641E-2,
 1.11446150876699213025E0
};

/* x exp(-x) chin(x), inverted interval 18 to 88 */
static double C2[] = {
 8.06913408255155572081E-18,
-2.08074168180148170312E-17,
-5.98111329658272336816E-17,
 2.68533951085945765591E-16,
 4.52313941698904694774E-16,
-3.10734917335299464535E-15,
-4.42823207332531972288E-15,
 3.49639695410806959872E-14,
 6.63406731718911586609E-14,
-3.71902448093119218395E-13,
-1.27135418132338309016E-12,
 2.74851141935315395333E-12,
 2.33781843985453438400E-11,
 2.71436006377612442764E-11,
-2.56600180000355990529E-10,
-1.61021375163803438552E-9,
-4.72543064876271773512E-9,
-3.00095178028681682282E-9,
 7.79387474390914922337E-8,
 1.06942765566401507066E-6,
 1.59503164802313196374E-5,
 3.49592575153777996871E-4,
 1.28475387530065247392E-2,
 1.03665693917934275131E0
};
#endif

#ifdef DEC
static unsigned short C1[] = {
0122025,0157055,0021702,0021427,
0022310,0130043,0123265,0022340,
0022561,0002231,0017746,0013043,
0123610,0136375,0002352,0024467,
0024300,0171555,0141300,0000446,
0124531,0176777,0126210,0035616,
0125207,0046604,0167760,0077132,
0026111,0120666,0026606,0064143,
0126516,0103615,0054127,0005436,
0126305,0104721,0025415,0004134,
0027700,0131556,0164725,0157553,
0130361,0170602,0077274,0055406,
0130116,0131420,0125472,0017231,
0031541,0153747,0177312,0056304,
0132114,0035517,0041545,0043151,
0132556,0020415,0110044,0172442,
0033402,0117041,0031152,0010364,
0033621,0072737,0050647,0013720,
0134600,0121366,0140010,0063265,
0135244,0022637,0013756,0044742,
0035402,0052052,0006523,0043564,
0036730,0106660,0020277,0162146,
0040216,0123254,0135147,0005724
};

static unsigned short C2[] = {
0022024,0154550,0104311,0144257,
0122277,0165037,0133443,0155601,
0122611,0165102,0157053,0055252,
0023232,0146235,0153511,0113222,
0023402,0057340,0145304,0010471,
0124137,0164171,0113071,0100002,
0124237,0105473,0056130,0022022,
0025035,0073266,0056746,0164433,
0025225,0061313,0055600,0165407,
0125721,0056312,0107613,0051215,
0126262,0166534,0115336,0066653,
0026501,0064307,0127442,0065573,
0027315,0121375,0142020,0045356,
0027356,0140764,0070641,0046570,
0130215,0010503,0146335,0177737,
0130735,0047134,0015215,0163665,
0131242,0056523,0155276,0050053,
0131116,0034515,0050707,0163512,
0032247,0057507,0107545,0032007,
0033217,0104501,0021706,0025047,
0034205,0146413,0033746,0076562,
0035267,0044605,0065355,0002772,
0036522,0077173,0130716,0170304,
0040204,0130454,0130571,0027270
};
#endif

#ifdef IBMPC
static unsigned short C1[] = {
0x4463,0xa478,0xbbc5,0xbc62,
0xa49c,0x74d6,0x1604,0x3c79,
0xc2c4,0x23fc,0x2093,0x3c8e,
0x4527,0xa09d,0x179f,0xbcd1,
0x0025,0xb858,0x1e6d,0x3cf8,
0x0772,0xf591,0x3fbf,0xbd0b,
0x0fcb,0x9dfe,0xe9b0,0xbd30,
0xcd0c,0xc5b0,0x3436,0x3d69,
0xe164,0xab0a,0xd0f1,0xbd89,
0xa10c,0x2561,0xb13a,0xbd78,
0xbbed,0xdd3a,0x166d,0x3dd8,
0x8b61,0x4fd7,0x3e30,0xbdfe,
0x43d3,0x1567,0xd662,0xbde9,
0x4b98,0xffd9,0x3afc,0x3e4c,
0xa8cd,0xe86c,0x8769,0xbe69,
0x9ea4,0xb204,0xc421,0xbe8d,
0x421f,0x264d,0x53c4,0x3ec0,
0xe2fa,0xea34,0x2ebb,0x3ed2,
0x0cd7,0xd801,0x145e,0xbf10,
0xc93c,0xe2fd,0x84b3,0xbf34,
0x68ef,0x41aa,0x4a85,0x3f40,
0xfc8d,0x0417,0x11b6,0x3f9b,
0xe17b,0x974c,0xd4d5,0x3ff1
};

static unsigned short C2[] = {
0x3916,0x1119,0x9b2d,0x3c62,
0x7b70,0xf6e4,0xfd43,0xbc77,
0x6b55,0x5bc5,0x3d48,0xbc91,
0x32d2,0xbae9,0x5993,0x3cb3,
0x8227,0x1958,0x4bdc,0x3cc0,
0x3000,0x32c7,0xfd0f,0xbceb,
0x0482,0x6b8b,0xf167,0xbcf3,
0xdd23,0xcbbc,0xaed6,0x3d23,
0x1d61,0x6b70,0xac59,0x3d32,
0x6a52,0x51f1,0x2b99,0xbd5a,
0xcdb5,0x935b,0x5dab,0xbd76,
0x4d6f,0xf5e4,0x2d18,0x3d88,
0x095e,0xb882,0xb45f,0x3db9,
0x29af,0x8e34,0xd83e,0x3dbd,
0xbffc,0x799b,0xa228,0xbdf1,
0xbcf7,0x8351,0xa9cb,0xbe1b,
0xca05,0x7b57,0x4baa,0xbe34,
0xfce9,0xaa38,0xc729,0xbe29,
0xa681,0xf1ec,0xebe8,0x3e74,
0xc545,0x2478,0xf128,0x3eb1,
0xcfae,0x66fc,0xb9a1,0x3ef0,
0xa0bf,0xad5d,0xe930,0x3f36,
0xde19,0x7639,0x4fcf,0x3f8a,
0x25d7,0x962f,0x9625,0x3ff0
};
#endif

#ifdef MIEEE
static unsigned short C1[] = {
0xbc62,0xbbc5,0xa478,0x4463,
0x3c79,0x1604,0x74d6,0xa49c,
0x3c8e,0x2093,0x23fc,0xc2c4,
0xbcd1,0x179f,0xa09d,0x4527,
0x3cf8,0x1e6d,0xb858,0x0025,
0xbd0b,0x3fbf,0xf591,0x0772,
0xbd30,0xe9b0,0x9dfe,0x0fcb,
0x3d69,0x3436,0xc5b0,0xcd0c,
0xbd89,0xd0f1,0xab0a,0xe164,
0xbd78,0xb13a,0x2561,0xa10c,
0x3dd8,0x166d,0xdd3a,0xbbed,
0xbdfe,0x3e30,0x4fd7,0x8b61,
0xbde9,0xd662,0x1567,0x43d3,
0x3e4c,0x3afc,0xffd9,0x4b98,
0xbe69,0x8769,0xe86c,0xa8cd,
0xbe8d,0xc421,0xb204,0x9ea4,
0x3ec0,0x53c4,0x264d,0x421f,
0x3ed2,0x2ebb,0xea34,0xe2fa,
0xbf10,0x145e,0xd801,0x0cd7,
0xbf34,0x84b3,0xe2fd,0xc93c,
0x3f40,0x4a85,0x41aa,0x68ef,
0x3f9b,0x11b6,0x0417,0xfc8d,
0x3ff1,0xd4d5,0x974c,0xe17b
};

static unsigned short C2[] = {
0x3c62,0x9b2d,0x1119,0x3916,
0xbc77,0xfd43,0xf6e4,0x7b70,
0xbc91,0x3d48,0x5bc5,0x6b55,
0x3cb3,0x5993,0xbae9,0x32d2,
0x3cc0,0x4bdc,0x1958,0x8227,
0xbceb,0xfd0f,0x32c7,0x3000,
0xbcf3,0xf167,0x6b8b,0x0482,
0x3d23,0xaed6,0xcbbc,0xdd23,
0x3d32,0xac59,0x6b70,0x1d61,
0xbd5a,0x2b99,0x51f1,0x6a52,
0xbd76,0x5dab,0x935b,0xcdb5,
0x3d88,0x2d18,0xf5e4,0x4d6f,
0x3db9,0xb45f,0xb882,0x095e,
0x3dbd,0xd83e,0x8e34,0x29af,
0xbdf1,0xa228,0x799b,0xbffc,
0xbe1b,0xa9cb,0x8351,0xbcf7,
0xbe34,0x4baa,0x7b57,0xca05,
0xbe29,0xc729,0xaa38,0xfce9,
0x3e74,0xebe8,0xf1ec,0xa681,
0x3eb1,0xf128,0x2478,0xc545,
0x3ef0,0xb9a1,0x66fc,0xcfae,
0x3f36,0xe930,0xad5d,0xa0bf,
0x3f8a,0x4fcf,0x7639,0xde19,
0x3ff0,0x9625,0x962f,0x25d7
};
#endif



/* Sine and cosine integrals */

#define EUL 0.57721566490153286061
extern double MACHEP, MAXNUM, PIO2;

int shichi( x, si, ci )
double x;
double *si, *ci;
{
double k, z, c, s, a;
short sign;

if( x < 0.0 )
	{
	sign = -1;
	x = -x;
	}
else
	sign = 0;


if( x == 0.0 )
	{
	*si = 0.0;
	*ci = -MAXNUM;
	return( 0 );
	}

if( x >= 8.0 )
	goto chb;

z = x * x;

/*	Direct power series expansion	*/

a = 1.0;
s = 1.0;
c = 0.0;
k = 2.0;

do
	{
	a *= z/k;
	c += a/k;
	k += 1.0;
	a /= k;
	s += a/k;
	k += 1.0;
	}
while( fabs(a/s) > MACHEP );

s *= x;
goto done;


chb:

if( x < 18.0 )
	{
	a = (576.0/x - 52.0)/10.0;
	k = exp(x) / x;
	s = k * chbevl( a, S1, 22 );
	c = k * chbevl( a, C1, 23 );
	goto done;
	}

if( x <= 88.0 )
	{
	a = (6336.0/x - 212.0)/70.0;
	k = exp(x) / x;
	s = k * chbevl( a, S2, 23 );
	c = k * chbevl( a, C2, 24 );
	goto done;
	}
else
	{
	if( sign )
		*si = -MAXNUM;
	else
		*si = MAXNUM;
	*ci = MAXNUM;
	return(0);
	}
done:
if( sign )
	s = -s;

*si = s;

*ci = EUL + log(x) + c;
return(0);
}