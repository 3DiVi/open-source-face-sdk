using CommandLine;
using CSharpApi;
using OpenCvSharp;
using System.Collections.Generic;
using System.ComponentModel.DataAnnotations;
using System.Data;
using System.Security.Cryptography;
using System.Xml.Linq;
using System.IO;
using static System.Net.Mime.MediaTypeNames;

namespace ApiTest
{
    internal class Program
    {
        static int thickness = 1;
        static Scalar color = new Scalar(0, 255, 0);
        static List<string> allModes = new List<string>{ "all", "age", "gender", "emotion", "liveness", "mask", "glasses", "eye_openness" };
        
        static void HelpMessage()
        {
            string message = $"usage: {AppDomain.CurrentDomain.FriendlyName}.exe " +
             $"[--mode detection | pose | reidentification] \r\n" +
              " [--input_image <path to image>] \r\n" +
              " [--sdk_path ..] \r\n" +
              " [--output <yes/no>] \r\n";
            Console.WriteLine(message);
        }
        static string mode = "", inputImagePath = "", sdkpath = "", window = "", output = "";
        static void ParseArguments(string[] args)
        {
            Parser.Default.ParseArguments<ConsoleOptions>(args).WithParsed<ConsoleOptions>(parsed =>
            {
                mode = parsed.Mode;
                inputImagePath = parsed.InputImagePath;
                sdkpath = parsed.SdkPath;
                window = parsed.Window;
                output = parsed.Output;
            }).WithNotParsed<ConsoleOptions>(notparsed =>
            {
                HelpMessage();
                Environment.Exit(0);
            });
        }

        /**
         * @brief Check is path exist
         *
         * @param path Path to image
        */
        static void CheckFileExist(string path)
        {
            if (File.Exists(path))
                return;
            else
                throw new FileNotFoundException($"File {path} not found");
        }

        /**
         * @brief Convert Mat to array of bytes
         *
         * @param bsmCtx Context with result array of bytes
         * @param img Source image
        */
        public static void MatToBsm(ref Dictionary<object, object> bsmCtx, Mat img)
        {
            var input_img = img.IsContinuous() ? img : img.Clone();
            long copySize = !img.IsContinuous() ? input_img.Total() * input_img.ElemSize() : 0;

            bsmCtx["format"] = "NDARRAY";
            long size = input_img.Total() * input_img.ElemSize();

            byte[] arr = new byte[size]; //start of fix
            using (Mat temp = new Mat(input_img.Rows, input_img.Cols, input_img.Type(), arr))
            {
                input_img.CopyTo(temp);
            }
            bsmCtx["blob"] = arr; //end of fix
            List<object> sizes = new List<object>();
            for (int i = 0; i < input_img.Dims; ++i)
            {
                sizes.Add(input_img.Size(i));
            }
            sizes.Add(input_img.Channels());
            bsmCtx["shape"] = sizes;
            bsmCtx["dtype"] = CvTypeToStr[input_img.Depth()];
        }

        /**
         * @brief List of tracked bones
        */
        static List<KeyValuePair<string, string>> bone_map = new List<KeyValuePair<string, string>>
        {
            new KeyValuePair<string, string>("right_ankle","right_knee"),
            new KeyValuePair<string, string>("right_knee","right_hip"),
            new KeyValuePair<string, string>("left_hip", "right_hip"),
            new KeyValuePair<string, string>("left_shoulder","left_hip"),
            new KeyValuePair<string, string>("right_shoulder","right_hip"),
            new KeyValuePair<string, string>("left_shoulder","right_shoulder"),
            new KeyValuePair<string, string>("left_shoulder","left_elbow"),
            new KeyValuePair<string, string>("right_shoulder","right_elbow"),
            new KeyValuePair<string, string>("left_elbow","left_wrist"),
            new KeyValuePair<string, string>("right_elbow","right_wrist"),
            new KeyValuePair<string, string>("left_eye","right_eye"),
            new KeyValuePair<string, string>("nose","left_eye"),
            new KeyValuePair<string, string>("left_knee", "left_hip"),
            new KeyValuePair<string, string>( "right_ear", "right_shoulder"),
            new KeyValuePair<string, string>("left_ear", "left_shoulder"),
            new KeyValuePair<string, string>("right_eye", "right_ear"),
            new KeyValuePair<string, string>("left_eye", "left_ear"),
            new KeyValuePair<string, string>("nose", "right_eye"),
            new KeyValuePair<string, string>("left_ankle", "left_knee")
        };

