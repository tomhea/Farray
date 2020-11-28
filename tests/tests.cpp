#include <vector>
#include <algorithm>
#include <random>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <chrono>

#include "../include/fill_array_1bit.hpp"
#include "testClasses.hpp"

using namespace std;
using namespace std::chrono;
using namespace FillArray;


template<typename T>
bool eq(T* arr, const Holder<T>& h1, const Holder<T>& h2, T* A, int n, bool flag) {
    for (size_t i = 0; i < n; i++) {
        if (!(arr[i] == read(A, n, i, flag) && arr[i] == h1.read(i) && arr[i] == h2[i])) {
            cout << "index " << i << ":  arr[i]=" << arr[i] << ", while A.read(i)=" << read(A, n, i, flag) << "." << endl;
            return false;
        }
    }
    return true;
}


template<typename T, getRandom<T> rnd>
bool stressTest(int n, T def, int inits, int reads, int writes) {
    auto h1 = Holder<T>(n, def);
    auto h2 = Holder<T>(n, def);
    T* A = new T[n];
    bool flag = fill(A, n, def);
    vector<char> actions;
    actions.reserve(reads+writes+inits);
    for (int i = 0; i < reads ; i++) actions.emplace_back('R');
    for (int i = 0; i < writes; i++) actions.emplace_back('W');
    for (int i = 0; i < inits ; i++) actions.emplace_back('I');
    auto rng = default_random_engine{};
    shuffle(begin(actions), end(actions), rng);
    auto arr = new T[n];
    for (int u = 0; u < n; u++) arr[u] = def;
    if (!eq<T>(arr, h1, h2, A, n, flag)) {
        cout << "Just initialized! def = " << def << "." << endl;
        return false;
    }

    int count = 0;
    int last2 = 0;
    for (auto op : actions) {
        count++;
        int i = rand() % n; T v = rnd();
        if (op == 'I') {
            last2 = count;
            if (rand()&1) def = v;
            for (int u = 0; u < n; u++) arr[u] = def;
            flag = fill(A,n,def);
            h1.fill(def);
            h2 = def;
        } else if (op == 'W') {
            arr[i] = v;
            flag = write(A,n,i,v,flag);
            h1.write(i, v);
            h2[i] = v;
        } else {
            if (!(arr[i] == read(A,n,i,flag) && arr[i] == h1.read(i) && arr[i] == h2[i])) {
                cout << "Bad Read: at index " << i << ",  count " << count << endl;
                return false;
            }
        }

        if (!eq<T>(arr, h1, h2, A, n, flag)) {
            cout << "Last op = " << op << ":    i = " << i << ", v = " << v << "." << endl;
            cout << "Last def = " << def << ", flag = " << (int)flag << ".    op count = " << count << ", last2 = " << last2 << "." << endl;
            return false;
        }
    }

    return true;
}


void handleTestResult(const string& pretext, bool successRes, int& goodTestsNum, int& testsNum) {
    testsNum++;
    goodTestsNum += successRes;
    cout << pretext << (successRes ? "Success!" : "Failed.") << endl;
}


int max(int x, int y) {
    return x>y ? x : y;
}

void printArrayNice(Holder<int>& A) {
    int j = 0;
    for (size_t i : A) cout << "A[" << i << "] = " << A[i] << (++j % 10 == 0 ? "\n" : "    ");
    cout << endl;
}
void iteratorPrintTest() {
    Holder<int> A(100, 2);
    cout << "Should be nothing:" << endl;
    printArrayNice(A);

    A[24] = 100;
    cout << "Should be 24's block (10 nums for int:32 and size_t:64), only A[24] != 2:" << endl;
    printArrayNice(A);

    A[17] = 11*A[23] + A[24];
    A[30] = A[20] = A[16] = 997;
    A[89] = 19;
    A[77] = A[77];
    cout << "Should be nums 0-39, and 70-89, not in order, only A[{ 16,17,20,24,30,89 }] != 2:" << endl;
    printArrayNice(A);

    A = 18;
    cout << "Should be nothing:" << endl;
    printArrayNice(A);

    cout << "Done." << endl;
}


bool tests(vector<size_t> sizes) {
    srand(time(0));
    auto startTime = high_resolution_clock::now();
    int goodTestsNum = 0, testsNum = 0;
    for (auto size : sizes) {
        int i = max(10000 / sqrt(size), 100);
        int r = max(30000 / sqrt(size), 300);
        int w = max(50000 / sqrt(size), 500);
        cout << "ARRAY SIZE " << size << ":" << endl;
        handleTestResult("X:      ", stressTest<X, X::getRandom>(size, {1, 2, 3}, i, r, w), goodTestsNum, testsNum);
        handleTestResult("Y:      ", stressTest<Y, Y::getRandom>(size, {14, 56}, i, r, w), goodTestsNum, testsNum);
        handleTestResult("Z:      ", stressTest<Z, Z::getRandom>(size, {123456789098, 9876543212345}, i, r, w), goodTestsNum, testsNum);
        handleTestResult("ZZ:     ", stressTest<ZZ, ZZ::getRandom>(size, {5523456789098, 44876543212345, 0, 1348765432578, 446}, i, r, w), goodTestsNum, testsNum);
        handleTestResult("int:    ", stressTest<int, getRand<int, 10000000>>(size, 123, i, r, w), goodTestsNum, testsNum);
        handleTestResult("size_t: ", stressTest<size_t, getRand<size_t, 100000000000>>(size, 123, i, r, w), goodTestsNum, testsNum);
        handleTestResult("int16:  ", stressTest<int16_t, getRand<int16_t, 60000>>(size, 123, i, r, w), goodTestsNum, testsNum);
        handleTestResult("int8:   ", stressTest<int8_t, getRand<int8_t, 250>>(size, 123, i, r, w), goodTestsNum, testsNum);
        handleTestResult("bool:   ", stressTest<bool, getRand<bool, 2>>(size, true, i, r, w), goodTestsNum, testsNum);
        cout << endl;
    }
    auto endTime = high_resolution_clock::now();
    auto ms = duration_cast<microseconds>(endTime - startTime).count();
    cout << "Overall time: " << ((double)ms)/1000000 << "s." << endl;
    bool success = goodTestsNum == testsNum;
    cout << "Tests passed: " << goodTestsNum << "/" << testsNum << ". " << (success ? "Success!" : "Failed.") << endl << endl;
    return success;
}


int main() {
    tests({1, 5, 10, 20, 40, 100, 500, 1000, 2000, 5000, 10000, 20000, 50000});
//    iteratorPrintTest();
    return 0;
}
