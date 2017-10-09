/* based upon Snyder and Linck, USGS-NMD */
#define PJ_LIB__
#include <errno.h>
#include "projects.h"

PROJ_HEAD(mil_os, "Miller Oblated Stereographic") "\n\tAzi(mod)";
PROJ_HEAD(lee_os, "Lee Oblated Stereographic") "\n\tAzi(mod)";
PROJ_HEAD(gs48, "Mod. Stereographic of 48 U.S.") "\n\tAzi(mod)";
PROJ_HEAD(alsk, "Mod. Stereographic of Alaska") "\n\tAzi(mod)";
PROJ_HEAD(gs50, "Mod. Stereographic of 50 U.S.") "\n\tAzi(mod)";

#define EPSLN 1e-12

struct pj_opaque {
    COMPLEX *zcoeff; \
    double  cchio, schio; \
    int     n;
};


static XY e_forward (LP lp, PJ *P) {          /* Ellipsoidal, forward */
    XY xy = {0.0,0.0};
    struct pj_opaque *Q = P->opaque;
    double sinlon, coslon, esphi, chi, schi, cchi, s;
    COMPLEX p;

    sinlon = sin(lp.lam);
    coslon = cos(lp.lam);
    esphi = P->e * sin(lp.phi);
    chi = 2. * atan(tan((M_HALFPI + lp.phi) * .5) *
        pow((1. - esphi) / (1. + esphi), P->e * .5)) - M_HALFPI;
    schi = sin(chi);
    cchi = cos(chi);
    s = 2. / (1. + Q->schio * schi + Q->cchio * cchi * coslon);
    p.r = s * cchi * sinlon;
    p.i = s * (Q->cchio * schi - Q->schio * cchi * coslon);
    p = pj_zpoly1(p, Q->zcoeff, Q->n);
    xy.x = p.r;
    xy.y = p.i;

    return xy;
}


static LP e_inverse (XY xy, PJ *P) {          /* Ellipsoidal, inverse */
    LP lp = {0.0,0.0};
    struct pj_opaque *Q = P->opaque;
    int nn;
    COMPLEX p, fxy, fpxy, dp;
    double den, rh = 0.0, z, sinz = 0.0, cosz = 0.0, chi, phi = 0.0, esphi;

    p.r = xy.x;
    p.i = xy.y;
    for (nn = 20; nn ;--nn) {
        fxy = pj_zpolyd1(p, Q->zcoeff, Q->n, &fpxy);
        fxy.r -= xy.x;
        fxy.i -= xy.y;
        den = fpxy.r * fpxy.r + fpxy.i * fpxy.i;
        dp.r = -(fxy.r * fpxy.r + fxy.i * fpxy.i) / den;
        dp.i = -(fxy.i * fpxy.r - fxy.r * fpxy.i) / den;
        p.r += dp.r;
        p.i += dp.i;
        if ((fabs(dp.r) + fabs(dp.i)) <= EPSLN)
            break;
    }
    if (nn) {
        rh = hypot(p.r, p.i);
        z = 2. * atan(.5 * rh);
        sinz = sin(z);
        cosz = cos(z);
        lp.lam = P->lam0;
        if (fabs(rh) <= EPSLN) {
            /* if we end up here input coordinates were (0,0).
             * pj_inv() adds P->lam0 to lp.lam, this way we are
             * sure to get the correct offset */
            lp.lam = 0.0;
            lp.phi = P->phi0;
            return lp;
        }
        chi = aasin(P->ctx, cosz * Q->schio + p.i * sinz * Q->cchio / rh);
        phi = chi;
        for (nn = 20; nn ;--nn) {
            double dphi;
            esphi = P->e * sin(phi);
            dphi = 2. * atan(tan((M_HALFPI + chi) * .5) *
                pow((1. + esphi) / (1. - esphi), P->e * .5)) - M_HALFPI - phi;
            phi += dphi;
            if (fabs(dphi) <= EPSLN)
                break;
        }
    }
    if (nn) {
        lp.phi = phi;
        lp.lam = atan2(p.r * sinz, rh * Q->cchio * cosz - p.i *
            Q->schio * sinz);
    } else
        lp.lam = lp.phi = HUGE_VAL;
    return lp;
}


