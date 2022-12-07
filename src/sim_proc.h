#ifndef SIM_PROC_H
#define SIM_PROC_H

#include <vector>
#include <iostream>

using namespace std;

typedef struct proc_params
{
    unsigned long int rob_size;
    unsigned long int iq_size;
    unsigned long int width;
} proc_params;

// Put additional data structures here as per your requirement

/**************************** Timing Information Structure **************************
 * Stores the timing based information at each stage                                *
 *********************** End Timing Information Structure ***************************/
typedef struct Timing_Information_Structure
{
    unsigned int start_time_at_each_stage[9];
    unsigned int duration_at_each_stage[9];
} Timing_Information_Structure;

/****************************** Instruction  Structure ******************************
 * Structure for storing a single instruction                                        *
 ***************************** End Instruction  Structure ***************************/

typedef struct Instruction_Structure
{
    unsigned int seq_no;
    unsigned long PC;
    int op_type;
    int dest_register;
    int src1_register;
    int src2_register;
    // if renamed_registers value < 0, then no renaming happened
    int renamed_dest;
    int renamed_src1;
    int renamed_src2;
    bool src1_ready_status;
    bool src2_ready_status;
    Timing_Information_Structure time_info;
} Instruction_Structure;

/************************** Individual Rename Map Table Struct **********************
 * Structure for individual element in Rename Map Table                              *
 ************************* End Rename Map Table Struct ******************************/
typedef struct Individual_Rename_Map_Table_struct
{
    int rob_tag;
    bool valid;
} Individual_Rename_Map_Table_struct;

/***************************** Individual Reorder Buffer ****************************
 * Structure for individual entry in Reorder Buffer                                  *
 **************************** End Individual Reorder Buffer *************************/
typedef struct Individual_ROB_struct
{
    int value;
    int destination;
    bool ready;
    bool exception;
    bool misprediction;
    unsigned int PC;
    unsigned int seq_no;
} Individual_ROB_struct;

/***************************** Individual Issue Queue *******************************
 * Structure for individual entry in Issue Queue                                     *
 **************************** End Individual Issue Queue ****************************/
typedef struct Individual_Issue_Queue_struct
{
    bool valid_bit;
    bool src1_ready_bit;
    bool src2_ready_bit;
    Instruction_Structure instruction;
} Individual_Issue_Queue_struct;

typedef struct Selective_Removal_Struct
{
    bool success;
    Instruction_Structure instruction;
} Selective_Removal_Struct;

/************************** Rename Map Table Operator *******************************
 * Structure for Rename Map Table                                                    *
 * The index of arrays represents the register number                                *
 ************************* End Rename Map Table Operator ****************************/
class Rename_Map_Table_Operator
{
private:
    unsigned int total_no_of_registers;
    vector<Individual_Rename_Map_Table_struct> Rename_Map_Table_vector;

public:
    Rename_Map_Table_Operator(unsigned int no_of_registers)
    {
        total_no_of_registers = no_of_registers;
        Rename_Map_Table_vector = vector<Individual_Rename_Map_Table_struct>(no_of_registers, {0, 0});
    }

    /*************************** set_rob_tag ********************************************
     * For a given register register_no                                                  *
     * Sets the rob tag in the Rename Map Table                                          *
     * Sets the valid bit to 1                                                           *
     ********************** End set_rob_tag *********************************************/
    void Set_Rob_Tag_in_RMT(int register_no, unsigned int rob_index)
    {
        Rename_Map_Table_vector[register_no].valid = 1;
        Rename_Map_Table_vector[register_no].rob_tag = rob_index;
    }

    /************************* reset_rob_tag ********************************************
     * For a given register register_no                                                  *
     * Sets the valid bit to 0                                                           *
     ******************** End reset_rob_tag *********************************************/
    void Reset_Rob_Tag_in_RMT(unsigned int register_no)
    {
        Rename_Map_Table_vector[register_no].valid = 0;
    }

    /*************************** get_rob_tag ********************************************
     * Returns the Individual_Rename_Map_Table_struct for a given register register_no   *
     ********************** End get_rob_tag *********************************************/
    Individual_Rename_Map_Table_struct Get_Rob_Tag_from_RMT(unsigned int register_no)
    {
        return Rename_Map_Table_vector[register_no];
    }

    void Print_RMT()
    {
        cout << "Rename Map Table" << endl;
        for (unsigned int indexing = 0; indexing < total_no_of_registers; indexing++)
        {
            if (Rename_Map_Table_vector[indexing].valid == 1)
            {
                cout << "R" << indexing << ": "
                     << "ROB" << Rename_Map_Table_vector[indexing].rob_tag << endl;
            }
        }
        cout << "Rename Map Table Ends Here" << endl;
    }
};

/*********************************** Issue Queue ************************************
 * Class for storing issue queue size, actual issue queue                            *
 * Available functions for performing operations in Issue Queue :                    *
 *                                                                                   *
 *********************************** End Issue Queue ********************************/
