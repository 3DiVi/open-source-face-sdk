from typing import Callable

import torch
from torch import nn

from ..utils import get_running_vals, kl_div_normal


class KLDExtractor(nn.Module):
    """
    Extracts symmetric Kullback–Leibler divergence between running
    statistics of batch normalization layers and distributions of BN inputs during network inference
    """
    def __init__(self, backbone, device='cuda'):
        super(KLDExtractor, self).__init__()
        self.backbone = backbone.to(device).eval()
        self.features = []
        self.device = device
        self.running = get_running_vals(self.backbone)

        ctr = 0
        for layer in self.backbone.modules():
            classname = layer.__class__.__name__
            if classname.find('BatchNorm') == 0:  # not BN
                layer.register_forward_hook(self.save_features_hook(ctr))
                ctr += 1
        assert ctr > 0  # At least one BN found

    def save_features_hook(self, i: int) -> Callable:
        def fn(_, _input, __):
            x = _input[0]
            if len(x.shape) <= 2:
                return
            x = x.flatten(start_dim=2)
            cv, cm = torch.var_mean(x, dim=-1)
            kld = kl_div_normal(cm, cv, self.running['means'][i], self.running['vars'][i])
            self.features.append(kld)

        return fn

    def forward(self, x):
        self.features.clear()
        emb = self.backbone(x)
        return emb, torch.hstack(self.features)

    def __call__(self, x):
        return self.forward(x)
