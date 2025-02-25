#include "PPacketManager.h"
#include "PUserManager.h"


bool PPacketManager::is_both_pool_empty_ = false;
std::mutex PPacketManager::mutex_;
std::mutex PPacketManager::process_mutex_;
std::mutex PPacketManager::recv_mutex_;
std::mutex PPacketManager::send_packet_mutex_;
std::mutex PPacketManager::recv_packet_mutex_;
std::condition_variable PPacketManager::recv_event_;
std::condition_variable PPacketManager::process_event_;


bool PPacketManager::SendPacketFromPacketPool(SOCKET socket, PACKET packet)
{
	int send_bytes = 0;
	do
	{
		int once_send = send(socket, (char*)& packet, packet.ph.len - send_bytes, 0);
		if (once_send == SOCKET_ERROR)
		{
			if (WSAGetLastError() == WSAEWOULDBLOCK)
			{
			}
			else
			{
				E_MSG("send");
				return false;
			}
		}
		else
		{
			if (once_send >= 0)
			{
				send_bytes += once_send;
			}
		}

	} while (packet.ph.len > send_bytes);


	return true;
}

unsigned __stdcall RecvPacketThread(LPVOID param) //패킷을 받는 recv를 수행하는 스레드
{

	PACKET** packet = nullptr;


	ThreadParamSet* param_set = (ThreadParamSet*)param; //소켓, recv_pool, send_pool이 들어가있는 파라미터 세트

	int& notify_request_count_from_paramater = *(param_set->recv_notify_request_count); //헷갈리니까 아예 참조로 받아온다.
	SOCKET& socket_ref_from_parameter = *(param_set->socket);

	while (g_window_terminated == false)
	{		

		std::unique_lock<std::mutex> recv_lock(PPacketManager::recv_mutex_);  //Lock(mutex)을 얻는다. recv_notify_req...가 변형되는걸 막기 위함
		PPacketManager::recv_event_.wait(recv_lock, [&notify_request_count_from_paramater]()
		{return notify_request_count_from_paramater > 0; });
		notify_request_count_from_paramater -= 1;

		if (g_window_terminated == true)
			break;


		


		//lock은 return()의 조건을 보호하기 위함 
		//wait앞에서 대기, notify가 콜되면 인수의 조건을 체크하여, false면 recv_lock을 풀고(쓰레드가 대기할 수 있도록)
		//Thread를 block한다. 만약 true면 recv_lock을 걸고(쓰레드 하나만 접근) 블록을 해제한다(코드 진행)
		//의문 : 1회차는 돌았다. 2회차에서 wait을 만나면 notify가 또 터질때까지 기다릴 것인가, 조건에 맞기만 하면 진행할 것인가?
		 
		SOCKET socket_ref_from_parameter = *(param_set->socket); //소켓 자체를 바꿔버리므로 참조는 단지 원래 소켓자체를 가리킬뿐.
		
		PUser* current_user;

		if (g_operate_mode == OperateMode::SERVER)
		{
			current_user = PUserManager::GetInstance().FindUserBySocket(socket_ref_from_parameter);
			packet = &current_user->packet;
		}
		else //client
		{
			current_user = &PUserManager::GetInstance().oneself_user_;
			packet = &current_user->packet;
		}
		
		int& current_recv_bytes = current_user->get_recv_bytes();
		char* recv_buf_ptr = current_user->get_recv_buffer_by_ptr();


		if (current_recv_bytes < PACKET_HEADER_SIZE)
		{
			
			int once_recv = recv(socket_ref_from_parameter, &recv_buf_ptr[current_recv_bytes],
				PACKET_HEADER_SIZE - current_user->get_recv_bytes(), 0);
		
			//std::wstring once_r = L"\nonce_rv_first: " + to_wstring(once_recv);
			//OutputDebugString(once_r.c_str());

			if (once_recv == SOCKET_ERROR)
			{
				if (WSAGetLastError() == WSAEWOULDBLOCK)
				{
					//OutputDebugString(L"\n WSAEWOULDBLOCK ");
					continue;
				}
				else
				{
					OutputDebugString(L"\n recv error 발생!!");
					int err = WSAGetLastError();
					E_MSG("recv error 발생!!");
					return false;
				}
			}
			else
			{
				current_user->AddRecvBytes(once_recv);

				if (current_recv_bytes == PACKET_HEADER_SIZE)
				{
					//packet = (PACKET*)recv_buf_ptr; //헤더로 일단 패킷을 만들어 둔다.
					*packet = (PACKET*)recv_buf_ptr; //각 유저별 패킷에 패킷헤더정보를 할당한다. (헤더가 연속으로 들어올경우 대책)

					//std::wstring once_r = L" \n [6byte_header_sock : " + to_wstring(socket_ref_from_parameter)
					//	+ L" header_pid : " + to_wstring((*packet)->ph.id) + L" header_plen : " + to_wstring((*packet)->ph.len) +
					//	L" header_type : "  + to_wstring((*packet)->ph.type) + L" ]   ";
					//OutputDebugString(once_r.c_str());
				}
				else
				{
					OutputDebugString(L"\n packet header size 부족, continue");
					notify_request_count_from_paramater += 1;
					continue;
				}


				if ((*packet)->ph.len == current_recv_bytes)
				{
					if ((*packet)->ph.type == PACKET_ANYDIR_SAY_HI)
					{
						OutputDebugString(L"\n === HIPACKET");
						memcpy((*packet)->msg, &socket_ref_from_parameter, sizeof(SOCKET));
						(*packet)->ph.len = PACKET_HEADER_SIZE + sizeof(SOCKET);
					}

					PPacketManager::GetInstance().PushPacket(PushType::RECV, *(*packet));
					PPacketManager::GetInstance().NotifyProcessEvent();
					current_user->set_recv_bytes(0);

					
				
					//return true; //리턴하면 안 되는게, 또 recv해서 0이 될 때 까지..
				}

			}
		}
		else
		{

			//packet = (UPACKET*)recv_buffer;
			int once_recv = recv(socket_ref_from_parameter, &recv_buf_ptr[current_recv_bytes],
				(*packet)->ph.len - current_recv_bytes, 0);

			//std::wstring once_r = L"\nrecv_bytes : " + to_wstring(current_recv_bytes) + L" once_recv : " + to_wstring(once_recv) + L" p.len : " 
			//	+ to_wstring((*packet)->ph.len)
			//	+ L" p.id : " + to_wstring((*packet)->ph.id) + L" p.type " + to_wstring((*packet)->ph.type)
			//	+ L" p.socket : " + to_wstring(socket_ref_from_parameter);
			//OutputDebugString(once_r.c_str());
			
			if (once_recv == SOCKET_ERROR)
			{
				if (WSAGetLastError() == WSAEWOULDBLOCK)
				{
					//OutputDebugString(L"\n WSAEWOULDBLOCK");
					continue;
				}
				else
				{
					OutputDebugString(L"\n recv error 발생!!\n");
					int err = WSAGetLastError();
					E_MSG("recv");
					return false;
				}
			}

			current_user->AddRecvBytes(once_recv);

			if ((*packet)->ph.len == current_recv_bytes)
			{
				PPacketManager::GetInstance().PushPacket(PushType::RECV, *(*packet));
				current_user->set_recv_bytes(0);
				PPacketManager::GetInstance().NotifyProcessEvent();
				//return true;// 리턴하면 안 되는게, 또 recv해서 0이 될 때 까지 받아야 함
			}
			else
			{
				OutputDebugString(L"\n 패킷 본체 길이 부족, continue");
				notify_request_count_from_paramater += 1;
				continue;
			}

		}
	}


	//return true;
}