class IssueQueue_Operator
{
private:
    unsigned int issue_queue_size;
    vector<Individual_Issue_Queue_struct> issue_queue;
    unsigned int available_iq_elements;

public:
    /* Constructor for the class to initialize the issue queue of size iq_size */
    IssueQueue_Operator(unsigned int iq_size)
    {
        issue_queue_size = iq_size;
        issue_queue = vector<Individual_Issue_Queue_struct>(iq_size);
        available_iq_elements = iq_size;
    }

    /************************************
     Returns the available / free entries in IQ
     ****************************************/
    unsigned int Get_No_Available_Elements_in_IQ()
    {
        return available_iq_elements;
    }

    void Print_IQ()
    {
        cout << "Issue Queue" << endl;
        cout << "SEQNO\tDST\tRS1RDY\tRS1\tRS2RDY\tRS2" << endl;
        for (unsigned int indexing = 0; indexing < issue_queue_size; indexing++)
        {
            if (issue_queue[indexing].valid_bit == 1)
            {
                cout << issue_queue[indexing].instruction.seq_no << "\tROB" << issue_queue[indexing].instruction.renamed_dest << "\t" << issue_queue[indexing].src1_ready_bit
                     << "\tROB" << issue_queue[indexing].instruction.renamed_src1 << "\t" << issue_queue[indexing].src2_ready_bit << "\tROB" << issue_queue[indexing].instruction.renamed_src2 << endl;
            }
        }
        cout << "Issue Queue Ends Here" << endl;
    }

    /****************************** Add_Instruction_to_IQ *******************************
     * Adds an instruction to the issue queue                                            *
     * Returns:                                                                          *
     *           1: Successfully added instruction to issue queue                        *
     *           0: Issue queue is full                                                  *
     *************************** End Add_Instruction_to_IQ ******************************/
    bool Add_Instruction_to_IQ(Instruction_Structure instruction_to_be_added)
    {
        // Add only when the number of instructions is equal to WIDTH size needs to be checked in the main function
        unsigned int indexing;
        for (indexing = 0; indexing < issue_queue_size; indexing++)
        {
            // Adds only into empty entries
            if (issue_queue[indexing].valid_bit == 0)
            {
                issue_queue[indexing].instruction = instruction_to_be_added;
                // Check for the ready bits of src1 and src2 registers
                if ((issue_queue[indexing].instruction.renamed_src1 == -1) || (issue_queue[indexing].instruction.src1_ready_status == 1))
                {
                    issue_queue[indexing].src1_ready_bit = 1;
                }
                else
                {
                    issue_queue[indexing].src1_ready_bit = 0;
                }
                if ((issue_queue[indexing].instruction.renamed_src2 == -1) || (issue_queue[indexing].instruction.src2_ready_status == 1))
                {
                    issue_queue[indexing].src2_ready_bit = 1;
                }
                else
                {
                    issue_queue[indexing].src2_ready_bit = 0;
                }
                issue_queue[indexing].valid_bit = 1;
                --available_iq_elements;
                return 1;
            }
        }

        // For checking_printing
        // cout << "\n Error(IQ): Issuequeue is full, so please check if there is already any barrier before this: " << available_iq_elements << endl;
        // End checking_printing
        return 0;
    }

    /******************** Get_Oldest_Instructions_from_IQ *******************************
     * Gets the oldest instruction from the issue queue                                  *
     * Inputs:                                                                           *
     *       size_to_get: Number of instructions to be removed from the IQ               *
     * Returns:                                                                          *
     *           Vector of instructions that are ready to execute                        *
     ***************** End Get_Oldest_Instructions_from_IQ ******************************/
    Selective_Removal_Struct Query_for_Oldest_Single_Instruction_from_IQ(vector<unsigned int> found_already)
    {
        unsigned int mini_seq_no = 100000000;
        unsigned int smallest_seq = 0;
        Selective_Removal_Struct instruction_temp;
        instruction_temp.success = 0;
        for (unsigned int indexing = 0; indexing < issue_queue.size(); indexing++)
        {
            // Search through the valid bits only
            if (issue_queue[indexing].valid_bit == 1)
            {
                bool found = 0;
                for (unsigned int sub_ind = 0; sub_ind < found_already.size(); sub_ind++)
                {
                    if (found_already[sub_ind] == issue_queue[indexing].instruction.seq_no)
                    {
                        found = 1;
                    }
                }
                if (found != 1)
                {
                    //cout << "FOre seq no: " << issue_queue[indexing].instruction.seq_no << ", and Checking ready bits, " << issue_queue[indexing].src1_ready_bit << ", while" << issue_queue[indexing].src2_ready_bit << endl;
                    //  Checking if both the src registers are ready
                    if ((issue_queue[indexing].src1_ready_bit == 1) && (issue_queue[indexing].src2_ready_bit == 1))
                    {
                        // Finding the index of instruction that is the oldest in the IQ
                        if (issue_queue[indexing].instruction.seq_no < mini_seq_no)
                        {
                            mini_seq_no = issue_queue[indexing].instruction.seq_no;
                            // cout << "Got : " << mini_seq_no << endl;
                            instruction_temp.success = 1;
                            instruction_temp.instruction = issue_queue[indexing].instruction;
                            if (instruction_temp.instruction.seq_no < smallest_seq)
                            {
                                smallest_seq = instruction_temp.instruction.seq_no;
                            }
                        }
                    }
                }
            }
        }
        return instruction_temp;
        ;
    }

