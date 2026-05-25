#ifndef FS_H
#define FS_H

#include "../include/types.h"

#define FS_MAX_NODES    64
#define FS_MAX_NAME     32
#define FS_MAX_DATA     4096
#define FS_MAX_PATH     256

typedef enum { FS_FILE, FS_DIR } fs_type_t;

typedef struct {
    uint32_t  id;
    uint32_t  parent_id;
    fs_type_t type;
    char      name[FS_MAX_NAME];
    uint32_t  size;
    uint8_t   data[FS_MAX_DATA];
    bool      used;
} fs_node_t;

void  fs_init(void);

/* Returns 0 on success, -1 on error */
int   fs_create(const char *path, fs_type_t type);
int   fs_mkdir(const char *path);
int   fs_delete(const char *path);
int   fs_write(const char *path, const uint8_t *data, uint32_t len);
int   fs_read(const char *path, uint8_t *buf, uint32_t maxlen);

/* Lists entries in dir; returns count, fills names[][FS_MAX_NAME] */
int   fs_list(const char *path, char names[][FS_MAX_NAME],
              fs_type_t types[], int max);

bool  fs_exists(const char *path);
int   fs_size(const char *path);   /* -1 if not found */
void  fs_chdir(const char *path);
const char *fs_getcwd(void);

#endif
