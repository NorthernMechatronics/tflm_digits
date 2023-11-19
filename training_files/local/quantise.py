import argparse
import pathlib
import tensorflow as tf
import ocr_model
import os
import subprocess
import utils

MODEL_DIR = "./models/"

def main():
    parser = argparse.ArgumentParser(description="Import dataset.")
    parser.add_argument("--images", dest="images", action="store", required=True)
    parser.add_argument("--output", dest="output", action="store", required=True)
    args = parser.parse_args()

    basename = os.path.basename(args.images)
    if "." in basename:
        model_name = ".".join(basename.split(".")[:-1])
    else:
        model_name = basename

    weights_dir = MODEL_DIR + model_name + "/weights"
    tflite_dir = MODEL_DIR + model_name + "/tflite"
    os.makedirs(tflite_dir, exist_ok=True)

    tflite_model_filename = tflite_dir + "/" + model_name + ".tflite"
    tflite_model_quant_filename = tflite_dir + "/" + model_name + "_quant.tflite"

    train_images, train_labels, val_images, val_labels, test_images, test_labels = utils.load_images(args.images)
    image_shape = train_images[0].shape

    model = ocr_model.create_model(image_shape)
    model.load_weights(weights_dir + "/weights")
    converter = tf.lite.TFLiteConverter.from_keras_model(model)
    tflite_model = converter.convert()
    tflite_models_file = pathlib.Path(tflite_model_filename)
    tflite_models_file.write_bytes(tflite_model)

    train_images, train_labels, val_images, val_labels, test_images, test_labels = utils.load_images(args.images)

    def representative_dataset_generator():
        for input_value in tf.data.Dataset.from_tensor_slices(train_images).batch(1).take(400):
            yield[tf.dtypes.cast(input_value, tf.float32)]

    converter.optimizations = [tf.lite.Optimize.DEFAULT]
    converter.representative_dataset = representative_dataset_generator
    # If any operations can't be quantized, converter throws an error
    converter.target_spec.supported_ops = [tf.lite.OpsSet.TFLITE_BUILTINS_INT8]
    # Set input and output tensors to uint8
    converter.inference_input_type = tf.int8
    converter.inference_output_type = tf.int8
    tflite_model_quant = converter.convert()
    tflite_model_quant_file = pathlib.Path(tflite_model_quant_filename)
    tflite_model_quant_file.write_bytes(tflite_model_quant)

    tflite_stats = os.stat(tflite_model_filename)
    tflite_quant_stats = os.stat(tflite_model_quant_filename)
    print()
    print(f"Model Size: {tflite_stats.st_size}")
    print(f"Quantized Model Size: {tflite_quant_stats.st_size}")
    print()

    basename = os.path.basename(args.output)
    if "." in basename:
        var_name = ".".join(basename.split(".")[:-1])
    else:
        var_name = basename

    subprocess.run(["xxd", "-i", "-n", var_name, tflite_model_quant_filename, args.output], capture_output=True)

    header = "#include \"" + var_name + ".h\"\n\nalignas(8) const "
    with open(args.output, "r") as f:
        contents = f.readlines()

    contents.insert(0, header)

    with open(args.output, "w") as f:
        contents = "".join(contents)
        f.write(contents)

main()