    vector<Selective_Removal_Struct> Query_for_Oldest_Instructions_from_IQ(unsigned int available_execution, unsigned int pipe_line_width)
    {
        std::vector<Selective_Removal_Struct> Instructions_to_be_returned;
        // Can only add instructions of size size_to_get
        bool nothing_found = 1;
        vector<unsigned int> elements_found;
        elements_found.clear();
        while (Instructions_to_be_returned.size() <= available_execution)
        {
            // Searching through valid instructions in the IQ
            Selective_Removal_Struct temp = Query_for_Oldest_Single_Instruction_from_IQ(elements_found);
            if (temp.success == 0)
            {
                break;
            }
            elements_found.push_back(temp.instruction.seq_no);
            Instructions_to_be_returned.push_back(temp);
            if (Instructions_to_be_returned.size() >= pipe_line_width)
            {
                break;
            }
        }
        if (nothing_found == 1)
        {
            //    cout << "Error(IQ): Check for better barrier possibility" << endl;
        }
        return Instructions_to_be_returned;
    }

    Selective_Removal_Struct Remove_Instruction_from_IQ(unsigned int seq_no)
    {
        Selective_Removal_Struct Instruction_to_be_returned;

        for (unsigned int indexing = 0; indexing < issue_queue_size; indexing++)
        {
            // Loop through valid elements of IQ only
            if (issue_queue[indexing].valid_bit == 1)
            {
                // If the required instruction is found
                if (seq_no == issue_queue[indexing].instruction.seq_no)
                {
                    // Just setting the valid bit to 0 works
                    issue_queue[indexing].valid_bit = 0;
                    // Increment the available elements in IQ for a safe addition to IQ
                    ++available_iq_elements;

                    Instruction_to_be_returned.success = 1;
                    Instruction_to_be_returned.instruction = issue_queue[indexing].instruction;
                    return Instruction_to_be_returned;
                }
                else
                {
                    // cout << "Uhhhh " << seq_no << endl;
                }
            }
        }
        // cout << "Error(IQ): Trying to remove instruction that is not part of IQ" << endl;
        return Instruction_to_be_returned;
    }

    /***************************** Set_SRC_Ready_Bit ************************************
     * Gets the oldest instruction from the issue queue                                  *
     * Inputs:                                                                           *
     *       src_register: The register that has just been ready                         *
     * Returns:                                                                          *
     *           Nothing                                                                 *
     ************************** End Set_SRC_Ready_Bit ***********************************/
    void Set_SRC_Ready_Bit(int src_register)
    {
        for (unsigned int indexing = 0; indexing < issue_queue_size; indexing++)
        {
            // Set the ready bits of only that are valid
            if (issue_queue[indexing].valid_bit == 1)
            {
                //cout << "SRCs: " << issue_queue[indexing].instruction.renamed_src1 << " with: " << issue_queue[indexing].instruction.renamed_src2 << endl;
                //  Sets the src1 ready bit if the renamed register is src_register
                if (issue_queue[indexing].instruction.renamed_src1 == src_register)
                {
                    issue_queue[indexing].src1_ready_bit = 1;
                }
                // Sets the src2 ready bit if the renamed register is src_register
                if (issue_queue[indexing].instruction.renamed_src2 == src_register)
                {
                    issue_queue[indexing].src2_ready_bit = 1;
                }
            }
        }
        // cout << "To set: " << src_register << endl;
        // cout << "Error(IQ): Unable to find the required instruction to set the ready bits" << endl;
    }
};

/********************************** Reorder Buffer **********************************
 * Class for storing issue queue size, actual issue queue                            *
 * Available functions for performing operations in Issue Queue :                    *
 *                                                                                   *
 ********************************** End Reorder Buffer ******************************/
class ROB_Operator
{
private:
    unsigned int rob_size;
    vector<Individual_ROB_struct> ROB;
    unsigned int header;
    unsigned int tail;
    unsigned long int no_of_available_elements_in_rob;

public:
    /* Constructor for the class to initialize the reorder buffer of size robsize */
    ROB_Operator(unsigned int robsize)
    {
        rob_size = robsize;
        ROB = vector<Individual_ROB_struct>(robsize);
        no_of_available_elements_in_rob = robsize;
        header = 0; // Changed when elements are added into the rob
        tail = 0;   // Changed when elements are removed from rob
        // Ensure that header and tail is only equal at initialization
    }

