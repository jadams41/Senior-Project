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

    /* from linux kernel 2.2.9 documentation 
     this method is called to read a spcific inode from the mounted filesystem.*/
    Inode *(*read_inode)(struct SuperBlock*, unsigned long inode_num); //assuming inode_num is cluster number

    /*** fs maintenance operations ***/
    int (*sync_fs)(struct SuperBlock*);

    /* from linux kernel vfs documentation (https://www.kernel.org/doc/Documentation/filesystems/vfs.txt)
       called when the VFS wishes to free the superblock (i.e. unmount) */
    void (*put_super) (struct SuperBlock *);

    /*** fs meta-data ***/
    const char *name;
    const char *type;
};
typedef struct SuperBlock SuperBlock;

/* Generic Inode class
 * - A node (file or directory) on disk */
struct Inode {
    SuperBlock *parent_super;
    struct Inode *parent_inode;

    //char name[FAT32_NAME_MAX + 1];
    char name[255 + 1];
    
    /*** Inode meta-data ***/
    ino_t st_ino;  //File serial number
    mode_t st_mode; //Mode of file
    uid_t st_uid; //User ID of file
    gid_t st_gid;  //Group ID of file
    off_t st_size; //For regular files, the file size in bytes, for symbolic links, the length in bytes of the pathname contained in the symbolic link
    
    /*** File Operations ***/
    /* OPEN: create a file descriptor entry for specified inode
     */
    struct File *(*open)(unsigned long inode);

    /*** Directory Operations ***/
    /* READDIR: read next entry in directory
       Background
         + Seemingly applicable information from readdir(2): 
	   - `int readdir(unsigned int fd, struct old_linux_dirent *dirp, unsigned int count);` 
	   - reads one "old linux dirent" structure from the directory referred to 
             by the file descriptor 'fd' into the buffer pointed to by 'dirp'. The 
	     argument count is ignored; at most one "old linux dirent" structure is 
	     read.
	   - `struct old_linux_dirent {
	        long  d_ino; // `d_ino` is an inode number.
		off_t d_off; // `d_off` is the distance from the start of the directory to this `old_linux_dirent`.
		unsigned short d_reclen; //`d_reclen` is the size of `d_name`, not counting the terminating null byte ('\0').
		char d_name[NAME_MAX+1]; //`d_name` is a null-terminated filename.
	      };`
	   - On success, 1 is returned. On end of directory, 0 is returned. On error, -1 is returned, and errno is set appropriately.
	     + EBADF   - Invalid file descriptor `fd`
	     + EFAULT  - Argument points outside the calling process's address space (likely referring to `dirp`)
	     + EINVAL  - Result buffer is too small (likely referring to `dirp`)
	     + ENOENT  - No such directory (seems to indicate that `fd` does not point to a valid file)
	     + ENOTDIR - File descriptor does not refer to a directory
	   - call this using syscall(2)
	   
	 + Applying this information to provided definition:
	   - I understand readdir(2) to generally be doing:
	     + Looks up directory using `fd`.
	       - Directory must already be "opened" file.
	     + Attempts to read next entry from the directory found.
	       - Probably keeping track of current position using file position information in File Descriptor table.
	     + If another directory entry was found:
	       1) Pull directory entry information from current file position
	       2) Increment file position by dirent size
e	       
	   - Since provided definition does not include dirent pointer, I assume that the `dirent` information is returned by
	     calling the supplied `readdir_cb` with this information.

	   - Attempting to understand `int readdir(
	       struct Inode *inode, //assuming this is the inode of the directory
	       readdir_cb, //assuming this is how information is returned
	       void *p     //no idea what this is, but best guess is that it is related to `readdir_cb`'s void * argument 
	   - Attempting to understand `int readdir_cb(
	       const char *, //unsure but assume this is the filename for the directory entry
	       struct Inode *, //unsure but assuming this contains information for the entry
	       void * //no idea what this is doing
	     );`
    */
    int (*readdir)(struct Inode *inode, readdir_cb cb, void *p);

    /* UNLINK: delete a file from the fs
       Background
         + Seemingly applicable information from unlink(2):
	   - `int unlink(const char *pathname);`
	     + Deletes a file name from the file system. If that name was the last link to a file and no processes have the file open,
	       the file is deleted and the space it was using is made available for reuse.
	     + If the name referred to a symbolic link, the link is removed.
	     
	   - `int unlinkat(int dirfd, const char *pathname, int flags);`
	     + Operates in exactly the same way as either `unlink()` or `rmdir(2)`.
	     + If the pathname given in `pathname` is relative, then it is interpreted relative to the directory referred to by
	       the file descriptor `dirfd` (rather than relative to the current working directory of the calling process).
	     + If the pathname given in `pathname` is absolute, then `dirfd` is ignored.

	  - On success, 0 is returned. On error, -1 is returned, and `errno` is set appropriately.
	    + EACESS       - Write access to the directory containing `pathname` is not allowed for the process's effective UID, 
	                     or one of the directories in `pathname` did not allow search permission.
	    + EBUSY        - The file `pathname` cannot be unlinked because it is being used by the system or another process; for 
	                     example, it is a mount point or the NFS client software created it to represent an active but otherwise
			     nameless inode ("NFS silly renamed").
	    + EFAULT       - `pathname` points outside your accessible address space.
	    + EIO          - An I/O error occurred.
	    + EISDIR       - `pathame` refers to a directory.
	    + ELOOP        - Too many symbolic links were encountered in translating filename.
	    + ENAMETOOLONG - `pathname` was too long.
	    + ENOENT       - A component in `pathname` does not exist or is a dangling symbolic link, or `pathname` is empty.
	    + ENOMEM       - Insufficient kernel memory was available.
	    + ENOTDIR      - A component used as a directory in `pathname` is not, in fact, a directory.
	    + EPERM        - The filesystem does not allow unlinking of files.
	    + EROFS        - `pathname` refers to a read-only filesystem.
	    + EBADF        - `dirfd` is not a valid file descriptor
	    + EINVAL       - an invalid flag value was specified in `flags`
	 + Applying this information to provided definition:
	   - inode represents the current directory
	   - name represents the name of the file to delete
    */
    int (*unlink)(struct Inode *inode, const char *name);
    
    /*** Inode management ***/
    void (*free)(struct Inode **inode);
};

/* Generic File class
 * - A file opened for reading, writing, or both
 * - A file has an associated inode
 * - One inode can be opened multiple times concurrently */
struct File {
    struct Inode *assoc_inode;
    struct Inode *cur_inode;
    struct Inode *prev_inode;
    uint64_t cur_offset;
    
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
