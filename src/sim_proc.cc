#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sim_proc.h"

/*  argc holds the number of command line arguments
    argv[] holds the commands themselves

    Example:-
    sim 256 32 4 gcc_trace.txt
    argc = 5
    argv[0] = "sim"
    argv[1] = "256"
    argv[2] = "32"
    ... and so on
*/
int main (int argc, char* argv[])
{
    FILE *FP;               // File handler
    char *trace_file;       // Variable that holds trace file name;
    proc_params params;       // look at sim_bp.h header file for the the definition of struct proc_params
    int op_type, dest, src1, src2;  // Variables are read from trace file
    unsigned long int pc; // Variable holds the pc read from input file
    
    if (argc != 5)
    {
        printf("Error: Wrong number of inputs:%d\n", argc-1);
        exit(EXIT_FAILURE);
    }
    
    params.rob_size     = strtoul(argv[1], NULL, 10);
    params.iq_size      = strtoul(argv[2], NULL, 10);
    params.width        = strtoul(argv[3], NULL, 10);
    trace_file          = argv[4];
    
    printf("rob_size:%lu "
            "iq_size:%lu "
            "width:%lu "
            "tracefile:%s\n", params.rob_size, params.iq_size, params.width, trace_file);
    // Open trace_file in read mode
    FP = fopen(trace_file, "r");
    if(FP == NULL)
    {
        // Throw error and exit if fopen() failed
        printf("Error: Unable to open file %s\n", trace_file);
        exit(EXIT_FAILURE);
    }
    


    /* Initializing the basic structure of the microprocessor */
    Rename_Map_Table_Operator Rename_Map_Table(67);
    IssueQueue_Operator IQ_controller(params.iq_size);
    ROB_Operator ROB_controller(params.rob_size);

    Pipeline_Stage_Operator FE(params.width, 0);
    Pipeline_Stage_Operator DE(params.width, 1);
    Pipeline_Stage_Operator RN(params.width, 2);
    Pipeline_Stage_Operator RR(params.width, 3);
    Pipeline_Stage_Operator DI(params.width, 4);
    Pipeline_Stage_Operator IS(params.width, 5);
    Pipeline_Stage_Operator EX(params.width * 5, 6);
    Pipeline_Stage_Operator WB(params.width * 5, 7);
    Pipeline_Stage_Operator RT(params.width, 8);
    /* Completed initializing the basic structure of the microprocessor */





    /* Initialization simulation environment */
    unsigned int sim_time = 0;
    bool continue_sim = 1, fetched_all_instructions = 0, empty_pipeline = 1;
    /* Completed Initialization simulation environment */





    ///////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // The following loop just tests reading the trace and echoing it back to the screen.
    //
    // Replace this loop with the "do { } while (Advance_Cycle());" loop indicated in the Project 3 spec.
    // Note: fscanf() calls -- to obtain a fetch bundle worth of instructions from the trace -- should be
    // inside the Fetch() function.
    //
    ///////////////////////////////////////////////////////////////////////////////////////////////////////
    do{
        /******************************************* Retire Stage ******************************************/
        
        /**************************************** End Retire Stage *****************************************/


        /***************************************** Writeback Stage *****************************************/

        /************************************** End Writeback Stage ****************************************/


        /****************************************** Execute Stage ******************************************/

        /*************************************** End Execute Stage *****************************************/


        /******************************************** Issue Stage ******************************************/

        /***************************************** End Issue Stage *****************************************/


        /****************************************** Dispatch Stage *****************************************/

        /************************************** End Dispatch Stage *****************************************/


        /****************************************** Regread Stage ******************************************/

        /*************************************** End Regread Stage *****************************************/


        /******************************************* Rename Stage ******************************************/

        /**************************************** End Rename Stage *****************************************/


        /******************************************* Decode Stage ******************************************/

        /**************************************** End Decode Stage *****************************************/


        /******************************************* Fetch Stage *******************************************/
        if (fscanf(FP, "%lx %d %d %d %d", &pc, &op_type, &dest, &src1, &src2) != EOF)
        {
            printf("%lx %d %d %d %d\n", pc, op_type, dest, src1, src2); //Print to check if inputs have been read correctly
            // Need to add to the fetch stage
        }
        else
        {
            fetched_all_instructions = 1;
        }
        /**************************************** End Fetch Stage ******************************************/





        /************************************ Advance cycle calculation ************************************/
        // Increments the simulation time
        ++sim_time;
        // Check if simulation is completed
        if ((empty_pipeline == 1) && (fetched_all_instructions == 1))
        {
            continue_sim = 0;
        }
        /********************************* End Advance cycle calculation ***********************************/
    }while(continue_sim);
    



    /******************************************* Final Output *********************************************/
    printf("\n# === Simulator Command =========");
    printf("\n# ./sim %lu %lu %lu %s", params.rob_size, params.iq_size, params.width, trace_file);
    printf("\n# === Processor Configuration ===");
    printf("\n# ROB_SIZE = %lu", params.rob_size);
    printf("\n# IQ_SIZE  = %lu", params.iq_size);
    printf("\n# WIDTH    = %lu", params.width);
    printf("\n# === Simulation Results ========");
    printf("\n# Dynamic Instruction Count    = ");
    printf("\n# Cycles                       = ");
    printf("\n# Instructions Per Cycle (IPC) = ");
    printf("\n");
    /**************************************** Final Output Ends *******************************************/
    return 0;
}