    vector<bool> Get_All_ROB()
    {
        vector<bool> to_be_returned;
        for (unsigned int indexing = 0; indexing < ROB.size(); indexing++)
        {
            to_be_returned.push_back(ROB[indexing].ready);
        }
        return to_be_returned;
    }

    /********************** Add Instructions to ROB *************************************
     * Adds a vector of Instructions of type Instruction_Structure                      *
     * Returns:                                                                         *
     *          1: If operation is successfull                                          *
     *          0: ROB is full, not possible to add instructions in this cycle          *
     ******************** End Add Instructions to ROB ***********************************/
    unsigned int Add_Instruction_to_ROB(int corresponding_register, unsigned long PC, unsigned int seq_no)
    {
        // If reached the end of rob, set the header to 0
        ROB[header].PC = PC;
        ROB[header].destination = corresponding_register;
        ROB[header].seq_no = seq_no;
        no_of_available_elements_in_rob--;
        // cout << "Header value is " << header << endl;
        return header;
    }

    void Increment_header()
    {
        header++;
        if (header >= rob_size)
        {
            header -= rob_size;
        }
    }

    void Print_ROB()
    {
        cout << "ROB" << endl;
        cout << "Header at: " << header << endl;
        cout << "Pointer at: " << tail << endl;
        for (unsigned int indexing = 0; indexing < rob_size; indexing++)
        {
            cout << indexing << "\t" << ROB[indexing].destination << "\t" << ROB[indexing].ready;
            if (indexing == header)
                cout << "\tH";
            if (indexing == tail)
                cout << "\tT";
            cout << endl;
        }
        cout << "ROB Ends Here" << endl;
    }
    /********************* Remove Instruction from ROB **********************************
     * Removes an instruction from ROB                                                  *
     * Returns:                                                                         *
     *          1: Successfully removed the instruction from ROB                        *
     *          0: Not possible to remove instruction from ROB                          *
     ***************** End Remove Instruction from ROB **********************************/
    bool Remove_Instruction_from_ROB()
    {
        if (ROB[tail].ready == 1)
        {
            if (no_of_available_elements_in_rob != rob_size)
            {
                no_of_available_elements_in_rob++;
                ROB[tail].destination = 0;
                ROB[tail].ready = 0;
                tail++;
                if (tail >= rob_size)
                    tail -= rob_size;
                // cout << "Okay, tail value is " << tail << endl;
                return 1;
            }
            else
            {
                return 0;
            }
        }
        else
        {
            // cout << "Kyaaaa" << endl;
            return 0;
        }
    }

    bool Check_if_instruction_is_ready_to_retire()
    {
        // cout << "BITCH IS " << tail << endl;
        if (ROB[tail].ready == 1)
        {
            if (no_of_available_elements_in_rob != rob_size)
            {
                return 1;
            }
            else
            {
                return 0;
            }
        }
        else
        {
            // cout << "Kyaaaa" << endl;
            return 0;
        }
    }

    /************************** Mark_Instruction_Ready **********************************
     * Marks an instruction ready in  ROB, using rob_to_be_marked_as_rdy as index:      *
     * Returns:                                                                         *
     *          Nothing                                                                 *
     ********************** End Mark_Instruction_Ready **********************************/
    void Mark_Instruction_Ready(unsigned int rob_to_be_marked_as_rdy)
    {
        ROB[rob_to_be_marked_as_rdy].ready = 1;
    }

    bool Check_Status_of_Entry(unsigned int rob_index)
    {
        if (ROB[rob_index].ready == 1)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }

    unsigned long int Get_Availability_in_ROB()
    {
        return no_of_available_elements_in_rob;
    }

    unsigned int Get_Tail_from_ROB()
    {
        // cout << "Tail is: " << tail << endl;
        return tail;
    }

    unsigned int Get_SEQ_from_ROB(unsigned int rob_value)
    {
        return ROB[rob_value].seq_no;
    }
    unsigned long Get_PC_from_ROB(unsigned int rob_value)
    {
        return ROB[rob_value].PC;
    }
};

/**************************** Generic Pipleline Stage *******************************
 * Class for storing number of instructions in a single stage of pipeline            *
 * Available functions for performing operations in a stage :                        *
 *                                                                                   *
 **************************** End Generic Pipleline Stage ***************************/
