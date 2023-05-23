#include <iostream>
#include <vector>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <string>

#include <random>
#include <unordered_set>

#define NUM_ELEMENTS 10000000

using namespace std;
using namespace std::chrono;

void random_numbers() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, NUM_ELEMENTS);

    std::unordered_set<int> numbers;
    while (numbers.size() < NUM_ELEMENTS) {
        int num = dis(gen);
        if (numbers.find(num) == numbers.end()) {
            numbers.insert(num);
        }
    }
    string filename = "in" + to_string(NUM_ELEMENTS) + ".txt";
    ofstream fin(filename);

    for (int num : numbers) {
        fin << num << " ";
    }

    fin.close();
}

void merge(vector<int>& elements, int left, int mid, int right) {
    int left_dim = mid - left + 1;
    int right_dim = right - mid;

    vector<int> leftArr(left_dim);
    vector<int> rightArr(right_dim);

    for (int i = 0; i < left_dim; i++)
        leftArr[i] = elements[left + i];

    for (int j = 0; j < right_dim; j++)
        rightArr[j] = elements[mid + 1 + j];

    int i = 0, j = 0, k = left;

    while (i < left_dim && j < right_dim) {
        if (leftArr[i] <= rightArr[j]) {
            elements[k] = leftArr[i];
            i++;
        }
        else {
            elements[k] = rightArr[j];
            j++;
        }
        k++;
    }

    while (i < left_dim) {
        elements[k] = leftArr[i];
        i++;
        k++;
    }

    while (j < right_dim) {
        elements[k] = rightArr[j];
        j++;
        k++;
    }
}

void mergeSort(vector<int>& elements, int left, int right) {
    if (left >= right)
        return;

    int middle = (left + right) / 2;

    mergeSort(elements, left, middle);
    mergeSort(elements, middle + 1, right);

    merge(elements, left, middle, right);
}

int main() {
    vector<int> arr;

    string filename_in = "in" + to_string(NUM_ELEMENTS) + ".txt";
    ifstream fin(filename_in);

    random_numbers();

    int num;

    for (int i = 0; i < NUM_ELEMENTS; i++) {
        fin >> num;

        arr.push_back(num);
    }

    fin.close();

    auto start = high_resolution_clock::now();

    mergeSort(arr, 0, arr.size() - 1);

    auto stop = high_resolution_clock::now();

    auto duration = duration_cast<microseconds>(stop - start);

    float sec = float(duration.count()) / float(1000000);
    cout << "Timpul necesar: " << sec << " secunde" << endl;

    string filename_out = "out" + to_string(NUM_ELEMENTS) + ".txt";
    ofstream fout(filename_out);

    for (int i = 0; i < arr.size(); i++)
        fout << arr[i] << " ";

    fout.close();

    return 0;
}