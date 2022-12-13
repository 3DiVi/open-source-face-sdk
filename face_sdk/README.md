# 3DiVi Open Source Face SDK

## Available Models
||||  
|--|--|--|  
|Face Detector :heavy_check_mark: |Face Mesh :heavy_check_mark: | Face Recognizer :heavy_check_mark:| 
|Gender Estimator :hourglass_flowing_sand:| Age Estimator :hourglass_flowing_sand:| Emotions Estimator :hourglass_flowing_sand:| 
|Liveness Detector :hourglass_flowing_sand:| Glass and Mask Detectors :hourglass_flowing_sand:|Eyes Openness Estimator :hourglass_flowing_sand:|
| Body Detector :hourglass_flowing_sand:| Person Re-Identificator :hourglass_flowing_sand:| Human Pose Estimator :hourglass_flowing_sand:| 


## APIs and Platforms support

|API Support|Linux x86-64| Windows| 
|--|--|--|
|C++            |:heavy_check_mark: | :heavy_check_mark:|
|Python         |:heavy_check_mark: | :heavy_check_mark:|
|C#             |:hourglass_flowing_sand:|:hourglass_flowing_sand:|
|Java           |:hourglass_flowing_sand:|:hourglass_flowing_sand:|
|JavaScript     |:hourglass_flowing_sand:|:hourglass_flowing_sand:|

## Documentation
Detailed documentation for commercial and open source SDK is available at [docs.3divi.ai](https://docs.3divi.ai/face-sdk/face-sdk-overview)

## Project structure:
* [models](data/models)  - contains base models for SDK submodules. it's empty by default, you should download models by yourself;
* [image samples ](img_samples) - contains test images for running demo examples, also in the doc folder there are images with the result of the work of the modules;
* [samples](samples) - contains demo source code(see: [Sample](#sample));
* [scripts](scripts) - contains scripts for the basic assembly of the SDK(see: [Building the library](#building-the-library));
* [src](src) and [include](include) - contains SDKs source code 

## Building the library
### Download models
Download [archive with models](https://drive.google.com/file/d/1_LYrVU0Kn_CH4fT-fJFD0ELlICIIg91D/view?usp=sharing) and unpack it in data/models folder
### Build
Build instructions are available for the following platforms:
* [Linux x86-64](scripts/linux_x86_64/BUILD_LINUX_X86_64.md)
* [Windows x86-64 MSVC_17+](scripts/windows/BUILD_WINDOWS.md)
* Install python_api (located in _build/make-install_ after library building) via the _pip install <path_to_python_api>_ command to execute python sample

## Sample

Startup arguments:
* `--mode` - optional, operating mode, default value is "detection", see the list of available modes below
* `--input_image` - required, path to the input image file
* `--input_image2` - required for recognition mode, path to the second input image file
* `--sdk_path` - optional, the path to the installed SDK directory, default value is ".." to launch from the default location _build/make-install/bin_
* `--window` - optional, allows to disable displaying window with results (specify any value except of "yes"), default value is "yes"
* `--output` - optional, allows to disable printing results (points coordinates) in console (specify any value except of "yes"), default value is "yes"

Run the following commands from the _build/make-install/bin_ directory to execute the sample:

* С++ (Windows): 
```bash
./face_demo.exe --mode detection --input_image <path to image>
```
* С++ (Linux): 
```bash
LD_LIBRARY_PATH=../lib ./face_demo --mode detection --input_image <path to image>
```

* Python: 
```bash
python3 face_demo.py --mode detection --input_image <path to image>
```

### Sample modes
* **detection** - Detects the face on the input image visualize bounding box. 
* **landmarks** - Estimates face keypoints and visualize them.
* **recognition** - Detects a face on each image, builds and compares face patterns. Prints matching result and the distance between templates. Draw the crops of detected faces.  
_**Note:** If there is more than one face detected on the image, sample will process the detection with greatest confidence._