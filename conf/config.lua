gconfig = {
    log_name = 'log/test.log',
    db_name  = 'db/urls-info.db',
    log_level= 4,
    log_size = 102400,
    do_daemonize = 1,
    dir_magic_number = 11,
    download_thread_number = 2,
    extract_thread_number  = 4,
    url_density = 23,
};

g_module_config = {
    module_count = 2,
    module_name  = {
        'ganji',
        '58'
    },
    entry_url = {
        'http://bj.ganji.com/',
        'http://bj.58.com/',
    },
    entry_handle = {
        'common_parse',
        'common_parse',
    },
};
