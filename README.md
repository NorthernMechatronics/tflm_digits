# Tensorflow Lite for Microcontrollers documentation

This exmaple shows how to run Tensorflow Lite for Microcontrollers (TFLM) to take a picture from a camera, and run inferences on neural networks with varying sizes. The goal is to detect a single digit from a given image.

## Table of Contents

- [Converting the model](#converting-the-model)
- [Model settings](#model-settings)
- [Running inferences on the model](#running-inferences-on-the-model)
  - [Discussions on importing operations for a resolver](#discussions-on-importing-operations-for-a-resolver)
  - [How to use Netron](#how-to-use-netron)
- [Possible errors](#possible-errors)

## Converting the model

If you have converted the trained model to a quantized version, you must convert the model to a hexadecimal character array. On Linux and Mac, this is done through the following:

```
xxd -i name_of_model.tflite > name_of_model.cpp
```

To ensure proper alignment when running the model, you must add `alignas(8)` at the start of the definition.

The .cpp file contains two variables, one containing the hexadecimal bytes of the model and the other one containing the length of the array (or the size of the model). Once created, you must add a header file with the same name and export their declarations. There are examples on how to do this with `quant_model_small.h`, `quant_model_medium.h`, `quant_model_large.h`

## Model settings

See `model_settings.cc` and `model_settings.h` for more details.

Any model imported will have different settings (i.e., different input and output tensors) depending on their use case. In order to prevent errors during
runtime, you **must** explicitly declare the input and output dimensions, along with the output format.

To figure out the correct dimensions and format, you must return back to your file from which you trained the model and check the interpreter's dimensions. Alternatively, you can upload your file to this [Colab file](https://colab.research.google.com/drive/1pdGA1Bw2lMcB66HFVWosK0YfwB1oaRLl#scrollTo=a-seiDlzCKpr), and determine the details from there. Follow the instructions in the file.

![Picture of the input and output dimensions and data types for a given model](/images/input_output_details.png?raw=true 'Input and Output details')

## Running inferences on the model

See `tflm.cc` for more details. We will tweak some of the instructions found within [TFLM's official documentation](https://www.tensorflow.org/lite/microcontrollers/get_started_low_level).

TFLM requires the following libraries:

```
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"
```

TFLM offers two ways to import the resolver: importing specific operations required for the model, and importing all operations. For the former, import the library through `#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"`, and for the latter, `#include "tensorflow/lite/micro/all_ops_resolver.h"`. More discussion found [here](#discussions-on-importing-operations-for-a-resolver) on which solution to choose.

1. Import the model.
   Include the header file after importing the libraries stated above.

2. Set up logging.
   Similar to printf in C, we set up logging. Declare the error reporter in the `tflm_setup` function.
   ```
    static tflite::MicroErrorReporter micro_error_reporter;
    error_reporter = &micro_error_reporter;
   ```
3. Load the model into the microcontroller.
   The model is currently in a char array within the model imported in Step 1. Verify that the model's version is compatible with the schema.

   ```
    model = tflite::GetModel(quant_model_medium);
    if (model->version() != TFLITE_SCHEMA_VERSION)
    {
        TF_LITE_REPORT_ERROR(error_reporter,
                             "Model provided is schema version %d not equal "
                             "to supported version %d.",
                             model->version(),
                             TFLITE_SCHEMA_VERSION);
        return;
    }
   ```

4. Instantiate operations within the resolver. See [here](#discussions-on-importing-operations-for-a-resolver).

5. Allocate memory for the input, output and intermediate arrays using a uint8_t array.
   There is no set size required for the tensor arena size. TFLM will report errors if the tensor arena is too small. We recommend allocating
   based on a kilobyte (1024 bytes).

   ```
   static tflite::MicroInterpreter static_interpreter(
        model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
   ```

6. Instantiate the interpreter.
   Taking all the variables crated from Step 1-5, pass in the variables into the interpreter constructor.

7. Allocate memory from the tensor arena to the model's tensors.

   ```
   TfLiteStatus allocate_status = interpreter->AllocateTensors();
   ```

8. Validate input shape.
   Obtain the pointer to the input tensor. We perform error checking on the input based on the model settings declared.

   ```
   input = interpreter->input(0);
   ```

9. Copy the data from the camera into the model.
   Using memcpy, we copy the image data from the input buffer of the program to the input tensor. We also perform error checking on this as well.

   ```
   memcpy(input->data.int8, in, inlen);
   ```

10. Run the model.
    Invoke the interpreter through calling the `invoke()` function on the MicroInterpreter instance.

```
TfLiteStatus invoke_status = interpreter->Invoke();
```

11. Obtain output and print out predictions.
    Access the output directly from the interpreter, and store the tensor data into the output buffer. Afterwards, you will print out the results coming from the model. The way predictions are returned vary from model to model, but it is imporant to know the format to accurately represent
    the inferences. You should define these details in some way in `model_settings.cc` and `model_settings.h`, and provide a readable format when
    providing the predictions.

### Discussions on importing operations for a resolver

Resolvers define operations that the interpreter needs to access in order to run the model. At the time of writing, there are 71 operations allowed within TFLM.
We do not recommend importing all operations due to high memory usage, and importing multiple unused operations is generally unacceptable.
There are cases when this is acceptable: for instance, when the model's layers are not explicitly defined, or during development (i.e., experimentation purposes).

If you add `#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"`, you must understand which layers are required within your graph. You must either return back to the environment from which you built the model, or use a third-party service that explicitly outlines the layers for you, such as Netron. We have another discussion [here](#how-to-use-netron).

To instantiate the resolver, you must first declare the resolver using `tflite::MicroMutableOpResolver<number_of_ops>`
class. Then, fill in the number of operations you will use under `number_of_ops`, and invoke the functions for each operaion you will use. For instance:

```
static tflite::MicroMutableOpResolver<10> resolver;
resolver.AddConv2D();
resolver.AddMaxPool2D();
resolver.AddFullyConnected();
resolver.AddSoftmax();
resolver.AddRelu();
resolver.AddReshape();
resolver.AddAdd();
resolver.AddMul();
resolver.AddQuantize();
```

If you choose to use an operation either not currently within AllOpsResolver -- that is, it exists in Tensorflow Core but not in Tensorflow Lite, you must first verify whether [Tensorflow Lite already supports it](https://www.tensorflow.org/mlir/tfl_ops). If it exists, you need to quantize the model with that given operation after training, and explicitly declare it using tflite::MicroMutableOpResolver AddCustom(). If you need to create a custom operator, you will need to follow the steps in [building and testing your own](https://www.tensorflow.org/lite/guide/ops_compatibility) before importing it into the model. [Tensorflow Lite does not recommend building your own operations](https://www.tensorflow.org/lite/guide/ops_compatibility) unless needed for performance and size reasons.

If you add `#include "tensorflow/lite/micro/all_ops_resolver.h"`, you will declare the all_ops_resolver before declaring the interpreter:
`static tflite::AllOpsResolver resolver;`.

### How to use Netron

[Netron](www.netron.app) is a open-source project that allows you to view the different layers and operations of a model. To use the service, upload your generated model (whether quantized or not).

![Picture of Netron's app](/images/netron_details.png)

When you review the layers, you must include all types of operations the model uses. At the time of writing, this includes the following:

- Normal operations such as multiplication and addition
- Pooling layers
- Convolutional layers
- Activation functions
- Reshaping functions

## Possible errors
