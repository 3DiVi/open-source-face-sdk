from os.path import join

import cv2
import numpy as np
import pandas as pd
import torch
from skimage import transform as trans
from torchvision import transforms
from tqdm import tqdm

transform = transforms.Compose([
    transforms.ToTensor(),
    transforms.Normalize(mean=[0.5, 0.5, 0.5], std=[0.5, 0.5, 0.5]),
])


def get_embedding_and_quality(backbone, regressor, image_tensor):
    if len(image_tensor.shape) == 3:
        image_tensor = image_tensor.unsqueeze(0)
    embedding, features = backbone(image_tensor)
    sim = np.clip(regressor.predict(features.detach().cpu().numpy()), a_min=0.0, a_max=1.0)
    quality = (sim.squeeze() * 100).astype(np.int16)
    return embedding, quality


def normalize(img, landmark5):
    """
    Align face by five keypoints(left eye, right eye, nose, left mouth corner, right mouth corner)

    :param img: source image
    :param landmark5: array with x,y coordinates of required points
    :return: warped image
    """
    src = np.array([
        [30.2946, 51.6963],
        [65.5318, 51.5014],
        [48.0252, 71.7366],
        [33.5493, 92.3655],
        [62.7299, 92.2041]], dtype=np.float32)
    src[:, 0] += 8.0
    tform = trans.SimilarityTransform()
    tform.estimate(landmark5, src)
    M = tform.params[0:2, :]
    img = cv2.warpAffine(img,
                         M, (112, 112),
                         borderValue=0.0)

    return img


def precalculate_ijbc_features(backbone_bnf, regressor, image_dir, lmk_5pts_path, use_flip_test=False,
                               save_path="./precalculated_features/", batch_size=512, device="cuda:0"):
    """Used to precalculate features and quality for IJB-C dataset. Saves it as numpy array to save_path.

    :param backbone_bnf: backbone that return embedding and BN features
    (for example via bn_extraction.models.KLDExtractor)
    :param regressor: quality regressor (CatBoost)
    :param image_dir: directory with IJB-C images
    :param lmk_5pts_path: path to file with landmarks (IJB-C meta)
    :param use_flip_test: use flipped images in calculation
    :param save_path: path to save calculated arrays
    :return: list of qualities, list of readed faceness scores from meta(unchanged), list of extracted features
    """

    backbone = backbone_bnf
    qualities = []
    faceness_scores = []
    embeddings = None
    current_batch = []
    flipped_batch = []
    ctr = 0

    lmk = pd.read_csv(lmk_5pts_path, sep=' ', header=None).values
    with torch.no_grad():
        for row in tqdm(lmk):

            img = cv2.imread(image_dir + row[0])
            landmark5 = row[1:11].reshape((5, 2)).astype(np.float32)
            faceness_scores.append(row[11])
            img = normalize(img, landmark5)
            img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
            if use_flip_test:
                img_flip = img.copy()
                img_flip = np.fliplr(img_flip)
                img_flip = transform(img_flip.copy())
                flipped_batch.append(img_flip)
            img = transform(img)
            current_batch.append(img)

            ctr += 1

            if ctr == batch_size:
                img_tensor = torch.stack(current_batch).to(device)
                embedding, quality = get_embedding_and_quality(backbone, regressor, img_tensor)

                if use_flip_test:
                    flipped_img_tensor = torch.stack(flipped_batch).to(device)
                    embedding_flip, _ = backbone(flipped_img_tensor)
                    embedding = torch.hstack((embedding, embedding_flip))

                if embeddings is None:
                    embeddings = embedding
                else:
                    embeddings = torch.vstack((embeddings, embedding))

                qualities.extend(quality.flatten())
                ctr = 0
                current_batch.clear()
                flipped_batch.clear()

        if ctr != 0:
            img_tensor = torch.stack(current_batch).to(device)
            embedding, quality = get_embedding_and_quality(backbone, regressor, img_tensor)

            if use_flip_test:
                flipped_img_tensor = torch.fliplr(img_tensor)
                embedding_flip, _ = backbone(flipped_img_tensor)
                embedding = torch.hstack((embedding, embedding_flip))

            if embeddings is None:
                embeddings = embedding
            else:
                embeddings = torch.vstack((embeddings, embedding))

            qualities.extend(quality.flatten())
            current_batch.clear()

    np.save(join(save_path, "qualities"), qualities)
    img_feats = embeddings.cpu().numpy()
    np.save(join(save_path, "features"), img_feats)
    faceness_scores = np.array(faceness_scores).astype(np.float32)
    print(f"Precalculated qualities and features are saved to {save_path}")
    return qualities, faceness_scores, img_feats


