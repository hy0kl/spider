#include "util.h"

config_t          gconfig;
global_variable_t g_vars;
/*
task_t   gtask_id = 0;
task_queue_t    *task_queue_head = NULL;
task_queue_t    *task_queue_tail = NULL;
pthread_mutex_t  task_queue_mutex;
*/

static void usage(void)
{
    printf(PACKAGE " " VERSION "\n");
    printf("Build-date %s\n", build_date);
    printf("-p <file>     set ABS path, Necessarily\n"
           "-v            show version and help\n"
           "-h            show this help and exit\n");

    return;
}

static int parse_args(int argc, char *argv[])
{
    int c = 0;
    int ret = 0;
    size_t str_len = 0;

#if (_DEBUG)
    logprintf("grgc = %d", argc);
#endif


    while (-1 != (c = getopt(argc, argv,
        "p:"  /* ABS path for work */
        "v"   /* show version */
        "h"  /* show usage */
    )))
    {
        switch (c)
        {
        case 'p':
            str_len = strlen(optarg);
            if (str_len)
            {
                snprintf(gconfig.prefix, FILENAME_MAX_LEN,
                    '/' == optarg[str_len] ? "%s" : "%s/", optarg);
            }
            else
            {
                usage();
                ret = -1;
            }
            break;

        case 'v':
        case 'h':
            usage();
            exit(EXIT_SUCCESS);
        }
    }

    if (! strlen(gconfig.prefix))
    {
        usage();
        exit(EXIT_SUCCESS);
    }

    return ret;
}

