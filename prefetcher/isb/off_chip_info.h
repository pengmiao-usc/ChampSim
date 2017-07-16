#ifndef __OFF_CHIP_INFO_H
#define __OFF_CHIP_INFO_H

#include <map>

#define STREAM_MAX_LENGTH 256
#define STREAM_MAX_LENGTH_BITS 8

//#define DEBUG

class PS_Entry 
{
  public:
    unsigned int str_addr;
    bool valid;
    unsigned int confidence;

    PS_Entry() {
	reset();
    }

    void reset(){
        valid = false;
        str_addr = 0;
        confidence = 0;
    }
    void set(unsigned int addr){
        reset();
        str_addr = addr;
        valid = true;
        confidence = 3;
    }
    void increase_confidence(){
        confidence = (confidence == 3) ? confidence : (confidence+1);
    }
    bool lower_confidence(){
        confidence = (confidence == 0) ? confidence : (confidence-1);
        return confidence;
    }
};

class SP_Entry 
{
  public:
    uint64_t phy_addr;
    bool valid;

    void reset(){
        valid = false;
        phy_addr = 0;
    }

    void set(uint64_t addr){
        phy_addr = addr;
        valid = true;
    }
};

class OffChipInfo
{
    public:
        std::map<uint64_t,PS_Entry*> ps_map;
        std::map<unsigned int,SP_Entry*> sp_map;

        OffChipInfo()
        {
            reset();
        }

        void reset()
        {
            ps_map.clear();
            sp_map.clear();
        }
        bool get_structural_address(uint64_t phy_addr, unsigned int& str_addr)
        {
            std::map<uint64_t, PS_Entry*>::iterator ps_iter = ps_map.find(phy_addr);
            if(ps_iter == ps_map.end()) {
#ifdef DEBUG	
                (*outf)<<"In get_structural address of phy_addr "<<phy_addr<<", str addr not found\n";
#endif  
                return false;
            }
            else {
                if(ps_iter->second->valid) {
                    str_addr = ps_iter->second->str_addr;
#ifdef DEBUG    
                    (*outf)<<"In get_structural address of phy_addr "<<phy_addr<<", str addr is "<<str_addr<<"\n";
#endif
                    return true;
                }
                else {
#ifdef DEBUG    
                    (*outf)<<"In get_structural address of phy_addr "<<phy_addr<<", str addr not valid\n";
#endif
                    return false;
                }
            }			

        }

        bool get_physical_address(uint64_t& phy_addr, unsigned int str_addr)
        {
            std::map<unsigned int, SP_Entry*>::iterator sp_iter = sp_map.find(str_addr);
            if(sp_iter == sp_map.end()) {
#ifdef DEBUG    
                (*outf)<<"In get_physical_address of str_addr "<<str_addr<<", phy addr not found\n";
#endif
                return false;
            }
            else {
                if(sp_iter->second->valid) {
                    phy_addr = sp_iter->second->phy_addr;
#ifdef DEBUG    
                    (*outf)<<"In get_physical_address of str_addr "<<str_addr<<", phy addr is "<<phy_addr<<"\n";
#endif
                    return true;
                }
                else {
#ifdef DEBUG    
                    (*outf)<<"In get_physical_address of str_addr "<<str_addr<<", phy addr not valid\n";
#endif

                    return false;
                }
            }

        }
        void update(uint64_t phy_addr, unsigned int str_addr)
        {
#ifdef DEBUG    
            (*outf)<<"In off_chip_info update, phy_addr is "<<phy_addr<<", str_addr is "<<str_addr<<"\n";
#endif

            //PS Map Update
            std::map<uint64_t, PS_Entry*>::iterator ps_iter = ps_map.find(phy_addr);
            if(ps_iter == ps_map.end()) {
                PS_Entry* ps_entry = new PS_Entry();
                ps_map[phy_addr] = ps_entry;
                ps_map[phy_addr]->set(str_addr);
            }
            else {
                ps_iter->second->set(str_addr);
            }	

            //SP Map Update
            std::map<unsigned int, SP_Entry*>::iterator sp_iter = sp_map.find(str_addr);
            if(sp_iter == sp_map.end()) {
                SP_Entry* sp_entry = new SP_Entry();
                sp_map[str_addr] = sp_entry;
                sp_map[str_addr]->set(phy_addr);
            }
            else {
                sp_iter->second->set(phy_addr);
            }	

        }
        void invalidate(uint64_t phy_addr, unsigned int str_addr)
        {
#ifdef DEBUG    
            (*outf)<<"In off_chip_info invalidate, phy_addr is "<<phy_addr<<", str_addr is "<<str_addr<<"\n";
#endif
            //PS Map Invalidate
            std::map<uint64_t, PS_Entry*>::iterator ps_iter = ps_map.find(phy_addr);
            if(ps_iter != ps_map.end()) {
                ps_iter->second->reset();
                delete ps_iter->second;
                ps_map.erase(ps_iter);
            }
            else {
                //TODO TBD
            }

            //SP Map Invalidate
            std::map<unsigned int, SP_Entry*>::iterator sp_iter = sp_map.find(str_addr);
            if(sp_iter != sp_map.end()) {
                sp_iter->second->reset();
                delete sp_iter->second;
                sp_map.erase(sp_iter);
            }
            else {
                //TODO TBD
            }

        }
        void increase_confidence(uint64_t phy_addr)
        {
#ifdef DEBUG    
            (*outf)<<"In off_chip_info increase_confidence, phy_addr is "<<phy_addr<<"\n";
#endif
            std::map<uint64_t, PS_Entry*>::iterator ps_iter = ps_map.find(phy_addr);
            if(ps_iter != ps_map.end()) {
                ps_iter->second->increase_confidence();
            }
            else {
                assert(0);
            }

        }

        bool lower_confidence(uint64_t phy_addr)
        {
            bool ret = false;

#ifdef DEBUG    
            (*outf)<<"In off_chip_info lower_confidence, phy_addr is "<<phy_addr<<"\n";
#endif

            std::map<uint64_t, PS_Entry*>::iterator ps_iter = ps_map.find(phy_addr);
            if(ps_iter != ps_map.end()) {
                ret = ps_iter->second->lower_confidence();
            }
            else {
                assert(0);
            }
            return ret;

        }
        //void reassign_stream(unsigned int str_addr, unsigned int new_str_addr);
};

#endif
