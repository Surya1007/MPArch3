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
    Pipeline_Stage_Operator RT(params.rob_size, 8); // May require change

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
    // bool empty_stage = 1;
    bool DEBUG = 1;
    // bool unsuccessfull_rename = 0;
    //  Instruction Trace file number
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

    // Variable to stall the previous pipeline stage

    bool STALL = 0;
    vector<bool> evicted_status;
    vector<unsigned int> evicted_tails;
    vector<Instruction_Structure> completed_instructions;
    vector<Instruction_Structure> TempInstructions_to_EX;
    vector<Selective_Removal_Struct> temp_structure;

    bool ready_to_retire = 1;
    // unsigned int lanja_called = 0;
    bool starting = 1;
    vector<int> registers_to_set_ready;
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
        registers_to_set_ready.clear();

        /********************************************** RETIRE ******************************************************/
        /*********************************************** TEST *******************************************************/

        /********************************************* END TEST *****************************************************/
        if ((RT.Get_Availability_of_Pipeline() != (params.rob_size)) && (ROB_controller.Get_Availability_in_ROB() != params.rob_size))
        {
            completed_instructions.clear();
            unsigned int no_of_retired_instructions = 0;
            bool no_instruction_to_retire = 1;
            // If instruction is ready to removed from RT
            while (no_of_retired_instructions < params.width)
            {
                // Check if there is any instruction to retire
                if (ROB_controller.Check_if_instruction_is_ready_to_retire() == 1)
                {
                    unsigned int rob_tail = ROB_controller.Get_Tail_from_ROB();
                    if (DEBUG == 1)
                        cout << "Tails is " << rob_tail << endl;
                    unsigned int to_be_remove_seq = ROB_controller.Get_SEQ_from_ROB(rob_tail);
                    Selective_Removal_Struct Removal_Status = RT.Search_Specific_Register_using_seq(to_be_remove_seq);
                    // If we got the earliest in order instruction
                    if (Removal_Status.success == 1)
                    {
                        // Get the instruction from RT
                        Selective_Removal_Struct Removed_Instruction = RT.Selective_Remove_Instruction(Removal_Status.instruction);
                        if (Removed_Instruction.success == 1)
                        {
                            completed_instructions.push_back(Removed_Instruction.instruction);
                            registers_to_set_ready.push_back(Removed_Instruction.instruction.renamed_dest);
                            total_no_retired_instructions++;
                            if (DEBUG == 1)
                            {
                                cout << "Totlal number of retired instructions: " << total_no_retired_instructions << endl;
                                cout << "Broadcasting: Here: " << Removed_Instruction.instruction.renamed_dest << endl;
                            }
                            no_of_retired_instructions++;
                            // Need to verify
                            if (Removed_Instruction.instruction.dest_register != -1)
                            {
                                IQ_controller.Set_SRC_Ready_Bit(Removed_Instruction.instruction.renamed_dest);
                                Individual_Rename_Map_Table_struct To_be_Removed = Rename_Map_Table_controller.Get_Rob_Tag_from_RMT(Removed_Instruction.instruction.dest_register);
                                if (Removed_Instruction.instruction.renamed_dest == To_be_Removed.rob_tag)
                                {
                                    Rename_Map_Table_controller.Reset_Rob_Tag_in_RMT(Removed_Instruction.instruction.dest_register);
                                }
                            }

                            bool incremented_tail = ROB_controller.Remove_Instruction_from_ROB();
                            if ((incremented_tail != 1) && (DEBUG == 1))
                            {
                                cout << "Really wrong" << endl;
                            }
                        }
                        else
                        {
                            cout << "Something is wrong" << endl;
                        }
                    }
                    else
                    {
                        break;
                        cout << "Unable to do it" << endl;
                    }
                }
                // If there is no instruction ready to retire
                else
                {
                    no_instruction_to_retire = 1;
                    break;
                }
            }
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
            }
        }
        /******************************************* END RETIRE *****************************************************/

        /********************************************** WRITEBACK ******************************************************/
        /*********************************************** TEST *******************************************************/

        /********************************************* END TEST *****************************************************/
        if (WB.Get_Availability_of_Pipeline() != (params.width * 5))
        {
            to_RT = WB.Get_and_Remove_Instructions_from_Register();
            if (to_RT.size() != 0)
            {
                // Mark the ready bit in ROB for all instructions entering RT
                for (unsigned int indexing = 0; indexing < to_RT.size(); indexing++)
                {
                    // cout << "Marked: " << to_RT[indexing].seq_no << endl;
                    ROB_controller.Mark_Instruction_Ready(to_RT[indexing].renamed_dest);
                    registers_to_set_ready.push_back(to_RT[indexing].renamed_dest);
                    // May or may not be required
                    IQ_controller.Set_SRC_Ready_Bit(to_RT[indexing].renamed_dest);
                }
                RT.Add_Instructions_to_Register(to_RT, sim_time);
                for (unsigned int indexing = 0; indexing < to_RT.size(); indexing++)
                    RT.Set_Ready_to_Move_Instruction(to_RT[indexing].seq_no);
            }
        }
        if (DEBUG == 1)
        {
            cout << "Retire Stage: " << endl;
            RT.Print_Instructions_in_Register();
        }
        /******************************************* END WRITEBACK *****************************************************/

        /********************************************** EXECUTE ******************************************************/
        /*********************************************** TEST *******************************************************/

        /********************************************* END TEST *****************************************************/
        if (EX.Get_Availability_of_Pipeline() != (params.width * 5))
        {
            // Search for completed instructions
            to_WB = EX.Search_for_Completed_Instructions();
            // If there is any instruction that has completed execution in this cycle
            if (to_WB.size() != 0)
            {
                WB.Add_Instructions_to_Register(to_WB, sim_time);
                for (unsigned int indexing = 0; indexing < to_WB.size(); indexing++)
                    WB.Set_Ready_to_Move_Instruction(to_WB[indexing].seq_no);
                for (unsigned int indexing = 0; indexing < to_WB.size(); indexing++)
                {
                    if (DEBUG == 1)
                        cout << "Broadcasting: " << to_WB[indexing].renamed_dest << endl;
                    // Notify registers in IQ
                    IQ_controller.Set_SRC_Ready_Bit(to_WB[indexing].renamed_dest);
                    registers_to_set_ready.push_back(to_WB[indexing].renamed_dest);
                    // Notify registers in DI
                    // Notify registers in RR
                }
            }
        }
        if (DEBUG == 1)
        {
            cout << "Writeback Stage: " << endl;
            WB.Print_Instructions_in_Register();
        }
        /******************************************* END EXECUTE *****************************************************/

        /********************************************** ISSUE ******************************************************/
        /*********************************************** TEST *******************************************************/

        /********************************************* END TEST *****************************************************/
        if ((IQ_controller.Get_No_Available_Elements_in_IQ() != params.iq_size) && (IS.Get_Availability_of_Pipeline() != params.iq_size))
        {
            if (registers_to_set_ready.size() != 0)
            {
                IS.Set_Renamed_Register_Ready(registers_to_set_ready);
            }
            vector<Selective_Removal_Struct> temp_to_EX;
            // Checking if there is any available slot in EX
            if (EX.Get_Availability_of_Pipeline() >= params.width)
            {
                to_EX.clear();
                // Find the instructions that are ready to execute
                temp_to_EX.clear();
                temp_to_EX = IQ_controller.Query_for_Oldest_Instructions_from_IQ(EX.Get_Availability_of_Pipeline(), params.width);
                // If there is atleast one instruction that is ready to execute
                if (temp_to_EX.size() != 0)
                {
                    for (unsigned int indexing = 0; indexing < temp_to_EX.size(); indexing++)
                    {
                        if (temp_to_EX[indexing].success == 1)
                        {
                            // cout << "No issue here " << temp_to_EX[indexing].instruction.seq_no << endl;
                            Selective_Removal_Struct Status_IS = IS.Pseudo_Selective_Remove_Instruction(temp_to_EX[indexing].instruction);
                            if (Status_IS.success == 1)
                            {
                                to_EX.push_back(Status_IS.instruction);
                                IS.Selective_Remove_Instruction(Status_IS.instruction);
                                IQ_controller.Remove_Instruction_from_IQ(Status_IS.instruction.seq_no);
                            }
                        }
                    }
                    EX.Add_Instructions_to_Register(to_EX, sim_time);
                    for (unsigned int indexing = 0; indexing < to_EX.size(); indexing++)
                        EX.Set_Ready_to_Move_Instruction(to_EX[indexing].seq_no);
                }
            }
        }
        if (DEBUG == 1)
        {
            cout << "Execute Stage: " << endl;
            EX.Print_Instructions_in_Register();
        }
        /******************************************* END ISSUE *****************************************************/

        /********************************************** DISPATCH ******************************************************/
        /*********************************************** TEST *******************************************************/

        /********************************************* END TEST *****************************************************/
        if (DI.Get_Status_of_Pipeline() == params.width)
        {

            if (registers_to_set_ready.size() != 0)
            {
                DI.Set_Renamed_Register_Ready(registers_to_set_ready);
            }
            if (DEBUG == 1)
            {
                cout << "TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT" << endl;
                cout << "IQ are: " << IS.Get_Availability_of_Pipeline() << ", and " << IQ_controller.Get_No_Available_Elements_in_IQ() << endl;
            }
            if ((IS.Get_Availability_of_Pipeline() >= params.width) && (IQ_controller.Get_No_Available_Elements_in_IQ() >= params.width)) // Essentially both should have the same size
            {
                to_IS = DI.Get_and_Remove_Instructions_from_Register();
                for (unsigned int indexing = 0; indexing < to_IS.size(); indexing++)
                {
                    IQ_controller.Add_Instruction_to_IQ(to_IS[indexing]);
                }
                IS.Add_Instructions_to_Register(to_IS, sim_time);
                for (unsigned int indexing = 0; indexing < to_IS.size(); indexing++)
                    IS.Set_Ready_to_Move_Instruction(to_IS[indexing].seq_no);
                // Notifying registers in Issue stage
                if (registers_to_set_ready.size() != 0)
                {
                    IS.Set_Renamed_Register_Ready(registers_to_set_ready);
                }
            }
        }
        if (DEBUG == 1)
        {
            cout << "Issue Stage: " << endl;
            IS.Print_Instructions_in_Register();
        }
        /******************************************* END DISPATCH *****************************************************/

        /********************************************** REG READ ******************************************************/
        /*********************************************** TEST *******************************************************/

        /********************************************* END TEST *****************************************************/
        if (RR.Get_Status_of_Pipeline() == params.width)
        {
            if (registers_to_set_ready.size() != 0)
            {
                RR.Set_Renamed_Register_Ready(registers_to_set_ready);
            }
            // Check for the readiness of the ROB registers in RR bundle -------------------------------------------------------------------------------------------------------------------------------------
            if (DI.Get_Availability_of_Pipeline() == params.width)
            {
                to_DI = RR.Get_and_Remove_Instructions_from_Register();
                DI.Add_Instructions_to_Register(to_DI, sim_time);
                if (registers_to_set_ready.size() != 0)
                {
                    DI.Set_Renamed_Register_Ready(registers_to_set_ready);
                }
                for (unsigned int indexing = 0; indexing < to_DI.size(); indexing++)
                    DI.Set_Ready_to_Move_Instruction(to_DI[indexing].seq_no);
            }
        }
        else
        {
            // cout << "Yallaa" << endl;
            // STALL = 1;
        }
        if (DEBUG == 1)
        {
            cout << "Dispatch Stage: " << endl;
            DI.Print_Instructions_in_Register();
        }
        /******************************************* END REG READ *****************************************************/

        /********************************************** RENAME ******************************************************/
        /*********************************************** TEST *******************************************************/

        /********************************************* END TEST *****************************************************/
        if (RN.Get_Status_of_Pipeline() == params.width)
        {
            if ((RR.Get_Availability_of_Pipeline() == params.width) && (ROB_controller.Get_Availability_in_ROB() >= params.width)) // May need to be changed for last batch
            {
                to_RR = RN.Get_and_Remove_Instructions_from_Register();
                // Adding instructions one by one
                for (unsigned int indexing = 0; indexing < to_RR.size(); indexing++)
                {
                    // Allocate ROB entry
                    unsigned int rob_header = ROB_controller.Add_Instruction_to_ROB(to_RR[indexing].dest_register, to_RR[indexing].PC, to_RR[indexing].seq_no);
                    ROB_controller.Increment_header();
                    // Rename Source 1
                    if (to_RR[indexing].src1_register != -1)
                    {
                        Individual_Rename_Map_Table_struct src1 = Rename_Map_Table_controller.Get_Rob_Tag_from_RMT(to_RR[indexing].src1_register);
                        if (src1.valid == 1)
                        {
                            to_RR[indexing].renamed_src1 = src1.rob_tag;
                            // Need to check ROB if the element is ready or not
                            to_RR[indexing].src1_ready_status = ROB_controller.Check_Status_of_Entry(src1.rob_tag);
                        }
                    }
                    // Rename Source 2
                    if (to_RR[indexing].src2_register != -1)
                    {
                        Individual_Rename_Map_Table_struct src2 = Rename_Map_Table_controller.Get_Rob_Tag_from_RMT(to_RR[indexing].src2_register);
                        if (src2.valid == 1)
                        {
                            to_RR[indexing].renamed_src2 = src2.rob_tag;
                            to_RR[indexing].src2_ready_status = ROB_controller.Check_Status_of_Entry(src2.rob_tag);
                        }
                    }
                    // Rename Destination
                    to_RR[indexing].renamed_dest = rob_header;
                    // Make a new entry in RMT
                    if (to_RR[indexing].dest_register != -1)
                    {
                        Rename_Map_Table_controller.Set_Rob_Tag_in_RMT(to_RR[indexing].dest_register, rob_header);
                    }
                }
                RR.Add_Instructions_to_Register(to_RR, sim_time);
                // cout << "PPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPP" << endl;
                // if (registers_to_set_ready.size() != 0)
                {
                    //    RR.Set_Renamed_Register_Ready(registers_to_set_ready);
                }
                for (unsigned int indexing = 0; indexing < to_RR.size(); indexing++)
                    RR.Set_Ready_to_Move_Instruction(to_RR[indexing].seq_no);
                STALL = 0;
            }
            else
            {
                // cout << "Koun" << endl;
            }
        }
        if (DEBUG == 1)
        {
            cout << "Reg read Stage: " << endl;
            RR.Print_Instructions_in_Register();
            cout << "############################################################################################################" << endl;
        }
        /******************************************* END RENAME *****************************************************/

        /********************************************** DECODE ******************************************************/
        /*********************************************** TEST *******************************************************/

        /********************************************* END TEST *****************************************************/
        if ((DE.Get_Status_of_Pipeline() == params.width)) // May need to be changed for superscalar or not :P
        {
            if ((RN.Get_Availability_of_Pipeline() == params.width))
            {
                to_RN = DE.Get_and_Remove_Instructions_from_Register();
                RN.Add_Instructions_to_Register(to_RN, sim_time);
                if (registers_to_set_ready.size() != 0)
                {
                    RN.Set_Renamed_Register_Ready(registers_to_set_ready);
                }
                for (unsigned int indexing = 0; indexing < to_RN.size(); indexing++)
                    RN.Set_Ready_to_Move_Instruction(to_DE[indexing].seq_no);
            }
        }
        else
        {
            cout << "What heppen" << endl;
        }
        if (DEBUG == 1)
        {
            cout << "Rename Stage: " << endl;
            RN.Print_Instructions_in_Register();
        }
        /******************************************* END DECODE *****************************************************/

        /********************************************** FETCH ******************************************************/
        /*********************************************** TEST *******************************************************/

        /********************************************* END TEST *****************************************************/
        if ((starting != 1) && (FE.Get_Availability_of_Pipeline() != params.width))
        {
            if ((DE.Get_Availability_of_Pipeline() == params.width) || (fetched_all_instructions == 1))
            {
                to_DE = FE.Get_and_Remove_Instructions_from_Register();
                DE.Add_Instructions_to_Register(to_DE, sim_time);
                if (to_DE.size() != 0)
                {
                    for (unsigned int indexing = 0; indexing < to_DE.size(); indexing++)
                        DE.Set_Ready_to_Move_Instruction(to_DE[indexing].seq_no);
                }
            }
            else
            {
                STALL = 1;
            }
        }
        if (DEBUG == 1)
        {
            cout << "Decode Stage: " << endl;
            DE.Print_Instructions_in_Register();
        }
        /******************************************* END FETCH *****************************************************/
        /*********************************************** TEST *******************************************************/
        if (DEBUG == 1)
        {
            cout << "No problem at " << ROB_controller.Get_Availability_in_ROB() << endl;
            cout << "DI availability is: " << DI.Get_Status_of_Pipeline() << endl;
            cout << "RR availability is: " << RR.Get_Status_of_Pipeline() << endl;
            cout << "RN availability is: " << RN.Get_Status_of_Pipeline() << endl;
            cout << "DE availability is: " << DE.Get_Status_of_Pipeline() << endl;
            // cout << "RR availability is: " << RR.Get_Availability_of_Pipeline() << endl;
        }
        /********************************************* END TEST *****************************************************/

        if (DEBUG == 1)
        {
            cout << "RT are: " << RT.Get_Availability_of_Pipeline() << endl;
            cout << "WB are: " << WB.Get_Availability_of_Pipeline() << endl;
            cout << "EX are: " << EX.Get_Availability_of_Pipeline() << endl;
            cout << "IQ are: " << IS.Get_Availability_of_Pipeline() << ", and " << IQ_controller.Get_No_Available_Elements_in_IQ() << endl;
            cout << "DI are: " << DI.Get_Availability_of_Pipeline() << ", whose are: " << DI.Get_Status_of_Pipeline() << endl;
            cout << "RR are: " << RR.Get_Availability_of_Pipeline() << ", whose are: " << RR.Get_Status_of_Pipeline() << endl;
            cout << "RN are: " << RN.Get_Availability_of_Pipeline() << ", whose are: " << RN.Get_Status_of_Pipeline() << endl;
            cout << "DE are: " << DE.Get_Availability_of_Pipeline() << ", whose are: " << DE.Get_Status_of_Pipeline() << endl;
            cout << "ROB are: " << ROB_controller.Get_Availability_in_ROB() << endl;
        }

        unsigned int tot_inst_may_be_removed = 0; // Counts the number of spaces in ROB might be created in the next cycle.
        bool starting_removed = 0;
        unsigned int rob_tail = ROB_controller.Get_Tail_from_ROB();
        unsigned int blocks_to_be_cleared = 0;
        int search_through = params.width - ROB_controller.Get_Availability_in_ROB();
        // cout << "Search through is: " << search_through << endl;
        if (search_through > 0)
        {
            for (unsigned int incremented_tail = 0; incremented_tail < (unsigned)search_through; incremented_tail++)
            {
                // cout << "Tail is " << rob_tail << endl;
                unsigned int to_be_remove_seq = ROB_controller.Get_SEQ_from_ROB(rob_tail);
                Selective_Removal_Struct Removal_Status = RT.Search_Specific_Register_using_seq(to_be_remove_seq);
                //(IQ_controller.Get_No_Available_Elements_in_IQ() == 0)
                // The need to stall
                if ((Removal_Status.success == 1))
                {
                    if (incremented_tail == 0)
                    {
                        starting_removed = 1;
                        if (DEBUG == 1)
                            cout << "Can Fetch" << endl;
                    }
                    ready_to_retire = 1;
                    tot_inst_may_be_removed++;
                }
                else
                {
                    ready_to_retire = 0;
                }
                rob_tail++;
            }
        }

        // cout << "tot_inst_may_be_removed is " << tot_inst_may_be_removed << ", while: " << params.width << endl;
        if (tot_inst_may_be_removed < params.width)
        {
            // cout << "ROB Availability: " << (ROB_controller.Get_Availability_in_ROB() + tot_inst_may_be_removed) << endl;
            //  Check if ROB can accomodate new instructions
            if (((ROB_controller.Get_Availability_in_ROB() + tot_inst_may_be_removed) < params.width))
            {
                // if ((DI.Get_Availability_of_Pipeline() == params.width) && (RR.Get_Availability_of_Pipeline() == params.width))
                STALL = 1;
            }
            else
            {
                STALL = 0;
            }
        }
        else
        {
            STALL = 0;
        }
        cout << "Hello " << endl;

        if (STALL == 0)
        {
            vector<Selective_Removal_Struct> checking_status = IQ_controller.Query_for_Oldest_Instructions_from_IQ(EX.Get_Availability_of_Pipeline(), params.width);
            unsigned int to_be_issued_next = 0;
            if (checking_status.size() != 0)
            {
                for (unsigned int indexing = 0; indexing < checking_status.size(); indexing++)
                {
                    cout << "Index is " << indexing << endl;
                    if (checking_status[indexing].success == 1)
                    {
                        to_be_issued_next++;
                    }
                }
            }
            if ((DE.Get_Availability_of_Pipeline() != params.width) && (ROB_controller.Get_Availability_in_ROB() != 0)) 
            {
                cout << "Moshi moshi" << endl;
                cout << "IQ Availability: " << IQ_controller.Get_No_Available_Elements_in_IQ() << " and " << (IQ_controller.Get_No_Available_Elements_in_IQ() + to_be_issued_next) << endl;
                if ((RN.Get_Availability_of_Pipeline() < params.width) && (RR.Get_Availability_of_Pipeline() < params.width))
                {
                    if (((IQ_controller.Get_No_Available_Elements_in_IQ() + to_be_issued_next) < params.width)) // && (RN.Get_Availability_of_Pipeline() == params.width))
                    {
                        if (DI.Get_Availability_of_Pipeline() != params.width)
                        {
                            cout << "Stalling " << endl;
                            STALL = 1;
                        }
                    }
                    else
                    {
                        cout << "Gone jhere " << endl;
                        // Write code here, this is working fine, just tune this
                        if (DE.Get_Availability_of_Pipeline() < params.width)
                        {
                            cout << "Hmm.... " << endl;
                            if ((DI.Get_Status_of_Pipeline() == -1) && (RR.Get_Status_of_Pipeline() == -1) && (RR.Get_Availability_of_Pipeline() == params.width) && (IQ_controller.Get_No_Available_Elements_in_IQ() <= params.width) && (WB.Get_Availability_of_Pipeline() == (params.width)))
                            {
                                cout << "Alright" << endl;
                                STALL = 1;
                            }
                        }
                    }
                }
            }
        }

        if ((STALL != 1))
        {
            uint8_t fetched_count = 0;
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
                    for (unsigned int indexing = 0; indexing < to_FE.size(); indexing++)
                        FE.Set_Ready_to_Move_Instruction(to_FE[indexing].seq_no);
                    starting = 0;
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
        if (DEBUG == 1)
        {
            cout << "Fetch Stage: " << endl;
            FE.Print_Instructions_in_Register();
        }

        RT.Increment_Time();
        WB.Increment_Time();
        EX.Increment_Time();
        IS.Increment_Time();
        DI.Increment_Time();
        RR.Increment_Time();
        RN.Increment_Time();
        DE.Increment_Time();
        FE.Increment_Time();

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
            cout << "Available elements in EX: " << EX.Get_Just_Availability() << endl;
            cout << "Available elements in IQ: " << IQ_controller.Get_No_Available_Elements_in_IQ() << endl;
        }

        if (DEBUG == 1)
            cout << "HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH" << endl;
        /************************************ Advance cycle calculation ************************************/
        // Increments the simulation time
        ++sim_time;
        if (trace_no >= 10000)
        {
            fetched_all_instructions = 1;
        }
        // cout << "TRACENO is : " << trace_no << ", while " << total_no_retired_instructions << endl;
        //   Check if simulation is completed
        if ((total_no_retired_instructions >= 10000) && (fetched_all_instructions == 1))
        {
            continue_sim = 0;
        }
        /********************************* End Advance cycle calculation ***********************************/
    } while (continue_sim);
    sim_time -= 1;

    /******************************************* Final Output *********************************************/
    printf("# === Simulator Command =========");
    printf("\n# ./sim %lu %lu %lu %s", params.rob_size, params.iq_size, params.width, trace_file);
    printf("\n# === Processor Configuration ===");
    printf("\n# ROB_SIZE = %lu", params.rob_size);
    printf("\n# IQ_SIZE  = %lu", params.iq_size);
    printf("\n# WIDTH    = %lu", params.width);
    printf("\n# === Simulation Results ========");
    printf("\n# Dynamic Instruction Count    = %d", total_no_retired_instructions);
    printf("\n# Cycles                       = %d", sim_time);
    printf("\n# Instructions Per Cycle (IPC) = %.2f%%\n", (((double)total_no_retired_instructions) / ((double)sim_time)));
    /**************************************** Final Output Ends *******************************************/
    return 0;
}
