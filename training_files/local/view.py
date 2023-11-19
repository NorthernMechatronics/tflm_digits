import argparse
import utils

from matplotlib import pyplot as plt

def main():
    parser = argparse.ArgumentParser(description="Import dataset.")
    parser.add_argument("--set", dest="set", action="store", choices=["train", "val", "test"], default="train")
    parser.add_argument("input", action="store")
    parser.add_argument("index", type=int, action="store")
    args = parser.parse_args()

    train_images, train_labels, val_images, val_labels, test_images, test_labels = utils.load_images(args.input)

    match args.set:
        case "train":
            ds = train_images
            lb = train_labels
        case "val":
            ds = val_images
            lb = val_labels
        case "test":
            ds = test_images
            lb = test_labels

    print(lb[args.index])
    plt.imshow(ds[args.index])
    plt.show()

main()