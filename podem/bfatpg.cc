#include <iostream>
#include "circuit.h"
#include "GetLongOpt.h"
#include <algorithm>   
using namespace std;

extern GetLongOpt option;

void CIRCUIT::BridgingAtpg()
{
    cout << "Run bridging fault ATPG" << endl;
    unsigned i, total_backtrack_num(0), pattern_num(0);
    ATPG_STATUS status, status2;
    BFAULT* fptr;
    list<BFAULT*>::iterator fite;

    //Prepare the output files
    ofstream OutputStrm;
    if (option.retrieve("output")){
        OutputStrm.open((char*)option.retrieve("output"),ios::out);
        if(!OutputStrm){
              cout << "Unable to open output file: "
                   << option.retrieve("output") << endl;
              cout << "Unsaved output!\n";
              exit(-1);
        }
    }

    if (option.retrieve("output")){
	    for (i = 0;i<PIlist.size();++i) {
		OutputStrm << "PI " << PIlist[i]->GetName() << " ";
	    }
	    OutputStrm << endl;
    }

    int ti=0;
    for (fite = BFlist.begin(); fite != BFlist.end();++fite){
        ti++;
        fptr = *fite;
        if (fptr->GetStatus() == DETECTED) { continue; }
        //run podem algorithm
        fptr->Set_ftype01();
        status = BridgingPodem(fptr, total_backtrack_num);
        
        if(status!=TRUE){
            fptr->Set_ftype02();
            fptr->SetStatus(UNKNOWN);
            status2 = BridgingPodem(fptr, total_backtrack_num);
           
            if(status2==TRUE )status = TRUE;
            else if(status2==FALSE)status = FALSE;
        }

        switch (status) {
            case TRUE:
                fptr->SetStatus(DETECTED);
                ++pattern_num;
                //run fault simulation for fault dropping
                for (i = 0;i < PIlist.size();++i) {
			            ScheduleFanout(PIlist[i]); 
                        if (option.retrieve("output")){ OutputStrm << PIlist[i]->GetValue();}
		        }
                if (option.retrieve("output")){ OutputStrm << endl;}
                for (i = PIlist.size();i<Netlist.size();++i) { Netlist[i]->SetValue(X); }
                LogicSim();
                BridgingFaultSim();
                break;
            case CONFLICT:
                fptr->SetStatus(REDUNDANT);
                break;
            case FALSE:
                fptr->SetStatus(ABORT);
                break;
        }

    }

    //compute fault coverage
    unsigned eqv_abort_num(0), eqv_redundant_num(0), eqv_detected_num(0);
    for (fite = BFlist.begin();fite!=BFlist.end();++fite) {
        fptr = *fite;
        switch (fptr->GetStatus()) {
            case DETECTED:
                ++eqv_detected_num;
                break;
            case REDUNDANT:
                ++eqv_redundant_num;
                break;
            case ABORT:
                ++eqv_abort_num;
                break;
            default:
                cerr << "Unknown fault type exists" << endl;
                break;
        }
    }
    cout.setf(ios::fixed);
    cout.precision(2);
    cout << "---------------------------------------" << endl;
    cout << "Test pattern number = " << pattern_num << endl;
    cout << "Total backtrack number = " << total_backtrack_num << endl;
    cout << "---------------------------------------" << endl;
    cout << "Total bridging fault number = " << BFlist.size() << endl;
    cout << "Detected fault number = " << eqv_detected_num << endl;
    cout << "Undetected fault number = " << eqv_abort_num + eqv_redundant_num << endl;
    cout << "Abort fault number = " << eqv_abort_num << endl;
    cout << "Redundant fault number = " << eqv_redundant_num << endl;
    cout << "---------------------------------------" << endl;
    cout << "Fault Coverge = " << 100*eqv_detected_num/double(BFlist.size()) << "%" << endl;
    cout << "Fault Efficiency = " << 100*eqv_detected_num/double(BFlist.size() - eqv_redundant_num) << "%" << endl;
    cout << "---------------------------------------" << endl;
    return;
}


