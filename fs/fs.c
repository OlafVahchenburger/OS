#include "fs.h"
#include "../libc/string.h"

static fs_node_t nodes[FS_MAX_NODES];
static uint32_t  next_id  = 1;
static uint32_t  cwd_id   = 0;
static char      cwd_path[FS_MAX_PATH] = "/";

/* ---- helpers ------------------------------------------------------------ */

static fs_node_t *node_by_id(uint32_t id) {
    for (int i = 0; i < FS_MAX_NODES; i++)
        if (nodes[i].used && nodes[i].id == id) return &nodes[i];
    return NULL;
}

static fs_node_t *node_by_path(const char *path) {
    if (!path || !path[0]) return NULL;

    if (path[0] != '/') return NULL;   /* relative paths not supported */
    if (path[1] == '\0') return &nodes[0];   /* root */

    uint32_t cur = 0;
    const char *p = path + 1;
    while (*p) {
        char comp[FS_MAX_NAME] = {0};
        int  ci = 0;
        while (*p && *p != '/') comp[ci++] = *p++;
        if (*p == '/') p++;
        if (ci == 0)   continue;

        bool found = false;
        for (int i = 0; i < FS_MAX_NODES; i++) {
            if (nodes[i].used && nodes[i].parent_id == cur &&
                strcmp(nodes[i].name, comp) == 0) {
                cur   = nodes[i].id;
                found = true;
                break;
            }
        }
        if (!found) return NULL;
    }
    return node_by_id(cur);
}

static void split_path(const char *path, char *parent, char *name) {
    const char *slash = NULL;
    for (const char *p = path; *p; p++)
        if (*p == '/') slash = p;

    if (!slash || slash == path) {
        strcpy(parent, "/");
        strcpy(name, slash ? slash + 1 : path);
    } else {
        size_t len = (size_t)(slash - path);
        strncpy(parent, path, len);
        parent[len] = '\0';
        strcpy(name, slash + 1);
    }
}

static fs_node_t *alloc_node(void) {
    for (int i = 0; i < FS_MAX_NODES; i++)
        if (!nodes[i].used) return &nodes[i];
    return NULL;
}

/* ---- public API --------------------------------------------------------- */

void fs_init(void) {
    memset(nodes, 0, sizeof(nodes));
    /* Root directory */
    nodes[0].id        = 0;
    nodes[0].parent_id = 0;
    nodes[0].type      = FS_DIR;
    nodes[0].name[0]   = '/';
    nodes[0].name[1]   = '\0';
    nodes[0].used      = true;
    next_id = 1;

    /* Default directory tree */
    fs_mkdir("/bin");
    fs_mkdir("/etc");
    fs_mkdir("/home");
    fs_mkdir("/tmp");

    /* Welcome files */
    const char *readme =
        "Welcome to RanOS v0.1!\n"
        "Type 'help' for available commands.\n";
    fs_write("/README", (const uint8_t *)readme, (uint32_t)strlen(readme));

    const char *motd =
        "RanOS – a hobby OS built from scratch.\n"
        "Architecture: x86 (32-bit protected mode)\n";
    fs_write("/etc/motd", (const uint8_t *)motd, (uint32_t)strlen(motd));
}

int fs_create(const char *path, fs_type_t type) {
    if (fs_exists(path)) return -1;

    char parent[FS_MAX_PATH], name[FS_MAX_NAME];
    split_path(path, parent, name);
    if (name[0] == '\0') return -1;

    fs_node_t *par = node_by_path(parent);
    if (!par || par->type != FS_DIR) return -1;

    fs_node_t *n = alloc_node();
    if (!n) return -1;

    memset(n, 0, sizeof(*n));
    n->id        = next_id++;
    n->parent_id = par->id;
    n->type      = type;
    strncpy(n->name, name, FS_MAX_NAME - 1);
    n->used      = true;
    return 0;
}

int fs_mkdir(const char *path) { return fs_create(path, FS_DIR); }

int fs_delete(const char *path) {
    fs_node_t *n = node_by_path(path);
    if (!n || n->id == 0) return -1;
    n->used = false;
    return 0;
}

int fs_write(const char *path, const uint8_t *data, uint32_t len) {
    fs_node_t *n = node_by_path(path);
    if (!n) {
        if (fs_create(path, FS_FILE) != 0) return -1;
        n = node_by_path(path);
    }
    if (!n || n->type != FS_FILE) return -1;
    uint32_t sz = (len > FS_MAX_DATA) ? FS_MAX_DATA : len;
    memcpy(n->data, data, sz);
    n->size = sz;
    return (int)sz;
}

int fs_read(const char *path, uint8_t *buf, uint32_t maxlen) {
    fs_node_t *n = node_by_path(path);
    if (!n || n->type != FS_FILE) return -1;
    uint32_t sz = (maxlen < n->size) ? maxlen : n->size;
    memcpy(buf, n->data, sz);
    return (int)sz;
}

int fs_list(const char *path, char names[][FS_MAX_NAME],
            fs_type_t types[], int max) {
    fs_node_t *dir = node_by_path(path);
    if (!dir || dir->type != FS_DIR) return -1;

    int count = 0;
    for (int i = 0; i < FS_MAX_NODES && count < max; i++) {
        if (!nodes[i].used) continue;
        if (nodes[i].parent_id != dir->id) continue;
        if (nodes[i].id == dir->id)        continue;  /* skip self */
        strncpy(names[count], nodes[i].name, FS_MAX_NAME - 1);
        names[count][FS_MAX_NAME - 1] = '\0';
        if (types) types[count] = nodes[i].type;
        count++;
    }
    return count;
}

bool fs_exists(const char *path) { return node_by_path(path) != NULL; }

int fs_size(const char *path) {
    fs_node_t *n = node_by_path(path);
    return n ? (int)n->size : -1;
}

void fs_chdir(const char *path) {
    fs_node_t *n = node_by_path(path);
    if (n && n->type == FS_DIR) {
        cwd_id = n->id;
        strncpy(cwd_path, path, FS_MAX_PATH - 1);
        cwd_path[FS_MAX_PATH - 1] = '\0';
    }
}

const char *fs_getcwd(void) { return cwd_path; }
