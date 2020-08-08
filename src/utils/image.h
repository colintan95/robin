#ifndef UTILS_IMAGE_H_
#define UTILS_TEXTURE_H_

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace utils {

enum class ImageFormat {
  kInvalid = -1,
  kRGB = 0,
  kRGBA
};

struct Image {
  ImageFormat format = ImageFormat::kInvalid;

  uint32_t width = 0;
  uint32_t height = 0;

  std::vector<uint8_t> data;
};

std::shared_ptr<Image> LoadImageFromFile(const std::string& path, bool flip);

} // namespace utils

#endif // UTILS_IMAGE_H_