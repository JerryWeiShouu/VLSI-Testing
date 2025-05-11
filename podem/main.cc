#include <iostream>
#include <ctime>
#include "circuit.h"
#include "GetLongOpt.h"
#include "ReadPattern.h"
#include <sys/types.h>
#include <unistd.h>
using namespace std;

// All defined in readcircuit.l
extern char* yytext;
extern FILE *yyin;
extern CIRCUIT Circuit;
extern int yyparse (void);
extern bool ParseError;

extern void Interactive();

GetLongOpt option;

int SetupOption(int argc, char ** argv)
{
    option.usage("[options] input_circuit_file");
    option.enroll("help", GetLongOpt::NoValue,
            "print this help summary", 0);
    option.enroll("logicsim", GetLongOpt::NoValue,
            "run logic simulation", 0);
    option.enroll("plogicsim", GetLongOpt::NoValue,
            "run parallel logic simulation", 0);
    option.enroll("fsim", GetLongOpt::NoValue,
            "run stuck-at fault simulation", 0);
    option.enroll("stfsim", GetLongOpt::NoValue,
            "run single pattern single transition-fault simulation", 0);
    option.enroll("transition", GetLongOpt::NoValue,
            "run transition-fault ATPG", 0);
    option.enroll("input", GetLongOpt::MandatoryValue,
            "set the input pattern file", 0);
    option.enroll("output", GetLongOpt::MandatoryValue,
            "set the output pattern file", 0);
    option.enroll("bt", GetLongOpt::OptionalValue,
            "set the backtrack limit", 0);
    //Assignment 0
    option.enroll("ass0", GetLongOpt::NoValue,
            "do assiment 0", 0);
    //Assignment 1
    option.enroll("path", GetLongOpt::NoValue,
            "list and count all possible paths connecting", 0);
    option.enroll("start", GetLongOpt::MandatoryValue,
            "set the input gate", 0);
    option.enroll("end", GetLongOpt::MandatoryValue,
            "set the output gate", 0);
    //Assignment 2
    option.enroll("pattern", GetLongOpt::NoValue,
            "generate the random patterns", 0);
    option.enroll("num", GetLongOpt::MandatoryValue,
            "the number of patterns", 0);
    option.enroll("unknown", GetLongOpt::NoValue,
            "generate the random patterns with unknown", 0);
    option.enroll("mod_logicsim", GetLongOpt::NoValue,
            "run logic simulation of assignment 2", 0);
    //Assignment 3
    option.enroll("simulator", GetLongOpt::MandatoryValue,
            "do simulation run", 0);
    //Assignment 4
    option.enroll("check_point", GetLongOpt::NoValue,
            "implement the fault generation function with the check point theorm", 0);
    option.enroll("bridging", GetLongOpt::NoValue,
            "create a fault list for bridging faults", 0);
    //Assignment 5
    option.enroll("bridging_fsim",GetLongOpt::NoValue,
            "run the parallel fault simulator for the bridging fault model", 0);
    //Assignment 6
    option.enroll("flist",GetLongOpt::NoValue,
            "generate fault list of net17 or n60", 0);
    option.enroll("net",GetLongOpt::MandatoryValue,
            "generate fault list of net", 0);
    option.enroll("random_pattern",GetLongOpt::NoValue,
            "run the PODEM with random pattern", 0);
    option.enroll("bridging_atpg",GetLongOpt::NoValue,
            "run bridging fault atpg", 0);
    int optind = option.parse(argc, argv);
    if ( optind < 1 ) { exit(0); }
    if ( option.retrieve("help") ) {
        option.usage();
        exit(0);
    }
    return optind;
}

