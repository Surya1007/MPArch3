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

    std::vector<Instruction_Structure> to_FE;
    std::vector<Instruction_Structure> to_DE;
    std::vector<Instruction_Structure> to_RN;
    std::vector<Instruction_Structure> to_RR;
    std::vector<Instruction_Structure> to_DI;
    std::vector<Instruction_Structure> to_IS;
    std::vector<Instruction_Structure> to_EX;
    std::vector<Instruction_Structure> to_WB;
    std::vector<Instruction_Structure> to_RT;
    /* Completed initializing the basic structure of the microprocessor */





    /* Initialization simulation environment */


    // Instruction Trace file number
    unsigned int trace_no = 0;
    // Simulation time
    unsigned int sim_time = 0;
    // Counts the total number of retired instructions
    unsigned int total_no_retired_instructions = 0;
    // Deciding Variable to let the program know if all instructions are completed
    bool continue_sim = 1;
    // Fetched all instructions from the file
    bool fetched_all_instructions = 0;
    // There is no instruction in all stages of the pipeline
    bool empty_pipeline = 1;

    // Variable to stall the previous pipeline stage
    bool stall_FE = 0;
    bool stall_DE = 0;
    bool stall_RN = 0;
    bool stall_RR = 0;
    bool stall_DI = 0;
    bool stall_IS = 0;
    bool stall_EX = 0;
    bool stall_WB = 0;
    
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
        std::cout << "Sim time: " << sim_time << std::endl;
        /******************************************* Retire Stage ******************************************/
        // Get instruction from previous stage
        to_RT = WB.Get_and_Remove_Instructions_from_Register();
        // Add instruction from the pervious stage
        stall_WB = RT.Add_Instructions_to_Register(to_RT, sim_time);

        // Change the ROB tail pointer
        /**************************************** End Retire Stage *****************************************/


        /***************************************** Writeback Stage *****************************************/
        //std::cout << "WB status: " <<  stall_WB << std::endl;
        if (stall_WB != 1)
        {
            // Get instruction from previous stage
            to_WB = EX.Get_and_Remove_Instructions_from_Register();
            // Add instruction from the pervious stage
            stall_EX = WB.Add_Instructions_to_Register(to_WB, sim_time);     
            // Mark the ready bits in ROB       
        }
        else
        {
            stall_EX = 0;
        }
        /************************************** End Writeback Stage ****************************************/


        /****************************************** Execute Stage ******************************************/
        //std::cout << "EX status: " <<  stall_EX << std::endl;
        if (stall_EX != 1)
        {
            // Get instruction from previous stage
            to_EX = IS.Get_and_Remove_Instructions_from_Register();
            // Add instruction from the pervious stage
            stall_IS = EX.Add_Instructions_to_Register(to_EX, sim_time);
            // May need to modify the structure

            // If instruction completed execution, remove from execute_list / execute stage
            // Add instruction to writeback stage
            // Wakeup dependent instructions in IQ, DI, RR
        }
        else
        {
            stall_IS = 0;
        }
        /*************************************** End Execute Stage *****************************************/


        /******************************************** Issue Stage ******************************************/
        //std::cout << "IS status: " <<  stall_IS << std::endl;
        if (stall_IS != 1)
        {
            // Get instruction from previous stage
            to_IS = DI.Get_and_Remove_Instructions_from_Register();
            // Add instruction from the pervious stage
            stall_DI = IS.Add_Instructions_to_Register(to_IS, sim_time);

            // Register for issue stage maynot be required
            // Older instructions have higher priority
                // Remove instruction from IQ
                // Add instruction to the execute_list, or execute stage
            // Include bypass here
        }
        else
        {
            stall_DI = 0;
        }
        // Another possibility for a bypass
        /***************************************** End Issue Stage *****************************************/


        /****************************************** Dispatch Stage *****************************************/
        //std::cout << "DI status: " <<  stall_DI << std::endl;
        if ((stall_DI != 1) && (DI.Get_Status_of_Pipeline() == params.width)) // Also check the issuequeue
        {
            // Get instruction from previous stage
            RR.Print_Instructions_in_Register();
            to_DI = RR.Get_and_Remove_Instructions_from_Register();
            // Add instruction from the pervious stage
            stall_RR = DI.Add_Instructions_to_Register(to_DI, sim_time);
            // Add instruction to issuequeue
            // Include bypass here
        }
        else
        {
            stall_RR = 1;
        }
        // Another possibility for a bypass
        /************************************** End Dispatch Stage *****************************************/


        /****************************************** Regread Stage ******************************************/
        //std::cout << "RR status: " <<  stall_RR << std::endl;
        if ((stall_RR != 1) && (RR.Get_Status_of_Pipeline() == params.width))
        {
            // Get instruction from previous stage
            to_RR = RN.Get_and_Remove_Instructions_from_Register();
            // Add instruction from the pervious stage
            stall_RN = RR.Add_Instructions_to_Register(to_RR, sim_time);
            // Need to include a bypass here

        }
        else
        {
           stall_RN = 1;
        }
        // Another possibility for a bypass
        /*************************************** End Regread Stage *****************************************/


        /******************************************* Rename Stage ******************************************/
        //std::cout << "RN status: " <<  stall_RN << std::endl;
        if ((stall_RN != 1) && (RN.Get_Status_of_Pipeline() == params.width)) // Also need to add the condition if rob is full or not
        {
            // Get instruction from previous stage
            to_RN = DE.Get_and_Remove_Instructions_from_Register();

            // Allocate an entry in ROB for instructions in order
            
            // Add the renamed source registers to srcs1, srcs2
            // Add the renamed destination register to dests

            // Add instruction from the pervious stage
            stall_DE = RN.Add_Instructions_to_Register(to_RN, sim_time);
            // Add the modified register data

        }
        else
        {
            stall_DE = 1;
        }
        /**************************************** End Rename Stage *****************************************/


        /******************************************* Decode Stage ******************************************/
        //std::cout << "DE status: " <<  stall_DE << std::endl;
        std::cout << "----- Start Decode -----------" << std::endl;
        if ((stall_DE != 1) && (DE.Get_Status_of_Pipeline() == params.width))
        {
                // Get instruction from previous stage
                to_DE = FE.Get_and_Remove_Instructions_from_Register();
                // Add instruction from the pervious stage
                stall_FE = DE.Add_Instructions_to_Register(to_DE, sim_time);
                std::cout << "DE: ";
                DE.Print_Instructions_in_Register();
                std::cout << std::endl;
        }
        else
        // If no space available in DE, then stall the FE stage
        {
            stall_FE = 1;
        }
        std::cout << "----- End Decode ------------" << std::endl;
        /**************************************** End Decode Stage *****************************************/


        /******************************************* Fetch Stage *******************************************/
        uint8_t fetched_count = 0;
        //std::cout << "FE status: " <<  stall_FE << std::endl;
        if (stall_FE != 1)
        {
            while (fetched_count < params.width)
            {
                if (fscanf(FP, "%lx %d %d %d %d", &pc, &op_type, &dest, &src1, &src2) != EOF)
                {
                    trace_no++;
                    to_FE.push_back(Instruction_Structure{trace_no, pc, op_type, dest, src1, src2, -1, -1, -1, {0, 0}});
                    fetched_count++;
                    printf("Fetched: %lx %d %d %d %d\n", pc, op_type, dest, src1, src2); //Print to check if inputs have been read correctly
                    // Need to add to the fetch stage
                    FE.Add_Instructions_to_Register(to_FE, sim_time);
                    to_FE.clear();
                }
                else
                {
                    fetched_all_instructions = 1;
                    break;
                }
            }
        }
        /**************************************** End Fetch Stage ******************************************/





        /************************************ Advance cycle calculation ************************************/
        // Increments the simulation time
        std::cout << "***************************************" << std::endl;
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
    printf("\n# Dynamic Instruction Count    = %d", total_no_retired_instructions);
    printf("\n# Cycles                       = %d", sim_time);
    printf("\n# Instructions Per Cycle (IPC) = %.4f%%\n", (((double) total_no_retired_instructions) / ((double) sim_time)));
    /**************************************** Final Output Ends *******************************************/
    return 0;
}
