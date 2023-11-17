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

#include "model_settings.h"
#include "quant_model.h"

#include "tflm.h"

namespace
{
// Declare all of the necessary variables: error_reporter, model, interpreter,
// input and output tensors.
tflite::ErrorReporter *error_reporter = nullptr;
const tflite::Model *model = nullptr;
tflite::MicroInterpreter *interpreter = nullptr;
TfLiteTensor *input = nullptr;
TfLiteTensor *output = nullptr;
int inference_count = 0;

// Set the size of the tensor arena - the tensor arena will vary depending on
// the model, but the arena size should be slightly above the minimum required
// to reduce the amount of memory allocated.
// There will be an error if the tensor arena size is too small.
constexpr int kTensorArenaSize = 100 * 1024;
alignas(16) uint8_t tensor_arena[kTensorArenaSize];
} // namespace

void tflm_setup() {
    tflite::InitializeTarget();

    // Declare the error_reporter.
    static tflite::MicroErrorReporter micro_error_reporter;
    error_reporter = &micro_error_reporter;

    // Load in the model.
    model = tflite::GetModel(QUANT_MODEL);
    if (model->version() != TFLITE_SCHEMA_VERSION)
    {
        TF_LITE_REPORT_ERROR(error_reporter,
                             "Model provided is schema version %d not equal "
                             "to supported version %d.",
                             model->version(),
                             TFLITE_SCHEMA_VERSION);
        return;
    }

    // Resolvers load in operations into the interpreter
    // that are used within the model. Typically, you will explicitly declare the layers
    // you use in the model. If you don't already know
    // the layers, you can use AllOpsResolver, with some codespace penalty.
    static tflite::AllOpsResolver resolver;

    // Build an interpreter to run the model with.
    static tflite::MicroInterpreter static_interpreter(
        model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
    interpreter = &static_interpreter;

    // Allocate memory from the tensor_arena for the model's tensors.
    TfLiteStatus allocate_status = interpreter->AllocateTensors();
    if (allocate_status != kTfLiteOk)
    {
        TF_LITE_REPORT_ERROR(error_reporter, "AllocateTensors() failed.");
        return;
    }

    // Obtain pointers to the model's input and output tensors.
    input = interpreter->input(0);

    // Check the settings match the model you have
    if (kNumRows != input->dims->data[1]) 
    {
        TF_LITE_REPORT_ERROR(error_reporter, "Number of rows expected: %d\nNumber of input rows given: %d", kNumRows, input->dims->data[1]);
        return;
    }

    if (kNumCols != input->dims->data[2]) 
    {
        TF_LITE_REPORT_ERROR(error_reporter, "Number of columns expected: %d\nNumber of input columns given: %d", kNumCols, input->dims->data[2]);
        return;
    }

    if (kNumChannels != input->dims->data[3]) 
    {
        TF_LITE_REPORT_ERROR(error_reporter, "Number of channels expected: %d\nNumber of input columns given :%d", kNumChannels, input->dims->data[3]);
        return;
    }

    if (kTfLiteInt8 != input->type) 
    {
        TF_LITE_REPORT_ERROR(error_reporter, "The input type is not int8.");
        return;
    }

    // Reset the inferences count every time you start the project.
    inference_count = 0;

    TF_LITE_REPORT_ERROR(error_reporter, "Completed setup");
}

// Produce prediction results based on the inferences from the model.
void prediction_results(int8_t *out, size_t *outlen) 
{
    // Resize the scores to from [-128, 127], to [0, 255] for better readability.
    const int RESIZE_CONSTANT = 128;
    int max_score = 0, max_index = 0;
    for (int i = 0; i < *outlen; ++i) 
    {
        int curr_score = out[i] + RESIZE_CONSTANT;
        if (max_score <= curr_score) 
        {
            max_index = i;
            max_score = curr_score;
        }
    }

    // Show predicted digits and raw categories. The output tensor format is dependent on
    // the model itself, so you must verify before running on the microcontroller.
    TF_LITE_REPORT_ERROR(error_reporter, "Predicted digit: %c\nScore: %d", kCategoryLabels[max_index], max_score);

    TF_LITE_REPORT_ERROR(error_reporter, "\x01\x01{");
    for (int i = 0; i < kCategoryCount; i++)
    {
        if (i < (kCategoryCount - 1))
        {
            TF_LITE_REPORT_ERROR(error_reporter, "    \"%c\": %d,", kCategoryLabels[i], out[i] + RESIZE_CONSTANT);
        }
        else
        {
            TF_LITE_REPORT_ERROR(error_reporter, "    \"%c\": %d", kCategoryLabels[i], out[i] + RESIZE_CONSTANT);
        }
    }
    TF_LITE_REPORT_ERROR(error_reporter, "}");
    TF_LITE_REPORT_ERROR(error_reporter, "\x02\x02");
}

void tflm_inference(uint8_t *in, size_t inlen, int8_t *out, size_t *outlen)
{
    // Check that the number of bytes coming from the camera is the same going into the model.
    if (inlen != input->bytes) 
    {
        TF_LITE_REPORT_ERROR(error_reporter, "The outgoing number of bytes from camera does not match incoming number of bytes in input tensor.");
        return;
    }

    // Copy the input from the camera into the input buffer of the model.
    memcpy(input->data.int8, in, inlen);

    // Invoke the interpreter.
    TfLiteStatus invoke_status = interpreter->Invoke();
    if (invoke_status != kTfLiteOk) 
    {
        TF_LITE_REPORT_ERROR(error_reporter, "Interpreter invoke failed.\n");
        return;
    }

    TF_LITE_REPORT_ERROR(error_reporter, "Completed inference %d\n", inference_count);

    output = interpreter->output(0);

    // Grab the output tensor and type cast it to int8_t.
    out = tflite::GetTensorData<int8_t>(output);
    *outlen = output->dims->data[1];

    if (output->dims->size != 2) 
    {
        TF_LITE_REPORT_ERROR(error_reporter, "Shape of the output tensor is incorrect.");
        return;
    }

    if (output->dims->data[0] != 1) 
    {
        TF_LITE_REPORT_ERROR(error_reporter, "More than one output tensor is being outputted.");
        return;
    }

    if (*outlen != kCategoryCount) 
    {
        TF_LITE_REPORT_ERROR(error_reporter, "Number of categories in output tensor: %d\nNumber of categories expected: %d", outlen, kCategoryCount);
        return;
    }

    if (output->type != kTfLiteInt8) 
    {
        TF_LITE_REPORT_ERROR(error_reporter, "Output type is not int8.");
        return;
    }

    prediction_results(out, outlen);

    inference_count++;
}