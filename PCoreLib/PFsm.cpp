#include "PFsm.h"


PFsm::PFsm()
{
}


PFsm::~PFsm()
{
}

void PFsm::Add(FSM_State key_inputstate, FSM_Event event, FSM_State key_outputstate)
{
	auto iter = state_space_.find(key_inputstate);

	PFiniteState* find_state = nullptr;
	if (iter == state_space_.end()) //��ǲ������Ʈ�� ������Ʈ ������ ���ٸ� 
	{
		auto data = make_shared<PFiniteState>();
		state_space_[key_inputstate] = data;
		find_state = data.get();
	}
	else //������ �̹� �־��ٸ�, output������Ʈ�� �߰����ش�.
	{
		find_state = iter->second.get();
	}

	find_state->next_state_list_[event] = key_outputstate;

}

PFiniteState * PFsm::get_state(FSM_State key)
{
	auto iter = state_space_.find(key);
	if (iter == state_space_.end()) return nullptr;

	PFiniteState* data = iter->second.get();
	return data;


	return nullptr;
}
