// main.cpp
// This program models a simple system for validating and displaying Address and Person objects.
// <!> Warning: must compile with compiler flag -std=c++17

#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <optional>
#include <system_error>
#include <variant>
#include <map>
#include <windows.h>

#if __cplusplus >= 202002L
  #include <format>
  #define FORMAT_ENABLED 1
#else
  #define FORMAT_ENABLED 0
#endif // __cplusplus

enum class ValidationError {
  None = 0,
  EmptyStreet,
  EmptyCity,
  InvalidPostalCode,
  EmptyName,
  InvalidAge
};

namespace Config {
  // Do not use 'if constexpr' if I wanted to implement runtime toggling in the future.
  constexpr bool DEVELOPER_MODE{true};
  constexpr bool DEBUG_MODE{false};
  constexpr bool CONFIDENTIAL_OVERRIDE{false};
  
  // Maximum length for log entries before truncation
  constexpr std::uint16_t MAX_LOG_LENGTH{50};
}

namespace AddressDefaults {
  constexpr const char* STREET_UNSET{"Unset"};
  constexpr const char* CITY_UNSET{"Unset"};
  constexpr int POSTAL_CODE_UNSET{-1};
}

namespace PersonDefaults {
  constexpr const char* NAME_UNSET{"Unset"};
  constexpr std::uint8_t AGE_UNSET{0};
}

class Validator {
  public:
    static std::optional<ValidationError> ValidateAddress(const std::string& street, const std::string& city, int postal_code) {
      if (street.empty()) return ValidationError::EmptyStreet;
      if (city.empty()) return ValidationError::EmptyCity;
      if (postal_code < 1 || postal_code > 99950) return ValidationError::InvalidPostalCode;
      return std::nullopt; // Validation successful
    }

    static std::optional<ValidationError> ValidatePerson(const std::string& name, std::uint8_t age) {
      if (name.empty()) return ValidationError::EmptyName;
      if (age == 0 || age > 120) return ValidationError::InvalidAge;
      return std::nullopt; // Validation successful
    }

    static void HandleValidationFailure(
        ValidationError error,
        const std::string& context = "",
        const std::string& additional_info = "",
        std::size_t max_length = 100) // Default maximum length for context and info
    {
      // Map ValidationError to error message
      std::string error_message = GetErrorMessage(error);
      
      auto Truncate = [max_length](const std::string& input) -> std::string {
        if (input.length() > max_length) {
          return input.substr(0, max_length) + "..."; // Add ellipsis to indicate truncation
        }
        return input;
      };

      if constexpr (Config::DEBUG_MODE) {
        #if FORMAT_ENABLED
          std::cerr << std::format("[Validation Failure] {}", error_message);
          if (!context.empty()) std::cerr << std::format(" | Context: {}", Truncate(context));
          if (!additional_info.empty()) std::cerr << std::format(" | Info: {}", Truncate(additional_info));
          std::cerr << '\n';
        #else
          std::cerr << "[Validation Failure] " << error_message;
          if (!context.empty()) std::cerr << " | Context: " << Truncate(context);
          if (!additional_info.empty()) std::cerr << " | Info: " << Truncate(additional_info);
          std::cerr << '\n';
        #endif // FORMAT_ENABLED
      }
    }

    static std::string GetErrorMessage(ValidationError error)
    {
      switch (error) {
        case ValidationError::EmptyStreet: return "Street cannot be empty.";
        case ValidationError::EmptyCity: return "City cannot be empty.";
        case ValidationError::InvalidPostalCode: return "Postal code must be between 1 and 99950.";
        case ValidationError::EmptyName: return "Name cannot be empty.";
        case ValidationError::InvalidAge: return "Age must be between 1 and 120.";
        default: return "Unknown validation error.";
      }
    }
};

class Address {
  public:
  // Constructors
    // Use Constructor Delegation
    Address() : Address(AddressDefaults::STREET_UNSET, AddressDefaults::CITY_UNSET, std::nullopt) {
      if constexpr (Config::DEBUG_MODE) {
        std::cout << "Default constructor running for '" << *this << "'.\n";
      }
    }
    
    explicit Address(std::string street, std::string city, std::optional<int> postal_code)
        : m_street(std::move(street)), m_city(std::move(city)), m_postal_code(postal_code)
        { // Either const & or std::move
          if (auto error = Validator::ValidateAddress(m_street, m_city, m_postal_code.value())) {
            ResetToDefaults();
            Validator::HandleValidationFailure(error.value(), "Address Constructor",
                "Street: " + m_street + ", City: " + m_city + ", Postal Code: "
                + std::to_string(m_postal_code.value()), Config::MAX_LOG_LENGTH);
          } else if constexpr (Config::DEBUG_MODE) {
            std::cout << "Parameterized constructor (3/3) running for '" << *this << "'.\n";
          }        
    }

