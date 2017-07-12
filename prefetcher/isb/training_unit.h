#define TU_WAY_COUNT 64
#define PHY_ADDRESS_ENCODER_SIZE 256
#define STR_ADDRESS_ENCODER_SIZE 256

#include <map>

struct TrainingUnitEntry
{
    uint64_t key;
    uint64_t addr;
    unsigned int str_addr;

    TrainingUnitEntry(){
        reset();
    } 
    void reset(){
        key = 0;
        addr = 0;
        str_addr = 0;
    }  
    TrainingUnitEntry(uint64_t _key){
        key = _key;
        addr = 0;
        str_addr = 0;
    }   
};

typedef std::map<uint64_t, TrainingUnitEntry*> TUCache;

