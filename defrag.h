#ifndef DEFRAG_H
#define DEFRAG_H

#define _GNU_SOURCE
#define N_DBLOCKS 10
#define N_IBLOCKS 4
#define OUTPUT_FILE_NAME "disk_defrag"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

// Superblock structure
  struct superblock {
    int blocksize; /* size of blocks in bytes */
    int inode_offset; /* offset of inode region in blocks */
    int data_offset; /* data region offset in blocks */
    int swap_offset; /* swap region offset in blocks */
    int free_inode; /* head of free inode list */
    int free_block; /* head of free block list */
  };

// Inode structure  
  struct inode {  
      int next_inode; /* list for free inodes */  
      int protect;        /*  protection field */ 
      int nlink;  /* Number of links to this file */ 
      int size;  /* Number of bytes in file */   
      int uid;   /* Owner's user ID */  
      int gid;   /* Owner's group ID */  
      int ctime;  /* Time field */  
      int mtime;  /* Time field */  
      int atime;  /* Time field */  
      int dblocks[N_DBLOCKS];   /* Pointers to data blocks */  
      int iblocks[N_IBLOCKS];   /* Pointers to indirect blocks */  
      int i2block;     /* Pointer to doubly indirect block */  
      int i3block;     /* Pointer to triply indirect block */  
   };

// Global var
extern unsigned char *og_disk;
extern struct superblock super_block;
extern size_t total_size;

int read_int(unsigned char *ptr);
void write_int(unsigned char *ptr, int value);
void load_superblock(void);
void load_inode(int index, struct inode *node);
void save_inode(unsigned char *disk, int index, struct inode *node);
unsigned char* get_block(unsigned char *disk, int block_num);
void copy_block(unsigned char *new_disk, int new_num, unsigned char *og_disk, int og_num);
void defrag_file(unsigned char *new_disk, struct inode *node, int *next);

#endif