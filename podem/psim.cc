#include <iostream>
#include "circuit.h"
#include "GetLongOpt.h"
using namespace std;

extern GetLongOpt option;

// Event-driven Parallel Pattern Logic simulation
void CIRCUIT::ParallelLogicSimVectors()
{
    cout << "Run Parallel Logic simulation" << endl;
    unsigned pattern_num(0);
    unsigned pattern_idx(0);
    while(!Pattern.eof()){ 
	for(pattern_idx=0; pattern_idx<PatternNum; pattern_idx++){
	    if(!Pattern.eof()){ 
	        ++pattern_num;
	        Pattern.ReadNextPattern(pattern_idx);
	    }
	    else break;
	}
	ScheduleAllPIs();
	ParallelLogicSim();
	//PrintParallelIOs(pattern_idx);
    }
}

//Assign next input pattern to PI's idx'th bits
void PATTERN::ReadNextPattern(unsigned idx)
{
    char V;
    for (int i = 0; i < no_pi_infile; i++) {
        patterninput >> V;
        if (V == '0') {
            inlist[i]->ResetWireValue(0, idx);
            inlist[i]->ResetWireValue(1, idx);
        }
        else if (V == '1') {
            inlist[i]->SetWireValue(0, idx);
            inlist[i]->SetWireValue(1, idx);
        }
        else if (V == 'X') {
            inlist[i]->SetWireValue(0, idx);
            inlist[i]->ResetWireValue(1, idx);
        }
    }
    //Take care of newline to force eof() function correctly
    patterninput >> V;
    if (!patterninput.eof()) patterninput.unget();
    return;
}

//Simulate PatternNum vectors
void CIRCUIT::ParallelLogicSim()
{
    GATE* gptr;
    for (unsigned i = 0;i <= MaxLevel;i++) {
        while (!Queue[i].empty()) {
            gptr = Queue[i].front();
            Queue[i].pop_front();
            gptr->ResetFlag(SCHEDULED);
            ParallelEvaluate(gptr);
        }
    }
    return;
}

//Evaluate parallel value of gptr
void CIRCUIT::ParallelEvaluate(GATEPTR gptr)
{
    register unsigned i;
    bitset<PatternNum> new_value1(gptr->Fanin(0)->GetValue1());
    bitset<PatternNum> new_value2(gptr->Fanin(0)->GetValue2());
    switch(gptr->GetFunction()) {
        case G_AND:
        case G_NAND:
            for (i = 1; i < gptr->No_Fanin(); ++i) {
                new_value1 &= gptr->Fanin(i)->GetValue1();
                new_value2 &= gptr->Fanin(i)->GetValue2();
            }
            break;
        case G_OR:
        case G_NOR:
            for (i = 1; i < gptr->No_Fanin(); ++i) {
                new_value1 |= gptr->Fanin(i)->GetValue1();
                new_value2 |= gptr->Fanin(i)->GetValue2();
            }
            break;
        default: break;
    } 
    //swap new_value1 and new_value2 to avoid unknown value masked
    if (gptr->Is_Inversion()) {
        new_value1.flip(); new_value2.flip();
        bitset<PatternNum> value(new_value1);
        new_value1 = new_value2; new_value2 = value;
    }
    if (gptr->GetValue1() != new_value1 || gptr->GetValue2() != new_value2) {
        gptr->SetValue1(new_value1);
        gptr->SetValue2(new_value2);
        ScheduleFanout(gptr);
    }
    return;
}

void CIRCUIT::PrintParallelIOs(unsigned idx)
{
    register unsigned i;
    for (unsigned j=0; j<idx; j++){
	    for (i = 0;i<No_PI();++i) { 
		    if(PIGate(i)->GetWireValue(0, j)==0){ 
			   if(PIGate(i)->GetWireValue(1, j)==1){
	    			cout << "F";
			   }
			   else cout << "0";
		    }
		    else{
			   if(PIGate(i)->GetWireValue(1, j)==1){
	    			cout << "1";
			   }
			   else cout << "X";
		    }

	    }
	    cout << " ";
	    for (i = 0;i<No_PO();++i) { 
		    if(POGate(i)->GetWireValue(0, j)==0){ 
			   if(POGate(i)->GetWireValue(1, j)==1){
	    			cout << "F";
			   }
			   else cout << "0";
		    }
		    else{
			   if(POGate(i)->GetWireValue(1, j)==1){
	    			cout << "1";
			   }
			   else cout << "X";
		    }
	    }
	    cout << endl;
    }
    return;
}

void CIRCUIT::ScheduleAllPIs()
{
    for (unsigned i = 0;i < No_PI();i++) {
        ScheduleFanout(PIGate(i));
    }
    return;
}