ATPG_STATUS CIRCUIT::BridgingPodem(BFAULT* fptr, unsigned &total_backtrack_num)
{
    unsigned i, backtrack_num(0);
    GATEPTR pi_gptr(0), decision_gptr(0);
    ATPG_STATUS status;

    //set all values as unknown
    for (i = 0;i<Netlist.size();++i) { Netlist[i]->SetValue(X); }
    //mark propagate paths
    MarkPropagateTree(fptr->GetOutputGate());
    //propagate fault free value
    status = BridgingSetUniqueImpliedValue(fptr);

    switch (status){
        case TRUE:
            LogicSim();
            
            //inject faulty value
            if (BridgingFaultEvaluate(fptr)) {
                //forward implication
                ScheduleFanout(fptr->GetOutputGate());
                LogicSim();
            }
            //check if the fault has propagated to PO
            
            if (!CheckTest()) { status = FALSE; }
            break;
        case CONFLICT:
            status = CONFLICT;
            break;
        case FALSE: break;
    }

    while(backtrack_num < BackTrackLimit && status == FALSE){
        //search possible PI decision
        pi_gptr = BridgingTestPossible(fptr);
        if (pi_gptr) { //decision found
            ScheduleFanout(pi_gptr);
            //push to decision tree
            GateStack.push_back(pi_gptr);
            decision_gptr = pi_gptr;
        }
        else { //backtrack previous decision
            while (!GateStack.empty() && !pi_gptr) {
                //all decision tried (1 and 0)
                if (decision_gptr->GetFlag(ALL_ASSIGNED)) {
                    decision_gptr->ResetFlag(ALL_ASSIGNED);
                    decision_gptr->SetValue(X);
                    ScheduleFanout(decision_gptr);
                    //remove decision from decision tree
                    GateStack.pop_back();
                    decision_gptr = GateStack.back();
                }
                //inverse current decision value
                else {
                    decision_gptr->InverseValue();
                    ScheduleFanout(decision_gptr);
                    decision_gptr->SetFlag(ALL_ASSIGNED);
                    ++backtrack_num;
                    pi_gptr = decision_gptr;
                }
            }
            //no other decision
            if (!pi_gptr) { status = CONFLICT; }
        }
        if (pi_gptr) {
            LogicSim();
            //fault injection
            if(BridgingFaultEvaluate(fptr)) {
                //forward implication
                ScheduleFanout(fptr->GetOutputGate());
                LogicSim();
            }
            if (CheckTest()) { status = TRUE; }
        }
    } //end while loop

    //clean ALL_ASSIGNED and MARKED flags
    list<GATEPTR>::iterator gite;
    for (gite = GateStack.begin();gite != GateStack.end();++gite) {
        (*gite)->ResetFlag(ALL_ASSIGNED);
    }
    for (gite = PropagateTree.begin();gite != PropagateTree.end();++gite) {
        (*gite)->ResetFlag(MARKED);
    }

    //clear stacks
    GateStack.clear(); PropagateTree.clear();

    //assign true values to PIs
    if (status ==  TRUE) {
		for (i = 0;i<PIlist.size();++i) {
		    switch (PIlist[i]->GetValue()) {
			case S1: break;
			case S0: break;
			case D: PIlist[i]->SetValue(S1); break;
			case B: PIlist[i]->SetValue(S0); break;
			case X: PIlist[i]->SetValue(VALUE(2.0 * rand()/(RAND_MAX + 1.0))); break;
			default: cerr << "Illigal value" << endl; break;
		    }
		}//end for all PI
    } //end status == TRUE

    total_backtrack_num += backtrack_num;
    return status;
}


ATPG_STATUS CIRCUIT::BridgingSetUniqueImpliedValue(BFAULT* fptr)
{
    //register ATPG_STATUS status(FALSE);
    GATEPTR igptr(fptr->GetInputGate());
    register ATPG_STATUS status1(FALSE),status2(FALSE);
    //backward implication fault-free value
    status1 = BackwardImply(igptr, NotTable[fptr->GetValue()]);
    status2 = BackwardImply(fptr->Get_lucky_gate(), fptr->GetValue());

    if(status1==CONFLICT || status2==CONFLICT) 
        return CONFLICT;
    if(status1==FALSE || status2==FALSE) 
        return FALSE;
    
    return TRUE; 
}

bool CIRCUIT::BridgingFaultEvaluate(BFAULT* fptr)
{
    GATEPTR igptr(fptr->GetInputGate());
    //store input value
    VALUE ivalue(igptr->GetValue());
    //can not do fault injection
    if (ivalue == fptr->Get_lucky_gate()->GetValue()) { return false; }
    if (ivalue == X || ivalue == fptr->GetValue())    { return false; }
    else if (ivalue == S1) { igptr->SetValue(D); }
    else if (ivalue == S0) { igptr->SetValue(B); }
    else {return false; } //fault has been injected

    return true; 
}

GATEPTR CIRCUIT::BridgingTestPossible(BFAULT* fptr)
{
    GATEPTR decision_gptr;
    GATEPTR ogptr(fptr->GetOutputGate());
    VALUE decision_value;
    if (!ogptr->GetFlag(OUTPUT)) {
        if (ogptr->GetValue() != X && fptr->Get_lucky_gate()->GetValue() != X) {
            //if no fault injected, return 0
            if (ogptr->GetValue() != B && ogptr->GetValue() != D) { return 0; }
            //search D-frontier
            decision_gptr = FindPropagateGate();
            if (!decision_gptr) { return 0;}
            switch (decision_gptr->GetFunction()) {
                case G_AND:
                case G_NOR: decision_value = S1; break;
                case G_NAND:
                case G_OR: decision_value = S0; break;
                default: return 0;
            }
        }
        else if(fptr->Get_lucky_gate()->GetValue() != fptr->GetValue()) {
            decision_value = fptr->GetValue();
            decision_gptr = fptr->Get_lucky_gate();
        }
        else { //output gate == X
            //test if any unknown path can propagate the fault
            if (!TraceUnknownPath(ogptr)) { return 0; }
            if (1) { //stem
                decision_value = NotTable[fptr->GetValue()];
                decision_gptr = ogptr;
            }
        } //end output gate == X
    } //end if output gate is PO
    else { //reach PO
        if (fptr->GetInputGate()->GetValue() == X) {
            decision_value = NotTable[fptr->GetValue()];
            decision_gptr = fptr->GetInputGate();
        }
        else { return 0; }
    }
    return FindPIAssignment(decision_gptr, decision_value);
}