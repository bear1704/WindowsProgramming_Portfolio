#pragma once
#include <string>
#include <map>
#include <fstream>
#include <regex>
#include <iostream>

using namespace std;

const int kParsingLineMaxSize = 256;

class PParser
{
public:
	PParser();
	~PParser();
private:
	

public:
	int XmlParse(std::string path, std::vector<std::pair<string,string>>* data_map); //��ȯ�� : �����ͼ�
	std::vector<std::string> SplitString(std::string str, char delimiter);

};

