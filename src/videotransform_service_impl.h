#pragma once
#include <memory>
#include "videotransform.h"
#include "ring_buffer.h"

namespace vt {

  class VideoTransformServiceImpl: public VideoTransformService {
    public:
      VtErrorCode init(const VideoTransformConfig&, VideoHandler*);
      VtErrorCode addAudio(uint32_t, const void*, size_t) override;
      VtErrorCode addVideo(uint32_t, const void*, size_t) override;
    private:
      VtErrorCode extractPicture();
      VtErrorCode scaleVideo();
      RingBuffer<char, 4*1024> buffer_;
      struct TransformCtx;
      std::unique_ptr<TransformCtx> ctx_;
      VideoHandler* handler_ = nullptr;
      VideoTransformConfig cfg_;
  };

} // namespace vt
