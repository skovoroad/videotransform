#pragma once
#include <vector>
template<typename TElem, size_t Size>
class RingBuffer {
public:
  RingBuffer() {
    //reset();
    buffer_.reserve(Size);
  }

  bool push(const TElem *pbuffer, size_t nbytes) {
    buffer_.insert(buffer_.end(), pbuffer, pbuffer + nbytes);
    return true;
  }

  bool consume(size_t nbytes) {
    buffer_.erase(buffer_.begin(), buffer_.begin() + nbytes);
    return true;
  }

  TElem *data() {
    return buffer_.data();
  }

  size_t size() {
    return buffer_.size();
  }

  bool empty() {
    return buffer_.empty();
  }

  void reset() {
     buffer_.clear();
  }
private:
/*  TElem buffer_[Size];
  TElem * data_ = nullptr;
  size_t dataSize_ = 0;*/
  std::vector<TElem> buffer_;

};
/*
template<typename TElem, size_t Size>
class RingBuffer {
public:
  RingBuffer() {
    reset();
  }

  bool push(const TElem *pbuffer, size_t nbytes) {
    if (dataSize_ + nbytes > Size) // no enough room 
      return false;

    if (data_ + dataSize_ + nbytes > buffer_ + Size) { // end of buffer reached, need to shift old data to head
      if (data_ - buffer_ < dataSize_) { // cannot shift tail to head
        reset();
        return false;
      }

      memcpy(buffer_, data_, dataSize_);
      data_ = buffer_;
    }

    memcpy(data_ + dataSize_, pbuffer, nbytes);
    dataSize_ += nbytes;
    return true;
  }

  bool consume(size_t nbytes) {
    if (nbytes > dataSize_) {
      reset();
      return false;
    }

    data_ += nbytes;
    dataSize_ -= nbytes;
    return true;
  }

  TElem *data() {
    return data_;
  }

  size_t size() {
    return dataSize_;
  }

  bool empty() {
    return dataSize_ == 0;
  }

  void reset() {
    data_ = buffer_;
    dataSize_ = 0;
  }
private:
  TElem buffer_[Size];
  TElem * data_ = nullptr;
  size_t dataSize_ = 0;

};
*/