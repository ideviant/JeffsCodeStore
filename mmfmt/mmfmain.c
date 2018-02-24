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

#include <stdio.h>
#include <string.h>
#include "sim_opt.h"
#include "flvparse.h"

int main(int argc, char **argv)
{
    int i=0, j = 0;
    int exit_code = 0;
    
    typedef struct sub_module {
        const char *name; 
        int (*func)(int argc, char **argv);
        const char *help;
    } sub_module_t;
    
    const static sub_module_t sub_main[] = {
        {"flvps",   flv_parse,  "flv_parser"},
    };

    xlog_init(SLOG_PRINT);
    i = arg_parse_xlevel(1, argc, argv);

    if (i>=argc) {
        printf("No module specified. ");
    } else {
        for (j=0; j<ARRAY_SIZE(sub_main); ++j) {
            if (0==strcmp(argv[i], sub_main[j].name)) {
                return sub_main[j].func(argc-i, argv+i);
            }
        }
        if (strcmp(argv[i], "-h") && strcmp(argv[i], "--help")) {
            printf("`%s` is not support. ", argv[i]);
        }
    }
    
    printf("Use the following modules:\n");
    for (j=0; j<ARRAY_SIZE(sub_main); ++j) {
        printf("\t%4s - %s\n", sub_main[j].name, sub_main[j].help);
    }
    return exit_code;
    
    return 0;
}
