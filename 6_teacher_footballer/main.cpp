// main.cpp
// This program handles Teacher and Footballer objects and calculates their earnings.
// <!> Warning: must compile with compiler flag -std=c++17

#include <iostream>
#include <cstdint>
#include <string>
#include <memory>

using Age = std::uint8_t;
using MaxInstances = std::uint8_t; // Depends on the size of the program.

namespace Config {
  constexpr bool DEVELOPER_MODE{true};
  constexpr bool CONFIDENTIAL_OVERRIDE{true};
}

template<typename T, typename... ExtraArgs>
auto CreatePerson(ExtraArgs&&... extra_args) -> std::unique_ptr<T> {
  std::uint16_t age;

  std::cout << "3) Enter age: ";
  std::cin >> age;
  std::cin.ignore();

  return std::make_unique<T>(static_cast<std::uint8_t>(age), std::forward<ExtraArgs>(extra_args)...);
}

class Person {
  public:
    Person(Age age) : m_age(age) {
      if constexpr (Config::DEVELOPER_MODE) {
        ++number_of_instances;
        std::cout << "Person object created: " << *this << '\n';
      }
    }

    virtual ~Person() noexcept {
      if constexpr (Config::DEVELOPER_MODE && Config::CONFIDENTIAL_OVERRIDE) {
        std::cout << "Person object destroyed: " << *this << '\n';
      }
    }

    virtual double ComputeEarnings() = 0;

    static MaxInstances GetNumberOfInstances() {
      return number_of_instances;
    }

    // Possible causes for the problem:
    // 1. 'std::ostream' doesn't distinguish between 'unsigned char' and 'std::uint8_t'
    // (which is still a 'char' type on many compilers). Explicit casting to 'int'
    // forces 'ostream' to interpret it as a numeric value.
    // 
    // 2. 'ostream' may still interpret 'std::uint8_t' as a character when streamed. This
    // happens because 'ostream' prioritizes overloads for 'char' types over numeric ones.
    //
    // Possible solutions that don't work:
    // 1. Double static casting to unsigned int and then int OR unsigned int and then unsigned int.
    // 2. Unary '+'.
    // 3. Set to a variable with implicit or explicit type cast (neither work).
    friend std::ostream& operator<<(std::ostream& os, const Person& person) {
      int numeric_age = static_cast<int>(person.m_age);
      return os << numeric_age;
    }

  private:
    // Shared across all instances of a class, but with private access (through the getter).
    static MaxInstances number_of_instances;

  protected:
    Age m_age;
};

class Teacher : public Person {
  public:
    Teacher(Age age, std::string profession)
        : Person(age), m_profession(std::move(profession)) {
      if constexpr (Config::DEVELOPER_MODE) {
        std::cout << "Teacher object created: " << *this << '\n';
      }
    }

    virtual ~Teacher() noexcept override {
      if constexpr (Config::DEVELOPER_MODE && Config::CONFIDENTIAL_OVERRIDE) {
        std::cout << "Teacher object destroyed: " << *this << '\n';
      }
    }

    friend std::ostream& operator<<(std::ostream& os, const Teacher& teacher) {
      return os << +teacher.m_age << ", " << teacher.m_profession;
    }

    double ComputeEarnings() override {
      return 1'000.0;
    }

  private:
    std::string m_profession;
};

class Footballer : public Person {
  public:
    Footballer(Age age, std::string team)
        : Person(age), m_team(std::move(team)) {
      if constexpr (Config::DEVELOPER_MODE) {
        std::cout << "Footballer object created: " << *this << '\n';
      }
    }

    virtual ~Footballer() noexcept override {
      if constexpr (Config::DEVELOPER_MODE && Config::CONFIDENTIAL_OVERRIDE) {
        std::cout << "Footballer object destroyed: " << *this << '\n';
      }
    }

    friend std::ostream& operator<<(std::ostream& os, const Footballer& footballer) {
      return os << static_cast<unsigned int>(footballer.m_age) << ", " << footballer.m_team;
    }

    double ComputeEarnings() override {
      return 100'000.0;
    }

  private:
    std::string m_team;
};

void CollectPeople(std::unique_ptr<Person>* people, std::size_t size) {
  for (std::size_t i{0}; i < size; ++i) {
    int choice;
    std::cout << "\nCreate person " << i + 1 << ":\n";
    std::cout << "1) Enter 1 for Teacher, 2 for Footballer: ";
    std::cin >> choice;

    switch (choice) {
      case 1: {
        std::string profession;
        std::cout << "2) Enter profession: ";
        std::cin >> profession;
        people[i] = CreatePerson<Teacher>(profession);
        break;
      }
      
      case 2: {
        std::string team;
        std::cout << "2) Enter team: ";
        std::cin >> team;
        people[i] = CreatePerson<Footballer>(team);
        break;
      }
      
      default:
        std::cerr << "Invalid choice. Please, try again.\n";
        --i;
    }
  }
}

void DisplayPersonInstances() {
    std::cout << "Number of 'Person' instances: "
        << static_cast<unsigned int>(Person::GetNumberOfInstances()) << '\n';
}

// Must be defined at file/global scope to make sure it has a unique memory location.
MaxInstances Person::number_of_instances = 0;


int main() {
  constexpr MaxInstances kNumberOfPeople{5};
  std::unique_ptr<Person> people[kNumberOfPeople];

  CollectPeople(people, kNumberOfPeople);

  std::cout << "\nSummary of people:\n";
  for (std::size_t i{0}; i < kNumberOfPeople; ++i) {
    std::cout << *people[i] << " - Earnings: $" << people[i]->ComputeEarnings() << '\n';
  }

  DisplayPersonInstances();

  return 0;
}
