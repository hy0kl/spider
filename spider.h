#ifndef _SPIDER_H
#define _SPIDER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <pthread.h>
#include <netdb.h>
#include <fcntl.h>
#include <time.h>
#include <assert.h>
#include <signal.h>
#include <errno.h>
#include <err.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/queue.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <arpa/inet.h>
#include <openssl/md5.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include <sqlite3.h>

#define _DEBUG  1

#define DEFAULT_PROXY_PATH          "./conf/proxy.txt"
#define DEFAULT_PROXY_SITEID_PATH   "./conf/proxy_siteid.txt"
#define DEFAULT_MAX_PROXY_IP_NUM    1000
#define DEFAULT_MAX_PROXY_SITE_NUM  20
#define DEFAULT_MAX_USING_COUNT     5
#define DEFAULT_PROXY_IP_NEED_RECHECK_NUM 20
#define DEFAULT_GOOD_IP_FAIL_COUNT  3
#define FILENAME_MAX_LEN    512

#define CHECK_PROXY_FILE_INTERVAL   60
#define DEFAULT_PROXY_MAX_FAIL_TIME 30

#define DEFAULT_WORK_THREAD_NUM     2

#define DEFAULT_CONNECT_TIMEOUT 7   // sec
#define DEFAULT_READ_TIMEOUT    10
#define DEFAULT_WRITE_TIMEOUT   20
#define DEFAULT_PROXY_CONNECT_TIMEOUT   15
#define DEFAULT_PROXY_READ_TIMEOUT      20
#define DEFAULT_PROXY_WRITE_TIMEOUT     40

#define DEFAULT_LOG_PATH    "/log"
#define DEFAULT_LOG_NAME    "spider."
#define DEFAULT_LOG_LEVEL   4
#define DEFAULT_LOG_SIZE    500000

#define MAX_STATUS_LEN  20 // 表示状态的字符串的最大长度
#define PACKAGE         "spider"
#define VERSION         "0.1.0"

#define SPIDER_METHOD_GET   "get"
#define SPIDER_METHOD_POST  "post"

#define HTTP_HOST           "Host: %s\r\n"
#define HTTP_METHOD_GET     "Get %s\r\n"
#define HTTP_METHOD_POST    "POST %s\r\n"
#define ACCEPT_PARAM        "Accept: */*\r\n"
#define CONNECTION_PARAM    "Connection: close\r\n"
#define LANGUAGE_PARAM      "Accept-Language: zh-CN,zh;q=0.8\r\n"
#define LANGUAGE_CHARSET    "Accept-Charset: UTF-8,*;q=0.5"
#define CACHE_PARAM         "Cache-Control: no-cache\r\n"
#define HTTP_VERSION_10     "HTTP/1.0\r\n"
#define HTTP_VERSION_11     "HTTP/1.1\r\n"
#define USER_AGENT          "User-Agent: %s\r\n"
#define COOKIE              "Cookie: %s\r\n"
#define REFERER             "Referer: %s\r\n"

#define MAX_NUMBUF_LEN  20      //存储数字的缓冲区的长度
#define MAX_STR_LEN     2028    //一行的最大程度
#define MAX_URL_LEN     1024
#define MAX_USER_AGENT_LEN  1024
#define MAX_COOKIE_LEN      1024
#define MAX_POSTDATA_LEN    1024 //http中，POST体的最大长度
#define MAX_X_FLASH_VERSION_LEN 256
/** spider request header buffer length */
#define HTTP_HEADER_BUF_LEN 1024 * 4
/** spider download content body buffer length, include http header */
#define HTTP_CONTENT_BUF_LEN    1024 * 500

/** 数据库相关 */
#define SQL_BUF_LEN         1024 * 4
#define DB_NAME_BUF_LEN     64
#define DEFAULT_DB_NAME     "url.db"
//#define DB_STRUCTURE_BUF_LEN    2048
#define DEFAULT_DB_STRUCTURE    "CREATE TABLE IF NOT EXISTS \
url_info(\
    uid INTEGER PRIMARY KEY, \
    url_md5 TEXT, \
    url TEXT, \
    tid INTERER, \
    pid INTERER, \
    content_md5 TEXT, \
    time INTERER, \
    deleted INTERER\
);"