class Pipeline_Stage_Operator
{
private:
    unsigned int pipeline_width;
    vector<Instruction_Structure> Pipeline_Registers;
    unsigned int available_elements_in_stage_count;
    vector<bool> available_elements_in_stage;
    vector<bool> ready_to_move;
    unsigned int pipeline_stage;

public:
    /* Constructor for the class to initialize the pipeline register of size width */
    Pipeline_Stage_Operator(unsigned int width, unsigned int stage_no)
    {
        pipeline_width = width;
        Pipeline_Registers = vector<Instruction_Structure>(width, {0, 0, 0, 0, 0, 0, -1, -1, -1, 0, 0, {0, 0}});
        available_elements_in_stage_count = width;
        pipeline_stage = stage_no;
        ready_to_move = vector<bool>(pipeline_width, 0);
        available_elements_in_stage = vector<bool>(pipeline_width, 1);
    }

    /**************** Function to add instructions to pipeline register *****************
     * Any number of instructions can be added,                                          *
     * but should be of vector of Instruction_Structure                                  *
     * single_cycle_instructions is a parameter to set all bits of ready_to_move to 1    *
     * Returns:                                                                          *
     *           0: Successfully added all instructions to the register                  *
     *           1: Did not add instructions to register, due to insufficient capacity   *
     ************** End Function to add instructions to pipeline register ***************/
    bool Add_Instructions_to_Register(vector<Instruction_Structure> instruction_to_be_added, unsigned int sim_time_at_this_instant)
    {
        unsigned int no_of_added_elements = 0;
        // Check if all instructions can be added at once or not
        if (instruction_to_be_added.size() <= available_elements_in_stage_count)
        {
            for (unsigned int indexing = 0; indexing < pipeline_width; indexing++)
            {
                // Adding each instruction one by one at the available locations
                if (available_elements_in_stage[indexing] == 1)
                {
                    Pipeline_Registers[indexing] = instruction_to_be_added[no_of_added_elements];
                    Pipeline_Registers[indexing].time_info.start_time_at_each_stage[pipeline_stage] = sim_time_at_this_instant;
                    available_elements_in_stage[indexing] = 0;
                    available_elements_in_stage_count--;
                    // If register is not in EX, or WB, then the instruction usually stays for 1 clock cycle only
                    if ((pipeline_stage != 5) || (pipeline_stage != 6) || (pipeline_stage != 7))
                    {
                        ready_to_move[indexing] = 1;
                    }
                    ready_to_move[indexing] = 0;
                    no_of_added_elements++;
                }
                if (no_of_added_elements == instruction_to_be_added.size())
                {
                    break;
                }
            }
            return 0;
        }
        else
        {
            // cout << "Error(Pipeline): Better barrier can be kept" << endl;
            return 1;
        }
    }

    vector<Instruction_Structure> Get_Just_Registers()
    {
        vector<Instruction_Structure> temp;
        if (available_elements_in_stage_count == 0)
            return Pipeline_Registers;
        else
        {
            cout << "Neeabba" << endl;
            temp.clear();
            return temp;
        }
    }

    void Add_Just_Registers(vector<Instruction_Structure> instructions_to_be_added)
    {
        if (instructions_to_be_added.size() == pipeline_width)
        {
            for (unsigned int indexing = 0; indexing < pipeline_width; indexing++)
            {
                Pipeline_Registers[indexing] = instructions_to_be_added[indexing];
            }
        }
        else
        {
            // cout << "Chuthiye" << endl;
        }
    }

    void Set_Renamed_Register_Ready(vector<int> ready_registers)
    {
        for (unsigned int sub_indexing = 0; sub_indexing < ready_registers.size(); sub_indexing++)
        {
            // cout << "Trying to set: " << ready_registers[sub_indexing] << endl;
            for (unsigned int indexing = 0; indexing < pipeline_width; indexing++)
            {
                if (available_elements_in_stage[indexing] == 0)
                {
                    //cout << "Trying to set: " << ready_registers[sub_indexing] << " with: " << Pipeline_Registers[indexing].renamed_src1 << ", " << Pipeline_Registers[indexing].renamed_src2 << endl;
                    if (Pipeline_Registers[indexing].renamed_src1 == ready_registers[sub_indexing])
                    {
                        // cout << "Good1" << endl;
                        Pipeline_Registers[indexing].src1_ready_status = 1;
                    }
                    if (Pipeline_Registers[indexing].renamed_src2 == ready_registers[sub_indexing])
                    {
                        // cout << "Good2" << endl;
                        Pipeline_Registers[indexing].src2_ready_status = 1;
                    }
                }
            }
        }
    }
    /********** Function to get, and remove instructions to pipeline register ***********
     * Any number of instructions can be removed,                                        *
     * Returns:                                                                          *
     *           Vector of Instructions which are ready to be moved to the next stage    *
     ****** End Function to get, and remove instructions to pipeline register ***********/
    vector<Instruction_Structure> Get_and_Remove_Instructions_from_Register()
    {
        vector<Instruction_Structure> Registers_to_be_returned;
        for (unsigned int indexing = 0; indexing < pipeline_width; indexing++)
        {
            // If register is not empty
            if (available_elements_in_stage[indexing] != 1)
            {
                // If instruction is ready to move
                if (ready_to_move[indexing] == 1)
                {
                    // Pipeline_Registers[indexing].time_info.duration_at_each_stage[pipeline_stage]++;
                    Registers_to_be_returned.push_back(Pipeline_Registers[indexing]);
                    available_elements_in_stage[indexing] = 1;
                    available_elements_in_stage_count++;
                    ready_to_move[indexing] = 0;
                }
            }
        }
        return Registers_to_be_returned;
    }

