#pragma once
#include <string>
#include <vector>
#include <sstream>
using namespace std;

struct Predicted{
public:
	double positive;
	double negative;
	wstring text;
};

class Baes
{
private:
	wstring positive_path;
	wstring negative_path;
	vector<wstring> positive_text;
	vector<wstring> negative_text;
	vector<wstring> negative_words;
	vector<wstring> positive_words;

	bool _trained;
	size_t Dp;
	size_t Dn;
	size_t D;
	size_t V;
	size_t Ln;
	size_t Lp;

	void GetTexts(int);
	vector<wstring> SplitWords(const vector<wstring>&);
	vector<wstring> Split(const wstring& str, wchar_t delimiter);
	size_t GetUniqueCount(vector<wstring>&, vector<wstring>&);
	void Stemming(vector<wstring>&);

public:
	Baes(const wstring&, const wstring&);
	~Baes();

	void Train(int);
	void Save(wstring&);
	bool Load(wstring&);

	//Training model from negative and positive files
	Predicted Predict(const wstring&);
};

