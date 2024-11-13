// main.cpp
// This program
// <!> Warning: must compile with compiler flag -std=c++17

#include <iostream>
#include <cstdint> // More specific data types (for optimization)
#include <string>
#include <memory> // Unique pointers
#include <windows.h> // UTF-8 (supports greek language and the euro sign)

using RegistrationNumber = std::uint16_t;
using EngineCC = std::uint16_t;
using NumberOfDoors = std::uint8_t;
using MaxTruckWeight = std::uint32_t;
using Tax = std::uint32_t;
using NumberOfVehicles = std::uint8_t;

namespace Config {
  constexpr bool DEVELOPER_MODE{false};
  constexpr bool CONFIDENTIAL_OVERRIDE{false};
}

class Vehicle {
  public:
    Vehicle(RegistrationNumber registration_number, const std::string& owner_name, EngineCC engine_cc)
        : m_registration_number(registration_number), m_owner_name(std::move(owner_name)), m_engine_cc(engine_cc) {
          if constexpr (Config::DEVELOPER_MODE) {
            std::cout << "Vehicle object created: " << *this << '\n';
          }
    }

    virtual ~Vehicle() noexcept {
      if constexpr (Config::DEVELOPER_MODE && Config::CONFIDENTIAL_OVERRIDE) {
        std::cout << "Vehicle object created: " << *this << '\n';
      }
    }

    // Pure virtual function makes Vehicle abstract
    virtual Tax CalculateTrafficTax() const = 0;
    
    static Tax CalculateTotalTax(const std::unique_ptr<Vehicle> vehicles[], std::size_t size, std::size_t index = 0) {
      if (index == size) {
        return 0;
      }

      return vehicles[index]->CalculateTrafficTax() + CalculateTotalTax(vehicles, size, index + 1);
    }

    friend std::ostream& operator<<(std::ostream& os, const Vehicle& vehicle) {
      return os << static_cast<unsigned int>(vehicle.m_registration_number) << ' '
          << vehicle.m_owner_name<< ' ' << vehicle.m_engine_cc;
    }

  protected:
    RegistrationNumber m_registration_number;
    std::string m_owner_name;
    EngineCC m_engine_cc;
};

class Car : public Vehicle {
  public:
    Car(RegistrationNumber registration_number, const std::string& owner_name, EngineCC engine_cc, NumberOfDoors number_of_doors)
        : Vehicle(registration_number, std::move(owner_name), engine_cc), m_number_of_doors(number_of_doors) {
          if constexpr (Config::DEVELOPER_MODE) {
            std::cout << "Car object created: " << *this << '\n';
          }
    }

    ~Car() noexcept override {
      if constexpr (Config::DEVELOPER_MODE && Config::CONFIDENTIAL_OVERRIDE) {
        std::cout << "Car object created: " << *this << '\n';
      }
    }

    Tax CalculateTrafficTax() const override {
      return (m_engine_cc <= 1'000 ? 140 : 140 + ((m_engine_cc - 1'000) / 100) * 10);
    }

    friend std::ostream& operator<<(std::ostream& os, const Car& car) {
      return os << "RN: " << static_cast<unsigned int>(car.m_registration_number)
        << ", Owner: " << car.m_owner_name<< ", CC: " << car.m_engine_cc
        << ", No. of doors: " << static_cast<unsigned int>(car.m_number_of_doors);
    }

  private:
    NumberOfDoors m_number_of_doors;
};

class Truck : public Vehicle {
  public:
    Truck(RegistrationNumber registration_number, const std::string& owner_name, EngineCC engine_cc, MaxTruckWeight max_weight)
        : Vehicle(registration_number, std::move(owner_name), engine_cc), m_max_weight(max_weight) {
          if constexpr (Config::DEVELOPER_MODE) {
            std::cout << "Truck object created: " << *this << '\n';
          }
    }

    ~Truck() noexcept override {
      if constexpr (Config::DEVELOPER_MODE && Config::CONFIDENTIAL_OVERRIDE) {
        std::cout << "Truck object created: " << *this << '\n';
      }
    }

    Tax CalculateTrafficTax() const override {
      return (m_max_weight <= 3'000 ? 300 : m_max_weight <= 6'000 ? 400 : 600);
    }

    friend std::ostream& operator<<(std::ostream& os, const Truck& truck) {
      return os << "RN: " << truck.m_registration_number
          << ", Owner: " << truck.m_owner_name << ", CC: " << truck.m_engine_cc
          << ", Max Weight: " << truck.m_max_weight;
    }

  private:
    MaxTruckWeight m_max_weight;
};

static void LocaleSetup() {
  // Because the locale is not always supported, I enforce it
  SetConsoleOutputCP(CP_UTF8);
  std::locale::global(std::locale(""));  // Uses the default locale with UTF-8
  std::cout.imbue(std::locale());
}

template<typename T, typename... ExtraArgs>
std::unique_ptr<Vehicle> CreateVehicle(ExtraArgs&&... extra_args) {
  RegistrationNumber registration_number;
  std::string owner_name;
  EngineCC engine_cc;

  std::cout << "3) Enter registration number: ";
  std::cin >> registration_number;
  std::cin.ignore();

  std::cout << "4) Enter owner's name: ";
  std::getline(std::cin, owner_name);
  
  std::cout << "5) Enter engine's cc: ";
  std::cin >> engine_cc;

  return std::make_unique<T>(registration_number, std::move(owner_name), engine_cc, std::forward<ExtraArgs>(extra_args)...);
}

void CollectVehicles(std::unique_ptr<Vehicle>* vehicles, std::size_t size){
  for (std::size_t i{0}; i < size; ++i) {
    int choice;
    std::cout << "\nCreate vehicle " << i + 1 << ":\n";
    std::cout << "1) Enter 1 for car, 2 for Truck: ";
    std::cin >> choice;

    switch (choice) {
      case 1:
        NumberOfDoors number_of_doors;
        std::cout << "2) Enter number of doors: ";
        std::cin >> number_of_doors;
        vehicles[i] = CreateVehicle<Car>(number_of_doors);
        break;

      case 2:
        MaxTruckWeight max_weight;
        std::cout << "2) Enter max weight: ";
        std::cin >> max_weight;
        vehicles[i] = CreateVehicle<Truck>(max_weight);
        break;

      default:
        std::cerr << "Invalid choice. Please, try again.\n";
        --i;
    }
  }
}


int main() {
  LocaleSetup(); // To display the euro sign in cmd

  constexpr NumberOfVehicles kNumberOfVehicles{5};
  std::unique_ptr<Vehicle> vehicles[kNumberOfVehicles];

  CollectVehicles(vehicles, kNumberOfVehicles);

  std::cout << "\nSummary of vehicles:\n";
  for (std::size_t i{0}; i < kNumberOfVehicles; ++i) {
    std::cout << *vehicles[i] << " - €" << vehicles[i]->CalculateTrafficTax() << '\n';
  }

  std::cout << "\nTotal tax for all vehicles: €"
      << Vehicle::CalculateTotalTax(vehicles, kNumberOfVehicles) << '\n';

  return 0;
}