    Selective_Removal_Struct Pseudo_Selective_Remove_Instruction(Instruction_Structure Instr_to_be_removed)
    {
        Selective_Removal_Struct To_be_returned;
        for (unsigned int indexing = 0; indexing < pipeline_width; indexing++)
        {
            // cout << "Here pokay" << endl;
            if (ready_to_move[indexing] == 1)
            {
                // cout << "Here pokayw4werew" << endl;
                // cout << Instr_to_be_removed.seq_no << ", while " << Pipeline_Registers[indexing].seq_no << endl;
                if (Instr_to_be_removed.seq_no == Pipeline_Registers[indexing].seq_no)
                {
                    // cout << "potyyy pokay" << endl;
                    // Pipeline_Registers[indexing].time_info.duration_at_each_stage[pipeline_stage]++;
                    ready_to_move[indexing] = 1;
                    To_be_returned.success = 1;
                    To_be_returned.instruction = Pipeline_Registers[indexing];
                    return To_be_returned;
                }
            }
        }
        To_be_returned.success = 0;
        return To_be_returned;
    }

    Selective_Removal_Struct Selective_Remove_Instruction(Instruction_Structure Instr_to_be_removed)
    {
        Selective_Removal_Struct To_be_returned;
        for (unsigned int indexing = 0; indexing < pipeline_width; indexing++)
        {
            // cout << "Here pokay" << endl;
            if (ready_to_move[indexing] == 1)
            {
                // cout << "Here pokayw4werew" << endl;
                if (Instr_to_be_removed.seq_no == Pipeline_Registers[indexing].seq_no)
                {
                    // cout << "potyyy pokay" << endl;
                    // Pipeline_Registers[indexing].time_info.duration_at_each_stage[pipeline_stage]++;
                    available_elements_in_stage[indexing] = 1;
                    available_elements_in_stage_count++;
                    ready_to_move[indexing] = 0;
                    To_be_returned.success = 1;
                    To_be_returned.instruction = Pipeline_Registers[indexing];
                    return To_be_returned;
                }
            }
        }
        To_be_returned.success = 0;
        return To_be_returned;
    }
    /***************************** Get_Status_of_Pipeline *******************************
     * Checks if there are any instructions that are ready to move                       *
     * Also it makes the required changes in timing information for each instruction     *
     * Returns:                                                                          *
     *           1: Elements ready to moved in this stage                                *
     *           0: No elements in this stage are ready to be moved                      *
     *************************** End Get_Status_of_Pipeline *****************************/
    int Get_Status_of_Pipeline()
    // Not used in cc file
    {

        if (available_elements_in_stage_count != pipeline_width)
        {
            int no_of_elements_that_are_ready_to_move = 0;
            for (unsigned int indexing = 0; indexing < pipeline_width; indexing++)
            {
                // Timing changes here
                // Pipeline_Registers[indexing].time_info.duration_at_each_stage[pipeline_stage]++;
                if (ready_to_move[indexing] == 1)
                {
                    no_of_elements_that_are_ready_to_move++;
                }
            }
            return no_of_elements_that_are_ready_to_move;
        }
        else
        {
            return -1;
        }
    }

    bool Set_Ready_to_Move_Instruction(unsigned int seq_number)
    {
        for (unsigned int indexing = 0; indexing < pipeline_width; indexing++)
        {
            if (Pipeline_Registers[indexing].seq_no == seq_number)
            {
                ready_to_move[indexing] = 1;
                return 1;
            }
        }
        return 0;
    }

    void Increment_Time()
    {
        for (unsigned int indexing = 0; indexing < pipeline_width; indexing++)
        {
            // Timing changes here
            Pipeline_Registers[indexing].time_info.duration_at_each_stage[pipeline_stage]++;
        }
    }

    unsigned int Get_Availability_of_Pipeline()
    {
        // for(unsigned int indexing = 0; indexing < pipeline_width; indexing++)
        //{
        //  Timing changes here
        // if (available_elements_in_stage[indexing] != 1)
        //     Pipeline_Registers[indexing].time_info.duration_at_each_stage[pipeline_stage]++;
        // }
        return available_elements_in_stage_count;
    }

    unsigned int Get_Just_Availability()
    {
        return available_elements_in_stage_count;
    }

