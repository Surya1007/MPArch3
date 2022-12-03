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
    Rename_Map_Table_Operator Rename_Map_Table_controller(67);
    IssueQueue_Operator IQ_controller(params.iq_size);
    ROB_Operator ROB_controller(params.rob_size);

    Pipeline_Stage_Operator FE(params.width, 0);
    Pipeline_Stage_Operator DE(params.width, 1);
    Pipeline_Stage_Operator RN(params.width, 2);
    Pipeline_Stage_Operator RR(params.width, 3);
    Pipeline_Stage_Operator DI(params.width, 4);
    Pipeline_Stage_Operator IS(params.iq_size, 5); // Concerned part
    Pipeline_Stage_Operator EX(params.width * 5, 6);
    Pipeline_Stage_Operator WB(params.width * 5, 7);
    Pipeline_Stage_Operator RT(params.width * 10, 8); // May require change

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
    std::vector<bool> evicted_status;
    std::vector<unsigned int> evicted_tails;
    std::vector<Instruction_Structure> completed_instructions;
    
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
        std::cout << "---------------------------- Retire Stage -------------------------------" << std::endl;
        // If ROB is not empty
        if (ROB_controller.Get_Availability_in_ROB() != 0)
        {
            unsigned int tail_index = ROB_controller.Get_Tail_from_ROB();
            bool temp_status = ROB_controller.Remove_Instruction_from_ROB();
            // Evict till width instructions only, or till the lates ready instruction, which ever is early
            while ((temp_status == 1) || (evicted_tails.size() <= params.width))
            {
                evicted_status.push_back(temp_status);
                evicted_tails.push_back(tail_index);
                tail_index = ROB_controller.Get_Tail_from_ROB();
                temp_status = ROB_controller.Remove_Instruction_from_ROB();
            }

            // Search in RT for the completed instructions
            /**/
            completed_instructions = RT.Get_and_Remove_Instructions_from_Register();
            for(unsigned int indexing = 0; indexing < completed_instructions.size(); indexing++)
            {
                std::cout << completed_instructions[indexing].seq_no << " fu{" << completed_instructions[indexing].op_type
                << "} src{" << completed_instructions[indexing].src1_register << "," << completed_instructions[indexing].src2_register 
                << "} dst{" << completed_instructions[indexing].dest_register
                << "} FE{" << completed_instructions[indexing].time_info.start_time_at_each_stage[0] << "," << completed_instructions[indexing].time_info.duration_at_each_stage[0]
                << "} DE{" << completed_instructions[indexing].time_info.start_time_at_each_stage[1] << "," << completed_instructions[indexing].time_info.duration_at_each_stage[1]
                << "} RN{" << completed_instructions[indexing].time_info.start_time_at_each_stage[2] << "," << completed_instructions[indexing].time_info.duration_at_each_stage[2]
                << "} RR{" << completed_instructions[indexing].time_info.start_time_at_each_stage[3] << "," << completed_instructions[indexing].time_info.duration_at_each_stage[3]
                << "} DI{" << completed_instructions[indexing].time_info.start_time_at_each_stage[4] << "," << completed_instructions[indexing].time_info.duration_at_each_stage[4]
                << "} IS{" << completed_instructions[indexing].time_info.start_time_at_each_stage[5] << "," << completed_instructions[indexing].time_info.duration_at_each_stage[5]
                << "} EX{" << completed_instructions[indexing].time_info.start_time_at_each_stage[6] << "," << completed_instructions[indexing].time_info.duration_at_each_stage[6]
                << "} WB{" << completed_instructions[indexing].time_info.start_time_at_each_stage[7] << "," << completed_instructions[indexing].time_info.duration_at_each_stage[7]
                << "} RT{" << completed_instructions[indexing].time_info.start_time_at_each_stage[8] << "," << completed_instructions[indexing].time_info.duration_at_each_stage[8]
                << "}" << std::endl;
                total_no_retired_instructions++;
            }
            evicted_status.clear();
        }

        unsigned int available_space_in_RT = RT.Get_Availability_of_Pipeline();
        // Get instruction from previous stage
        to_RT = WB.Get_and_Remove_Instructions_from_Register();
        // Add instruction from the pervious stage
        stall_WB = RT.Add_Instructions_to_Register(to_RT, sim_time);
        RT.Print_Timing();
        // Change the ROB tail pointer
        std::cout << "---------------------------- End Retire Stage -------------------------------" << std::endl;
        /**************************************** End Retire Stage *****************************************/

        /***************************************** Writeback Stage *****************************************/
        std::cout << "---------------------------- Writeback Stage -------------------------------" << std::endl;
        unsigned int available_space_in_WB = WB.Get_Availability_of_Pipeline();
        if (stall_WB != 1)
        {
            // Need to remove instructions that have completed execution only
            //std::cout << "Hello" << std::endl;
            // Get instruction from previous stage
            to_WB = EX.Search_for_Completed_Instructions();
            // Add instruction from the pervious stage
            stall_EX = WB.Add_Instructions_to_Register(to_WB, sim_time);     
            // Mark the ready bits in ROB   
            
            // If instruction completed execution, remove from execute_list / execute stage
            // Add instruction to writeback stage
            // Wakeup dependent instructions in IQ, DI, RR    
        }
        else
        {
            stall_EX = 0;
        }
        WB.Print_Timing();
        std::cout << "---------------------------- End Writeback Stage -------------------------------" << std::endl;
        /************************************** End Writeback Stage ****************************************/

        /****************************************** Execute Stage ******************************************/
        std::cout << "---------------------------- Execute Stage -------------------------------" << std::endl;
        // Execution happens here
        unsigned int available_space_in_EX = EX.Get_Availability_of_Pipeline();
        //std::cout << "Shit is : " << stall_EX << available_space_in_EX << std::endl;
        if ((stall_EX != 1) && (available_space_in_EX != 0) && (IS.Get_Just_Availability() != 0))
        {
            //IS.Get_and_Remove_Instructions_from_Register();
            // Older instructions have higher priority
            std::vector<Instruction_Structure> TempInstructions_to_EX = IQ_controller.Get_Oldest_Instructions_from_IQ(available_space_in_EX);
            if (TempInstructions_to_EX.size() != 0)
            {
                for(unsigned int indexing = 0; indexing < TempInstructions_to_EX.size(); indexing++) // The max value should be iq_size or WIDTH
                {
                    // Remove instruction from IQ
                    Selective_Removal_Struct Status_IS = IS.Pseudo_Selective_Remove_Instruction(TempInstructions_to_EX[indexing]);
                    if (Status_IS.success == 0)
                    {
                        //std::cout << "Something is Fishy!" << std::endl;

                    }
                    else
                    {
                        to_EX.push_back(Status_IS.instruction);
                        IS.Selective_Remove_Instruction(Status_IS.instruction);
                        IQ_controller.Remove_Instruction_from_IQ(Status_IS.instruction.seq_no);
                    }
                    // Add instruction to the execute_list, or execute stage
                }
            }
            //std::cout << "SIZE is : " << to_EX.size() << std::endl;
            if (to_EX.size() != 0)
            {
                //std::cout << "OK" << std::endl;
                stall_IS = EX.Add_Instructions_to_Register(to_EX, sim_time);
            }
            std::vector<Instruction_Structure> Almost_completed = EX.Search_for_Almost_Completed_Instructions();
        }
        else
        {
            stall_IS = 0;
        }
        EX.Print_Timing();
        std::cout << "---------------------------- End Execute Stage -------------------------------" << std::endl;
        /*************************************** End Execute Stage *****************************************/

        /******************************************** Issue Stage ******************************************/
        std::cout << "---------------------------- Issue Stage -------------------------------" << std::endl;
        unsigned int available_space_in_IS = IS.Get_Availability_of_Pipeline();
        //std::cout << "Dithc " << available_space_in_IS << std::endl;
        // If issue queue is not empty, and IS stage is not stalled.
        if ((stall_IS != 1) && (IQ_controller.Get_No_Available_Elements_in_IQ() != 0) && (available_space_in_IS != 0))
        {
            to_EX.clear();
            // Get instruction from previous stage
            to_IS = DI.Get_and_Remove_Instructions_from_Register();
            // Add instruction from the pervious stage
            stall_DI = IS.Add_Instructions_to_Register(to_IS, sim_time);
            std::cout << std::endl;
            IQ_controller.Print_IQ();
            //std::cout << "Yalla" << std::endl;
            // Include bypass here
        }
        else
        {
            stall_DI = 0;
        }
        // Another possibility for a bypass
        IS.Print_Timing();
        std::cout << "---------------------------- End Issue Stage -------------------------------" << std::endl;
        /***************************************** End Issue Stage *****************************************/

        /****************************************** Dispatch Stage *****************************************/
        std::cout << "---------------------------- Dispatch Stage -------------------------------" << std::endl;
        if ((stall_DI != 1) && (DI.Get_Availability_of_Pipeline() == params.width) && (IQ_controller.Get_No_Available_Elements_in_IQ() >= params.width)) // Also check the issuequeue
        {
            // Get instruction from previous stage
            to_DI = RR.Get_and_Remove_Instructions_from_Register();
            // Add instruction to issuequeue
            if (to_DI.size() != 0)
            {
                for(unsigned int indexing = 0; indexing < params.width; indexing++)
                {
                    IQ_controller.Add_Instruction_to_IQ(to_DI[indexing]);
                }
            }
            // Add instruction from the pervious stage
            stall_RR = DI.Add_Instructions_to_Register(to_DI, sim_time);
            // Include bypass here
        }
        else
        {
            stall_RR = 1;
        }
        // Another possibility for a bypass
        DI.Print_Timing();
        std::cout << "---------------------------- End Dispatch Stage -------------------------------" << std::endl;
        /************************************** End Dispatch Stage *****************************************/

        /****************************************** Regread Stage ******************************************/
        std::cout << "---------------------------- Regread Stage -------------------------------" << std::endl;
        if ((stall_RR != 1) && (RR.Get_Availability_of_Pipeline() == params.width))
        {
            // Get instruction from previous stage
            to_RR = RN.Get_and_Remove_Instructions_from_Register();
            // Add instruction from the pervious stage
            stall_RN = RR.Add_Instructions_to_Register(to_RR, sim_time);
            // Need to include a bypass here
            // Not sure if ready bits are required or not
            // If required, search through ROB for the renamed source registers,
            /*&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&*/
        }
        else
        {
           stall_RN = 1;
        }
        // Another possibility for a bypass
        RR.Print_Timing();
        std::cout << "---------------------------- End RegRead Stage -------------------------------" << std::endl;
        /*************************************** End Regread Stage *****************************************/

        /******************************************* Rename Stage ******************************************/
        std::cout << "---------------------------- Rename Stage -------------------------------" << std::endl;
        if ((stall_RN != 1) && (RN.Get_Availability_of_Pipeline() == params.width) && (ROB_controller.Get_Availability_in_ROB() >= params.width))
        {
            // Get instruction from previous stage
            to_RN = DE.Get_and_Remove_Instructions_from_Register();
            if (to_RN.size() != 0)
            {
                for (unsigned int indexing = 0; indexing < params.width; indexing++)
                {
                    // Allocate an entry in ROB for instructions in order
                    //std::cout << "Hey, " << to_RN[indexing].dest_register << std::endl;
                    unsigned int dest_pointer = ROB_controller.Add_Instruction_to_ROB(to_RN[indexing].dest_register);
                    // Add the renamed source registers to srcs1
                    if (to_RN[indexing].src1_register != -1)
                    {
                        Individual_Rename_Map_Table_struct src1 = Rename_Map_Table_controller.Get_Rob_Tag_from_RMT(to_RN[indexing].src1_register);
                        if (src1.valid == 1)
                        {
                            to_RN[indexing].renamed_src1 = src1.rob_tag;
                        }
                    }
                    // Add the renamed source registers to srcs2
                    if (to_RN[indexing].src2_register != -1)
                    {
                        Individual_Rename_Map_Table_struct src2 = Rename_Map_Table_controller.Get_Rob_Tag_from_RMT(to_RN[indexing].src2_register);
                        if (src2.valid == 1)
                        {
                            to_RN[indexing].renamed_src2 = src2.rob_tag;
                        }
                    }
                    // Add the renamed destination register to dests
                    to_RN[indexing].renamed_dest = dest_pointer;
                    Rename_Map_Table_controller.Set_Rob_Tag_in_RMT(indexing, dest_pointer);
                }
            }
            // Add modified instruction from the pervious stage
            stall_DE = RN.Add_Instructions_to_Register(to_RN, sim_time);
        }
        else
        {
            stall_DE = 1;
        }
        RN.Print_Timing();
        std::cout << "---------------------------- End Rename Stage -------------------------------" << std::endl;
        /**************************************** End Rename Stage *****************************************/


        /******************************************* Decode Stage ******************************************/
        std::cout << "---------------------------- Decode Stage -------------------------------" << std::endl;
        if ((stall_DE != 1) && (DE.Get_Availability_of_Pipeline() == params.width))
        {
                // Get instruction from previous stage
                to_DE = FE.Get_and_Remove_Instructions_from_Register();
                // Add instruction from the pervious stage
                stall_FE = DE.Add_Instructions_to_Register(to_DE, sim_time);
                std::cout << std::endl;
        }
        else
        // If no space available in DE, then stall the FE stage
        {
            stall_FE = 1;
        }
        DE.Print_Timing();
        std::cout << "------------------------- End Decode Stage -------------------------------" << std::endl;
        /**************************************** End Decode Stage *****************************************/


        /******************************************* Fetch Stage *******************************************/
        std::cout << "---------------------------- Fetch Stage -------------------------------" << std::endl;
        uint8_t fetched_count = 0;
        FE.Get_Availability_of_Pipeline();
        //std::cout << "FE status: " <<  stall_FE << std::endl;
        if (stall_FE != 1)
        {
            while (fetched_count < params.width)
            {
                if (fscanf(FP, "%lx %d %d %d %d", &pc, &op_type, &dest, &src1, &src2) != EOF)
                {
                    to_FE.push_back(Instruction_Structure{trace_no, pc, op_type, dest, src1, src2, -1, -1, -1, {0, 0}});
                    fetched_count++;
                    printf("Fetched: %d, %lx %d %d %d %d\n", trace_no, pc, op_type, dest, src1, src2); //Print to check if inputs have been read correctly
                    // Need to add to the fetch stage
                    FE.Add_Instructions_to_Register(to_FE, sim_time);
                    to_FE.clear();
                    trace_no++;
                }
                else
                {
                    fetched_all_instructions = 1;
                    break;
                }
            }
        }
        FE.Print_Timing();
        std::cout << "------------------------- End Fetch Stage -------------------------------" << std::endl;
        /**************************************** End Fetch Stage ******************************************/





        /************************************ Advance cycle calculation ************************************/
        // Increments the simulation time
        std::cout << "****************************************************************************************************************************" << std::endl;
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
