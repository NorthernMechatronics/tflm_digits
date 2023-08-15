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

Convolutional layers consist of two different components: the kernel and the stride. The kernel or filter, typically a 3x3 grid containing a number in each square, passes over the input and performs an [element-wise product](https://en.wikipedia.org/wiki/Hadamard_product_%28matrices%29#:~:text=In%20mathematics%2C%20the%20Hadamard%20product,elements%20i%2C%20j%20of%20the) over an identically sized matrix on the image. Before mapping the result, a [bias]() is added to the product. In Tensorflow, the numbers within the kernel are usually randomized through the normal distribution at 0, but are then learned throughout the training process.

The stride length is the amount of squares the kernel moves once the element-wise product has been calculated. The kernel will pass over the image from left to right, and will shift down to the left side of the image until the kernel has passed over the entire image. In most cases, the stride length is 1 for vertical and horizontal shifts. In Tensorflow, there are parameters which allow you to [control the stride length](https://www.tensorflow.org/api_docs/python/tf/keras/layers/Conv2D) for both vertical and horizontal shifts. Larger strides (e.g., stride length is 2 or 3 instead of 1) can be used to reduce resolution of the feature map.

Convolutional layers are known to identify certain features from the input image. The nature of the element-wise product suggests that local clusters or patterns of pixel values in the input images are amplified. For instance, if an input image is a handwritten digit, pixels after convolution which have a markedly lower or higher value than other parts of an image could indicate a part of the digit instead of the background. Broadly, low-level features such as edges and colors captured in the first set of convolutional layers, whereas high-level features such as the overall space or background of the image is captured in later convolutional layers.

Max pooling layers are designed to reduce the dimensions of the overall input image. It accomplishes this by taking the maximum value of a small grid (usually 2x2) within a part of the image. Evidently, the resulting matrix will be smaller in size with less detail about the original image, but the overall design and shape of the image is maintained. Max pooling behaves differently than convolutional layers because it does not shift in the same way as the stride. Once the maximum value has been evaluated in one grid, we shift the grid to the right by the width of the grid, not by the stride length.

Fully connected/dense layers are one of the last layers of a convolutional neural network. The purpose is to retrieve the relationships of features learned in previous layers, and to perform proper classification tasks. After a series of convolutional and max pooling layers, the fully connected layers will flatten the previous outputs coming from the convolutional layers and transforms them into a single vector through a weights matrix within the layer. The flattening process occurs because the feature maps resulting from a convolutional or max pooling layer are in a two-dimensional matrix, whereas a normal fully connected layer accepts a one dimensional vector. The weights matrix exists because in a fully connected layer, all of its neurons are connected to each neuron in the previous layer, and each neuron has a certain weight that represnts the strength of the connection between two neurons.

Features and capturing complicated relationships are the foci of dense layers. High-level features, especially those captured in later convolutional layers, are collated together to help the network recognize patterns that allow it to classify the input image properly. Typically, the more neurons in a fully connected layer exist, the more able the network is in capturing important relationships, but there is a computational penalty because the high number of connections and weights increase the necessary resources. As well, the large number of parameters within a fully connected layer increases the probability that overfitting may occur. Overfitting occurs when a machine learning model is able to give accurate predictions on the training data, but any data or images outside of the training set cannot be recognized.

Dropout layers are not needed, but they exist to prevent overfitting. They work by randomly removing synapses from neurons of previous layers so that they no longer have an impact on subsequent layers.

### Model architecture: how do you arrange them?

For this section, we use **CONV** to represent convolutional layers, **POOL** for max pooling layers, and **DENSE** for fully connected layers.

In most cases, it's enough to have the following:

INPUT -> CONV -> POOL -> DENSE -> DENSE -> OUTPUT

We may have alternating convolutional and max pooling layers:

INPUT -> CONV -> CONV -> POOL -> CONV -> CONV -> POOL -> CONV -> POOL -> DENSE -> DENSE -> OUTPUT

In some existing papers such as MobileNet, we have the following architecture:

INPUT -> CONV -> CONV (DW) -> CONV -> CONV (DW) -> ... (repeat another 11 times) -> AVG POOL -> DENSE -> DENSE -> OUTPUT

Note that CONV (DW) is a "depthwise convolution layer", and AVG POOL is an average pool layer (akin to a max pooling layer, but taking the average of all the numbers in a specified grid). In the first dense layer, all of the neurons from the previous pooling layer connect to all of the layers in the dense layer, and the second dense layer is the layer responsible for providing predictions, depending on the number of classes that exist.

### Model architecture: activation functions

A crucial part of activation functions is the idea of linearity and non-linearity.
Linear functions are of the form, **y = W'x + b'**, where W' is some weights matrix multipled by the input vector, and b' is the bias. When you input some vector into the linear activation function, the result is simply another vector. The problem is that if you were to take the entire set of vectors existing in a n dimensional space, they would also map a linear function. Non-linear functions on the other hand, are functions for which do not have a linear relationship between their variables. Think of functions that have curves or are piecewise.

Part of our goals when developing a machine learning algorithm is to determine more complicated and intricate relationships of the input data. Using linear activation functions limits the model to learn relations between input and outputs which are linear, whereas data in the real world tends to be less straight-forward and therefore are non-linear.

There are different types of functions that are useful for introducing non-linearity: ReLU, Sigmoid, Tanh and Softmax. For our purposes, we used ReLU and Softmax as our activation functions. In many articles, we'll see ReLU is largely used within a hidden layer – that is, a layer that is neither the first nor the last layer in a model. Softmax functions are used for the last layer in a model, although some articles have debated the usage of other functions such as sigmoid.

Brief history lesson: in the past, tanh and sigmoid were considered great activation functions because not only were they non-linear, but they were also differentiable. Differentiability plays a key role during backpropagation when optimizing the weights during training.

ReLU stands for rectified linear unit. ReLU is simply defined as the piecewise function **f(x) = max{0, x}** where x is just the input value. Although this function apperas linear, this function is considered non-linear because it does not represent a linear function – any negative inputs automatically output zero.

Softmax is a function represented by the following function: \*\*f(z)\_i = (e^z_i)/(sum of e^z_j from 1 to K). In essence, this function transforms some number of real numbers (K real numbers) to a probability distribution. Evidently, this function is used as the last activation function becaue we would like to represent the output of the neural network to some sort of probability distribution when we perform image classification.

Sigmoid is another function similar to softmax, represented by the following function: **S(x) = 1/(1+ e^-x)**. Similar to softmax, the sigmoid function plotted on a graph is a non-linear function with the slope at its highest marked when x = 0. Sigmoid and softmax are both used in classification problems, but the sigmoid function is used for binary classification (i.e., when there are two classes to choose from, or a "YES" or "NO" problem), and the softmax function is used for multi-class classification. Digit recognition involves multiple classes, and we proceed onwards with the softmax function.

### Choosing a good dataset

We'll use information from [Google for Developers](https://developers.google.com/machine-learning/data-prep/construct/collect/data-size-quality) to summarize our findings.

These days, finding datasets is easy since there are many platforms that make publicly available datasets easy to find. Tensorflow makes this easy by way of their API functions such as **tf.data.Datasets**, which allow you to create a seamless input pipeline. However, finding datasets that ideally suits your needs is harder. To evaluate what you need from your dataset, consider:

- the size of the dataset. There is no set number on how many data points or images you need. One argues that for each class you have, you should have at least 1000 data points in your training set. This is not necessarily arbitrary since deep learning requires a lot of data and this can be feasible. However, it is important to not misconstrue that more data points are necessarily better, even if they are of bad quality.
- the quality of the dataset. A successful dataset works to solve the problem you want to achieve. There are certain aspects you want to consider:
  - reliability: how well labelled are your data points? If they're labelled by humans, are there any mistakes within the data?
  - feature representation: how do you handle outliers? If there are data points that are out of ordinary, do we discard those data?
  - minimizing skew: your dataset needs to closely match what you will do when you use the model. The model trained on a dataset intended to recognize animals in the desert should be used to predict animals in the desert (or in general).

## Process of model construction

### Setting up your environment

In the course of the project, we've found four ways to set up the envrionment: Jupyter Notebook, Colab, Kaggle, or simply running Python.

Jupyter Notebook, Colab, Kaggle are recommended for two main reasons. First of all, these environments allow you to segment your code so that any errors that occur do not require you to run the entire script again. Furthermore, these environments already have most of the libraries that you need installed, so there is no need to run **pip install** for certain packages.

### Import the dataset

### Preprocessing the dataset

### Train the model

### Evaluate the effectiveness of the model

### Writing the model to a .tflite file

### Model compression for microcontrollers

## References

Yuval Netzer, Tao Wang, Adam Coates, Alessandro Bissacco, Bo Wu, Andrew Y. Ng Reading Digits in Natural Images with Unsupervised Feature Learning NIPS Workshop on Deep Learning and Unsupervised Feature Learning 2011. (PDF)
