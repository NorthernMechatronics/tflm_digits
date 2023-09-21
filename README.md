# Tensorflow Lite for Microcontrollers documentation

This example shows how to run [Tensorflow Lite for Microcontrollers (TFLM)](https://www.tensorflow.org/lite/microcontrollers) to take a picture from a camera, and run inferences on neural networks with varying sizes. The goal is to detect a single digit from a given image.

Watch on youtube: https://youtu.be/s95ANzzGLiE

## Table of Contents

- [Tensorflow Lite for Microcontrollers documentation](#tensorflow-lite-for-microcontrollers-documentation)
  - [Table of Contents](#table-of-contents)
  - [Training the model](#training-the-model)
  - [Converting the model](#converting-the-model)
  - [Setting up the module with the camera](#setting-up-the-module-with-the-camera)
  - [Model settings](#model-settings)
  - [Running inferences on the model](#running-inferences-on-the-model)
    - [Discussions on importing operations for a resolver](#discussions-on-importing-operations-for-a-resolver)
    - [How to use Netron](#how-to-use-netron)
  - [Running the build](#running-the-build)
  - [Possible errors related to running the build](#possible-errors-related-to-running-the-build)
    - ["Sorry, could not find a PTY" or "Cannot open line... for R/W: Resource busy" from MacOS terminal](#sorry-could-not-find-a-pty-or-cannot-open-line-for-rw-resource-busy-from-macos-terminal)
  - [Possible errors directly related to TFLM](#possible-errors-directly-related-to-tflm)
    - [AllocateTensors() failed](#allocatetensors-failed)
    - [Poor alignment](#poor-alignment)
    - [Declaring arrays in the inference function](#declaring-arrays-in-the-inference-function)
    - [Improper input and output tensor types](#improper-input-and-output-tensor-types)
    - [Improper input and output tensor shapes](#improper-input-and-output-tensor-shapes)
    - [Predictions are not printing out properly](#predictions-are-not-printing-out-properly)

## Training the model

See [here](training_files) for more information.

## Converting the model

If you have converted the trained model to a quantized version, you must convert the model to a hexadecimal character array. On Linux and Mac, this is done through the following:

```
xxd -i name_of_model.tflite > name_of_model.cpp
```

To ensure proper alignment when running the model, you must add `alignas(8)` at the start of the definition.

The .cpp file contains two variables, one containing the hexadecimal bytes of the model and the other one containing the length of the array (or the size of the model). Once created, you must add a header file with the same name and export their declarations. There are examples on how to do this with `quant_model_small.h`, `quant_model_medium.h`, `quant_model_large.h`

## Setting up the module with the camera

The camera used alongside the NM180100 module was the [Arducam Mega 3MP SPI Camera](https://www.arducam.com/camera-for-any-microcontroller).

There is a [whole host of documentation](https://docs.arducam.com/Arduino-SPI-camera/MEGA-SPI/MEGA-SPI-Camera/#common-specifications) online along with the specifications and [diagram](https://www.arducam.com/product/presale-mega-3mp-color-rolling-shutter-camera-module-with-solid-camera-case-for-any-microcontroller/) for the camera.

Pay attention to where you connect each pin to the evaluation board (EVB). In **bsp_pins.src**, located within the bsp/nm180100evb folder, the following connections to the EVB are mentioned:

- VCC (power source) connects to either 5V or 3.3V
- GND (ground) connects to any ground pin named GND
- SCK (clock signal) connects to pin 5
- MISO (master in, slave out) connects to pin 6
- MOSI (master out, slave in) connects to pin 7
- CS (chip select) connects to pin 11

## Model settings

See `model_settings.cc` and `model_settings.h` for more details.

Any model imported will have different settings (i.e., different input and output tensors) depending on their use case. In order to prevent errors during runtime, you **must** explicitly declare the input and output dimensions, along with the output format.

To figure out the correct dimensions and format, you must return back to your file from which you trained the model and check the interpreter's dimensions. Alternatively, you can upload your file to this [Colab file](https://colab.research.google.com/drive/1pdGA1Bw2lMcB66HFVWosK0YfwB1oaRLl#scrollTo=a-seiDlzCKpr), or run the model_details.ipynb file located in this folder and determine the details from there.

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

For the purposes below, steps 1-8 will be done within the **tflm_setup()** function, whereas steps 9-11 will be done within the **tflm_inference()** function. This is because once the model has been loaded in the interpreter, the interpreter remains static between inferences until debugging ceases.

1. Import the model.
   Include the header file after importing the libraries stated above. In addition, you must include the source file in the list of target sources under `CMakeLists.txt`. Assuming the path is the main folder, you can simply write the source file as `name_of_model.cc`. For example:

   ```
   ...
   target_sources(
    ${APPLICATION}
    PRIVATE
    ${TARGET_SRC}
    main.c
    application_task_cli.c
    application_task.c
    button_task.c
    camera_task.c
    camera_task_cli.c
    console_task.c

    drivers/arducam/ArducamAmbiqHAL.c
    drivers/arducam/ArducamCamera.c
    drivers/arducam/ArducamLink.c
    drivers/arducam/ArducamUart.c

    constants.cc
    model_settings.cc
    /* Add your model source file here */
    output_handler.cc
    tflm.cc

    utils/RTT/RTT/SEGGER_RTT.c
    utils/RTT/RTT/SEGGER_RTT_printf.c
   )
   ...
   ```

   In tflm.cc:

   ```
   #include "model_settings.h"
   #include "quant_model_medium.h"
   ```

2. Set up logging.
   Similar to printf in C, we set up logging. Declare the error reporter in the `tflm_setup` function.

   ```
    static tflite::MicroErrorReporter micro_error_reporter;
    error_reporter = &micro_error_reporter;
   ```

3. Load the model into the microcontroller and initialize the target.
   The model is currently in a char array within the model imported in Step 1. Verify that the model's version is compatible with the schema.

   ```
    tflite::InitializeTarget();

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
   There is no set size required for the tensor arena size. TFLM will report errors if the tensor arena is too small. We recommend allocating the tensor arena based on the kilobyte (1024 bytes).

   ```
   constexpr int kTensorArenaSize = 300 * 1024;
   alignas(16) uint8_t tensor_arena[kTensorArenaSize];
   ```

6. Instantiate the interpreter.
   Taking all the variables crated from Step 1-5, pass in the variables into the interpreter constructor.

   ```
   static tflite::MicroInterpreter static_interpreter(
        model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
   ```

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

    We show an example in how we print out the predictions through the **prediction_results** function.

### Discussions on importing operations for a resolver

Resolvers define operations that the interpreter needs to access in order to run the model. At the time of writing, there are **71 operations** allowed within TFLM.
**We do not recommend importing all operations** due to high memory usage, and importing unused operations into the resolver is unacceptable.
There are certain cases where this is acceptable: when the model's layers are not explicitly defined (e.g., using a model developed by someone else), or during development (i.e., experimentation purposes).

If you choose to only add the operations you need, you must understand which layers are required within your graph. You must either return back to the environment from which you built the model, or use a third-party service that explicitly outlines the layers for you, such as Netron. We have another discussion [here](#how-to-use-netron).

To instantiate the resolver, you must first declare the resolver using `tflite::MicroMutableOpResolver<number_of_ops>`
class. Then, fill in the number of operations you will use under `number_of_ops`, and invoke the functions for each operaion you will use. For instance:

```
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
...

static tflite::MicroMutableOpResolver<9> resolver;
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

If you choose to use an operation not currently within AllOpsResolver – that is, it exists in Tensorflow Core but not in Tensorflow Lite, you must first verify whether [Tensorflow Lite already supports it](https://www.tensorflow.org/mlir/tfl_ops). If it exists, you need to quantize the model with that given operation after training, and explicitly declare it using the function `tflite::MicroMutableOpResolver AddCustom()`. If you need to create a custom operator, you will need to follow the steps in [building and testing your own](https://www.tensorflow.org/lite/guide/ops_compatibility) before importing it into the model.

In general, [Tensorflow Lite does not recommend building your own operations](https://www.tensorflow.org/lite/guide/ops_compatibility) unless needed for performance and size reasons.

On the other hand, if you move forward with importing all of the resolver operations, you will declare the all_ops_resolver before declaring the interpreter:

```
#include "tensorflow/lite/micro/all_ops_resolver.h"
...
static tflite::AllOpsResolver resolver;
```

### How to use Netron

[Netron](www.netron.app) is a open-source project that allows you to view the different layers and operations of a model. To use the service, upload your generated .tflite model (whether quantized or not).

![Picture of Netron's app](/images/netron_details.png)

When you review the layers, you must identify all types of operations the model uses. At the time of writing, this includes the following:

- Basic operations (usually shown in black)
- Pooling layers (usually shown in green)
- Convolutional layers (usually shown in blue)
- Activation functions (usually shown in red)
- Reshaping functions (usually shown in beige/peach color)

The input and output layers colored in light gray (i.e., the first and last layer in the model) don't need to be included. Once you identify all the operations,
check within the [`micro_mutable_op_resolver.h`](https://github.com/tensorflow/tflite-micro/blob/main/tensorflow/lite/micro/micro_mutable_op_resolver.h) file to see their respective operators. Each operation will have an `Add` prefix to them. Then, add them as described [here](#discussions-on-importing-operations-for-a-resolver).

If you fail to add all of the layers, TFLM will give an error and you will not be able to run inferences.

## Running the build

We assume you have already set up the microcontroller properly with the camera mounted, and plugged in to your computer.

To build the executable files, follow the steps in [nmapp2](https://github.com/NorthernMechatronics/nmapp2/blob/master/doc/getting_started.md#build-the-application) to create the build.

Once completed, follow instructions within [nmapp2](https://github.com/NorthernMechatronics/nmapp2/blob/master/doc/getting_started.md#build-the-application) to run debugging on the microcontroller. On Visual Studio Code, this is easy to do: go to Run -> Start Debugging.

Once the debugging process has started, you need to communicate through serial. On MacOS, you can use the terminal:

1. Open up a Terminal window.
2. Find all of the available serial ports by running this command: `ls /dev/tty.*`. Assuming that the microcontroller is on and plugged in, you will likely see a unique name (on our end, it's called `/dev/tty.usbmodem0009000021581`).
3. Run the command: `screen /dev/tty.<insert_name_of_usb_modem> 115200`. [`screen`](https://linuxize.com/post/how-to-use-linux-screen/) is a command within Linux that allows you to open multiple terminals within one session. For our sake, we care only about monitoring the serial port regardless if the program is debugged or not. `/dev/tty.<insert_name_of_usb_modem>` is the name of the microcontroller's serial port; replace the `<insert_name_of_usb_modem` with the actual name of the microcontroller. Lastly, **115200** refers to the [baud rate](https://en.wikipedia.org/wiki/Baud); the speed at which communication can occur between the computer and the microcontroller. This is a fixed rate, and you will receive an error if the baud rate does not match.
4. If you are already debugging, hit `Enter` a couple of times. You may see times printed out along with a `>` sign. If you have not started debugging, and you run debugging through VSCode, you will see Northern Mechatronics, NM180100 Command Console and then `Completed setup`. Both cases mean the program is working.
5. To take pictures, center the images over the digit you want to recognize, and type in `cam capture` on the serial monitor or terminal. The inference should be quick (i.e., a couple of seconds), and you will be able to see the predictions.

VSCode also has a [serial monitor extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode.vscode-serial-monitor) that you can use. The setup is relatively straightforward but the settings are as follows:

- Monitor Mode: Serial
- Port: /dev/tty.<insert_name_of_usb_modem>
- Baud Rate: 115200
- Toggle Terminal Mode is turned On

You can then follow Steps 4 and 5 to verify that the setup works and run inferences.

For Windows, we find that [PuTTY](https://www.putty.org/) is a useful tool for serial communication. Use the same settings as mentioned above.

## Possible errors related to running the build

These errors vary depending on the system you are running the inferences. However, there are errors we have encountered before that prove useful to know.

### "Sorry, could not find a PTY" or "Cannot open line... for R/W: Resource busy" from MacOS terminal

When you first open a screen terminal in MacOS, you should not close the terminal until you are done debugging your program with the microcontroller. Otherwise, you will not be able to reopen that same terminal with the `screen` command. To resolve this, simply turn off the microcontroller, and turn it back on again. Then, run the steps again in [running the build](#running-the-build).

## Possible errors directly related to TFLM

See `tflm.cc` for how error checking is done when running the model.

TFLM does not require explicit error checking since most of them are reported during runtime, and there are accompanying debug statements. However, it is your responsibility to know the model's shapes and required operations, because they influence the predictions that come out of the model. You should be aware the requirements of running a model on a microcontroller are stricter than running one on a laptop or phone.

### AllocateTensors() failed

If the tensor arena size is not large enough to fit the input and output tensors, then TFLM will return an error. You need to try different
sizes through kTensorArenaSize in order to find the minimum needed to run the model.

### Poor alignment

This may not necessarily be coming from TFLM but rather from the module. Ensure that you add **alignas(8)** at the beginning of the model's definition:

```
alignas(8) const unsigned char model_name = {
   ...
}
```

Also, in following TFLM's convention, add **alignas(16)** at the beginning of declaring the tensor arena:

```
constexpr int kTensorArenaSize = 100 * 1024;
alignas(16) uint8_t tensor_arena[kTensorArenaSize];
```

### Declaring arrays in the inference function

The input and output buffers provided within `tflm_inference` are sufficient when running an inference on the model. You may not declare static arrays nor dynamic arrays within the inference functions. This will cause the stack overflow hook within `main.c` to be called – the function's signature is `void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName)`.

Declaring static arrays in the setup function is allowed, but there is no real use in doing this because the setup function is intended to build the interpreter.

### Improper input and output tensor types

Deploying models on microcontrollers require more work because the technical capabilities on a microcontroller are more limiting than other devices. Consequently, you must ensure the model's input and output types are compatible with TFLM.

During the quantization process, you will specify in the converter the inference input and output type. Currently, Tensorflow Lite supports three types of quantization: dynamic range quantization, full integer quantization and float16 quantization. For TFLM, [full integer quantization is best suited for microcontrollers](https://www.tensorflow.org/lite/performance/post_training_quantization#full_integer_quantization).

**Hybrid models are not allowed** – that is, when the inference input type is different than the output type. As of the time of writing, we can **properly** run inferences when both the input and output types are either **tf.uint8** or **tf.int8**.

Whichever type you choose, check that you are copying the image and returning the data to the right data type. For instance, if the data type of both the input and output tensors are **tf.int8**:

- The destination during memcpy should be `input->data.int8`
  ```
  memcpy(input->data.int8, in, inlen->bytes)
  ```
- The output buffer `out` passed in as a parameter in `tflm_inference` should be set to `int8_t` in `tflm.cc` and `tflm.h`.

For **tf.uint8**, the destination during memcpy should be `input->data.uint8`, and the output buffer `out` should be set to `uint8_t`. In tflm.cc, you should also adjust any error checking during setup and inferences so that the model does not return an error

### Improper input and output tensor shapes

Error checking exists within `tflm.cc` to check the input and output shapes are correct. You will adjust this in `model_settings.h`.

Input images typically have a 4D shape. In that shape, the first axis (i.e., `input->data[0]`) indicates how many images are being passed into the model. For inferences, this is always 1. The second and third axes (i.e., `input->data[1]` and `input->data[2]`) indicate the number of rows and columns respectively. The last axis (i.e., `input->data[3]`) indicates the number of channels or bytes within each pixel. For grayscale, the number is usually 1, and for RGB, the number is usually 3. You should verify this by checking the interpreter input and output details – see [model settings](#model-settings) for more details.

Output tensors typically have a 2D shape. The first axis (i.e., `output->data[0]` ) reflects how many tensors are outputted. This number is always 1, because there is always one result for one image. The second axis (`output->data[1]`) indicates how many elements are in the output tensor. For the example models provided, there are 10 elements provided.

### Predictions are not printing out properly

This is uncommon, but if text is not printed out of the error_reporter correctly or doesn't print all the way, you need to ensure that:

- syntax when invoking `TF_LITE_REPORT_ERROR` is correct
- all variables passed into `TF_LITE_REPORT_ERROR` have the correct types within the string.

As a reminder, `TF_LITE_REPORT_ERROR` has at least two parameters: the error reporter `error_reporter` and the text printed out as a string.

If any variables are referenced, then you must:

- reference the data types within the string and where each variable will be used
- pass the variables as extra parameters in `TF_LITE_REPORT_ERROR`.

For instance:

```
TF_LITE_REPORT_ERROR(error_reporter, "Number of rows expected: %d\nNumber of input rows given: %d", kNumRows, input->dims->data[1]);
```

kNumRows and input->dims->data[1] are both integers, so they are referenced by `%d` (similar to `printf` in C, %d indicates a decimal number).
Notice that the variables will be printed in order, so kNumRows will be printed after "expected", and input->dims->data[1] is printed
after "given: ".

Other data types exist too: `%c` for char variables, and `%s` for string variables. See [here](https://cplusplus.com/reference/cstdio/printf/) for more information.