static int init_config(void)
{
    int ret = 0;

    char config_file[FILENAME_MAX_LEN];

    char  s[FILENAME_MAX_LEN];
    char  key[8] = {0};
    int   d = 0;
    int   i = 0;
    size_t size = 0;

    lua_State *L = luaL_newstate(); /* opens Lua */
    luaL_openlibs(L);   /** Opens all standard Lua libraries into the given state. */

    snprintf(config_file, FILENAME_MAX_LEN, "%sconf/config.lua", gconfig.prefix);

    if (luaL_loadfile(L, config_file) || lua_pcall(L, 0, 1, 0))
    {
        lua_ext_error(L);
        goto FINISH;
    }

    lua_getglobal(L, "gconfig");
    if (! lua_istable(L, -1))
    {
        fprintf(stderr, "gconfig is NOT table.\n");
        ret = -1;
        goto FINISH;
    }

    if (0 != get_field(L, "log_name", s, sizeof(s)))
    {
        fprintf(stderr, "get gconfig.log_name fail.\n");
        snprintf(s, sizeof(s), "log/%s.log", PACKAGE);
    }
    snprintf(gconfig.log_name, sizeof(gconfig.log_name), "%s%s", gconfig.prefix, s);

    if (0 != get_field(L, "db_name", s, sizeof(s)))
    {
        fprintf(stderr, "get gconfig.db_name fail.\n");
        snprintf(s, sizeof(s), "db/%s", DEFAULT_DB_NAME);
    }
    snprintf(gconfig.db_name, sizeof(gconfig.db_name), "%s%s", gconfig.prefix, s);

    if (0 != get_field(L, "log_level", &gconfig.log_level, sizeof(gconfig.log_level)))
    {
        fprintf(stderr, "get gconfig.log_level fail.\n");
        gconfig.log_level = 4;

    }

    if (0 != get_field(L, "log_size", &gconfig.log_size, sizeof(gconfig.log_size)))
    {
        fprintf(stderr, "get gconfig.log_size fail.\n");
        gconfig.log_size = 102400;
    }

    if (0 != get_field(L, "do_daemonize", &gconfig.do_daemonize, sizeof(gconfig.do_daemonize)))
    {
        fprintf(stderr, "get gconfig.do_daemonize fail.\n");
        gconfig.do_daemonize = 0;
    }

    if (0 != get_field(L, "dir_magic_number", &gconfig.dir_magic_number, sizeof(gconfig.dir_magic_number)))
    {
        fprintf(stderr, "get gconfig.dir_magic_number fail.\n");
        gconfig.dir_magic_number = 11;
    }

    if (0 != get_field(L, "download_thread_number",
            &gconfig.download_thread_number, sizeof(gconfig.download_thread_number)))
    {
        fprintf(stderr, "get gconfig.download_thread_number fail.\n");
        gconfig.download_thread_number = 2;
    }

    if (0 != get_field(L, "extract_thread_number",
            &gconfig.extract_thread_number, sizeof(gconfig.extract_thread_number)))
    {
        fprintf(stderr, "get gconfig.download_thread_number fail.\n");
        gconfig.extract_thread_number = 4;
    }

    if (0 != get_field(L, "url_density", &d, sizeof(d)))
    {
        fprintf(stderr, "get gconfig.url_density fail.\n");
    }
    if (d > 0 && d < 23)
    {
        gconfig.url_density = (float)d;
    }
    else
    {
        gconfig.url_density = 7.3;
    }

    lua_pop(L, 1);
    /** get module config */
    lua_getglobal(L, "g_module_config");
    if (! lua_istable(L, -1))
    {
        fprintf(stderr, "g_module_config is NOT table.\n");
        ret = -1;
        goto FINISH;
    }

    if (0 != get_field(L, "module_count",
            &gconfig.module_config.count, sizeof(gconfig.module_config.count)))
    {
        fprintf(stderr, "get gconfig.module_config.module_count fail.\n");
        ret = -1;
        goto FINISH;
    }
    //logprintf("gconfig.module_config.count = %d", gconfig.module_config.count);
    /* malloc for module config
     * init
     * {
     */
    size = sizeof(char *) * gconfig.module_config.count;
    gconfig.module_config.module_name = (char **)malloc(size);
    if (NULL == gconfig.module_config.module_name)
    {
        fprintf(stderr, "can NOT malloc memory for \
gconfig.module_config.module_name, need size: %ld\n", size);
        ret = -1;
        goto FINISH;
    }
    gconfig.module_config.entry_handle = (char **)malloc(size);
    if (NULL == gconfig.module_config.entry_handle)
    {
        fprintf(stderr, "can NOT malloc memory for \
gconfig.module_config.entry_handle, need size: %ld\n", size);
        ret = -1;
        goto FINISH;
    }
    gconfig.module_config.entry_url = (char **)malloc(size);
    if (NULL == gconfig.module_config.entry_url)
    {
        fprintf(stderr, "can NOT malloc memory for \
gconfig.module_config.entry_url, need size: %ld\n", size);
        ret = -1;
        goto FINISH;
    }

    for (i = 0; i < gconfig.module_config.count; i++)
    {
        size = sizeof(char) * FILENAME_MAX_LEN;
        if (NULL == (gconfig.module_config.module_name[i] = (char *)malloc(size)))
        {
            fprintf(stderr, "can NOT malloc memory for \
gconfig.module_config.module_name[%d], need size: %ld\n", i, size);
            ret = -1;
            goto FINISH;
        }
        gconfig.module_config.module_name[i][0] = '\0';
        if (NULL == (gconfig.module_config.entry_handle[i] = (char *)malloc(size)))
        {
            fprintf(stderr, "can NOT malloc memory for \
gconfig.module_config.entry_handle[%d], need size: %ld\n", i, size);
            ret = -1;
            goto FINISH;
        }
        gconfig.module_config.entry_handle[i][0] = '\0';

        size = sizeof(char) * MAX_URL_LEN;
        if (NULL == (gconfig.module_config.entry_url[i] = (char *)malloc(size)))
        {
            fprintf(stderr, "can NOT malloc memory for \
gconfig.module_config.entry_url[%d], need size: %ld\n", i, size);
            ret = -1;
            goto FINISH;
        }
        gconfig.module_config.entry_url[i][0] = '\0';

        /** module_name */
        lua_pushstring(L, "module_name");
        lua_gettable(L, -2);
        snprintf(key, sizeof(key), "%d", i);
        if (0 != get_field(L, key, s, sizeof(s)))
        {
            fprintf(stderr, "get gconfig.module_config.module_name[%s].\n", key);
            s[0] = '\0';
        }
        snprintf(gconfig.module_config.module_name[i], FILENAME_MAX_LEN, "%s", s);
        logprintf("gconfig.module_config.module_name[%d] = [%s]", i, s);
        lua_pop(L, 1);

        /** entry_url */
        lua_pushstring(L, "entry_url");
        lua_gettable(L, -2);
        snprintf(key, sizeof(key), "%d", i);
        if (0 != get_field(L, key, s, sizeof(s)))
        {
            fprintf(stderr, "get gconfig.module_config.module_name[%s].\n", key);
            s[0] = '\0';
        }
        snprintf(gconfig.module_config.entry_url[i], MAX_URL_LEN, "%s", s);
        logprintf("gconfig.module_config.entry_url[%d] = [%s]", i, s);
        lua_pop(L, 1);

        /** entry_handle */
        lua_pushstring(L, "entry_handle");
        lua_gettable(L, -2);
        snprintf(key, sizeof(key), "%d", i);
        if (0 != get_field(L, key, s, sizeof(s)))
        {
            fprintf(stderr, "get gconfig.module_config.entry_handle[%s].\n", key);
            s[0] = '\0';
        }
        snprintf(gconfig.module_config.entry_handle[i], FILENAME_MAX_LEN, "%s", s);
        logprintf("gconfig.module_config.entry_handle[%d] = [%s]", i, s);
        lua_pop(L, 1);
    }
    /**
     * }
     * */

FINISH:
    lua_close(L);

    return ret;
}

