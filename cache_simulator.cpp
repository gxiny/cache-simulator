#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <string.h>
#include <vector>
#include <list>
#include <cmath>
#include "cs.h"

//transfer hex string into binary string
std::string hex_to_binary(std::string hex){
  std::string binary;
  int size = hex.size();
  for(int i=0;i<size;i++){
    char c = hex[i];
    std::string temp;
    switch(c){
      case '0' : temp = "0000"; break;
      case '1' : temp = "0001"; break;
      case '2' : temp = "0010"; break;
      case '3' : temp = "0011"; break;
      case '4' : temp = "0100"; break;
      case '5' : temp = "0101"; break;
      case '6' : temp = "0110"; break;
      case '7' : temp = "0111"; break;
      case '8' : temp = "1000"; break;
      case '9' : temp = "1001"; break;
      case 'a' : temp = "1010"; break;
      case 'b' : temp = "1011"; break;
      case 'c' : temp = "1100"; break;
      case 'd' : temp = "1101"; break;
      case 'e' : temp = "1110"; break;
      case 'f' : temp = "1111"; break;
      default  : std::cout << "address error" << std::endl; break;
    }
    binary = binary + temp;
  }
  return binary;
}

//break input into code and address
std::vector<std::string> parse(std::string str){
  std::vector<std::string> result;
  std::size_t found = str.find(" ");
  std::string code = str.substr(0,1);
  result.push_back(code);
  std::string hex_address = str.substr(found+1);
  int num = 8 - hex_address.size();
  std::string extend;
  for(int i=0;i<num;i++){
    extend = extend + "0";
  }
  hex_address.insert(0,extend);
  //std::cout<<hex_address<<std::endl;
  std::string address = hex_to_binary(hex_address);
  result.push_back(address);
  return result;               //result[0]=code result[1]=address
}
//transfer binary int to decimal int
int binaryToDecimal(int num){
  int dec = 0, rem, base = 1;
  while (num > 0){
    rem = num % 10;
    dec = dec + rem * base;
    base = base * 2;
    num = num / 10;
  }
  return dec;
}
//parse address into tag\index\offset
std::vector<size_t> tag_index(std::string address,size_t tag_len,size_t index_len){
  std::vector<size_t> t_i;
  std::string tag_s = address.substr(0,tag_len);
  std::string index_s = address.substr(tag_len,index_len);
  size_t tag_b = atoi(tag_s.c_str());
  size_t index_b = atoi(index_s.c_str());
  size_t tag_d = binaryToDecimal(tag_b);
  size_t index_d = binaryToDecimal(index_b);
  t_i.push_back(tag_d);
  t_i.push_back(index_d);
  return t_i;         //tag_index[0]=tag,tag_index[1]=index
}

//if miss, push_front or replace and set dirty bit
int update(size_t code,size_t tag,size_t index,block* new_block,cache &L){
  int size = L.cache_table[index].size();
  if(size < L.associativity){  //if miss && set size < associativity, push front a new block
    L.cache_table[index].push_front(std::pair<size_t,block>(tag,*new_block));
  }
  else if(size == L.associativity){
    //set is full, replace
    if(L.replace_method == 0){ //LRU
      std::pair<size_t,block> lastone = L.cache_table[index].back();
      if(lastone.second.dirty){
        if(L.layer == 1){
          //if L1 dirty miss, write back to L2
          return 1;
        }
      }
      L.cache_table[index].push_front(std::pair<size_t,block>(tag,*new_block));
      L.cache_table[index].pop_back();
    }
    if(L.replace_method == 1){
      //RND
    }
  }
  return 0;
}

