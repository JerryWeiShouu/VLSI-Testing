#include <iostream> 
//#include <alg.h>
#include "circuit.h"
#include "GetLongOpt.h"
using namespace std;

extern GetLongOpt option;

void CIRCUIT::FanoutList()
{
    unsigned i = 0, j;
    GATE* gptr;
    for (;i < No_Gate();i++) {
        gptr = Gate(i);
        for (j = 0;j < gptr->No_Fanin();j++) {
            gptr->Fanin(j)->AddOutput_list(gptr);
        }
    }
}

void CIRCUIT::Levelize()
{
    list<GATE*> Queue;
    GATE* gptr;
    GATE* out;
    unsigned j = 0;
    for (unsigned i = 0;i < No_PI();i++) {
        gptr = PIGate(i);
        gptr->SetLevel(0);
        for (j = 0;j < gptr->No_Fanout();j++) {
            out = gptr->Fanout(j);
            if (out->GetFunction() != G_PPI) {
                out->IncCount();
                if (out->GetCount() == out->No_Fanin()) {
                    out->SetLevel(1);
                    Queue.push_back(out);
                }
            }
        }
    }
    for (unsigned i = 0;i < No_PPI();i++) {
        gptr = PPIGate(i);
        gptr->SetLevel(0);
        for (j = 0;j < gptr->No_Fanout();j++) {
            out = gptr->Fanout(j);
            if (out->GetFunction() != G_PPI) {
                out->IncCount();
                if (out->GetCount() ==
                        out->No_Fanin()) {
                    out->SetLevel(1);
                    Queue.push_back(out);
                }
            }
        }
    }
    int l1, l2;
    while (!Queue.empty()) {
        gptr = Queue.front();
        Queue.pop_front();
        l2 = gptr->GetLevel();
        for (j = 0;j < gptr->No_Fanout();j++) {
            out = gptr->Fanout(j);
            if (out->GetFunction() != G_PPI) {
                l1 = out->GetLevel();
                if (l1 <= l2)
                    out->SetLevel(l2 + 1);
                out->IncCount();
                if (out->GetCount() ==
                        out->No_Fanin()) {
                    Queue.push_back(out);
                }
            }
        }
    }
    for (unsigned i = 0;i < No_Gate();i++) {
        Gate(i)->ResetCount();
    }
}

void CIRCUIT::Check_Levelization()
{

    GATE* gptr;
    GATE* in;
    unsigned i, j;
    for (i = 0;i < No_Gate();i++) {
        gptr = Gate(i);
        if (gptr->GetFunction() == G_PI) {
            if (gptr->GetLevel() != 0) {
                cout << "Wrong Level for PI : " <<
                gptr->GetName() << endl;
                exit( -1);
            }
        }
        else if (gptr->GetFunction() == G_PPI) {
            if (gptr->GetLevel() != 0) {
                cout << "Wrong Level for PPI : " <<
                gptr->GetName() << endl;
                exit( -1);
            }
        }
        else {
            for (j = 0;j < gptr->No_Fanin();j++) {
                in = gptr->Fanin(j);
                if (in->GetLevel() >= gptr->GetLevel()) {
                    cout << "Wrong Level for: " <<
                    gptr->GetName() << '\t' <<
                    gptr->GetID() << '\t' <<
                    gptr->GetLevel() <<
                    " with fanin " <<
                    in->GetName() << '\t' <<
                    in->GetID() << '\t' <<
                    in->GetLevel() <<
                    endl;
                }
            }
        }
    }
}

void CIRCUIT::SetMaxLevel()
{
    for (unsigned i = 0;i < No_Gate();i++) {
        if (Gate(i)->GetLevel() > MaxLevel) {
            MaxLevel = Gate(i)->GetLevel();
        }
    }
}

//Setup the Gate ID and Inversion
//Setup the list of PI PPI PO PPO
void CIRCUIT::SetupIO_ID()
{
    unsigned i = 0;
    GATE* gptr;
    vector<GATE*>::iterator Circuit_ite = Netlist.begin();
    for (; Circuit_ite != Netlist.end();Circuit_ite++, i++) {
        gptr = (*Circuit_ite);
        gptr->SetID(i);
        switch (gptr->GetFunction()) {
            case G_PI: PIlist.push_back(gptr);
                break;
            case G_PO: POlist.push_back(gptr);
                break;
            case G_PPI: PPIlist.push_back(gptr);
                break;
            case G_PPO: PPOlist.push_back(gptr);
                break;
            case G_NOT: gptr->SetInversion();
                break;
            case G_NAND: gptr->SetInversion();
                break;
            case G_NOR: gptr->SetInversion();
                break;
            default:
                break;
        }
    }
}

