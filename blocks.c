#include "blocks.h"
#include "defrag.h"

//singleindirect block
int fix_single_indirect(unsigned char *new_disk, int og_block,int *next, int blocks,int bytes){

    int i;
    int ptrs_per_block =super_block.blocksize /4;
    int new_ptr_block=(*next)++;
    
    unsigned char *og_ptr=get_block(og_disk,og_block);
    unsigned char *new_ptr = get_block(new_disk,new_ptr_block);
    
    int num_blocks =(bytes /super_block.blocksize);
    if(bytes % super_block.blocksize != 0){
        num_blocks++;
    } 
    if(num_blocks >blocks){
        num_blocks =blocks;
    }
    for(i =0; i < num_blocks; i++){
        int og_data =read_int(og_ptr + i * 4);
        if(og_data >=0){
            int new_data =(*next)++;
            copy_block(new_disk, new_data,og_disk,og_data);
            write_int(new_ptr +i *4,new_data);
        }
        else{
            write_int(new_ptr + i* 4, -1);
        }
    }
    
    for(i =num_blocks;i < ptrs_per_block;i++){
        write_int(new_ptr + i * 4,-1);
    }
    
    return new_ptr_block;
}

//double indirect block
int fix_double_indirect(unsigned char *new_disk,int og_block,int *next, int bytes){

    int i;
    int ptrs_per_block =super_block.blocksize /4;
    int new_ptr_block =(*next)++;
    
    unsigned char *og_ptr= get_block(og_disk,og_block);
    unsigned char *new_ptr = get_block(new_disk,new_ptr_block);
    
    int total_blocks =(bytes / super_block.blocksize);
    if(bytes % super_block.blocksize != 0 ){
        total_blocks++;
    }
    int num_indirects =(total_blocks / ptrs_per_block);
    if(total_blocks % ptrs_per_block != 0){
        num_indirects++;
    }
    if(num_indirects > ptrs_per_block){
        num_indirects =ptrs_per_block;
    }

    for(i =0; i < num_indirects; i++){
        int og_ind =read_int(og_ptr + i * 4);
        if(og_ind >=0){
            int blocks_here = total_blocks > ptrs_per_block ? ptrs_per_block : total_blocks;
            int bytes_remaining =blocks_here * super_block.blocksize;
            if(bytes < bytes_remaining){
                bytes_remaining = bytes;
            }

            int new_ind =fix_single_indirect(new_disk,og_ind, next,blocks_here, bytes_remaining);
            write_int(new_ptr + i * 4,new_ind);
            
            total_blocks -=blocks_here;
            bytes -=bytes_remaining;
        }
        else{
            write_int(new_ptr +i * 4,-1);
        }
    }
    
    for(i =num_indirects; i< ptrs_per_block; i++){
        write_int(new_ptr +i * 4,-1);
    }
    
    return new_ptr_block;
}

//triple indirect blck
int fix_triple_indirect(unsigned char *new_disk,int og_block,int *next,int bytes){

    int i;
    int ptrs_per_block =super_block.blocksize /4;
    int new_ptr_block =(*next)++;
    
    unsigned char *og_ptr =get_block(og_disk,og_block);
    unsigned char *new_ptr = get_block(new_disk,new_ptr_block);
    
    int blocks_per_double =ptrs_per_block * ptrs_per_block;
    int total_blocks =(bytes / super_block.blocksize);
    if (bytes % super_block.blocksize  !=0 ){
        total_blocks++;
    } 
    int num_doubles =(total_blocks /blocks_per_double);
    if (total_blocks % blocks_per_double != 0){
        num_doubles++;
    }

    if(num_doubles > ptrs_per_block){
        num_doubles =ptrs_per_block;
    }

    for(i =0; i <num_doubles; i++){
        int og_dbl =read_int(og_ptr + i * 4);
        if(og_dbl >=0){
            int bytes_remaining =blocks_per_double * super_block.blocksize;
            if(bytes < bytes_remaining){
                bytes_remaining=bytes;
            }

            int new_dbl =fix_double_indirect(new_disk,og_dbl,next,bytes_remaining);
            write_int(new_ptr +i * 4,new_dbl);
            
            bytes -=bytes_remaining;
        }
        else{
            write_int(new_ptr+ i * 4,-1);
        }
    }
    
    for(i =num_doubles; i <  ptrs_per_block; i++){
        write_int(new_ptr + i * 4,-1);
    }
    
    return new_ptr_block;
}

