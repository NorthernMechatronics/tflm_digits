import argparse
import numpy as np
import ocr_model
import os
import tensorflow as tf
import utils
from matplotlib import pyplot as plt

MODEL_DIR = "./models/"

def predict_lite(interpreter, images, labels, index):
    input_details = interpreter.get_input_details()
    output_details = interpreter.get_output_details()
    image = images[index].reshape(input_details[0]["shape"])
    interpreter.set_tensor(input_details[0]["index"], image)
    interpreter.invoke()
    prediction = interpreter.get_tensor(output_details[0]["index"])
    with np.printoptions(formatter={'all': lambda x: " {:-3.2f} ".format(x)}):
        print("Predict: ", prediction[0])
        print("Label:   ", labels[index])

def predict_quant(interpreter, images, labels, index):
    input_details = interpreter.get_input_details()
    output_details = interpreter.get_output_details()
    image = images[index] * 126
    image = image.astype("int8")
    image = image.reshape(input_details[0]["shape"])
    interpreter.set_tensor(input_details[0]["index"], image)
    interpreter.invoke()
    prediction = interpreter.get_tensor(output_details[0]["index"])
    prediction = prediction.astype("int")
    with np.printoptions(formatter={'all': lambda x: " {:-3.0f} ".format(x)}):
        print("Predict: ", prediction[0] + 128)
        print("Label:   ", labels[index])

    plt.imshow(images[index])
    plt.show()

def main():
    parser = argparse.ArgumentParser(description="Import dataset.")
    parser.add_argument("--model", action="store", choices=["full", "lite", "quant"], default="full")
    parser.add_argument("--images", dest="images", action="store", required=True)
    parser.add_argument("index", type=int, action="store")
    args = parser.parse_args()

    basename = os.path.basename(args.images)
    if "." in basename:
        model_name = ".".join(basename.split(".")[:-1])
    else:
        model_name = basename
    weights_dir = MODEL_DIR + model_name + "/weights"
    tflite_dir = MODEL_DIR + model_name + "/tflite"
    tflite_model_filename = tflite_dir + "/" + model_name + ".tflite"
    tflite_model_quant_filename = tflite_dir + "/" + model_name + "_quant.tflite"

    train_images, train_labels, val_images, val_labels, test_images, test_labels = utils.load_images(args.images)
    image_shape = train_images[0].shape
    match args.model:
        case "lite":
            with open(tflite_model_filename, 'rb') as f:
                tflite_lite_model = f.read()
            interpreter_lite = tf.lite.Interpreter(model_content=tflite_lite_model)
            interpreter_lite.allocate_tensors()
            predict_lite(interpreter_lite, test_images, test_labels, args.index)

        case "quant":
            with open(tflite_model_quant_filename, 'rb') as f:
                tflite_quant_model = f.read()
            interpreter_quant = tf.lite.Interpreter(model_content=tflite_quant_model)
            interpreter_quant.allocate_tensors()
            predict_quant(interpreter_quant, test_images, test_labels, args.index)

        case "full:":
            model = ocr_model.create_model(image_shape)
            model.load_weights(weights_dir + "/weights")

            test_loss, test_acc = model.evaluate(x=test_images, y=test_labels, verbose=0)
            print('Test accuracy is: {:0.4f} \nTest loss is: {:0.4f}'.format(test_acc, test_loss))

main()