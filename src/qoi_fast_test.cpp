#include "qoi_fast.h"

#include <chrono>
#include <filesystem>
#include <fstream>

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>

static uint8_t* read_file(const std::filesystem::path path, size_t* size) {
  *size = std::filesystem::file_size(path);
  uint8_t* buffer = new uint8_t[*size];
  std::ifstream file{path, std::ios::in | std::ios::binary};
  file.read(reinterpret_cast<char*>(buffer), *size);
  return buffer;
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    return 1;
  }

  const std::filesystem::path path{argv[1]};

  size_t size;
  uint8_t* input = read_file(path, &size);

  if (!input) {
    return 1;
  }

  qoi_header header;
  if (!qoi_decode_header(input, &header)) {
    return 1;
  }

  cv::Mat output(header.height, header.width, CV_8UC(header.channels));
  auto t0 = std::chrono::steady_clock::now();
  if (!qoi_decode(input, output.data, size)) {
    return 1;
  }
  auto t1 = std::chrono::steady_clock::now();

  std::cout << std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count() << std::endl;

  if (header.channels == 3) {
    cv::cvtColor(output, output, cv::COLOR_RGB2BGR);
  }
  else if (header.channels == 4) {
    cv::cvtColor(output, output, cv::COLOR_RGBA2BGRA);
  }
  else {
    return 1;
  }

  cv::imshow("image", output);
  cv::waitKey();

  return 0;
}