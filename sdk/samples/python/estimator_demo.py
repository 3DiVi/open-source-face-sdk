import sys
import argparse
import os.path
from typing import List, Tuple

import cv2
import numpy as np

from face_sdk import Service
from face_sdk.modules.context import Context

font_face = cv2.FONT_HERSHEY_SIMPLEX
thickness = 1
precision = 5
font_scale = 0.3
color = (0, 255, 0)
all_modes = ["all", "age", "gender", "emotion", "liveness", "mask", "glasses", "eye_openness"]
output: bool


def get_all_modes(delimiter: str):
    """
    Get printable string with all modes
    :param delimiter: Delimiter between modes
    :return: Printable string with all modes
    """
    result: str = ""

    for i in range(len(all_modes)):
        result += all_modes[i]

        if i + 1 != len(all_modes):
            result += ' ' + delimiter + ' '

    return result


def print_text(text: str):
    """
    Print results in standard output if output is True
    :param text: Result
    """
    if output:
        print(text)


def help_message():
    message = f"usage: {sys.argv[0]} " \
              f" [--mode {get_all_modes('|')}]" \
              " [--input_image <path to image>]" \
              " [--sdk_path <path to models> by default download in package folder]" \
              " [--window <yes/no>]" \
              " [--output <yes/no>]"
    print(message)


def draw_bbox(obj: Context, img: np.ndarray):
    """
    Draw in separate window bounding box
    :param obj: Context with bbox array
    :param img: Source image
    :return: Top left and bottom right points of bounding box
    """
    rect = obj["bbox"]
    first = (int(rect[0].get_value() * img.shape[1]), int(rect[1].get_value() * img.shape[0]))
    second = (int(rect[2].get_value() * img.shape[1]), int(rect[3].get_value() * img.shape[0]))

    cv2.rectangle(img, first, second, color, thickness)

    return first, second


def image_to_sdk_form(image: np.ndarray, service: Service):
    """
    Create Context with image Context
    :param image: Source image
    :param service: Service from Service.create_service(sdk_path)
    :return: Context
    """
    image_context = {"blob": image.tobytes(), "dtype": "uint8_t", "format": "NDARRAY",
                     "shape": [dim for dim in image.shape]}

    return service.create_context({"image": image_context})


def apply_precision(value: float) -> str:
    """
    Apply precision to value
    :param value: Double precision value
    :return: String value with precision
    """
    return "%.{}f".format(precision) % value


def age_evaluate_parse(data: Context, estimation_text: List[str]):
    """
    Parse age
    :param data: ProcessingBlock infer data
    :param estimation_text: Result text
    """
    estimation_text.append(f"Age: {data['age'].get_value()}")


def gender_evaluate_parse(data: Context, estimation_text: List[str]):
    """
    Parse gender
    :param data: ProcessingBlock infer data
    :param estimation_text: Result text
    """
    estimation_text.append(f"Gender: {data['gender'].get_value()}")


def emotion_evaluate_parse(data: Context, estimation_text: List[str]):
    """
    Parse emotions
    :param data: ProcessingBlock infer data
    :param estimation_text: Result text
    """
    emotions = data["emotions"]

    for i in range(len(emotions)):
        estimation_text.append(
            f"{emotions[i]['emotion'].get_value()}: {apply_precision(emotions[i]['confidence'].get_value())}")


def mask_evaluate_parse(data: Context, estimation_text: List[str]):
    """
    Parse mask
    :param data: ProcessingBlock infer data
    :param estimation_text: Result text
    :return:
    """
    estimation_text.append(f"Has mask: {data['has_medical_mask']['value'].get_value()}")


def glasses_evaluate_parse(data: Context, estimation_text: List[str]):
    """
    Parse glasses
    :param data: ProcessingBlock infer data
    :param estimation_text: Result text
    :return:
    """
    estimation_text.append(f"Has glasses: {data['has_glasses'].get_value()}")

    estimation_text.append(f"Glasses confidence: {apply_precision(data['glasses_confidence'].get_value())}")


def eye_openness_evaluate_parse(data: Context, estimation_text: List[str]):
    """
    Parse eye openness
    :param data: ProcessingBlock infer data
    :param estimation_text: Result text
    """
    is_left_eye_open = data["is_left_eye_open"]
    is_right_eye_open = data["is_right_eye_open"]

    estimation_text.append(f"Is left eye open: {is_left_eye_open['value'].get_value()}")

    estimation_text.append(f"Is right eye open: {is_right_eye_open['value'].get_value()}")

    estimation_text.append(f"Left eye openness: {apply_precision(is_left_eye_open['confidence'].get_value())}")

    estimation_text.append(f"Right eye openness: {apply_precision(is_right_eye_open['confidence'].get_value())}")


def liveness_evaluate_parse(data: Context, estimation_text: List[str]):
    """
    Parse liveness
    :param data: ProcessingBlock infer data
    :param estimation_text: Result text
    :return:
    """
    liveness = data["liveness"]

    estimation_text.append(f"Liveness confidence: {apply_precision(liveness['confidence'].get_value())}")

    estimation_text.append(f"Liveness value: {liveness['value'].get_value()}")


