#include <iostream>
#include <vector>
#include <numeric>

using namespace std;

int main() {
    cout << "Micro Console Runtime - Boot Sequence Initiated..." << endl;

    vector<int> data = {1, 2, 3, 4, 5};

    if constexpr (true) {
        cout << "C++17 features detected." << endl;
    }

    return 0;
}