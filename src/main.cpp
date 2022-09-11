#include "gif_parser.hpp"

#include <vector>
#include <fstream>


int main(int argc, char *argv[])
{
  if (argc != 2)
    std::cout << "Usage: " << argv[0] << " <filename>\n";
  else {
    std::ifstream gif_file (argv[1], std::ios::in | std::ios::binary);
    if (!gif_file.is_open()) {
      std::cout << "Could not open file\n";
      return -1;
    }
    else {
        std::vector<uint8_t> gif_data((std::istreambuf_iterator<char>(gif_file)), 
                                        std::istreambuf_iterator<char>());
        auto gif = parse_gif(&gif_data[0], sizeof(gif_data[0]) * gif_data.size());
        if (gif) {
            std::cout << "Size of image: " << gif->width() << "x" << gif->height() 
                        << " " << gif->frames.size() <<
         "\n";
        } 
    }
  }
    return 1;
}