#include "ns_shm_hash.h"

#include <stdio.h>
#include <string.h>
#define USED_FLAG (1)
#define UNUSED_FLAG (0)
#define LIST_NODE(i) (shm_hash->start_addr + i)
#define UNUSED_VAR(a) (a=a)
#define ELEMENT_NODE(i) ((void*)((unsigned long)(shm_hash->heap_start_addr) +  shm_hash->element_size*i))

static void* init_shm_memory(ns_shm_hash *shm_hash,unsigned long memory_size,key_t shm_key)
{
    void *addr = NULL;
    int shm_id=shmget(shm_key,memory_size,0666);
    if(shm_id==-1){
        shm_id=shmget(shm_key,memory_size,0666|IPC_CREAT);//create the shm
        if(shm_id==-1){
            return NULL;
        }
    }
    addr = shmat(shm_id,NULL,0);    //mount the shm
    if( addr == (void*)(-1) )
    {
        return NULL;
    }
    return addr ;
}

static int init_hash_list(ns_shm_hash *shm_hash,unsigned short element_size,unsigned int max_element_cnt)
{
    unsigned int node_size = sizeof(List_Node);
    LIST_NODE(0)->use_flag = UNUSED_FLAG;
    LIST_NODE(0)->next_node_pos = 1 ;
    LIST_NODE(0)->front_node_pos = max_element_cnt-1;

    unsigned int pos;
    int i = 0;
    for(i = 1;i < max_element_cnt-1;i++)
    {
        LIST_NODE(i)->use_flag = UNUSED_FLAG;
        LIST_NODE(i)->next_node_pos = i+1 ;
        LIST_NODE(i)->front_node_pos = i-1;
        LIST_NODE(i)->element_pos = i;
    }

    //末尾索引指向头指针
    LIST_NODE(max_element_cnt-1)->use_flag = UNUSED_FLAG;
    LIST_NODE(max_element_cnt-1)->next_node_pos = 0 ;
    LIST_NODE(max_element_cnt-1)->front_node_pos = max_element_cnt-2;
    LIST_NODE(max_element_cnt-1)->element_pos = max_element_cnt-1;
    return 0;
}


static int use_empty_node(ns_shm_hash *shm_hash,unsigned int direct_pos)
{
    unsigned int cur_pos = INVALID_POS;
    unsigned int empty_pos = INVALID_POS;

    if(direct_pos==INVALID_POS ||direct_pos == shm_hash->empty_head_pos)
    {
        cur_pos = shm_hash->empty_head_pos;
        empty_pos = LIST_NODE(cur_pos)->next_node_pos;
    }
    else
    {
        cur_pos = direct_pos;
        empty_pos = shm_hash->empty_head_pos;
    }

    //最后一个可用节点 直接用
    if(LIST_NODE(cur_pos)->next_node_pos == cur_pos ||
        LIST_NODE(cur_pos)->front_node_pos==cur_pos)
    {
        shm_hash->empty_head_pos = INVALID_POS;
        return cur_pos;
    }

    //非最后一个节点
    LIST_NODE(LIST_NODE(cur_pos)->front_node_pos)->next_node_pos = LIST_NODE(cur_pos)->next_node_pos;
    LIST_NODE(LIST_NODE(cur_pos)->next_node_pos)->front_node_pos = LIST_NODE(cur_pos)->front_node_pos;

    shm_hash->empty_head_pos = empty_pos;
    return cur_pos;
}



int init_SHM_hash(ns_shm_hash *shm_hash,unsigned short element_size,unsigned int max_element_cnt,
  key_t shm_key)
{
    shm_hash->element_size = element_size;    
    shm_hash->max_element_cnt = max_element_cnt; 

    //共享内存空间大小
    unsigned int max_memory_size = sizeof(List_Node)*max_element_cnt + element_size*max_element_cnt;
    shm_hash->start_addr =  (List_Node*)init_shm_memory(shm_hash,max_memory_size,shm_key);
    if(shm_hash->start_addr==NULL)
    {
        return -1;
    }
    shm_hash->empty_head_pos = 0;
    shm_hash->heap_start_addr = (void*)((unsigned long)(shm_hash->start_addr) + sizeof(List_Node)*max_element_cnt);
    return init_hash_list(shm_hash, element_size, max_element_cnt);

}

