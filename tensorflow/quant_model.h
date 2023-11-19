/*
 *  BSD 3-Clause License
 *
 * Copyright (c) 2023, Northern Mechatronics, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef _QUANT_MODE_H_
#define _QUANT_MODE_H_

#if defined(MODEL_SIZE_SMALL)
    #include "quant_model_small.h"
    #define QUANT_MODEL         quant_model_small
    #define QUANT_MODEL_LEN     quant_model_small_len
#elif defined(MODEL_SIZE_MEDIUM)
    #include "quant_model_medium.h"
    #define QUANT_MODEL         quant_model_medium
    #define QUANT_MODEL_LEN     quant_model_medium_len
#elif defined(MODEL_SIZE_LARGE)
    #include "quant_model_large.h"
    #define QUANT_MODEL         quant_model_large
    #define QUANT_MODEL_LEN     quant_model_large_len
#elif defined(MODEL_OPT)
    #include "quant_model_opt.h"
    #define QUANT_MODEL         quant_model_opt
    #define QUANT_MODEL_LEN     quant_model_opt_len
#else
    #error  "Model size not specified"
#endif

#endif