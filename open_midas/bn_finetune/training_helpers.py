import torch
import imgaug.augmenters as iaa

blur_augmentation = iaa.Sometimes(0.05, iaa.GaussianBlur((3, 5))).augment_image


def freeze_backbone(model: torch.nn.Module):
    """
    Freezes the network completely. Only running_mean and running_var parameters keep changing
    :param model: feature extractor
    :return: model
    """
    for n, p in model.named_parameters():
        if torch.distributed.is_initialized():
            torch.distributed.broadcast(p, 0)
        p.requires_grad = False
    return model


def add_blur_to_transforms(dataset: torch.utils.data.Dataset, transforms_attr: str = 'transform'):
    """
    Allows you to insert blur augmentation into the transforms of the dataset
    :param dataset: your train dataset
    :param transforms_attr: attribute name of transformations
    :return: transformed dataset
    """
    if hasattr(dataset, transforms_attr):
        transforms = getattr(dataset, transforms_attr).transforms
        transforms.insert(0, blur_augmentation)
    return dataset


def add_noise_to_batch(images: torch.Tensor, labels: torch.Tensor, noise_percent: float = 0.05):
    """
    Expands the batch with normal distributed noise
    :param images: input images
    :param labels: input labels
    :param noise_percent: percent of noise
    :return: expanded images and labels
    """
    batch_size = images.size(0)
    noise_imgs_shape = list(images.shape)
    noise_imgs_shape[0] = int(batch_size * noise_percent)
    imgs_noise = torch.normal(0, 1, noise_imgs_shape).to(images.device)
    labels_noise = labels[0].repeat(noise_imgs_shape[0])
    images = torch.concat([images, imgs_noise], 0).to(images.device)
    labels = torch.concat([labels, labels_noise], 0).to(images.device)
    return images, labels