    /************************ Print_Instructions_in_Register ****************************
     * Prints the seq no., PC (in hex)                                                  *
     ********************** End Print_Instructions_in_Register **************************/
    void Print_Instructions_in_Register()
    {
        for (unsigned int indexing = 0; indexing < pipeline_width; indexing++)
        {
            if (available_elements_in_stage[indexing] == 0)
            {
                cout << Pipeline_Registers[indexing].seq_no << " with renamed destination: " << Pipeline_Registers[indexing].renamed_dest << "\t" << Pipeline_Registers[indexing].renamed_src1 << "\t" << Pipeline_Registers[indexing].renamed_src2 << endl;
                cout << Pipeline_Registers[indexing].seq_no << " with destination: " << Pipeline_Registers[indexing].dest_register << "\t" << Pipeline_Registers[indexing].src1_register << "\t" << Pipeline_Registers[indexing].src2_register << endl;
            }
        }
    }

    // This function may not be of any use
    /************************** Add_Modified_Source_Registers ***************************
     * Adds the renamed register values                                                 *
     * Type of register:
     *              0: Dest
     *              1: SRC1
     *              2: SRC2
     * Returns:                                                                         *
     *          Nothing                                                                 *
     ********************* End Add_Modified_Source_Registers ****************************/
    void Add_Modified_Source_Registers(unsigned int indexing, int type_of_register, int reg_value)
    {
        if (type_of_register == 0)
        {
            Pipeline_Registers[indexing].renamed_dest = reg_value;
        }
        else if (type_of_register == 1)
        {
            Pipeline_Registers[indexing].renamed_src1 = reg_value;
        }
        else if (type_of_register == 2)
        {
            Pipeline_Registers[indexing].renamed_src2 = reg_value;
        }
    }

    vector<Instruction_Structure> Search_for_Completed_Instructions()
    {
        vector<Instruction_Structure> to_be_returned;
        for (unsigned int indexing = 0; indexing < pipeline_width; indexing++)
        {
            // Pipeline_Registers[indexing].time_info.duration_at_each_stage[pipeline_stage]++;
            if (available_elements_in_stage[indexing] == 0)
            {
                // cout << "For SEQ: " << Pipeline_Registers[indexing].seq_no << " with, "<< Pipeline_Registers[indexing].time_info.duration_at_each_stage[pipeline_stage]  << endl;
                if (Pipeline_Registers[indexing].op_type == 0)
                {
                    if (Pipeline_Registers[indexing].time_info.duration_at_each_stage[pipeline_stage] == 1)
                    {
                        // cout << "Pushing back instruction: " << Pipeline_Registers[indexing].seq_no << endl;
                        to_be_returned.push_back(Pipeline_Registers[indexing]);
                        available_elements_in_stage[indexing] = 1;
                        available_elements_in_stage_count++;
                        ready_to_move[indexing] = 0;
                    }
                }
                else if (Pipeline_Registers[indexing].op_type == 1)
                {
                    if (Pipeline_Registers[indexing].time_info.duration_at_each_stage[pipeline_stage] == 2)
                    {
                        // cout << "Pushing back instruction: " << Pipeline_Registers[indexing].seq_no << endl;
                        to_be_returned.push_back(Pipeline_Registers[indexing]);
                        available_elements_in_stage[indexing] = 1;
                        available_elements_in_stage_count++;
                        ready_to_move[indexing] = 0;
                    }
                }
                else if (Pipeline_Registers[indexing].op_type == 2)
                {
                    if (Pipeline_Registers[indexing].time_info.duration_at_each_stage[pipeline_stage] == 5)
                    {
                        // cout << "Pushing back instruction: " << Pipeline_Registers[indexing].seq_no << endl;
                        to_be_returned.push_back(Pipeline_Registers[indexing]);
                        available_elements_in_stage[indexing] = 1;
                        available_elements_in_stage_count++;
                        ready_to_move[indexing] = 0;
                    }
                }
            }
        }
        return to_be_returned;
    }

    Selective_Removal_Struct Search_Specific_Register_using_PC(unsigned long PC_address)
    {
        Selective_Removal_Struct to_be_returned;
        to_be_returned.success = 0;
        for (unsigned int indexing = 0; indexing < pipeline_width; indexing++)
        {
            if (available_elements_in_stage[indexing] == 0)
            {
                if (Pipeline_Registers[indexing].PC == PC_address)
                {
                    to_be_returned.success = 1;
                    to_be_returned.instruction = Pipeline_Registers[indexing];
                    // cout << "Instruction seq is: " << to_be_returned.instruction.seq_no;
                    // cout << "Alpha Instruction seq is: " << Pipeline_Registers[indexing].seq_no;
                    return to_be_returned;
                }
            }
        }
        return to_be_returned;
    }

