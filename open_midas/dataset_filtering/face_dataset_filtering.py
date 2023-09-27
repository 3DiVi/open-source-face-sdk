import networkx as nx
import numpy as np
import torch
from sklearn.cluster import DBSCAN
from sklearn.preprocessing import normalize
from tqdm.notebook import tqdm


class FaceDatasetFiltering:
    """Class to perform dataset filtering using QAA scores.

    Attributes:
        dataset: dataset to filter with help of image quality.
        face_model: model that expects face image and returns templates(embeddings).
        qaa_model: model that expects face image and returns it's qualtiy score.
        device: device to make computations ("cpu"/"cuda")
    """
    def __init__(self, dataset, face_model, qaa_model, device):
        self.dataset = dataset
        self.loader = torch.utils.data.DataLoader(self.dataset, batch_size=32, shuffle=False, num_workers=4)

        self.quality_model = qaa_model.eval()
        self.face_embedder = face_model.eval()
        self.quality_scores = []
        self.embeddings = []
        self.targets = []
        self.device = device

        self.filtering_results = {}
        self.filtering_results_dbscan = {}

    def compute_quality_scores(self):
        self.quality_scores = []
        with torch.no_grad():
            for images, target in tqdm(self.loader):
                scores = self.quality_model(images.to(self.device))
                self.quality_scores.extend(scores.cpu().numpy())
                self.targets.extend(target.cpu().numpy())
        self.quality_scores = np.array(self.quality_scores)
        self.targets = np.array(self.targets)

    def compute_face_embeddings(self):
        self.embeddings = []
        with torch.no_grad():
            for images, _ in tqdm(self.loader):
                embeds = self.face_embedder(images.to(self.device))
                self.embeddings.extend(normalize(embeds.cpu().numpy()))
        self.embeddings = np.array(self.embeddings)

    @staticmethod
    def filter_group_new_clustering(embeddings, quality_scores):
        distances = np.inner(embeddings, embeddings)
        quality_scores_matrix = np.zeros_like(distances)
        for i1 in range(len(embeddings)):
            for i2 in range(len(embeddings)):
                quality_scores_matrix[i1][i2] = min(quality_scores[i1], quality_scores[i2])
        condition = (quality_scores_matrix - distances) > 0.1
        connections = []
        for i1 in range(condition.shape[0]):
            for i2 in range(condition.shape[1]):
                if i1 != i2 and not condition[i1][i2]:
                    connections.append((i1, i2))
        G = nx.Graph(connections)
        communities = nx.algorithms.community.label_propagation_communities(G)
        labels = [-1] * len(embeddings)
        for i, community in enumerate(communities):
            for s_l in community:
                labels[s_l] = i
        return np.array(labels)

    @staticmethod
    def filter_group_dbscan_clustering(embeddings):
        cl = DBSCAN(metric='cosine', eps=0.3, min_samples=2)
        labels = cl.fit_predict(embeddings)
        return labels

    def filter_dataset(self):
        self.compute_quality_scores()
        self.compute_face_embeddings()
        for group in tqdm(set(self.targets)):
            idx = np.where(self.targets == group)
            qa = self.quality_scores[idx]
            emb = self.embeddings[idx]
            self.filtering_results[group] = FaceDatasetFiltering.filter_group_new_clustering(emb, qa)
            self.filtering_results_dbscan[group] = FaceDatasetFiltering.filter_group_dbscan_clustering(emb)
