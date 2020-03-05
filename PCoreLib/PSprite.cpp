#include "PSprite.h"



PSprite::PSprite()
{
	isDead = false;
	alpha_ = 1.0f;
	is_dmg_ = false;
}


PSprite::~PSprite()
{
}

bool PSprite::Init()
{
	return false;
}

bool PSprite::Frame()
{
	if (remain_lifetime_ <= 0)
	{
		isDead = true;
	}
	if (isDead == true) return true;

	remain_lifetime_ -= g_SecondPerFrame;
	
	time_after_spriteopen_ += g_SecondPerFrame;
	if (time_after_spriteopen_ > allocatetime_for_onesprite)
	{
		current_played_spriteframe_++;
		if (current_played_spriteframe_ >= number_of_max_spriteframe_)
		{
			current_played_spriteframe_ = 0;
		}
		time_after_spriteopen_ = 0;
	}

	return true;
}

bool PSprite::Render()
{
	pPoint scrpos = P2DCamera::GetInstance().WorldToGamescreen(position_);
	pPoint origin_pos = (position_);
	SetPosition(scrpos.x, scrpos.y);
	Draw(is_reversal_for_automata_);
	SetPosition(origin_pos.x, origin_pos.y);
	return true;
}

bool PSprite::Release()
{
	if (isDead)
	{
		if(bitmap_ != nullptr)
			bitmap_->Release();
		if(bitmap_mask_ != nullptr)
			bitmap_mask_->Release();
	}
	return false;
}

bool PSprite::Load(std::wstring filename)
{
	int key = PBitmapManager::GetInstance().BitmapLoad(filename, PLoadMode::BITMAP);
	if (key != -1)
		bitmap_ = PBitmapManager::GetInstance().get_bitmap_from_map(key);

	int key_mask = PBitmapManager::GetInstance().BitmapLoad(filename, PLoadMode::BITMAPMASK);
	if (key_mask != -1)
		bitmap_mask_= PBitmapManager::GetInstance().get_bitmap_from_map(key_mask);

	return true;
}

bool PSprite::Set(SpriteDataInfo info, float alpha, float scale = 1.0f)
{
	rect_list = info.rect_list;
	original_size_list = info.rect_list;
	if (info.lifetime == 777)
	{
		remain_lifetime_ = 999999999.5f;
		lifetime_ = 999999999.5f;
	}
	else
	{
		remain_lifetime_ = info.lifetime;
		lifetime_ = info.lifetime;
	}
	
	position_.x = info.posX;
	position_.y = info.posY;
	number_of_max_spriteframe_ = info.max_frame;
	allocatetime_for_onesprite = info.once_playtime / info.max_frame;
	alpha_ = alpha;
	scale_ = scale;
	current_played_spriteframe_ = 0;

	Load(info.bitmap_path);

	
	return true;
}

bool PSprite::SetPosition(float x, float y)
{
	position_.x = x;
	position_.y = y;
	return false;
}

void PSprite::Play()
{
	isDead = false;
	remain_lifetime_ = lifetime_;
}

void PSprite::Draw(bool is_reversal)
{
	if (isDead)
		return;

	BLENDFUNCTION bf;
	bf.BlendOp = AC_SRC_OVER;
	bf.BlendFlags = 0;
	bf.SourceConstantAlpha = alpha_ * 255.f;
	if (bitmap_->bitmap_info.bmBitsPixel == 32) //ARGB �̹���
	{
		bf.AlphaFormat = AC_SRC_ALPHA;
		bitmap_->Draw(position_.x, position_.y,
			rect_list[current_played_spriteframe_], bf, scale_, is_reversal);
	}
	else // RGB �̹���
	{
		bf.AlphaFormat = AC_SRC_OVER;
		if (bitmap_mask_ != nullptr)
		{
			Alpha24BitsDraw(*this, alpha_, scale_);
		}
		else
		{
			bitmap_->Draw(position_.x, position_.y,
				rect_list[current_played_spriteframe_], SRCCOPY, scale_);

		}
	}
}