int main(int argc, char ** argv){
  if (argc < 12) {
  //argv[0] is cache_simulator, argv[1] is trace file, argv[2] is L1 associativity
  //argv[3] is L1 block_size, argv[4] is L1 capacity, argv[5] is L2 associativity
  //argv[6] is L2 block_size, argv[7] is L2 capacity, argv[8] is replace method
  //argv[9] is L1 hit time, argv[10] is L2 hit time, argv[11] is DRAM access time
    std::cerr << "Please input enough parameter" << std::endl;
    return EXIT_FAILURE;
  }  
  size_t associativity1 = atoi(argv[2]);      //A number of blocks in a set
  size_t block_size1 = atoi(argv[3]);         //B number of byte in a block
  size_t capacity1 = atoi(argv[4]);           //C capacity in bytes
  size_t associativity2 = atoi(argv[5]);      
  size_t block_size2 = atoi(argv[6]);         
  size_t capacity2 = atoi(argv[7]);           
  size_t replace = atoi(argv[8]);            //replace method
  size_t L1_hit_time = atoi(argv[9]);        //1
  size_t L2_hit_time = atoi(argv[10]);       //20
  size_t DRAM_access_time = atoi(argv[11]);  //120
  //read trace file
  std::ifstream ifs;
  std::string line;
  ifs.open(argv[1],std::ifstream::in);
  if(!ifs.is_open()){
    std::cout << "file open failed" << std::endl;
    return EXIT_FAILURE;
  }
  //assume L1 hit time is 2, L2 hit time is 20, DRAM access time is 120
  cache LD(associativity1,block_size1,capacity1/2,replace,1);//data cache
  cache LI(associativity1,block_size1,capacity1/2,replace,1);//instruction cache
  cache L2(associativity2,block_size2,capacity2,replace,2);//L2
  while(getline(ifs,line)){
    std::vector<std::string> result = parse(line);
    std::string address = result[1];
    size_t code = atoi(result[0].c_str());
    //std::cout << code << std::endl;
    std::vector<size_t> L1_ti = tag_index(address,LD.tag_len,LD.index_len);//L1_ti[0] = tag for L1, L1_ti[1] = index for L1
    std::vector<size_t> L2_ti = tag_index(address,L2.tag_len,L2.index_len);
    //std::cout << "tag:" << L1_ti[0] << std::endl;
    //std::cout << "index:" << L1_ti[1] << std::endl;
    bool LI_read_hit = true;
    bool LD_read_hit = true;
    bool LD_write_hit = true;
    if(code == 2){
      LI_read_hit = LI.read(L1_ti[0],L1_ti[1],code);
    }
    if(code == 0){ 
      LD_read_hit = LD.read(L1_ti[0],L1_ti[1],code);
    }
    if(code == 1){
      LD_write_hit = LD.write(L1_ti[0],L1_ti[1]);
    }
    //if L1 hit, nothing to do with L2
    //if L1 clean miss, all operation in L2 is read
    //if L1 dirty miss, operations in L2 is read and write
    if(!LI_read_hit){//if instruction miss
      //update LI
      block* new_block = new block(L1_ti[0]);
      int LI_update = update(code,L1_ti[0],L1_ti[1],new_block,LI);
      delete new_block;
      if(LI_update){
        //if LI dirty miss, write back L2, then find required block in L2
        std::pair<size_t,block> lastone = LI.cache_table[L1_ti[1]].back();
        bool L2_write_hit = L2.write(lastone.second.tag,L1_ti[1]);
        bool L2_read_hit = L2.read(L2_ti[0],L2_ti[1],code);
        if(!L2_read_hit){ 
          //if required block miss, update L2
          block* new_block = new block(L2_ti[0]);
          update(code,L2_ti[0],L2_ti[1],new_block,L2);
          delete new_block;
        }
        //update LI
        block* new_block = new block(L1_ti[0]);
        size_t index = L1_ti[1];
        LI.cache_table[index].push_front(std::pair<size_t,block>(index,*new_block));
        LI.cache_table[index].pop_back();
        delete new_block;  
      }
      else{
        std::pair<size_t,block> lastone = LI.cache_table[L1_ti[1]].back();
        bool L2_write_hit = L2.write(lastone.second.tag,L1_ti[1]);
        bool L2_read_hit = L2.read(L2_ti[0],L2_ti[1],code);
        if(!L2_read_hit){ 
          //if required block miss, update L2
          block* new_block = new block(L2_ti[0]);
          update(code,L2_ti[0],L2_ti[1],new_block,L2);
          delete new_block;
        }
      }
    }     
  
      if((!LD_read_hit)||(!LD_write_hit)){//if data miss
      //update LD
      block* new_block = new block(L1_ti[0]);
      int LD_update = update(code,L1_ti[0],L1_ti[1],new_block,LD);
      delete new_block;
      if(LD_update){
        //if LD dirty miss, write back L2, then find required block in L2
        std::pair<size_t,block> lastone = LD.cache_table[L1_ti[1]].back();
        bool L2_write_hit = L2.write(lastone.second.tag,L1_ti[1]);
        bool L2_read_hit = L2.read(L2_ti[0],L2_ti[1],code);
        if(!L2_read_hit){ 
          //if required block miss, update L2
          block* new_block = new block(L2_ti[0]);
          update(code,L2_ti[0],L2_ti[1],new_block,L2);
          delete new_block;
        }
        //update LD
        block* new_block = new block(L1_ti[0]);
        size_t index = L1_ti[1];
        LD.cache_table[index].push_front(std::pair<size_t,block>(index,*new_block));
        LD.cache_table[index].pop_back();
        delete new_block;  
      }
      else{//if LD clean miss, search in L2  
        bool L2_read_hit = L2.read(L2_ti[0],L2_ti[1],code);
        if(!L2_read_hit){ 
          //if required block miss, update L2
          block* new_block = new block(L2_ti[0]);
          update(code,L2_ti[0],L2_ti[1],new_block,L2);
          delete new_block;
        }        
      }
    }    
  }
  std::cout << "for L1 data cache:" << std::endl;
  LD.print(0);
  std::cout << "for L1 instruction cache:" << std::endl;
  LI.print(2);
  std::cout << std::endl;
  std::cout << "for L2 cache:" << std::endl;
  L2.print(0);
  L2.print(2);
  std::cout<<std::endl;
  double LI_miss_rate = 100*(LI.insn_miss_count)/(LI.insn_miss_count+LI.insn_hit_count);
  double LD_miss_rate = 100*(LD.read_miss_count+LD.write_miss_count)/(LD.read_miss_count+LD.read_hit_count+LD.write_miss_count+LD.write_hit_count);
  double L2_miss_rate = 100*(L2.read_miss_count+L2.write_miss_count+L2.insn_miss_count)/(L2.read_miss_count+L2.read_hit_count+L2.write_miss_count+L2.write_hit_count+L2.insn_miss_count+L2.insn_hit_count);
  double t2 = DRAM_access_time*L2_miss_rate/100+L2_hit_time;
  double tI = L2_hit_time*LI_miss_rate/100+L1_hit_time;
  double tD = L2_hit_time*LD_miss_rate/100+L1_hit_time;
  std::cout << "LI average time = "<< tI << std::endl;
  std::cout << "LD average time = "<< tD << std::endl;
  std::cout << "L2 average time = "<< t2 << std::endl;
	return 0;
}
