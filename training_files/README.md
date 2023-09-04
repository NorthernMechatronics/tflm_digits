# How to train a model

The purpose of this project is to demonstrate how to train a machine learning model designed for digit recognition on a microcontroller. We are going to use publicly available datasets to demonstrate, in particular, the Street View House Numbers (SVHN) dataset created by Stanford researchers.

## Table of Contents

- [What are convolutional neural networks (CNN)?](#what-are-convolutional-neural-networks-cnn)
  - [Brief history behind CNNs](#brief-history-behind-cnns)
- [What plays into a good model?](#what-plays-into-a-good-model)
  - [Model architecture: what does each layer mean?](#model-architecture-what-does-each-layer-mean)
  - [Model architecture: how do you arrange them?](#model-architecture-how-do-you-arrange-them)
  - [Model architecture: activation functions](#model-architecture-activation-functions)
  - [Choosing a good dataset](#choosing-a-good-dataset)
- [Tensorflow vs. Keras](#tensorflow-and-keras)
- [Process of model construction](#process-of-model-construction)
  - [Import the dataset](#import-the-dataset)
  - [Preprocessing the dataset](#preprocessing-the-dataset)
  - [Splitting the dataset](#splitting-the-dataset)
  - [Data augmentation](#data-augmentation)
  - [Train the model](#train-the-model)
  - [Construct the model architecture](#construct-the-model-architecture)
  - [Compile the model](#compile-the-model)
  - [Fit the model to the data](#fit-the-model-to-the-data)
  - [Evaluate the effectiveness of the model](#evaluate-the-effectiveness-of-the-model)
  - [Loss and accuracy](#loss-and-accuracy)
  - [Overfitting and underfitting](#overfitting-and-underfitting)
  - [Writing the model to a .tflite file](#writing-the-model-to-a-tflite-file)
  - [Model compression with microcontrollers](#model-compression-for-microcontrollers)
- [References](#references)

## What are convolutional neural networks (CNN)?

Convolutional neural networks (CNN) is a type of neural network where there are convolutional layers within the model. This is different from other networks because convolutional layers play a key role when recognizing important features within the input. We are concerned with how a model intakes an image and processes it to produce a prediction.

#### Brief history behind CNNs

Convolutional neural networks first came into the limelight around the 1980s. A significant discovery was when Yann Lecun developed a model to recognize handwritten digits through the use of convolutional layers that had 5x5 kernels and 2x2 max-pooling layers, called [LeNet](http://vision.stanford.edu/cs598_spring07/papers/Lecun98.pdf). LeNet became a key discovery but was limiting since it required thousands of images to train and images were largely low resolution. The [Modified National Institute of Standards and Technology (MNIST) database](https://en.wikipedia.org/wiki/MNIST_database) is a good foundation for training image processing systems.

Other convolutional neural networks entered the fray. [AlexNet](https://proceedings.neurips.cc/paper_files/paper/2012/file/c399862d3b9d6b76c8436e924a68c45b-Paper.pdf), in 2012, consisted of varying sizes of convolution layers whereas [InceptionNet](https://arxiv.org/pdf/1409.4842.pdf) focused on deep sequential models with various sizes of kernels in its convolutional layers with the goal of better performance. [VGG](https://www.mathworks.com/help/deeplearning/ref/vgg16.html) is noted as a good start for transfer learning, for its small convolutional kernels (3x3 compared to 5x5) makes it computationally easier without the added complexity. On top of that, [MobileNet](https://arxiv.org/pdf/1704.04861.pdf) became an important model for mobile devices as the architecture is less resource intensive.

## What plays into a good model?

### Model architecture: what does each layer mean?

There are three main layers in a CNN: [convolutional layers, fully connected layers, and max pooling layers](https://stanford.edu/~shervine/teaching/cs-230/cheatsheet-convolutional-neural-networks#overview). There are other optional layers that can be included, such as Dropout.

[Convolutional layers](https://www.tensorflow.org/api_docs/python/tf/keras/layers/Conv2D) consist of two different components: the kernel and the stride. The kernel or filter, typically a 3x3 grid containing a number in each square, passes over the input and performs an [element-wise product](https://en.wikipedia.org/wiki/Hadamard_product_%28matrices%29#:~:text=In%20mathematics%2C%20the%20Hadamard%20product,elements%20i%2C%20j%20of%20the) over an identically sized matrix on the image. Before mapping the result, a [bias](https://developers.google.com/machine-learning/glossary#bias-math-or-bias-term) is added to the product. In Tensorflow, the numbers within the kernel are usually randomized through the [normal distribution (mean at 0, standard deviation at 1)](https://en.wikipedia.org/wiki/Normal_distribution), but are then learned throughout the training process.

The stride length is the amount of squares the kernel moves once the element-wise product has been calculated. The kernel will pass over the image from left to right, and will shift down to the left side of the image until the kernel has passed over the entire image. In most cases, the stride length is 1 for vertical and horizontal shifts. In Tensorflow, there are parameters which allow you to [control the stride length](https://www.tensorflow.org/api_docs/python/tf/keras/layers/Conv2D#args) for both vertical and horizontal shifts. Larger strides (e.g., stride length is 2 or 3 instead of 1) can be used to reduce resolution of the feature map.

Convolutional layers are known to identify certain features from the input image. The nature of the element-wise product suggests that local clusters or patterns of pixel values in the input images are amplified. For instance, if an input image is a handwritten digit, pixels after convolution which have a markedly lower or higher value than other parts of an image could indicate a part of the digit instead of the background. Broadly, low-level features such as edges and colors captured in the first set of convolutional layers, whereas high-level features such as the overall space or background of the image is captured in later convolutional layers.

[Max pooling layers](https://www.tensorflow.org/api_docs/python/tf/keras/layers/MaxPooling2D) are designed to reduce the dimensions of the overall input image. It accomplishes this by taking the maximum value of a small grid (usually 2x2) within a part of the image. Evidently, the resulting matrix will be smaller in size with less detail about the original image, but the overall design and shape of the image is maintained. Max pooling behaves differently than convolutional layers because it does not shift in the same way as the stride. Once the maximum value has been evaluated in one grid, we shift the grid to the right by the width of the grid, not by the stride length.

[Fully connected/dense layers](https://www.tensorflow.org/api_docs/python/tf/keras/layers/Dense) are one of the last layers of a convolutional neural network. The purpose is to retrieve the relationships of features learned in previous layers, and to perform proper classification tasks. After a series of convolutional and max pooling layers, the fully connected layers will flatten the previous outputs coming from the convolutional layers and transforms them into a single vector through a weights matrix within the layer. The flattening process occurs because the feature maps resulting from a convolutional or max pooling layer are in a two-dimensional matrix, whereas a normal fully connected layer accepts a one dimensional vector. The weights matrix exists because in a fully connected layer, all of its neurons are connected to each neuron in the previous layer, and each neuron has a certain weight that represnts the strength of the connection between two neurons.

[Features and capturing complicated relationships are the foci of dense layers](<https://en.wikipedia.org/wiki/Layer_(deep_learning)). High-level features, especially those captured in later convolutional layers, are collated together to help the network recognize patterns that allow it to classify the input image properly. Typically, the more neurons in a fully connected layer exist, the more able the network is in capturing important relationships, but there is a computational penalty because the high number of connections and weights increase the necessary resources. As well, the large number of parameters within a fully connected layer increases the probability that overfitting may occur.

[Dropout layers](https://keras.io/api/layers/regularization_layers/dropout/#:~:text=The%20Dropout%20layer%20randomly%20sets,over%20all%20inputs%20is%20unchanged.) are not needed, but they exist to prevent overfitting. They work by randomly removing synapses from neurons of previous layers so that they no longer have an impact on subsequent layers.

### Model architecture: how do you arrange them?

For this section, we use **CONV** to represent convolutional layers, **POOL** for max pooling layers, and **DENSE** for fully connected layers.

[In general](https://stanford.edu/~shervine/teaching/cs-230/cheatsheet-convolutional-neural-networks#overview), the architecture is as follows:

INPUT -> CONV -> POOL -> DENSE -> OUTPUT

We may have alternating convolutional and max pooling layers, provided that we can reduce the dimensions to some degree:

INPUT -> CONV -> CONV -> POOL -> CONV -> CONV -> POOL -> CONV -> POOL -> DENSE -> DENSE -> OUTPUT

There are already different CNN models that have more complicated, deeper architectures, while still maintaining a small model size: see [AlexNet](https://proceedings.neurips.cc/paper_files/paper/2012/file/c399862d3b9d6b76c8436e924a68c45b-Paper.pdf), [MobileNet](https://arxiv.org/pdf/1704.04861.pdf), [EfficientNet](https://arxiv.org/pdf/1905.11946v5.pdf)

### Model architecture: activation functions

A crucial part of activation functions is the idea of linearity and non-linearity. This is explained well [in this video](https://www.youtube.com/watch?v=NkOv_k7r6no).

Linear functions are of the form, **y = W'x + b'**, where W' is some weights matrix multipled by the input vector, and b' is the bias. When you input some vector into the linear activation function, the result is simply another vector. The problem is that if you were to take the entire set of vectors existing in a n-dimensional space, they would also map a linear function. Non-linear functions on the other hand, are functions which do not have a linear relationship between their variables. Think of functions that have curves or are piecewise.

Part of our goals when developing a machine learning algorithm is to determine more complicated and intricate relationships of the input data. Using linear activation functions limits the model to learn relations between input and outputs which are linear, whereas data in the real world tends to be less straight-forward and are non-linear.

There are different types of functions that are useful for introducing non-linearity: ReLU, Sigmoid, Tanh and Softmax. For our purposes, we used ReLU and Softmax as our activation functions. ReLU is mainly used within a hidden layer – that is, a layer that is neither the first nor the last layer in a model. The softmax function is used for the last layer in a model.

[ReLU stands for rectified linear unit](https://www.tensorflow.org/api_docs/python/tf/nn/relu). ReLU is simply defined as the piecewise function **f(x) = max{0, x}** where x is just the input value. Although this function apperas linear, this function is considered non-linear because it does not represent a linear function – any negative inputs automatically output zero.

[Softmax](https://www.tensorflow.org/api_docs/python/tf/nn/softmax) is a function that transforms some number of real numbers (**K** real numbers) to a probability distribution. Evidently, this function is used as the last activation function becaue we would like to represent the output of the neural network to some sort of probability distribution when we perform image classification, especially multi-class classification. Since digit recognition involves multiple classes, using the softmax function in the last layer is a good choice.

### Choosing a good dataset

We'll use information from [Google for Developers](https://developers.google.com/machine-learning/data-prep/construct/collect/data-size-quality) to summarize our findings.

These days, finding datasets is easy since there are many platforms that make datasets publicly available. Tensorflow facilitates this by way of their API functions such as **tf.data.Datasets**, which allow you to create a seamless input pipeline. However, finding datasets that ideally suits your needs is harder. To evaluate what you need from your dataset, consider:

- the size of the dataset. There is no set number on how many data points or images you need. One argues that for each class you have, you should have at least 1000 data points in your training set. This is not necessarily arbitrary since deep learning requires a lot of data and this can be feasible. However, it is important to not misconstrue that more data points are necessarily better, even if they are of bad quality.
- the quality of the dataset. A successful dataset works to solve the problem you want to achieve. There are certain aspects you want to consider:
  - reliability: how well labelled are your data points? If they're labelled by humans, are there any mistakes within the data?
  - feature representation: how do you handle outliers? If there are data points that are out of ordinary, do we discard those data?
  - minimizing skew: your dataset needs to closely match what you will do when you use the model. The model trained on a dataset intended to recognize animals in the desert should be used to predict animals in the desert (or in general).

## Tensorflow and Keras

[Keras and Tensorflow go hand in hand](https://www.tensorflow.org/guide/keras). Tensorflow is an open-source platform developed by Google used to build machine learning and/or deep learning models, whereas Keras is the API used on Tensorflow to construct the models themselves. Keras is the high-level interface that allows you to abstract much of the work needed to produce a solid model, such as tweaking hyperparameters in the model or data processing. People who start learning Tensorflow will learn Keras at the same time.

## Process of model construction

There are a plethora of different models and code already available that are trained on SVHN since the dataset's inception. We will detail each step from importing the dataset to quantizing and preparing it for use on microcontrollers.

### Setting up your environment

In the course of the project, we've found four ways to set up the envrionment: Jupyter Notebook, Colab, Kaggle, or simply running Python.

Jupyter Notebook, Colab, Kaggle are recommended for two main reasons. First, these environments allow you to segment your code so that any errors that occur do not require you to run the entire script again. Furthermore, these environments already have most of the libraries that you need installed, so there is no need to run **pip install** for certain packages.

Here are some tutorials and documentation files for setting up your environment:

Jupyter Notebook:

- [https://docs.jupyter.org/en/latest/](https://docs.jupyter.org/en/latest/)
- [https://www.youtube.com/watch?v=HW29067qVWk](https://www.youtube.com/watch?v=HW29067qVWk)

Colab (requires some sort of Gmail account):

- [https://colab.research.google.com/drive/16pBJQePbqkz3QFV54L4NIkOn1kwpuRrj](https://colab.research.google.com/drive/16pBJQePbqkz3QFV54L4NIkOn1kwpuRrj)

Kaggle (requires you to create an account):

- [https://www.kaggle.com/docs/datasets](https://www.kaggle.com/docs/datasets)

### Import the dataset

This varies based on the dataset you are going to use.

Tensorflow makes available hundreds of datasets on its website, and they are free to use provided there are citations.

To use any of the datasets in Tensorflow, you use the [**tfds.load(name: str, \*, split: ...)**](https://www.tensorflow.org/datasets/api_docs/python/tfds/load) API.

SVHN is available through MATLAB, and we use the function [**scipy.io.loadmat**](https://docs.scipy.org/doc/scipy/reference/generated/scipy.io.loadmat.html) for this.

### Preprocessing the dataset

We are going to talk about SVHN specifically, and explain more generally on how preprocessing works.

Preprocessing is required when the dataset is not currently in the format we would like it to be, or when the dataset needs to be cleaned or transformed to fit your model's requirements.

Currently, if you import the SVHN dataset directly from their website, it will be in a object format with two keys: **'X'** and **'y'**. The former stores all of the images, whereas the latter stores all of the labels.

```
train_images = np.array(train_raw['X'])
test_images = np.array(test_raw['X'])

train_labels = np.array(train_raw['y'])
test_labels = np.array(test_raw['y'])
```

Each image is a 4D array. Originally, the first and second axes indicate the number of rows and columns, and the third axis indicates the number of channels. A [channel](<https://en.wikipedia.org/wiki/Channel_(digital_image)>) in computer vision is simply a grayscale version of the image but comprised of a particular color (i.e., RGB has three channels: red, blue and green; the red channel will take care of varying versions of red).

The bigger problem is that the shape looks like this:

```
(32, 32, 3, 73257)
(32, 32, 3, 26052)
```

The fourth axis indicates how many images are in the set (training or testing). It is far easier to access each image in the first axis than in the fourth axis because the commands are simpler:

```
# accessing the 30000th image in train_images
train_images[0:32][0:32][0:3][30000] # accessing the image through the fourth axis
train_images[30000] # accessing the image through the first axis
```

We use [np.moveaxis](https://numpy.org/doc/stable/reference/generated/numpy.moveaxis.html) to shift the axes over to the right:

```
train_images = np.moveaxis(train_images, -1, 0)
test_images = np.moveaxis(test_images, -1, 0)
```

As part of the [quantization process](#model-compression-for-microcontrollers) and to reduce space, we need to convert the images themselves to a float datatype. We will use 32-bit float numbers.

```
train_images = train_images.astype('float32')
test_images = test_images.astype('float32')
```

Converting the images to a float datatype requires us to normalize the images to maintain [the convention](https://www.mathworks.com/help/matlab/creating_plots/image-types.html) that a black color component is 0 and a white color component is 1.

```
train_images = train_images / 255.0
test_images = test_images / 255.0
```

You will notice that each of the labels are arranged in a 1D array, but each element is an array in itself with 1 element. There are two key parts of multi-classification: identifying the number of classes that exist, and identifying which class best fits the input. In the dataset, there are 10 classes because there are 10 digits. Typically, in binary classification where there are two classes A and B, the input is in either A or B. In multi-class classification, this process is extended to determine which class in some number **n** of classes A_1, A_2, ..., A_n does the input belong to. This strategy is better known as [one vs. all](https://developers.google.com/machine-learning/crash-course/multi-class-neural-networks/one-vs-all#:~:text=all%20provides%20a%20way%20to,classifier%20for%20each%20possible%20outcome.).

[LabelBinarizer](https://scikit-learn.org/stable/modules/generated/sklearn.preprocessing.LabelBinarizer.html) helps us to transform each labels so that each class can be indicated in a binary format. For example, in SVHN, if a digit is identified as 2, then its respective label is `[0 1 0 0 0 0 0 0 0 0]`.

```
lb = LabelBinarizer()
train_labels = lb.fit_transform(train_labels)
test_labels = lb.fit_transform(test_labels)
```

### Splitting the dataset

[train_test_split](https://scikit-learn.org/stable/modules/generated/sklearn.model_selection.train_test_split.html) is a function within `scikit-learn` that splits the training dataset into training and validation sets. The proportion of the images in the testing sets is an argument within the function. Both of these will be used during the training process.

### Data augmentation

[Data augmentation](https://www.tensorflow.org/tutorials/images/data_augmentation) relies on artificially changing the original dataset by adding new images that are slightly modified. This suggests that some of the images have transformed in some way, such as inducing a slant, zooming in and out of the images or shifting the height of the image. The goal of this is to increase the breadth of the images within the dataset so that the model habituates to images taken in different situations – in other words, it aims to prevent overfitting. Keras and Tensorflow offer [ImageDataGenerator](https://www.tensorflow.org/api_docs/python/tf/keras/preprocessing/image/ImageDataGenerator), which can then be used during the model fitting process. As of the time of writing, this function is deprecated.

### Train the model

Training the model requires you to do three things: construct the model's architecture, compile the model, and fit the model to the data.

### Construct the model architecture

For the most part, CNNs usually have different layers stacked on top of each other in which each layer accepts one input tensor and provides one output tensor. In Keras, this is called a [sequential model](https://keras.io/guides/sequential_model/) because the layers are ordered one after the other.

There are several ways to do this:

```
# First way
model = keras.Sequential([
  keras.layers.Conv2D(32, (4, 4), activation="relu", input_shape=(32, 32, 3)),
  keras.layers.MaxPooling2D((2, 2)),
  keras.layers.Dense(128, activation="relu"),
  keras.layers.Dense(2, activation="sigmoid")
])


# Second way
model = keras.Sequential(name="put_model_name_here")
model.add(keras.layers.Conv2D(32, (4, 4), activation="relu", input_shape=(32, 32, 3)))
model.add(keras.layers.MaxPooling2D((2, 2)))
model.add(keras.layers.Dense(128, activation="relu"))
model.add(keras.layers.Dense(2, activation="relu"))

```

### Compile the model

When you [compile](https://www.tensorflow.org/api_docs/python/tf/keras/Model#compile) the model, you need to define three important parameters: the optimizers, the type of loss function, and the metrics you would like to use.

During training, the machine learning algorithm tries to find a good set of weights and biases to form a good enough model that minimizes the error between the predictions it makes on a set of data points and the actual labels associated with each data point. The [loss function](https://developers.google.com/machine-learning/glossary#loss-function) calculates the loss.

[Gradient descent](https://en.wikipedia.org/wiki/Gradient_descent) centers around the idea of finding a local minimum within a differentiable function – in this case, the function is the loss function. There are different optimizers that implement this algorithm - we use [Adam](https://www.tensorflow.org/api_docs/python/tf/keras/optimizers/Adam), which is based on [this paper](https://arxiv.org/abs/1412.6980). Certain hyperparameters exist to tweak the model during the training process, such as the [learning rate](https://www.youtube.com/watch?v=QzulmoOg2JE).

[The metrics](https://www.tensorflow.org/api_docs/python/tf/keras/metrics) are simply different ways to measure the model's performance during training. [The accuracy](https://www.tensorflow.org/api_docs/python/tf/keras/metrics/Accuracy) is an important metric, because it allows us to measure the model's predictions against the actual labels associated to an image.

Once you compile the model, you can run [model.summary()](https://www.tensorflow.org/api_docs/python/tf/keras/Model#summary) to understand the shape of each layer.

### Fit the model to the data

Fitting the model to the data requires the use of [model.fit](https://www.tensorflow.org/api_docs/python/tf/keras/Model#fit), which will train the model for some number of epochs (i.e., iterating over the entire dataset for a set number of times). If you augmented the data before training the model, you will need to use [ImageDataGenerator.flow](https://www.tensorflow.org/api_docs/python/tf/keras/preprocessing/image/ImageDataGenerator#flow) within the fitting process.

Within our examples, we use callbacks to stop training when there is no further improvement. This is called [early stopping](https://en.wikipedia.org/wiki/Early_stopping#:~:text=In%20machine%20learning%2C%20early%20stopping,training%20data%20with%20each%20iteration.), and it is often used to prevent overfitting.

### Evaluate the effectiveness of the model

Evaluating the effectiveness of the model requires the use of a [testing set](https://stats.stackexchange.com/questions/19048/what-is-the-difference-between-test-set-and-validation-set). This set contains images along with their respective labels that the model has not yet seen during the training process. In Keras, we use [model.evaluate](https://www.tensorflow.org/api_docs/python/tf/keras/Model#evaluate).

### Loss and accuracy

First of all, we need to distinguish between training, validation and testing sets.

The set of data we use to [fit the model](#fit-the-model-to-the-data) was the training set. After each epoch, we evaluated the accuracy of the model through a separate set of images called the validation set. This is intended to [tune the hidden layers within the neural network](https://en.wikipedia.org/wiki/Training,_validation,_and_test_data_sets). We generated the validation set during the train_test_split in [splitting the dataset](#splitting-the-dataset).

After fitting the model, we [evaluate the effectiveness of the model](#evaluate-the-effectiveness-of-the-model) using a testing set, which is not seen during the training process.

From Google for Developers, [loss](https://developers.google.com/machine-learning/crash-course/descending-into-ml/training-and-loss) is seen as a penalty for the model making a bad prediction during training and/or validation. For instance, if a model consistently mislabels an image of a digit 2 as a number 4, the loss will increase. The number produced by the loss is dependent on the type of loss function you use.

[Accuracy](https://developers.google.com/machine-learning/crash-course/classification/accuracy) is simply a metric that measures how many correct predictions out of total predictions the model has made. This is seen during both the training and validation stages of training the model.

### Overfitting and underfitting

Generally, when a model overfits, the model can easily recognize data points located within the training set, but fails to recognize other points outside of the training set. On the other hand, when a model underfits, the model fails to recognize the relationship between the input (i.e., the image) and the output (i.e., the label of the image), which increases the error of the course of its training and validation stages.

To investigate whether a model is overfitting, look at the training and validation loss in tandem. If the training loss is considerably lower than the validation or testing loss, then it is likely that the model is overfitting, because it failed to recognize images the model has never seen before.

Underfitting is usually seen in graphs where the validation loss flattens or reaches some sort of minimum and never decreases in spite of the number of epochs or the number of data points in the training set.

If you see that your validation loss is less than training loss, then it is likely you are either doing data augmentation (which makes the training set harder than the validation set), dropout or batch normalization layers.

![Classification on overfit and underfit](../images/classification_training.jpg?raw=true 'Table of underfitting, just right and overfitting models')

### Writing the model to a .tflite file

Converting the model requires you to first [build a converter](https://www.tensorflow.org/api_docs/python/tf/lite/TFLiteConverter#from_keras_model), then convert the model and [store it temporarily](https://www.tensorflow.org/api_docs/python/tf/lite/TFLiteConverter#convert).

### Model compression for microcontrollers

Tensorflow Lite offers multiple techniques to compress a model's size. We investigate two techniques: quantization and pruning.

[Quantization](https://www.mathworks.com/discovery/quantization.html#:~:text=Quantization%20is%20the%20process%20of,and%20range%20of%20a%20value.) is the technique of mapping an infinite number of continuous values to a set of discrete finite values. [In Tensorflow](https://www.tensorflow.org/lite/performance/model_optimization?hl=en), quantization reduces the precision of a model's parameters so that the model size is reduced but the accuracy is relatively maintained. This is important because the constraints on microcontrollers are stricter than those in mobile or computer devices, specifically the size of the model and effective computational power.

When working with microcontrollers, we use [post-training integer quantization](https://www.tensorflow.org/lite/performance/post_training_integer_quant?hl=en#convert_using_integer-only_quantization) since it is easy to implement directly into the compression process. Tensorflow Lite for Microcontrollers supports a [limited number of configurations](https://github.com/tensorflow/tflite-micro/blob/main/tensorflow/lite/micro/kernels/dequantize.cc) where the input and output tensors must be one of several data types.

Some key things to note:

- You are required to generate a representative dataset from your training images.
- The inference input and output types should both be the same, and must be of some integer type (either tf.uint8 or tf.int8). In some examples, TFLM suggests using the signed format (tf.int8).

```
# Representative dataset containing 400 images
def representative_data_gen():
    for input_value in tf.data.Dataset.from_tensor_slices(train_images).batch(1).take(400):
        yield[tf.dtypes.cast(input_value, tf.float32)]


# Optimize the model
"""
For our purposes, we will use integer quantization, and use the
default signatures that Keras API leverages.
"""
converter.optimizations = [tf.lite.Optimize.DEFAULT]
converter.representative_dataset = representative_data_gen
# If any operations can't be quantized, converter throws an error
converter.target_spec.supported_ops = [tf.lite.OpsSet.TFLITE_BUILTINS_INT8]
# Set input and output tensors to int8
converter.inference_input_type = tf.int8
converter.inference_output_type = tf.int8
tflite_model_quant = converter.convert()
tflite_model_quant_file = tflite_models_dir/NAME_OF_TF_QUANT_MODEL
tflite_model_quant_file.write_bytes(tflite_model_quant)
```

[Pruning](https://www.tensorflow.org/model_optimization/guide/pruning?hl=en) centers its focus on removing weights during the training process that have little impact on the model's accuracy. This makes the model easier to compress and the model's size during compression is reduced significantly. It is important to note that the model's size on disk is generally the same, but pruning helps to make downloading the model a lot more easier. According to Tensorflow, pruning is experimental on computer vision problems.

Tensorflow provides a [good tutorial](https://www.tensorflow.org/model_optimization/guide/pruning/comprehensive_guide?hl=en) on how to perform pruning on a dataset.

## References

Yuval Netzer, Tao Wang, Adam Coates, Alessandro Bissacco, Bo Wu, Andrew Y. Ng Reading Digits in Natural Images with Unsupervised Feature Learning NIPS Workshop on Deep Learning and Unsupervised Feature Learning 2011. ([PDF](http://ufldl.stanford.edu/housenumbers/nips2011_housenumbers.pdf))
