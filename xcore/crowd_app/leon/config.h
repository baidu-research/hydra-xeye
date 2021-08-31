#ifndef __DEFINES__
#define __DEFINES__

#define    MAX_PATH_LEN         (512)
#define    MAX_FILE_NAME_LEN    (128)

int parse_config_file(char *path_to_config_file);  
char *get_config_var(char *var_name);  
void print_all_vars();

#endif