static int init_g_vars(void)
{
    int ret = 0;
    char *zTail;
    char  sql[SQL_BUF_LEN] = {0};

    /** 打开数据库 */
    int r = sqlite3_open(gconfig.db_name, &(g_vars.db));
    if (r)
    {
        fprintf(stderr, "[Error]: %s\n", sqlite3_errmsg(g_vars.db));
        ret = SQLITE_ERROR;
        goto FINISH;
    }
#if (_DEBUG)
    else
    {
        fprintf(stderr, "You have opened a sqlite3 database named %s successfully!\n\
Congratulations! Have fun ! ^-^ \n", gconfig.db_name);
    }
#endif
    /** 初始化数据库 */
    snprintf(sql, sizeof(sql), DEFAULT_DB_STRUCTURE);
    r = sqlite3_exec(g_vars.db, sql, NULL, NULL, &zTail);
    if (r)
    {
        fprintf(stderr, "[Error]: %s\n", zTail);
        ret = -1;
        goto FINISH;
    }
#if (_DEBUG)
    else
    {
        fprintf(stderr, "init db success.\n");
    }
#endif

    g_vars.gtask_id = 0;

    g_vars.d_tq_head = NULL;
    g_vars.d_tq_tail = NULL;
    g_vars.e_tq_head = NULL;
    g_vars.e_tq_tail = NULL;

    g_vars.dt_data = NULL;
    g_vars.et_data = NULL;

FINISH:
    return ret;
}

static void print_config(void)
{
    int i = 0;
    printf("---gconfig---\n");
    printf("prefix:   %s\n", gconfig.prefix);
    printf("log_name: %s\n", gconfig.log_name);
    printf("db_name:  %s\n", gconfig.db_name);
    printf("log_level:%d\n", gconfig.log_level);
    printf("log_size: %d\n", gconfig.log_size);
    printf("do_daemonize:    %d\n", gconfig.do_daemonize);
    printf("dir_magic_number:%d\n", gconfig.dir_magic_number);
    printf("download_thread_number: %d\n", gconfig.download_thread_number);
    printf("extract_thread_number:  %d\n", gconfig.extract_thread_number);
    printf("url_density:            %f\n", gconfig.url_density);

    printf("module_config.cout:     %d\n", gconfig.module_config.count);
    for (i = 0; i < gconfig.module_config.count; i++)
    {
        printf("    >---<\n");
        printf("    module_name[%d]: %s\n", i, gconfig.module_config.module_name[i]);
        printf("    entry_url[%d]:   %s\n", i, gconfig.module_config.entry_url[i]);
        printf("    entry_handle[%d]:%s\n", i, gconfig.module_config.entry_handle[i]);
    }

    printf("---end print gconfig---\n");

    return;
}