def parse_all(data: Context, estimation_text: List[str]):
    """
    Parse all estimators
    :param data: ProcessingBlock infer data
    :param estimation_text: Result text
    :return:
    """
    age_evaluate_parse(data, estimation_text)
    gender_evaluate_parse(data, estimation_text)
    emotion_evaluate_parse(data, estimation_text)
    mask_evaluate_parse(data, estimation_text)
    glasses_evaluate_parse(data, estimation_text)
    eye_openness_evaluate_parse(data, estimation_text)
    liveness_evaluate_parse(data, estimation_text)


def estimate_implementation(service: Service, unit_type: str, data: Context):
    service.create_processing_block({"unit_type": unit_type})(data)

    return data


def estimate(service: Service, mode: str, data: Context):
    """
    Create ProcessingBlock and infer
    :param service: Service from Service.create_service(sdk_path)
    :param mode: One of all, age, gender, emotion, liveness, mask, glasses, eye_openness
    :param data: ProcessingBlock infer data
    :return:
    """
    mode = mode.upper() + "_ESTIMATOR"

    return estimate_implementation(service, mode, data)


def sample(sdk_path: str, image_path: str, mode: str, window: str):
    """
    Demonstration
    :param sdk_path: Path to folder with data/models
    :param image_path: Path to image
    :param mode: One of all, age, gender, emotion, liveness, mask, glasses, eye_openness
    :param window: Show separate window with results
    """
    parsers = {
        "all": parse_all,
        "age": age_evaluate_parse,
        "gender": gender_evaluate_parse,
        "emotion": emotion_evaluate_parse,
        "liveness": liveness_evaluate_parse,
        "mask": mask_evaluate_parse,
        "glasses": glasses_evaluate_parse,
        "eye_openness": eye_openness_evaluate_parse
    }
    service = Service.create_service(sdk_path)

    # prepare image
    image: np.ndarray = cv2.imread(image_path, cv2.IMREAD_COLOR)
    input_image: np.ndarray = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)
    
    # create input/output data context with image context
    data = image_to_sdk_form(input_image, service)

    ###########Detector################
    data = estimate_implementation(service, "FACE_DETECTOR", data)
    ###################################

    if mode == "eye_openness" or mode == "all":
        ###########Fitter################
        data = estimate_implementation(service, "FITTER", data)
        #################################

        if mode == "all":
            for value in all_modes:
                if value == "all":
                    continue

                data = estimate(service, value, data) # create ProcessingBlock for each mode and infer

    if mode != "all":
        data = estimate(service, mode, data) # create specific ProcessingBlock and infer

    estimation_text: List[List[str]] = []
    bboxes: List[Tuple[Tuple[int, int], Tuple[int, int]]] = []
    max_width: int = 0
    max_height: int = 0

    for data_object in data["objects"]:
        if data_object["class"].get_value() != "face":
            continue

        tem: List[str] = []
        height: int = 0

        bboxes.append(draw_bbox(data_object, image))

        parsers[mode](data_object, tem)

        for text in tem:
            (text_width, text_height), baseline = cv2.getTextSize(text, font_face, font_scale, thickness)

            height += text_height

            max_width = max(max_width, text_width)

        max_height = max(max_height, height)

        estimation_text.append(tem)

    image = cv2.copyMakeBorder(image, max_height, max_height, max_width, max_width, cv2.BORDER_CONSTANT)

    for i in range(len(estimation_text)):
        previous_height = 0
        first, second = bboxes[i]

        print_text(f"BBox coordinates: ({first[0]}, {first[1]}), ({second[0]}, {second[1]})")

        for text in estimation_text[i]:
            print_text(text)

            if window == "yes":
                offset = 2

                cv2.putText(
                    image,
                    text,
                    (second[0] + max_width, first[1] + max_height + previous_height),
                    font_face,
                    font_scale,
                    color,
                    thickness
                )

                previous_height += offset + cv2.getTextSize(text, font_face, font_scale, thickness)[0][1]

    if window == "yes":
        cv2.imshow("result", image)
        cv2.waitKey(0)
        cv2.destroyWindow("result")


def parse_args():
    parser = argparse.ArgumentParser(description='Video Recognition Demo')
    parser.add_argument('--mode', default="all", type=str)
    parser.add_argument('--input_image', type=str, required=True)
    parser.add_argument('--sdk_path', default="", type=str)
    parser.add_argument('--window', default="yes", type=str)
    parser.add_argument('--output', default="yes", type=str)

    return parser.parse_args()


if __name__ == "__main__":
    help_message()

    args = parse_args()

    if not os.path.exists(args.input_image):
        raise Exception(f"not exist file {args.input_image}")

    if args.mode not in all_modes:
        raise Exception(f"--mode should be: {get_all_modes('|')}")

    output = args.output == "yes"

    sample(args.sdk_path, args.input_image, args.mode, args.window)
