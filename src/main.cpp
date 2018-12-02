#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string>
#include <vector>
#include <cmath>

#define cacheset 8
#define cachedata 4
#define cachetag 20

using namespace std;

typedef struct{
  int cache_tag;
  int set_index;
  int data_index;
  int hit;
  int miss;
}cache_line;

vector<uint64_t> L1,L2,ADDR;
cache_line cache[256];
bool PageFault = false;

uint64_t translateAddress(uint64_t virtual_address,vector<uint64_t> l1table, vector<uint64_t> l2table);
int doCache(uint64_t address);

int main(const int argc,const char *argv[]){

  if(argc!=4){
    cout << "Usage: ./cachesim <l1 table path> <l2 table path> <address path>\n";
    return(-20);
  }

  fstream input;
  input.open(argv[1]);

  uint64_t line;

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
    if(!PageFault){
      doCache(physics_address);
    }
    PageFault = false;
   }

  for(int i=0;i<256;i++){
    if(cache[i].cache_tag){
      cache_line show;
      show.cache_tag = cache[i].cache_tag;
      show.set_index = cache[i].set_index;
      show.data_index = cache[i].data_index;
      show.hit = cache[i].hit;
      show.miss = cache[i].miss;
      uint64_t startaddr = ((show.cache_tag << 12) & 0xFFFFF000) | (show.set_index << 4 );
      uint64_t finishaddr = startaddr + 15;
      cout << hex << i << " : ";
      printf("[0x%08x][0x%08x-0x%08x] H(%d) M(%d)\n",show.cache_tag,startaddr,finishaddr,show.hit,show.miss);
      //cout << hex /*<< "[0x" << show.cache_tag */<< "] [0x" << startaddr << " - 0x" << finishaddr << "] " << dec << "H(" << show.hit << ") M(" << show.miss << ")" << endl;
    }
  }
  return(0);
}

uint64_t translateAddress(uint64_t virtual_address,vector<uint64_t> l1table, vector<uint64_t> l2table){

  uint64_t l1index,l2index,physics_address,physics_base,physics_offset;

  l1index = (virtual_address & 0xFFF00000) >> 20;

  if((l1table[l1index] & 0x3) == 0x1){ //Coarse
    //cout << "L1 Coarse" << endl;
    l2index = (virtual_address & 0x000FF000) >> 12;
  }else if((l1table[l1index] & 0x3) == 0x3){ //Fine
    //cout << "L1 Fine" << endl;
    l2index = (virtual_address & 0x000FFC00) >> 10;
  }else if((l1table[l1index] & 0x3) == 0x2){ //Section
    //cout << "L1 Section" << endl;
    physics_base = (l1table[l1index] & 0xFFF00000);
    physics_offset = (virtual_address & 0x00FFFFF);
    physics_address = physics_base | physics_offset;
    return physics_address;
  }else{ //Page Fault
     cout << "L1 Page Fault" << endl;
     PageFault = true;
     return 0;
  }

  //cout << "L2 index " << l2index << endl << "Conteudo da linha " << hex << l2table[l2index] << " Endereço " << virtual_address << dec << endl;

  if((l2table[l2index] & 0x3) == 0x1){ //Large Page
    //cout << "L2 Large Page" << endl;
    physics_base = (l2table[l2index] & 0xFFFF0000);
    physics_offset = (virtual_address & 0x0000FFFF);
    physics_address = physics_base | physics_offset;
  }else if((l2table[l2index] & 0x3) == 0x3){ //Tiny Page
    //cout << "L2 Tiny Page" << endl;
    physics_base = (l2table[l2index] & 0xFFFFFC00);
    physics_offset = (virtual_address & 0x000003FF);
    physics_address = physics_base | physics_offset;
    if((l1table[l1index] & 0x3) == 0x1){
        //cout << "L2 Page Fault" << endl;
        PageFault = true;
        return 0;
    }
  }else if((l2table[l2index] & 0x3) == 0x2){ //Small Page
    //cout << "L2 Small Page" << endl;
    physics_base = (l2table[l2index] & 0xFFFFF000);
    physics_offset = (virtual_address & 0x00000FFF);
    physics_address = physics_base | physics_offset;
  }else{//Page Fault
    cout << "L2 Page Fault" << endl;
    PageFault = true;
    return 0;
  }
    return physics_address;
}

int doCache(uint64_t address){

  int tag,set,data;

  set = (address & 0x00000FF0) >> 4;
  tag = (address & 0xFFFFF000) >> 12;
  data = (address & 0xF);

  if((cache[set].cache_tag == tag)&&(cache[set].miss != 0)){
    cache[set].hit++;
    //cache[set].set_index = set;
  }else{
    cache[set].miss++;
    cache[set].cache_tag = tag;
    cache[set].set_index = set;
  }

  return 0;
}
