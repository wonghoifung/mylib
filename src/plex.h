#ifndef PLEX_MEMORY_H
#define PLEX_MEMORY_H

/**
 * @file plex.h
 * @brief Ԥ�����ڴ����
 */

/*
#ifdef __KERNEL__
    #include <linux/types.h>
    #include <linux/stddef.h>
    #include <linux/kernel.h>
    #ifndef assert
        #define assert(s) BUG_ON(!(s))
    #endif
#else*/
    #include <stddef.h>
    #include <stdlib.h>
    #include <assert.h>
	#include <string.h>
//#endif

#ifdef __GNUC__
    #define PLEX_INLINE inline
    #define PLEX_STATIC static
#elif _MSC_VER
    #define PLEX_INLINE __inline
    #define PLEX_STATIC
#else
    #define PLEX_INLINE
    #define PLEX_STATIC static
#endif

#ifdef __cplusplus
extern "C" {
#endif

///������ڴ�� (�����С�ڴ�Ƭ)
struct mem_block {
    struct mem_block* next;         //��һ���ָ��
#ifdef PLEX_ENABLE_RESET
    unsigned int      nmemb;        //Ԥ�����Ա����������¼ÿһ���ڴ���Ƭ����������������(plex_reset)
#endif
    char              data[0];      //���ݲ��֣����ڶ�С�ڴ�Ƭ
};

///����С�ڴ�Ƭ
union mem_item {
    union mem_item*   next;               //��һ���ڴ�Ƭ��ָ��
    char              data[sizeof(char*)];//�ڴ�Ƭ�����ݲ���
};

///�����ڴ�plex�Ľṹ
struct plex {
    struct mem_block* head;         //��¼��ϵͳ������ڴ��
    union mem_item*   free;         //��¼�ɷ�����ڴ��
    size_t            item_size;    //Ԥ�����Ա�Ĵ�С
#ifdef PLEX_DEBUG
    size_t            count_alloc;  //��¼����Ĵ���
    size_t            count_free;   //��¼�ͷŵĴ���
#endif
};

typedef struct plex plex_t;

/**
 * plex��ʼ�� ����û�д�ϵͳ�����ڴ棩
 * @param me plex����ָ�룬����Ϊ��
 * @param item_size Ԥ�����Ա�Ĵ�С
 * @return void
 */
PLEX_STATIC PLEX_INLINE void plex_init(struct plex* me, size_t item_size) 
{
    assert(me && item_size);
    me->head = 0;
    me->free = 0;
    me->item_size = item_size<sizeof(void*) ? sizeof(void*) : item_size;
#ifdef PLEX_DEBUG
    me->count_alloc = 0;
    me->count_free = 0;
#endif
}

/**
 * ����plex����ϵͳ�����ڴ�
 *  ����Ĵ�СΪsizeof(struct mem_block)+nmemb*size
 * @param me plex����ָ�룬����Ϊ��
 * @param nmemb Ԥ�����Ա������
 * @param malloc_f ����ϵͳ�ڴ�ĺ���ָ�룬����Ϊ��
 * @param ctx ����ϵͳ�ڴ��������
 * @return 0:ok; -1:error,see errno
 */
PLEX_STATIC PLEX_INLINE int plex_expand(struct plex* me, unsigned int nmemb, void*(*malloc_f)(void*, size_t), void* ctx) 
{
    struct mem_block* block = 0;
    union mem_item* item = 0;
    unsigned int i = 0;

    assert(me && malloc_f);

    block = (struct mem_block*)malloc_f(ctx, sizeof(struct mem_block) + nmemb * me->item_size);
    if (!block)
        return -1;
#ifdef PLEX_ENABLE_RESET
    //remember nmemb
    block->nmemb = nmemb; 
#endif

    //join to head
    block->next = me->head;
    me->head = block;

    //join to free_list
    item = (union mem_item*)block->data;
    for (; i<nmemb; i++) {
        item->next = me->free;
        me->free = item; 
        item = (union mem_item*)(item->data + me->item_size);
    }
    return 0;
}

/**
 * ���plex���黹�ڴ��ϵͳ
 * @param me plex����ָ�룬����Ϊ��
 * @param free_f �ͷ�ϵͳ�ڴ�ĺ���ָ�룬����Ϊ��
 * @param ctx ����ϵͳ�ڴ��������
 * @return void
 */
PLEX_STATIC PLEX_INLINE void plex_clear(struct plex* me, void(*free_f)(void*ctx, void*ptr), void* ctx) 
{
    assert(me && free_f);
    while (me->head) {
        struct mem_block* bak = me->head;
        me->head = bak->next;
        free_f(ctx, bak);
    }
    me->head = 0;
    me->free = 0;
#ifdef PLEX_DEBUG
    me->count_alloc = 0;
    me->count_free = 0;
#endif
}

/**
 * ����һ����Ա���ڴ�
 * @param me plex����ָ�룬����Ϊ��
 * @return �ڴ�ָ��, 0Ϊʧ��
 */
PLEX_STATIC PLEX_INLINE void* plex_alloc(struct plex* me) 
{
    void* ret = 0;

    assert(me);
    if (!me->free)
        return 0;

    ret = me->free;
    me->free = me->free->next;
#ifdef PLEX_DEBUG
    me->count_alloc++;
#endif
    return ret;
}

/**
 * �黹һ����Ա���ڴ�
 * @param me plex����ָ�룬����Ϊ��
 * @param ptr ��Ա��ָ��
 * @return �ڴ�ָ��, 0Ϊʧ��
 */
PLEX_STATIC PLEX_INLINE void plex_free(struct plex* me, void* ptr) 
{
    assert(me && ptr);
    ((union mem_item*)ptr)->next = me->free;
    me->free = (union mem_item*)ptr;
#ifdef PLEX_DEBUG
    me->count_free++;
#endif
}

#ifdef PLEX_ENABLE_RESET

/**
 * ����plex�����е��ڴ�����·���
 * @param me plex����ָ�룬����Ϊ��
 * @return void
 */
PLEX_STATIC PLEX_INLINE void plex_reset(struct plex* me)
{
    unsigned int i = 0;
    struct mem_block* block = me->head;

    me->head = 0;
    me->free = 0;
#ifdef PLEX_DEBUG
    me->count_alloc = 0;
    me->count_free = 0;
#endif

    //join to free_list
    while (block) {
        union mem_item* item = (union mem_item*)block->data;
        for (i=0; i<block->nmemb; i++) {
            item->next = me->free; 
            me->free = item; 
            item = (union mem_item*)(item->data + me->item_size);
        }
        block = block->next;
    }
}

/**
 * ��ȡplex�ڴ������������ڴ��С
 * @param me plex����ָ�룬����Ϊ��
 * @return plex�ڴ������������ڴ��С
 */
PLEX_STATIC PLEX_INLINE  size_t plex_size(struct plex* me)
{
    size_t size = 0;
    struct mem_block* block = me->head;
    while (block) {
        size += (sizeof(struct mem_block) + block->nmemb * me->item_size);
        block = block->next;
    }
    return size;
}

/**
 * �����ڴ�Ƭ������
 * @param me plex����ָ�룬����Ϊ��
 * @return �����ڴ�Ƭ������
 */
PLEX_STATIC PLEX_INLINE size_t plex_item_count(struct plex* me)
{
    size_t count = 0;
    struct mem_block* block = me->head;
    while (block) {
        count += block->nmemb;
        block = block->next;
    }
    return count;
}
#endif

///Ĭ�ϵ�ϵͳ�ڴ����뺯��
PLEX_STATIC PLEX_INLINE void* sys_malloc(void* ctx, size_t size)
{
    void* buf = malloc(size);
    memset(buf, 0, size); //��ֹϵͳ���١����䡪���õ�ʱ����������
    return buf;
}

///Ĭ�ϵ�ϵͳ�ڴ��ͷź���
PLEX_STATIC PLEX_INLINE void sys_free(void* ctx, void* ptr)
{
    free(ptr);
}

#define ITEM_MALLOC(plex, size) \
(plex)? plex_alloc(plex) : malloc(size); 

#define ITEM_FREE(plex, ptr) \
(plex)? plex_free(plex, ptr) : free(ptr); 



#ifdef __cplusplus
}
#endif

#endif /* PLEX_MEMORY_H */