unsigned __stdcall ProcessThread(LPVOID param)
{

	ThreadParamSet* param_set = (ThreadParamSet*)param;

	std::list<PACKET>& recv_packet_pool = *param_set->recv_packet_pool;
	std::list<PACKET>& send_packet_pool = *param_set->send_packet_pool;

	/*if (recv_packet_pool->size() == 0 && send_packet_pool->size() == 0)
		return false;*/

	SOCKET& socket_ref_from_parameter = *(param_set->socket);

	while (g_window_terminated == false)
	{
		//PPacketManager::is_both_pool_empty_ = (recv_packet_pool.size() == 0 && send_packet_pool.size() == 0) ? true : false; //풀 방법이 없음!
		std::unique_lock<std::mutex> process_lock(PPacketManager::GetInstance().process_mutex_);
		PPacketManager::process_event_.wait(process_lock, [&recv_packet_pool, &send_packet_pool]()
			{return !(((recv_packet_pool.size() == 0) && (send_packet_pool.size() == 0)) ? true : false);  });

		
		if (g_window_terminated == true)
			break;

		SOCKET& socket_ref_from_parameter = *(param_set->socket);

			for (auto iter = recv_packet_pool.begin(); iter != recv_packet_pool.end();)
			{
				
				PACKET& packet = *iter;

				if (g_operate_mode == OperateMode::CLIENT) // 클라이언트
				{
	

					switch (packet.ph.type)
					{
						default:
						{
							PInstructionManager::GetInstance().AddInstruction(packet);
							break;
						}
					}
				}
				else //서버
				{
					

					switch (packet.ph.type)
					{
						default:
						{
							PInstructionManager::GetInstance().AddInstruction(packet);
							break;
						}
					}
				}

				{
					std::lock_guard<std::mutex> lk(PPacketManager::recv_packet_mutex_);
					iter = recv_packet_pool.erase(iter);
				}

			}
			

			if (g_operate_mode == OperateMode::CLIENT)
			{
				std::lock_guard<std::mutex> lk(PPacketManager::send_packet_mutex_);

				for (auto iter = send_packet_pool.begin(); iter != send_packet_pool.end();)
				{
					PACKET& packet = *iter;
					PPacketManager::GetInstance().SendPacketFromPacketPool(socket_ref_from_parameter, packet);
					iter = send_packet_pool.erase(iter);
				}
			
			}
			else if(g_operate_mode == OperateMode::SERVER)
			{
				

				for(auto iter = send_packet_pool.begin(); iter != send_packet_pool.end() ;)
				{
					PACKET& packet = *iter;
					PUser* user = PUserManager::GetInstance().FindUserById(packet.ph.id);

					PKT_MSG_REGULAR_POS_REPORT rep;
					memcpy(&rep, packet.msg, sizeof(PKT_MSG_REGULAR_POS_REPORT));

				/*	std::wstring wstr = L"\n [packet.id = " + std::to_wstring(packet.ph.id) + L"packet.len = " + std::to_wstring(packet.ph.len) +
						L" packet.type = " + std::to_wstring(packet.ph.type) + L" send_pool_size = " + std::to_wstring(send_packet_pool.size()) +
						L" report_cid = " + std::to_wstring(rep.cid) + L" report_posx = " + std::to_wstring(rep.posx) + L"]";
					OutputDebugString(wstr.c_str());*/

					if(user != nullptr)
						PPacketManager::GetInstance().SendPacketFromPacketPool(user->get_socket() , packet);
					else
					{
						/*SOCKET sock;
						memcpy(&sock, packet.msg, sizeof(SOCKET));
						PPacketManager::GetInstance().SendPacketFromPacketPool(sock, packet);*/ //무의미한 코드로 의심되지만 일단 남겨둠
						//애초에 ID가 없는 경우가 유저 퇴장시 말고는 있을수가 없음. 연결 accept하자마자 ID부터 부여하고 시작하기 때문.
					}
					{
						std::lock_guard<std::mutex> lk(PPacketManager::send_packet_mutex_);
						iter = send_packet_pool.erase(iter);
					}
					
					
				}
			}
			
	}


	return true;
}


