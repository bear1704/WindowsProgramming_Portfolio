#pragma once
#include "PSelectModel.h"

#define MAX_PLAYER 8


class PNetwork //PClient?
{
protected:
	std::shared_ptr<PSelectModel> current_model_;
	c_string			ip_adress_;
	int					port_number_;
	HWND				hwnd_;
	SOCKET				socket_;  //client only
	SOCKET				listen_sock_; //server only


public:
	bool CreateSock();
	bool SendFromPacketPool(); //��ŶǮ�κ��� ��Ŷ�� ������ ����
	bool SendMsg(int type, char* msg_data, int msg_size); //��Ŷ�� ������ ����.
	bool Connect(const char* ip_address, int port_number, HWND hwnd);
	bool BindListen();
	bool set_current_model(Sptr<PSelectModel> model);
	bool ShutDown();
	LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static bool connect_;


public:
	virtual bool Init();
	virtual bool Frame();
	virtual bool Render();
	virtual bool Release();

public:
	SOCKET& get_socket();
	SOCKET& get_listen_sock();
	Sptr<PSelectModel>& get_current_model();
	static bool is_connect();
	static void set_connect(bool connect);

public:
	PNetwork();
	virtual ~PNetwork();

};

