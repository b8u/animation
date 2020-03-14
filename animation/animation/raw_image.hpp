#pragma once
#include <optional>
#include <filesystem>

namespace b8u
{
  struct RawImage
  {
    size_t width = 0;
    size_t height = 0;

    // for debug
    size_t row_size{};

    std::vector<uint8_t> data;


    static std::optional<RawImage> Load(const std::filesystem::path& path);
    static std::optional<RawImage> LoadPng(const std::filesystem::path& path);
  };


}
