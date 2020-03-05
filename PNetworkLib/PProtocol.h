#pragma once
#define PACKET_HEADER_SIZE 6
#define PACKET_MAX_DATA_SIZE 2048
#include <cassert>

#define ZAKUM_ID	20000
#define SERVER_ID   44444;

extern bool g_custom_tick;



template<typename T>
using PacketMessage = T*;

#pragma pack( push, 1)

enum class Direction
{
	LEFT,
	RIGHT,
};

typedef struct ph
{
	WORD len;   // ������ ����+�������   WORD : unisgned short
	WORD type;  // ��Ŷ Ÿ�� (��������)
	WORD id;  //Ŭ��� �����̵�, ������ �������̵�, ������ ��ü ��ε�ĳ��Ʈ�� SERVER_ID, Ư�� ���� ������ ��ε�ĳ��Ʈ�� Ư�� ���� ID

}PACKET_HEADER;
typedef struct pkt{
	PACKET_HEADER ph;
	char          msg[PACKET_MAX_DATA_SIZE];
}PACKET;

#define PACKET_ANYDIR_SAY_HI				600		//���� �� Hello
#define	PACKET_CHAR_MSG					601		//�޽��� ����(ä��)
#define PACKET_CS_LOGIN_SEND_USERNAME	602		//�α��� �� �����̸��� �Է¹޾� ����
#define PACKET_SC_LOGIN_ACCEPT			603		//�α��� ����
#define PACKET_SC_ID_PROVIDE			604		//���� ���� �� ID�� �ο�
#define PACKET_SC_SPAWN_CHARACTER				632		//������ Ŭ�󿡰� �����÷��̾� ���� ����, ID�ο�, ������ �ο� 
#define PACKET_CS_SPAWN_COMPLETE				633
#define PACKET_BROADCAST_USERX_SPAWN			634		//������ Ŭ��鿡�� USERX�� ���������� �˸�
#define PACKET_BROADCAST_USERX_MOVEAXIS_AtoB	700		//�����鿡�� ����X�� �̵����¸� ����϶�� ����.user pos���� + ���� + �ӵ�
#define PACKET_BROADCAST_USERX_ATTACK_SUCCESS	701		//����X�� ���� ������ �������� �־����� ��ε�ĳ��Ʈ
#define PACKET_CS_USER_ATTACK					702		//Ŭ���� ĳ���Ͱ� ������ �����Ͽ����� �˸�(��ų, ���� ���� ����)
#define	PACKET_CS_MONSTER_HIT					703		//Ŭ�� �������� ���Ͱ� �ǰݴ��� �������� �Ծ����� �˸�.
#define	PACKET_SC_ATTACK_ACK					704		//
#define	PACKET_BROADCAST_USERX_JUMP				705		//����X�� ������ �����鿡�� �˸�
#define	PACKET_CS_REPORT_MYPOSITION				706		//��ġ������ ��ġ����
#define PACKET_CS_REPORT_MOVE_STATUS_MYPOSITION 707		//Ŭ�� �������� �̵����̶� �˸�. ���� pos + �̵��ӵ� + �̵�����(���巹Ŀ�� �߷п�)
#define PACKET_BROADCAST_BE_ATTACKED			708		//������ Ŭ�󿡰� ���ݹ޾Ҵ� �˸�  ������ + ����
#define PACKET_BROADCAST_SPAWN_CHARACTER		709		//
#define PAKCET_BROADCAST_USERX_EXIT				710		//USERX�� �����ٰ� ��ε�ĳ��Ʈ
#define PACKET_BROADCAST_USERX_HIT				730		//USERX ������ �Ծ����� �˸�
#define PACKET_CS_IM_HIT						731		//�ڱⰡ �������� �Ծ����� �������� ����
#define PACKET_SC_SPAWN_BOSS					780		//���� ��ȯ ���
#define PACKET_SC_BOSS_REAMIN_HP				781		//���� ���� HP ����->Ŭ��� ����. �ڽ��� MONSTER_HIT�� ���������� �ݼ۵�(HP ��߳��°� ����)
#define PACKET_BROADCAST_METEOR_ACTIVE			901		//���׿� ��ų ��� ����
#define PACKET_BROADCAST_ALLATTACK_ACTIVE		902		//��ü���ݱ� ��� ���� 
#define PACKET_BROADCAST_GAMEOVER				903		//���ӿ���
#define PACKET_BROADCAST_GAMECLEAR				904		//����Ŭ����

typedef struct{
	char username[30];
	int user_id;
}User;

typedef struct EXIT_MSG_{
	WORD id;
}EXIT_MSG;

typedef struct {
	User user;
	PACKET packet;
}USER_PACKET;

typedef struct {
	char text[256];
}TextMessage;

typedef struct PKT_MSG_SPAWN_ {
	WORD id;
	float posx;
	float posy;
}PKT_MSG_SPAWN;

typedef struct PKT_MSG_REGULAR_POS_REPORT_
{
	WORD cid;
	float posx; float posy;
	FSM_State current_state;
	Direction dir;

}PKT_MSG_REGULAR_POS_REPORT;


typedef struct PKT_MSG_MONSTER_HIT_
{
	WORD monster_id;
	WORD player_cid;
	WORD player_user_id;
	int damage;
}PKT_MSG_MONSTER_HIT;

typedef struct INFO_CID_
{
	WORD cid;
}INFO_CID;

typedef struct MeteorRandNumberForPacket_
{
	int skillnum;
	int initpos[8];
	int downspeed[8];
}MeteorRandNumberForPacket;



#pragma pack(pop)
