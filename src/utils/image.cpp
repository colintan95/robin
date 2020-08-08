#include "utils/image.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <cstdint>
#include <memory>

namespace utils {

std::shared_ptr<Image> LoadImageFromFile(const std::string& path, bool flip) {
  int load_width = -1;
  int load_height = -1;
  int load_channels = -1;

  stbi_set_flip_vertically_on_load(flip);
  stbi_uc *pixels = stbi_load(path.c_str(), &load_width, &load_height, 
                              &load_channels, 0);
  if (pixels == nullptr) {
    return nullptr;
  }

  auto img = std::make_shared<Image>();
  img->width = static_cast<uint32_t>(load_width);
  img->height  = static_cast<uint32_t>(load_height);

  switch (load_channels) {
    case 3:
      img->format = kImageFormatRGB;
      break;
    case 4:
      img->format = kImageFormatRGBA;
      break;
    default:
      return nullptr;
  }

  uint32_t load_size = load_width * load_height * load_channels;
  img->data = std::vector<uint8_t>(pixels, pixels + load_size);

  return img;
}

} // namespace utils