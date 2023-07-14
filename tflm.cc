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

#include "test_images/Number_0_Light.h"
#include "test_images/Number_1_Light.h"
#include "test_images/Number_2_Light.h"
#include "test_images/Number_3_Light.h"
#include "test_images/Number_4_Light.h"
#include "test_images/Number_5_Light.h"
#include "test_images/Number_6_Light.h"
#include "test_images/Number_7_Light.h"
#include "test_images/Number_8_Light.h"
#include "test_images/Number_9_Light.h"



#include "tflm.h"

namespace
{
tflite::ErrorReporter *error_reporter = nullptr;
const tflite::Model *model = nullptr;
tflite::MicroInterpreter *interpreter = nullptr;
TfLiteTensor *input = nullptr;
TfLiteTensor *output = nullptr;
int inference_count = 0;

constexpr int kTensorArenaSize = 136 * 1024;
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


void tflm_loop() {
    memcpy(input->data.int8, Number_3_Light, input->bytes);
    TfLiteStatus invoke_status = interpreter->Invoke();
    if (invoke_status != kTfLiteOk) {
        TF_LITE_REPORT_ERROR(error_reporter, "Invoke failed\n");
    }
    output = interpreter->output(0);

    TF_LITE_REPORT_ERROR(error_reporter, "Size of output tensor: %d", output->dims->size);
    TF_LITE_REPORT_ERROR(error_reporter, "Shape: %d", output->dims->data[0]);
    TF_LITE_REPORT_ERROR(error_reporter, "Number of categories: %d", output->dims->data[1]);
    TF_LITE_REPORT_ERROR(error_reporter, "Completed inference");


    TF_LITE_REPORT_ERROR(error_reporter, "Zero score: %d", output->data.int8[kZeroIndex]);
    TF_LITE_REPORT_ERROR(error_reporter, "One score: %d", output->data.int8[kOneIndex]);
    TF_LITE_REPORT_ERROR(error_reporter, "Two score: %d", output->data.int8[kTwoIndex]);
    TF_LITE_REPORT_ERROR(error_reporter, "Three score: %d", output->data.int8[kThreeIndex]);
    TF_LITE_REPORT_ERROR(error_reporter, "Four score: %d", output->data.int8[kFourIndex]);
    TF_LITE_REPORT_ERROR(error_reporter, "Five score: %d", output->data.int8[kFiveIndex]);
    TF_LITE_REPORT_ERROR(error_reporter, "Six score: %d", output->data.int8[kSixIndex]);
    TF_LITE_REPORT_ERROR(error_reporter, "Seven score: %d", output->data.int8[kSevenIndex]);
    TF_LITE_REPORT_ERROR(error_reporter, "Eight score: %d", output->data.int8[kEightIndex]);
    TF_LITE_REPORT_ERROR(error_reporter, "Nine score: %d", output->data.int8[kNineIndex]);
}