#define SIGNO_END       111
#define TMP_STR_BUF_LEN 1024 * 20
#define DEFAULT_LINK_LENGTH 1024

#define logprintf(format, arg...) fprintf(stderr, "[NOTICE]%s:%d:%s "format"\n", __FILE__, __LINE__, __func__, ##arg)

#if defined(__DATE__) && defined(__TIME__)
static const char build_date[] = __DATE__ " " __TIME__;
#else
static const char build_date[] = "unknown";
#endif

typedef long unsigned int task_t;
typedef long unsigned int indext_t;
/**
 * 任务队列
 **/
typedef struct _task_queue_t
{
    task_t task_id;
    task_t task_pid;    /** parent tid */
    short http_version;      // HTTP的版本，1为HTTP/1.1，0为HTTP/1.0
    short method;    // 抓取的方法，1: POST, 0: GET

    char url[MAX_URL_LEN];  // 目标url
    // char x_flash_version[MAX_X_FLASH_VERSION_LEN];
    char cookie[MAX_COOKIE_LEN];
    char referer[MAX_URL_LEN];
    char post_data[MAX_POSTDATA_LEN];
    char module_name[FILENAME_MAX_LEN];

    /**
     * 抓取内容的类型
     * 0: 纯文本,包括 html, html,js,统一为 txt,默认类型
     * 1: 媒体类型,例如 img, mp3, swf...
     * */
    int media_type;
    /**
     * 抽取数据的类型标志位
     * 0: 抽取 url,默认的抽取行为
     * 1: 抽取特定的文本数据
     * */
    int extract_type;

    struct _task_queue_t *next;
} task_queue_t;

/**
 * 多线程传递参数到各工作线程
 * */
typedef struct _argument_t
{
    int tindex;
} argument_t;

typedef struct _module_config_t
{
    short  count;
    char **module_name; // FILENAME_MAX_LEN
    char **entry_url;   // MAX_URL_LEN
    char **entry_handle;    // FILENAME_MAX_LEN
} module_config_t;

typedef struct _config_t
{
    char prefix[FILENAME_MAX_LEN];  /** 程序工作的绝对路径,必须指定 */
    char log_name[FILENAME_MAX_LEN];    /** $prefix/log/spider.log */
    char db_name[DB_NAME_BUF_LEN];  /** 数据库名字,可配置,但结构不提供配置 */
    int  log_level;
    int  log_size;
    int  do_daemonize;
    /** dir modulo magic number */
    int  dir_magic_number;
    /** threads number config */
    int  download_thread_number;
    int  extract_thread_number;

    /**
     * url 相对于文本的密度
     * 当大于设定的阀值时,认为抽取类型为抽取特定文本数据
     * */
    float url_density;

    /** 模块配置项目 */
    module_config_t module_config;
} config_t;

/** 下载线程空间数据 */
typedef struct _download_thread_data_t
{
    char *http_header;
    char *http_body;
    char *d_sql;
} download_thread_data_t;

/** 抽取线程空间数据 */
typedef struct _extract_thread_data_t
{
    /** 工做者的缓冲区 */
    char *str_buf;
    char *e_sql;
    /**
     * 抽取线程由于各个站点的文档结构千差万别
     * 所以需要使用 Lua 来适配
     * */
    lua_State *L;
} extract_thread_data_t;

typedef struct _global_variable_t
{
    /** 全局共用的数据库链接 */
    sqlite3 *db;
    /** 数据库写操作的互斥锁 */
    pthread_mutex_t  db_mutex;

    /** 任务队列 */
    task_t   gtask_id;
    pthread_mutex_t  tid_mutex;

    /** 下载队列 */
    task_queue_t    *d_tq_head;
    task_queue_t    *d_tq_tail;
    pthread_mutex_t  d_tq_mutex;

    /** 解析队列 */
    task_queue_t    *e_tq_head;
    task_queue_t    *e_tq_tail;
    pthread_mutex_t  e_tq_mutex;

    /** 线程空间 */
    download_thread_data_t *dt_data;
    extract_thread_data_t  *et_data;
} global_variable_t;

extern config_t gconfig;
extern global_variable_t g_vars;

#endif

