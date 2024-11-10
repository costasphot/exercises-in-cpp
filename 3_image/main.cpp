// main.cpp
// This program models a system for managing and validating image properties, calculate 
// the size of images, and sort images based on their size in ascending or descending order.

#include <iostream>
#include <stdexcept>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <optional> // Include compiler flag: -std=c++17

namespace Config {
  constexpr bool DEVELOPER_MODE = false;
  constexpr bool CONFIDENTIAL_OVERRIDE = false;
}

constexpr std::uint8_t kImages = 4;

class Image {
  public:
  // Constructors and Destructor
    Image() : m_width(1920), m_height(1080), m_color_depth(6) {
      if (Config::DEVELOPER_MODE) {
        std::cout << "Default constructor running for image '1920x1080, 6'.\n";
      }
    }

    explicit Image(std::uint16_t width, std::uint16_t height, std::uint8_t color_depth)
        : m_width(width), m_height(height), m_color_depth(color_depth) {
          try {
            ValidateParameters(m_width, m_height, m_color_depth);
            if (Config::DEVELOPER_MODE) {
              std::cout << "Parameterized constructor (3/3) running for image '" << width << 'x' << height << ", " << color_depth << "'.\n";
            }
          } catch (const std::invalid_argument& e) {
            HandleValidationFailure(e);

            // Take corrective action
            m_width = 1920;
            m_height = 1080;
            m_color_depth = 6;
            std::cout << "Fallback dimensions set to '1920x1080, 6'.\n'";
          }
    }

    ~Image() noexcept {
      if (Config::DEVELOPER_MODE && Config::CONFIDENTIAL_OVERRIDE) {
        std::cout << "Destructor running for image '" << m_width << 'x' << m_height << ", " << m_color_depth << "'.\n";
      }
    }

  // Getter
    std::uint8_t GetColorDepth() const {
      return m_color_depth;
    }
  
  // Setter
    void SetColorDepth(std::uint8_t color_depth) {
      try {
        ValidateParameters(std::nullopt, std::nullopt, color_depth); // 'std::nullopt': Do not validate this parameter.
      } catch (const std::invalid_argument& e) {
        HandleValidationFailure(e);
        return;
      }

      m_color_depth = color_depth;
    }

  // Class Method
    unsigned long int GetSize() const {
      return m_width * m_height * static_cast<int>(m_color_depth);
    }

  // Overloaded Operator Functions
    bool operator<(const Image& other_image) const {
      return GetSize() < other_image.GetSize();
    }

    friend std::ostream& operator<<(std::ostream& os, const Image& image) {
      return os << "Width = " << image.m_width << ", Height = " << image.m_height
                << ", Depth = " << static_cast<int>(image.m_color_depth) << ", Size = " << image.GetSize();
    }
  
  // Helper Functions
    void ValidateParameters(std::optional<std::uint16_t> width, std::optional<std::uint16_t> height, std::uint8_t color_depth) {
      // Instead of 'std::optional', use sentinel values (0, -1, etc.) and obviously don't throw an exception for that value.
      // E.g.: I can only compile until C++17 with a compiler flag, while at the same time, the macro '#if __cplusplus >= 201703L' would fail.
      if (color_depth == 0 || color_depth > 128 || (!(color_depth == 1 || color_depth == 3) && (color_depth % 2) != 0)) {
        // We often use: 1 2 4 8 16 24 30 36 48 64 96 128 - the instructions say we need 3
        throw std::invalid_argument("Error: The color depth must be either 1, 3, or an even number between 1 and 128.\n");
      }
      
      if (width.has_value()) {
        // 'std::optional' wraps a value and represents an 'optional' object; it must be dereferenced to access its value.
        if (width.value() < 40 || width.value() > 7'680 || (width.value() % 2) != 0) { // Equivalently: *width
          throw std::invalid_argument("Error: The width must be an even number between 40 and 7,680.\n");
        }
      }

      if (height.has_value()) {
        if (height.value() < 25 || height.value() > 4'320 || (height.value() % 2) != 0) {
          throw std::invalid_argument("Error: The height must be an even number between 25 and 4,320.\n");
        }
      }
    }

    void HandleValidationFailure(const std::invalid_argument& e) {
      if (Config::DEVELOPER_MODE) {
        std::cerr << "[Validation Failure] " << e.what();
      }
    }

  private:
    std::uint16_t m_width;
    std::uint16_t m_height;
    std::uint8_t m_color_depth;
};

bool CompareAscending(const Image& a, const Image& b) {
    return a.GetSize() < b.GetSize();
}

bool CompareDescending(const Image& a, const Image& b) {
    return a.GetSize() > b.GetSize();
}

std::vector<Image> Sort(std::vector<Image>& images) {
  std::sort(images.begin(), images.end(), CompareAscending);
  // Alternatively with lambdas: [](const Image& a, const Image& b) { ... }
  return images;
}

std::vector<Image> ReverseSort(std::vector<Image>& images) {
  std::sort(images.begin(), images.end(), CompareDescending);
  return images;
}

template <typename T>
void PrintVector(const std::vector<T>& vector, const std::string& text) {
  std::cout << text << '\n';
  for (const T& element : vector) {
    std::cout << element << '\n';
  }
}


int main() {
  std::vector<Image> images;
  images.reserve(kImages);

  images.emplace_back(640, 360, 3);
  images.emplace_back(1024, 768, 2);
  images.emplace_back(800, 600, 4);
  images.emplace_back(1280, 1024, 2);
  
  ReverseSort(images);
  PrintVector(images, "Descending Order:");

  std::cout << '\n';

  Sort(images);
  PrintVector(images, "Ascending Order:");

  return 0;
}
