import matplotlib.pyplot as plt
import numpy as np
import math
import random
import cv2


def visualize_ith_group(dataset, i, cols=5):
    """
    Visualizes the i-th group of the given dataset (for SelectedImageFolder only).

    Args:
    - dataset: an instance of SelectedFoldersDataset
    - i: the index of the group/folder to visualize
    - cols: number of columns for the visualization
    """
    if i >= len(dataset.classes):
        print(f"The dataset only has {len(dataset.classes)} groups. Cannot visualize group {i}.")
        return

    # Filter samples to only include the i-th group
    group_samples = [j for j, sample in enumerate(dataset.samples) if sample[1] == i]
    rows = math.ceil(len(group_samples) / cols)
    fig, axes = plt.subplots(rows, cols, figsize=(3 * cols, 3 * rows))

    ds_tansforms = dataset.transform
    dataset.transform = None

    for ax in axes.ravel():
        ax.axis('off')
        if group_samples:
            index = group_samples.pop()
            image = dataset[index][0]
            image = np.array(image)
            ax.imshow(np.array(image))
            # ax.set_title(dataset.classes[label])
    dataset.transform = ds_tansforms
    plt.tight_layout()
    plt.show()


def scale_to_01_range(x):
    """scale and move the coordinates so they fit [0; 1] range"""
    # compute the distribution range
    value_range = np.max(x) - np.min(x)

    # move the distribution so that it starts from zero
    # by extracting the minimal value from all its values
    starts_from_zero = x - np.min(x)

    # make the distribution fit [0; 1] by dividing by its range
    return starts_from_zero / value_range


def scale_image(image, max_image_size):
    image_height, image_width, _ = image.shape

    scale = max(1, image_width / max_image_size, image_height / max_image_size)
    image_width = int(image_width / scale)
    image_height = int(image_height / scale)

    image = cv2.resize(image, (image_width, image_height))
    return image


def compute_plot_coordinates(image, x, y, image_centers_area_size, offset):
    image_height, image_width, _ = image.shape

    # compute the image center coordinates on the plot
    center_x = int(image_centers_area_size * x) + offset

    # in matplotlib, the y axis is directed upward
    # to have the same here, we need to mirror the y coordinate
    center_y = int(image_centers_area_size * (1 - y)) + offset

    # knowing the image center, compute the coordinates of the top left and bottom right corner
    tl_x = center_x - int(image_width / 2)
    tl_y = center_y - int(image_height / 2)

    br_x = tl_x + image_width
    br_y = tl_y + image_height

    return tl_x, tl_y, br_x, br_y


def visualize_grid_images(tx, ty, images, plot_size=1000, max_image_size=100, figsize=None, savepath=None,
                          xlabel=None, ylabel=None, original_axes_ranges=None):
    # we'll put the image centers in the central area of the plot
    # and use offsets to make sure the images fit the plot
    offset = max_image_size // 2
    image_centers_area_size = plot_size - 2 * offset

    tsne_plot = 255 * np.ones((plot_size, plot_size, 3), np.uint8)

    # now we'll put a small copy of every image to its corresponding T-SNE coordinate
    for image, x, y in zip(images, tx, ty):
        # scale the image to put it to the plot
        image = scale_image(image, max_image_size)

        # compute the coordinates of the image on the scaled plot visualization
        tl_x, tl_y, br_x, br_y = compute_plot_coordinates(
            image, x, y, image_centers_area_size, offset,
        )

        # put the image to its TSNE coordinates using numpy subarray indices
        tsne_plot[tl_y:br_y, tl_x:br_x, :] = image

    plt.figure('Embeddings projection with t-SNE', figsize=figsize)
    plt.imshow(tsne_plot)

    if original_axes_ranges is not None:
        min_x, max_x, min_y, max_y = original_axes_ranges

        # Calculate the rescaled x-tick and y-tick positions
        ticks_rescaled = np.linspace(0, plot_size, num=5)
        x_ticks_original = np.linspace(min_x, max_x, num=5)
        y_ticks_original = np.linspace(min_y, max_y, num=5)[::-1]

        # Set custom x-ticks and y-ticks
        plt.xticks(ticks_rescaled, [f"{x:.2f}" for x in x_ticks_original])
        plt.yticks(ticks_rescaled, [f"{y:.2f}" for y in y_ticks_original])
    else:
        plt.axis('off')

    if xlabel is not None:
        plt.xlabel(xlabel)
    if ylabel is not None:
        plt.ylabel(ylabel)

    if savepath:
        plt.savefig(savepath)
    plt.show()


def visualize_tsne(tsne, images, plot_size=1000, max_image_size=100, figsize=None, savepath=None,
                   show_axes=False, xlabel=None, ylabel=None):
    # extract x and y coordinates representing the positions of the images on T-SNE plot
    tx = tsne[:, 0]
    ty = tsne[:, 1]

    # scale and move the coordinates so they fit [0; 1] range
    tx_scaled = scale_to_01_range(tx)
    ty_scaled = scale_to_01_range(ty)

    # visualize the plot: samples as images
    visualize_grid_images(
        tx_scaled, ty_scaled, images, plot_size=plot_size, max_image_size=max_image_size, figsize=figsize,
        savepath=savepath, xlabel=xlabel, ylabel=ylabel,
        original_axes_ranges=(min(tx), max(tx), min(ty), max(ty)) if show_axes else None
    )


def plot_tsne_with_labels(tsne, images, labels):
    colors = {
        0: [0, 255, 0],       # green
        1: [0, 128, 128],     # teal
        2: [255, 0, 0],       # blue
        3: [255, 255, 0],     # cyan
        4: [255, 0, 255],     # magenta
        5: [0, 255, 255],     # yellow
        6: [0, 0, 0],         # black
        7: [128, 0, 128],     # purple
        8: [147, 20, 255],    # pink
        9: [0, 0, 128],       # navy
        10: [128, 128, 0],    # olive
        11: [240, 230, 140],  # khaki
        12: [165, 42, 42],    # brown
        13: [100, 149, 237],  # cornflower blue
        14: [255, 127, 80],   # coral
        15: [220, 20, 60],    # crimson
        16: [255, 215, 0],    # gold
        -1: [0, 0, 255]       # red for noise
    }

    border_images = []
    for label, image in zip(labels, images):
        image = cv2.copyMakeBorder(image, 10, 10, 10, 10, cv2.BORDER_CONSTANT, value=colors[label])
        border_images.append(image)
    visualize_tsne(tsne, border_images, plot_size=1000, max_image_size=112, figsize=(10, 10))
    plt.show()
