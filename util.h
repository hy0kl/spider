#ifndef _UTIL_H_
#define _UTIL_H_
#include "spider.h"

/**
 * get localtime string.
 * */
int get_localtime_str(char *src, const size_t buf_len);

/**
 * set the program as daemon
 * */
int daemonize(int nochdir, int noclose);

/**
 * set up signal
 * */
void signal_setup(void);

indext_t hash(const char *key, const int hash_table_size);

/**
 * Replace all occurrences of the search string with the replacement string
 * The value being searched for, otherwise known as the needle.
 * The replacement value that replaces found search values.
 * */
char * str_replace(char *src, const size_t buf_size, const char *search, const char *replace);

char *strtolower(char *src, const size_t buf_len, const char *encoding);

int url_encode(char *str, int ext);

/**
 * ret: 实际截取的字符个数
 * */
int cut_str(const char *src,
            char *des,
            const size_t des_buf_len,
            const char *charset,
            unsigned int length,
            const char *suffix);

/**
 * 前缀与查询词比对
 * */
int prefix_cmp(const char *prefix, const char *cmp_str);

/**
 * 自定义 lua 出错函数
 * 主要功能在于有 pop 操作
 * */
void lua_ext_error(lua_State *L);

/**
 * 从 lua 脚本里面取得配置值
 * */
int get_field(lua_State *L, const char *key, void *dest, const size_t size);

int c_md5(const unsigned char *data, char *des_buf, const size_t des_size);

#endif

