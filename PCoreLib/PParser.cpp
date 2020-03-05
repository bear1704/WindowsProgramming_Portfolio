#include "PParser.h"

PParser::PParser()
{
}

PParser::~PParser()
{
}

int PParser::XmlParse(std::string path, std::vector<std::pair<string, string>>* data_map)
{
	std::ifstream in_stream(path);
	//char buffer[kParsingLineMaxSize];
	std::string str;
	if (in_stream.is_open())
	{
		while (!in_stream.eof())
		{
			std::string type_string;
			std::string type_endstring;
			std::string middle_string;
			size_t start_position = 0; // �̵齺Ʈ���� ù��°
			size_t end_position = 0; // �̵齺Ʈ���� ������
			bool is_comment_line = false; //�ڸ�Ʈ �������� üũ
			
			std::getline(in_stream, str);

			if (str == "\t" || str == "") //���鰨��
				continue;
			
			std::regex type_start("<[a-zA-Z_]*>");
			std::regex type_end("<[\/]+[a-zA-Z_]*>");
			std::regex comment("<!--[a-zA-Z0-9-_\.\:\!\?\\s\(\)\']*-->");

			std::sregex_iterator it_start(str.begin(), str.end(), type_start);
			std::sregex_iterator end;
			std::sregex_iterator it_end(str.begin(), str.end(), type_end);
			std::sregex_iterator it_comment(str.begin(), str.end(), comment);
		
			while (it_comment != end && it_start == end && it_end == end) //���� �ּ��˻�
			{
				is_comment_line = true;
				it_comment++;
			}
			if (is_comment_line) //�ּ� ��ȿ 
			{
				is_comment_line = false;
				continue;
			}

			while(it_start != end)
			{
				std::smatch m = *it_start;
				type_string = m.str(0).substr(1, m.str().size() - 2); //<>�� �����ϰ� ȣ��        
				start_position = m.position() + m.str().size();
				it_start++;
			}			
			while(it_end != end)
			{
				std::smatch m = *it_end;
				type_endstring = m.str(0).substr(1, m.str().size() - 2); //<>�� �����ϰ� ȣ��
				end_position = m.position();
				it_end++;
			}
		
		
			if(end_position > 0 && start_position > 0) //�̵彺Ʈ�� ����, ���԰���
			{
				middle_string = str.substr(start_position, end_position - start_position);
				middle_string.erase(remove_if(middle_string.begin(), middle_string.end(), isspace), middle_string.end()); //��������
				auto pair = std::make_pair(type_string, middle_string);
				data_map->emplace_back(pair);
				//std::cout << "type : " << type_string << " | midstring :" << middle_string << std::endl;
			}
			else //�̵彺Ʈ���� ����
			{
				if (start_position == 0)//�̵彺Ʈ���� ����, end
				{
					auto pair = std::make_pair("END", type_endstring);
					data_map->emplace_back(pair);
					//std::cout << "type(end) : " << type_endstring << std::endl;
				}
				else //�̵彺Ʈ���� ����, start
				{
					auto pair = std::make_pair("START", type_string);
					data_map->emplace_back(pair);
					//std::cout << "type(start) : " << type_string << std::endl;
				}
			}
			
		}
	}
	else
	{
		std::cout << "���� �б� ����" << std::endl;
	}

	in_stream.close();
	return 0;

}

std::vector<std::string> PParser::SplitString(std::string str, char delimiter)
{
	std::string::iterator str_start = str.begin();
	std::string::iterator str_iter;
	std::vector<std::string> strvec;
	for (str_iter = str.begin(); str_iter != str.end(); str_iter++)
	{

		if (*str_iter == delimiter)
		{
			std::string temp_str(str_start, str_iter);

			strvec.push_back(temp_str);
			str_start = str_iter + 1;
		}

	}
	std::string temp_str(str_start, str.end());
	strvec.push_back(temp_str);
	return strvec;
}

