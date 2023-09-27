import sys
import argparse
import os.path

import cv2
import numpy as np

from face_sdk import Service


def help_message():
    message = f"usage: {sys.argv[0]} [--mode detection | landmarks| recognition] " \
              " [--input_image <path to image>]" \
              " [--input_image2 <path to image>]" \
              " [--sdk_path <path to models> by default download in package folder]" \
              " [--window <yes/no>]" \
              " [--output <yes/no>]"
    print(message)


def get_crop(obj, image):
    """
    Get face crop
    :param obj: Context with bbox array
    :param image: Source image
    :return: cv2.Mat with crop face
    """
    img_w = image.shape[1]
    img_h = image.shape[0]

    rect_ctx = obj["bbox"]
    x = int(rect_ctx[0].get_value() * img_w)
    y = int(rect_ctx[1].get_value() * img_h)
    width = int(rect_ctx[2].get_value() * img_w) - x
    height = int(rect_ctx[3].get_value() * img_h) - y

    crop_image = image[max(0, y - int(height * 0.25)): min(img_h, y + int(height * 1.25)),
                 max(0, x - int(width * 0.25)): min(img_w, x + int(width * 1.25))]

    return crop_image


def get_objects_with_max_confidence(data, result):
    """
    Get object with max confidence
    :param data: ProcessingBlock infer data
    :param result: Result object
    """
    max_confidence = 0
    index_max_confidence = 0
    for i in range(len(data["objects"])):
        confidence = data["objects"][i]["confidence"].get_value()
        if confidence > max_confidence:
            index_max_confidence = i

    result["objects"] = []
    result["objects"].push_back(data["objects"][index_max_confidence])


def draw_bbox(obj, img, output, color=(0, 255, 0)):
    """
    Draw in separate window bounding boxes
    :param obj: Context with bbox array
    :param img: Source image
    :param output: Print results in standard output
    :param color: Bounding box color
    :return: Bounding box rectangle
    """
    rect = obj["bbox"]
    if output == "yes":
        print(f"BBox coordinates: {int(rect[0].get_value() * img.shape[1])}, {int(rect[1].get_value() * img.shape[0])}, {(int(rect[2].get_value() * img.shape[1]))}, {int(rect[3].get_value() * img.shape[0])}")
    return cv2.rectangle(img, (int(rect[0].get_value() * img.shape[1]), int(rect[1].get_value() * img.shape[0])),
                         (int(rect[2].get_value() * img.shape[1]), int(rect[3].get_value() * img.shape[0])), color, 2)


def draw_points(obj, img, output):
    """
    Draw in separate window landmark points
    :param obj: Context with bbox and keypoints array
    :param img: Source image
    :param output: Print results in standard output
    :return: img
    """
    rect = obj["bbox"]
    width = rect[2].get_value() * img.shape[1] - rect[0].get_value() * img.shape[1]
    height = rect[3].get_value() * img.shape[0] - rect[1].get_value() * img.shape[0]

    point_size = 3 if width * height > 480 * 320 else 1

    for points in obj["keypoints"]["points"]:
        img = cv2.circle(img, (int(points["x"].get_value() * img.shape[1]), int(points["y"].get_value() * img.shape[0])), 1,
                         (0, 255, 0), point_size)

    img = cv2.circle(img, (int(obj["keypoints"]["left_eye"]["proj"][0].get_value() * img.shape[1]), int(obj["keypoints"]["left_eye"]["proj"][1].get_value() * img.shape[0])),
                     1, (0, 0, 255), point_size)
    img = cv2.circle(img, (int(obj["keypoints"]["right_eye"]["proj"][0].get_value() * img.shape[1]), int(obj["keypoints"]["right_eye"]["proj"][1].get_value() * img.shape[0])),
                     1, (0, 0, 255), point_size)
    img = cv2.circle(img, (int(obj["keypoints"]["mouth"]["proj"][0].get_value() * img.shape[1]), int(obj["keypoints"]["mouth"]["proj"][1].get_value() * img.shape[0])),
                     1, (0, 0, 255), point_size)

    if output == "yes":
        print(f'left eye ({int(obj["keypoints"]["left_eye"]["proj"][0].get_value() * img.shape[1])}, {int(obj["keypoints"]["left_eye"]["proj"][1].get_value() * img.shape[0])}) right eye ({int(obj["keypoints"]["right_eye"]["proj"][0].get_value() * img.shape[1])}, {int(obj["keypoints"]["right_eye"]["proj"][1].get_value() * img.shape[0])}) mouth ({int(obj["keypoints"]["mouth"]["proj"][0].get_value() * img.shape[1])}, {int(obj["keypoints"]["mouth"]["proj"][1].get_value() * img.shape[0])})')

    return img


