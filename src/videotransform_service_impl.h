#pragma once
#include <memory>
#include "videotransform.h"
#include "ring_buffer.h"

namespace vt {

  class VideoTransformServiceImpl: public VideoTransformService {
    public:
      VtErrorCode init(const VideoTransformConfig&, VideoHandler*);
      VtErrorCode doTransform(const void*, size_t) override;
    private:
      RingBuffer<char, 4*1024> buffer_;
      struct TransformCtx;
      std::unique_ptr<TransformCtx> ctx_;
      VideoHandler* handler_ = nullptr;
      VideoTransformConfig cfg_;
  };

} // namespace vt