    Selective_Removal_Struct Search_Specific_Register_using_seq(unsigned int SEQ)
    {
        Selective_Removal_Struct to_be_returned;
        to_be_returned.success = 0;
        for (unsigned int indexing = 0; indexing < pipeline_width; indexing++)
        {
            if (available_elements_in_stage[indexing] == 0)
            {
                if (Pipeline_Registers[indexing].seq_no == SEQ)
                {
                    to_be_returned.success = 1;
                    to_be_returned.instruction = Pipeline_Registers[indexing];
                    // cout << "Instruction seq is: " << to_be_returned.instruction.seq_no;
                    // cout << "Alpha Instruction seq is: " << Pipeline_Registers[indexing].seq_no;
                    return to_be_returned;
                }
            }
        }
        return to_be_returned;
    }

    void Print_Timing()
    {
        cout << "For pipeline stage :" << pipeline_stage << ", with width: " << pipeline_width << endl;
        for (unsigned int indexing = 0; indexing < pipeline_width; indexing++)
        {
            cout << Pipeline_Registers[indexing].seq_no << " fu{" << Pipeline_Registers[indexing].op_type
                 << "} src{" << Pipeline_Registers[indexing].src1_register << "," << Pipeline_Registers[indexing].src2_register
                 << "} dst{" << Pipeline_Registers[indexing].dest_register
                 << "} FE{" << Pipeline_Registers[indexing].time_info.start_time_at_each_stage[0] << "," << Pipeline_Registers[indexing].time_info.duration_at_each_stage[0]
                 << "} DE{" << Pipeline_Registers[indexing].time_info.start_time_at_each_stage[1] << "," << Pipeline_Registers[indexing].time_info.duration_at_each_stage[1]
                 << "} RN{" << Pipeline_Registers[indexing].time_info.start_time_at_each_stage[2] << "," << Pipeline_Registers[indexing].time_info.duration_at_each_stage[2]
                 << "} RR{" << Pipeline_Registers[indexing].time_info.start_time_at_each_stage[3] << "," << Pipeline_Registers[indexing].time_info.duration_at_each_stage[3]
                 << "} DI{" << Pipeline_Registers[indexing].time_info.start_time_at_each_stage[4] << "," << Pipeline_Registers[indexing].time_info.duration_at_each_stage[4]
                 << "} IS{" << Pipeline_Registers[indexing].time_info.start_time_at_each_stage[5] << "," << Pipeline_Registers[indexing].time_info.duration_at_each_stage[5]
                 << "} EX{" << Pipeline_Registers[indexing].time_info.start_time_at_each_stage[6] << "," << Pipeline_Registers[indexing].time_info.duration_at_each_stage[6]
                 << "} WB{" << Pipeline_Registers[indexing].time_info.start_time_at_each_stage[7] << "," << Pipeline_Registers[indexing].time_info.duration_at_each_stage[7]
                 << "} RT{" << Pipeline_Registers[indexing].time_info.start_time_at_each_stage[8] << "," << Pipeline_Registers[indexing].time_info.duration_at_each_stage[8]
                 << "}" << endl;
        }
        cout << endl;
    }

    vector<Instruction_Structure> Search_for_Almost_Completed_Instructions()
    {
        vector<Instruction_Structure> to_be_returned;
        to_be_returned.clear();
        for (unsigned int indexing = 0; indexing < pipeline_width; indexing++)
        {
            // cout << "Moshi moshi " << pipeline_stage << endl;
            if (available_elements_in_stage[indexing] == 0)
            {
                if (pipeline_stage == 6)
                {
                    // cout << "Checking: " << Pipeline_Registers[indexing].seq_no << " with timing: " << Pipeline_Registers[indexing].time_info.duration_at_each_stage[pipeline_stage] << endl;
                    if (Pipeline_Registers[indexing].op_type == 0)
                    {
                        if (Pipeline_Registers[indexing].time_info.duration_at_each_stage[pipeline_stage] == 0)
                        {
                            to_be_returned.push_back(Pipeline_Registers[indexing]);
                            //ready_to_move[indexing] = 1;
                        }
                    }
                    else if (Pipeline_Registers[indexing].op_type == 1)
                    {
                        if (Pipeline_Registers[indexing].time_info.duration_at_each_stage[pipeline_stage] == 1)
                        {
                            to_be_returned.push_back(Pipeline_Registers[indexing]);
                            //ready_to_move[indexing] = 1;
                        }
                    }
                    else // if (Pipeline_Registers[indexing].op_type == 2)
                    {
                        // cout << "Simsim" << endl;
                        if (Pipeline_Registers[indexing].time_info.duration_at_each_stage[pipeline_stage] == 4)
                        {
                            to_be_returned.push_back(Pipeline_Registers[indexing]);
                            //ready_to_move[indexing] = 1;
                            // cout << "Setting" << endl;
                        }
                    }
                }
                else
                {
                    ready_to_move[indexing] = 1;
                }
            }
        }
        return to_be_returned;
    }
};

#endif
