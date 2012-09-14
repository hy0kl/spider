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

int parse_args(int argc, char *argv[])
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

int init_config()
{
    int ret = 0;

    char config_file[FILENAME_MAX_LEN];

    char  s[FILENAME_MAX_LEN];
    int   d = 0;

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

    if (0 != get_field(L, "module_count",
            &gconfig.module_config.count, sizeof(gconfig.module_config.count)))
    {
        fprintf(stderr, "get gconfig.module_config.module_count fail.\n");
        ret = -1;
        goto FINISH;
    }

    /** get module_array */
    // TODO

FINISH:
    lua_close(L);

    return ret;
}

void print_config()
{
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
    printf("---end print gconfig---\n");

    return;
}

int main(int argc, char *argv[])
{
    /**
    argument_t *thread_args = NULL;
    pthread_t *pt_download_core = NULL;
    pthread_t *pt_extract_core  = NULL;
    */

    // signal_setup();

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

    pthread_mutex_init(&g_vars.task_queue_mutex, NULL);
FINISH:
    return 0;
}

