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
        ['0'] = 'ganji',
        ['1'] = '58'
    },
    entry_url = {
        ['0'] = 'http://bj.ganji.com/',
        ['1'] = 'http://bj.58.com/',
    },
    entry_handle = {
        ['0'] = 'common_parse',
        ['1'] = 'common_parse',
    },
};