PPacketManager::PPacketManager()
{

}



void PPacketManager::PushPacket(PushType type, PACKET packet)
{

	
	
		if (type == PushType::SEND)
		{
			std::lock_guard<std::mutex > lock(send_packet_mutex_);
			send_packet_pool_.push_back(packet);
			//std::wstring wstr = L"\nsnd_pool ph.len : " + std::to_wstring(packet.ph.len) + L" |  ph.id : " + std::to_wstring(packet.ph.id) +
			//	L"  | ph.type : " + std::to_wstring(packet.ph.type) + L" | snd_pool_size : " + std::to_wstring(send_packet_pool_.size());
			//OutputDebugString(wstr.c_str());
		}
		else
		{
			std::lock_guard<std::mutex > lock(recv_packet_mutex_);
			recv_packet_pool_.push_back(packet);
			//std::wstring wstr = L"\nrv_pool ph.len : " + std::to_wstring(packet.ph.len) + L" |  ph.id : " + std::to_wstring(packet.ph.id) +
			//	L" | ph.type : " + std::to_wstring(packet.ph.type) + L" | rv_pool_size : " + std::to_wstring(recv_packet_pool_.size());
			//OutputDebugString(wstr.c_str());
		}
	
	PPacketManager::GetInstance().NotifyProcessEvent();

}


