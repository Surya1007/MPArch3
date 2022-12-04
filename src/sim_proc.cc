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
    
    /*
    printf("rob_size:%lu "
            "iq_size:%lu "
            "width:%lu "
            "tracefile:%s\n", params.rob_size, params.iq_size, params.width, trace_file);
    */
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

    vector<Instruction_Structure> to_FE;
    vector<Instruction_Structure> to_DE;
    vector<Instruction_Structure> to_RN;
    vector<Instruction_Structure> to_RR;
    vector<Instruction_Structure> to_DI;
    vector<Instruction_Structure> to_IS;
    vector<Instruction_Structure> to_EX;
    vector<Instruction_Structure> to_WB;
    vector<Instruction_Structure> to_RT;
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
    vector<bool> evicted_status;
    vector<unsigned int> evicted_tails;
    vector<Instruction_Structure> completed_instructions;
    vector<Instruction_Structure> TempInstructions_to_EX;
    vector<Selective_Removal_Struct> temp_structure;
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
        //cout << "Sim time: " << sim_time << endl;
        //cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << endl;
        //Rename_Map_Table_controller.Print_RMT();
        //cout << "?????????????????????????????????????????????????????????????????????????" << endl;
        //IQ_controller.Print_IQ();
        //cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" << endl;
        //ROB_controller.Print_ROB();
        //cout << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<" << endl;
        /******************************************* Retire Stage ******************************************/
        //cout << "---------------------------- Retire Stage -------------------------------" << endl;
        // If ROB is not empty
        RT.Increment_Time();
        if (ROB_controller.Get_Availability_in_ROB() != 0)
        {

            

            //cout << "ATLEAST HERE" << endl;
            unsigned int tail_index = 0;
            bool Removal_Status = 1;
            unsigned int no_of_retired_instructions = 0;
            
            while((Removal_Status == 1) && (RT.Get_Status_of_Pipeline() != -1))
            {
                tail_index = ROB_controller.Get_Tail_from_ROB();
                Removal_Status = ROB_controller.Remove_Instruction_from_ROB();
                if (Removal_Status == 1)
                {
                    completed_instructions.clear();
                    unsigned long PC_temp = ROB_controller.Get_PC_from_ROB(tail_index);
                    //cout << "Porching for " << hex << PC_temp << dec << endl;
                    Selective_Removal_Struct temp =  RT.Search_Specific_Register_using_PC(PC_temp);
                    Selective_Removal_Struct storing;
                    if (temp.success == 1)
                        storing = RT.Selective_Remove_Instruction(temp.instruction);
                    else
                    {
                        //cout << "Error(Retire): Somethiong wrong in retiring instructions" << endl;
                    }
                    if (storing.success == 1)
                    {
                        completed_instructions.push_back(storing.instruction);
                        for(unsigned int indexing = 0; indexing < completed_instructions.size(); indexing++)
                        {
                            cout << completed_instructions[indexing].seq_no << " fu{" << completed_instructions[indexing].op_type
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
                            << "}" << endl;


                            total_no_retired_instructions++;
                            if (completed_instructions[indexing].dest_register != -1)
                            {
                                if (completed_instructions[indexing].renamed_dest == Rename_Map_Table_controller.Get_Rob_Tag_from_RMT(completed_instructions[indexing].dest_register).rob_tag)
                                {
                                    Rename_Map_Table_controller.Reset_Rob_Tag_in_RMT(completed_instructions[indexing].dest_register);
                                }
                            }
                        }
                        evicted_status.clear();
                        evicted_tails.clear();
                        //cout << "Poshu" << endl;
                        no_of_retired_instructions++;
                    }
                }
                if (no_of_retired_instructions >= params.width)
                {
                    break;
                }
            }

        }



        WB.Increment_Time();
        if (WB.Get_Status_of_Pipeline() != -1)
        {
            unsigned int available_space_in_RT = RT.Get_Availability_of_Pipeline();
            // Get instruction from previous stage
            to_RT = WB.Get_and_Remove_Instructions_from_Register();
            
            // Add instruction from the pervious stage
            stall_WB = RT.Add_Instructions_to_Register(to_RT, sim_time);
            //cout << "Retire stage: ";
            //RT.Print_Instructions_in_Register();
        }
        //RT.Print_Timing();
        // Change the ROB tail pointer
        //cout << "---------------------------- End Retire Stage -------------------------------" << endl;
        /**************************************** End Retire Stage *****************************************/

        /***************************************** Writeback Stage *****************************************/
        //cout << "---------------------------- Writeback Stage -------------------------------" << endl;
        unsigned int available_space_in_WB = WB.Get_Availability_of_Pipeline();
        EX.Increment_Time();
        if ((stall_WB != 1) && (EX.Get_Status_of_Pipeline() != -1))
        {
            //if (EX.Get_Status_of_Pipeline() != -1)
            {
                // Need to remove instructions that have completed execution only
                // Get instruction from previous stage
                to_WB = EX.Search_for_Completed_Instructions();
                
                // Add instruction from the pervious stage
                if (to_WB.size() != 0)
                {
                    
                    for(unsigned int indexing = 0; indexing < to_WB.size(); indexing++)
                    {
                        //cout << "Moshi: " << to_WB[indexing].seq_no << "\t" << to_WB[indexing].renamed_dest << endl;
                        ROB_controller.Mark_Instruction_Ready(to_WB[indexing].renamed_dest);
                        //cout << "WriteBack stage: " << to_WB[indexing].seq_no << endl;
                    }
                    stall_EX = WB.Add_Instructions_to_Register(to_WB, sim_time);    
                    //cout << "WriteBack stage: ";
                    //WB.Print_Instructions_in_Register(); 
                }
                // Mark the ready bits in ROB   
                
                // If instruction completed execution, remove from execute_list / execute stage
                // Add instruction to writeback stage
                // Wakeup dependent instructions in IQ, DI, RR    
            }
        }
        else
        {
            //cout << "Stalling EX" << endl;
            stall_EX = 0;
        }
        //WB.Print_Timing();
        //cout << "---------------------------- End Writeback Stage -------------------------------" << endl;
        /************************************** End Writeback Stage ****************************************/

        /****************************************** Execute Stage ******************************************/
        //cout << "---------------------------- Execute Stage -------------------------------" << endl;
        // Execution happens here
        unsigned int available_space_in_EX = EX.Get_Availability_of_Pipeline();
        //cout << "Shit is : " << stall_EX << available_space_in_EX << endl;
        IS.Increment_Time();
        if ((stall_EX != 1) && (available_space_in_EX != 0) && (IS.Get_Just_Availability() != 0))
        {
            //cout << "Execute Stage: ";
            //EX.Print_Instructions_in_Register();
            //if (IS.Get_Status_of_Pipeline() != -1)
            IS.Get_Status_of_Pipeline();
            //IS.Get_and_Remove_Instructions_from_Register();
            // Older instructions have higher priority

            //for (unsigned int indexing = 0; indexing < to_IS.size(); indexing++)
            //{
            //    cout << "Issue stage: " << to_IS[indexing].seq_no << endl;
            //}
            //cout << "SIZE is : " << to_EX.size() << endl;
            if (to_EX.size() != 0)
            {
                //cout << "OK fucker : " << to_EX[0].seq_no << endl;
                stall_IS = EX.Add_Instructions_to_Register(to_EX, sim_time);
                //cout << "Heyyyyyy:::: "  << stall_IS << endl;
            }
            to_EX.clear();
            temp_structure.clear();
            vector<Instruction_Structure> Almost_completed = EX.Search_for_Almost_Completed_Instructions();
            for (unsigned int indexing = 0; indexing < Almost_completed.size(); indexing++)
            {
                IQ_controller.Set_SRC_Ready_Bit(Almost_completed[indexing].dest_register);
            }
        }
        else
        {
            cout << "Stalling IS" << endl;
            stall_IS = 0;
        }
        //EX.Print_Timing();
        //cout << "---------------------------- End Execute Stage -------------------------------" << endl;
        /*************************************** End Execute Stage *****************************************/

        /******************************************** Issue Stage ******************************************/
        //cout << "---------------------------- Issue Stage -------------------------------" << endl;
        unsigned int available_space_in_IS = IS.Get_Availability_of_Pipeline();
        // = IS.Get_Status_of_Pipeline();
        //cout << "Dithc " << available_space_in_IS << endl;
        // If issue queue is not empty, and IS stage is not stalled.
        DI.Increment_Time();
        if ((stall_IS != 1) && (IQ_controller.Get_No_Available_Elements_in_IQ() != 0) && (available_space_in_IS != 0))
        {
            if (DI.Get_Status_of_Pipeline() != -1)
            {
                // Get instruction from previous stage
                to_IS = DI.Get_and_Remove_Instructions_from_Register();
                // Add instruction from the pervious stage
                stall_DI = IS.Add_Instructions_to_Register(to_IS, sim_time);
                
                //cout << "Issue Stage: ";
                //IS.Print_Instructions_in_Register();
                
                TempInstructions_to_EX.clear();
                temp_structure = IQ_controller.Query_for_Oldest_Instructions_from_IQ(available_space_in_EX);
                unsigned int no_of_issues = 0;
                //cout << "-------------------Issue Queue-------------------------" << endl;
                while(TempInstructions_to_EX.size() != temp_structure.size())
                {
                    //cout << "Values are: " << TempInstructions_to_EX.size() << ", while " << temp_structure.size() << endl;
                    //cout << "Pushed is " << temp_structure[no_of_issues].instruction.seq_no << endl;
                    TempInstructions_to_EX.push_back(temp_structure[no_of_issues].instruction);
                    no_of_issues++;
                }
                //cout << "---------------- End Issue Queue-------------------------" << endl;
                if (TempInstructions_to_EX.size() != 0)
                {
                    for(unsigned int indexing = 0; indexing < TempInstructions_to_EX.size(); indexing++) // The max value should be iq_size or WIDTH
                    {
                        // Remove instruction from IQ
                        Selective_Removal_Struct Status_IS = IS.Pseudo_Selective_Remove_Instruction(TempInstructions_to_EX[indexing]);
                        if (Status_IS.success == 0)
                        {
                            //cout << "Something is Fishy!" << endl;

                        }
                        else
                        {
                            //cout << "Maybe Succesfully pushing is " << Status_IS.instruction.seq_no << endl;
                            Status_IS.instruction.time_info.duration_at_each_stage[5]++;
                            to_EX.push_back(Status_IS.instruction);
                            IS.Selective_Remove_Instruction(Status_IS.instruction);
                            IQ_controller.Remove_Instruction_from_IQ(Status_IS.instruction.seq_no);
                        }
                        // Add instruction to the execute_list, or execute stage
                    }
                }
                //IQ_controller.Print_IQ();
                //cout << "Yalla" << endl;
                // Include bypass here
            }
        }
        else
        {
            cout << "Stalling DI" << endl;
            stall_DI = 0;
        }
        // Another possibility for a bypass
        //IS.Print_Timing();
        //cout << "---------------------------- End Issue Stage -------------------------------" << endl;
        /***************************************** End Issue Stage *****************************************/

        /****************************************** Dispatch Stage *****************************************/
        //cout << "---------------------------- Dispatch Stage -------------------------------" << endl;
        RR.Increment_Time();
        if ((stall_DI != 1) && (DI.Get_Availability_of_Pipeline() == params.width) && (IQ_controller.Get_No_Available_Elements_in_IQ() >= params.width)) // Also check the issuequeue
        {
            if(RR.Get_Status_of_Pipeline() != -1)
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
                //cout << "Dispatch Stage: ";
                //DI.Print_Instructions_in_Register();

                // Include bypass here
            }
        }
        else
        {
            cout << "Stalling RR" << endl;
            stall_RR = 1;
        }
        // Another possibility for a bypass
        //DI.Print_Timing();
        //cout << "---------------------------- End Dispatch Stage -------------------------------" << endl;
        /************************************** End Dispatch Stage *****************************************/

        /****************************************** Regread Stage ******************************************/
        //cout << "---------------------------- Regread Stage -------------------------------" << endl;
        RN.Increment_Time();
        if ((stall_RR != 1) && (RR.Get_Availability_of_Pipeline() == params.width))
        {
            if (RN.Get_Status_of_Pipeline() != -1)
            {
                // Get instruction from previous stage
                to_RR = RN.Get_and_Remove_Instructions_from_Register();
                //for (unsigned int indexing = 0; indexing < to_RR.size(); indexing++)
                //{
                //    cout << "Register read: " << to_RR[indexing].seq_no << endl;
                //}
                // Add instruction from the pervious stage
                stall_RN = RR.Add_Instructions_to_Register(to_RR, sim_time);
                // Need to include a bypass here
                // Not sure if ready bits are required or not
                // If required, search through ROB for the renamed source registers,
                /*&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&*/
            }

        }
        else
        {
            cout << "Stalling RN" << endl;
           stall_RN = 1;
        }
        // Another possibility for a bypass
        //RR.Print_Timing();
        //cout << "---------------------------- End RegRead Stage -------------------------------" << endl;
        /*************************************** End Regread Stage *****************************************/

        /******************************************* Rename Stage ******************************************/
        //cout << "---------------------------- Rename Stage -------------------------------" << endl;
        DE.Increment_Time();
        if ((stall_RN != 1) && (RN.Get_Availability_of_Pipeline() == params.width) && (ROB_controller.Get_Availability_in_ROB() >= params.width))
        {
            if (DE.Get_Status_of_Pipeline() != -1)
            {
                // Get instruction from previous stage
                to_RN = DE.Get_and_Remove_Instructions_from_Register();
                if (to_RN.size() != 0)
                {
                    for (unsigned int indexing = 0; indexing < params.width; indexing++)
                    {
                        // Allocate an entry in ROB for instructions in order
                        unsigned int dest_pointer = ROB_controller.Add_Instruction_to_ROB(to_RN[indexing].dest_register, to_RN[indexing].PC);
                        ROB_controller.Increment_header();
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
                        //cout << "Renaming the registers as: " << to_RN[indexing].dest_register << "with: " << dest_pointer << endl;
                        if (to_RN[indexing].dest_register != -1)
                            Rename_Map_Table_controller.Set_Rob_Tag_in_RMT(to_RN[indexing].dest_register, dest_pointer);
                    }
                }
                // Add modified instruction from the pervious stage
                for (unsigned int indexing = 0; indexing < to_RN.size(); indexing++)
                {
                    //cout << "Renamed: " << to_RN[indexing].seq_no << endl;
                }
                stall_DE = RN.Add_Instructions_to_Register(to_RN, sim_time);
            }
        }
        else
        {
            cout << "Stalling DE" << endl;
            cout << "ROB availability: " << ROB_controller.Get_Availability_in_ROB() << endl;
            stall_DE = 1;
        }
        //RN.Print_Timing();
        //cout << "---------------------------- End Rename Stage -------------------------------" << endl;
        /**************************************** End Rename Stage *****************************************/


        /******************************************* Decode Stage ******************************************/
        //cout << "---------------------------- Decode Stage -------------------------------" << endl;
        FE.Increment_Time();
        if ((stall_DE != 1) && (DE.Get_Availability_of_Pipeline() == params.width))
        {
            if (FE.Get_Status_of_Pipeline() != -1)
            {
                // Get instruction from previous stage
                to_DE = FE.Get_and_Remove_Instructions_from_Register();
                // Add instruction from the pervious stage
                stall_FE = DE.Add_Instructions_to_Register(to_DE, sim_time);
                for (unsigned int indexing = 0; indexing < to_DE.size(); indexing++)
                {
                    //cout << "Decoded: " << to_DE[indexing].seq_no << endl;
                }
            }
        }
        else
        // If no space available in DE, then stall the FE stage
        {
            cout << "Stalling FE" << endl;
            stall_FE = 1;
        }
        //DE.Print_Timing();
        //cout << "------------------------- End Decode Stage -------------------------------" << endl;
        /**************************************** End Decode Stage *****************************************/


        /******************************************* Fetch Stage *******************************************/
        //cout << "---------------------------- Fetch Stage -------------------------------" << endl;
        uint8_t fetched_count = 0;
        FE.Get_Availability_of_Pipeline();
        //cout << "FE status: " <<  stall_FE << endl;
        bool testing = 1;
        if ((stall_FE != 1) && (testing))
        {
            while (fetched_count < params.width)
            {
                if (fscanf(FP, "%lx %d %d %d %d", &pc, &op_type, &dest, &src1, &src2) != EOF)
                {
                    to_FE.push_back(Instruction_Structure{trace_no, pc, op_type, dest, src1, src2, -1, -1, -1, 0, 0, {0, 0}});
                    fetched_count++;
                    //printf("Fetched: %d, %lx %d %d %d %d\n", trace_no, pc, op_type, dest, src1, src2); //Print to check if inputs have been read correctly
                    // Need to add to the fetch stage
                    FE.Add_Instructions_to_Register(to_FE, sim_time);
                    to_FE.clear();
                    trace_no++;
                    //testing = 0;
                }
                else
                {
                    fetched_all_instructions = 1;
                    break;
                }
            }
        }
        //FE.Print_Timing();
        //cout << "------------------------- End Fetch Stage -------------------------------" << endl;
        /**************************************** End Fetch Stage ******************************************/





        /************************************ Advance cycle calculation ************************************/
        // Increments the simulation time
        //cout << "****************************************************************************************************************************" << endl;
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
