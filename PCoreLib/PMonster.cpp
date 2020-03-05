#include "PMonster.h"
#include "PMobIdleAction.h"
#include "PMobAttackAction.h"
#include "PMobMoveAction.h"
#include "PMobJumpAction.h"
#include "PMobDeadAction.h"
#include "PMobHitAction.h"
#include "PMobReviveAction.h"


PMonster::PMonster()
{

}


PMonster::~PMonster()
{
}

bool PMonster::Init()
{
	

	action_list_.insert((std::make_pair(FSM_State::IDLE, new PMobIdleAction(this))));
	action_list_.insert((std::make_pair(FSM_State::MOVE, new PMobMoveAction(this))));
	action_list_.insert((std::make_pair(FSM_State::JUMP, new PMobJumpAction(this))));
	action_list_.insert((std::make_pair(FSM_State::HIT, new PMobHitAction(this))));
	action_list_.insert((std::make_pair(FSM_State::ATTACK, new PMobAttackAction(this))));
	action_list_.insert((std::make_pair(FSM_State::DEAD, new PMobDeadAction(this))));
	action_list_.insert((std::make_pair(FSM_State::REVIVE, new PMobReviveAction(this))));

	current_monster_action_ = action_list_[FSM_State::IDLE];
	current_monster_state_ = FSM_State::IDLE;


	//MOVE
	monster_fsm_.Add(FSM_State::IDLE, FSM_Event::MOB_TIME_OUT, FSM_State::MOVE);
	monster_fsm_.Add(FSM_State::MOVE, FSM_Event::MOB_TIME_OUT, FSM_State::IDLE);
	
	//JUMP
	monster_fsm_.Add(FSM_State::IDLE, FSM_Event::MOB_JUMP_TIME_OUT, FSM_State::JUMP);
	monster_fsm_.Add(FSM_State::MOVE, FSM_Event::MOB_JUMP_TIME_OUT, FSM_State::JUMP);
	monster_fsm_.Add(FSM_State::JUMP, FSM_Event::JUMP_END, FSM_State::MOVE);

	//HIT
	monster_fsm_.Add(FSM_State::IDLE, FSM_Event::HIT, FSM_State::HIT);
	monster_fsm_.Add(FSM_State::MOVE, FSM_Event::HIT, FSM_State::HIT);
	monster_fsm_.Add(FSM_State::JUMP, FSM_Event::HIT, FSM_State::HIT);
	monster_fsm_.Add(FSM_State::HIT, FSM_Event::MOB_CHASE, FSM_State::ATTACK);
	monster_fsm_.Add(FSM_State::HIT, FSM_Event::HPEMPTY, FSM_State::DEAD);

	//ATTACK
	monster_fsm_.Add(FSM_State::ATTACK, FSM_Event::MOB_TIME_OUT, FSM_State::IDLE);
	monster_fsm_.Add(FSM_State::ATTACK, FSM_Event::HIT, FSM_State::HIT);

	//DEAD
	monster_fsm_.Add(FSM_State::DEAD, FSM_Event::MOB_TIME_OUT, FSM_State::REVIVE);
	//REVIVE
	monster_fsm_.Add(FSM_State::REVIVE, FSM_Event::MOB_TIME_OUT, FSM_State::IDLE);
	monster_fsm_.Add(FSM_State::REVIVE, FSM_Event::HIT, FSM_State::HIT);

	PUIDataManager::GetInstance().LoadDataFromScript(L"data/UI/UI_composition_list.txt");
	dmg_comp = PUIDataManager::GetInstance().get_ui_composition_list_from_map(L"DMGPRESENT");
	
	

	return false;
}

bool PMonster::Frame()
{
	SavePrevPosition();
	sprite_.Frame();
	set_collision_box_(collision_box_norm_);
	ProcessAction(target_player_);
	Movement();
	physics_.Gravity(position_, gravity_);
	physics_.Jump(physics_.jump_init_time, position_, 650, 0.25f);
	PlatformWallCollision();
	MonsterWallCollision();
	dmg_comp->ReviseAllComponentPosition(dmg_comp->hit_world_pos);
	return false;
}

bool PMonster::Render()
{
	Spawn();
	dmg_comp->Render();
	return false;
}

bool PMonster::Release()
{
	sprite_.Release();
	return false;
}

