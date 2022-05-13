#include "qoi.h"
// #define QOI_IMPLEMENTATION
// #include "qoi_ref.h"

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>


int main(int, char* argv[]) {
  const std::filesystem::path input_path{argv[1]};
  const int input_file_size = std::filesystem::file_size(input_path);

  std::cout << "compressed size: " << input_file_size << std::endl;

  std::ifstream input_file{input_path, std::ios::in | std::ios::binary};

  uint8_t* input_buffer = new uint8_t[input_file_size];
  input_file.read(reinterpret_cast<char*>(input_buffer), input_file_size);

  qoi_header header;
  qoi_decode_header(input_buffer, &header);

  std::cout << "width: "    << header.width
            << " height: "   << header.height
            << " channels: " << static_cast<int>(header.channels) << std::endl;

  // uint8_t* output_buffer = new uint8_t[header.width * header.height * 4];

  cv::Mat image(header.height, header.width, CV_8UC4);

  auto t0 = std::chrono::steady_clock::now();
  qoi_decode(input_buffer, image.data, header);
  auto t1 = std::chrono::steady_clock::now();

  std::cout << "uncompressed size: " << header.width * header.height * static_cast<int>(header.channels) << std::endl;
  
  std::cout << "compression ratio: " << static_cast<float>(input_file_size) / (header.width * header.height * static_cast<int>(header.channels))
            << " time: " << std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count() << std::endl;

  // qoi_desc header;
  // auto output_buffer = qoi_read(input_path.c_str(), &header, 4);

  // cv::Mat image(header.height, header.width, CV_8UC4, reinterpret_cast<void*>(output_buffer));
  cv::cvtColor(image, image, cv::COLOR_RGBA2BGRA);
  cv::imshow("image", image);
  cv::waitKey();
}
