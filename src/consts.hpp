#pragma once
#include "videotransform.h"

namespace vt {
  namespace vt2av {
     
    VtErrorCode codecId(VtCodec id, AVCodecID& retval) {
      switch (id) {
        case VT_H264:
          retval = AV_CODEC_ID_H264;
          break;
        case VT_H263:
          retval = AV_CODEC_ID_H263;
          break;
        default:
          return VT_UNKNOWN_CODEC;
      };
      return VT_OK;
    }

    VtErrorCode pixFormatId(VtPixFormat id, AVPixelFormat& retval) {
      switch(id) {
        case VT_BGR24:
          retval = AV_PIX_FMT_BGR24;
          break;
        case VT_YUV420P:
          retval = AV_PIX_FMT_YUV420P;
          break;
        default:
          return VT_UNKNOWN_PIX;
      }
      return VT_OK;
    }
  } // namespace vt2av
} //namespace vt

