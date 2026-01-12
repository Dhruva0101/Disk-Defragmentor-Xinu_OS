#include "defrag.h"
#include "blocks.h"

unsigned char *og_disk =NULL;
size_t total_size=0;
struct superblock super_block;

int read_int(unsigned char *ptr){    //read integer  from disk
    return *(ptr+3) << 24 | *(ptr+2) << 16 | *(ptr+1) << 8 | *ptr;
}

void write_int(unsigned char *ptr, int value){       //write an integer to disk
    *ptr =value & 0xFF;
    *(ptr+1) =(value >> 8) & 0xFF;
    *(ptr+2) =(value >> 16) &0xFF;
    *(ptr+3) =(value >> 24)& 0xFF;
}

void load_superblock(){  //load superblock from disk
    unsigned char *ptr =og_disk +512;
    super_block.blocksize =read_int(ptr);
    super_block.inode_offset =read_int(ptr+4);
    super_block.data_offset =read_int(ptr+8);
    super_block.swap_offset =read_int(ptr+12);
    super_block.free_inode =read_int(ptr+16);
    super_block.free_block =read_int(ptr+20);
}

void load_inode(int index,struct inode *i_node){  //load an inode from disk

    int i;
    int start =1024 +super_block.inode_offset* super_block.blocksize;
    unsigned char *ptr =og_disk+ start +index * sizeof(struct inode);
    
    i_node->next_inode =read_int(ptr); ptr +=4;
    i_node->protect =read_int(ptr); ptr +=4;
    i_node->nlink =read_int(ptr); ptr +=4;
    i_node->size =read_int(ptr); ptr +=4;
    i_node->uid =read_int(ptr); ptr +=4;
    i_node->gid =read_int(ptr); ptr +=4;
    i_node->ctime =read_int(ptr); ptr +=4;
    i_node->mtime =read_int(ptr);ptr +=4;
    i_node->atime =read_int(ptr); ptr +=4;
    
    for(i =0; i <N_DBLOCKS; i++){
        i_node->dblocks[i] =read_int(ptr);ptr +=4;
    }
    for(i =0; i < N_IBLOCKS; i++){
        i_node->iblocks[i] =read_int(ptr);ptr +=4;
    }
    i_node->i2block = read_int(ptr);ptr +=4;
    i_node->i3block =read_int(ptr);
}

//save an inode to a disk
void save_inode(unsigned char *disk, int index,struct inode *i_node){

    int i;
    int start =1024 + super_block.inode_offset *super_block.blocksize;
    unsigned char *ptr =disk+ start +index * sizeof(struct inode);
    
    write_int(ptr,i_node->next_inode);ptr +=4;
    write_int(ptr,i_node->protect); ptr +=4;
    write_int(ptr,i_node->nlink); ptr +=4;
    write_int(ptr,i_node->size); ptr +=4;
    write_int(ptr,i_node->uid); ptr +=4;
    write_int(ptr,i_node->gid); ptr +=4;
    write_int(ptr,i_node->ctime); ptr +=4;
    write_int(ptr,i_node->mtime); ptr +=4;
    write_int(ptr,i_node->atime); ptr +=4;
    
    for(i =0 ; i < N_DBLOCKS; i++){
        write_int(ptr,i_node->dblocks[i]);ptr +=4;
    }
    for(i =0; i < N_IBLOCKS ; i++){
        write_int(ptr,i_node->iblocks[i]);ptr +=4;
    }
    write_int(ptr, i_node->i2block);ptr +=4;
    write_int(ptr ,i_node->i3block);
}

//get pointer to data block
unsigned char* get_block(unsigned char *disk,int block_num){
    int data_start =1024 + super_block.data_offset * super_block.blocksize;
    return disk +data_start +block_num * super_block.blocksize;
}

//copy a data block
void copy_block(unsigned char *new_disk,int new_num,unsigned char *og_disk, int og_num){
    memcpy(get_block(new_disk, new_num), get_block(og_disk, og_num),super_block.blocksize);
}