//Assignment 0
void CIRCUIT::GetCircuitInfo()
{
    cout << "Number of inputs: " << No_PI() << endl;
    cout << "Number of outputs: " << No_PO() << endl;
    cout << "Total number of gates: " << No_Gate() << endl;
    cout << "Number of gates for each type*******" << endl;
    string gate_type[12]={"PI","PO","PPI","PPO","NOT","AND","NAND","OR","NOR","DFF","BUF","BAD"};
    unsigned EachGateNum[12] = {0};
    unsigned Total_Fanout = 0;
    unsigned Total_Stem = 0;
    unsigned Total_Branch = 0;
    for(unsigned i=0; i<No_Gate(); i++)
    {
        EachGateNum[GATEFUNC(Gate(i)->GetFunction())]++;
        if(Gate(i)->No_Fanout() > 1)
        {
            Total_Branch += Gate(i)->No_Fanout();
            Total_Stem++;
        }
        Total_Fanout += Gate(i)->No_Fanout();
    }
    for(int i=0; i<12; i++)
    {
        if(gate_type[i] == "DFF")
            i++;
        cout << gate_type[i] << ": " << EachGateNum[i] << endl;
    }
    cout << "************************************" << endl;

    cout << "Number of flip-flops: " << EachGateNum[2] << endl;
    cout << "Total number of signal nets: " << Total_Fanout + Total_Stem << endl;
    cout << "Number of branch nets: " << Total_Branch << endl;
    cout << "Number of stem nets: " << Total_Stem << endl;
    cout << "Average number of fanouts of each gate: " << double(Total_Fanout)/No_Gate() << endl;
}

//Assignment 1
void CIRCUIT::ListPath(string start_gate, string end_gate)
{
    GATE *gate1 = nullptr;
    GATE *gate2 = nullptr;
    vector<GATE*>::iterator it_PI = PIlist.begin();
    for(; it_PI!=PIlist.end(); it_PI++)
    {
        if((*it_PI)->GetName() == start_gate)
            gate1 = *it_PI;
    }
    
    vector<GATE*>::iterator it_PO = POlist.begin();
    for(; it_PO!=POlist.end(); it_PO++)
    {
        if((*it_PO)->GetName() == end_gate)
            gate2 = *it_PO;
    }

    //cout << "Got input and output" << endl;

    bool input_error = 0;
    if(gate1 == nullptr)
    {
        cout << start_gate << " is not a primary input!!!" << endl;
        input_error = 1;
    }
    if(gate2 == nullptr)
    {
        cout << end_gate << " is not a primary output!!!" << endl;
        input_error = 1;
    }
    if(input_error)
        return;

    vector<GATE*> DFS_stack;
    int total_path = 0;
    DFS_stack.push_back(gate1);
    //cout << "Doing the DFS" << endl;
    DFSPath(gate2, DFS_stack, &total_path);

    cout << "The paths from " << start_gate << " to " << end_gate << ": " << total_path << endl;

    return;
}

void CIRCUIT::DFSPath(GATE *target_gate, vector<GATE*> &DFS_stack, int *total_path)
{
    if(DFS_stack.back() == target_gate)
    {
        //cout << "Get the path" << endl;
        PrintPath(DFS_stack);
        (*total_path)++;
    }
    else
    {
        //cout << "Current gate " << DFS_stack.back()->GetName() << endl;
        unsigned no_fanout = DFS_stack.back()->No_Fanout();
        for(unsigned i=0; i<no_fanout; i++)
        {
            DFS_stack.push_back(DFS_stack.back()->Fanout(i));
            DFSPath(target_gate, DFS_stack, total_path);
        }
    }
    
    DFS_stack.pop_back();
    //cout << "Doing pop back" << endl;
    return;
}

void CIRCUIT::PrintPath(vector<GATE*> DFS_stack)
{
    unsigned size = DFS_stack.size();
    for(unsigned i=0; i<size; i++)
    {    
        cout << (DFS_stack[i])->GetName() << " ";
    }
    cout << endl;
    return;
}