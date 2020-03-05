#pragma once
#include <assert.h>
#include "PSprite.h"



class PSpriteManager : public PSingleton<PSpriteManager>
{
public:
	PSpriteManager();
	~PSpriteManager();
private:
	friend class PSingleton<PSpriteManager>;
	std::map<std::wstring, SpriteDataInfo*> sprite_data_list_;
	int index_;
public:
	bool Init() override;
	bool Frame() override;
	bool Render() override;
	bool Release() override;

	SpriteDataInfo* get_sprite_data_list_from_map(std::wstring key);
	void LoadDataFromScript(multibyte_string filepath);  //���Ͽ��� ��������Ʈ ����(Rect ��ǥ ��...)�� ���� �о �ʿ� �����Ѵ�.
	bool Delete(int key);
	


};

