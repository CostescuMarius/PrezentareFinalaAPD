#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <fstream>
#include <string>
#include <iostream>

#include <chrono>
#include <iomanip>

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

void merge(int elements[], int left, int middle, int right) {
    int i, j, k;
    int n1 = middle - left + 1;
    int n2 = right - middle;

    int* L = (int*)malloc(n1 * sizeof(int));
    int* R = (int*)malloc(n2 * sizeof(int));

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

    free(L);
    free(R);
}

void merge_sort(int elements[], int left, int right) {
    if (left < right) {
        int middle = left + (right - left) / 2;

        merge_sort(elements, left, middle);
        merge_sort(elements, middle + 1, right);

        merge(elements, left, middle, right);
    }
}

int main(int argc, char** argv) {
    int rank, size;
    int i;
    int* elements = (int*)malloc(NUM_ELEMENTS * sizeof(int));
    int* processor_elements = (int*)malloc((NUM_ELEMENTS / 50) * sizeof(int));
    int* sorted_elements = (int*)malloc(NUM_ELEMENTS * sizeof(int));

    chrono::microseconds final_duration = (microseconds)0;
    chrono::microseconds processor_duration = (microseconds)0;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) {
        string filename_in = "in" + to_string(NUM_ELEMENTS) + ".txt";
        ifstream fin(filename_in);

        random_numbers();

        int num;

        for (int i = 0; i < NUM_ELEMENTS; i++) {
            fin >> num;
            elements[i] = num;
        }

        fin.close();
    }

    MPI_Barrier(MPI_COMM_WORLD);

    MPI_Scatter(elements, NUM_ELEMENTS / size, MPI_INT, processor_elements, NUM_ELEMENTS / size, MPI_INT, 0, MPI_COMM_WORLD);

    auto start = high_resolution_clock::now();

    merge_sort(processor_elements, 0, NUM_ELEMENTS / size - 1);

    auto stop = high_resolution_clock::now();

    auto duration = duration_cast<microseconds>(stop - start);

    processor_duration = processor_duration + duration;

    MPI_Reduce(&processor_duration, &final_duration, 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

    MPI_Gather(processor_elements, NUM_ELEMENTS / size, MPI_INT, sorted_elements, NUM_ELEMENTS / size, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        merge_sort(sorted_elements, 0, NUM_ELEMENTS - 1);

        string filename_out = "out" + to_string(NUM_ELEMENTS) + ".txt";
        ofstream fout(filename_out);

        for (int i = 0; i < NUM_ELEMENTS; i++) {
            fout << sorted_elements[i] << " ";
        }

        fout.close();

        free(sorted_elements);

        float sec = float(final_duration.count()) / float(1000000);
        cout << "Durata totala: " << sec << " secunde" << endl;
    }

    MPI_Finalize();

    return 0;
}