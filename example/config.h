#pragma once

#include "videotransform.h"

struct Config {
  std::string fileIn;
  std::string dirOut;
};

bool parseConfig(
  int argc,
  char **argv,
  Config &config,
  vt::VideoTransformConfig& vConfig
  );