void init_SHM_hash_func(ns_shm_hash *shm_hash,hash hash_func,compare compare_func)
{
    shm_hash->hash_func = hash_func;
    shm_hash->compare_func = compare_func;
}

  
int insert(ns_shm_hash *shm_hash,void*element)
{
    unsigned int key = shm_hash->hash_func(element);
    //hash 设计时需要自行考虑 计算的值在可用区域间
    if(key >= shm_hash->max_element_cnt)
    {
        return -1;
    }
    //检查对应list node 是否可用
    
    //full
    if(shm_hash->empty_head_pos==INVALID_POS){
        return -1;
    }

    //未使用，则直接使用
    if(LIST_NODE(key)->use_flag == UNUSED_FLAG)
    {
        use_empty_node(shm_hash,key);
        LIST_NODE(key)->next_node_pos = INVALID_POS;
        LIST_NODE(key)->front_node_pos = INVALID_POS;
        LIST_NODE(key)->use_flag = USED_FLAG;
        unsigned int element_pos = LIST_NODE(key)->element_pos;
        memcpy(ELEMENT_NODE(element_pos),element,shm_hash->element_size);
        return 0;
    }

    //被自身使用，产生碰撞
    if(LIST_NODE(key)->front_node_pos==INVALID_POS)
    {
        //获取一个可用节点
        unsigned int next_pos =  use_empty_node(shm_hash,INVALID_POS);
        unsigned int tmp_pos = key;
        //取得碰撞尾节点
        while((LIST_NODE(tmp_pos)->next_node_pos!=INVALID_POS) )
        {
            tmp_pos = LIST_NODE(tmp_pos)->next_node_pos;
        }

        LIST_NODE(tmp_pos)->next_node_pos = next_pos;

        LIST_NODE(next_pos)->front_node_pos = tmp_pos;
        LIST_NODE(next_pos)->next_node_pos = INVALID_POS;

        LIST_NODE(next_pos)->use_flag = USED_FLAG;

        unsigned int element_pos = LIST_NODE(next_pos)->element_pos;

        memcpy(ELEMENT_NODE(element_pos),element,shm_hash->element_size);
        return 0;
    }

    //被其他key占用
    //1 获取empty 将当前key原有关系复制到empty
    
    unsigned int next_pos =  use_empty_node(shm_hash,INVALID_POS);
    //复制到空区域
    unsigned int element_pos = LIST_NODE(next_pos)->element_pos;
    unsigned int key_element_pos = LIST_NODE(key)->element_pos;
    memcpy(ELEMENT_NODE(element_pos),ELEMENT_NODE(key_element_pos),shm_hash->element_size);
    
    memcpy(LIST_NODE(next_pos),LIST_NODE(key),sizeof(List_Node));

    LIST_NODE(key)->front_node_pos = INVALID_POS;
    LIST_NODE(key)->next_node_pos = INVALID_POS;
    element_pos = LIST_NODE(key)->element_pos;
    memcpy(ELEMENT_NODE(element_pos),element,shm_hash->element_size);
    LIST_NODE(key)->use_flag = USED_FLAG;
    return 0;
}

int find(ns_shm_hash *shm_hash, void*element)
{
    unsigned int key = shm_hash->hash_func(element);
    //hash 设计时需要自行考虑 计算的值在可用区域间
    if(key >= shm_hash->max_element_cnt)
    {
        return -1;
    }

    //节点中不存在
    if(LIST_NODE(key)->use_flag == UNUSED_FLAG)
    {
        return -1;
    }

    unsigned int tmp_pos = key;
    unsigned int element_pos;
    do{
        element_pos = LIST_NODE(tmp_pos)->element_pos;
        if(shm_hash->compare_func(element,ELEMENT_NODE(element_pos))==0)
        {
            return 0;
        }
        if(LIST_NODE(tmp_pos)->next_node_pos==INVALID_POS)
        {
            return -1;
        }
        tmp_pos = LIST_NODE(tmp_pos)->next_node_pos;
        
    }
    while(1);
    return -1;
    
}

void fini_shm_hash(ns_shm_hash *shm_hash){
    UNUSED_VAR(shm_hash);
    return ;
}

void fresh(ns_shm_hash *shm_hash)
{
    init_hash_list(shm_hash,shm_hash->element_size,shm_hash->max_element_cnt);
}
