#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <fstream>
#include <string>
#include <iostream>
#include <thread>

#include <chrono>
#include <iomanip>

#include <random>
#include <unordered_set>

#include <mutex>

#define NUM_ELEMENTS 100
#define MAX_THREADS 50

using namespace std;
using namespace std::chrono;

std::mutex duration_mutex;
std::chrono::microseconds total_duration(0);

void print_and_update_duration(const std::chrono::microseconds& partial_duration) {
    std::lock_guard<std::mutex> lock(duration_mutex);
    total_duration += partial_duration;
}

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

void merge(std::vector<int>& elements, int left, int middle, int right) {
    int i, j, k;
    int n1 = middle - left + 1;
    int n2 = right - middle;

    std::vector<int> L(n1);
    std::vector<int> R(n2);

    for (i = 0; i < n1; i++)
        L[i] = elements[left + i];
    for (j = 0; j < n2; j++)
        R[j] = elements[middle + 1 + j];

    i = 0;
    j = 0;
    k = left;
    while (i < n1 && j < n2) {
        if (L[i] <= R[j]) {
            elements[k] = L[i];
            i++;
        }
        else {
            elements[k] = R[j];
            j++;
        }
        k++;
    }

    while (i < n1) {
        elements[k] = L[i];
        i++;
        k++;
    }

    while (j < n2) {
        elements[k] = R[j];
        j++;
        k++;
    }
}

void merge_sort(std::vector<int>& elements, int left, int right) {
    if (left < right) {
        int middle = left + (right - left) / 2;

        if (right - left <= NUM_ELEMENTS / MAX_THREADS) {
            merge_sort(elements, left, middle);
            merge_sort(elements, middle + 1, right);
        }
        else {
            std::thread t1(merge_sort, std::ref(elements), left, middle);
            std::thread t2(merge_sort, std::ref(elements), middle + 1, right);

            t1.join();
            t2.join();
        }

        merge(elements, left, middle, right);
    }
}

int main() {
    std::vector<int> elements(NUM_ELEMENTS);

    string filename_in = "in" + to_string(NUM_ELEMENTS) + ".txt";
    ifstream fin(filename_in);

    //random_numbers();

    int num;

    for (int i = 0; i < NUM_ELEMENTS; i++) {
        fin >> num;
        elements.push_back(num);
    }

    fin.close();

    chrono::microseconds final_duration = (microseconds)0;
    chrono::microseconds processor_duration = (microseconds)0;

    auto start = high_resolution_clock::now();

    merge_sort(elements, 0, NUM_ELEMENTS - 1);

    auto stop = high_resolution_clock::now();

    processor_duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

    final_duration += processor_duration;

    print_and_update_duration(final_duration);

    if (std::this_thread::get_id() == std::thread::id()) {
        float sec = static_cast<float>(total_duration.count()) / 1000000.0f;
        std::cout << "Durata totala: " << sec << " secunde" << std::endl;
    }

    string filename_out = "out" + to_string(NUM_ELEMENTS) + ".txt";
    ofstream fout(filename_out);

    for (int i = 0; i < NUM_ELEMENTS; i++) {
        fout << elements[i] << " ";
    }

    fout.close();



    return 0;
}