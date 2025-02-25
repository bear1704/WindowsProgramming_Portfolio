#pragma once
#include "PUIComponent.h"

enum BTN
{
	NORMAL,
	HOVER,
	PUSH,
	DISABLE,
};



class PButtonControl : public PUIComponent
{
public:
	PButtonControl();
	~PButtonControl();
public:
	bool is_collision;
	bool is_select;
	void Draw() override;
	//bool Set(TObjectInfo info, int id, int mask);
	bool  Frame();
	bool  Release();
	PUIComponent* Clone();
protected:
	int current_sprite_rect;
	int state_normal;
	int state_hover;
	int state_push;
	int state_disable;
};