// defragment the file
void defrag_file(unsigned char *new_disk,struct inode *i_node,int *next){

    int i;
    int ptrs_per_block = super_block.blocksize /4;
    int bytes_left =i_node->size;
    
    //handle direct blocks
    int num_direct =(bytes_left/ super_block.blocksize );
    if(bytes_left % super_block.blocksize != 0){
        num_direct++;
    }

    if(num_direct > N_DBLOCKS){
        num_direct = N_DBLOCKS;
    }

    for(i = 0; i < num_direct; i++){
        if(i_node->dblocks[i] >=0){
            int new_block =(*next)++;
            copy_block(new_disk ,new_block,og_disk, i_node->dblocks[i]);
            i_node->dblocks[i] =new_block;
        }
    }
    
    bytes_left -= num_direct * super_block.blocksize;
    if(bytes_left <=0){
        return;
    }
    
    //handle single indirect blocks
    for(i =0; i < N_IBLOCKS && bytes_left > 0; i++){
        if(i_node->iblocks[i] >=0){
            int blocks_here =(bytes_left / super_block.blocksize);
            if(bytes_left % super_block.blocksize !=0){
                blocks_here++;
            }

            if(blocks_here > ptrs_per_block){
                blocks_here =ptrs_per_block;
            }

            i_node->iblocks[i] =fix_single_indirect(new_disk,i_node->iblocks[i],next,blocks_here,bytes_left);
            bytes_left -=blocks_here * super_block.blocksize;
        }
    }
    
    if(bytes_left <=0){
        return;
    }

    //Handle duoble indirect
    if(i_node->i2block >=0){
        i_node->i2block =fix_double_indirect(new_disk,i_node->i2block,next,bytes_left);
        int blocks_in_double =ptrs_per_block * ptrs_per_block;
        bytes_left -=blocks_in_double *super_block.blocksize;
    }
    
    if(bytes_left <=0){
        return;
    }

    //Handle triple indirect
    if(i_node->i3block >=0){
        i_node->i3block =fix_triple_indirect(new_disk,i_node->i3block,next,bytes_left);
    }
}

int main(int argc,char *argv[]){

    FILE *file =fopen(argv[1],"rb");

    struct stat file_info;                //get filesize

    fstat(fileno(file),&file_info);
    total_size =file_info.st_size;
    
    og_disk =malloc(total_size);         //allocate memory for og disk

    fread(og_disk,1,total_size,file);        //read disk to memory
    fclose(file);
    
    load_superblock();
    
    unsigned char *new_disk =calloc(1,total_size);      //allocate mem for new disk

    memcpy(new_disk,og_disk,1024);        //copy boot block
    
    //calc  all region boundaries
    int inode_start =1024 + super_block.inode_offset* super_block.blocksize;
    int data_start = 1024 + super_block.data_offset* super_block.blocksize;
    int swap_start = 1024 + super_block.swap_offset *super_block.blocksize;
    int inode_size = data_start - inode_start;
    int num_inodes = inode_size /sizeof(struct inode);
    
    memcpy(new_disk + inode_start,og_disk + inode_start,inode_size);      //copy inode region
    memcpy(new_disk +swap_start,og_disk + swap_start,total_size - swap_start);      //copy swap space
    
    int next_block =0;
    int i;
    
    //Defragment each file
    for(i =0; i <num_inodes; i++){
        struct inode i_node;
        load_inode(i,&i_node);
        
        if(i_node.nlink > 0){
            defrag_file(new_disk, &i_node,&next_block);
            save_inode(new_disk,i, &i_node);
        }
    }
    
    //build free block list
    int total_blocks =(swap_start- data_start)/super_block.blocksize;
    int first_free =next_block;
    
    for(i =next_block; i < total_blocks; i++){
        unsigned char *block =get_block(new_disk,i);
        if(i + 1 <total_blocks){
            write_int(block,i+ 1);
        }
        else{
           write_int(block,-1);
        }
        memset(block +4, 0 ,super_block.blocksize - 4);
    }
    
    unsigned char *sb_ptr =new_disk + 512;       //Update superrblock with new free list
    write_int(sb_ptr + 20, first_free);

    FILE *out =fopen(OUTPUT_FILE_NAME,"wb");         //write o/p file
    
    fwrite(new_disk,1,total_size,out);
    fclose(out);

    free(og_disk);
    free(new_disk);
    
    return 0;
}