static PJ *setup(PJ *P) { /* general initialization */
    struct pj_opaque *Q = P->opaque;
    double esphi, chio;

    if (P->es != 0.0) {
        esphi = P->e * sin(P->phi0);
        chio = 2. * atan(tan((M_HALFPI + P->phi0) * .5) *
            pow((1. - esphi) / (1. + esphi), P->e * .5)) - M_HALFPI;
    } else
        chio = P->phi0;
    Q->schio = sin(chio);
    Q->cchio = cos(chio);
    P->inv = e_inverse;
    P->fwd = e_forward;

    return P;
}


/* Miller Oblated Stereographic */
PJ *PROJECTION(mil_os) {
    static COMPLEX AB[] = {
        {0.924500, 0.},
        {0.,       0.},
        {0.019430, 0.}
    };

    struct pj_opaque *Q = pj_calloc (1, sizeof (struct pj_opaque));
    if (0==Q)
        return pj_default_destructor (P, ENOMEM);
    P->opaque = Q;

    Q->n = 2;
    P->lam0 = DEG_TO_RAD * 20.;
    P->phi0 = DEG_TO_RAD * 18.;
    Q->zcoeff = AB;
    P->es = 0.;

    return setup(P);
}


/* Lee Oblated Stereographic */
PJ *PROJECTION(lee_os) {
    static COMPLEX AB[] = {
        {0.721316,    0.},
        {0.,          0.},
        {-0.0088162, -0.00617325}
    };

    struct pj_opaque *Q = pj_calloc (1, sizeof (struct pj_opaque));
    if (0==Q)
        return pj_default_destructor (P, ENOMEM);
    P->opaque = Q;

    Q->n = 2;
    P->lam0 = DEG_TO_RAD * -165.;
    P->phi0 = DEG_TO_RAD * -10.;
    Q->zcoeff = AB;
    P->es = 0.;

   return setup(P);
}


PJ *PROJECTION(gs48) {
    static COMPLEX /* 48 United States */
    AB[] = {
        {0.98879,   0.},
        {0.,        0.},
        {-0.050909, 0.},
        {0.,        0.},
        {0.075528,  0.}
    };

    struct pj_opaque *Q = pj_calloc (1, sizeof (struct pj_opaque));
    if (0==Q)
        return pj_default_destructor (P, ENOMEM);
    P->opaque = Q;

    Q->n = 4;
    P->lam0 = DEG_TO_RAD * -96.;
    P->phi0 = DEG_TO_RAD * 39.;
    Q->zcoeff = AB;
    P->es = 0.;
    P->a = 6370997.;

    return setup(P);
}


PJ *PROJECTION(alsk) {
    static COMPLEX  ABe[] = { /* Alaska ellipsoid */
        { .9945303, 0.},
        { .0052083, -.0027404},
        { .0072721,  .0048181},
        {-.0151089, -.1932526},
        { .0642675, -.1381226},
        { .3582802, -.2884586},
    };

    static COMPLEX ABs[] = { /* Alaska sphere */
        { .9972523, 0.},
        { .0052513, -.0041175},
        { .0074606,  .0048125},
        {-.0153783, -.1968253},
        { .0636871, -.1408027},
        { .3660976, -.2937382}
    };

    struct pj_opaque *Q = pj_calloc (1, sizeof (struct pj_opaque));
    if (0==Q)
        return pj_default_destructor (P, ENOMEM);
    P->opaque = Q;

    Q->n = 5;
    P->lam0 = DEG_TO_RAD * -152.;
    P->phi0 = DEG_TO_RAD * 64.;
    if (P->es != 0.0) { /* fixed ellipsoid/sphere */
        Q->zcoeff = ABe;
        P->a = 6378206.4;
        P->e = sqrt(P->es = 0.00676866);
    } else {
        Q->zcoeff = ABs;
        P->a = 6370997.;
    }

    return setup(P);
}


PJ *PROJECTION(gs50) {
    static COMPLEX  ABe[] = { /* GS50 ellipsoid */
        { .9827497, 0.},
        { .0210669,  .0053804},
        {-.1031415, -.0571664},
        {-.0323337, -.0322847},
        { .0502303,  .1211983},
        { .0251805,  .0895678},
        {-.0012315, -.1416121},
        { .0072202, -.1317091},
        {-.0194029,  .0759677},
        {-.0210072,  .0834037}
    };

    static COMPLEX ABs[] = { /* GS50 sphere */
        { .9842990, 0.},
        { .0211642,  .0037608},
        {-.1036018, -.0575102},
        {-.0329095, -.0320119},
        { .0499471,  .1223335},
        { .0260460,  .0899805},
        { .0007388, -.1435792},
        { .0075848, -.1334108},
        {-.0216473,  .0776645},
        {-.0225161,  .0853673}
    };

    struct pj_opaque *Q = pj_calloc (1, sizeof (struct pj_opaque));
    if (0==Q)
        return pj_default_destructor (P, ENOMEM);
    P->opaque = Q;

    Q->n = 9;
    P->lam0 = DEG_TO_RAD * -120.;
    P->phi0 = DEG_TO_RAD * 45.;
    if (P->es != 0.0) { /* fixed ellipsoid/sphere */
        Q->zcoeff = ABe;
        P->a = 6378206.4;
        P->e = sqrt(P->es = 0.00676866);
    } else {
        Q->zcoeff = ABs;
        P->a = 6370997.;
    }

    return setup(P);
}


