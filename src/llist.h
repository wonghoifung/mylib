#ifndef _LLIST_H_
#define _LLIST_H_


#ifdef __cplusplus
extern "C" {
#endif

typedef struct{
	int i;
	int j;
	void* k;
}ELE;

struct lnode{
	struct lnode *prev, *next;
	ELE* element;	
};

//������������Ԫ�ظ�����ͷ���
typedef struct{
	struct lnode head;
	int num;
}LLIST;

//����ָ�룬Ϊ�˲���Ԫ�أ�key��Ҫ����Ԫ�صĹؼ���
typedef int(*list_find_p)(ELE* pELE, void *key);
typedef void(*list_travel_p)(ELE* pELE, void *key);

LLIST* lhandle;
list_travel_p travel_p;
list_find_p find_p;


LLIST* llist_creat(void);

//��壬������������
int llist_append(LLIST* handle, ELE* pnode);

//ǰ��,����ͷ���ĺ���ĵ�һ�����
int llist_preappend(LLIST* handle, ELE* pnode);

// ����Ľڵ���
int llist_size(LLIST* handle);

// ��������
void llist_destory(LLIST* handle);

// ɾ��һ��key���ݵĽڵ�
int llist_delete(LLIST* handle, list_find_p find, void* key);

// ��ѯ����
void llist_travel(LLIST* handle, list_travel_p trav, void* arg);




#ifdef __cplusplus
}
#endif

#endif //llist.h
