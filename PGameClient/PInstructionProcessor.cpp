#include "PInstructionProcessor.h"
#include "resource.h"

HWND g_edit_name;
HWND g_hDlg;

std::mutex PInstructionProcessor::process_mutex_;
std::mutex PInstructionProcessor::spawn_mutex_;
std::mutex PInstructionProcessor::client_mutex_;

LRESULT CALLBACK DlgProc(HWND hDlg,
	UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	switch (iMessage)
	{
	case WM_INITDIALOG:
	{
		g_hDlg = hDlg;
		g_edit_name = GetDlgItem(hDlg, IDC_NAMEEDIT);
	}break;
	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case IDOK:
		{
			TCHAR Name[16] = { 0, };
			SendMessage(g_edit_name, WM_GETTEXT, 16, (LPARAM)Name);
			std::wstring name(Name);
			std::string mstr = unicode_to_multibyte_str(name);

			char name_[16] = { 0, };
			WideCharToMultiByte(CP_ACP, 0, Name, lstrlenW(Name), name_, lstrlenW(Name), NULL, NULL);

			PUser* myuser = &(PUserManager::GetInstance().oneself_user_);
			PPacketManager::GetInstance().PushPacket(myuser, PACKET_CS_LOGIN_SEND_USERNAME,
				(char*)name_, mstr.length(), PushType::SEND, true);


			PUserManager::GetInstance().oneself_user_.set_name(mstr); //�ߺ�ó������, ��ǻ�ٷμ��� 
			EndDialog(hDlg, IDOK);

		}break;
		}
	}
	return 0;
	}
}


PInstructionProcessor::PInstructionProcessor()
{
	loading_end_ = false;
}


PInstructionProcessor::~PInstructionProcessor()
{
}


void PInstructionProcessor::BindScene(PScene * current_scene) 
{
	current_scene_ = current_scene;
}

