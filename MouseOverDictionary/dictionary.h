#pragma once

#include <string>
#include <unordered_map>

#include <iostream>
#include <fstream>

#include <QFile>
#include <QTextStream>
#include <QTextCodec>
#include <QApplication>
#include <QDir>

enum DictionaryFormat
{
	EJDIC,
	EIJIRO,
	PDIC1,
	OTHER,
};

class Dictionary
{
public:
	Dictionary();

	bool Load(std::string foldername);
	bool Find(std::string word, std::string& text);

private:
	std::unordered_map<std::string, std::string> data;

	bool LoadEJDIC(std::string filename);
	bool LoadEIJIRO(std::string filename);
	bool LoadPDIC1(std::string filename);
	enum DictionaryFormat getFormat(std::string filename);

};