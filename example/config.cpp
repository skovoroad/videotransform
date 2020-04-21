#include <iostream>
#include <fstream>
#include <vector>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include "config.h"

namespace vt {
  std::ostream& operator<<(std::ostream& out, vt::VtPixFormat pix) {
    std::string val = ( pix == vt::VT_YUV420P ? "yuv420p" :
                        pix == vt::VT_BGR24 ? "bgr24" :
	               "<unknown>" );
    out << val;
    return out;
  }
 
  std::ostream& operator<<(std::ostream& out, vt::VtCodec codec) {
    std::string val = ( codec == vt::VT_H264 ? "h264" :
                        codec == vt::VT_H263 ? "h263" :
	                "<unknown>" );
    out << val;
    return out;
  }
 

  std::istream& operator>>(std::istream& in, vt::VtCodec& codec)
  {
    std::string token;
    in >> token;
    if (boost::iequals(token, "h264"))
      codec = vt::VT_H264;
    else if (boost::iequals(token, "h263"))
      codec = vt::VT_H263;
    else
      in.setstate(std::ios_base::failbit);
    return in;
  }

  std::istream& operator>>(std::istream& in, vt::VtPixFormat& out)
  {
    std::string token;
    in >> token;
    if (boost::iequals(token, "YUV420P"))
      out = vt::VT_YUV420P;
    else if (boost::iequals(token, "BGR24"))
      out = vt::VT_BGR24;
    else
      in.setstate(std::ios_base::failbit);
    return in;
  }
}

bool dumpConfig(const vt::VideoTransformConfig& cfg, Config& config) {
  std::cout  
    << "file in: " << config.fileIn << std::endl
    << "dir out: " << config.dirOut << std::endl
    << "extract picture: " << cfg.actions_.extractPicture_ << std::endl
    << "scale video: " << cfg.actions_.scaleVideo_ << std::endl
    << "codec in: " << cfg.codecIn << std::endl
    << "codec out: " << cfg.codecOut << std::endl
    << "width out: " << cfg.widthOut << std::endl
    << "height out: " << cfg.heightOut << std::endl
    << "frame rate: " << cfg.frameRateOut << std::endl
    << "gop size: " << cfg.gopSize << std::endl
    << "bit rate: " << cfg.bitRateOut << std::endl
    << "preset: " << cfg.preset << std::endl
    << "tune: " << cfg.tune << std::endl
    << "in video pixel format: " << cfg.inPixelFormat << std::endl
    << "out video pixel format: " << cfg.outPixelFormat << std::endl
    << "out picture format: " << cfg.outPictureFormat << std::endl
 ;
  return true;
}

bool validateConfig(const vt::VideoTransformConfig& cfg) {
  std::vector<std::string> availablePresets = {"ultrafast", "superfast", "veryfast", "faster", "fast", "medium", "slow", "slower", "veryslow", "placebo"}; 
  std::vector<std::string> availableTunes = {"film", "animation", "grain", "stillimage", "fastdecode", "zerolatency", "psnr", "ssim" };

  return 
    (std::find(availablePresets.begin(), availablePresets.end(), cfg.preset) != availablePresets.end()) &&
    (std::find(availableTunes.begin(), availableTunes.end(), cfg.tune) != availableTunes.end())
  ;
}

bool parseConfig(
  int argc,
  char **argv,
  Config &config,
  vt::VideoTransformConfig& vConfig
  )
{
  namespace po = boost::program_options;
  try {
    po::options_description generalOptions{"General"};
    generalOptions.add_options()
      ("help,h", "Help screen")
      ("config", po::value<std::string>(), "Config file");
    
    po::options_description fileOptions{"File"};
    fileOptions.add_options()
      ("file-in", po::value<std::string>(&config.fileIn), "input file")
      ("dir-out", po::value<std::string>(&config.dirOut), "output directory (must exist)")
      ("extract-picture", po::value<bool>(&vConfig.actions_.extractPicture_)->default_value(true), "extract picture, default true")
      ("scale-video", po::value<bool>(&vConfig.actions_.scaleVideo_)->default_value(true), "scale video, default true")
      ("width", po::value<size_t>(&vConfig.widthOut)->default_value(320), "output video width, default 320")
      ("height", po::value<size_t>(&vConfig.heightOut)->default_value(240), "output video height, default 240")
      ("frame-rate", po::value<size_t>(&vConfig.frameRateOut)->default_value(25), "output frame rate, default 25 (more frame-rate -> more output size)")
      ("gop-size", po::value<size_t>(&vConfig.gopSize)->default_value(10), "output group-of-picture size, default 10 (more gop -> better quality and bigger output size)")
      ("bit-rate", po::value<size_t>(&vConfig.bitRateOut)->default_value(1024*1024*8), "output video bitrate, default 4*1024*1024")
      ("codec-out", po::value<vt::VtCodec>(&vConfig.codecOut)->default_value(vt::VT_H264), "output codec, default h264, possible values: h264, h263")
      ("codec-in", po::value<vt::VtCodec>(&vConfig.codecIn)->default_value(vt::VT_H264), "input codec, default h264, possible values: h264, h263")
      ("preset", po::value<std::string>(&vConfig.preset)->default_value(std::string("medium")), "preset, default medium, possible values: ultrafast superfast veryfast faster fast medium slow slower veryslow placebo")
      ("tune", po::value<std::string>(&vConfig.tune)->default_value(std::string("fastdecode")), "tune, default fastdecode, possible values: film animation grain stillimage fastdecode zerolatency psnr ssim")
      ("in-pixel-format", po::value<vt::VtPixFormat>(&vConfig.inPixelFormat), "input video pixel format, default yuv420p, possible values yuv420p, bgr24")
      ("out-pixel-format", po::value<vt::VtPixFormat>(&vConfig.outPixelFormat), "input video pixel format, default yuv420p, possible values yuv420p, bgr24")
      ("out-picture-format", po::value<vt::VtPixFormat>(&vConfig.outPictureFormat), "output picture format, default bgr24, possible values yuv420p, bgr24")
    ;
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, generalOptions), vm);
    po::notify(vm);
    if (vm.count("config")) {
      std::ifstream ifs { vm["config"].as<std::string>().c_str() };
      po::variables_map vmf;
 
      if (!ifs) {
        std::cerr << "cannot open config file: " << vm["config"].as<std::string>() << std::endl;
	return false;
      } 

      po::store(po::parse_config_file(ifs, fileOptions), vmf);
      po::notify(vmf);
    }
    
    if (vm.count("help")) {
      std::cout << "command line options: " << std::endl;
      std::cout << generalOptions << std::endl;
      std::cout << "config file options: " << std::endl;
      std::cout << fileOptions << std::endl;
      return false;
    }
  }
  catch (const po::error &ex)
  {
    std::cerr << ex.what() << std::endl;
  }
  return validateConfig(vConfig) && dumpConfig(vConfig, config);
}
