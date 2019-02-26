#ifndef VFS
#define VFS

#include "drivers/block/blockDeviceDriver.h"

/* This file provides a Generic (FS-type independent) API for accessing
 * files and directories in a FS */

/* all defined Inode mode_t values */
#define FILE_BLCK_SPECIAL 1
#define FILE_CHAR_SPECIAL 2
#define FILE_DIR          3
#define FILE_PIPE_OR_FIFO 4
#define FILE_REGULAR      5
#define FILE_SYMLINK      6
#define FILE_SOCKET       7

/* linux file information type definitions (taken from local system) */
typedef uint64_t ino_t;
typedef uint32_t mode_t;
typedef uint32_t uid_t;
typedef uint32_t gid_t;
typedef uint64_t off_t;

/* Classes defined in VFS API */
typedef struct SuperBlock SuperBlock;
typedef struct Inode Inode;
typedef struct File File;

/* more types */
typedef int (*readdir_cb)(const char *, struct Inode *, void *);

/* typedef struct Inode Inode; */


/* Generic SuperBlock class
 * - Abstraction for root of file system */
struct SuperBlock {
    /*** Inode management operations ***/
    Inode *root_inode;
    Inode *(*read_inode)(struct SuperBlock*, unsigned long inode_num);

    /*** fs maintenance operations ***/
    int (*sync_fs)(struct SuperBlock*);
    void (*put_super) (struct SuperBlock *);

    /*** fs meta-data ***/
    const char *name, *type;
};
typedef struct SuperBlock SuperBlock;

/* Generic Inode class
 * - A node (file or directory) on disk */
struct Inode {
    SuperBlock *parent_super;
    struct Inode *parent_inode;
    
    /*** Inode meta-data ***/
    ino_t st_ino;  //File serial number
    mode_t st_mode; //Mode of file
    uid_t st_uid; //User ID of file
    gid_t st_gid;  //Group ID of file
    off_t st_size; //For regular files, the file size in bytes, for symbolic links, the length in bytes of the pathname contained in the symbolic link

    /*** File Operations ***/
    struct File *(*open)(unsigned long inode);

    /*** Directory Operations ***/
    int (*readdir)(struct Inode *inode, readdir_cb cb, void *p);
    int (*unlink)(struct Inode *inode, const char *name);
    
    /*** Inode management ***/
    void (*free)(struct Inode **inode);
};

/* Generic File class
 * - A file opened for reading, writing, or both
 * - A file has an associated inode
 * - One iode can be opened multiple times concurrently */
struct File {

    /*** File Operations ***/
    int (*close)(struct File **file);
    int (*read)(struct File *file, char *dst, int len);
    int (*write)(struct File *file, char *dst, int len);
    int (*lseek)(struct File *file, off_t offset);
    int (*mmap)(struct File *file, void *addr);
    
};
typedef struct File File;

/* Filesystem driver interface */
/*** extensible interface for supporting multiple filesystems */
typedef struct SuperBlock *(*FS_detect_cb)(struct BlockDev *dev);
extern void FS_register(FS_detect_cb probe);

#endif
