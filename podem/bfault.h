#ifndef BFAULT_H
#define BFAULT_H
#include "gate.h"

class BFAULT
{
    private:
        GATE* Gate1;
        GATE* Gate2;
        bool Type; 
        FAULT_STATUS Status;
        bool f_type;
    public:
        BFAULT(GATE* g1, GATE* g2, bool type): Gate1(g1), Gate2(g2), Type(type), Status(UNKNOWN), f_type(false) {}
        ~BFAULT() {}
        GATE* GetGate1() { return Gate1; }
        GATE* GetGate2() { return Gate2; }
        bool BridgingType() { return Type; }
        FAULT_STATUS GetStatus() { return Status; }
        void SetStatus(FAULT_STATUS status) { Status = status; }
        VALUE GetValue() {
            if(Type) return S1;
            else return S0;
        }
        GATE* GetInputGate() {
            if(!f_type){//g1=0, g2=1
                if(Type) return Gate1;
                else return Gate2;
            }
            else{//g1=1, g2=0
                if(Type) return Gate2;
                else return Gate1;
            }
        }
        GATE* GetOutputGate() {
            if(!f_type){//g1=0, g2=1
                if(Type) return Gate1;
                else return Gate2;
            }
            else{//g1=1, g2=0
                if(Type) return Gate2;
                else return Gate1;
            }
        }
        GATE* Get_lucky_gate(){
            if(!f_type){//g1=0, g2=1
                if(Type) return Gate2;
                else return Gate1;
            }
            else{//g1=1, g2=0
                if(Type) return Gate1;
                else return Gate2;
            }
        }
        void Set_ftype01(){f_type=false;return;}
        void Set_ftype02(){f_type=true;return;}
};

#endif