#ifndef PJ_SELFTEST
int pj_mil_os_selftest (void) {return 0;}
#else

int pj_mil_os_selftest (void) {
    double tolerance_lp = 1e-10;
    double tolerance_xy = 1e-7;

    char s_args[] = {"+proj=mil_os   +R=6400000    +lat_1=0.5 +lat_2=2"};

    LP fwd_in[] = {
        { 2, 1},
        { 2,-1},
        {-2, 1},
        {-2,-1}
    };

    XY s_fwd_expect[] = {
        {-1908527.94959420455, -1726237.4730614475},
        {-1916673.02291848511, -1943133.88812552323},
        {-2344429.41208962305, -1706258.05121891224},
        {-2354637.83553299867, -1926468.60513541684},
    };

    XY inv_in[] = {
        { 200, 100},
        { 200,-100},
        {-200, 100},
        {-200,-100}
    };

    LP s_inv_expect[] = {
        {20.0020363939492398, 18.0009683469140498},
        {20.0020363715837419, 17.999031631815086},
        {19.9979636060507602, 18.0009683469140498},
        {19.9979636284162581, 17.999031631815086},
    };

    return pj_generic_selftest (0, s_args, tolerance_xy, tolerance_lp, 4, 4, fwd_in, 0, s_fwd_expect, inv_in, 0, s_inv_expect);
}

#endif


#ifndef PJ_SELFTEST
int pj_lee_os_selftest (void) {return 0;}
#else

int pj_lee_os_selftest (void) {
    double tolerance_lp = 1e-10;
    double tolerance_xy = 1e-7;

    char s_args[] = {"+proj=lee_os   +R=6400000    +lat_1=0.5 +lat_2=2"};

    LP fwd_in[] = {
        { 2, 1},
        { 2,-1},
        {-2, 1},
        {-2,-1}
    };

    XY s_fwd_expect[] = {
        {-25564478.9526050538, 154490848.8286255},
        { 30115393.9385746419, 125193997.439701974},
        {-31039340.5921660066,  57678685.0448915437},
        {-3088419.93942357088,  58150091.0991110131},
    };

    XY inv_in[] = {
        { 200, 100},
        { 200,-100},
        {-200, 100},
        {-200,-100}
    };

    LP s_inv_expect[] = {
        {-164.997479457813824,  -9.99875886103541411},
        {-164.997479438558884, -10.0012411200022751},
        {-165.002520542186289,  -9.99875886103545142},
        {-165.002520561440946, -10.0012411200022999},
    };

    return pj_generic_selftest (0, s_args, tolerance_xy, tolerance_lp, 4, 4, fwd_in, 0, s_fwd_expect, inv_in, 0, s_inv_expect);
}

#endif


#ifndef PJ_SELFTEST
int pj_gs48_selftest (void) {return 0;}
#else

int pj_gs48_selftest (void) {
    double tolerance_lp = 1e-12;
    double tolerance_xy = 1e-8;

    char s_args[] = {"+proj=gs48 +R=6370997"};

    /* All latitudes and longitudes within the continental US */
    LP fwd_in[] = {
        { -119.0, 40.0},
        {  -70.0, 64.0},
        {  -80.0, 25.0},
        {  -95.0, 35.0}
    };

    XY s_fwd_expect[] = {
        { -1923908.446529345820,   355874.658944479190},
        {  1354020.375109298155,  3040846.007866524626},
        {  1625139.160484319553, -1413614.894029108109},
        {    90241.658071457961,  -439595.048485902138},
    };

    XY inv_in[] = {
        { -1923000.0,   355000.0},
        {  1354000.0,  3040000.0},
        {  1625000.0, -1413000.0},
        {    90000.0,  -439000.0},
    };

    LP s_inv_expect[] = {
        {-118.987112613284, 39.994449789388},
        { -70.005208999424, 63.993387835525},
        { -80.000346610440, 25.005602546594},
        { -95.002606473071, 35.005424705030},
    };

    return pj_generic_selftest (0, s_args, tolerance_xy, tolerance_lp, 4, 4, fwd_in, 0, s_fwd_expect, inv_in, 0, s_inv_expect);
}