static int init_thread(void)
{
    int ret = 0;
    int thread_num = 0;
    int i = 0;
    size_t size = 0;

    thread_num = gconfig.download_thread_number;
    /** init for download thread { */
    size = sizeof(download_thread_data_t) * thread_num;
    g_vars.dt_data = (download_thread_data_t *)malloc(size);
    if (NULL == g_vars.dt_data)
    {
        fprintf(stderr, "Can NOT malloc memory for \
g_vars.dt_data, need size: %ld\n", size);
        ret = 1;
        goto FINISH;
    }
    for (i = 0; i < thread_num; i++)
    {
        size = sizeof(char) * HTTP_HEADER_BUF_LEN;
        g_vars.dt_data[i].http_header = (char *)malloc(size);
        if (NULL == g_vars.dt_data[i].http_header)
        {
            fprintf(stderr, "Can NOT malloc memory for \
g_vars.dt_data[%d].http_header, need size: %ld\n", i, size);
            ret = -1;
            goto FINISH;
        }

        size = sizeof(char) * HTTP_CONTENT_BUF_LEN;
        g_vars.dt_data[i].http_body = (char *)malloc(size);
        if (NULL == g_vars.dt_data[i].http_body)
        {
            fprintf(stderr, "Can NOT malloc memory for \
g.vars.dt_data[%d].http_body, need size: %ld\n", i, size);
            ret = -1;
            goto FINISH;
        }

        size = sizeof(char) * SQL_BUF_LEN;
        g_vars.dt_data[i].d_sql = (char *)malloc(size);
        if (NULL== g_vars.dt_data[i].d_sql)
        {
            fprintf(stderr, "Can NOT malloc memory for \
g.vars.dt_data[%d].d_sql, need size: %ld\n", i, size);
            ret = -1;
            goto FINISH;
        }

    }
    /** } */

    /** init for extract thread { */
    thread_num = gconfig.extract_thread_number;
    size = sizeof(extract_thread_data_t) * thread_num;
    g_vars.et_data = (extract_thread_data_t *)malloc(size);
    if (NULL == g_vars.et_data)
    {
        fprintf(stderr, "Can NOT malloc memory for \
g_vars.et_data, need size: %ld\n", size);
        ret = 1;
        goto FINISH;
    }
    for (i = 0; i < thread_num; i++)
    {
        size = sizeof(char) * HTTP_CONTENT_BUF_LEN;
        g_vars.et_data[i].str_buf = (char *)malloc(size);
        if (NULL== g_vars.et_data[i].str_buf)
        {
            fprintf(stderr, "Can NOT malloc memory for \
g.vars.et_data[%d].str_buf, need size: %ld\n", i, size);
            ret = -1;
            goto FINISH;
        }

        size = sizeof(char) * SQL_BUF_LEN;
        g_vars.et_data[i].e_sql = (char *)malloc(size);
        if (NULL== g_vars.et_data[i].e_sql)
        {
            fprintf(stderr, "Can NOT malloc memory for \
g.vars.et_data[%d].sql, need size: %ld\n", i, size);
            ret = -1;
            goto FINISH;
        }

        /* opens Lua */
        if (NULL == (g_vars.et_data[i].L = luaL_newstate()))
        {
            fprintf(stderr, "[Error]: Can NOT init Lua state for \
g.vars.et_data[%d].L\n", i);
            ret = -1;
            goto FINISH;
        }
    }
    /** } */

FINISH:
    return ret;
}

static task_t get_new_tid(void)
{
    task_t tid = 0;

    while (0 != pthread_mutex_lock(&g_vars.tid_mutex))
    {;}
    g_vars.gtask_id++;
    tid = g_vars.gtask_id;
    pthread_mutex_unlock(&g_vars.tid_mutex);

    return tid;
}

static int init_task(void)
{
    int ret = 0;
    int i   = 0;
    task_t tid  = 0;
    size_t size = 0;
    task_queue_t *task_queue = NULL;

    for (i = 0; i < gconfig.module_config.count; i++)
    {
        tid = get_new_tid();
        logprintf("tid = %ld", tid);
        size= sizeof(task_queue_t);
        if (NULL == (task_queue = (task_queue_t *)malloc(size)))
        {
            ;
        }
    }

FINISH:
    return ret;
}

static void free_memory(void)
{
    if (g_vars.db)
    {
        sqlite3_close(g_vars.db);
    }

    return;
}

int main(int argc, char *argv[])
{
    /**
    argument_t *thread_args = NULL;
    pthread_t *pt_download_core = NULL;
    pthread_t *pt_extract_core  = NULL;
    */

    //signal_setup();

    if (0 != parse_args(argc, argv))
    {
        fprintf(stderr, "parse args fail.\n");
        goto FINISH;
    }

    if (0 != init_config())
    {
        fprintf(stderr, "init_config() fail, please check out event.\n");
        goto FINISH;
    }
#if (_DEBUG)
    print_config();
#endif

    if (0 != init_g_vars())
    {
        fprintf(stderr, "init_g_vars() fail.\n");
        goto FINISH;
    }

    if (0 != init_thread())
    {
        fprintf(stderr, "init_thread() fail.\n");
        goto FINISH;
    }

    pthread_mutex_init(&g_vars.d_tq_mutex, NULL);
    pthread_mutex_init(&g_vars.e_tq_mutex, NULL);
    pthread_mutex_init(&g_vars.tid_mutex,  NULL);
    pthread_mutex_init(&g_vars.db_mutex,   NULL);

    if (0 != init_task())
    {
        fprintf(stderr, "init_task() fail.\n");
        goto FINISH;
    }

FINISH:

    free_memory();
    return 0;
}

