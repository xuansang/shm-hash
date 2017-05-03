/*********************************************************************************
  *FileName: hash_ip_security.h
  *Author:  sangxuan
  *Version:  1.0
  *Date:  2017-04-26
  *Description:
**********************************************************************************/
#ifndef _NS_SHM_HASH_H_
#define _NS_SHM_HASH_H_
#include <sys/shm.h>

#define INVALID_POS (0xFFFFFFF)
typedef struct List_Node
{   
    unsigned int next_node_pos;
    unsigned int front_node_pos;
    unsigned int element_pos;
    unsigned char use_flag;
}List_Node;


typedef unsigned int (*hash)(void*); 

typedef int (*compare)(void*,void*);

typedef struct ns_shm_hash
{
    List_Node *start_addr;               //共享内存在本进程的起始地址
    void *heap_start_addr;          //共享内存实际数据在本进程的起始地址
    unsigned short element_size;    //实际存储数据大小
    unsigned int max_element_cnt;   //支持的最大元素个数
    unsigned int  empty_head_pos;   //未使用链表头位置
    hash hash_func;                 //计算hash
    compare compare_func;           //比较函数
}ns_shm_hash;


/*********************************************************************************
    *Function: init_SHM_hash
    *Description: 初始化共享内存及响应的数据结构
    *Input: shm_hash  
    *       element_size 元素大小
    *       max_element_cnt 最大元素个数
    *       shm_key 共享内存标识
    *Output:none 
    *Return: 0 成功 其他失败
**********************************************************************************/
int init_SHM_hash(ns_shm_hash *shm_hash,unsigned short element_size,
    unsigned int max_element_cnt,key_t shm_key);

/*********************************************************************************
    *Function: init_SHM_hash_func
    *Description: 初始设置hash计算函数、元素比较函数
    *Input:  hash_func
    *       compare_func
    *Output: None
    *Return: None 
**********************************************************************************/
void init_SHM_hash_func(ns_shm_hash *shm_hash,hash hash_func,compare compare_func);

/*********************************************************************************
    *Function: insert
    *Description: 插入新新纪录
    *Input: shm_hash
    *       element 插入元素
    *Output: None
    *Return: 0 成功 其他失败
**********************************************************************************/
int insert(ns_shm_hash *shm_hash,void*element);

/*********************************************************************************
    *Function: find
    *Description: 查询元素是否在表中
    *Input: shm_hash
    *       element 查询元素内容
    *Output: element 元素全部信息
    *Return: *Return: 0 成功 其他失败
**********************************************************************************/
int find(ns_shm_hash *shm_hash, void*element);

/*********************************************************************************
    *Function: fini_shm_hash
    *Description: 解除对共享内存的引用
    *Input: shm_hash
    *Output: None
    *Return: None
**********************************************************************************/
void fini_shm_hash(ns_shm_hash *shm_hash);

void fresh(ns_shm_hash *shm_hash);
#endif