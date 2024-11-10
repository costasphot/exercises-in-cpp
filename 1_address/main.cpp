// main.cpp
// This program models a simple system for validating and displaying Address and Person objects.

#include <iostream>
#include <cstdint>
#include <stdexcept>
#include <limits>
#include <windows.h>

namespace Config {
  constexpr bool DEVELOPER_MODE = false;
  constexpr bool CONFIDENTIAL_OVERRIDE = false;
}

class Validator {
  public:
    static void ValidateAddress(const std::string& street, const std::string& city, int postal_code) {
      if (street.empty() || city.empty()) {
        throw std::invalid_argument("Error: Street and city cannot be empty.\n");
      }

      if (postal_code < 1 || postal_code > 99950) {
        throw std::invalid_argument("Error: The postal code must be between 1 and 99950.\n");
      }
    }

    static void ValidatePerson(const std::string& name, std::uint8_t age) {
      if (name.empty()) {
        throw std::invalid_argument("Error: Name cannot be empty.\n");
      }

      if (age == 0 || age > 120) {
        throw std::invalid_argument("Error: Age must be between 1 and 120.\n");
      }
    }

    static void HandleValidationFailure(const std::invalid_argument& e) {
      if (Config::DEVELOPER_MODE) {
        std::cerr << "[Validation Failure] " << e.what() << '\n';
      }
    }
};

class Address {
  public:
  // Constructors
    Address() : m_street("Παπακωστάκη 115"), m_city("Θεσσαλονίκη"), m_postal_code(41900) {
      if (Config::DEVELOPER_MODE) {
        std::cout << "Default constructor running for '" << *this << "'.\n";
      }
    }
    
    Address(std::string street, std::string city, int postal_code)
        : m_street(std::move(street)), m_city(std::move(city)), m_postal_code(postal_code) { // Either const & or std::move
          try {
            Validator::ValidateAddress(m_street, m_city, m_postal_code);
            if (Config::DEVELOPER_MODE) {
              std::cout << "Parameterized constructor (3/3) running for '" << *this << "'.\n";
            }
          } catch (const std::invalid_argument& e) {
            Validator::HandleValidationFailure(e);
            
            // Log additional information
            std::cerr << "Street: " << m_street << ", City: " << m_city << ", Postal Code: " << m_postal_code << '\n';
      
            // Take corrective action (or just close the program)        
            m_street = "Παπακωστάκη 115";
            m_city = "Θεσσαλονίκη";
            m_postal_code = 41900;
          }
    }

  // Destructor
    ~Address() noexcept {
      if (Config::DEVELOPER_MODE && Config::CONFIDENTIAL_OVERRIDE) {
        std::cout << "Destructor running for '" << *this << "'.\n";
      }
    }

  // Information Display
    void PrintAddress() {
      std::cout << "Street: " << m_street << ", City: " << m_city << ", Postal Code: " << m_postal_code;
    }

  // Overloaded Operator Function
    friend std::ostream& operator<<(std::ostream& os, const Address& address) {
      return os << address.m_street << ", " << address.m_postal_code;
    }
  
  private:
    std::string m_street;
    std::string m_city;
    int m_postal_code;
};

class Person {
  public:
  // Constructors
    Person() : m_name("Γιάννης"), m_age(25), m_address("Παπακωστάκη 115", std::string("Θεσσαλονίκη"), 41900) {
      if (Config::DEVELOPER_MODE) {
        std::cout << "Default constructor running for '" << *this << "'.\n";
      }
    }
    
    Person(const std::string& name, std::uint8_t age, const Address& address)
        : m_name(name), m_age(age), m_address(address) {
          try {
            Validator::ValidatePerson(m_name, m_age);
            // No need to call the Address Validator here; the Address Constructor already takes care of it.
            if (Config::DEVELOPER_MODE) {
              std::cout << "Parameterized constructor (3/3) with address object running for '" << *this << "'.\n";
            }
          } catch (const std::invalid_argument& e) {
            Validator::HandleValidationFailure(e);

            // TODO: Any corrective action here?
          }
      }

    Person(const std::string& name, const std::uint8_t age, const std::string& street,
        const std::string& city, int postal_code)
        : m_name(name), m_age(age), m_address(street, city, postal_code) {
          try {
            Validator::ValidatePerson(m_name, m_age);
            // Same comment about the Address Validator.
            if (Config::DEVELOPER_MODE) {
              std::cout << "Parameterized constructor (3/3) with manual address running for '" << *this << "'.\n";
            }
          } catch (const std::invalid_argument& e) {
            Validator::HandleValidationFailure(e);

            // Log additional information
            std::cerr << "Name: " << m_name << ", Age: " << m_age << '\n';
      
            // Take corrective action        
            m_name = "Γιάννης";
            m_age = 25;
          }
    }
  
  // Destructor
    ~Person() noexcept {
      if (Config::DEVELOPER_MODE && Config::CONFIDENTIAL_OVERRIDE) {
        std::cout << "Destructor running for '" << *this << "'.\n";
      }
    }

  // Overloaded Operator Function
    friend std::ostream& operator<<(std::ostream& os, const Person& person) {
      return os << person.m_name << ' ' << static_cast<int>(person.m_age);
    }

  // Information Display
    void PrintPerson() {
      std::cout << "Name: " << m_name << ", Age: " << static_cast<int>(m_age) << ", ";
      m_address.PrintAddress();
      std::cout << '\n';
    }

  private:
    std::string m_name;
    std::uint8_t m_age;
    Address m_address;
};

static void LocaleSetup() {
  // Because the locale is not always supported, I enforce it
  SetConsoleOutputCP(CP_UTF8);
  std::locale::global(std::locale(""));  // Uses the default locale with UTF-8
  std::cout.imbue(std::locale());
}

static void cmdOnEnd() {
  // Pause the program before returning 0
  std::cout << "\nPress Enter to exit...";
  std::cin.clear();
  std::cin.get();
}


int main() {
  LocaleSetup();
  Address my_address = Address("Παπακωστάκη 115", "Αθήνα", 19840);
  Person person1 = Person("Γιάννης", 25, "Παπαδιαμάντη 73", "Θεσσαλονίκη", 41900);
  Person person2 = Person("Μαρία", 27, my_address);

  // std::cout << person << '\n';
  person1.PrintPerson();
  person2.PrintPerson();

  cmdOnEnd();

  return 0;
}
