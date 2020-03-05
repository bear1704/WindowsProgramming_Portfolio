#pragma once
#include "PStd.h"
#include "PButtonControl.h"
#include "PImageControl.h"

class PUIDataManager : public PSingleton<PUIDataManager>
{
private:
	PUIDataManager();
public:
	virtual ~PUIDataManager();
private:
	friend class PSingleton<PUIDataManager>;
	std::map<std::wstring, PUIComponent*> ui_composition_list_;
	int index_;
	bool need_load_data_;
public:
	bool Init() override;
	bool Frame() override;
	bool Render() override;
	bool Release() override;

	PUIComponent* get_ui_composition_list_from_map(std::wstring key);
	void LoadDataFromScript(multibyte_string filepath);  //���Ͽ��� ��������Ʈ ����(Rect ��ǥ ��...)�� ���� �о �ʿ� �����Ѵ�. name maxframe lifetime once_playtime path
	//bool Delete(int key);


};
