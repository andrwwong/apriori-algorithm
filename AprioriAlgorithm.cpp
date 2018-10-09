#include "stdafx.h"
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <sstream>
#include <map>
#include <functional>
#include <algorithm>
#include <iterator>
#include <time.h>



/*
	Author: Andrew Wong

	This program creates association rules by mining frequent itemsets. A bottom-up approach
	is used where frequent item sets are expanded one item at a time. The program runs until
	all frequent item sets have been exhausted.

	Below is a list of basic terminology to better understand the program:
	- Frequent Item: An item is called frequent when its support is greater than or equal to the
						  minimum support threshold.
	- Item: A number or group of numbers, e.g. {1} or {1, 2, 5}
	- Itemset: A group of items.
	- Minimum Support Threshold: An arbritary number the support must equal or surpass for the item
										  to be called frequent.
	- Support: The number of times an item appears in the data set.
*/

using namespace std;

class AprioriAlgorithm {
public:
	AprioriAlgorithm();
	void compute();
private:
	int minSupp;
	double runtimeReading, runtimeAlgorithm, runtimeWriting;
	long clockStart;
	map<int, vector<int>> positions;
	vector<pair<vector<int>, int>> freqItemset, itemset, results;
	vector<vector<int>> originalData;
	bool done;


	vector<vector<int>> readFile(string filename);
	bool compare(vector<int> i, vector<int> j);
	void firstStep();
	void findSupport();
	void createItemset();
	void writeToFile(string filename);
};


AprioriAlgorithm::AprioriAlgorithm() {
	minSupp = 2;
	done = false;
	vector<vector<int>> originalData = vector<vector<int>>();
}

/*
	Reads in a data file in the form
	x # # # # # # #
	where x represents the number of items in the row and # represents
	a single item. The original data from the file is returned.

	filename: the name of the file
*/
vector<vector<int>> AprioriAlgorithm::readFile(string filename) {
	vector<vector<int>> data = vector<std::vector<int>>();
	std::vector<int> list = std::vector<int>();
	std::stringstream items;
	string input = "";
	ifstream file;
	int item;

	file.open(filename);

	// The file is read in line by line.
	for (int row = 0; getline(file, input); row++) {
		items = stringstream(input);
		items >> item;

		while (items >> item) {
			list.push_back(item);
			positions[item].push_back(row);
		}

		data.push_back(list);
		list.clear();
	}
	file.close();

	runtimeReading = (clock() - clockStart)/1000.0;

	return data;
}

/*
	Prunes the dataset by disregarding items with a support below the minimum support threshold.
	Adds the frequent items to the results to be outputted later, and creates new pairs {a, b} where
	a and b are both frequent.
*/
void AprioriAlgorithm::firstStep() {

	// Prunes the data set.
	for (const auto &item : positions) {
		if(item.second.size() >= minSupp) {
 			freqItemset.push_back(make_pair(vector<int> { item.first }, item.second.size()));
		}
	}

	results.insert(results.end(), freqItemset.begin(), freqItemset.end());

	// Creates new pairs {a, b} from the frequent items.
	for (int i = 0; i < freqItemset.size(); i++) {
		for (int j = i + 1; j < freqItemset.size(); j++) {
			itemset.push_back(make_pair(vector<int> { freqItemset[i].first[0],
												 freqItemset[j].first[0] }, 0));
		}
	}
}

/*
	Counts the support of the candidate items and prunes the candidate items whose support is below
	the minimum support threshold. Adds the remaining items to the results to be outputted later.
*/
void AprioriAlgorithm::findSupport() {
	int support = 0, min = 0;
	freqItemset.clear();

	// Loops through all the candidate items in the itemset
	for (int i = 0; i < itemset.size(); i++) {
		min = 0;

		// Finds the number in a given candidate item with the least support to more
		// efficiently determine whether a the item exceeds the minimum support threshold.
		for (int j = 1; j < itemset[i].first.size(); j++) {
			if (positions.at(itemset[i].first[min]).size() > positions.at(itemset[i].first[j]).size())
				min = j;
		}

		// Loops through the original data and counts the support of the item. The starting point
		// of the search is the number with the smallest support.
		for (int k = 0; k < positions.at(itemset[i].first[min]).size(); k++) {
			// Gets the current row's items
			vector<int> row = originalData[positions.at(itemset[i].first[min])[k]];

			// If the row contains the itemset, increase the support of the item by one.
			if (includes(row.begin(), row.end(), itemset[i].first.begin(), itemset[i].first.end()))
				support++;
		}

		if (support >= minSupp)
			freqItemset.push_back(make_pair(itemset[i].first, support));

		support = 0;
	}

	results.insert(results.end(), freqItemset.begin(), freqItemset.end());
}

// Creates new itemsets by adding an item {n} to a frequent item {a, b, ...}
void AprioriAlgorithm::createItemset() {
	vector<int> items;

	itemset.clear();

	if (freqItemset.size() > 2) {
		for (int i = 0; i < freqItemset.size() - 1; i++) {
			for (int j = i + 1; compare(freqItemset[i].first, freqItemset[j].first); j++) {
				// Adds an item {a, b, ...} in freqItemset to items
				items.insert(items.end(), freqItemset[i].first.begin(), freqItemset[i].first.end());
				// Adds another item {n} to items to create a new item
				items.push_back(freqItemset[j].first[freqItemset[j].first.size() - 1]);
				// Adds the new item to an itemset
				itemset.push_back(make_pair(items, 0));
				items.clear();
			}
		}
	}
	else
		done = true;
}

/*
	Compares two consecutive items. If those items are identical, excluding the last number,
	return true. Else, return false.
*/
bool AprioriAlgorithm::compare(vector<int> i, vector<int> j) {
	int size;

	if (i.size() > j.size())
		size = j.size();
	else
		size = i.size();

	for (int k = 0; k < size - 1; k++) {
		if (i[k] != j[k])
			return false;
	}

	return true;
}

/*
	Writes all the frequent itemsets to the specified file.

	filename: the file to output to
*/
void AprioriAlgorithm::writeToFile(string filename) {
	ofstream file;
	file.open(filename);

	for (int i = 0; i < results.size(); i++) {
		file << "[";
		for (int j = 0; j < results[i].first.size() - 1; j++)
			file << results[i].first[j] << " ";

		file << results[i].first[results[i].first.size() - 1] << "] " << results[i].second << "\n";
	}

	file.close();
}

// Finds all frequent itemsets and determines the association rules.
void AprioriAlgorithm::compute() {
	clockStart = clock();
	originalData = readFile("Data.txt");

	firstStep();

	clockStart = clock();

	while(!done) {
		findSupport();
		createItemset();
	}

	runtimeAlgorithm = (clock() - clockStart)/1000.0;

	clockStart = clock();
	//writeToFile("Results.txt");
	runtimeWriting = (clock() - clockStart) / 1000.0;

	cout << "Time to read file: " << runtimeReading << " seconds\n";
	cout << "Time to run algorithm: " << runtimeAlgorithm << " seconds\n";
	cout << "Time to write to file: " << runtimeWriting << " seconds\n";
	cout << "Apriori Algorithm has successfully run.";

	writeToFile("Results.txt");
}

int main() {
	cout << "Apriori Algorithm has begun. Results will be written to Results.txt.\n\n";
	AprioriAlgorithm apriori;
	apriori.compute();
}
