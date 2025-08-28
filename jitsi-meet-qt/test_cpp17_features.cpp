#include <iostream>
#include <optional>
#include <string_view>
#include <tuple>
#include <type_traits>

// Test C++17 features used in ConfigurationManager
int main() {
    std::cout << "Testing C++17 features..." << std::endl;
    
    // Test std::optional
    std::optional<int> opt_value = 42;
    if (opt_value) {
        std::cout << "✓ std::optional works: " << *opt_value << std::endl;
    }
    
    // Test std::string_view
    std::string_view sv = "Hello, C++17!";
    std::cout << "✓ std::string_view works: " << sv << std::endl;
    
    // Test structured bindings
    auto [x, y] = std::make_tuple(10, 20);
    std::cout << "✓ Structured bindings work: " << x << ", " << y << std::endl;
    
    // Test if constexpr
    constexpr bool is_int = std::is_integral_v<int>;
    if constexpr (is_int) {
        std::cout << "✓ if constexpr works: int is integral" << std::endl;
    }
    
    std::cout << "All C++17 features working correctly!" << std::endl;
    return 0;
}