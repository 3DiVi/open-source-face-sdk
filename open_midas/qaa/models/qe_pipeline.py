import torch
from torch import nn


class QualityEstimatorPipeline(nn.Module):
    """
    Used to combine face recognition model, extract it's features and estimate quality.
    """
    def __init__(self, backbone_ext, regressor, device="cpu"):
        """
        Get running values from every batch normalization layer of network.
        :param backbone_ext: should return embedding and features (see examples in bn_extraction/models)
        :param regressor: CatBoostRegressor trained to predict quality
        """
        super(QualityEstimatorPipeline, self).__init__()
        self.backbone = backbone_ext.to(device)
        self.regressor = regressor
        self.device = device

    def forward(self, x, return_emb=False):
        emb, features = self.backbone(x)
        quality = self.regressor.predict(features.detach().cpu().numpy())
        quality = torch.Tensor(quality)
        if return_emb:
            return emb, quality
        else:
            return quality