void PPacketManager::PushPacket(PUser* user, int protocol, char* data, int data_size, PushType type, bool ischar)
{
	
	
		PACKET packet;
		ZeroMemory(&packet, sizeof(PACKET));
		packet.ph.type = protocol;
		packet.ph.id = user->get_id();
		packet.ph.len = data_size + PACKET_HEADER_SIZE;

		if (data != nullptr)
		{
			if(ischar) //char 배열을 char*로 가져와서 주소그자체일경우
				memcpy(packet.msg, data, data_size);
			else //특정 값을 char*로 가져와서 값을 char*화한 경우
				memcpy(packet.msg, &data, data_size);
		}
			

		if (type == PushType::SEND)
		{
			std::lock_guard<std::mutex > lock(send_packet_mutex_);
			send_packet_pool_.push_back(packet);
			//std::wstring wstr = L"\nsnd_pool ph.len : " + std::to_wstring(packet.ph.len) + L" |  ph.id : " + std::to_wstring(packet.ph.id) +
			//	L"  | ph.type : " + std::to_wstring(packet.ph.type) + L" | rv_pool_size : " + std::to_wstring(send_packet_pool_.size());
			//OutputDebugString(wstr.c_str());
		}
		else
		{
			std::lock_guard<std::mutex > lock(recv_packet_mutex_);
			recv_packet_pool_.push_back(packet);

			//std::wstring wstr = L"\nrv_pool ph.len : " + std::to_wstring(packet.ph.len) + L" |  ph.id : " + std::to_wstring(packet.ph.id) +
			//	L" |  ph.type : " + std::to_wstring(packet.ph.type) + L" | rv_pool_size : " + std::to_wstring(recv_packet_pool_.size());
			//OutputDebugString(wstr.c_str());
		}
		PPacketManager::GetInstance().NotifyProcessEvent();
	


}

bool PPacketManager::NotifyReceiveEvent()
{

	recv_notify_request_count_ += 1;  //여유분을 줘야 하나
	PPacketManager::recv_event_.notify_all();

	return true;
}

bool PPacketManager::NotifyProcessEvent()
{

	PPacketManager::is_both_pool_empty_ = false;
	PPacketManager::process_event_.notify_all();
	return true;
}



bool PPacketManager::Init()
{

	return false;
}

bool PPacketManager::Frame()
{

	return false;
}

bool PPacketManager::Render()
{
	return false;
}

bool PPacketManager::Release()
{
	CloseHandle(process_thread_);
	CloseHandle(receive_thread_);
	return false;
}

void PPacketManager::ThreadInit(SOCKET* socket)
{
	OutputDebugString(L"\n Thread Initialize... ");
	std::lock_guard<std::mutex> lk(mutex_);
	{
		recv_notify_request_count_ = 0;
		send_notify_request_count_ = 0;
		param_set_.recv_packet_pool = &recv_packet_pool_;
		param_set_.send_packet_pool = &send_packet_pool_;
		param_set_.socket = socket;
		param_set_.recv_notify_request_count = &recv_notify_request_count_;
		param_set_.send_notify_request_count = &send_notify_request_count_;
		receive_thread_ = (HANDLE)_beginthreadex(NULL, 0, RecvPacketThread, &param_set_, 0, NULL);
		process_thread_ = (HANDLE)_beginthreadex(NULL, 0, ProcessThread, &param_set_, 0, NULL);
	}
}

void PPacketManager::ChangeSocketToParam(SOCKET* socket)
{
	param_set_.socket = socket;
}




PPacketManager::~PPacketManager()
{
}
