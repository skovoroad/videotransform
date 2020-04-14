extern "C" {
  #include <libavcodec/avcodec.h>
  #include <libswscale/swscale.h>
  #include <libavutil/frame.h>
  #include <libavutil/opt.h>
  #include <libavutil/pixfmt.h>
  #include <libavformat/avformat.h>
  #include <libavutil/imgutils.h>
  #include <libavutil/parseutils.h>
  #include <libswscale/swscale.h>
}

#include <memory>
#include "videotransform_service_impl.h"

namespace vt {

  template <typename ExitCallback>
  struct ScopeGuard {
    ExitCallback exit_callback_;
    bool disable_ = false;

    ScopeGuard(ExitCallback ec)
      : exit_callback_(ec) {
    };

    ~ScopeGuard() {
      if(!disable_)
        exit_callback_();
    }
  };

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

  VideoTransformService* createVideoTransformService(
      const VideoTransformConfig& vcfg,
      VideoHandler* vh)
  {
    auto retval = std::make_unique<VideoTransformServiceImpl>();
    if(retval->init(vcfg, vh) != VT_OK){
      retval.reset();
      return nullptr;
    }
    return retval.release();
  }

  struct VideoTransformServiceImpl::TransformCtx {
    
    TransformCtx() {
    };

    bool init(const VideoTransformConfig& cfg) {
      ScopeGuard guard( [&](){ free(); } );
      cfg_ = cfg;

      if( vt2av::codecId(cfg.codecIn, codecId_) != VT_OK ||
          vt2av::codecId(cfg.codecOut, codecIdOut_) != VT_OK  ||
          vt2av::pixFormatId(cfg.inPixelFormat, inFormat_) != VT_OK  ||
          vt2av::pixFormatId(cfg.outPictureFormat, outPicFormat_) != VT_OK ||
          vt2av::pixFormatId(cfg.outPixelFormat, outFormat_) != VT_OK )
        return false;

      decodeCodec_ = avcodec_find_decoder(codecId_);
      if(!decodeCodec_)
        return false;

      parser_ = av_parser_init(codecId_);
      if (!parser_)
        return false;

      decodeContext_ = avcodec_alloc_context3(decodeCodec_);
      if (!decodeContext_)
        return false;

      if( avcodec_open2(decodeContext_, decodeCodec_, nullptr) < 0)
        return false;

      receivedFrame_ = av_frame_alloc();
      if (!receivedFrame_)
        return false;

      if(cfg_.actions_.transformVideo_) {
	encodeCodec_ = avcodec_find_encoder(codecIdOut_);
	if (!encodeCodec_)
	  return false;

     }

      guard.disable_ = true;
      return  true;
    }

    ~TransformCtx() {
      free();
    }

    void free() {
      if (decodeContext_) {
        avcodec_close(decodeContext_);
        av_free(decodeContext_);
        decodeCodec_ = nullptr;
        decodeContext_ = nullptr;
      }

      if (encodeContext_) {
        avcodec_close(encodeContext_);
        av_free(encodeContext_);
        encodeContext_ = nullptr;
      }

      if (receivedFrame_) {
        av_free(receivedFrame_);
        receivedFrame_ = NULL;
      }

      if (scaledFrame_) {
        av_free(scaledFrame_);
        scaledFrame_ = NULL;
      }

      if (parser_) {
        av_parser_close(parser_);
        parser_ = NULL;
      }

      if(no_scale_ctx) {
        sws_freeContext(no_scale_ctx);
	no_scale_ctx = 0;
      }

     if(scale_ctx) {
        sws_freeContext(scale_ctx);
	scale_ctx = 0;
      }

    }

