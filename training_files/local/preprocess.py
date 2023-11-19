import argparse
import numpy as np
import random
import tensorflow as tf
import tensorflow_datasets as tfds

from matplotlib import pyplot as plt
from os.path import exists
from scipy import ndimage
from sklearn.preprocessing import LabelBinarizer

def rgb2gray(rgb):
    return np.dot(rgb[...,:3], [0.299, 0.587, 0.144])

def preprocess(dataset_tf, input_image_shape, output_image_shape):
    dataset = tfds.as_numpy(dataset_tf)
    dataset_shape = (len(dataset), output_image_shape[0], output_image_shape[1], input_image_shape[2])

    dataset_images = np.empty(dataset_shape, dtype='float32')
    dataset_labels = np.empty((dataset_shape[0], 1), dtype='int8')

    index = 0
    for image, label in dataset:
        resized_image = tf.image.resize_with_pad(image, output_image_shape[0], output_image_shape[1])
        dataset_images[index] = resized_image
        dataset_labels[index] = label
        index += 1

    label = LabelBinarizer()
    dataset_labels = label.fit_transform(dataset_labels)

    return (dataset_images, dataset_labels)

def normalize(dataset):
    normalized_dataset = dataset
    for index in range(dataset.shape[0]):
        max_value = np.max(dataset[index])
        normalized_dataset[index] = dataset[index] / max_value

    return normalized_dataset

def color_convert(dataset, input_colorspace, output_colorspace):
    if (input_colorspace == output_colorspace):
        return dataset

    if (output_colorspace < input_colorspace):
        converted_dataset = rgb2gray(dataset)
        print(converted_dataset.shape)
        return converted_dataset

    raise

def rotate(dataset, angular_range):
    rotated_dataset = dataset
    for index in range(dataset.shape[0]):
        rotated_dataset[index] = ndimage.rotate(
            dataset[index],
            random.randint(angular_range[0], angular_range[1]),
            reshape=False,
            mode='reflect'
            )
    
    return rotated_dataset

def invert(dataset):
    inverted_dataset = dataset
    for index in range(dataset.shape[0]):
        inverted_dataset[index] = 1 - dataset[index]
    
    return inverted_dataset


def main():
    parser = argparse.ArgumentParser(description="Import dataset.")
    parser.add_argument("--colorspace", dest="colorspace", action="store", choices=["rgb", "gray"], default="gray")
    parser.add_argument("--randomised", dest="randomised", action="store_true", default=False)
    parser.add_argument("--invert", dest="invert", action="store_true", default=False)
    parser.add_argument("--model", dest="model", action="store", choices=["svhn_cropped", "mnist"], required=True)
    parser.add_argument("--output", dest="output", action="store", required=True)
    args = parser.parse_args()

    if exists(args.output):
        print("Output file already exists.  Exiting.")
        return

    """Upload the SVHN datasets to the local environment. You can import them [here](http://ufldl.stanford.edu/housenumbers/)."""
    (train_ds, val_ds, test_ds), metadata = tfds.load(args.model,
                                                    split=['train[:80%]', 'train[80%:100%]', 'test'],
                                                    with_info=True,
                                                    as_supervised=True)

    input_image_shape = metadata.features['image'].shape
    output_image_shape = (32, 32)

    input_color_channels = input_image_shape[2]
    if args.colorspace == 'rgb':
        output_color_channels = 3
    else:
        output_color_channels = 1

    train_images, train_labels = preprocess(train_ds, input_image_shape, output_image_shape)
    val_images, val_labels = preprocess(val_ds, input_image_shape, output_image_shape)
    test_images, test_labels = preprocess(test_ds, input_image_shape, output_image_shape)

    if args.randomised:
        train_images = rotate(train_images, (-20, 20))
        test_images = rotate(test_images, (-20, 20))

    train_images = color_convert(train_images, input_color_channels, output_color_channels)
    val_images = color_convert(val_images, input_color_channels, output_color_channels)
    test_images = color_convert(test_images, input_color_channels, output_color_channels)

    train_images = normalize(train_images)
    val_images = normalize(val_images)
    test_images = normalize(test_images)

    if args.invert:
        train_images = invert(train_images)
        val_images = invert(val_images)
        test_images = invert(test_images)

    with open(args.output, 'wb') as f:
        np.save(f, train_images)
        np.save(f, train_labels)
        np.save(f, val_images)
        np.save(f, val_labels)
        np.save(f, test_images)
        np.save(f, test_labels)

main()