#endif


#ifndef PJ_SELFTEST
int pj_alsk_selftest (void) {return 0;}
#else

int pj_alsk_selftest (void) {

    /* The standard test points are way outside the definition area bounds, hence we relax tolerances */
    double tolerance_lp = 1e-12;
    double tolerance_xy = 1e-8;

    char e_args[] = {"+proj=alsk +ellps=clrk66"};
    char s_args[] = {"+proj=alsk +R=6370997"};

    LP fwd_in[] = {
        {-160.0, 55.0},
        {-160.0, 70.0},
        {-145.0, 70.0},
        {-145.0, 60.0}
    };

    XY e_fwd_expect[] = {
        {-513253.146950842060, -968928.031867943470},
        {-305001.133897637190,  687494.464958650530},
        {266454.305088600490,   683423.477493030950},
        {389141.322439243960,  -423913.251230396680},
    };

    XY s_fwd_expect[] = {
        {-511510.319410844070, -967150.991676078060},
        {-303744.771290368980,  685439.745941123230},
        {265354.974019662940,   681386.892874573010},
        {387711.995394026630,  -422980.685505462640},
    };

    XY inv_in[] = {
        {-500000.0, -950000.0},
        {-305000.0,  700000.0},
        { 250000.0,  700000.0},
        { 400000.0, -400000.0}
    };

    LP e_inv_expect[] = {
        {-159.830804302926, 55.183195262220},
        {-160.042203155537, 70.111086864056},
        {-145.381043551466, 70.163900908411},
        {-144.758985461448, 60.202929200739},
    };

    LP s_inv_expect[] = {
        {-159.854014457557, 55.165653849074},
        {-160.082332371601, 70.128307617632},
        {-145.347827407243, 70.181566919011},
        {-144.734239827146, 60.193564732505},
    };

    return pj_generic_selftest (e_args, s_args, tolerance_xy, tolerance_lp, 4, 4, fwd_in, e_fwd_expect, s_fwd_expect, inv_in, e_inv_expect, s_inv_expect);
}

#endif


#ifndef PJ_SELFTEST
int pj_gs50_selftest (void) {return 0;}
#else

int pj_gs50_selftest (void) {
    double tolerance_lp = 1e-12;
    double tolerance_xy = 1e-8;

    char e_args[] = {"+proj=gs50 +ellps=clrk66"};
    char s_args[] = {"+proj=gs50 +R=6370997"};

    LP fwd_in[] = {
        {-160.0, 65.0},
        {-130.0, 45.0},
        { -65.0, 45.0},
        { -80.0, 36.0},
    };

    XY e_fwd_expect[] = {
        {-1874628.5377402329,   2660907.942291015300},
        { -771831.51885333552,    48465.166491304852},
        { 4030931.8339815089,   1323687.864777399200},
        { 3450764.2615361013,   -175619.041820732440},
    };

    XY s_fwd_expect[] = {
        {-1867268.2534600089,   2656506.230401823300},
        { -769572.18967299373,    48324.312440863941},
        { 4019393.068680791200, 1320191.309350289200},
        { 3442685.615172345700, -178760.423489428680},
    };

    XY inv_in[] = {
        {-1800000.0, 2600000.0},
        { -800000.0,  500000.0},
        { 4000000.0, 1300000.0},
        { 3900000.0, -170000.0},
    };

    LP e_inv_expect[] = {
        {-157.989284999679, 64.851559609698},
        {-131.171390466814, 49.084969745967},
        { -65.491568685301, 44.992837923774},
        { -75.550660091101, 34.191114075743},
    };

    LP s_inv_expect[] = {
        {-158.163295044933, 64.854288364994},
        {-131.206816959506, 49.082915350974},
        { -65.348945220767, 44.957292681774},
        { -75.446820242089, 34.185406225616},
    };

    return pj_generic_selftest (e_args, s_args, tolerance_xy, tolerance_lp, 4, 4, fwd_in, e_fwd_expect, s_fwd_expect, inv_in, e_inv_expect, s_inv_expect);
}


#endif