void PSprite::Clone(PSprite* sprite, float alpha, float scale)
{
	//rect_list.reserve(sprite->get_rect_list_copy().size());
	rect_list = sprite->get_rect_list_copy();
	original_size_list = rect_list;
	remain_lifetime_ = sprite->get_remain_lifetime_();
	lifetime_ = sprite->get_lifetime_();
	position_.x = sprite->get_position_().x;
	position_.y = sprite->get_position_().y;
	number_of_max_spriteframe_ = sprite->get_max_sprite_number();
	allocatetime_for_onesprite = sprite->get_allocatetime_for_onesprite();
	alpha_ = alpha;
	scale_ = scale;
	current_played_spriteframe_ = 0;
	if(sprite->get_bitmap_() != nullptr)
		bitmap_ = sprite->get_bitmap_();
	
	if (sprite->get_bitmap_mask_() != nullptr)
		bitmap_mask_ = get_bitmap_mask_();


	automata_ = false;
}

void PSprite::AutomataClone(PSprite* sprite, float alpha, float scale, bool is_reversal, pPoint position)
{
	rect_list = sprite->get_rect_list_copy();
	original_size_list = rect_list;
	remain_lifetime_ = sprite->get_remain_lifetime_();
	lifetime_ = sprite->get_lifetime_();
	position_ = position;
	number_of_max_spriteframe_ = sprite->get_max_sprite_number();
	allocatetime_for_onesprite = sprite->get_allocatetime_for_onesprite();
	alpha_ = alpha;
	scale_ = scale;
	current_played_spriteframe_ = 0;
	if (sprite->get_bitmap_() != nullptr)
		bitmap_ = sprite->get_bitmap_();

	if (sprite->get_bitmap_mask_() != nullptr)
		bitmap_mask_ = get_bitmap_mask_();

	automata_ = true;
	is_reversal_for_automata_ = is_reversal;
}



void PSprite::AlphaDrawNotCenter()
{
	if (isDead)
		return;

	assert(bitmap_->bitmap_info.bmBitsPixel == 32); //32��Ʈ ����ä���� �ִ� �̹����� �ƴϸ� ����� �� ����!(UI����)

	BLENDFUNCTION bf;
	bf.BlendOp = AC_SRC_OVER;
	bf.BlendFlags = 0;
	bf.SourceConstantAlpha = alpha_ * 255.f;

		bf.AlphaFormat = AC_SRC_ALPHA;
		bitmap_->DrawNotCenter(position_.x, position_.y,
			rect_list[current_played_spriteframe_], bf, scale_);


}



