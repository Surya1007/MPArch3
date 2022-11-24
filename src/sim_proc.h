#ifndef SIM_PROC_H
#define SIM_PROC_H

#include <vector>


typedef struct proc_params{
    unsigned long int rob_size;
    unsigned long int iq_size;
    unsigned long int width;
}proc_params;

// Put additional data structures here as per your requirement





/********************************* Rename Map Table *********************************
* Structure for Rename Map Table                                                    *
* The index of arrays represents the register number                                *
******************************** End Rename Map Table ******************************/
typedef struct Rename_Map_Table_struct{
    unsigned int rob_tags[67];
    bool valid[67];
}Rename_Map_Table_struct;





/***************************** Individual Issue Queue *******************************
* Structure for individual entry in Issue Queue                                     *
**************************** End Individual Issue Queue ****************************/
typedef struct Individual_Issue_Queue_struct{
    unsigned int seq_no;
    bool valid_bit;
    int destination_tag;
    bool src1_ready_bit;
    int src1_tag_value;
    bool src2_ready_bit;
    int src2_tag_value;    
}Individual_Issue_Queue_struct;





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
public:
    /* Constructor for the class to initialize the issue queue of size iq_size */ 
    IssueQueue_Operator(unsigned int iq_size)
    {
        issue_queue_size = iq_size;
        issue_queue = std::vector<Individual_Issue_Queue_struct>(iq_size);
    }
};





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





/************************ Generic Pipleline Register Structure **********************
* Structure for a register to store a single instruction                            *
********************* End Generic Pipleline Register Structure *********************/

typedef struct Pipeline_Register_Structure{
    unsigned int seq_no;
    unsigned int PC;
    unsigned int op_type;
    int dest_register;
    int src1_register;
    int src2_register;
}Pipeline_Register_Structure;





/**************************** Generic Pipleline Register ****************************
* Class for storing number of instructions in a single stage of pipeline            *
* Available functions for performing operations in a register :                     *
*                                                                                   *
**************************** End Generic Pipleline Register ************************/
class Pipeline_Register_Operator
{
private:
    unsigned int register_width;
    std::vector<Pipeline_Register_Structure> Pipeline_Register;
public:
    /* Constructor for the class to initialize the pipeline register of size width */ 
    Pipeline_Register_Operator(unsigned int width)
    {
        register_width = width;
        Pipeline_Register = std::vector<Pipeline_Register_Structure>(width);
    }
};

#endif