    VtErrorCode initScaleCtx(size_t w, size_t h) {
      if(cfg_.actions_.transformVideo_){ 
        float wScaleFactor = static_cast<float>(cfg_.widthOut) / w;
        float hScaleFactor = static_cast<float>(cfg_.heightOut) / h;	
	float scaleFactor = w * hScaleFactor <= cfg_.widthOut ? hScaleFactor: wScaleFactor;
        cfg_.widthOut = w * scaleFactor;
        cfg_.heightOut = h * scaleFactor;
	
	scale_ctx = sws_getContext(
	  w,
	  h,
	  inFormat_,
	  cfg_.widthOut,
	  cfg_.heightOut,
	  outFormat_,
	  SWS_BILINEAR,
	  NULL, NULL, NULL
	);

	if (!scale_ctx)
	  return VT_CANNOT_CREATE_SCALE_CTX;
	
	encodeContext_ = avcodec_alloc_context3(encodeCodec_);
	if (!encodeContext_)
	  return VT_CANNOT_OPEN_ENCODER;

	encodeContext_->width = cfg_.widthOut;
	encodeContext_->height = cfg_.heightOut;
	encodeContext_->time_base = AVRational { 1, 25 };
	encodeContext_->framerate = AVRational { static_cast<int>(cfg_.frameRateOut), 1 };
	encodeContext_->gop_size = cfg_.gopSize;
	encodeContext_->pix_fmt = outFormat_;
	encodeContext_->bit_rate = cfg_.bitRateOut; // +
	encodeContext_->profile = FF_PROFILE_H264_MAIN;

	if (encodeCodec_->id == AV_CODEC_ID_H264) {
	  av_opt_set(encodeContext_->priv_data, "preset", cfg_.preset.c_str(), 0);
	  av_opt_set(encodeContext_->priv_data, "tune", cfg_.tune.c_str(), 0);
	}
	if( avcodec_open2(encodeContext_, encodeCodec_, nullptr) < 0)
	  return VT_CANNOT_OPEN_ENCODER;

	scaledFrame_ = av_frame_alloc();
	if (!scaledFrame_)
	  return VT_CANNOT_ALLOC_IMAGE;

	scaledFrame_->format = outFormat_;
	scaledFrame_->width = cfg_.widthOut;
	scaledFrame_->height = cfg_.heightOut;

	if (av_frame_get_buffer(scaledFrame_, 1) < 0)
	  return VT_CANNOT_ALLOC_IMAGE;
 
	if (av_image_alloc(
	      scaledFrame_->data, 
	      scaledFrame_->linesize, 
	      cfg_.widthOut, 
	      cfg_.heightOut, 
	      outFormat_, 
	      1) < 0)
	  return VT_CANNOT_ALLOC_IMAGE;
       }
      return VT_OK;
    }

    VtErrorCode initNoScaleCtx(size_t w, size_t h) {
      no_scale_ctx = sws_getContext(
        w,
        h,
        inFormat_,
        w,
        h,
        outPicFormat_,
        SWS_BILINEAR,
        NULL, NULL, NULL
      );

      if (!no_scale_ctx)
        return VT_CANNOT_CREATE_SCALE_CTX;

      return VT_OK;
    }


    AVCodecID codecId_ = AV_CODEC_ID_H264;
    AVCodecID codecIdOut_ = AV_CODEC_ID_H264;
    AVPixelFormat inFormat_ = AV_PIX_FMT_YUV420P;
    AVPixelFormat outFormat_ = AV_PIX_FMT_YUV420P;
    AVPixelFormat outPicFormat_ = AV_PIX_FMT_BGR24;
    VideoTransformConfig cfg_;

    AVCodec* decodeCodec_ = nullptr;
    AVCodec* encodeCodec_ = nullptr;
    AVCodecContext* decodeContext_ = nullptr;
    AVCodecContext* encodeContext_ = nullptr;
    AVCodecParserContext* parser_ = nullptr;
    AVFrame* receivedFrame_ = nullptr;
    AVFrame* scaledFrame_ = nullptr;
    SwsContext *scale_ctx = nullptr;
    SwsContext *no_scale_ctx = nullptr;
  };

  VtErrorCode VideoTransformServiceImpl::init(
      const VideoTransformConfig& cfg,
      VideoHandler* h){

    ScopeGuard guard([&]() {
        ctx_.reset();
        }
      );
    cfg_ = cfg; 
    ctx_.reset(new TransformCtx);
    auto res = ctx_->init(cfg);
    if(!res)
      return VT_INIT_CTX_ERROR;
    handler_ = h;
    guard.disable_ = true;
    return VT_OK;
  }

