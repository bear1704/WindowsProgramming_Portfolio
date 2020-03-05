#pragma once
#include "TServerStd.h"
#include "PProtocol.h"
#include <string>
#include <vector>
#include <random>
#include <functional>

class PPacketManager;


class PUser
{
private:
	SOCKET		user_socket_;
	HANDLE		user_event_;
	bool		connected_;
	std::string name_;
	WORD		unique_id_;
	WORD		character_id_;
	SOCKADDR_IN client_addr_;
	int			recv_bytes_;
	char		recv_buffer_[PACKET_MAX_DATA_SIZE + PACKET_HEADER_SIZE]; //2052
	bool		is_dead_;


public:
	PUser();
	~PUser();
	

	//setter

	void set_connected(bool connect);
	void set_name(std::string name);
	void set_event(HANDLE event);
	void set_socket(SOCKET socket);
	void set_id(WORD id);
	void SetUser(bool connect, std::string name, HANDLE event, SOCKET socket, WORD id);
	void set_character_id(WORD id);
	void set_recv_bytes(int num);
	void set_is_dead(bool dead);

	//getter

	bool get_connected();
	std::string get_name();
	HANDLE get_event();
	SOCKET get_socket();
	HANDLE* get_event_by_ptr();
	SOCKADDR_IN* get_client_addr_by_ptr();
	SOCKET* get_socket_by_ptr();
	SOCKADDR_IN get_client_addr();
	WORD get_character_id();
	bool get_is_dead();
	int get_id();
	int& get_recv_bytes();
	char* get_recv_buffer_by_ptr();

	//etc
	void AddRecvBytes(int num);

public: // ���ǻ� �Ϻη� public�� ����. ���߿� �����丵
	PACKET* packet;

};


class PUserManager : public PSingleton<PUserManager>
{
private:
	friend class PSingleton<PUserManager>;
private:
	PUserManager();
public:
	std::vector<PUser*> user_list_;
	PUser oneself_user_; //�ڱ��ڽ�(����)
	std::mutex mutex_;
public:
	bool Init();
	bool Frame();
	bool Render();
	bool Release();


	void AddUserFirstTime(PUser* user);
	void AddUserSimpleType(WORD id, WORD cid);// client only
	void DeleteUser(PUser* user);
	PUser* FindUserById(WORD id);
	PUser* FindUserByCid(WORD cid);
	PUser* FindUserBySocket(SOCKET sock);


public:
	~PUserManager();
};

