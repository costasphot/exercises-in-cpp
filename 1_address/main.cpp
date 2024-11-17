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
#include <vector>
#include <windows.h>

#if __cplusplus >= 202002L
  #include <span>
  #define SUPPORTS_CPP20 1
#else
  #define SUPPORTS_CPP20 0
#endif // __cplusplus

// 'std::string_view': Any function that only needs to read the string without taking ownership or modifying it.

// Strong typedef to prevent accidental misuse.
using PostalCode = std::uint32_t;
using Age = std::uint8_t;
using LogEntryMaxLength = std::size_t;

enum class ValidationError {
  None = 0,
  EmptyStreet,
  EmptyCity,
  InvalidPostalCode,
  EmptyName,
  InvalidAge
};

struct ErrorReport {
  ValidationError error;
  std::string context;
  std::string message;

  void Log() const {
    Validator::HandleValidationFailure(error, context, message);
  }
};

namespace Config {
  // Do not use 'if constexpr' if I wanted to implement runtime toggling in the future.
  constexpr bool DEVELOPER_MODE{false};
  constexpr bool DEBUG_MODE{false};
  constexpr bool CONFIDENTIAL_OVERRIDE{false};
  
  // Maximum length for log entries before truncation.
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

// For C++20, use constexpr lambda below:
// constexpr auto isValidPostalCode = [](int code) { return code >= 1 && code <= 99950; };
constexpr bool IsValidPostalCode(PostalCode postal_code) { // Compile-time optimization
  return postal_code >= 1 && postal_code <= 99950;
}

class Validator {
  public:
    static std::optional<ValidationError> ValidateAddress(std::string_view street, std::string_view city, PostalCode postal_code) {
      if (street.empty()) return ValidationError::EmptyStreet;
      if (city.empty()) return ValidationError::EmptyCity;
      if (!IsValidPostalCode(postal_code)) return ValidationError::InvalidPostalCode;
      return std::nullopt; // Validation successful
    }
    
    #if SUPPORTS_CPP20
      static std::optional<ValidationError> ValidateBatchAddresses(std::span<const Address> addresses) {
        for (const auto& address : addresses) {
          if (auto error = ValidateAddress(address.GetStreet(), address.GetCity(), ))
        }
      }
    #else
        std::vector<ValidationError> ValidateBatchAddresses(const std::vector<Address>& addresses) {
        std::vector<ValidationError> errors;

        for (const auto& address : addresses) {
            if (auto error = Validator::ValidateAddress(address.GetStreet(), address.GetCity(), address.GetPostalCode())) {
                errors.push_back(*error);
            }
        }

        return errors;
      }
    #endif // SUPPORTS_CPP20

    static std::optional<ValidationError> ValidatePerson(std::string_view name, const Age age) {
      if (name.empty()) return ValidationError::EmptyName;
      if (age == 0 || age > 120) return ValidationError::InvalidAge;
      return std::nullopt; // Validation successful
    }

    static void HandleValidationFailure(
        ValidationError error,
        std::string_view context = "",
        std::string_view additional_info = "",
        LogEntryMaxLength max_length = 100) // Default maximum length for context and info
    {
      // Map ValidationError to error message
      std::string error_message = GetErrorMessage(error);
      
      auto Truncate = [max_length](std::string_view input) -> std::string {
        if (input.length() > max_length) {
          return std::string(input.substr(0, max_length)) + "..."; // Add ellipsis to indicate truncation
        }
        return std::string(input);
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
// Use 'std::string_view' to prevent multiple copies of error messages.
auto CreateSafely(std::string_view context, Args&&... args)
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
    ErrorReport report{std::get<ValidationError>(result), context, "Creation failed"};
    report.Log();

    // No need to manipulate or re-wrap; directly return the error from the variant.
  }

  // Return the result as is (could be either 'T' or 'ValidationError').
  return result;
}

template<typename T, typename... Args>
T CreateAndCheck(const std::string& context, Args&&... args) {
  auto result = CreateSafely<T>(context, std::forward<Args>(args)...);

  if (std::holds_alternative<ValidationError>(result)) {
    ErrorReport report{std::get<ValidationError>(result), context, "Critical Creation Failure"};
    report.Log();

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
    static constexpr PostalCode POSTAL_CODE_UNSET{0};

  // Factory Method to return either Address or ValidationError - involves pre-creation checks and logic.
    static std::variant<Address, ValidationError> Create(std::string_view street, std::string_view city, PostalCode postal_code) {
      if (auto error = Validator::ValidateAddress(street, city, postal_code)) {
        return *error;
      }

      return Address(std::string(street), std::string(city), postal_code); // Return valid Address if success
    }

  // Getters
    constexpr const std::string& GetStreet() const noexcept { return m_street; }
    constexpr const std::string& GetCity() const noexcept { return m_city; }
    constexpr const PostalCode GetPostalCode() const noexcept { return m_postal_code.value_or(POSTAL_CODE_UNSET); }
    // 'value_or': Returns the value if present; otherwise, it provides a default.

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
    Address(std::string street, std::string city, std::optional<PostalCode> postal_code)
        // 'std::string_view': Lightweight and non-owning.
        // 'std::move': When transferring ownership of a resource (e.g.: std::string) between scopes.
        : m_street(std::move(street)), m_city(std::move(city)), m_postal_code(postal_code) { // Explicit conversion to'std::string'.
          if constexpr (Config::DEBUG_MODE) {
            std::cout << "Address object created: " << *this << '\n';
          }
    }

    std::string m_street;
    std::string m_city;
    std::optional<PostalCode> m_postal_code; // Optional type for unset state
};

class Person {
  public:
    // Factory Method
    static std::variant<Person, ValidationError> Create(std::string_view name, const Age age, const Address& address) {
      if (auto error = Validator::ValidatePerson(name, age)) {
        return *error;
      }

      return Person(std::string(name), age, address); // Valid person
    }

  // Overloaded Operator Function
    friend std::ostream& operator<<(std::ostream& os, const Person& person) {
      return os << person.m_name << ' ' << static_cast<unsigned int>(person.m_age);
    }

  // Information Display
    void PrintPerson() const {
      std::cout << "Name: " << m_name << ", Age: " << static_cast<unsigned int>(m_age) << ", ";
      m_address.PrintAddress();
    }

  private:
    Person(std::string name, Age age, Address address)
        : m_name(std::move(name)), // Explicit conversion to 'std::string'.
          m_age(age),
          m_address(std::move(address)) // Move the Address to avoid copying.
      {
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
