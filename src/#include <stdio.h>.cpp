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
int main(int argc, char *argv[])
{
    FILE *FP;                      // File handler
    char *trace_file;              // Variable that holds trace file name;
    proc_params params;            // look at sim_bp.h header file for the the definition of struct proc_params
    int op_type, dest, src1, src2; // Variables are read from trace file
    unsigned long int pc;          // Variable holds the pc read from input file

    if (argc != 5)
    {
        printf("Error: Wrong number of inputs:%d\n", argc - 1);
        exit(EXIT_FAILURE);
    }

    params.rob_size = strtoul(argv[1], NULL, 10);
    params.iq_size = strtoul(argv[2], NULL, 10);
    params.width = strtoul(argv[3], NULL, 10);
    trace_file = argv[4];

    /*
    printf("rob_size:%lu "
            "iq_size:%lu "
            "width:%lu "
            "tracefile:%s\n", params.rob_size, params.iq_size, params.width, trace_file);
    */
    // Open trace_file in read mode
    FP = fopen(trace_file, "r");
    if (FP == NULL)
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
    Pipeline_Stage_Operator RT(params.width * 50, 8); // May require change

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
    bool empty_stage = 1;
    bool DEBUG = 1;
    bool unsuccessfull_rename = 0;
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
    vector<int> registers_to_set_ready;
    bool ready_to_retire = 0;
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
    do
    {
        // stall_DE = 0;
        registers_to_set_ready.clear();

        if (DEBUG == 1)
            cout << "ROB availability is: " << ROB_controller.Get_Availability_in_ROB() << endl;
        if (ROB_controller.Get_Availability_in_ROB() != params.rob_size)
        {
            unsigned int tail_index = 0;
            bool Removal_Status = 1;
            unsigned int no_of_retired_instructions = 0;
            while ((Removal_Status == 1) && (RT.Get_Status_of_Pipeline() != -1))
            {

                tail_index = ROB_controller.Get_Tail_from_ROB();
                Removal_Status = ROB_controller.Check_if_instruction_is_ready_to_retire();
                if (Removal_Status == 1)
                {

                    completed_instructions.clear();
                    unsigned int PC_temp = ROB_controller.Get_SEQ_from_ROB(tail_index);
                    if (DEBUG == 1)
                        cout << "Porching for " << PC_temp << endl;
                    Selective_Removal_Struct temp = RT.Search_Specific_Register_using_seq(PC_temp);
                    Selective_Removal_Struct storing;
                    storing.success = 0;
                    if (temp.success == 1)
                    {

                        if (DEBUG == 1)
                        {
                            cout << "Ready to retire are: , while searching for " << temp.instruction.seq_no << endl;
                            RT.Print_Instructions_in_Register();
                        }
                        storing = RT.Selective_Remove_Instruction(temp.instruction);

                        // cout << "So far so good with storing: " << storing.success << endl;
                    }
                    else
                    {
                        // cout << "Error(Retire): Somethiong wrong in retiring instructions" << endl;
                    }
                    if (storing.success == 1)
                    {
                        // ready_to_retire = 1;
                        completed_instructions.push_back(storing.instruction);
                        for (unsigned int indexing = 0; indexing < completed_instructions.size(); indexing++)
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
                                if (DEBUG == 1)
                                    cout << "I am the shithole trying to push: " << completed_instructions[indexing].renamed_dest << endl;
                                registers_to_set_ready.push_back(completed_instructions[indexing].renamed_dest);
                                if (completed_instructions[indexing].renamed_dest == Rename_Map_Table_controller.Get_Rob_Tag_from_RMT(completed_instructions[indexing].dest_register).rob_tag)
                                {
                                    Rename_Map_Table_controller.Reset_Rob_Tag_in_RMT(completed_instructions[indexing].dest_register);
                                }
                            }
                        }
                        evicted_status.clear();
                        evicted_tails.clear();
                        no_of_retired_instructions++;
                        Removal_Status = ROB_controller.Remove_Instruction_from_ROB();
                        if (Removal_Status == 0)
                        {
                            if (DEBUG == 1)
                                cout << "Serious shit" << endl;
                        }
                    }
                    else
                    {
                        break;
                    }
                }
                if (no_of_retired_instructions >= params.width)
                {
                    break;
                }
            }
        }

        if (WB.Get_Status_of_Pipeline() != -1)
        {
            
            unsigned int available_space_in_RT = RT.Get_Availability_of_Pipeline();
            // Get instruction from previous stage
            to_RT = WB.Get_and_Remove_Instructions_from_Register();
            for(unsigned int indexing = 0; indexing < to_RT.size(); indexing++)
                ROB_controller.Mark_Instruction_Ready(to_RT[indexing].renamed_dest);

            // Add instruction from the pervious stage
            stall_WB = RT.Add_Instructions_to_Register(to_RT, sim_time);
            if (DEBUG == 1)
            {
                cout << "Retire stage: ";
                RT.Print_Instructions_in_Register();
            }
        }
        RT.Increment_Time();

        // RT.Print_Timing();
        //  Change the ROB tail pointer
        // cout << "---------------------------- End Retire Stage -------------------------------" << endl;
        /**************************************** End Retire Stage *****************************************/

        /***************************************** Writeback Stage *****************************************/
        // cout << "---------------------------- Writeback Stage -------------------------------" << endl;
        unsigned int available_space_in_WB = WB.Get_Availability_of_Pipeline();

        if ((stall_WB != 1) && WB.Get_Availability_of_Pipeline()) //(EX.Get_Status_of_Pipeline() != -1))
        {
            to_WB = EX.Search_for_Completed_Instructions();
            // Add instruction from the pervious stage
            if (to_WB.size() != 0)
            {
                for (unsigned int indexing = 0; indexing < to_WB.size(); indexing++)
                {
                    if (DEBUG == 1)
                        cout << "Broadcasting: " << to_WB[indexing].renamed_dest << " by seq: " << to_WB[indexing].seq_no << endl;
                    registers_to_set_ready.push_back(to_WB[indexing].renamed_dest);
                    IQ_controller.Set_SRC_Ready_Bit(to_WB[indexing].renamed_dest);
                    // cout << "Moshi: " << to_WB[indexing].seq_no << "\t" << to_WB[indexing].renamed_dest << endl;
                    
                    // cout << "WriteBack stage: " << to_WB[indexing].seq_no << endl;
                }
                stall_EX = WB.Add_Instructions_to_Register(to_WB, sim_time);
                if (DEBUG == 1)
                {
                    cout << "WriteBack stage: ";
                    WB.Print_Instructions_in_Register();
                }
            }
            // Mark the ready bits in ROB
        }
        else
        {
            if (DEBUG == 1)
                cout << "Stalling EX" << endl;
            stall_EX = 0;
        }
        WB.Increment_Time();

        // WB.Print_Timing();
        // cout << "---------------------------- End Writeback Stage -------------------------------" << endl;
        /************************************** End Writeback Stage ****************************************/

        /****************************************** Execute Stage ******************************************/
        // cout << "---------------------------- Execute Stage -------------------------------" << endl;
        //  Execution happens here

        // cout << "Shit is : " << stall_EX << available_space_in_EX << endl;

        if ((stall_EX != 1) && (EX.Get_Availability_of_Pipeline() != 0)) // (IS.Get_Just_Availability() != 0)
        {
            to_EX.clear();
            temp_structure.clear();

            TempInstructions_to_EX.clear();
            unsigned int available_space_in_EX = EX.Get_Availability_of_Pipeline();
            temp_structure = IQ_controller.Query_for_Oldest_Instructions_from_IQ(available_space_in_EX, params.width);
            unsigned int no_of_issues = 0;
            // cout << "-------------------Issue Queue-------------------------" << endl;
            while (TempInstructions_to_EX.size() != temp_structure.size())
            {
                // cout << "Values are: " << TempInstructions_to_EX.size() << ", while " << temp_structure.size() << endl;
                // cout << "Pushed is " << temp_structure[no_of_issues].instruction.seq_no << endl;
                TempInstructions_to_EX.push_back(temp_structure[no_of_issues].instruction);
                no_of_issues++;
            }

            if (TempInstructions_to_EX.size() != 0)
            {
                for (unsigned int indexing = 0; indexing < TempInstructions_to_EX.size(); indexing++) // The max value should be iq_size or WIDTH
                {
                    // Remove instruction from IQ
                    Selective_Removal_Struct Status_IS = IS.Pseudo_Selective_Remove_Instruction(TempInstructions_to_EX[indexing]);
                    if (Status_IS.success == 0)
                    {
                        // cout << "Something is Fishy!" << endl;
                    }
                    else
                    {
                        // cout << "Maybe Succesfully pushing is " << Status_IS.instruction.seq_no << endl;
                        // Status_IS.instruction.time_info.duration_at_each_stage[5]++;
                        to_EX.push_back(Status_IS.instruction);
                        IS.Selective_Remove_Instruction(Status_IS.instruction);
                        IQ_controller.Remove_Instruction_from_IQ(Status_IS.instruction.seq_no);
                    }
                    // Add instruction to the execute_list, or execute stage
                }
            }
            if (to_EX.size() != 0)
            {
                // cout << "OK fucker : " << to_EX[0].seq_no << endl;
                stall_IS = EX.Add_Instructions_to_Register(to_EX, sim_time);

                // cout << "Heyyyyyy:::: "  << stall_IS << endl;
            }
            if (DEBUG == 1)
            {
                cout << "Execute Stage: ";
                EX.Print_Instructions_in_Register();
            }
        }
        else
        {
            if (DEBUG == 1)
                cout << "Stalling IS" << endl;
            stall_IS = 0;
        }
        EX.Increment_Time();

        if (EX.Get_Just_Availability() != (params.width * 5))
        {
            vector<Instruction_Structure> temp_registers = EX.Search_for_Almost_Completed_Instructions();
            if (temp_registers.size() != 0)
            {
                for (unsigned int indexing = 0; indexing < temp_registers.size(); indexing++)
                {
                    if (DEBUG == 1)
                        cout << "Torturing: " << temp_registers[indexing].renamed_dest << endl;
                    registers_to_set_ready.push_back(temp_registers[indexing].renamed_dest);
                }
            }
        }
        // EX.Print_Timing();
        // cout << "---------------------------- End Execute Stage -------------------------------" << endl;
        /*************************************** End Execute Stage *****************************************/

        /******************************************** Issue Stage ******************************************/
        // cout << "---------------------------- Issue Stage -------------------------------" << endl;
        unsigned int available_space_in_IS = IS.Get_Availability_of_Pipeline();
        // = IS.Get_Status_of_Pipeline();
        // cout << "Dithc " << available_space_in_IS << endl;
        // If issue queue is not empty, and IS stage is not stalled.

        if ((stall_IS != 1) && (IQ_controller.Get_No_Available_Elements_in_IQ() != 0) && (available_space_in_IS != 0))
        {
            if (DI.Get_Status_of_Pipeline() != -1)
            {
                // Get instruction from previous stage
                if (registers_to_set_ready.size() != 0)
                    DI.Set_Renamed_Register_Ready(registers_to_set_ready);
                to_IS = DI.Get_and_Remove_Instructions_from_Register();

                // Add instruction to issuequeue
                if (to_IS.size() != 0)
                {
                    for (unsigned int indexing = 0; indexing < params.width; indexing++)
                    {
                        // cout << "Moshi " << to_IS[indexing].src1_ready_status << to_IS[indexing].src1_ready_status << endl;
                        IQ_controller.Add_Instruction_to_IQ(to_IS[indexing]);
                    }
                }
                // Add instruction from the pervious stage
                stall_DI = IS.Add_Instructions_to_Register(to_IS, sim_time);

                if (DEBUG == 1)
                {
                    cout << "Issue Stage: ";
                    IS.Print_Instructions_in_Register();
                }

                // cout << "---------------- End Issue Queue-------------------------" << endl;

                // IQ_controller.Print_IQ();
                // cout << "Yalla" << endl;
                //  Include bypass here
            }
        }
        else
        {
            if (DEBUG == 1)
                cout << "Stalling DI" << endl;
            stall_DI = 0;
        }
        IS.Increment_Time();

        // Another possibility for a bypass
        // IS.Print_Timing();
        // cout << "---------------------------- End Issue Stage -------------------------------" << endl;
        /***************************************** End Issue Stage *****************************************/

        /****************************************** Dispatch Stage *****************************************/
        // cout << "---------------------------- Dispatch Stage -------------------------------" << endl;

        if ((stall_DI != 1) && (DI.Get_Availability_of_Pipeline() == params.width)) // Also check the issuequeue (IQ_controller.Get_No_Available_Elements_in_IQ() >= params.width)
        {
            if (RR.Get_Status_of_Pipeline() != -1)
            {
                // Get instruction from previous stage
                if (registers_to_set_ready.size() != 0)
                    RR.Set_Renamed_Register_Ready(registers_to_set_ready);
                to_DI = RR.Get_and_Remove_Instructions_from_Register();

                // Add instruction from the pervious stage
                stall_RR = DI.Add_Instructions_to_Register(to_DI, sim_time);
                if (DEBUG == 1)
                {
                    cout << "Dispatch Stage: ";
                    DI.Print_Instructions_in_Register();
                }

                // Include bypass here
            }
        }
        else
        {
            if (DEBUG == 1)
                cout << "Stalling RR" << endl;
            stall_RR = 1;
        }
        DI.Increment_Time();

        // Another possibility for a bypass
        // DI.Print_Timing();
        // cout << "---------------------------- End Dispatch Stage -------------------------------" << endl;
        /************************************** End Dispatch Stage *****************************************/

        /****************************************** Regread Stage ******************************************/
        // cout << "---------------------------- Regread Stage -------------------------------" << endl;

        if (RN.Get_Just_Availability() != params.width)
        {
            // cout << "Yallaaaaaa" << endl;
            if (registers_to_set_ready.size() != 0)
                RN.Set_Renamed_Register_Ready(registers_to_set_ready);
            // cout << "****************************************" << endl;
        }

        if ((stall_RR != 1) && (RR.Get_Availability_of_Pipeline() == params.width))
        {
            if (empty_stage == 0)
            {
                if ((ROB_controller.Get_Availability_in_ROB() >= params.width)) // && (empty_stage == 0)) // || (ready_to_retire == 1))
                {
                    vector<Instruction_Structure> temp_regs = RN.Get_Just_Registers();
                    for (unsigned int indexing = 0; indexing < params.width; indexing++)
                    {
                        //  Allocate an entry in ROB for instructions in order
                        unsigned int dest_pointer = ROB_controller.Add_Instruction_to_ROB(temp_regs[indexing].dest_register, temp_regs[indexing].PC, temp_regs[indexing].seq_no);
                        ROB_controller.Increment_header();
                        // Add the renamed source registers to srcs1
                        if (temp_regs[indexing].src1_register != -1)
                        {
                            Individual_Rename_Map_Table_struct src1 = Rename_Map_Table_controller.Get_Rob_Tag_from_RMT(temp_regs[indexing].src1_register);
                            if (src1.valid == 1)
                            {
                                temp_regs[indexing].renamed_src1 = src1.rob_tag; // bla bla
                            }
                        }
                        // Add the renamed source registers to srcs2
                        if (temp_regs[indexing].src2_register != -1)
                        {
                            Individual_Rename_Map_Table_struct src2 = Rename_Map_Table_controller.Get_Rob_Tag_from_RMT(temp_regs[indexing].src2_register);
                            if (src2.valid == 1)
                            {
                                temp_regs[indexing].renamed_src2 = src2.rob_tag; // bla bla
                            }
                        }
                        // Add the renamed destination register to dests
                        temp_regs[indexing].renamed_dest = dest_pointer;
                        RN.Add_Just_Registers(temp_regs);
                        if (registers_to_set_ready.size() != 0)
                            RN.Set_Renamed_Register_Ready(registers_to_set_ready);
                        // renamed = 1;
                        if (DEBUG == 1)
                        {
                            cout << "Renamed register set" << endl;
                        }
                        unsuccessfull_rename = 0;
                        stall_DE = 0;
                        // cout << "Renaming the registers as: " << to_RN[indexing].dest_register << "with: " << dest_pointer << endl;
                        if (temp_regs[indexing].dest_register != -1)
                            Rename_Map_Table_controller.Set_Rob_Tag_in_RMT(temp_regs[indexing].dest_register, dest_pointer);
                    }

                    if (DEBUG == 1)
                    {
                        cout << "Yoo Rename Stage: ";
                        RN.Print_Instructions_in_Register();
                    }
                }
                else
                {

                     cout << "ROBFULL " << endl;

                    stall_DE = 1;
                    unsuccessfull_rename = 1;
                }
            }

            // To let all instructions whose rob values are already ready
            vector<bool> temp_values = ROB_controller.Get_All_ROB();
            vector<int> registers_that_are_ready;
            for (unsigned int indexing = 0; indexing < temp_values.size(); indexing++)
            {
                if (temp_values[indexing] == 1)
                    registers_that_are_ready.push_back((int)indexing);
            }
            if (registers_that_are_ready.size() != 0)
                RN.Set_Renamed_Register_Ready(registers_that_are_ready);
            // Ends here  To let all instructions whose rob values are already ready
            if ((RN.Get_Status_of_Pipeline() != -1) && (unsuccessfull_rename == 0))
            {
                // Get instruction from previous stage
                to_RR = RN.Get_and_Remove_Instructions_from_Register();
                empty_stage = 1;
                // Add instruction from the pervious stage
                stall_RN = RR.Add_Instructions_to_Register(to_RR, sim_time);
                if (DEBUG == 1)
                {
                    cout << "Regread Stage: ";
                    RR.Print_Instructions_in_Register();
                }
                // renamed = 0;
            }
            else
            {
                if (DEBUG == 1)
                    cout << "Abba, " << endl; // renamed << endl;
            }
        }
        else
        {
            if (DEBUG == 1)
                cout << "Stalling RN" << endl;
            stall_RN = 1;
        }
        RR.Increment_Time();

        // cout << "ROB has " << ROB_controller.Get_Availability_in_ROB() << endl; //", while renamed is " << renamed << endl;
        //  Another possibility for a bypass
        //  RR.Print_Timing();
        //  cout << "---------------------------- End RegRead Stage -------------------------------" << endl;
        /*************************************** End Regread Stage *****************************************/

        /******************************************* Rename Stage ******************************************/
        // cout << "---------------------------- Rename Stage -------------------------------" << endl;

        // if ((stall_RN != 1) && (RN.Get_Availability_of_Pipeline() == params.width)) // && (ROB_controller.Get_Availability_in_ROB() >= params.width)
        if ((stall_RN != 1)) // && (RN.Get_Availability_of_Pipeline() == params.width))
        {
            if ((DE.Get_Status_of_Pipeline() != -1))
            {
                // Get instruction from previous stage
                to_RN = DE.Get_and_Remove_Instructions_from_Register();
                // Add modified instruction from the pervious stage
                stall_DE = RN.Add_Instructions_to_Register(to_RN, sim_time);
                empty_stage = 0;
                if (DEBUG == 1)
                {
                    cout << "Prio Rename Stage: ";
                    RN.Print_Instructions_in_Register();
                }
            }
        }
        else
        {
            if (DEBUG == 1)
            {
                cout << "Stalled RN " << endl;
                cout << "Stalling DE" << endl;
                cout << "ROB availability: " << ROB_controller.Get_Availability_in_ROB() << endl;
            }
            stall_DE = 1;
        }

        if (RN.Get_Just_Availability() != params.width)
        {
            if (registers_to_set_ready.size() != 0)
                RN.Set_Renamed_Register_Ready(registers_to_set_ready);
        }
        RN.Increment_Time();
        // else{stall_DE = 1;}
        if (DEBUG == 1)
        {
            cout << "Rename Stage: ";
            RN.Print_Instructions_in_Register();
        }

        // RN.Print_Timing();
        // cout << "---------------------------- End Rename Stage -------------------------------" << endl;
        /**************************************** End Rename Stage *****************************************/

        /******************************************* Decode Stage ******************************************/
        // cout << "---------------------------- Decode Stage -------------------------------" << endl;

        if ((stall_DE != 1)) // && (DE.Get_Availability_of_Pipeline() == params.width))
        {
            if ((FE.Get_Status_of_Pipeline() != -1)) // && (renamed == 0))
            {
                // Get instruction from previous stage
                to_DE = FE.Get_and_Remove_Instructions_from_Register();
                // Add instruction from the pervious stage
                stall_FE = DE.Add_Instructions_to_Register(to_DE, sim_time);
                if (DEBUG == 1)
                {
                    cout << "Decode Stage: ";
                    DE.Print_Instructions_in_Register();
                }
            }
        }
        else
        // If no space available in DE, then stall the FE stage
        {
            if (DEBUG == 1)
                cout << "Stalling FE" << endl;
            stall_FE = 1;
        }
        DE.Increment_Time();

        // DE.Print_Timing();
        // cout << "------------------------- End Decode Stage -------------------------------" << endl;
        /**************************************** End Decode Stage *****************************************/

        /******************************************* Fetch Stage *******************************************/
        // cout << "---------------------------- Fetch Stage -------------------------------" << endl;
        uint8_t fetched_count = 0;
        FE.Get_Availability_of_Pipeline();
        // cout << "FE status: " <<  stall_FE << endl;
        bool testing = 1;
        if ((stall_FE != 1)) // && (ROB_controller.Get_Availability_in_ROB() != 0))
        {
            while (fetched_count < params.width)
            {
                if (fscanf(FP, "%lx %d %d %d %d", &pc, &op_type, &dest, &src1, &src2) != EOF)
                {
                    to_FE.push_back(Instruction_Structure{trace_no, pc, op_type, dest, src1, src2, -1, -1, -1, 0, 0, {0, 0}});
                    fetched_count++;
                    if (DEBUG == 1)
                        printf("Fetched: %d, %lx %d %d %d %d\n", trace_no, pc, op_type, dest, src1, src2); // Print to check if inputs have been read correctly
                    // Need to add to the fetch stage
                    FE.Add_Instructions_to_Register(to_FE, sim_time);
                    to_FE.clear();
                    trace_no++;
                    // testing = 0;
                }
                else
                {
                    fetched_all_instructions = 1;
                    break;
                }
            }
        }
        FE.Increment_Time();

        // FE.Print_Timing();
        // cout << "------------------------- End Fetch Stage -------------------------------" << endl;
        /**************************************** End Fetch Stage ******************************************/
        if (DEBUG == 1)
        {
            cout << "****************************************************************************************************************************" << endl;
            cout << "Sim time: " << sim_time << endl;
            cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << endl;
            Rename_Map_Table_controller.Print_RMT();
            cout << "?????????????????????????????????????????????????????????????????????????" << endl;
            IQ_controller.Print_IQ();
            cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" << endl;
            ROB_controller.Print_ROB();
            cout << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<" << endl;
            // cout << "---------------------------- Retire Stage -------------------------------" << endl;
            //  If ROB is not empty
            cout << "Available elements in EX: " << EX.Get_Just_Availability() << endl;
            cout << "Available elements in IQ: " << IQ_controller.Get_No_Available_Elements_in_IQ() << endl;
        }

        /******************************************* Retire Stage ******************************************/

        /************************************ Advance cycle calculation ************************************/
        // Increments the simulation time
        ++sim_time;
        // Check if simulation is completed
        if ((total_no_retired_instructions >= 10000) && (fetched_all_instructions == 1))
        {
            continue_sim = 0;
        }
        /********************************* End Advance cycle calculation ***********************************/
    } while (continue_sim);

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
    printf("\n# Instructions Per Cycle (IPC) = %.4f%%\n", (((double)total_no_retired_instructions) / ((double)sim_time)));
    /**************************************** Final Output Ends *******************************************/
    return 0;
}