int main(int argc, char ** argv)
{
    int optind = SetupOption(argc, argv);
    clock_t time_init, time_end;
    time_init = clock();
    //Setup File
    if (optind < argc) {
        if ((yyin = fopen(argv[optind], "r")) == NULL) {
            cout << "Can't open circuit file: " << argv[optind] << endl;
            exit( -1);
        }
        else {
            string circuit_name = argv[optind];
            string::size_type idx = circuit_name.rfind('/');
            if (idx != string::npos) { circuit_name = circuit_name.substr(idx+1); }
            idx = circuit_name.find(".bench");
            if (idx != string::npos) { circuit_name = circuit_name.substr(0,idx); }
            Circuit.SetName(circuit_name);
        }
    }
    else {
        cout << "Input circuit file missing" << endl;
        option.usage();
        return -1;
    }
    cout << "Start parsing input file\n";
    yyparse();
    if (ParseError) {
        cerr << "Please correct error and try Again.\n";
        return -1;
    }
    fclose(yyin);
    Circuit.FanoutList();
    Circuit.SetupIO_ID();
    Circuit.Levelize();
    Circuit.Check_Levelization();
    Circuit.InitializeQueue();

    if (option.retrieve("logicsim")) {
        //logic simulator
        Circuit.InitPattern(option.retrieve("input"));
        Circuit.LogicSimVectors();
    }
    else if (option.retrieve("plogicsim")) {
        //parallel logic simulator
        Circuit.InitPattern(option.retrieve("input"));
        Circuit.ParallelLogicSimVectors();
    }
    else if (option.retrieve("stfsim")) {
        //single pattern single transition-fault simulation
        Circuit.MarkOutputGate();
        Circuit.GenerateAllTFaultList();
        Circuit.InitPattern(option.retrieve("input"));
        Circuit.TFaultSimVectors();
    }
    else if (option.retrieve("transition")) {
        Circuit.MarkOutputGate();
        Circuit.GenerateAllTFaultList();
        Circuit.SortFaninByLevel();
        if (option.retrieve("bt")) {
            Circuit.SetBackTrackLimit(atoi(option.retrieve("bt")));
        }
        Circuit.TFAtpg();
    }
    //Assignment 0
    else if(option.retrieve("ass0"))
    {
        Circuit.GetCircuitInfo();
    }
    //Assignment 1
    else if(option.retrieve("path"))
    {
        const string start_gate = option.retrieve("start");
        const string end_gate = option.retrieve("end");

        cout << "Assignment 1: Find all paths from " << start_gate << " to " << end_gate << endl;
        //cout << start_gate << " " << end_gate << endl;
        Circuit.ListPath(start_gate, end_gate);
    }
    //Assignment 2
    else if(option.retrieve("pattern"))
    {
        if(option.retrieve("unknown"))
            Circuit.GeneratePattern(option.retrieve("output"), atoi(option.retrieve("num")), 1);
        else
            Circuit.GeneratePattern(option.retrieve("output"), atoi(option.retrieve("num")), 0);
    }
    else if(option.retrieve("mod_logicsim"))
    {
        Circuit.InitPattern(option.retrieve("input"));
        Circuit.Mod_LogicSimVectors();
    }
    //Assignment 3
    else if(option.retrieve("simulator"))
    {
        Circuit.InitPattern(option.retrieve("input"));
        Circuit.CompiledCodeSimulator(option.retrieve("simulator"));
    }
    //Assignment 4
    else if(option.retrieve("check_point"))
    {
        Circuit.CheckPoint();
    }
    else if(option.retrieve("bridging"))
    {
        Circuit.CreateBridgingFaultListOutputFile(option.retrieve("output"));
    }
    //Assignment 5
    else if(option.retrieve("bridging_fsim"))
    {
        Circuit.GenerateBridgingFaultList();
        Circuit.SortFaninByLevel();
        Circuit.MarkOutputGate();
        Circuit.GateFlagReset();
        Circuit.InitPattern(option.retrieve("input"));
        Circuit.BridgingFaultSimVectors();
    }
    //Assignment 6
    else if(option.retrieve("flist"))
    {
        Circuit.GenerateNetFaultList(option.retrieve("net"));
        Circuit.SortFaninByLevel();
        Circuit.MarkOutputGate();
        if (option.retrieve("fsim")) {
            //stuck-at fault simulator
            Circuit.InitPattern(option.retrieve("input"));
            Circuit.FaultSimVectors();
        }

        else {
            if (option.retrieve("bt")) {
                Circuit.SetBackTrackLimit(atoi(option.retrieve("bt")));
            }
            //stuck-at fualt ATPG
            Circuit.Atpg();
        }
    }
    else if(option.retrieve("random_pattern"))
    {
        Circuit.GenerateAllFaultList();
        Circuit.SortFaninByLevel();
        Circuit.MarkOutputGate();
        Circuit.RandomPatternPODEM();
    }
    else if(option.retrieve("bridging_atpg"))
    {
        Circuit.GenerateBridgingFaultList();
        Circuit.SortFaninByLevel();
        Circuit.MarkOutputGate();
        Circuit.BridgingAtpg();
    }
    else {
        Circuit.GenerateAllFaultList();
        Circuit.SortFaninByLevel();
        Circuit.MarkOutputGate();
        if (option.retrieve("fsim")) {
            //stuck-at fault simulator
            Circuit.InitPattern(option.retrieve("input"));
            Circuit.FaultSimVectors();
        }

        else {
            if (option.retrieve("bt")) {
                Circuit.SetBackTrackLimit(atoi(option.retrieve("bt")));
            }
            //stuck-at fualt ATPG
            Circuit.Atpg();
        }
    }
    time_end = clock();
    cout << "total CPU time = " << double(time_end - time_init)/CLOCKS_PER_SEC << endl;
    /*cout << endl;
    int pid=(int) getpid();
    char buf[1024];
    sprintf(buf, "cat /proc/%d/statm",pid);
    system(buf);
    system("ps aux | grep atpg");*/
    return 0;
}
