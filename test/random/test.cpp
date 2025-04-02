#include <iostream>

int add(int a, int b) {
    return a + b;
}

int main() {
    int num1 = 10;
    int num2 = 5;
    int sum = add(num1, num2);
    std::cout << "The sum is: " << sum << std::endl;
    return 0;
}