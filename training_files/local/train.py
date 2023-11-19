import argparse
import numpy as np
import keras
import ocr_model
import os
import utils
from matplotlib import pyplot as plt

MODEL_DIR = "./models/"

def main():
    parser = argparse.ArgumentParser(description="Import dataset.")
    parser.add_argument("--images", dest="images", action="store", required=True)
    parser.add_argument("--history", dest="history", action="store_true", default=False)
    parser.add_argument("--compile", dest="compile", action="store_true")
    args = parser.parse_args()


    basename = os.path.basename(args.images)
    if "." in basename:
        model_name = ".".join(basename.split(".")[:-1])
    else:
        model_name = basename
    checkpoint_path = MODEL_DIR + model_name + "/cp.ckpt"
    checkpoint_dir = os.path.dirname(checkpoint_path)
    weights_dir = MODEL_DIR + model_name + "/weights"
    os.makedirs(checkpoint_dir, exist_ok=True)
    os.makedirs(weights_dir, exist_ok=True)

    train_images, train_labels, val_images, val_labels, test_images, test_labels = utils.load_images(args.images)

    image_shape = train_images[0].shape
    print(image_shape)
    model = ocr_model.create_model(image_shape)

    if (args.compile):
        quit()
    
    cp_callback = keras.callbacks.ModelCheckpoint(filepath=checkpoint_path, save_weights_only=True)

    early_stopping = keras.callbacks.EarlyStopping(patience=10, restore_best_weights=True)
    history = model.fit(train_images, train_labels,
                        batch_size=128,
                        epochs=125,
                        validation_data=(val_images, val_labels),
                        callbacks=[cp_callback, early_stopping])
    model.save_weights(weights_dir + "/weights")
    
    if (args.history is True):
        plt.figure(figsize=(20, 10))

        plt.subplot(1, 2, 1)
        plt.plot(history.history['loss'], label='train')
        plt.plot(history.history['val_loss'], label='val')
        plt.ylabel('loss')
        plt.legend()
        plt.title("Epochs vs. Training and Validation Loss")

        plt.subplot(1, 2, 2)
        plt.plot(history.history['accuracy'], label='train')
        plt.plot(history.history['val_accuracy'], label='val')
        plt.ylabel('accuracy')
        plt.legend()
        plt.title("Epochs vs. Training and Validation Accuracy")
        plt.show()
        plt.savefig(MODEL_DIR + model_name + ".png")

main()