def landmarks_demo(sdk_path, img_path, window, output):
    """
    Demonstration function
    :param sdk_path: Path to folder with data/models
    :param img_path: Path to image
    :param window: Show results in separate window
    :param output: Print results in standard output
    """
    service = Service.create_service(sdk_path)
    if not os.path.exists(img_path):
        raise Exception(f"not exist file {img_path}")

    # create processing blocks
    face_detector = service.create_processing_block({"unit_type": "FACE_DETECTOR"})
    mesh_fitter = service.create_processing_block({"unit_type": "FITTER"})

    # prepare image
    img: np.ndarray = cv2.imread(img_path, cv2.IMREAD_COLOR)
    input_image: np.ndarray = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
    height, width, _ = input_image.shape

    # create input/output data context with image context
    imgCtx = {"blob": input_image.tobytes(), "dtype": "uint8_t", "format": "NDARRAY",
              "shape": [dim for dim in input_image.shape]}
    ioData = service.create_context({"image": imgCtx})

    ###########Detector################
    face_detector(ioData)
    ###################################
    
    ###########Fitter#################
    mesh_fitter(ioData)
    ##################################

    for obj in ioData["objects"]:
        img = draw_bbox(obj, img, output)
        img = draw_points(obj, img, output)

    if window == "yes":
        cv2.imshow("result", img)
        cv2.waitKey(0)
        cv2.destroyAllWindows()


def detector_demo(sdk_path, img_path, window, output):
    """
    Demonstration function
    :param sdk_path: Path to folder with data/models
    :param img_path: Path to image
    :param window: Show results in separate window
    :param output: Print results in standard output
    :return:
    """
    service = Service.create_service(sdk_path)
    if not os.path.exists(img_path):
        raise Exception(f"not exist file {img_path}")

    # create processing block
    face_detector = service.create_processing_block({"unit_type": "FACE_DETECTOR"})

    # prepare image
    img: np.ndarray = cv2.imread(img_path, cv2.IMREAD_COLOR)
    input_image: np.ndarray = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
    height, width, _ = input_image.shape

    # create input/output data context with image context
    imgCtx = {"blob": input_image.tobytes(), "dtype": "uint8_t", "format": "NDARRAY",
              "shape": [dim for dim in input_image.shape]}
    ioData = service.create_context({"image": imgCtx})

    ###########Detector################
    face_detector(ioData)
    ###################################

    for obj in ioData["objects"]:
        img = draw_bbox(obj, img, output)

    if window == "yes":
        cv2.imshow("result", img)
        cv2.waitKey(0)
        cv2.destroyAllWindows()


