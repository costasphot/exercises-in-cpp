// main.cpp
// This program models a simple system for validating and calculating the volume of spheres.

#include <iostream>
#include <stdexcept>
#include <cstdint>
#include <cmath>
#include <vector>
#include <array>
#include <limits>

namespace Config {
  constexpr bool DEVELOPER_MODE = false;
  constexpr bool CONFIDENTIAL_OVERRIDE = false;
}

// Because 'std::acos' is not a 'constexpr' function, I can't write: constexpr double PI = std::acos(-1); by including the 'cmath' library.
// Don't have C++20 to use: constexpr double PI = std::numbers::pi; with the 'numbers' library.
constexpr double PI = 3.14159265358979323846;
constexpr std::uint8_t kSpheres = 5;

class Sphere {
  public:
  // Constructors and Destructor
    Sphere() : m_radius(10) {
      if (Config::DEVELOPER_MODE) {
        std::cout << "Default constructor running for sphere with radius: 10\n";
      }
    }

    explicit Sphere(double radius) : m_radius(radius) {
      try {
        ValidateParameter(m_radius);
        if (Config::DEVELOPER_MODE) {
          std::cout << "Parameterized constructor (1/1) running for sphere with radius: " << radius << '\n';
        }
      } catch (const std::invalid_argument& e) {
        HandleValidationFailure(e);

        // Take corrective action
        m_radius = 10;
        std::cout << "Fallback radius set to: " << m_radius << '\n';
      }
    }

    ~Sphere() noexcept {
      if (Config::DEVELOPER_MODE && Config::CONFIDENTIAL_OVERRIDE) {
        std::cout << "Destructor running for sphere with radius: " << m_radius << "\n";
      }
    }

  // Setter
    void SetRadius(double radius) {
      try {
        ValidateParameter(radius);
      } catch (const std::invalid_argument& e) {
        HandleValidationFailure(e);
        return;
      }

      m_radius = radius;
    }

  // Getter
    double GetRadius() const {
      return m_radius;
    }
  
  // Class Methods
    double CalculateVolume() const {
      return (4.0 / 3.0) * PI * std::pow(m_radius, 3);
    }

  // Overloaded Operator Function
    friend std::ostream& operator<<(std::ostream& os, const Sphere& sphere) {
      return os << "Radius = " << sphere.m_radius << ", Volume = " << sphere.CalculateVolume();
      // I could use a key for differentiating between objects with the same radius - even this is too much for such a small program
    }

  // Helper Functions
    void ValidateParameter(double radius) {
      if (radius <= 0 || radius > 1'025'867) {
        throw std::invalid_argument("Error: The radius must be greater than zero and less than a million.\n");
      }
    }

    void HandleValidationFailure(const std::invalid_argument& e) {
      if (Config::DEVELOPER_MODE) {
        std::cerr << "[Validation Failure] " << e.what();
      }
    }

  private:
    double m_radius;
};

static double CalculateAverageVolume(const std::array<double, kSpheres>& volumes) { 
  double average_volume{0.0};
  for (double volume : volumes) { // We need to know the size of the array
    average_volume += volume;
  }

  return average_volume / kSpheres;
}

static bool IsValidSphere(double& radius) {
  if (!(std::cin >> radius)) {
    std::cerr << "Error: Non-numeric radius entered; please, try again.\n";
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return false;
  }

  return true;
}


int main() {
  // Effectively handle a vector
  std::vector<Sphere> spheres;
  spheres.reserve(kSpheres);

  std::array<double, kSpheres> volumes{{}};

  {
    double input_radius{0.0};
    for (int i{0}; i < kSpheres; ++i) {
      std::cout << "Enter the radius for sphere " << (i + 1) << ": ";
      if (!IsValidSphere(input_radius)) {
        --i;
        continue;
      }

      spheres.emplace_back(input_radius);
      volumes[i] = spheres.back().CalculateVolume(); // Same as creating a 'Sphere' object and doing 'spheres[i].SetRadius(radius);'
    }

    for (const Sphere& sphere : spheres) {
      std::cout << sphere << '\n';
    }
  }

  std::cout << "Average Volume = " << CalculateAverageVolume(volumes) << '\n';

  return 0;
}