void PInstructionProcessor::ProcessInstruction()
{
	while (g_window_terminated == false)
	{
		//MessageBox(g_hWnd, L"proces", L"proces", MB_OK);
		std::unique_lock<std::mutex> ul(process_mutex_);
		PInstructionManager::GetInstance().process_event_.wait(ul, []()
			{return !PInstructionManager::GetInstance().IsQueueEmpty() || g_window_terminated;}); //ť�� ���� �ʾҴٸ� ����


		if (g_window_terminated == true)
			break;

		//�Ʒ��� ��������
		PACKET packet = PInstructionManager::GetInstance().PopBackInstruction();
		

		switch (packet.ph.type)
		{			
				case PACKET_SC_ID_PROVIDE:
				{
					
					WORD id = *((WORD*)packet.msg);
					PUserManager::GetInstance().oneself_user_.set_id(id);
					DialogBox(g_hInstance, MAKEINTRESOURCE(IDD_DIALOG1), HWND_DESKTOP, DlgProc);
					break;
				}
				case PACKET_SC_SPAWN_CHARACTER: //ĳ���� ������Ű�� ĳ���� ���� ���̵� �ο�
				{
					PKT_MSG_SPAWN pos_msg;
					memcpy(&pos_msg, packet.msg, packet.ph.len - PACKET_HEADER_SIZE);

					pPoint pos = pPoint(pos_msg.posx, pos_msg.posy);
					SpawnPlayer(pos, pos_msg.id);
					

					char msg[sizeof(PKT_MSG_SPAWN)];
					memcpy(msg, &pos_msg, sizeof(PKT_MSG_SPAWN));
					PPacketManager::GetInstance().PushPacket(&PUserManager::GetInstance().oneself_user_, 
						PACKET_CS_SPAWN_COMPLETE,
						msg, sizeof(PKT_MSG_SPAWN), PushType::SEND, true);

					loading_end_ = true;

					break;

				}
				case PACKET_BROADCAST_USERX_SPAWN:
				{
					PKT_MSG_SPAWN broad_msg;
					memcpy(&broad_msg, packet.msg, sizeof(PKT_MSG_SPAWN));
					
					if (broad_msg.id == PUserManager::GetInstance().oneself_user_.get_character_id())
						break;  //���� ��ε�ĳ��Ʈ ��ü�� �ڽ��� ĳ���� iD�̸�, ������ �ʿ� ����.
										
					pPoint pos = pPoint(broad_msg.posx, broad_msg.posy);
					SpawnOtherPlayer(pos, broad_msg.id);
					PUserManager::GetInstance().AddUserSimpleType(packet.ph.id, broad_msg.id); //Ŭ�� ��������Ʈ�� �����߰�

					break;
				}
				case PACKET_BROADCAST_USERX_MOVEAXIS_AtoB:
				{
					PKT_MSG_REGULAR_POS_REPORT report_msg;
					ZeroMemory(&report_msg, sizeof(PKT_MSG_REGULAR_POS_REPORT));
					memcpy(&report_msg, packet.msg, sizeof(PKT_MSG_REGULAR_POS_REPORT));

					if (report_msg.cid == PUserManager::GetInstance().oneself_user_.get_character_id())
						break;

					PPlayerCharacter* userx_character = (PPlayerCharacter*)current_scene_->FindObjectByCid(report_msg.cid);

					if (userx_character == nullptr) //�ʰ� ���� �÷��̾�� other_spawn ó���� �������Ƿ� ���� ���̴�.
					{
						//�׷��� �������� �÷��̾ ������ش�.
						pPoint other_player_pos = pPoint(report_msg.posx, report_msg.posy);
						SpawnOtherPlayer(other_player_pos, report_msg.cid);
						userx_character = (PPlayerCharacter*)current_scene_->FindObjectByCid(report_msg.cid);
					}
					if (report_msg.dir == Direction::RIGHT)
						userx_character->set_right_dir(true);
					else
						userx_character->set_right_dir(false);

					if (report_msg.current_state == FSM_State::MOVE)
					{
						pPoint move_lerp = pPoint((report_msg.posx - userx_character->get_position_().x) / 3,
							(report_msg.posy - userx_character->get_position_().y)/3);
						pPoint user_position = pPoint(userx_character->get_position_());
						userx_character->set_position_(pPoint(user_position.x + move_lerp.x , user_position.y));
						//y���� ����ȭ�� �ʿ� ����.(�̳�Ʀ)
					}
					else if (report_msg.current_state == FSM_State::IDLE)
					{
						userx_character->set_position_(pPoint(report_msg.posx, report_msg.posy));
					}
					else if (report_msg.current_state == FSM_State::JUMP)
					{
						userx_character->get_physics_().StartJump();  
   	 				}
					


					userx_character->SetTransition(FsmStateToFsmEvent(report_msg.current_state));
					
			

					break;
				}
				case PACKET_SC_SPAWN_BOSS:
				{
					PKT_MSG_SPAWN spawn_msg;
					ZeroMemory(&spawn_msg, sizeof(PKT_MSG_SPAWN));
					memcpy(&spawn_msg, packet.msg, sizeof(PKT_MSG_SPAWN));

					pPoint boss_pos = pPoint(spawn_msg.posx, spawn_msg.posy);
					
					SpawnBossMonster(boss_pos, spawn_msg.id);
					boss_spawn_ = true;

					break;
				}
				case PACKET_BROADCAST_USERX_ATTACK_SUCCESS:
				{
				

					PKT_MSG_MONSTER_HIT pmmh;
					ZeroMemory(&pmmh, sizeof(PKT_MSG_MONSTER_HIT));
					memcpy(&pmmh, packet.msg, sizeof(PKT_MSG_MONSTER_HIT));

					if (pmmh.player_user_id == PUserManager::GetInstance().oneself_user_.get_id())
						break;

					PPlayerCharacter* userx_character = (PPlayerCharacter*)current_scene_->FindObjectByCid(pmmh.player_cid);

					pPoint userx_attack_pos = pPoint(userx_character->get_attack_collision_box_().left + 10.0f,
						userx_character->get_attack_collision_box_().top - 30.0f); //hard_coded
					PSpriteManager::GetInstance().CreateDamageFontFromInteger(pmmh.damage, userx_attack_pos);

					PBossMonster* zakum = (PBossMonster*)g_current_scene_->FindObjectByCid(ZAKUM_ID);
					break;
				}
				case PACKET_BROADCAST_USERX_HIT:
				{
					/*WORD cid;
					memcpy(&cid, packet.msg, sizeof(WORD));
					
					if (cid == PUserManager::GetInstance().oneself_user_.get_character_id())
						break;

					PPlayerCharacter* ochar = (PPlayerCharacter*)g_current_scene_->FindObjectByCid(cid);
					ochar->SetTransition(FSM_Event::HIT);


					break;*/
					//hit�� �溸�� ���� ũ��.
					break;
				}
				case PACKET_SC_BOSS_REAMIN_HP:
				{
				/*	if (packet.ph.id == PUserManager::GetInstance().oneself_user_.get_id())
						break;*/

					int boss_remain_hp;
					memcpy(&boss_remain_hp, packet.msg, sizeof(int));
					PBossMonster* pbm = (PBossMonster*)g_current_scene_->FindObjectByCid(ZAKUM_ID);
					pbm->get_status().ModifyHP(boss_remain_hp);
					break;

				}
				case PACKET_BROADCAST_METEOR_ACTIVE:
				{
					PBossMonster* pbm = (PBossMonster*)current_scene_->FindObjectByCid(ZAKUM_ID);

					MeteorRandNumberForPacket data;
					memcpy(&data, packet.msg, sizeof(MeteorRandNumberForPacket));
					
					MeteorRandNumber skill_data;
					skill_data.skillnum = data.skillnum;
					skill_data.initpos.resize(8);
					skill_data.downspeed.resize(8);

					for (int i = 0; i < 8; i++)//hard coded
					{
						skill_data.initpos[i] = data.initpos[i];
						skill_data.downspeed[i] = data.downspeed[i];
					}


					PNetworkDataStorage::GetInstance().AddData(skill_data);
					pbm->StartSkillPhase(0);
					break;
					//test code end
				}
				case PACKET_BROADCAST_ALLATTACK_ACTIVE:
				{
					PBossMonster* pbm = (PBossMonster*)current_scene_->FindObjectByCid(ZAKUM_ID);

					WORD data;
					memcpy(&data, packet.msg, sizeof(WORD));


					PNetworkDataStorage::GetInstance().set_explosion_rand_number_(data);
					pbm->StartSkillPhase(1);
					break;
				}
				case PACKET_BROADCAST_GAMEOVER:
				{
					WORD scene_number;
					memcpy(&scene_number, packet.msg, sizeof(WORD));
					g_current_scene_->SceneChange(3);
					g_window_terminated = true;
					return;
					break;
				}
				case PACKET_BROADCAST_GAMECLEAR:
				{
					WORD scene_number;
					memcpy(&scene_number, packet.msg, sizeof(WORD));
					g_current_scene_->SceneChange(2);
					g_window_terminated = true;

					return;
					break;
				}

		}

	}

}

