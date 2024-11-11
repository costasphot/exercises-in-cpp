// main.cpp
// This program models a simple system for validating and displaying Address and Person objects.
// <!> Warning: must compile with compiler flag -std=c++17

#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <string>
#include <optional>
#include <system_error>
#include <variant>
#include <windows.h>

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
  constexpr bool DEVELOPER_MODE{false};
  constexpr bool DEBUG_MODE{false};
  constexpr bool CONFIDENTIAL_OVERRIDE{false};
  
  // Maximum length for log entries before truncation
  constexpr std::uint16_t MAX_LOG_LENGTH{50};
}

template<typename T>
std::string CleanTypeName() {
  // For more accurate and demangled type names, see: 'abi::__cxa_demangle' in GCC/Clang.
  std::string type_name{typeid(T).name()};
  std::size_t pos{0};

  // Find the first non-digit character in the type name.
  while (pos < type_name.length() && std::isdigit(type_name[pos])) {
    ++pos;
  }

  // Return the type name without the leading numbers.
  return type_name.substr(pos);
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
          std::cerr << "[Validation Failure] " << error_message;
          if (!context.empty()) std::cerr << " | Context: " << Truncate(context);
          if (!additional_info.empty()) std::cerr << " | Info: " << Truncate(additional_info);
          std::cerr << '\n';
      }
    }

    static std::string GetErrorMessage(ValidationError error) {
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

static void ProgramTermination(std::error_code error_code = std::error_code()) {
  // Pause the program before returning 0
  std::cout << "\nPress Enter to exit...";
  std::cin.clear();
  std::cin.get();

  std::exit(error_code ? error_code.value() : EXIT_SUCCESS);
}

template<typename T, typename... Args>
auto CreateSafely(const std::string& context, Args&&... args)
    -> decltype(T::Create(std::forward<Args>(args)...))
    //    'decltype' constraining:
    // Ensures this function only works for types 'T' that implement a
    // 'Create' method returning a 'std::variant<T, ValidationError>'.
    //
    //    Perfect Forwarding:
    // 'std::forward' ensures that each argument 'args...' retains its original value category
    // (lvalue or rvalue) as it was passed to 'CreateSafely' when being forwarded to 'T::Create'.
    // This preserves efficiency by avoiding unnecessary copies or moves.
{ 
  // Forwarding arguments to 'Create' and storing the result in a variant.
  auto result = T::Create(std::forward<Args>(args)...);

  if (std::holds_alternative<ValidationError>(result)) {
    // Handle the case where 'Create' returns a validation error.
    Validator::HandleValidationFailure(
      std::get<ValidationError>(result), context, "Creation failed"
    );
    // No need to manipulate or re-wrap; directly return the error from the variant.
  }

  // Return the result as is (could be either 'T' or 'ValidationError').
  return result;
}

template<typename T, typename... Args>
T CreateAndCheck(const std::string& context, Args&&... args) {
  auto result = CreateSafely<T>(context, std::forward<Args>(args)...);

  if (std::holds_alternative<ValidationError>(result)) {
    Validator::HandleValidationFailure(
      std::get<ValidationError>(result), context, "Critical Creation Failure"
    );

    ProgramTermination(std::make_error_code(std::errc::invalid_argument));
  }
  
  T created_object = std::get<T>(result);
  if constexpr (Config::DEBUG_MODE) {
    std::cout << "Successfully created object '" << created_object << "' of type '" << CleanTypeName<T>() << "'.\n";
  }
  return created_object; // Return the valid object
}

class Address {
  public:
    // Factory Method to return either Address or ValidationError - involves pre-creation checks and logic.
    static std::variant<Address, ValidationError> Create(const std::string& street, const std::string& city, int postal_code) {
      if (auto error = Validator::ValidateAddress(street, city, postal_code)) {
        return *error;
      }

      return Address(street, city, postal_code); // Return valid Address if success
    }

  // Information Display
    void PrintAddress() const {
      std::cout << "Street: " << m_street << ", City: " << m_city << ", Postal Code: "
          << (m_postal_code ? std::to_string(m_postal_code.value()) : "Unset") << '\n';
    }

  // Overloaded Operator <<
    friend std::ostream& operator<<(std::ostream& os, const Address& address) {
      return os << address.m_street << ", " << address.m_city << " ("
          << (address.m_postal_code ? std::to_string(address.m_postal_code.value()) : "Unset") << ')';
    }
  
  private:
    // Hide constructor, use Create for control
    Address(std::string street, std::string city, std::optional<int> postal_code)
        : m_street(std::move(street)), m_city(std::move(city)), m_postal_code(postal_code) {
          if constexpr (Config::DEBUG_MODE) {
            std::cout << "Address object created: " << *this << '\n';
          }
    }

    std::string m_street;
    std::string m_city;
    std::optional<int> m_postal_code; // Optional type for unset state
};

class Person {
  public:
    // Factory Method
    static std::variant<Person, ValidationError> Create(const std::string& name, const std::uint8_t age, const Address& address) {
      if (auto error = Validator::ValidatePerson(name, age)) {
        return *error;
      }

      return Person(name, age, address); // Valid person
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

  private:
    Person(std::string name, std::uint8_t age, Address address)
        : m_name(std::move(name)), m_age(age), m_address(std::move(address)) {
          if constexpr (Config::DEBUG_MODE) {
            std::cout << "Person object created: " << *this << '\n';
          }
    }

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


int main() {
  SetupConsole();
  LocaleSetup();

  auto address = CreateAndCheck<Address>("Main", "Valid Street", "Valid City", 12345);
  auto person = CreateAndCheck<Person>("Main", "John Doe", 30, address);

  person.PrintPerson();

  ProgramTermination();
}
