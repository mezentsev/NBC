# coding: utf-8

import re
from nltk.stem import SnowballStemmer
import sys  
import time
import math
import pickle
import argparse

reload(sys)  
sys.setdefaultencoding('utf8')

class Classificator(object):
	""" Baes classificator with count """
	def __init__(self, negative_path, positive_path):
		super(Classificator, self).__init__()
		self.negative_path = negative_path
		self.positive_path = positive_path

		self.negative_text = list()
		self.positive_text = list()

		self.words = list()

	def get_texts(self, count):
		''' Get text from files '''

		# reading nagative
		with open(self.negative_path, "r") as f:
			for i,row in enumerate(f): 
				if i == count:
					break
				try:
					self.negative_text.append(row.split(";")[3][1:-1])
				except Exception, e:
					pass			

		# reading positive
		with open(self.positive_path, "r") as f:
			for i,row in enumerate(f): 
				if i == count:
					break
				try:
					self.positive_text.append(row.split(";")[3][1:-1])
				except Exception, e:
					pass		

	def get_words(self, text_array):
		''' Get words from text array 
		@return array of word in all texts '''

		words = list()
		for t in text_array:
			words.extend(t.split())

		return words

	def filter_words(self, words):
		''' Removing smiles, symbols, hyperlinks '''

		pass

	def stemming(self, words):
		''' Make stem for each word in array 
		@return array of stemming words	'''

		russian_stemmer = SnowballStemmer('russian')
		stemming = list()
		for w in words:
			try:
				stemming.append(russian_stemmer.stem(w))
			except Exception, e:
				pass

		return stemming

	def train(self, count):
		''' Train model
		@return TrainingModel '''

		start_time = time.time()
		self.get_texts(count)
		print "Reading texts: " + str(time.time() - start_time)

		start_time = time.time()
		negative_words = self.stemming(self.get_words(self.negative_text))
		print "Stemming negative words: " + str(time.time() - start_time)

		start_time = time.time()
		positive_words = self.stemming(self.get_words(self.positive_text))
		print "Stemming positive words: " + str(time.time() - start_time)

		return TrainingModel(self.negative_text, self.positive_text, negative_words, positive_words)

	def load(self, model_path):
		with open(model_path, "r") as f:
			return pickle.load(f)

class TrainingModel(object):
	"""Training model"""
	def __init__(self, negative_text, positive_text, negative_words, positive_words):
		super(TrainingModel, self).__init__()
		# Количество текстов в обучающей выборке в классе negative
		self.Dn = len(negative_text)
		# Количество текстов в обучающей выборке в классе positive
		self.Dp = len(positive_text)
		# Общее число текстов в обучающей выборке
		self.D = self.Dn+self.Dp
		# Количество уникальных слов во всех текстах
		# всех уникальных?
		self.V = len(set(negative_words)) + len(set(positive_words))
		# Суммарное количество слов в текстах класса negative
		self.Ln = len(negative_words)
		# Суммарное количество слов в текстах класса positive
		self.Lp = len(positive_words)


		self.positive_words = positive_words
		self.positive_text = positive_text

		self.negative_text = negative_text
		self.negative_words = negative_words

	def save(self, file_name):
		with open(file_name, "w") as f:
			pickle.dump(self, f)
			print "TrainingModel saved to: " + file_name

	def predict(self, text):
		# Сколько раз i-ое слово встречалось в документах класса negative в обучающей выборке
		#Wn
		# Сколько раз i-ое слово встречалось в документах класса positive в обучающей выборке
		#Wp
		# Множество слов классифицируемого документа (включая повторы)
		#Q

		# разбор текста на слова и прогоняем через стеммер
		russian_stemmer = SnowballStemmer('russian')
		words = list()
		for w in text.split():
			try:
				words.append(russian_stemmer.stem(w))
			except:
				pass

		Wp = dict.fromkeys(words, 0)
		Wn = dict.fromkeys(words, 0)

		for w in words:
			if w in self.positive_words:
				Wp[w] += 1
			if w in self.negative_words:
				Wn[w] += 1

		Q = len(words)

		#c = log(Dc/D) + Q[log((Wic + 1)/(V+Lc))]
		lwp = 0
		lwn = 0
		for i,w in enumerate(words):
			lwp += math.log( float(Wp[w]+1) / float(self.V+self.Lp) )
			lwn += math.log( float(Wn[w]+1) / float(self.V+self.Ln) )

		Cp = math.log(float(self.Dp) / float(self.D)) + lwp
		Cn = math.log(float(self.Dn) / float(self.D)) + lwn

		Pp = 1 / float(1 + math.e ** (Cn - Cp) ) 
		Pn = 1 - Pp

		return {'negative': Pn, 'positive': Pp, 'text': text}

if __name__ == "__main__":
	parser = argparse.ArgumentParser(description='Train and predict.')
	subparsers = parser.add_subparsers(help='List of commands')

	train_parser = subparsers.add_parser('train', help='Training model')
	train_parser.add_argument('--negative', help='Negative.csv path', required=True)
	train_parser.add_argument('--positive', help='Positive.csv path', required=True)
	train_parser.add_argument('--model', help='Model path to save', required=True)
	train_parser.add_argument('--count', help='Training count', type=int, required=True)
	train_parser.set_defaults(which='train')

	predict_parser = subparsers.add_parser('predict', help='Predict model')
	predict_parser.add_argument('--model', required=True, help='Model path to load')
	predict_parser.set_defaults(which='predict')

	args = parser.parse_args()
	if args.which == 'train':
		start_time = time.time()

		baes = Classificator(args.negative, args.positive)
		model = baes.train(args.count)
		model.save(args.model)

		print "Execution time: " + str(time.time() - start_time)
	elif args.which == 'predict':
		baes = Classificator(None, None)
		start_time = time.time()
		model = baes.load(args.model)
		print "Loading: " + str(time.time() - start_time)

		test = list()
		test.append(model.predict("Пост хрень, да и всё херово кончилось в фильме :("))
		test.append(model.predict("Нарвалы любят единорогов больше жизни"))
		test.append(model.predict("Корабли лавировали лавировали, да всё хорошо кончилось :)"))
		test.append(model.predict("Меня на выходе дверью пришибло, поэтому на троечку"))
		test.append(model.predict("Хамло!"))
		test.append(model.predict("Да успокойтесь, барышня, всё не так плохо ;-)"))

		for x in test:
			print x.get("text") + ": positive=" + str(x.get("positive")) + ", negative=" + str(x.get("negative"))

		print "Execution time: " + str(time.time() - start_time)