  // Destructor
    ~Address() noexcept {
      if constexpr (Config::DEBUG_MODE && Config::CONFIDENTIAL_OVERRIDE) {
        std::cout << "Destructor running for '" << *this << "'.\n";
      }
    }

  // Information Display
    void PrintAddress() const {
      std::cout << "Street: " << m_street << ", City: " << m_city << ", Postal Code: "
          << (m_postal_code ? std::to_string(m_postal_code.value()) : "Unset") << '\n';
    }

  // Overloaded Operator Function
    friend std::ostream& operator<<(std::ostream& os, const Address& address) {
      return os << address.m_street << ", " << address.m_postal_code.value();
    }

  // Helper Function
    void ResetToDefaults() {
      m_street = AddressDefaults::STREET_UNSET;
      m_city = AddressDefaults::CITY_UNSET;
      m_postal_code = std::nullopt;
    }
  
  private:
    std::string m_street;
    std::string m_city;
    std::optional<int> m_postal_code; // Optional type for unset state
};

class Person {
  public:
  // Constructors
    Person() : Person(PersonDefaults::NAME_UNSET, PersonDefaults::AGE_UNSET, Address()) {
      if constexpr (Config::DEBUG_MODE) {
        std::cout << "Default constructor running for '" << *this << "'.\n";
      }
    }
    
    explicit Person(const std::string& name, std::uint8_t age, const Address& address)
        : m_name(name), m_age(age), m_address(address) {
          // No need to call the Address Validator here; the Address Constructor already takes care of it.
          if (auto error = Validator::ValidatePerson(m_name, m_age)) {
            ResetToDefaults();
            Validator::HandleValidationFailure(error.value(), "Person Constructor",
                "Name: " + m_name + ", Age: " + std::to_string(m_age), Config::MAX_LOG_LENGTH);
          } else if constexpr (Config::DEBUG_MODE) {
            std::cout << "Parameterized constructor (3/3) with address object running for '" << *this << "'.\n";
          }
      }

    explicit Person(const std::string& name, const std::uint8_t age, const std::string& street,
        const std::string& city, int postal_code)
        : m_name(name), m_age(age), m_address(street, city, postal_code) {
          if (auto error = Validator::ValidatePerson(m_name, m_age)) {
            // Same comment about the Address Validator.
            ResetToDefaults();
            Validator::HandleValidationFailure(error.value(), "Person Constructor",
                "Name: " + m_name + ", Age: " + std::to_string(m_age), Config::MAX_LOG_LENGTH);
          } else if constexpr (Config::DEBUG_MODE) {
            std::cout << "Parameterized constructor (3/3) with manual address running for '" << *this << "'.\n";
          }
    }
  
  // Destructor
    ~Person() noexcept {
      if constexpr (Config::DEBUG_MODE && Config::CONFIDENTIAL_OVERRIDE) {
        std::cout << "Destructor running for '" << *this << "'.\n";
      }
    }

  // Overloaded Operator Function
    friend std::ostream& operator<<(std::ostream& os, const Person& person) {
      return os << person.m_name << ' ' << static_cast<int>(person.m_age);
    }

  // Information Display
    void PrintPerson() const {
      std::cout << "Name: " << m_name << ", Age: " << static_cast<int>(m_age) << ", ";
      m_address.PrintAddress();
    }

  // Helper function
    void ResetToDefaults() {
      m_name = PersonDefaults::NAME_UNSET;
      m_age = PersonDefaults::AGE_UNSET;
    }

  private:
    std::string m_name;
    std::uint8_t m_age;
    Address m_address;
};

static void SetupConsole() {
  if (!Config::DEVELOPER_MODE) std::system("cls");
  std::system("title \"Address & Person\"");
  std::system("color e");
}

static void LocaleSetup() {
  // Because the locale is not always supported, I enforce it
  SetConsoleOutputCP(CP_UTF8);
  std::locale::global(std::locale(""));  // Uses the default locale with UTF-8
  std::cout.imbue(std::locale());
}

static void ProgramTermination(std::error_code error_code = std::error_code()) {
  // Pause the program before returning 0
  std::cout << "\nPress Enter to exit...";
  std::cin.clear();
  std::cin.get();
  std::exit(error_code.value());
}


int main() {
  SetupConsole();

  LocaleSetup();
  Address my_address = Address("Παπακωστάκη 115", "Αθήνα", 19840);
  Person person1 = Person("Γιάννης", 25, "Παπαδιαμάντη 73", "Θεσσαλονίκη", 41900);
  Person person2 = Person("Μαρία", 27, my_address);

  person1.PrintPerson();
  person2.PrintPerson();

  Address valid_address = Address("Valid Street", "Valid City", 12345);
  Person valid_person = Person("John Doe", 25, valid_address);

  // Address invalid_address = Address("", "", -5);
  // Person invalid_person = Person("", 150, invalid_address);

  // valid_person.PrintPerson();
  // invalid_person.PrintPerson();

  ProgramTermination();
}
