/*****************************************************************************
 * Copyright 2014 Jeff <ggjogh@gmail.com>
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

#include <string.h>
#include "sim_opt.h"
#include "avc_exp.h"


int main(int argc, char **argv)
{
    int i=0, j = 0;
    int exit_code = 0;
    
    typedef struct test_module {
        const char *name; 
        int (*func)(int argc, char **argv);
        const char *help;
    } test_module_t;
    
    const static test_module_t sub_main[] = {
        {"cabac",     cabac_test,       "ff_h264_init_cabac_states()"},
        {"idct4",     idct4_dyn,        
         "show dynamic range (max value) in each step of the h264 4x4-DCT"},
    };

    xlog_init(SLOG_PRINT);
    i = arg_parse_xlevel(1, argc, argv);

    if (i>=argc) {
        xprint("No module specified. ");
    } else {
        for (j=0; j<ARRAY_SIZE(sub_main); ++j) {
            if (0==strcmp(argv[i], sub_main[j].name)) {
                return sub_main[j].func(argc-i, argv+i);
            }
        }
        if (strcmp(argv[i], "-h") && strcmp(argv[i], "--help")) {
            xprint("`%s` is not support. ", argv[i]);
        }
    }
    
    xprint("Use the following modules:\n");
    for (j=0; j<ARRAY_SIZE(sub_main); ++j) {
        xprint("\t%4s - %s\n", sub_main[j].name, sub_main[j].help);
    }
    
    return exit_code;
}