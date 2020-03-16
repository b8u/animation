#include "raw_image.hpp"

#include <png.h>
#include <cassert>


namespace fs = std::filesystem;

namespace b8u
{

// TODO: png_set_read_fn() and png_set_write_fn()
struct FileDeleter
{ 
  void operator()(FILE *ptr) const noexcept
  {
    if (ptr) {
      fclose(ptr);
    }
  }
};

std::optional<RawImage> RawImage::Load(const fs::path& path)
{
                               
  const auto& extension = path.extension();
  if (extension == ".png") {
    return LoadPng(path);
  }
  return std::nullopt;
}

std::optional<RawImage> RawImage::LoadPng(const fs::path& png_path)
{
  std::unique_ptr<FILE, FileDeleter> fp{fopen( png_path.string().c_str(), "rb"), FileDeleter{}};
  if (!fp) return std::nullopt;

  // Лучше не трогать. В этом деле замешен еще info_ptr.
  png_structp png_ptr = png_create_read_struct(
      PNG_LIBPNG_VER_STRING,
      nullptr,  // error_ptr
      nullptr,  // error_fn
      nullptr); // warn_fn

  if (!png_ptr) return std::nullopt;

  png_infop info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr) {
    png_destroy_read_struct(&png_ptr, nullptr, nullptr);
    return std::nullopt;
  }

  if (setjmp(png_jmpbuf(png_ptr))) {
    png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
    return std::nullopt;
  }

  // TODO: better to set functions.
  png_init_io(png_ptr, fp.get());

  // We didn't read anything, did we?
  //png_set_sig_bytes(png_ptr, sig_read);

  png_read_png(
      png_ptr,
      info_ptr,
      PNG_TRANSFORM_STRIP_16 |  // Strip 16-bit samples to 8 bits
      PNG_TRANSFORM_PACKING |   // Expand 1, 2 and 4-bit samples to bytes
      PNG_TRANSFORM_EXPAND,     // Perform set_expand() ?? png_set_palette_to_rgb ??
      nullptr // not used (in library)
  );


  struct png_info
  {
    png_uint_32 width{};
    png_uint_32 height{};
    int color_type{};
    int interlace_type{};
    int bit_depth{};
  };

  

  const png_info img_info = [](png_structp png_ptr, png_infop info_ptr) {
    png_info info;

    png_get_IHDR(png_ptr, info_ptr, &info.width, &info.height, &info.bit_depth, &info.color_type, &info.interlace_type,
        nullptr, // compression_method
        nullptr  // filter_method
    );
    
    return info;
  }(png_ptr, info_ptr);

  RawImage image{img_info.width, img_info.height};

  //assert(color_type == PNG_COLOR_TYPE_RGB);
  //assert(bit_depth == 8); // per channel

  const size_t row_size = png_get_rowbytes(png_ptr, info_ptr);
  image.row_size = row_size;

  image.data.reserve(row_size * image.height);

  png_bytepp row_pointers = png_get_rows(png_ptr, info_ptr);
  for (size_t i = 0; i < image.height; ++i) {
    std::copy(row_pointers[i], row_pointers[i] + row_size, std::back_inserter(image.data));
  }

  png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);

  return {std::move(image)};
}

}
