from typing import Callable

import torch
from torch import nn


class VarMVExtractor(nn.Module):
    """
    Extracts mean and variance of BN inputs during network inference.
    """
    def __init__(self, backbone, device='cuda'):
        super(VarMVExtractor, self).__init__()
        self.backbone = backbone.to(device).eval()
        self.features = []
        self.device = device

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

            v = torch.var(x.detach(), dim=(2, 3))
            vv, vm = torch.var_mean(v, dim=1)

            self.features.append(vv)
            self.features.append(vm)

        return fn

    def forward(self, x):
        self.features.clear()
        emb = self.backbone(x)
        return emb, torch.stack(self.features).T

    def __call__(self, x):
        return self.forward(x)
