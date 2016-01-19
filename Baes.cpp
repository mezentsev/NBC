#define _USE_MATH_DEFINES
#include "Baes.h"
#include "include/libstemmer.h"
#include <iostream>
#include <fstream>
#include <map>
#include <ctime>
#include <set>
#include <cmath>

#define M_E 2.71828182845904523536
#include "Util.h"

/// Read N texts from positive and negative files for training
void Baes::GetTexts(int count)
{
	int i = 0;
	wstring line;
	wifstream positive_file(positive_path, ios::in);
	while (getline(positive_file, line) && i < count)
	{
		try
		{
			auto text = Split(line, L';');
			if (text.size() > 3)
				positive_text.push_back(text[3].substr(1, text[3].length() - 1));
		}
		catch (exception& ex)
		{
			// Possible missed line
			cerr << ex.what() << endl;
		}
		i++;
	}
	positive_file.close();

	i = 0;
	wifstream negative_file(negative_path, ios::in);
	while (getline(negative_file, line) && i < count)
	{
		try
		{
			auto text = Split(line, L';');
			if (text.size() > 3)
				negative_text.push_back(text[3].substr(1, text[3].length() - 1));
		}
		catch (exception& ex)
		{
			// Possible missed line
			cerr << ex.what() << endl;
		} 
		i++;
	}
	negative_file.close();
}

/// Split words in text
vector<wstring> Baes::SplitWords(const vector<wstring>& text)
{
	vector<wstring> words;
	for (auto &t : text)
	{
		auto splited = Split(t, L' ');
		words.insert(words.end(), splited.begin(), splited.end());
	}

	return words;
}

/// Split words by delimiter
vector<wstring> Baes::Split(const wstring &str, wchar_t delimiter)
{
	vector<wstring> internal;
	wstringstream ss(str);
	wstring tok;

	while (getline(ss, tok, delimiter)) {
		internal.push_back(tok);
	}

	return internal;
}

/// Get unique words count in texts
size_t Baes::GetUniqueCount(vector<wstring>& negative_words, vector<wstring>& positive_words)
{
	set<wstring> unique;
	for (auto &w : negative_words)
		unique.insert(w);

	for (auto &w : positive_words)
		unique.insert(w);

	return unique.size();
}

/// Russian Porter's stemming (snowball) 
void Baes::Stemming(vector<wstring>& words)
{
	auto stemmer = sb_stemmer_new("russian", "UTF_8"); 
	if (stemmer == nullptr)
	{
		wcerr << L"Can't allocate stemmer" << endl;
		return;
	}
	
	for (auto &w : words)
	{
		auto stem = sb_stemmer_stem(stemmer, reinterpret_cast<const sb_symbol *>(w.c_str()), w.size()*2);
		string str = reinterpret_cast<const char *>(stem);
		//w = Util::ToWide(str);
	}

	sb_stemmer_delete(stemmer);
}

Baes::Baes(const wstring& negative_path, const wstring& positive_path) :positive_path(positive_path), negative_path(negative_path), _trained(false)
{
}

/// Training model from negative and positive files
void Baes::Train(int count = -1)
{
	// Reading files
	wcout << L"Training started..." << endl;
	float start_time = clock();
	GetTexts(count);
	wcout << L"Reading texts: " << double(clock() - start_time) / CLOCKS_PER_SEC << " sec." << endl;

	// Splited words
	negative_words = SplitWords(negative_text);
	positive_words = SplitWords(positive_text);
	
	// Stemming
	start_time = clock();
	Stemming(negative_words);
	Stemming(positive_words);

	wcout << L"Stemming words: " << double(clock() - start_time) / CLOCKS_PER_SEC << " sec." << endl;

	// Filter for stop-words, symbols, links
	// Filter(negative_words);
	// Filter(positive_words);

	// Количество текстов в обучающей выборке в классе negative
	Dn = negative_text.size();
	// Количество текстов в обучающей выборке в классе positive
	Dp = positive_text.size();
	// Общее число текстов в обучающей выборке
	D = Dn + Dp;
	// Количество уникальных слов во всех текстах
	V = GetUniqueCount(negative_words, positive_words);
	// Суммарное количество слов в текстах класса negative
	Ln = negative_words.size();
	// Суммарное количество слов в текстах класса positive
	Lp = positive_words.size();

	_trained = true;

	wcout << L"Training end." << endl;

#ifdef DEBUG
	wofstream of(L"output", ios::out);
	for (auto &t : negative_text)
	{
		of << t << endl;
	}
#endif
}

