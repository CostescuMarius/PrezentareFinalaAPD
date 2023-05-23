#include <omp.h>
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

	if (left < right) {
		int middle = (left + right) / 2;
		mergeSort(elements, left, middle);
		mergeSort(elements, middle + 1, right);
		merge(elements, left, middle, right);
	}
}

vector<int> mergeIntervals(vector<int>& left, vector<int>& right) {
	vector<int> result;
	int i = 0, j = 0;

	while (i < left.size() && j < right.size()) {
		if (left[i] <= right[j]) {
			result.push_back(left[i]);
			i++;
		}
		else {
			result.push_back(right[j]);
			j++;
		}
	}

	while (i < left.size()) {
		result.push_back(left[i]);
		i++;
	}

	while (j < right.size()) {
		result.push_back(right[j]);
		j++;
	}

	return result;
}


int main() {
	vector<int> elements;
	string filename_in = "in" + to_string(NUM_ELEMENTS) + ".txt";
	ifstream fin(filename_in);

	random_numbers();

	int num;

	for (int i = 0; i < NUM_ELEMENTS; i++) {
		fin >> num;
		elements.push_back(num);
	}

	fin.close();

	vector<vector<int>> sortedIntervals;

	auto start = high_resolution_clock::now();

#pragma omp parallel
	{
		int num_threads = omp_get_num_threads();
		int tid = omp_get_thread_num();
		int chunk_size = NUM_ELEMENTS / num_threads;
		int left = tid * chunk_size;
		int right = (tid == num_threads - 1) ? NUM_ELEMENTS - 1 : (left + chunk_size - 1);

		vector<int> thread_elements;
		for (int i = left; i <= right; i++) {
			thread_elements.push_back(elements[i]);
		}
		mergeSort(thread_elements, 0, thread_elements.size() - 1);

#pragma omp critical
		sortedIntervals.push_back(thread_elements);
	}

	while (sortedIntervals.size() > 1) {
		vector<vector<int>> mergedIntervals;
		for (int i = 0; i < sortedIntervals.size() - 1; i += 2) {
			mergedIntervals.push_back(mergeIntervals(sortedIntervals[i], sortedIntervals[i + 1]));
		}
		if (sortedIntervals.size() % 2 == 1) {
			mergedIntervals.push_back(sortedIntervals.back());
		}
		sortedIntervals = mergedIntervals;
	}
	vector<int> final_elements = sortedIntervals[0];

	auto stop = high_resolution_clock::now();

	auto duration = duration_cast<microseconds>(stop - start);

	float sec = float(duration.count()) / float(1000000);
	cout << "Timpul necesar: " << sec << " secunde" << endl;

	string filename_out = "out" + to_string(NUM_ELEMENTS) + ".txt";
	ofstream fout(filename_out);

	for (int i = 0; i < final_elements.size(); i++)
		fout << final_elements[i] << " ";

	fout.close();

	return 0;
}
