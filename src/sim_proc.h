#ifndef SIM_PROC_H
#define SIM_PROC_H

#include <vector>
#include <iostream>


typedef struct proc_params{
    unsigned long int rob_size;
    unsigned long int iq_size;
    unsigned long int width;
}proc_params;

// Put additional data structures here as per your requirement





/**************************** Timing Information Structure **************************
 * Stores the timing based information at each stage                                *
*********************** End Timing Information Structure ***************************/
typedef struct Timing_Information_Structure{
    unsigned int start_time_at_each_stage[9];
    unsigned int duration_at_each_stage[9];
}Timing_Information_Structure;





/****************************** Instruction  Structure ******************************
* Structure for storing a single instruction                                        *
***************************** End Instruction  Structure ***************************/

typedef struct Instruction_Structure{
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
    Timing_Information_Structure time_info;
}Instruction_Structure;





/************************** Individual Rename Map Table Struct **********************
* Structure for individual element in Rename Map Table                              *
************************* End Rename Map Table Struct ******************************/
typedef struct Individual_Rename_Map_Table_struct{
    int rob_tag;
    bool valid;
}Individual_Rename_Map_Table_struct;





/***************************** Individual Reorder Buffer ****************************
* Structure for individual entry in Reorder Buffer                                  *
**************************** End Individual Reorder Buffer *************************/
typedef struct Individual_ROB_struct{
    int value;
    int destination;
    bool ready;
    bool exception;
    bool misprediction;
    unsigned int PC; // seq_no
}Individual_ROB_struct;





/***************************** Individual Issue Queue *******************************
* Structure for individual entry in Issue Queue                                     *
**************************** End Individual Issue Queue ****************************/
typedef struct Individual_Issue_Queue_struct{
    bool valid_bit;
    bool src1_ready_bit;
    bool src2_ready_bit;
    Instruction_Structure instruction;
}Individual_Issue_Queue_struct;





/************************** Rename Map Table Operator *******************************
* Structure for Rename Map Table                                                    *
* The index of arrays represents the register number                                *
************************* End Rename Map Table Operator ****************************/
class Rename_Map_Table_Operator
{
private:
    unsigned int total_no_of_registers;
    std::vector<Individual_Rename_Map_Table_struct> Rename_Map_Table_vector;
public:
    Rename_Map_Table_Operator(unsigned int no_of_registers)
    {
        total_no_of_registers = no_of_registers;
        Rename_Map_Table_vector = std::vector<Individual_Rename_Map_Table_struct>(no_of_registers);
    }



