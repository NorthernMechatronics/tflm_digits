# Tensorflow Lite for Microcontrollers documentation

This exmaple shows how to run Tensorflow Lite for Microcontrollers (TFLM) to take a picture from a camera, and run inferences on neural networks with varying sizes. The goal is to detect a single digit from a particular image.

## Table of Contents

## Converting the model

If you have converted the trained model to a quantized version, you must convert the model to a hexadecimal character array. On Linux and Mac, this is done through the following:

```
xxd -i name_of_model.tflite > name_of_model.cpp
```

To ensure proper alignment when running the model, you must add `alignas(8)` at the start of the definition.

For Windows, see ... (fill in with another README.md in another folder called conversion...)

The .cpp file contains two variables, one containing the hexadecimal bytes of the model and the other one containing the length of the array (or the size of the model). Once created, you must add a header file with the same name and export their declarations.

## Model settings

See `model_settings.cc` and `model_settings.h` for more details.

Any model imported will have different settings (i.e., different input and output tensors) depending on their use case. In order to prevent errors during
runtime, you **must** explicitly declare the input and output dimensions, along with the output format.

To figure out the correct dimensions and format, you must return back to your file from which you trained the model and check the interpreter's dimensions. Alternatively, you can upload your file to this [Colab file](https://colab.research.google.com/drive/1pdGA1Bw2lMcB66HFVWosK0YfwB1oaRLl#scrollTo=a-seiDlzCKpr), and determine the details from there. Follow the instructions in the file.

![Picture of the input and output dimensions and data types for a given model](/images/input_output_details.png?raw=true 'Input and Output details')

## Sandbox settings

See `tflm.cc` for more details.

Before running inferences on the model, you must declare key variables and functions within the namespace as well as build and invoke the interpreter:

### Discussions on resolver

TFLM requires the user to define operations within the model, and these are found within the `micro_mutable_op_resolver`.

## Running inferences on the model

## Possible errors

## Observing input into model