void PInstructionProcessor::ProcessClientTask()
{
	while (g_window_terminated == false)
	{
		
		std::unique_lock<std::mutex> ul(client_mutex_);
		PInstructionManager::GetInstance().process_event_.wait(ul, []()
			{return !PNetworkDataStorage::GetInstance().IsQueueEmpty() || g_window_terminated; }); //ť�� ���� �ʾҴٸ� ����

		if (g_window_terminated == true)
			break;

		if (PNetworkDataStorage::GetInstance().GetHitListSize() > 0)
		{
			HitData data = PNetworkDataStorage::GetInstance().PopHitData();
			PACKET packet;
			ZeroMemory(&packet, sizeof(PACKET));
			packet.ph.type = PACKET_CS_MONSTER_HIT;
			packet.ph.id = PUserManager::GetInstance().oneself_user_.get_id();
			packet.ph.len = PACKET_HEADER_SIZE + sizeof(PKT_MSG_MONSTER_HIT);

			PKT_MSG_MONSTER_HIT pmmh;
			pmmh.damage = data.damage;
			pmmh.monster_id = data.boss_id;
			pmmh.player_user_id = PUserManager::GetInstance().oneself_user_.get_id();
			pmmh.player_cid = data.player_cid;

			memcpy(packet.msg, &pmmh, sizeof(PKT_MSG_MONSTER_HIT));

			PPacketManager::GetInstance().PushPacket(PushType::SEND, packet);

		}



	}
}

bool PInstructionProcessor::Init()
{
	return false;
}

bool PInstructionProcessor::Frame()
{
	return false;
}

bool PInstructionProcessor::Render()
{
	return false;
}

bool PInstructionProcessor::Release()
{
	return false;
}

void PInstructionProcessor::SpawnPlayer(pPoint& pos, WORD id)
{
	std::lock_guard<std::mutex> lk(spawn_mutex_);
	{
		PPlayerCharacter* component;
		component = new PPlayerCharacter();
		component->Set(path, L"player", pPoint(pos.x, pos.y));
		PObjectDataManager::GetInstance().LoadAnimationDataFromScriptEx(animation_path); //ĳ���� ��������Ʈ ���� �ε� �Ŀ� ��ġ�ؾ� ��.
		component->set_gravity_(450.0f);
		component->set_type_(Type::PLAYER);
		component->get_status().set_owner(component);
		component->StatusSet(status_path, component->get_object_name());
		component->set_animation_list_(PObjectDataManager::GetInstance().get_animation_list_from_map(L"player"));
		component->set_alpha_and_scale_(component->get_alpha_(), component->get_scale_());
		component->set_client_owner_character(true);
		current_scene_->set_target(component);
		component->set_id(id);
		PUserManager::GetInstance().oneself_user_.set_character_id(id);
		component->Init();
		my_character_ = component;

		current_scene_->AddGameObjects(component);
	}
	return;
}

