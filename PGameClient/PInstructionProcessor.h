#pragma once
#include "TServerStd.h"
#include "PProtocol.h"
#include "PInstructionManager.h"
#include "PUserManager.h"
#include "PPacketManager.h"
#include "PScene.h"
#include "PObjectDataManager.h"
#include "PNetworkDataStorage.h"
#include "PMeteor.h"
#include "PAllAttack.h"

class PInstructionProcessor : public PSingleton<PInstructionProcessor>
{
private:
	friend class PSingleton<PInstructionProcessor>;
	
	PScene* current_scene_;
	static std::mutex process_mutex_;
	static std::mutex client_mutex_;
	static std::mutex spawn_mutex_;

	const std::wstring path = L"data/character/character_data.txt";
	const std::wstring map_path = L"data/map/map_data.txt";
	const std::wstring status_path = L"data/character/character_status_data.txt";
	const std::wstring animation_path = L"data/character/sprite/animation_list.txt";

	bool loading_end_; //Ÿ�̸� �� ���ӽ��� �� �ٷ� �����ϸ� ����� �Լ����� �����ϴ� Ÿ�̹� ���� 

private:
	PInstructionProcessor();
public:
	~PInstructionProcessor();
	
	//Client only
	void BindScene(PScene* current_scene); //Client Only
	void ProcessInstruction();
	void ProcessClientTask();


public:
	bool Init();
	bool Frame();
	bool Render();
	bool Release();

public:
	void SpawnPlayer(pPoint& pos, WORD id);
	void SpawnOtherPlayer(pPoint& pos, WORD cid);
	void SpawnBossMonster(pPoint& pos, WORD id);
	void ReportPositionMsg();
	PPlayerCharacter* my_character_;

	bool boss_spawn_;
private:
public:
	const bool& get_loading_end();


};

