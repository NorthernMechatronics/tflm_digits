/*
 * BSD 3-Clause License
 *
 * Copyright (c) 2022, Northern Mechatronics, Inc.
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
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"

#include "constants.h"
#include "model.h"
#include "svhn_model.h"
#include "output_handler.h"

#include "model_settings.h"
#include <cstdint>

#include "tflm.h"

namespace
{
tflite::ErrorReporter *error_reporter = nullptr;
const tflite::Model *model = nullptr;
tflite::MicroInterpreter *interpreter = nullptr;
TfLiteTensor *input = nullptr;
TfLiteTensor *output = nullptr;
int inference_count = 0;

constexpr int kTensorArenaSize = 210 * 1024;
alignas(16) uint8_t tensor_arena[kTensorArenaSize];
} // namespace

void tflm_setup() {
    tflite::InitializeTarget();
    static tflite::MicroErrorReporter micro_error_reporter;

    error_reporter = &micro_error_reporter;
    model = tflite::GetModel(tf_quant_model_july_13_2_tflite);
    if (model->version() != TFLITE_SCHEMA_VERSION)
    {
        TF_LITE_REPORT_ERROR(error_reporter,
                             "Model provided is schema version %d not equal "
                             "to supported version %d.",
                             model->version(),
                             TFLITE_SCHEMA_VERSION);
    }

    static tflite::AllOpsResolver resolver;

    // Build an interpreter to run the model with.
    static tflite::MicroInterpreter static_interpreter(
        model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
    interpreter = &static_interpreter;

    // Allocate memory from the tensor_arena for the model's tensors.
    TfLiteStatus allocate_status = interpreter->AllocateTensors();
    if (allocate_status != kTfLiteOk)
    {
        TF_LITE_REPORT_ERROR(error_reporter, "AllocateTensors() failed");
        return;
    }

    // Obtain pointers to the model's input and output tensors.
    input = interpreter->input(0);
    output = interpreter->output(0);

    TF_LITE_REPORT_ERROR(error_reporter, "The input type was %s", TfLiteTypeGetName(input->type));
    TF_LITE_REPORT_ERROR(error_reporter, "Size: %d", input->dims->size);
    TF_LITE_REPORT_ERROR(error_reporter, "Type of tensor: %s", TfLiteTypeGetName(input->type));
    TF_LITE_REPORT_ERROR(error_reporter, "First shape: %d", input->dims->data[0]);
    TF_LITE_REPORT_ERROR(error_reporter, "Number of rows: %d", input->dims->data[1]);
    TF_LITE_REPORT_ERROR(error_reporter, "Number of columns: %d", input->dims->data[2]);
    TF_LITE_REPORT_ERROR(error_reporter, "Number of channels: %d", input->dims->data[3]);

    // Keep track of how many inferences we have performed.
    inference_count = 0;

    TF_LITE_REPORT_ERROR(error_reporter, "Completed setup");
}

void prediction_results(uint8_t *out, size_t *outlen) {
    int max_score = 0;
    int max_index = 0;
    for (int i = 0; i < kCategoryCount; ++i) {
        if (max_score < out[i] + 128) {
            max_index = i;
            max_score = out[i] + 128;
        }
    }

    TF_LITE_REPORT_ERROR(error_reporter, "Predicted digit: %c\nScore: %d", kCategoryLabels[max_index], max_score);
    TF_LITE_REPORT_ERROR(error_reporter, "Raw categories: [1 2 3 4 5 6 7 8 9 0]");
    TF_LITE_REPORT_ERROR(error_reporter, "Raw scores: [%d %d %d %d %d %d %d %d %d %d]", 
        out[0] + 128, out[1] + 128, out[2] + 128, out[3] + 128, 
        out[4] + 128, out[5] + 128, out[6] + 128, out[7] + 128, 
        out[8] + 128, out[9] + 128);
}

void tflm_inference(uint8_t *in, size_t inlen, uint8_t *out, size_t *outlen)
{
    memcpy(input->data.int8, in, inlen);

    TfLiteStatus invoke_status = interpreter->Invoke();
    if (invoke_status != kTfLiteOk) {
        TF_LITE_REPORT_ERROR(error_reporter, "Invoke failed\n");
    }
    output = interpreter->output(0);

    out = tflite::GetTensorData<uint8_t>(output);
    *outlen = output->dims->data[1];

    TF_LITE_REPORT_ERROR(error_reporter, "Completed inference");

    prediction_results(out, outlen);

}