def filter_ijbc_by_qaa(tid_mid_path, lmk_5pts_path, pairs_path, qualities, img_feats, faceness_scores,
                       filter_percent=10,
                       save_path="./filtered_meta/"):

    """Used to generate dataset annotation without images with low quality

    :param tid_mid_path: path to original dataset tid_mid annotation
    :param lmk_5pts_path: path to original dataset landmark annotation
    :param pairs_path: path to original dataset pairs annotation
    :param qualities: array of precalculated qualities for IJB-C dataset
    :param img_feats: array of precalculated embeddings for IJB-C dataset
    :param faceness_scores: array with faceness scores
    :param filter_percent: percent of images with low qualities to throw away
    :param save_path: path to save new generated annotations
    """
    sorted_qualities = np.array(sorted(qualities))
    quality_threshold = sorted_qualities[int(len(sorted_qualities) * filter_percent / 100)]

    filtered_annot = join(save_path, "ijbc_face_tid_mid_qaa_filtered.txt")
    filtered_5pts = join(save_path, "ijbc_name_5pts_score_qaa_filtered.txt")
    filtered_pairs = join(save_path, "ijbc_template_pair_label_qaa_filtered.txt")
    lowest_quality_i = []

    reject_ctr = 0
    keep_idx = np.empty(len(qualities), dtype=np.bool_)
    with open(tid_mid_path, "r") as orig_annot:
        with open(filtered_annot, "w") as new_annot:
            for i, line in enumerate(orig_annot.readlines()):
                if qualities[i] >= quality_threshold:
                    keep_idx[i] = True
                    new_annot.write(line)
                else:
                    keep_idx[i] = False
                    lowest_quality_i.append(i)
                    reject_ctr += 1

    with open(lmk_5pts_path, "r") as orig_annot:
        with open(filtered_5pts, "w") as new_annot:
            for i, line in enumerate(orig_annot.readlines()):
                if qualities[i] >= quality_threshold:
                    new_annot.write(line)
    filter_percent = reject_ctr / len(qualities) * 100
    print("Rejected ", round(filter_percent, 3), "% of images")
    print(f"Images left: {(qualities >= quality_threshold).sum()}/{len(qualities)}\n")

    print("Generated new tid_mid annotation")
    print(f"Saved in {filtered_annot}")
    print("Generated new landmark annotation")
    print(f"Saved in {filtered_5pts}\n")

    filtered_img_feats = img_feats[keep_idx]
    filtered_faceness_scores = faceness_scores[keep_idx]

    from eval_ijbc import read_template_pair_list

    filtered_meta = pd.read_csv(filtered_annot, sep=' ', header=None).values
    templates = filtered_meta[:, 1].astype(np.int)

    p1, p2, label = read_template_pair_list(pairs_path)

    original_pairs_counter = len(p1)
    original_templates = np.unique(np.hstack((p1, p2)))
    not_rejected_templates = np.unique(templates)

    if len(not_rejected_templates) != len(original_templates):

        print(f"Found {len(np.unique(np.hstack((p1, p2)))) - len(np.unique(templates))} empty templates")
        print("Removing pairs with empty template...\n")
        empty_templates = set(original_templates).difference(not_rejected_templates)

        rejected_pairs_ctr = 0
        with open(pairs_path, "r") as orig_annot:
            with open(filtered_pairs, "w") as new_annot:
                for i, line in enumerate(orig_annot.readlines()):
                    p1, p2, label = map(int, line.split())
                    if p1 not in empty_templates and p2 not in empty_templates:
                        new_annot.write(line)
                    else:
                        rejected_pairs_ctr += 1
        print(f"Removed {rejected_pairs_ctr} pairs")

    print("Generated new pair annotation")
    print(f"Saved in {filtered_pairs}\n")
    p1, p2, label = read_template_pair_list(filtered_pairs)
    new_pairs_counter = len(p1)
    filtered_templates_pairs = np.unique(np.hstack((p1, p2)))

    print(f"Pairs left: {new_pairs_counter}/{original_pairs_counter}")
    assert len(not_rejected_templates) == len(filtered_templates_pairs)
    return filtered_img_feats, filtered_faceness_scores, filter_percent, lowest_quality_i
