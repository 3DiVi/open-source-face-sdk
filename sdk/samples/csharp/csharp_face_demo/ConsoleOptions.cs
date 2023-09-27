using CommandLine.Text;
using CommandLine; 
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ApiTest
{
    public class ConsoleOptions
    {
        [Option("mode", Default = "detection", HelpText = "Mode")]
        public string Mode { get; set; }

        [Option("input_image", Required = true, HelpText = "Input image 1")]
        public string InputImagePath { get; set; }
        [Option("input_image2", HelpText = "Input image 2")]
        public string InputImagePath2 { get; set; }

        [Option("sdk_path", Default = "..", HelpText = "SDK path")]
        public string SdkPath { get; set; }

        [Option("window", Default = "yes", HelpText = "Window")]
        public string Window { get; set; }

        [Option("output", Default = "yes", HelpText = "Output")]
        public string Output { get; set; }
    }
}
