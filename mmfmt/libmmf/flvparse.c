/*****************************************************************************
 * Copyright 2016 Jeff <ggjogh@gmail.com>
 *****************************************************************************
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*****************************************************************************/

#include <assert.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>

#include "flvdef.h"
#include "flvparse.h"


int flvparse_arg_init (flvparse_opt_t *cfg, int argc, char *argv[])
{
    return 0;
}

int flvparse_arg_parse(flvparse_opt_t *cfg, int argc, char *argv[])
{
    return 0;
}

int flvparse_arg_check(flvparse_opt_t *cfg, int argc, char *argv[])
{
    if (!ios_open(cfg->ios, FLVPARSE_IOS_CNT, 0)) {
        ios_close(cfg->ios, FLVPARSE_IOS_CNT);
        return 1;
    }
    
    return 0;
}

int flvparse_arg_close(flvparse_opt_t *cfg)
{
    ios_close(cfg->ios, FLVPARSE_IOS_CNT);

    return 0;
}

int flvparse_arg_help()
{
    printf("flv parser. Options:\n");
    printf("\t -i|-src name<%%s> {...props...}\n");
    printf("\t ...frame range...   <%%d~%%d>\n");

    printf("\nset frame range as follow:\n");
    printf("\t [-f-range    <%%d~%%d>]\n");
    printf("\t [-f-start    <%%d>]\n");
    printf("\t [-frame|-f   <%%d>]\n");
    
    return 0;
}

int flv_parse(int argc, char **argv)
{
    int         r, i;
    flvparse_opt_t   cfg;

    memset(&cfg, 0, sizeof(cfg));
    flvparse_arg_init (&cfg, argc, argv);
    
    r = flvparse_arg_parse(&cfg, argc, argv);
    if (r) {
        if (r==1) {
            //help exit
            return 0;
        } else {
            // xerr("flvparse_arg_parse() failed\n");
            return 1;
        }
    }
    r = flvparse_arg_check(&cfg, argc, argv);
    if (r) {
        // xerr("flvparse_arg_check() failed\n");
        return 2;
    }
    
    /*************************************************************************
     *                          frame loop
     ************************************************************************/
    while (0) {
    } // end frame loop
    
    flvparse_arg_close(&cfg);

    return 0;
}
