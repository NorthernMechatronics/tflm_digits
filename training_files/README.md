# How to train a model

The purpose of this project is to demonstrate how to train a machine learning model designed for digit recognition on a microcontroller. We are going to use publicly available datasets to demonstrate, in particular, the Street View House Numbers (SVHN) dataset created by Stanford researchers.

## Table of Contents

- [References](#references)

## What are convolutional neural networks (CNN)?

Convolutional neural networks (CNN) is a type of neural network where there are convolutional layers within the model. This is different from other networks because convolutional layers play a key role when recognizing important features within the input. In the case of this project, we are concerned with how a model intakes an image and processes it to produce a prediction.

#### Brief history behind CNNs

Convolutional neural networks first came into the limelight during the 1980s and 1990s. A main discovery occured when Yann Lecun developed a model to recognize handwritten digits through the use of convolutional layers that had 5x5 kernels and 2x2 max-pooling layers. As it's known, LeNet became a key discovery but was limiting since it required thousands of images to train and images were largely low resolution. The Modified National Institute of Standards and Technology (MNIST) database is a good foundation for training image processing systems.

Other convolutional neural networks entered the fray. AlexNet, in 2012, consisted of varying sizes of convolution layers, and ultimately rendered previous CNNs useless because its peformance was more superior. InceptionNet focused on deep sequential models with various sizes of kernels in its convolutional layers with the goal of better performance. VGG is noted as a good start for transfer learning, for its small convolutional kernels (3x3 compared to 5x5) makes it computationally easier without the added complexity. On top of that, MobileNet became an important model for edge devices as the architecture is less resource intensive.

### What plays into a good model?

### Model architecture: what does each layer mean?

There are three main layers in a CNN: convolutional layers, fully connected layers, and max pooling layers. There are other optional layers that can be included, such as Dropout.

Convolutional layers consist of two different components: the kernel and the stride. The kernel or filter, typically a 3x3 grid containing a number in each square, passes over the input and performs an [element-wise product](https://en.wikipedia.org/wiki/Hadamard_product_%28matrices%29#:~:text=In%20mathematics%2C%20the%20Hadamard%20product,elements%20i%2C%20j%20of%20the) over an identically sized matrix on the image. Before mapping the result, a [bias]() is added to the product. In Tensorflow, the numbers within the kernel are randomly generated.

The stride length is the amount of squares the kernel moves once the element-wise product has been calculated. The kernel will pass over the image from left to right, and will shift down to the left side of the image until the kernel has passed over the entire image. In most cases, the stride length is 1 for vertical and horizontal shifts. In Tensorflow, there are parameters which allow you to [control the stride length](https://www.tensorflow.org/api_docs/python/tf/keras/layers/Conv2D) for both vertical and horizontal shifts.

Convolutional layers are known to identify certain features from the input image. For us, the nature of the element-wise product in the kernel suggests that colors of an image which are closer to extremes (i.e., purely black or white pixels, so 0 or 255) are likely to be recognized by earlier convolutional layers within the model. Pixels which have less extreme colors or are closer to the middle of the spectrum (i.e., somewhat gray or black pixels such as 120-180) will be pronounced in later convolutional layers within the model. Broadly, low-level features such as edges and colors captured in the first set of convolutional layers, whereas high-level features such as the overall space or background of the image is captured in later convolutional layers.

Max pooling layers are designed to reduce the dimensions of the overall input image. It accomplishes this by taking the maximum value of a small grid (usually 2x2) within a part of the image. Evidently, the resulting matrix will be smaller in size with less detail about the original image. Max pooling behaves differently than convolutional layers because it does not shift in the same way as the stride. Once the maximum value has been evaluated, we shift the grid to the right by the width of the grid, not by the stride length.

Fully connected/dense layers are one of the last layers of a convolutional neural network. After a series of convolutional and max pooling layers, the fully connected layers will flatten the previous outputs coming from the convolutional layers and transforms them into a single vector through a weights matrix within the layer. These transformations imply that the input vector for a dense layer will have an influence for every output of the output vector.

### Model architecture: how do you arrange them?

For this section, we use **CONV** to represent convolutional layers, **POOL** for max pooling layers, and **DENSE** for fully connected layers.

In most cases, it's enough to have the following:

INPUT -> CONV -> POOL -> DENSE -> OUTPUT

We may have alternating convolutional and max pooling layers:

INPUT -> CONV -> CONV -> POOL -> CONV -> CONV -> POOL -> CONV -> POOL -> DENSE -> OUTPUT

In some existing papers such as MobileNet, we have the following architecture:

INPUT -> CONV -> CONV (DW) -> CONV -> CONV (DW) -> ... (repeat another 11 times) -> AVG POOL -> DENSE -> DENSE -> OUTPUT

Note that CONV (DW) is a "depthwise convolution layer". In the first dense layer, all of the neurons from the previous pooling layer connect to all of the layers in the dense layer, and the second dense layer is the layer responsible for providing predictions, depending on the number of classes that exist.

### Model architecture: activation functions

There are a couple of functions to add...

### Choosing a good dataset

## Process of model construction

### Setting up your environment

### Import the dataset

### Preprocessing the dataset

### Train the model

### Evaluate the effectiveness of the model

### Writing the model to a .tflite file

### Model compression for microcontrollers

## References

Yuval Netzer, Tao Wang, Adam Coates, Alessandro Bissacco, Bo Wu, Andrew Y. Ng Reading Digits in Natural Images with Unsupervised Feature Learning NIPS Workshop on Deep Learning and Unsupervised Feature Learning 2011. (PDF)