/// Saving model
void Baes::Save(wstring& file_name)
{
	// TODO Save model
	wcout << L"Saving model..." << endl;
	wofstream of(file_name,  ios::binary);
	of << negative_text.size() << endl;
	for (auto &t: negative_text)
		of << t << endl;
	
	of << positive_text.size() << endl;
	for (auto &t : positive_text)
		of << t << endl;

	of << negative_words.size() << endl;
	for (auto &t : negative_words)
		of << t << endl;

	of << positive_words.size() << endl;
	for (auto &t : positive_words)
		of << t << endl;
	of << Dn << endl << Dp << endl << D << endl << V << endl << Ln << endl << Lp << endl;

	of.close();
	wcout << L"Model saved to: " << file_name << endl;
}

/// Loading model from file
bool Baes::Load(wstring& file_name)
{
	_trained = false;

	wcout << L"Model loading..." << endl;
	
	wifstream file(file_name, ios::in);
	wstring line{};

	try
	{
		// negative text
		getline(file, line);
		auto nt_count = stoi(line);
		negative_text.clear();
		for (auto i = 0; i < nt_count; ++i)
		{
			getline(file, line);
			negative_text.push_back(line);
		}

		// positive text
		getline(file, line);
		auto pt_count = stoi(line);
		positive_text.clear();
		for (auto i = 0; i < pt_count; ++i)
		{
			getline(file, line);
			positive_text.push_back(line);
		}

		// negative words
		getline(file, line);
		auto nw_count = stoi(line);
		negative_words.clear();
		for (auto i = 0; i < nw_count; ++i)
		{
			getline(file, line);
			negative_words.push_back(line);
		}

		// positive words
		getline(file, line);
		auto pw_count = stoi(line);
		positive_words.clear();
		for (auto i = 0; i < pw_count; ++i)
		{
			getline(file, line);
			positive_words.push_back(line);
		}

		// of << Dn << endl << Dp << endl << D << endl << V << endl << Ln << endl << Lp << endl;
		getline(file, line);
		Dn = stof(line);

		getline(file, line);
		Dp = stof(line);

		getline(file, line);
		D = stof(line);

		getline(file, line);
		V = stof(line);

		getline(file, line);
		Ln = stof(line);

		getline(file, line);
		Lp = stof(line);

		_trained = true;
	}
	catch (...)
	{
		wcerr << L"Can't read " << file_name << endl;
	}

	file.close();

	wcout << L"Model loaded." << endl;

	return _trained;
}

/// Find prediction for text
Predicted Baes::Predict(const wstring& str)
{
	// c = log(Dc / D) + Q[log((Wic + 1) / (V + Lc))]
	if (!_trained)
	{
		wcerr << L"Need to train or load model first." << endl;
		return { NULL };
	}

	auto start_time = clock();
	
	// Классфицируемые слова
	auto words = Split(str, L' ');

	Stemming(words);
		
	// Сколько раз i-ое слово встречалось в документах класса negative в обучающей выборке
	map<wstring, int> Wn;
	// Сколько раз i-ое слово встречалось в документах класса positive в обучающей выборке
	map<wstring, int> Wp;
	
	for (auto &w : words)
	{
		Wn.insert({ w, 0 });
		Wp.insert({ w, 0 });

		// Поиск слова w классифицируемого предложения в классе negative
		auto negative_w = find(begin(negative_words), end(negative_words), w);
		if (negative_w != negative_words.end())
		{
			// Если слово не найдено, то нужно создать
			auto iter = Wn.find(w);
			if (iter == Wn.end())
				Wn.insert({ w, 1 });
			else
				iter->second += 1;
		}

		// Поиск слова w классифицируемого предложения в классе positive
		auto positive_w = find(begin(positive_words), end(positive_words), w);
		if (positive_w != positive_words.end())
		{
			// Если слово не найдено, то нужно создать
			auto iter = Wp.find(w);
			if (iter == Wp.end())
				Wp.insert({ w, 1 });
			else
				iter->second += 1;
		}
	}
	wcout << L"Calculated Wn and Wp: " << double(clock() - start_time) / CLOCKS_PER_SEC << L" sec." << endl;

	// Множество слов классифицируемого документа(включая повторы)
	auto Q = words.size();
	
	double lwp = 0;
	double lwn = 0;
	for (auto &w : words)
	{
		lwp += log(double(Wp[w] + 1) / double(V + Lp));
		lwn += log(double(Wn[w] + 1) / double(V + Ln));
	}

	double Cp = log(double(Dp) / double(D)) + lwp;
	double Cn = log(double(Dn) / double(D)) + lwn;

	double Pp = 1 / double(1 + pow(M_E, Cn - Cp));
	double Pn = 1 - Pp;
		
	return { Pp, Pn, str };
}

Baes::~Baes()
{
#ifdef DEBUG
	wcout << L"~Classificator";
#endif
}