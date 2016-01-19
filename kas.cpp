#include <iostream>
#include <ctime>
#include <fstream>
#include "Baes.h"
#include "Util.h"

int main(int argc, char * argv[])
{
	wstring negative_path = L"negative.csv";
	wstring positive_path = L"positive.csv";
	wstring model_file	  = L"b.model";
	
		Baes model(negative_path, positive_path);
		model.Train(1000);
		model.Save(model_file);
		model.Load(model_file);

		Predicted models[] = {
			model.Predict(L"Пост хрень, да и всё херово кончилось в фильме :("),
			model.Predict(L"Нарвалы любят единорогов больше жизни"),
			model.Predict(L"Меня на выходе дверью пришибло, поэтому на троечку"),
			model.Predict(L"Корабли лавировали лавировали, да всё хорошо кончилось :)")
		};

		for (const auto &m : models)
		{
			wcout << L"Text: '" << m.text << L"' (P: " << m.positive << L", N: " << m.negative << ")" << endl;
		}
	
	return 0;
}