    /*************************** set_rob_tag ********************************************
    * For a given register register_no                                                  *
    * Sets the rob tag in the Rename Map Table                                          *
    * Sets the valid bit to 1                                                           *
    ********************** End set_rob_tag *********************************************/
    void Set_Rob_Tag_in_RMT(unsigned int register_no, unsigned int rob_index)
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
    std::vector<Individual_Issue_Queue_struct> issue_queue;
    unsigned int available_iq_elements;
public:
    /* Constructor for the class to initialize the issue queue of size iq_size */ 
    IssueQueue_Operator(unsigned int iq_size)
    {
        issue_queue_size = iq_size;
        issue_queue = std::vector<Individual_Issue_Queue_struct>(iq_size);
        available_iq_elements = iq_size;
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
        unsigned int index;
        for(index = 0; index < issue_queue_size; index++)
        {
            if (issue_queue[index].valid_bit == 0)
            {
                issue_queue[index].instruction = instruction_to_be_added;
                // Check for the ready bits of src1 and src2 registers

                issue_queue[index].valid_bit = 1;
                --available_iq_elements;
                return 1;
            }
        }

        // For checking_printing
        std::cout << "\n Available elements in issue queue are: " << available_iq_elements << std::endl;
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
    std::vector<Instruction_Structure> Get_Oldest_Instructions_from_IQ(unsigned int size_to_get)
    {
        std::vector<Instruction_Structure> Instructions_to_be_returned;
        // Can only add instructions of size size_to_get
        while (Instructions_to_be_returned.size() < size_to_get)
        {
            unsigned int mini_seq_no = 100000000;
            unsigned int index;
            for(unsigned int indexing = 0; indexing < issue_queue_size; indexing++)
            {
                // Searching through valid instructions in the IQ
                if (issue_queue[indexing].valid_bit == 1)
                {
                    // Checking if both the src registers are ready
                    if ((issue_queue[indexing].src1_ready_bit == 1) && (issue_queue[indexing].src2_ready_bit == 1))
                    {
                        // Finding the index of instruction that is the oldest in the IQ
                        if (issue_queue[indexing].instruction.seq_no < mini_seq_no)
                        {
                            index = indexing;
                            mini_seq_no = issue_queue[indexing].instruction.seq_no;
                        }
                    }
                }
            }
            // Just setting the valid bit to 0 works
            issue_queue[index].valid_bit = 0;
            // Increment the available elements in IQ for a safe addition to IQ
            ++available_iq_elements;
            // Add the instruction to the vector
            Instructions_to_be_returned.push_back(issue_queue[index].instruction);
        }
        return Instructions_to_be_returned;
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
        for(unsigned int indexing = 0; indexing < issue_queue_size; indexing++)
        {
            // Set the ready bits of only that are valid
            if (issue_queue[indexing].valid_bit == 1)
            {
                // Sets the src1 ready bit if the renamed register is src_register
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
    std::vector<Individual_ROB_struct> ROB;
    unsigned int header;
    unsigned int tail;
    unsigned long int no_of_available_elements_in_rob;
public:
    /* Constructor for the class to initialize the reorder buffer of size robsize */ 
    ROB_Operator(unsigned int robsize)
    {
        rob_size = robsize;
        ROB = std::vector<Individual_ROB_struct>(robsize);
        no_of_available_elements_in_rob = robsize;
        header = 0; // Changed when elements are added into the rob
        tail = 0; // Changed when elements are removed from rob
        // Ensure that header and tail is only equal at initialization
    }



    /********************** Add Instructions to ROB *************************************
     * Adds a vector of Instructions of type Instruction_Structure                      *
     * Returns:                                                                         *
     *          1: If operation is successfull                                          *
     *          0: ROB is full, not possible to add instructions in this cycle          *
    ******************** End Add Instructions to ROB ***********************************/
    unsigned int Add_Instruction_to_ROB(int corresponding_register)
    {        
        // If reached the end of rob, set the header to 0
        if (header >= (rob_size - 1))
        {
            header -= (rob_size - 1);
        }
        ROB[header].destination = corresponding_register;
        no_of_available_elements_in_rob--;
        header++;
        return header;
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
            if (no_of_available_elements_in_rob != 0)
            {
                ++tail;
                if (tail >= rob_size)
                    tail -= rob_size;
                return 1;
            }
            else
            {
                return 0;
            }
        }
        else
        {
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

    unsigned long int Get_Availability_in_ROB()
    {
        return no_of_available_elements_in_rob;
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
    std::vector<Instruction_Structure> Pipeline_Registers;
    unsigned int available_elements_in_stage_count;
    std::vector<bool> available_elements_in_stage;
    std::vector<bool> ready_to_move;
    unsigned int pipeline_stage;
public:
    /* Constructor for the class to initialize the pipeline register of size width */ 
    Pipeline_Stage_Operator(unsigned int width, unsigned int stage_no)
    {
        pipeline_width = width;
        Pipeline_Registers = std::vector<Instruction_Structure>(width, {0, 0, 0, 0, 0, 0, -1, -1, -1, {0, 0}});
        available_elements_in_stage_count = width;
        pipeline_stage = stage_no;
        ready_to_move = std::vector<bool>(pipeline_width, 0);
        available_elements_in_stage = std::vector<bool>(pipeline_width, 1);
    }



    /**************** Function to add instructions to pipeline register *****************
    * Any number of instructions can be added,                                          *
    * but should be of vector of Instruction_Structure                                  *
    * single_cycle_instructions is a parameter to set all bits of ready_to_move to 1    *
    * Returns:                                                                          *
    *           0: Successfully added all instructions to the register                  *
    *           1: Did not add instructions to register, due to insufficient capacity   *
    ************** End Function to add instructions to pipeline register ***************/
    bool Add_Instructions_to_Register(std::vector<Instruction_Structure> instruction_to_be_added, unsigned int sim_time_at_this_instant)
    {
        // Check if all instructions can be added at once or not
        //std::cout << "Adding stage: " << instruction_to_be_added.size() << ", " << available_elements_in_stage_count << std::endl;
        if (instruction_to_be_added.size() <= available_elements_in_stage_count)
        {
            unsigned int index_of_element_to_be_added = 0;
            for(unsigned int indexing = 0; indexing < instruction_to_be_added.size(); indexing++)
            {
                // Adding each instruction one by one at the available locations
                if (available_elements_in_stage[indexing] == 1)
                {
                    Pipeline_Registers[indexing] = instruction_to_be_added[index_of_element_to_be_added];
                    Pipeline_Registers[indexing].time_info.start_time_at_each_stage [pipeline_stage] = sim_time_at_this_instant;
                    available_elements_in_stage[indexing] = 0;
                    available_elements_in_stage_count--;
                    // If register is not in IS, EX, or WB, then the instruction usually stays for 1 clock cycle only
                    //if ((pipeline_stage != 5) || (pipeline_stage != 6) || (pipeline_stage != 7))
                    {
                        ready_to_move[indexing] = 1;
                    }
                }
            }
            return 0;
        }
        else
        {
            return 1;
        }
    }


    /********** Function to get, and remove instructions to pipeline register ***********
    * Any number of instructions can be removed,                                        *
    * Returns:                                                                          *
    *           Vector of Instructions which are ready to be moved to the next stage    *
    ****** End Function to get, and remove instructions to pipeline register ***********/
    std::vector<Instruction_Structure> Get_and_Remove_Instructions_from_Register()
    {
        std::vector<Instruction_Structure> Registers_to_be_returned;
        for(unsigned int indexing = 0; indexing < pipeline_width; indexing++)
        {
            if (ready_to_move[indexing] == 1)
            {
                Registers_to_be_returned.push_back(Pipeline_Registers[indexing]);
                available_elements_in_stage[indexing] = 1;
                available_elements_in_stage_count++;
                ready_to_move[indexing] = 0;
            }
        }
        return Registers_to_be_returned;
    }



    /***************************** Get_Status_of_Pipeline *******************************
    * Checks if there are any instructions that are ready to move                       *
    * Also it makes the required changes in timing information for each instruction     *
    * Returns:                                                                          *
    *           1: Elements ready to moved in this stage                                *
    *           0: No elements in this stage are ready to be moved                      *
    *************************** End Get_Status_of_Pipeline *****************************/
    unsigned int Get_Status_of_Pipeline()
    {
        if (available_elements_in_stage_count != pipeline_width)
        {
            int no_of_elements_that_are_ready_to_move = 0;
            for(unsigned int indexing = 0; indexing < pipeline_width; indexing++)
            {
                // Timing changes here
                Pipeline_Registers[indexing].time_info.duration_at_each_stage[pipeline_stage] += 1;
                if (ready_to_move[indexing] == 1)
                {
                    no_of_elements_that_are_ready_to_move++;
                }
            }
            return no_of_elements_that_are_ready_to_move;
        }
        else
        {
            return pipeline_width;
        }
    }



    /************************ Print_Instructions_in_Register ****************************
     * Prints the seq no., PC (in hex)                                                  * 
    ********************** End Print_Instructions_in_Register **************************/
    void Print_Instructions_in_Register()
    {
        for(unsigned int indexing = 0; indexing < pipeline_width; indexing++)
        {
            std::cout << Pipeline_Registers[indexing].seq_no << ", " << std::hex << Pipeline_Registers[indexing].PC << std::dec << std::endl;
        }
    }


    /************************** Add_Modified_Source_Registers ***************************
     * Adds the renamed register values                                                 *
     * Returns:                                                                         *
     *          Nothing                                                                 *
    ********************* End Add_Modified_Source_Registers ****************************/
   void Add_Modified_Source_Registers(std::vector<int> dests, std::vector<int> srcs1, std::vector<int> srcs2)
    {
        for(unsigned int indexing = 0; indexing < pipeline_width; indexing++)
        {
            Pipeline_Registers[indexing].renamed_dest = dests[indexing];
            Pipeline_Registers[indexing].renamed_src1 = srcs1[indexing];
            Pipeline_Registers[indexing].renamed_src2 = srcs2[indexing];
        }
    }
};



#endif
