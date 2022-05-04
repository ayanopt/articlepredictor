using namespace std;
#include <iostream>
#include "csvstream.h"
#include <cmath>
#include <set>

set<string> unique_words(string in) {
	istringstream source(in);
	set<string> words;
	string word;
	while (source >> word) {
		words.insert(word);
	}
	return words;
}
double loglikelihood(pair<int, vector<string>> p,
	map<pair<string, string>, int>& counts,
	map<string, int>& words,
	map<string, int>& classes) {

	pair<string, string> MediaW{ p.second[1],p.second[0] };
	if (counts[MediaW] >= 1) {
		return log(double(counts[MediaW]) /
			double(classes[MediaW.first]));
	}
	else if (words[(MediaW.second)] >= 1) {
		return log(double(words[MediaW.second]) / double(p.first));
	}
	else {
		return log(1.0 / double(p.first));
	}
}
double logp(int posts,map<string, int> classes, string Media) {
	double x = classes[Media];
	return log(x / (double)posts);
}
pair<string, double> predict2(pair<int,string> posts,
	map<string, int>& words,
	map<string, int>& classes,
	map<pair<string, string>, int>& counts) {
	set<string> words2 = unique_words(posts.second);
	double pai = 0;
	map<string, double> logs;
	pair<string, double> res={ "error", -1000000 };
	for (auto x : classes) {
		pai += logp(posts.first, classes, x.first);
		for (auto w : words2) {
			pai += loglikelihood({ posts.first, {w, x.first} },
				counts, words, classes);
		}
		logs[x.first] = pai;
		pai = 0;
	}
	for (auto& x : logs) {
		if ((x.second > res.second)||
			(res.second == x.second && res.first > x.first)) {
			res = x;
		}
	}
	return res;
}
double sigmoid(double in) {
	return 1 / (1 - exp((-1)*(abs(in))));
}
void debug2(map<string, int>& words,
	pair<int, csvstream&> row,
	map<string, int>&
	classes, map<pair<string, string>, int>& counts) {
	int cnt = 0;
	int corr = 0;
	map<string, string> row2;
	cout << "testing data predictions:"<<endl;
	while (row.second >> row2) {
		auto pai = predict2({ row.first, row2["Article"]}, words,
			classes, counts);
		cout << "  correct media outlet = " << row2["Media"] << endl<<"   >predicted media outlet = "
			<< pai.first <<endl	<<"     >log likelihood λ = " << (pai.second);
		//cout<< "key identifiers"<<k<<
		cout << endl << endl;
		cnt++;
		if (row2["Media"] == pai.first)	corr++;
	}
	cout << "performance: " << corr << " / " << cnt
		<< " articles predicted correctly" << endl;
}
void mapify(string Media, map<string, int>& m) {
	if (m[Media] > 0) {
		m[Media]++;
	}
	else {
		m[Media] = 1;
	}
}
void debug(string train_file, string test_file) {
	csvstream csvin(train_file);
	csvstream csvin2(test_file);
	map<string, string> row;
	map<pair<string, string>, int> counts;
	map<string, int> words;
	map<string, int> classes;
	int size = 0;
	while (csvin >> row) {
		set<string> words2 = unique_words(row["Article"]);
		for (auto& x : words2) {
			if (counts[{ row["Media"], x }] >0) counts[{ row["Media"], x }]++;
			else counts[{ row["Media"], x }] = 1;
			mapify(x, words);
		}
		mapify(row["Media"], classes);
		
		row.clear();
		words2.clear(); size++;
	}
	cout << "trained on " << size<< " articles"
		<< endl;
	cout << endl;
	debug2(words, { size,csvin2 }, classes, counts);
}


int main(int argc, char* argv[]) {
	cout.precision(5);
	if (argc != 3) {
		cout << "Please follow format: ./predictor.exe train.csv test.csv" << endl;
		return 2;
	}
	string test_file = argv[2];
	string train_file = argv[1];
	try {
		debug(train_file,test_file);
	}
	catch (const csvstream_exception& e) {
		cout << e.what() << endl;
		return 1;
	}
    return 0;
}
