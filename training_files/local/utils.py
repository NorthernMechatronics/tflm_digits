import numpy as np

def load_images(filename):
    try:
        with open(filename, 'rb') as f:
            train_images = np.load(f)
            train_labels = np.load(f)
            val_images = np.load(f)
            val_labels = np.load(f)
            test_images = np.load(f)
            test_labels = np.load(f)
    except:
        raise

    return train_images, train_labels, val_images, val_labels, test_images, test_labels

