#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string>
#include <vector>

#define numbersofcachewords

using namespace std;

std::fstream& goToLine(std::fstream& file, unsigned int num);

typedef struct{

  int cache_tag;
  int set_index;
  int data_index;
  int hit;
  int miss;

}cache_line;

vector<uint64_t> L1,L2,ADDR;
vector<cache_line> cache;

uint64_t translateAddress(int virtual_address,vector<uint64_t> l1table, vector<uint64_t> l2table);
vector<cache_line>  doCache(uint64_t address, vector<cache_line> cache);

int main(const int argc,const char *argv[]){

  if(argc!=4){
    cout << "Usage: ./cachesim <l1 table path> <l2 table path> <address path>\n";
    return(-20);
  }

  fstream input;
  input.open(argv[1]);

  uint64_t line;
  string hex_perfix;

  if(!input){
    std::cerr << "Unable to open L1 file" << std::endl;
    return -19;
  }else{
    while(input >>  hex  >> line){
      L1.push_back(line);
    }
  }
  
  input.close();
  input.open(argv[2]);
  if(!input){
    std::cerr << "Unable to open L2 file" << std::endl;
    return -18;
  }else{
    while(input >>  hex  >> line){
      L2.push_back(line);
    }
  }
  input.close();
  input.open(argv[3]);
  if(!input){
    cout << "Unable to open ADDR file" << std::endl;
    return -17;
  }else{
    while(input >>  hex  >> line){
      ADDR.push_back(line);
    }
  }
  input.close();

  for(int i=0;i<ADDR.size();i++){
    uint64_t physics_address = translateAddress(ADDR[i],L1,L2);
    cache = doCache(physics_address, cache);
  }

  for(int i=0;i<cache.size();i++){

    cache_line show;
    show.cache_tag = cache[i].cache_tag;
    show.set_index = cache[i].set_index;
    show.data_index = cache[i].data_index;
    show.hit = cache[i].hit;
    show.miss = cache[i].miss;
    uint64_t startaddr = ((show.cache_tag << 12) & 0xFFFFF000) + (show.set_index << 4) + (show.data_index);
    uint64_t finishaddr = startaddr + 3;

    cout << hex << "[0x" << show.cache_tag << "] [0x" << startaddr << " - 0x" << finishaddr << "] " << dec << "H(" << show.hit << ") M(" << show.miss << ")" << endl;
  }
  return(0);
}

uint64_t translateAddress(int virtual_address,vector<uint64_t> l1table, vector<uint64_t> l2table){

  uint64_t l1index,l2index,physics_address,physics_base,physics_offset;

  l1index = (virtual_address & 0xFFF00000) >> 20;

  if((l1table[0] & 0x3) == 0x1){ //Coarse

     l2index = (virtual_address & 0x000FF000) >> 12;

  }else if((l1table[0] & 0x3) == 0x3){ //Fine

     l2index = (virtual_address & 0x000FFC00);

  }else if((l1table[0] & 0x3) == 0x2){ //Section
    /* Not Implemented Yet */ 
     cout << "Section" << endl;
  }else{ //Page Fault
     cout << "Fault" << endl;
    return -15;
  }

  if((l2table[l2index] & 0x3) == 0x1){ //Large Page

    physics_base = (l2table[l2index] & 0xFFFF0000);
    physics_offset = (virtual_address & 0x0000FFFF);
    physics_address = physics_base + physics_offset;

  }else if((l2table[l2index] & 0x3) == 0x3){ //Tiny Page
    physics_base = (l2table[l2index] & 0xFFFFFC00);
    physics_offset = (virtual_address & 0x000003FF);
    physics_address = physics_base + physics_offset;

  }else if((l2table[l2index] & 0x3) == 0x2){ //Small Page

    physics_base = (l2table[l2index] & 0xFFFFF000);
    physics_offset = (virtual_address & 0x00000FFF);
    physics_address = physics_base + physics_offset;

  }else{//Page Fault
    cout << "L2 Page Fault" << endl;
    return -14;
  }

    return physics_address;
}

vector<cache_line> doCache(uint64_t address, vector<cache_line> cache){

  int tag,set,data,cache_index=-1;

  set = (address & 0x00000FF0) >> 4;
  tag = (address & 0xFFFFF000) >> 12;
  data = (address & 0xF);

  for(int i=0;i<cache.size();i++){
    if(set == cache[i].set_index){
      cache_index = i;
      break;
    }
  }

  if(cache_index != -1){
    if(cache[cache_index].cache_tag == tag){
      cache[cache_index].hit++;
    }else{
      cache[cache_index].miss++;
      cache[cache_index].cache_tag = tag;
    }
  }else{
    cache_line line;
    line.set_index = set;
    line.cache_tag = tag;
    line.data_index = data;
    line.hit =0;
    line.miss =1;
    cache.push_back(line);
  }


  return cache;
}
