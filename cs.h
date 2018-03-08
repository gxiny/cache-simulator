#ifndef __CS_H__
#define __CS_H__
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <string.h>
#include <vector>
#include <cmath>

class block{
public:
  size_t tag;
  bool validbit;        // true if valid
  bool dirty;          // true if dirty
  //constructor
  block(size_t t):tag(t),validbit(true),dirty(false){}
};

class cache{
public:
  size_t capacity;     //C capacity in bytes
  size_t associativity;//A number of blocks in a set
  size_t block_size;   //B number of byte in a block
  size_t replace_method;   //0 is LRU, 1 is RND, 2 is NMRU, 3 is FIFO
  size_t read_miss_count;
  size_t read_hit_count;
  size_t write_miss_count;
  size_t write_hit_count;
  size_t insn_miss_count;
  size_t insn_hit_count;
  size_t block_num;        //number of blocks
  size_t set_num;          //set number
  size_t offset_len;
  size_t index_len;
  size_t tag_len;
  size_t tag;
  size_t index;
  int layer;
  double read_miss_rate;
  double write_miss_rate;
  double insn_miss_rate;
  std::vector<std::list<std::pair<size_t,block> > > cache_table;
  
  //constructor
  cache(size_t a,size_t b,size_t c,int r,int l):associativity(a),block_size(b),capacity(c),replace_method(r),
                                          read_miss_count(0),read_hit_count(0),write_miss_count(0),
                                          write_hit_count(0),insn_miss_count(0),insn_hit_count(0),layer(l)
  {
    set_num = capacity/block_size/associativity;        //number of set
    block_num = capacity/block_size;                    //number of blocks
    set_num = block_num/associativity;                  //set number
    offset_len = log2(block_size);
    index_len = log2(set_num);
    tag_len = 32 - offset_len - index_len;
  
    for(int i=0;i<set_num;i++){
      std::list<std::pair<size_t,block> >* new_list = new std::list<std::pair<size_t,block> >();
      cache_table.push_back(*new_list);
      delete new_list;
    }
  }
  //read
  //if hit,return 1, if miss, return 0
  bool read(size_t tag,size_t index,size_t code){
    std::list<std::pair<size_t,block> > set = cache_table[index];
    std::list<std::pair<size_t,block> >::iterator iter;
    for(iter=set.begin();iter != set.end();++iter){
      if(tag == iter->first){
        if(iter->second.validbit){ 
          //if hit 
          if(code == 0){
            read_hit_count++; 
          }
          else if(code == 2){
            insn_hit_count++; 
          }
          //cache_table[index].push_front(*iter);
          //cache_table[index].erase(iter);
          return true;
        }
      }
    }
    if(code == 0){
      read_miss_count++; 
    }
    else if(code == 2){
      insn_miss_count++; 
    }   
    return false;
  }
  //write
  int write(size_t tag,size_t index){
    std::list<std::pair<size_t,block> > set = cache_table[index];
    std::list<std::pair<size_t,block> >::iterator iter;
    for(iter=set.begin();iter != set.end();++iter){
      if(tag == iter->first){
        if(iter->second.validbit){ 
          //if hit 
          write_hit_count++; 
          iter->second.dirty = true;
          //cache_table[index].push_front(*iter);
          //cache_table[index].erase(iter);
          //std::cout << "write_hit_count=" << write_hit_count << std::endl;
          return true;
        }
      }
    }
    write_miss_count++; 
    //std::cout << "write_miss_count=" << write_miss_count << std::endl;
    return false;
  }
  
  //print
  void print(size_t code){
    if(code == 0||code ==1){
      std::cout << "read_hit_count=" << read_hit_count << std::endl;
      std::cout << "read_miss_count=" << read_miss_count << std::endl;
      std::cout << "write_hit_count=" << write_hit_count << std::endl;
      std::cout << "write_miss_count=" << write_miss_count << std::endl;
      if((read_hit_count+read_miss_count)!=0){
        read_miss_rate = 100*(read_miss_count)/(read_hit_count+read_miss_count);
        std::cout << "read_miss_rate=" << read_miss_rate <<"%"<< std::endl;
      }
      if((write_hit_count+write_miss_count)!=0){
        write_miss_rate = 100*(write_miss_count)/(write_hit_count+write_miss_count);
        std::cout << "write_miss_rate=" << write_miss_rate <<"%"<< std::endl;
      }
    }
    if(code == 2){
      std::cout << "insn_hit_count=" << insn_hit_count << std::endl;
      std::cout << "insn_miss_count=" << insn_miss_count << std::endl;
    }
    
    if((insn_hit_count+insn_miss_count)!=0){
      insn_miss_rate = 100*(insn_miss_count)/(insn_hit_count+insn_miss_count);
      std::cout << "insn_miss_rate=" << insn_miss_rate <<"%"<< std::endl;
    }
  }
};

#endif