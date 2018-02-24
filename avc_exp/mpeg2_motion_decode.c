/*!
******************************************************************************
\author
    jogh<163jogh@163.com>
\copyright
    2013, reserved
\brief
    MPEG2 IntraDC VLC & InterMV VLC
******************************************************************************
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sim_opt.h"

typedef struct fcode_opt {
    int fcode;
    int mcode;      // motion_code
    int mres;       // motion_residual
}fcode_opt_t;

#define FCODE_OPT_M(member) offsetof(fcode_opt_t, member)
cmdl_opt_t fcode_opt[] = 
{
    { 0, "h,help",              0, cmdl_parse_help,         0,      0,  },
    { 0, "xl,xlevel,xall,xnon", 0, cmdl_parse_xlevel,       0,      0,     "config log level"},
    { 0, "fcode",   1, cmdl_parse_int,  FCODE_OPT_M(fcode), "6",    "fcode"},
    { 1, "mcode",   1, cmdl_parse_int,  FCODE_OPT_M(mcode), 0,      "motion_code"},
    { 1, "mres",    1, cmdl_parse_int,  FCODE_OPT_M(mres ), 0,      "motion_residual"},
};
const int n_fcode_opt = ARRAY_SIZE(fcode_opt);

//===========================================================================
// decompose mdiff to mcode,mres according fcode
// @param fcode: see <ISO/IEC 13818-2: 1995 (E)>
// @param mdiff: motion_different, mdiff = mv - mv_pred
// @see mpeg2_motion_decode()
//===========================================================================
void mpeg2_motion_encode(int fcode, int mdiff)
{
    int mcode, mres;
}

//===========================================================================
// motion vector decoding
// @param  mcode:  motion_code
// @param  mres:   motion_residual
// @return mdiff:  motion_different
// @see <ISO/IEC 13818-2: 1995 (E), 7.6.3.1 Decoding the motion vectors>
//===========================================================================
int mpeg2_motion_decode(int mcode, int mres, int fcode)
{
    int mdiff;

    if ( fcode==1 || mcode==0 )
        mdiff = mcode ;
    else
    {
        int r_size = fcode - 1;
        mdiff = (abs(mcode) - 1) << r_size;
        mdiff += mres + 1;
        if (mcode < 0)
            mdiff = - mdiff;
    }

    return mdiff;
}

int mpeg2_motion_decode_test( fcode_opt_t *cfg )
{
    int fcode = cfg->fcode;
    int mcode = cfg->mcode; 
    int mres  = cfg->mres; 
    int mdiff = 0;
    char buf[256] = {0};
    int r_size = fcode - 1;

    if(mres<0 || mres > (1<<r_size)-1 )
    {
        printf("mres exceed %d bits\n", r_size);
        return 1;
    }

    mdiff = mpeg2_motion_decode(mcode, mres, fcode);
    printf("mpeg2_motion_decode_test with: \n\t fcode = %d\n", fcode);
    printf("\t mcode  : %6d %10s\n", mcode, itoa(mcode, buf, 2) );
    printf("\t mres   : %6d %10s\n", mres , itoa(mres , buf, 2) );
    printf("\t mdiff  : %6d %10s\n", mdiff, itoa(mdiff, buf, 2) );
    printf("\n");
    
    int mdiffDEC = (int)( (unsigned int)(mdiff) - 1 );
    int mcodeINC = (int)( (unsigned int)(mcode) + 1 );
    int mcodeDEC = (int)( (unsigned int)(mcode) - 1 );
    printf("\t mdiff--: %6d %10s\n", mdiffDEC, itoa(mdiffDEC, buf, 2) );
    printf("\t mcode++: %6d %10s\n", mcodeINC, itoa(mcodeINC, buf, 2) );
    printf("\t mcode++: %6d %10s\n", mcodeDEC, itoa(mcodeDEC, buf, 2) );
    
    return 0;
}

int fcode_decode(int argc, char** argv)
{
    fcode_opt_t   cfg;
    memset(&cfg, 0, sizeof(cfg));
    cmdl_iter_t iter = cmdl_iter_init(argc, argv, 0);
    int r = cmdl_parse(&iter, &cfg, n_fcode_opt, fcode_opt);
    if (r == CMDL_RET_HELP) {
        return cmdl_help(&iter, 0, n_fcode_opt, fcode_opt);
    } else if (r < 0) {
        xerr("cmdl_parse() failed, ret=%d\n", r);
        return 1;
    }
    
    return mpeg2_motion_decode_test(&cfg);
}
