#pragma once
#include <string>
namespace vt {

  enum VtCodec {
    VT_CODEC_UNKNOWN,
    VT_H264,
    VT_H263,
  };

  enum VtPixFormat {
    VT_YUV420P,
    VT_BGR24
  };

  enum VtErrorCode {
    VT_OK,
    VT_UNKNOWN_ERROR,
    VT_UNKNOWN_CODEC,
    VT_UNKNOWN_PIX,
    VT_CANNOT_CREATE_SCALE_CTX,
    VT_CANNOT_ALLOC_IMAGE,
    VT_CONTEXT_NOT_INITED,
    VT_CANNOT_PARSE_FRAME,
    VT_CANNOT_DECODE_FRAME,
    VT_CANNOT_WRITE_FRAME,
    VT_CANNOT_ENCODE_FRAME,
    VT_INIT_CTX_ERROR,
    VT_CANNOT_OPEN_ENCODER,
    VT_CANCELED_BY_USER
  };

  struct VideoTransformConfig {
    struct {
      bool extractPicture_ = true;
      bool scaleVideo_ = true;  
      bool makeAvi_ = true;  
    } actions_;
    VtCodec codecIn = VT_H264;
    VtCodec codecOut = VT_H264;
    size_t widthOut = 0;
    size_t heightOut = 0;

    size_t frameRateOut = 25;  // + more frame rate - more out bytes
    size_t gopSize = 10; // + the number of pictures in a group of pictures - more gop - better quality and more bytes
    size_t bitRateOut = 1024*1024*8;
    
    std::string preset = "veryfast"; // ultrafast superfast veryfast faster fast medium (default) slow slower veryslow placebo
    std::string tune = "fastdecode"; // film animation grain stillimage fastdecode zerolatency psnr ssim

    VtPixFormat inPixelFormat = VT_YUV420P; // can we detect it from picture?
    VtPixFormat outPixelFormat = VT_YUV420P;
    VtPixFormat outPictureFormat = VT_BGR24;
  };

  class VideoHandler {
    public:
      virtual  bool handleExtractedPicture(const void *, size_t nbytes, size_t w, size_t h) = 0;
      virtual  bool handleScaledVideo(const void *, size_t) = 0;
      virtual  bool handleAvi(const void *, size_t) = 0;
  };

  class VideoTransformService {
    public:
      virtual  VtErrorCode addAudio(uint32_t ts, const void *, size_t) = 0;
      virtual  VtErrorCode addVideo(uint32_t ts, const void *, size_t) = 0;
  };

  VideoTransformService*  createVideoTransformService(
      const VideoTransformConfig& vcfg,
      VideoHandler* vh
  );

}
