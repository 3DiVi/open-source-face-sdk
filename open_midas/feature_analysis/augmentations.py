import random

import matplotlib.pyplot as plt
import numpy as np
import torch
from torchvision import transforms


random.seed(0)


def draw_grid(image, grid_brightness, visualise=False):
    """ Apply grid to image """
    _image = np.moveaxis(image.numpy(), 0, -1) / 2 + 0.5
    new_image = np.zeros(_image.shape)
    new_image[16::16, ::, :] = grid_brightness
    new_image[::, 16::16, :] = grid_brightness
    if visualise:
        plt_image = np.clip((new_image + _image), 0, 1)
        plt.imshow(plt_image)
        plt.show()
    new_image = (np.clip((new_image + _image), 0, 1) * 2) - 1
    new_image = np.moveaxis(new_image, -1, 0)
    return new_image


def rotate_image(image, rotate_scale, visualise=False):
    """ Rotate image by rotate_scale degrees """
    img = image
    rotated_img = transforms.functional.rotate(img, rotate_scale)
    if visualise:
        vis = np.moveaxis(rotated_img.numpy(), 0, -1)
        vis = vis / 2 + 0.5
    return rotated_img


def blur_image(image, blur_scale, visualise=False):
    """ Add Gaussian blur to image """
    img = image
    blur_img = transforms.GaussianBlur(kernel_size=(35, 35), sigma=blur_scale)(img)
    if visualise:
        vis = np.moveaxis(blur_img.numpy(), 0, -1)
        vis = vis / 2 + 0.5
    return blur_img


def jit_of_color(image, brightness=None, contrast=None, saturation=None, hue=None, visualise=False):
    """ Change brightness of image """
    return (transforms.functional.adjust_brightness(image, brightness) - 0.5) * 2
