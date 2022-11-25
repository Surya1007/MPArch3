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
    unsigned int PC;
    unsigned int op_type;
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
    unsigned int rob_tag;
    bool valid;
}Individual_Rename_Map_Table_struct;





/***************************** Individual Reorder Buffer ****************************
* Structure for individual entry in Reorder Buffer                                  *
**************************** End Individual Reorder Buffer *************************/
typedef struct Individual_ROB_struct{
    unsigned int seq_no;
    int value;
    int destination;
    bool ready;
    bool exception;
    bool misprediction;
    unsigned int PC;
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
    void set_rob_tag_in_rmt(unsigned int register_no, unsigned int rob_index)
    {
        Rename_Map_Table_vector[register_no].valid = 1;
        Rename_Map_Table_vector[register_no].rob_tag = rob_index;
    }



    /************************* reset_rob_tag ********************************************
    * For a given register register_no                                                  *
    * Sets the valid bit to 0                                                           *
    ******************** End reset_rob_tag *********************************************/
    void reset_rob_tag_in_rmt(unsigned int register_no)
    {
        Rename_Map_Table_vector[register_no].valid = 0;
    }



    /*************************** get_rob_tag ********************************************
    * Returns the Individual_Rename_Map_Table_struct for a given register register_no   *
    ********************** End get_rob_tag *********************************************/
    Individual_Rename_Map_Table_struct get_rob_tag_from_rmt(unsigned int register_no)
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

    bool check_if_register_is_ready()
    {

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
    unsigned int pointer;
public:
    /* Constructor for the class to initialize the reorder buffer of size robsize */ 
    ROB_Operator(unsigned int robsize)
    {
        rob_size = robsize;
        ROB = std::vector<Individual_ROB_struct>(robsize);
        header = 0;
        pointer = 0;
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
    * Returns:                                                                          *
    *           1: Successfully added all instructions to the register                  *
    *           0: Did not add instructions to register, due to insufficient capacity   *
    ************** End Function to add instructions to pipeline register ***************/
    bool Add_Instructions_to_Register(std::vector<Instruction_Structure> instruction_to_be_added)
    {
        // Check if all instructions can be added at once or not
        if (instruction_to_be_added.size() <= available_elements_in_stage_count)
        {
            unsigned int index_of_element_to_be_added = 0;
            for(unsigned int indexing = 0; indexing < pipeline_width; indexing++)
            {
                // Adding each instruction one by one at the available locations
                if (available_elements_in_stage[indexing] == 1)
                {
                    Pipeline_Registers[indexing] = instruction_to_be_added[index_of_element_to_be_added];
                    available_elements_in_stage[indexing] = 0;
                    available_elements_in_stage_count++;
                }
            }
            return 1;
        }
        else
        {
            return 0;
        }
    }



    /********** Function to get, and remove instructions to pipeline register ***********
    * Any number of instructions can be removed,                                        *
    * Returns:                                                                          *
    *           Vector of Instructions which are ready to be moved to the next stage    *
    ****** End Function to get, and remove instructions to pipeline register ***********/
    std::vector<Instruction_Structure> Get_Instructions_from_Register()
    {
        std::vector<Instruction_Structure> Registers_to_be_returned;
        for(unsigned int indexing = 0; indexing < pipeline_width; indexing++)
        {
            if (ready_to_move[indexing] == 1)
            {
                Registers_to_be_returned.push_back(Pipeline_Registers[indexing]);
                available_elements_in_stage[indexing] = 1;
                available_elements_in_stage_count--;
            }
        }
        return Registers_to_be_returned;
    }



};

#endif