void PMonster::Set(multibyte_string data_path, multibyte_string object_name, pPoint position)
{
	PObjectInfoManager::GetInstance().LoadDataFromScript(data_path, ObjectLoadType::CHARACTER);
	ObjectInfo info = *(PObjectInfoManager::GetInstance().get_object_info_list_from_map(object_name));

	character_name_ = info.object_name_;
	object_name_ = info.object_name_;
	position_ = position;
	collision_box_norm_ = info.collision_box_;
	alpha_ = info.alpha_;
	scale_ = info.scale_;
	move_speed_ = info.move_speed_;

	//PSpriteManager::GetInstance().LoadDataFromScript(info.sprite_path);
	//sprite_.Set(*PSpriteManager::GetInstance().get_sprite_data_list_from_map(info.sprite_name), alpha_, scale_);
	PSpriteManager::GetInstance().LoadSpriteDataFromScript(info.sprite_path, ObjectLoadType::CHARACTER);
	sprite_.Clone(PSpriteManager::GetInstance().get_sprite_from_map_ex(info.sprite_name), alpha_, scale_);
	
	sprite_.SetPosition(position_.x, position_.y);

	FLOAT_RECT scaled_collisionbox_norm = { collision_box_norm_.left*scale_, collision_box_norm_.top*scale_ ,
	collision_box_norm_.right*scale_, collision_box_norm_.bottom*scale_ };

	collision_box_norm_ = scaled_collisionbox_norm;
	set_collision_box_(collision_box_norm_);

	spawn_position_ = position;

	my_direction_side_ = SIDE::RIGHT;
	
	//���ʹ� ī�޶� ������ ����.
}

void PMonster::MonsterWallCollision()
{

	const std::vector<FLOAT_RECT>& monster_wall_list = PWallAndPlatform::GetInstance().get_monster_wall_list_();

	for (auto& it : monster_wall_list)
	{
		if (PCollision::GetInstance().RectInRect(it, collision_box_))
		{
			if (my_direction_side_ == SIDE::LEFT)
			{
				my_direction_side_ = SIDE::RIGHT;
				position_.x = it.left + it.right + collision_box_.right / 2 + 1.0f; //1.0f =  ����ġ
			}
			else
			{
				my_direction_side_ = SIDE::LEFT;
				position_.x = it.left -  collision_box_.right / 2  - 1.0f;
			}
		}
	}
}

void PMonster::Movement()
{
	sprite_.SetPosition(position_.x, position_.y);
}

SIDE PMonster::get_direction_side_()
{
	return my_direction_side_;
}

void PMonster::set_direction_side_(SIDE side)
{
	my_direction_side_ = side;
}

void PMonster::set_target_player_(PPlayerCharacter * player)
{
	target_player_ = player;
}

void PMonster::SetTransition(FSM_Event event)
{
	PFiniteState* state = monster_fsm_.get_state(current_monster_state_); //FSM�������� �÷��̾��� ���� ������Ʈ�� �����´�.
	if (!state) assert(!state); //���� ������Ʈ�� ������ ���� 

	FSM_State next = state->get_next_state(event); //���� ������Ʈ���� �̺�Ʈ�� Ʈ�����ǵǴ� ���� ������Ʈ ��������
	
	if (next == FSM_State::ERR) //���� ������Ʈ�� ������ ���� ������Ʈ ����
		next = current_monster_state_;
	
	current_monster_action_ = action_list_[next]; //�� ���� ������Ʈ�� �°� �׼� ��������.(������Ʈ ��ȯ)
	current_monster_state_ = next;  //���� ������Ʈ ������ ����������Ʈ�� ����Ű�� ��ȯ

}

void PMonster::ProcessAction(PPlayerCharacter* target)
{
	current_monster_action_->Process(target);
}

bool PMonster::check_hit(FLOAT_RECT player_attack_col)
{
	if (PCollision::GetInstance().RectInRect(player_attack_col, collision_box_))
		return true;

	return false;
}

void PMonster::set_ishit_(bool hit)
{
	ishit_ = hit;
}

bool PMonster::get_ishit_()
{
	return ishit_;
}

void PMonster::set_enemy_to_direction_side_(SIDE side)
{
	enemy_to_direction_side_ = side;
}

SIDE PMonster::get_enemy_to_direction_side_()
{
	return enemy_to_direction_side_;
}

void PMonster::set_be_received_damage_(int damage)
{
	be_received_damage_ = damage;
}

int PMonster::get_be_received_damage()
{
	return be_received_damage_;
}







FSM_State PMonster::get_current_monster_state_()
{
	return current_monster_state_;
}