bool PSprite::Alpha24BitsDraw(PSprite sprite, float alpha, float scale, HDC colorDC, HDC maskDC)
{

	//����! ��ǥ(sprite_coord.txt����)���� ���� ����� left, top, right, bottom�� �ƴ϶�  left, right-left, top, bottom-top ��! abs
	int sprite_width = sprite.rect_list[current_played_spriteframe_].right;
	int sprite_height = sprite.rect_list[current_played_spriteframe_].bottom;
	int stretched_width = sprite.rect_list[current_played_spriteframe_].right * scale; 
	int stretched_height = sprite.rect_list[current_played_spriteframe_].bottom * scale;
	float half_width = stretched_width / 2;
	float half_height = stretched_height / 2;


	HDC bitmapDC; HDC mask_bitmapDC;

	if (colorDC) // ���� ȸ�����̶� colorDC�� ���ĺ���� �Ѿ�´ٸ�, colorDC�� ����� bitmap DC�� �� ���̴�.
		bitmapDC = colorDC;
	else
		bitmapDC = sprite.get_bitmap_()->handle_memoryDC_;

	if (maskDC) //���� ���� 
		mask_bitmapDC = maskDC;
	else if (sprite.get_bitmap_mask_() != nullptr)
		mask_bitmapDC = sprite.get_bitmap_mask_()->handle_memoryDC_;
	else
		mask_bitmapDC = NULL;


	//FLOAT_RECT rectangle_client;
	//GetClientRect(g_hWnd, &rectangle_client);
	//FLOAT_RECT rectangle_client_for_idiot;
	//GetClientRect(g_hWnd, &rectangle_client_for_idiot);

	BITMAPINFO bmi;
	ZeroMemory(&bmi, sizeof(BITMAPINFO));
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = stretched_width;
	bmi.bmiHeader.biHeight = stretched_height;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biSizeImage = stretched_width * stretched_height * 4;

	void* pivot_bitmap = 0;
	void* pivot_mask_bitmap = 0;
	void* pivot_offscreen_bitmap = 0;

	HBITMAP handle_bitmap = CreateDIBSection(g_handle_screenDC, &bmi, DIB_RGB_COLORS, &pivot_bitmap, NULL, 0);
	HDC bitmap_dc = CreateCompatibleDC(NULL);
	SelectObject(bitmap_dc, handle_bitmap);

	StretchBlt(bitmap_dc, 0, 0, stretched_width, stretched_height, bitmapDC,
		sprite.rect_list[current_played_spriteframe_].left, sprite.rect_list[current_played_spriteframe_].top,
		sprite_width, sprite_height, SRCCOPY);
	DeleteDC(bitmap_dc);

	HBITMAP handle_mask_bitmap = CreateDIBSection(g_handle_screenDC, &bmi, DIB_RGB_COLORS, &pivot_mask_bitmap, NULL, 0);
	HDC bitmap_mask_dc = CreateCompatibleDC(NULL);
	SelectObject(bitmap_mask_dc, handle_mask_bitmap);

	StretchBlt(bitmap_mask_dc, 0, 0, stretched_width, stretched_height, mask_bitmapDC,
		sprite.rect_list[current_played_spriteframe_].left, sprite.rect_list[current_played_spriteframe_].top,
		sprite_width, sprite_height, SRCCOPY);
	DeleteDC(bitmap_mask_dc);

	HBITMAP handle_offscreen_bitmap = CreateDIBSection(g_handle_screenDC, &bmi, DIB_RGB_COLORS, &pivot_offscreen_bitmap, NULL, 0);
	HDC bitmap_offscreen_dc = CreateCompatibleDC(NULL);
	SelectObject(bitmap_offscreen_dc, handle_offscreen_bitmap);

	StretchBlt(bitmap_offscreen_dc, 0, 0, stretched_width, stretched_height, g_handle_off_screenDC,
		sprite.position_.x - half_width, sprite.position_.y - half_height, stretched_width, stretched_height, SRCCOPY);
	//����� �þ ũ�⸸ŭ �߶�� ��� �Ѵ�. ���� ���� ����̹Ƿ� ���� - FLOAT_RECT/2��ŭ�� ��ġ���� �����;� �Ѵ�. 
	DeleteDC(bitmap_offscreen_dc);

	for (int y = 0; y < stretched_height; y++)
	{
		byte* bitmap_RGB = (byte*)&((DWORD*)pivot_bitmap)[y * stretched_width];
		byte* bitmap_mask_RGB = (byte*)&((DWORD*)pivot_mask_bitmap)[y* stretched_width];
		byte* bitmap_offscreen_RGB = (byte*)&((DWORD*)pivot_offscreen_bitmap)[y* stretched_width];

		for (int x = 0; x < stretched_width; x++)
		{
			byte origin_A = bitmap_RGB[3];
			byte origin_R = bitmap_RGB[2];
			byte origin_G = bitmap_RGB[1];
			byte origin_B = bitmap_RGB[0];

			if (bitmap_mask_RGB[0] == 255 && bitmap_mask_RGB[1] == 255 && bitmap_mask_RGB[2] == 255)
			{    //����ũRGB�� WHITE�� ���, ������� ó��
				bitmap_RGB[3] = bitmap_offscreen_RGB[3];
				bitmap_RGB[2] = bitmap_offscreen_RGB[2];
				bitmap_RGB[1] = bitmap_offscreen_RGB[1];
				bitmap_RGB[0] = bitmap_offscreen_RGB[0];
			}
			else
			{
				bitmap_RGB[3] = 0xff;
				bitmap_RGB[2] = bitmap_offscreen_RGB[2] * (1.0f - alpha) + origin_R * alpha;
				bitmap_RGB[1] = bitmap_offscreen_RGB[1] * (1.0f - alpha) + origin_G * alpha;
				bitmap_RGB[0] = bitmap_offscreen_RGB[0] * (1.0f - alpha) + origin_B * alpha;
			}

			bitmap_RGB += 4;
			bitmap_mask_RGB += 4;
			bitmap_offscreen_RGB += 4;
		}
	}



	bitmap_dc = CreateCompatibleDC(NULL);
	SelectObject(bitmap_dc, handle_bitmap);

	SetStretchBltMode(g_handle_off_screenDC, HALFTONE);
	StretchBlt(g_handle_off_screenDC,
		sprite.position_.x - half_width, sprite.position_.y - half_height , stretched_width, stretched_height,
		bitmap_dc, 0, 0, stretched_width, stretched_height,
		SRCCOPY);

	DeleteObject(handle_bitmap);
	DeleteObject(handle_mask_bitmap);
	DeleteObject(handle_offscreen_bitmap);
	DeleteObject(bitmap_dc);

	DeleteDC(bitmap_dc);

	return true;
}


