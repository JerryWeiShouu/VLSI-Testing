/* stuck-at bridging fault simulator for combinational circuit */
#include <iostream>
#include "circuit.h"
#include "GetLongOpt.h"
using namespace std;

extern GetLongOpt option;

void CIRCUIT::GateFlagReset()
{
    for(unsigned i=0; i<No_Gate(); i=i+1)
        Gate(i)->ResetFlag(SCHEDULED);
}

void CIRCUIT::BridgingFaultSim()
{
    register unsigned i, fault_idx(0);
    GATEPTR gptr;
    BFAULT *fptr;
    BFAULT* simulate_flist[PatternNum];
    list<GATEPTR>::iterator gite;

    for(i=0; i<Netlist.size(); i++)
        Netlist[i]->SetFaultFreeValue();
    
    list<BFAULT*>::iterator fite = UBFlist.begin();
    for(; fite!=UBFlist.end(); fite++)
    {
        fptr = *fite;

        if(fptr->GetStatus() == REDUNDANT || fptr->GetStatus() == DETECTED) 
            continue;
        if (fptr->GetGate1()->GetValue() == fptr->GetGate2()->GetValue()) 
            continue;
        
        VALUE fv;
        if(fptr->BridgingType() == 1)
        {
            fv = S1;
            if(fptr->GetGate1()->GetValue() == S0)
                gptr = fptr->GetGate1();
            else
                gptr =fptr->GetGate2();
        }
        else {
            fv = S0;
            if(fptr->GetGate1()->GetValue() == S1)
                gptr = fptr->GetGate1();
            else
                gptr =fptr->GetGate2();
        }
        
        if (gptr->GetFlag(OUTPUT)){
            fptr->SetStatus(DETECTED);
            continue;
        }
        
        if (!gptr->GetFlag(FAULTY)) {
                gptr->SetFlag(FAULTY); GateStack.push_front(gptr);
        }
        InjectFaultValue(gptr, fault_idx, fv);
        gptr->SetFlag(FAULT_INJECTED);
        ScheduleFanout(gptr);
        simulate_flist[fault_idx++] = fptr;

        if (fault_idx == PatternNum){
            //do parallel fault simulation
            for (i = 0;i<= MaxLevel; ++i) {
                while (!Queue[i].empty()) {
                    gptr = Queue[i].front(); Queue[i].pop_front();
                    gptr->ResetFlag(SCHEDULED);
                    FaultSimEvaluate(gptr);
                }
            }

            // check detection and reset wires' faulty values
            // back to fault-free values
            for (gite = GateStack.begin();gite != GateStack.end();++gite) {
                gptr = *gite;
                //clean flags
                gptr->ResetFlag(FAULTY);
                gptr->ResetFlag(FAULT_INJECTED);
                gptr->ResetFaultFlag();
                if (gptr->GetFlag(OUTPUT)) {
                    for (i = 0; i < fault_idx; ++i) {
                        if (simulate_flist[i]->GetStatus() == DETECTED) { continue; }
                        //faulty value != fault-free value && fault-free != X &&
                        //faulty value != X (WireValue1[i] == WireValue2[i])
                        if (gptr->GetValue() != VALUE(gptr->GetValue1(i)) && gptr->GetValue() != X 
                                && gptr->GetValue1(i) == gptr->GetValue2(i)) {
                            simulate_flist[i]->SetStatus(DETECTED);
                        }
                    }
                }
                gptr->SetFaultFreeValue();    
            } //end for GateStack
            GateStack.clear();
            fault_idx = 0;
        }
    }

    if (fault_idx) {
        //do parallel fault simulation
        for (i = 0;i<= MaxLevel; ++i) {
            while (!Queue[i].empty()) {
                gptr = Queue[i].front(); Queue[i].pop_front();
                gptr->ResetFlag(SCHEDULED);
                FaultSimEvaluate(gptr);
            }
        }

        // check detection and reset wires' faulty values
        // back to fault-free values
        for (gite = GateStack.begin();gite != GateStack.end();++gite) {
            gptr = *gite;
            //clean flags
            gptr->ResetFlag(FAULTY);
            gptr->ResetFlag(FAULT_INJECTED);
            gptr->ResetFaultFlag();
            if (gptr->GetFlag(OUTPUT)) {
                for (i = 0; i < fault_idx; ++i) {
                    if (simulate_flist[i]->GetStatus() == DETECTED) { continue; }
                    //faulty value != fault-free value && fault-free != X &&
                    //faulty value != X (WireValue1[i] == WireValue2[i])
                    if (gptr->GetValue() != VALUE(gptr->GetValue1(i)) && gptr->GetValue() != X 
                            && gptr->GetValue1(i) == gptr->GetValue2(i)) {
                        simulate_flist[i]->SetStatus(DETECTED);
                    }
                }
            }
            gptr->SetFaultFreeValue();    
        } //end for GateStack
        GateStack.clear();
        fault_idx = 0;
    } //end fault simulation

    // remove detected faults
    for (fite = UBFlist.begin();fite != UBFlist.end();) {
        fptr = *fite;
        if (fptr->GetStatus() == DETECTED || fptr->GetStatus() == REDUNDANT) {
            fite = UBFlist.erase(fite);
        }
        else { ++fite; }
    }
}

void CIRCUIT::BridgingFaultSimVectors()
{
    cout << "Run bridging fault simulation" << endl;
    unsigned pattern_num(0);
    if(!Pattern.eof()){ // Readin the first vector
        while(!Pattern.eof()){
            ++pattern_num;
            Pattern.ReadNextPattern();
            //fault-free simulation
            SchedulePI();
            LogicSim();
            //single pattern parallel fault simulation
            BridgingFaultSim();
        }
    }


    //compute fault coverage
    unsigned total_num(0);
    unsigned undetected_num(0), detected_num(0);
    //unsigned eqv_undetected_num(0), eqv_detected_num(0);
    BFAULT* fptr;
    list<BFAULT*>::iterator fite;
    for (fite = BFlist.begin();fite!=BFlist.end();++fite) {
        fptr = *fite;
        switch (fptr->GetStatus()) {
            case DETECTED:
                detected_num += 1;
                break;
            default:
                undetected_num += 1;
                break;
        }
    }

    total_num = detected_num + undetected_num;
    cout.setf(ios::fixed);
    cout.precision(2);
    cout << "---------------------------------------" << endl;
    cout << "Test pattern number = " << pattern_num << endl;
    cout << "---------------------------------------" << endl;
    cout << "Total bridging fault number = " << total_num << endl;
    cout << "Detected bridging fault number = " << detected_num << endl;
    cout << "Undetected bridging fault number = " << undetected_num << endl;
    cout << "---------------------------------------" << endl;
    cout << "Fault Coverge = " << 100*detected_num/double(total_num) << "%" << endl;
    cout << "---------------------------------------" << endl;
    return;
}