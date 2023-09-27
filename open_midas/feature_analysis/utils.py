import os

import cv2
import matplotlib as mpl
import matplotlib.pyplot as plt
import numpy as np
import torch
from augmentations import *
from torch.utils.data import Dataset
from torchvision import transforms
from torchvision.transforms import Resize


class SimpleDataset(Dataset):
    def __init__(self, root_path):
        self.name_list = os.listdir(root_path)

        self.transform = transforms.Compose([
            transforms.ToTensor(),
            transforms.Normalize(mean=[0.5, 0.5, 0.5], std=[0.5, 0.5, 0.5]),
        ])
        self.root = root_path

    def __getitem__(self, key):
        img = transforms.Resize(size=(112, 112))(self.transform(
            cv2.cvtColor(cv2.imread(self.root + '/' + self.name_list[key], cv2.IMREAD_UNCHANGED), cv2.COLOR_BGR2RGB)))
        return img

    def __len__(self):
        return len(self.name_list)


def make_jutted_tensor(orig_image):
    images_jutted_color = []
    for i in np.arange(0.9, 0.1, -0.1):
        images_jutted_color.append(jit_of_color(orig_image * 0.5 + 0.5, brightness=i, visualise=False))
    for i in np.arange(1.5, 3, 0.1):
        images_jutted_color.append(jit_of_color(orig_image * 0.5 + 0.5, brightness=i, visualise=False))
    images_jutted_color = [orig_image, ] + images_jutted_color
    images_jutted_color = torch.stack(images_jutted_color)
    return images_jutted_color


def make_rotated_tensor(orig_image):
    images_with_rotation = []
    for i in range(0, 360, 10):
        images_with_rotation.append(rotate_image(orig_image, i))
    images_with_rotation = torch.stack(images_with_rotation)
    return images_with_rotation


def make_blurred_tensor(orig_image):
    images_with_blur = []
    for i in range(1, 250, 5):
        images_with_blur.append(blur_image(orig_image, i / 10))
    images_with_blur = torch.stack(images_with_blur)
    return images_with_blur


def make_grid_tensor(orig_image):
    images_with_grid = []
    n_of_grid_image = 20
    for i in range(0, n_of_grid_image):
        images_with_grid.append(
            torch.Tensor(draw_grid(orig_image, ((i - n_of_grid_image / 2) / n_of_grid_image) * 2)))

    images_with_grid = torch.stack(images_with_grid)
    return images_with_grid


def plot_augmented_features(
        image_tensor,
        feature_tensor,
        feature_name,
        clr_bar_range=(0, 1),
        clr_bar_label="",
        color_map='jet',
        transparency=1.0,
        log_scale=False,
        title=None,
        n_samples=None
):
    n_BNs = feature_tensor.shape[1]

    cmap = plt.get_cmap(color_map, feature_tensor.shape[0])
    resize = Resize((80, 80))
    fig = plt.figure(figsize=(28, 5))
    ax1 = fig.add_axes([0.10, 0.10, 0.70, 0.85])
    plt.title(title, fontsize=25)
    colors = []
    for j, n in enumerate(feature_tensor):
        ax1.plot(range(0, n_BNs), n[0:n_BNs], alpha=transparency, c=cmap(j))

    ax1.plot(range(0, n_BNs), feature_tensor[0][0:n_BNs], alpha=transparency, c=cmap(0))

    plt.xlabel('BN №', fontsize=20)
    plt.ylabel(feature_name, fontsize=20)

    norm = mpl.colors.Normalize(vmin=clr_bar_range[0], vmax=clr_bar_range[1])
    sm = plt.cm.ScalarMappable(cmap=cmap, norm=norm)
    sm.set_array([])
    ticklist = [0.0, 1.0, 2.8]
    ticklabels = [" ", ] * len(ticklist)
    if ticklabels:
        cbar = fig.colorbar(mpl.cm.ScalarMappable(cmap=cmap, norm=norm), ticks=ticklist, orientation='vertical')
        ticklabels = [" " * 20] * len(ticklabels)
        cbar.set_ticklabels(ticklabels, fontsize=30)
        fig_size = fig.get_size_inches() * fig.dpi
        lowest = ((resize(image_tensor[0]) + 1) * 127.5).numpy().swapaxes(0, 2).swapaxes(0, 1).astype(np.uint8)
        original = ((resize(image_tensor[5]) + 1) * 127.5).numpy().swapaxes(0, 2).swapaxes(0, 1).astype(np.uint8)
        higher = ((resize(image_tensor[-1]) + 1) * 127.5).numpy().swapaxes(0, 2).swapaxes(0, 1).astype(np.uint8)
        fig.figimage(lowest, fig_size[0] * 0.64, fig_size[1] * 0.17, zorder=10)
        fig.figimage(original, fig_size[0] * 0.64, fig_size[1] * 0.48, zorder=10)
        fig.figimage(higher, fig_size[0] * 0.64, fig_size[1] * 0.795, zorder=10)
    else:
        cbar = fig.colorbar(sm, ticks=np.linspace(clr_bar_range[0], clr_bar_range[1], N), orientation='vertical')

    if log_scale:
        ax1.set_yscale('log')
    plt.grid()
    plt.show()