        /**
         * @brief Demonstration function
        */
        static void DemoBody()
        {
            Service service = Service.CreateService(sdkpath);
            Dictionary<object, object> modelCtx = new Dictionary<object, object>(); // create config Context for HumanBodyDetector ProcessingBlock
            modelCtx["unit_type"] = "HUMAN_BODY_DETECTOR";

            ProcessingBlock bodyDetector = service.CreateProcessingBlock(modelCtx);

            // prepare image
            Mat image = Cv2.ImRead(inputImagePath, ImreadModes.Color);
            Mat input_image = new Mat();
            if (image.Channels() == 3)
            {
                Cv2.CvtColor(image, input_image, ColorConversionCodes.BGR2RGB);
            }
            else
            {
                input_image = image.Clone();
            }

            // create input/output data Context with image Context
            Dictionary<object, object> imgCtx = new Dictionary<object, object>();
            MatToBsm(ref imgCtx, input_image);
            Context ioData = service.CreateContext(new Dictionary<object, object> { { "image", imgCtx } });
            
            ///////////Detector////////////////
            bodyDetector.Invoke(ioData);
            ///////////////////////////////////

            if (mode == "reidentification")
            {
                Dictionary<object, object> bodyReidCtx = new Dictionary<object, object>(); // create config Context for BodyReidentification ProcessingBlock
                bodyReidCtx["unit_type"] = "BODY_RE_IDENTIFICATION";

                ProcessingBlock bodyReidentification = service.CreateProcessingBlock(bodyReidCtx);
                
                ///////////Reidentification////////////////
                bodyReidentification.Invoke(ioData);
                ///////////////////////////////////////////
            }
            else if (mode == "pose")
            {
                Dictionary<object, object> poseCtx = new Dictionary<object, object>(); // create config Context for PoseEstimator ProcessingBlock
                poseCtx["unit_type"] = "POSE_ESTIMATOR";

                ProcessingBlock poseEstimator = service.CreateProcessingBlock(poseCtx);

                ///////////Estimator////////////////
                poseEstimator.Invoke(ioData);
                ////////////////////////////////////
            }

            DisplayResultInWindow(ioData, image, mode, output);
        }

        /**
         * @brief Draw in separate window bounding boxes
         * @param obj Context with bbox array
         * @param image Source image
        */
        static void DrawBBox(Context obj, Mat img, string output, Scalar color)
        {
            Context rectCtx = obj["bbox"];
            Point topLeft = new Point((int)(rectCtx[0].GetDouble() * img.Width), (int)(rectCtx[1].GetDouble() * img.Height));
            Point bottomRight = new Point((int)(rectCtx[2].GetDouble() * img.Width), (int)(rectCtx[3].GetDouble() * img.Height));
            int thickness = 2;
            Cv2.Rectangle(img, topLeft, bottomRight, color, thickness);
            if (output == "yes")
            {
                Console.WriteLine("BBox Coordinates: "
                + (int)(rectCtx[0].GetDouble() * img.Width) + ", " + (int)(rectCtx[1].GetDouble() * img.Height) +
           ", " + (int)(rectCtx[2].GetDouble() * img.Width) + ", " + (int)(rectCtx[3].GetDouble() * img.Height));
            }
        }

        /**
         * Display results in separate window
         *
         * @param ioData ProcessingBlock infer data
         * @param image Source image
         * @param mode One of reidentification, pose
         * @param output Print results in standard output
        */
        static void DisplayResultInWindow(Context ioData, Mat image, string mode, string output)
        {
            for (int i = 0; i < (int)ioData["objects"].GetLength(); i++)
            {
                if (ioData["objects"][i]["class"].GetStr() != "body")
                {
                    continue;
                }
                DrawBBox(ioData["objects"][i], image, output, new Scalar(0,255,0));
                if (mode == "pose")
                {
                    Context posesCtx = ioData["objects"][i]["keypoints"];
                    foreach (var bone in bone_map)
                    {
                        string key1 = bone.Key;
                        string key2 = bone.Value;
                        int x1 = (int)(posesCtx[key1]["proj"][0].GetDouble() * image.Height);
                        int y1 = (int)(posesCtx[key1]["proj"][1].GetDouble() * image.Width);
                        int x2 = (int)(posesCtx[key2]["proj"][0].GetDouble() * image.Height);
                        int y2 = (int)(posesCtx[key2]["proj"][1].GetDouble() * image.Width);
                        if (output == "yes")
                            Console.WriteLine($"Pose: x1: {x1}, y1: {y1}, x2: {x2}, y2: {y2}");
                        Cv2.Line(image, new Point(x1, y1), new Point(x2, y2), color, thickness);

                        Cv2.Circle(image, new Point(x1, y1), 3, new Scalar(0, 0, 255), -1, 0);
                        Cv2.Circle(image, new Point(x2, y2), 3, new Scalar(0, 0, 255), -1, 0);
                    }
                }
            }

            if (mode == "reidentification")
            {
                Context outputData = ioData["output_data"];
                Context templateData = outputData["template"];
                ulong templateSize = outputData["template_size"].GetUnsignedLong();
                string resultFileName = Path.GetFileNameWithoutExtension(inputImagePath) + ".txt";

                using (StreamWriter templateFile = new StreamWriter(resultFileName))
                {
                    templateFile.WriteLine(templateSize);

                    for (ulong i = 0; i < templateSize; i++)
                    {
                        templateFile.Write(templateData[(int)i].GetDouble());

                        if (i + 1 != templateSize)
                        {
                            templateFile.Write(' ');
                        }
                    }
                }
            }

            Cv2.ImShow("image", image);
            Cv2.WaitKey();
        }


        unsafe static void Main(string[] args)
        {
            HelpMessage();
            ParseArguments(args);
            CheckFileExist(inputImagePath);
            if (mode == "detection" || mode == "reidentification" || mode == "pose")
            {
                DemoBody();
            }
            else
                throw new Exception("wrong mode");
        }
        public static Dictionary<int, string> CvTypeToStr = new Dictionary<int, string>
        { 
                {MatType.CV_8U,"uint8_t"}, {MatType.CV_8S, "int8_t"}, 
                {MatType.CV_16U, "uint16_t"}, {MatType.CV_16S, "int16_t"} ,
                {MatType.CV_32S, "int32_t"}, {MatType.CV_32F, "float"}, {MatType.CV_64F, "double"} 
        };
    }
}