import os
from typing import List

import requests

from tqdm import tqdm

__UNIT_TYPES = {
    "FACE_DETECTOR": ["data/models/face_detector/face.onnx"],
    "FACE_RECOGNIZER": ["data/models/recognizer/recognizer.onnx"],
    "FITTER": ["data/models/mesh_fitter/mesh_fitter.onnx"],
    "MATCHER_MODULE": [],
    "HUMAN_BODY_DETECTOR": ["data/models/body_detector/body.onnx"],
    "EMOTION_ESTIMATOR": ["data/models/emotion_estimator/emotion.onnx"],
    "AGE_ESTIMATOR": ["data/models/age_estimator/age_heavy.onnx"],
    "GENDER_ESTIMATOR": ["data/models/gender_estimator/gender_heavy.onnx"],
    "MASK_ESTIMATOR": ["data/models/mask_estimator/mask.onnx"],
    "GLASSES_ESTIMATOR": ["data/models/glasses_estimator/glasses_v2.onnx"],
    "EYE_OPENNESS_ESTIMATOR": ["data/models/eye_openness_estimator/eye.onnx"],
    "LIVENESS_ESTIMATOR": ["data/models/liveness_estimator/liveness_2_7.onnx",
                           "data/models/liveness_estimator/liveness_4_0.onnx"],
    "BODY_RE_IDENTIFICATION": ["data/models/body_reidentification/re_id_heavy_model.onnx"],
    "POSE_ESTIMATOR": ["data/models/top_down_hpe/hpe-td.onnx"],
    "POSE_ESTIMATOR_LABEL": ["data/models/top_down_hpe/label_map_keypoints.txt"],
}

__BASE_URL = "https://download.cvartel.com/facesdk/archives/artifacts/models/"


def make_model_paths(path_to_dir: str, unit_type: str) -> List[str]:
    result: List[str] = list()

    for model in __UNIT_TYPES[unit_type]:
        result.append(os.path.join(path_to_dir, model))

    return result


def download_models(path_to_dir: str, unit_type: str):
    model_paths = make_model_paths(path_to_dir, unit_type)
    models = len(__UNIT_TYPES[unit_type])

    for i in range(models):
        model_path = model_paths[i]
        url = __BASE_URL + __UNIT_TYPES[unit_type][i].replace("data/models/", "")

        response = requests.get(url, allow_redirects=True, stream=True)

        if not response.ok:
            print(f"Failed to download model {model_path} from {url} with status code: {response.status_code}")

            continue

        if not os.path.exists(os.path.dirname(model_path)):
            os.makedirs(os.path.dirname(model_path))

        content_length = int(response.headers.get("Content-Length", 0))

        with open(model_path, "wb") as file, tqdm(
                desc=f"Downloading {model_path}", total=content_length, unit="iB", unit_scale=True,
                unit_divisor=1024
        ) as progress_bar:
            for data in response.iter_content(chunk_size=1024):
                progress_bar.update(file.write(data))
