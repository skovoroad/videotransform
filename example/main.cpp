#include <sstream>
#include <memory>
#include "videotransform.h"
#include "bmp.h"
#include "config.h"

class ExampleVideoTransformHandler : public vt::VideoHandler{
    std::string dir_;
    size_t bmpCounter_ = 0;
    std::ofstream ofstr;
   public:
    ExampleVideoTransformHandler(const char *dir) {
       dir_ = dir;
      std::stringstream fname;
      fname << dir_ << "/out.h264"; 
      std::string h264fname = fname.str(); 
      ofstr.open(h264fname.c_str(), std::ios::out | std::ios::binary);
   }

    bool handleExtractedPicture(
       const void *data, 
        size_t nbytes,
        size_t w, 
        size_t h)  override 
    {
      std::stringstream fname;
      fname << dir_ << "/" << std::to_string(bmpCounter_++) << ".bmp"; 
      std::string bmpfname = fname.str();   
      if(! write_bmp(bmpfname.c_str(), reinterpret_cast<const uint8_t *>(data), nbytes, w, h))
        std::cerr << "cannot write file " << bmpfname << std::endl;   
      return true;
    }
 
    bool handleScaledVideo(const void *data, size_t size) override {
      ofstr.write(reinterpret_cast<const char *>(data), size);

      return true;
    }

    bool handleAvi(const void *, size_t) override {
      return true;
    }
};


int main(int argc, char** argv)
{
  Config cfg;
  vt::VideoTransformConfig vCfg;
  std::string fileIn;
  std::string dirOut;
  if (! parseConfig( argc, argv, cfg, vCfg) ) {
    std::cerr << "configuration error, exit" << std::endl;
    return -1;
  }

  auto handler = std::make_unique<ExampleVideoTransformHandler>(cfg.dirOut.c_str());
  auto service = createVideoTransformService(vCfg, handler.get());
  if(! service) {
    std::cerr << "Cannot create videotransform service" << std::endl;
    return -1;
  }

  std::ifstream istr (cfg.fileIn, std::ios::in | std::ios::binary);
  
  if(!istr) {
    std::cerr << "cannot open file " << cfg.fileIn << std::endl;
    return -2;
  }
  constexpr auto bufsize = 4096;
  char buff[bufsize];
  uint32_t ts = 0;
  while(istr) {
    istr.read(buff, bufsize);
    auto res = service->addVideo(++ts, reinterpret_cast<const void *>(buff), istr.gcount());
    if(res != vt::VT_OK) {
      std::cerr << "Error received: " << int(res) << std::endl;
      return -2;
    }
  }

  return 0; 
}
