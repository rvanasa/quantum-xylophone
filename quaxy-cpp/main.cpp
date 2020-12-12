#include <complex>
#include <bitset>
#include <random>

using std::complex;

float randomFloat(float max) {
    std::random_device r;
    std::default_random_engine randomSource{r()};
    std::uniform_real_distribution<float> floatDistribution;
    return floatDistribution(randomSource) * max;
}

#include "Register.hpp"

#include <iostream>

using namespace std;

int main() {
    Register<5> reg;

    reg.apply(1, 'H');
    reg.apply(1, 'C');
    reg.apply(0, 'X');

    for(int i = 0; i < reg.size(); i++) {
        if(i) {
            cout << ", ";
        }
        auto c = reg.view(i);
        cout << c.real();
        if(c.imag()) {
            cout << '+' << c.imag() << 'i';
        }
    }
    cout << endl;

    for(int i = 0; i < reg.qubitCount(); i++) {
        cout << reg.measure(i);
    }
    cout << endl;

    cout << reg.measure() << endl;
}
