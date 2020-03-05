#include "PImageControl.h"



PImageControl::PImageControl()
{
}


PImageControl::~PImageControl()
{
	
}

bool PImageControl::Init()
{
	
	return false;
}

bool PImageControl::Frame()
{
	sprite_.Frame();
	set_collision_box_(collision_box_norm_);
	return false;
}

bool PImageControl::Render()
{
	sprite_.AlphaDrawNotCenter(); ///�̹����� ���, Center���� �׸��� ����� ��Ȱ��ȭ�ؾ� ��(HP-Bar�� �������� �پ��� ���� ��..)
	return false;
}

bool PImageControl::Release()
{
	PRectObject::Release();
	return true;
}

void PImageControl::SetRectListSize(FLOAT_RECT size)
{
	sprite_.set_rect_list_size(size);
}

void PImageControl::set_max_rect_size(FLOAT_RECT size)
{
	max_rect_size = size;
}

FLOAT_RECT & PImageControl::get_max_rect_size()
{
	return max_rect_size;
}

void PImageControl::Draw()
{
	Spawn();
}