def recognition_demo(sdk_path, img_path_1, img_path_2, window, output):
    """
    Demonstration function
    :param sdk_path: Path to folder with data/models
    :param img_path_1: Path to first recognition image
    :param img_path_2: Path to second recognition image
    :param window: Show results in separate window
    :param output: Print results in standard output
    :return:
    """
    service = Service.create_service(sdk_path)
    if not os.path.exists(img_path_1):
        raise Exception(f"not exist file {img_path_1}")
    if not os.path.exists(img_path_2):
        raise Exception(f"not exist file {img_path_2}")

    # create processing blocks
    face_detector = service.create_processing_block({"unit_type": "FACE_DETECTOR"})
    recognizer = service.create_processing_block({"unit_type": "FACE_RECOGNIZER"})
    mesh_fitter = service.create_processing_block({"unit_type": "FITTER"})
    matcher = service.create_processing_block({"unit_type": "MATCHER_MODULE"})

    # prepare image
    img1: np.ndarray = cv2.imread(img_path_1, cv2.IMREAD_COLOR)
    input_image1: np.ndarray = cv2.cvtColor(img1, cv2.COLOR_BGR2RGB)
    height_img1, width_img1, _ = input_image1.shape

    # create input/output data context with image context
    imgCtx_img1 = {"blob": input_image1.tobytes(), "dtype": "uint8_t", "format": "NDARRAY",
                   "shape": [dim for dim in input_image1.shape]}
    ioData_img1 = service.create_context({"image": imgCtx_img1})

    ###########Detector################
    face_detector(ioData_img1)
    ###################################

    if not len(ioData_img1["objects"]):
        raise Exception(f"no face detected on {img_path_1}")
    obj1 = service.create_context({"image": imgCtx_img1})
    get_objects_with_max_confidence(ioData_img1, obj1)

    # prepare image
    img2: np.ndarray = cv2.imread(img_path_2, cv2.IMREAD_COLOR)
    input_image2: np.ndarray = cv2.cvtColor(img2, cv2.COLOR_BGR2RGB)
    height_img2, width_img2, _ = input_image2.shape

    # create input/output data context with image context
    imgCtx_img2 = {"blob": input_image2.tobytes(), "dtype": "uint8_t", "format": "NDARRAY",
                   "shape": [dim for dim in input_image2.shape]}
    ioData_img2 = service.create_context({"image": imgCtx_img2})

    ###########Detector################
    face_detector(ioData_img2)
    ###################################

    if not len(ioData_img2["objects"]):
        raise Exception(f"no face detected on {ioData_img2}")

    obj2 = service.create_context({"image": imgCtx_img2})
    get_objects_with_max_confidence(ioData_img2, obj2)

    ###########Fitter#################
    mesh_fitter(obj1)
    ##################################

    ###########Recognizer################
    recognizer(obj1)
    #####################################

    ###########Fitter#################
    mesh_fitter(obj2)
    ##################################

    ###########Recognizer################
    recognizer(obj2)
    #####################################

    matcherData = service.create_context({"verification": {"objects": []}})
    matcherData["verification"]["objects"].push_back(obj1["objects"][0])
    matcherData["verification"]["objects"].push_back(obj2["objects"][0])

    matcher(matcherData)
    verdict = matcherData["verification"]["result"]["verdict"].get_value()
    distance = matcherData["verification"]["result"]["distance"].get_value()

    color = (0, 255, 0) if verdict else (0, 0, 255)
    img1 = draw_bbox(obj1["objects"][0], img1, output, color)
    img2 = draw_bbox(obj2["objects"][0], img2, output, color)

    crop1 = get_crop(obj1["objects"][0], img1)
    crop2 = get_crop(obj2["objects"][0], img2)

    crop1 = cv2.resize(crop1, (320, 480))
    crop2 = cv2.resize(crop2, (320, 480))

    print(f"distance = {distance}")
    print(f"verdict = {verdict}")

    if window == "yes":
        cv2.imshow("result", np.concatenate((crop1, crop2), axis=1))
        cv2.waitKey(0)
        cv2.destroyAllWindows()


def parse_args():
    parser = argparse.ArgumentParser(description='Video Recognition Demo')
    parser.add_argument('--mode', default="detection", type=str)
    parser.add_argument('--input_image', type=str, required=True)
    parser.add_argument('--input_image2', type=str)
    parser.add_argument('--sdk_path', default="", type=str)
    parser.add_argument('--window', default="yes", type=str)
    parser.add_argument('--output', default="yes", type=str)

    return parser.parse_args()


if __name__ == "__main__":

    help_message()

    args = parse_args()

    if args.mode == "detection":
        detector_demo(args.sdk_path, args.input_image, args.window, args.output)
    elif args.mode == "landmarks":
        landmarks_demo(args.sdk_path, args.input_image, args.window, args.output)
    elif args.mode == "recognition":
        recognition_demo(args.sdk_path, args.input_image, args.input_image2, args.window, args.output)
    else:
        print("Incorrect mode")
