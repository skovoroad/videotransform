#include <sstream>
#include <memory>
#include "videotransform.h"
#include "bmp.h"

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
 
    bool handleTransformedVideo(const void *data, size_t size) override {
      ofstr.write(reinterpret_cast<const char *>(data), size);

      return true;
    }
};


int main(int argc, char** argv)
{
  if(argc < 3){
    std::cerr << "Usage: <input file> <output dir>" << std::endl;
    return -1;
  }

  std::string filename = argv[1];
  auto handler = std::make_unique<ExampleVideoTransformHandler>(argv[2]);
  vt::VideoTransformConfig cfg;
  cfg.widthOut=320;
  cfg.heightOut=240;
  auto service = createVideoTransformService(cfg, handler.get());
  if(! service) {
    std::cerr << "Cannot create videotransform service" << std::endl;
    return -1;
  }

  std::ifstream istr (filename, std::ios::in | std::ios::binary);
  
  while(true) {  
   
    if(!istr)
      break;
    constexpr auto bufsize = 4096;
    char buff[bufsize];
    while(istr) {
      istr.read(buff, bufsize);
      auto res = service->doTransform(reinterpret_cast<const void *>(buff), istr.gcount());
      if(res != vt::VT_OK) {
        std::cerr << "Error received: " << int(res) << std::endl;
        return -2;
      }
    }
  }
  
  return 0; 
}