//Assignment 3
void CIRCUIT::CompiledCodeSimulator(const char* cc_file)
{
    cout << "Generating " << cc_file << " file" << endl;

    ofstream ofile(cc_file,ios::out);
    ofile << "#include <iostream>"  << endl;
    ofile << "#include <ctime>"     << endl;
    ofile << "#include <bitset>"    << endl;
    ofile << "#include <string>"    << endl;
    ofile << "#include <fstream>"   << endl;
    ofile << "using namespace std;" << endl;
    ofile << "const unsigned PatternNum = 16;" << endl;
    ofile << "void evaluate();"     << endl;
    ofile << "void printIO(unsigned idx);" << endl;


    for(unsigned j=0; j<No_Gate(); j++)
        ofile << "bitset<PatternNum> G_" << Gate(j)->GetName() << "[2];" << endl;
    
    ofile << "bitset<PatternNum> temp;" << endl;
    ofile << "ofstream fout(" << '"' << GetName() << ".out" << '"' << ",ios::out);" << endl;
    ofile << "int main()" << endl;
    ofile << "{" << endl;
    ofile << "clock_t time_init, time_end;" << endl;
    ofile << "time_init = clock();" << endl; 


    unsigned pattern_num(0);
    unsigned pattern_idx(0);
    while(!Pattern.eof())
    { 
        for(pattern_idx=0; pattern_idx<PatternNum; pattern_idx++)
        {
            if(!Pattern.eof())
            { 
                ++pattern_num;
                Pattern.ReadNextPattern(pattern_idx);
            }
            else 
                break;
        }

        for(unsigned j = 0;j<No_PI();j++)
        {
            ofile << "G_"<<PIGate(j)->GetName() << "[0]=" << PIGate(j)->GetValue1().to_ulong() << ";"<<endl;
            ofile << "G_"<<PIGate(j)->GetName() << "[1]=" << PIGate(j)->GetValue2().to_ulong() << ";"<<endl;
        }
        ofile << endl;
        ofile << "evaluate();" << endl;
        ofile << "printIO(" << pattern_idx << ");" << endl;
    }

    ofile << "time_end = clock();" << endl;
    ofile << "cout << " << '"' << "Total CPU Time = " << '"' << " << double(time_end - time_init)/CLOCKS_PER_SEC << endl;" << endl;
    ofile << "//system(" << '"' <<"ps aux | grep a.out " << '"' << ");" << endl;
    ofile << "return 0;" << endl;
    ofile << "}" << endl;
    ofile << "void evaluate()" << endl;
    ofile << "{" << endl;

    ScheduleAllPIs();
    GATE* gptr;
    for(unsigned i=0; i<=MaxLevel; i++) 
    {
        while (!Queue[i].empty()) 
        {
            gptr = Queue[i].front();
            Queue[i].pop_front();
            gptr->ResetFlag(SCHEDULED);
            ofile << "G_" << gptr -> GetName() << "[0] = G_" << gptr->Fanin(0)->GetName() << "[0]" << ";" << endl;
            ofile << "G_" << gptr -> GetName() << "[1] = G_" << gptr->Fanin(0)->GetName() << "[1]" << ";" << endl;
            switch(gptr->GetFunction())
            {
                case G_AND:
                case G_NAND:
                    for (unsigned j = 1; j < gptr->No_Fanin(); ++j) {
                        ofile << "G_" << gptr -> GetName() << "[0] &= G_"<<gptr->Fanin(j)->GetName() << "[0]" << ";" << endl;
                        ofile << "G_" << gptr -> GetName() << "[1] &= G_"<<gptr->Fanin(j)->GetName() << "[1]" << ";" << endl;
                    }
                    break;
                case G_OR:
                case G_NOR:
                    for (unsigned j = 1; j < gptr->No_Fanin(); ++j){
                        ofile << "G_" << gptr -> GetName() << "[0] |= G_" << gptr->Fanin(j)->GetName() << "[0]" << ";" << endl;
                        ofile << "G_" << gptr -> GetName() << "[1] |= G_" << gptr->Fanin(j)->GetName() << "[1]" << ";" << endl;
                    }
                    break;
                default:
                    break;
                }
            if (gptr->Is_Inversion()) 
            {
                ofile << "temp = G_" << gptr -> GetName() << "[0] ;" << endl;
                ofile << "G_" << gptr -> GetName() << "[0] = ~G_" << gptr -> GetName() << "[1] ;" << endl;
                ofile << "G_" << gptr -> GetName() << "[1] = ~temp ;" << endl;
            }
           ScheduleFanout(gptr);
        }
    }

    ofile << "}" << endl;
    ofile << "void printIO(unsigned idx)" << endl;
    ofile << "{" << endl;
    ofile << "for (unsigned j=0; j<idx; j++)" << endl;
    ofile << "{" << endl;

    for(unsigned k=0; k<No_PI(); k++)
    {
        ofile << "if(G_" << PIGate(k)->GetName() << "[0][j]==0)" << endl << "{" << endl;
        ofile << "if(G_" << PIGate(k)->GetName() << "[1][j]==1)" << endl << "fout << " << '"' << "F" << '"' << ";" << endl;
        ofile << "else" << endl;
        ofile << "fout << " << '"' << "0" << '"' << ";" << endl;
        ofile << "}" << endl;
        ofile << "else" << endl;
        ofile << "{" << endl;
        ofile << "if(G_" << PIGate(k)->GetName() << "[1][j]==1)" << endl;
        ofile << "fout << " << '"' << "1" << '"' << ";" << endl;
        ofile << "else" << endl;
        ofile << "fout << " << '"' << "2" << '"' << ";" << endl;
        ofile << "}" << endl;
    }

    ofile << "fout << " << '"' << " " << '"' << ";" << endl;

    for(unsigned k=0; k<No_PO(); k++)
    {
        ofile << "if(G_" << POGate(k)->GetName() << "[0][j]==0)" << endl;
        ofile << "{" << endl;
        ofile << "if(G_" << POGate(k)->GetName() << "[1][j]==1)" << endl;
        ofile << "fout << " << '"' << "F" << '"' << ";" << endl;
        ofile << "else" << endl;
        ofile << "fout << " << '"' << "0" << '"' << ";" << endl;
        ofile << "}" << endl;
        ofile << "else" << endl;
        ofile << "{" << endl;
        ofile << "if(G_" << POGate(k)->GetName() << "[1][j]==1)" << endl;
        ofile << "fout << " << '"' << "1" << '"' << ";" << endl;
        ofile << "else" << endl;
        ofile << "fout << " << '"' << "2" << '"' << ";" << endl;
        ofile << "}" << endl;
    }

    ofile << "fout << endl;" << endl;
    ofile << "}" << endl;
    ofile << "}" << endl;
    ofile.close();
    return;
}