void PInstructionProcessor::SpawnOtherPlayer(pPoint& pos, WORD cid)
{
	std::lock_guard<std::mutex> lk(spawn_mutex_);
	{
		PPlayerCharacter* component;
		component = new PPlayerCharacter();
		component->Set(path, L"other_player", pPoint(pos.x, pos.y));
		PObjectDataManager::GetInstance().LoadAnimationDataFromScriptEx(animation_path); //ĳ���� ��������Ʈ ���� �ε� �Ŀ� ��ġ�ؾ� ��.
		component->set_gravity_(450.0f);
		component->set_type_(Type::OTHER_PLAYER);
		component->get_status().set_owner(component);
		component->StatusSet(status_path, component->get_object_name());
		component->set_animation_list_(PObjectDataManager::GetInstance().get_animation_list_from_map(L"other_player"));
		component->set_alpha_and_scale_(component->get_alpha_(), component->get_scale_());
		component->set_client_owner_character(false);
		current_scene_->set_target(component);
		component->set_id(cid);
		component->Init();

		current_scene_->AddGameObjects(component);
	}
	return;
}

void PInstructionProcessor::SpawnBossMonster(pPoint& pos, WORD id)
{

	std::lock_guard<std::mutex> lk(spawn_mutex_);
	{
		PMeteor* meteor = new PMeteor();
		PNetworkDataStorage::GetInstance().AddData(meteor);
		PAllAttack* allattack = new PAllAttack();
		PNetworkDataStorage::GetInstance().AddData(allattack);

		PBossMonster* component;
		component = new PBossMonster();
		component->Set(path, L"zakum", pPoint(pos.x, pos.y));
		PObjectDataManager::GetInstance().LoadAnimationDataFromScriptEx(animation_path); //ĳ���� ��������Ʈ ���� �ε� �Ŀ� ��ġ�ؾ� ��.
		//component->set_gravity_(450.0f);
		component->set_type_(Type::BOSS_MONSTER);
		component->get_status().set_owner(component);
		component->StatusSet(status_path, component->get_object_name());
		component->set_animation_list_(PObjectDataManager::GetInstance().get_animation_list_from_map(L"zakum"));
		component->set_alpha_and_scale_(component->get_alpha_(), component->get_scale_());
		component->set_id(id);
		component->Init();
		component->AddSkill(meteor);
		component->AddSkill(allattack);

		current_scene_->AddGameObjects(component);
	}
	return;
}

void PInstructionProcessor::ReportPositionMsg()
{
	if (g_window_terminated)
		return;

	PACKET packet;
	packet.ph.id = PUserManager::GetInstance().oneself_user_.get_id();
	packet.ph.len = sizeof(PKT_MSG_REGULAR_POS_REPORT) + PACKET_HEADER_SIZE;
	packet.ph.type = PACKET_CS_REPORT_MYPOSITION;
	PKT_MSG_REGULAR_POS_REPORT posmsg;
	posmsg.cid = PUserManager::GetInstance().oneself_user_.get_character_id();
	PPlayerCharacter* pp = (PPlayerCharacter*)g_current_scene_->FindObjectByCid(posmsg.cid);
	if (pp == nullptr)
		return;

	posmsg.current_state = pp->current_player_state_;
	posmsg.posx = pp->get_position_().x;
	posmsg.posy = pp->get_position_().y;

	//if (g_InputActionMap.leftArrowKey == KEYSTAT::KEY_HOLD)
	//	posmsg.dir = Direction::LEFT;
	//else if (g_InputActionMap.rightArrowKey == KEYSTAT::KEY_HOLD)
	//	posmsg.dir = Direction::RIGHT;

	if(pp->get_is_reversal_())
		posmsg.dir = Direction::RIGHT;
	else
		posmsg.dir = Direction::LEFT;

	memcpy(packet.msg, &posmsg, sizeof(PKT_MSG_REGULAR_POS_REPORT));

	PPacketManager::GetInstance().PushPacket(PushType::SEND, packet);
}

const bool& PInstructionProcessor::get_loading_end()
{
	return loading_end_;
}
