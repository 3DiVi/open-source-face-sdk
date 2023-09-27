import catboost as cb
import numpy as np
import torch
from torch.nn import CosineSimilarity
from torch.utils.data import DataLoader
from tqdm import tqdm

depth = 10
l2_leaf_reg = 2
iterations = 2500
learning_rate = 0.03
verbose = 500


def train_qaa(backbone_bnf, dataset, centroids, batch_size=512, device="cuda", save_path="./regressor"):
    """
    Train CatBoostRegressor to predict QAA and save it on save_path

    arguments:
    :param backbone_bnf: backbone that return embedding and BN features
    (for example via bn_extraction.models.KLDExtractor)
    :param dataset: torch dataset, must return image and label
    :param centroids: centroids of each class. centroids[N] must be centroid of class with label N.
    :param batch_size: batch_size in dataloader
    :param device: device to make calculation ("cpu"/"cuda")
    :param save_path: path to save quality regressor CatBoost model.
    """

    cos_sim = CosineSimilarity()
    train_loader = DataLoader(dataset, batch_size=batch_size)
    X_train = None
    y_train = None

    print("Preparing data...")
    with torch.no_grad():
        for batch in tqdm(train_loader):
            images, labels = batch
            embs, features = backbone_bnf(images.to(device))
            cent_sim = cos_sim(embs,
                               centroids[labels.to(device)]).flatten()  # get similarity with centroid for every sample
            if X_train is None:
                X_train = features
                y_train = cent_sim
            else:
                X_train = torch.vstack((X_train, features))
                y_train = torch.hstack((y_train, cent_sim))

    X_train = np.array(X_train.detach().cpu())
    y_train = np.array(y_train.detach().cpu())
    print(X_train.shape)

    print("Training model...")
    regressor = cb.CatBoostRegressor(depth=depth, l2_leaf_reg=l2_leaf_reg,
                                     iterations=iterations, learning_rate=learning_rate,
                                     verbose=verbose, allow_writing_files=False)
    regressor.fit(X_train, y_train)

    try:
        regressor.save_model(save_path)
    except OSError as e:
        print("Save directory not exists... Saving to current directory")
        regressor.save_model("./regressor")