  VtErrorCode VideoTransformServiceImpl::transformVideo() {
    if (!ctx_->scale_ctx) {
      auto isRes = ctx_->initScaleCtx(ctx_->receivedFrame_->width, ctx_->receivedFrame_->height);
      if(isRes != VT_OK)
	return isRes;
    }

    if(av_frame_make_writable(ctx_->scaledFrame_)< 0) 
      return VT_CANNOT_WRITE_FRAME;

    sws_scale(
      ctx_->scale_ctx, 
      ctx_->receivedFrame_->data, 
      ctx_->receivedFrame_->linesize, 
      0, 
      ctx_->receivedFrame_->height, 
      ctx_->scaledFrame_->data, 
      ctx_->scaledFrame_->linesize
    );

    ctx_->scaledFrame_->pts += 1;
    auto ret = avcodec_send_frame(ctx_->encodeContext_, ctx_->scaledFrame_);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
      return VT_OK;
    if(ret < 0)
      return VT_CANNOT_ENCODE_FRAME;

    while (ret >= 0) {
      AVPacket avpktOut;
      av_init_packet(&avpktOut);
      avpktOut.data = nullptr;
      avpktOut.size = 0;
      ScopeGuard guardPkt([&]() {     av_packet_unref(&avpktOut) ;});

      ret = avcodec_receive_packet(ctx_->encodeContext_, &avpktOut);
      if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
	break;
      if( ret < 0)
	return VT_CANNOT_ENCODE_FRAME;

      if(!handler_->handleTransformedVideo(avpktOut.data, avpktOut.size))
	return VT_CANCELED_BY_USER;
    }
    return VT_OK;
  }

  VtErrorCode VideoTransformServiceImpl::extractPicture() {
    if (!ctx_->no_scale_ctx) {
      auto isRes = ctx_->initNoScaleCtx(ctx_->receivedFrame_->width, ctx_->receivedFrame_->height);

      if(isRes != VT_OK)
	return isRes;
    }

    uint8_t *dst_data[4];
    int dst_linesize[4];
    int dst_bufsize = av_image_alloc(
      dst_data, 
      dst_linesize, 
      ctx_->receivedFrame_->width, 
      ctx_->receivedFrame_->height, 
      ctx_->outPicFormat_, 
      1);
    
    if (dst_bufsize < 0)
      return VT_CANNOT_DECODE_FRAME;
    
    sws_scale(
      ctx_->no_scale_ctx, 
      ctx_->receivedFrame_->data, 
      ctx_->receivedFrame_->linesize, 
      0, 
      ctx_->receivedFrame_->height,
      dst_data, 
      dst_linesize 
     );

     if(! handler_->handleExtractedPicture(
	dst_data[0],  
	dst_bufsize ,
	ctx_->receivedFrame_->width, 
	ctx_->receivedFrame_->height
	))
       return VT_CANCELED_BY_USER;
     
     return VT_OK; 
   }

  VtErrorCode VideoTransformServiceImpl::doTransform(
      const void * buff,
      size_t size){
    
    buffer_.push (reinterpret_cast<const char *>(buff), size);
    if(!ctx_)
      return VT_CONTEXT_NOT_INITED;

    while (!buffer_.empty()) {

      AVPacket avpkt;
      av_init_packet(&avpkt);
      avpkt.data = nullptr;
      avpkt.size = 0;

      auto parsed_bytes = 0;

      ScopeGuard guard([&]() {
        if(parsed_bytes >= 0) 
          buffer_.consume(parsed_bytes);
        av_packet_unref(&avpkt);
        }
      );

      parsed_bytes = av_parser_parse2(
        ctx_->parser_, 
        ctx_->decodeContext_, 
        &avpkt.data, 
        &avpkt.size, 
        reinterpret_cast<const unsigned char *>(buffer_.data()), 
        buffer_.size(), 
        AV_NOPTS_VALUE, 
        AV_NOPTS_VALUE, 
        0
      );
  
      if (parsed_bytes < 0) 
        return VT_CANNOT_PARSE_FRAME;

      if (avpkt.size <= 0) 
        continue; 
      
      auto ret = avcodec_send_packet(ctx_->decodeContext_, &avpkt);

      if (ret < 0) 
        return VT_CANNOT_DECODE_FRAME;

      while (ret >= 0) {
        ret = avcodec_receive_frame(ctx_->decodeContext_, ctx_->receivedFrame_);
        if( ret == AVERROR(EAGAIN) || ret == AVERROR_EOF )
          break;

        if( ret < 0 )
          return VT_CANNOT_DECODE_FRAME;
	
	ctx_->receivedFrame_->pts += 1;
        
	if(cfg_.actions_.extractPicture_) {
          auto extractRes = extractPicture();
	  if(extractRes != VT_OK)
	    return extractRes;
        }

        if(cfg_.actions_.transformVideo_) {
          auto transformRes = transformVideo();
	  if(transformRes != VT_OK)
	    return transformRes;
	}

      } // while decode
    
    } // while not empty buffer
    return VT_OK;
  }
}//namespace


