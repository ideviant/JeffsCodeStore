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

#ifndef __FLVPARSE_H__
#define __FLVPARSE_H__

enum flvparse_ios_ch {
    FLVPARSE_IOS_SRC = 0,
    FLVPARSE_IOS_DST = 1,
    FLVPARSE_IOS_CNT,
};

typedef struct _flvparse_opt {
    ios_t	ios[2];
    int         frame_range[2];
} flvparse_opt_t;

int flv_parse(int argc, char **argv);

#endif  // __FLVPARSE_H__