void PSprite::set_alpha_(float alpha)
{
	alpha_ = alpha;
}

void PSprite::set_scale_(float scale)
{
	scale_ = scale;
}


PBitmap * PSprite::get_bitmap_()
{
	return bitmap_;
}

PBitmap * PSprite::get_bitmap_mask_()
{
	return bitmap_mask_;
}

vector<FLOAT_RECT> PSprite::get_rect_list_copy()
{
	vector<FLOAT_RECT> copyrect;
	copyrect.resize(rect_list.size());

	for (int i = 0; i < rect_list.size(); i++)
	{
		copyrect[i] = rect_list[i];
	}

	return copyrect;
}

bool PSprite::get_is_dead_()
{
	return isDead;
}

float PSprite::get_scale()
{
	return scale_;
}

pPoint PSprite::get_position_()
{
	return position_;
}

void PSprite::set_current_played_spriteframe_(int current)
{
	current_played_spriteframe_ = current;
}

void PSprite::set_rect_list_size(FLOAT_RECT size)
{
	for (int i = 0; i < rect_list.size(); i++)
	{
		rect_list[i] = size;
	}
}

vector<FLOAT_RECT>& PSprite::get_original_size_list()
{
	return original_size_list;
}

void PSprite::set_animation_type_(ANIMATIONTYPE type)
{
	animation_type_ = type;
}

void PSprite::set_is_dmg(bool isdmg)
{
	is_dmg_ = isdmg;
}

ANIMATIONTYPE PSprite::get_animation_type_()
{
	return animation_type_;
}

float PSprite::get_remain_lifetime_()
{
	return remain_lifetime_;
}

float PSprite::get_lifetime_()
{
	return lifetime_;
}

float PSprite::get_alpha_()
{
	return alpha_;
}

int PSprite::get_max_sprite_number()
{
	return number_of_max_spriteframe_;
}

int PSprite::get_current_played_frame()
{
	return current_played_spriteframe_;
}

float PSprite::get_allocatetime_for_onesprite()
{
	return allocatetime_for_onesprite;
}

bool PSprite::get_is_dmg()
{
	return is_dmg_;
}
