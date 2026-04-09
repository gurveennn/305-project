#include<stdio.h>
#include<time.h>
#include<math.h>
#include<stdlib.h>
#include<unistd.h>
#include<assert.h>
#include<float.h>
#include <cerrno>
#include <limits.h>
#include <iostream>
#include <list>
#include <string>
#include <fstream> 
#include <sstream> 
#include <cmath>
#include"project.h"


// Function for consuming the trace, and creating Instruction objects containing the Instruction program counter (PC), 
// instruction type and a list of PC values for instructions that the current instruction depends on.
void Simulation::ReadTrace(std::string filename){
    std::ifstream file(filename);
    file.seekg(std::ios::beg);

    // Skip all lines(instructions) until start_inst
    for (int i = 0; i < start_inst - 1; ++i) {
        // Ignore characters up to the next newline character
        file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }


    // String to store each line of the file.
    std::string line;

    // Track current line to ensure only inst_count instructions were read
    double current_line_num = start_inst;
    double target_line = start_inst + inst_count;


    if (file.is_open()) {
        while (current_line_num < target_line && getline(file, line)){
                current_line_num += 1;
                std::stringstream ss(line);
                std::string token;
                getline(ss, token, ',');
                std::string pc = token;
                getline(ss, token, ',');
                int type = stoi(token);
                // Create a list containing all pc dependencies of the current instruction
                std::vector<std::string> pc_dependencies;
                while (getline(ss, token, ',')) {
                        pc_dependencies.push_back(token);
                }


                // Create a new instance of Instruction struct
                Instruction* inst = new Instruction;
                inst->instruction_type = type;
                inst->instruction_pc = pc;
                inst->dependencies = pc_dependencies;
                inst->ex_done =false;
                inst->mem_done=false;
                inst->seq_num=next_seq_num++;
                // Initialize the number of execution/memory cycles the instruction will need to complete 
                // based on the depth configuration
                inst->num_EX_stages_remaining = 1;
                inst->num_MEM_stages_remaining = 1;
                if ((D == 2 || D == 4) && inst->instruction_type == 2) {
                        inst->num_EX_stages_remaining = 2;
                        
                }
               
                if ((D == 3 || D == 4)  && inst->instruction_type == 4) {
                        inst->num_MEM_stages_remaining = 3;
                }
                // Push instruction into instruction queue
                instructions_queue.push(inst);
        }
        file.close();
    }
    else {
        printf("file doesnt exist\n");
        exit(1);
    }

}

// This function is used to check if another instruction in the same cycle is using the same functional unit. Returns true if two instructions 
// are matching types. Instructions cannot execute if they are matching types.
bool Simulation::ValidateStructuralHazards(std::vector<Instruction*> cur_stage) {
        if (cur_stage.size() < 2) {
                return true;
        }
        Instruction* first= cur_stage[0];
        Instruction* second = cur_stage[1];
        if (first->instruction_type == second->instruction_type) {
                return false;
        }
        return true;
}
//Find the most recent instruction with the same dependency
//I have taken help from ChatGPT to implement this function. I was confused about how to keep trach of the sequence number and it helped me with this part.
Instruction* Simulation::LatestPC(Instruction* current_inst, const std::string& dependencies_pc){
        Instruction* latest = nullptr; 
        //IF Stage
        for (Instruction* inst : IF_stage){
                if(inst==nullptr){
                        continue;
                }
                //Check if the instruction has the same PC as the dependency and was before the current instruction
                else if(inst->instruction_pc==dependencies_pc && inst->seq_num<current_inst->seq_num){
                        if (latest==nullptr || inst->seq_num>latest->seq_num){
                                latest=inst;
                        }

                }
        }
        //ID Stage
        for (Instruction* inst : ID_stage){
                if(inst==nullptr){
                        continue;
                }
                //Check if the instruction has the same PC as the dependency and was before the current instruction
                else if(inst->instruction_pc==dependencies_pc && inst->seq_num<current_inst->seq_num){
                        if (latest==nullptr || inst->seq_num>latest->seq_num){
                                latest=inst;
                        }

                }
        }
        //EX Stage
        for (Instruction* inst : EX_stage){
                if(inst==nullptr){
                        continue;
                }
                //Check if the instruction has the same PC as the dependency and was before the current instruction
                else if(inst->instruction_pc==dependencies_pc && inst->seq_num<current_inst->seq_num){
                        if (latest==nullptr || inst->seq_num>latest->seq_num){
                                latest=inst;
                        }

                }
        }
        //MEM Stage
        for (Instruction* inst : MEM_stage){
                if(inst==nullptr){
                        continue;
                }
                //Check if the instruction has the same PC as the dependency and was before the current instruction
                else if(inst->instruction_pc==dependencies_pc && inst->seq_num<current_inst->seq_num){
                        if (latest==nullptr || inst->seq_num>latest->seq_num){
                                latest=inst;
                        }

                }
        }
        //WB Stage
        for (Instruction* inst : WB_stage){
                if(inst==nullptr){
                        continue;
                }
                //Check if the instruction has the same PC as the dependency and was before the current instruction
                else if(inst->instruction_pc==dependencies_pc && inst->seq_num<current_inst->seq_num){
                        if (latest==nullptr || inst->seq_num>latest->seq_num){
                                latest=inst;
                        }

                }
        }
        for (Instruction* inst : retired_instructions){
                if(inst==nullptr){
                        continue;
                }
                else if(inst->instruction_pc==dependencies_pc && inst->seq_num<current_inst->seq_num){
                        if (latest==nullptr || inst->seq_num>latest->seq_num){
                                latest=inst;
                        }

                }
        }
        return latest;
}
// Checks if provided instruction violates data hazard. An instruction cannot go to EX until all its data dependences are satisfied.
// A dependence on an integer or floating point instruction is satisfied after they finish the EX stage. 
// A dependence on a load or store is satisfied after they finish the MEM stage.
bool Simulation::CheckDataHazard(Instruction* current_inst){
        for (const std::string& dependencies_pc: current_inst->dependencies){
                //most recent instruction with the same dependency
                Instruction* latest = LatestPC(current_inst,dependencies_pc);
                if(latest==nullptr){
                        continue;
                }
                //Check if the instruction is an integer or floating point type and if the data dependency has been satisfied. after EX stage. If not set it to false.
                if(latest->instruction_type == 1 || latest->instruction_type == 2){
                        if (!latest->ex_done){
                                return false;
                        }

                }
                //Check if the instruction is load or store type and if the data dependency has been satisfied. after MEM stage. If not set it to false.
                if(latest->instruction_type == 4 || latest->instruction_type == 5){
                        if (!latest->mem_done){
                                return false;
                        }

                }

        }
        
        return true;  
}

// Instruction fetch IF, first step of processor pipeline. Retrieves instruction objects stored in the instructions queue. 
// Fetches instructions for second stage ID (Instruction decode and read operands).
void Simulation::InstructionFetch() {
    if (!halt_instruction_fetch) {
        while (IF_stage.size() < 2 && !instructions_queue.empty()) {
            Instruction* inst = instructions_queue.front();
            IF_stage.push_back(inst);
            instructions_queue.pop();
        // If the current instruction is a branch type, it is a control hazard. 
        // A branch instruction halts instruction fetch until the cycle after the branch executes (finishes EX stage).
            if (inst->instruction_type == 3) {
                halt_instruction_fetch = true;
                break;
            }
        }
    }
    while (!IF_stage.empty() && ID_stage.size() < 2) {
        Instruction* cur_instruction = IF_stage.front();
        ID_stage.push_back(cur_instruction);
        IF_stage.erase(IF_stage.begin());
    }
}
// Instruction Decode and Read Operands (ID), second stage of processor pipeline. This stage
// Reads the instructions from fetch, determines the instruction types and checks if any hazards were violated. Sends instructions
// to Instruction Execute (EX) and handles any violated hazards accordingly.
void Simulation::InstructionDecodeAndReadOperands(){
    int count;
    // Check if structural hazards were violated, if yes only send the first instruction to execution stage
    if (ValidateStructuralHazards(ID_stage)) {
        count = std::ranges::min(2, (int)ID_stage.size());
    } else {
        count = 1;
    }
    //Check data hazard and if it has not been satisfied instruction cannot go to EX stage.
    for (int i = 0; i < count; i++) {
        Instruction* cur_instruction = ID_stage[i];
        // Check if any data hazards were violated. If yes, stop reading instructions.
        if(!CheckDataHazard(cur_instruction)){
                count=i;
                break;
        }
        EX_stage.push_back(cur_instruction);
     }
     // Remove instructions from ID stage that are progressing into EX stage  
     ID_stage.erase(ID_stage.begin(), ID_stage.begin() + count);
}


// Instruction Issue and Execute (EX) represents third stage of the processor pipeline. Simulates the execution of an instruction. 
// Maintains in-order execution and executes instruction depending on depth value.
void Simulation::InstructionIssueAndExecute(){
           if (EX_stage.empty()){
            return;
        }
        // Create a vector to store instructions for next iteration of the execution stage(if needed)
        std::vector<Instruction*> next_EX;
        bool mem_contains_load = false;
        bool mem_contains_store = false;
        // Checks if the memory stage currently has an instruction of type load or store to ensure in-order execution
        for (auto inst : MEM_stage) {
                if (inst->instruction_type == 4) {
                        mem_contains_load = true;
                }
                if (inst -> instruction_type == 5) {
                        mem_contains_store = true;
                }
        }
        for (auto inst : EX_stage) {
                // Checks how many more execution stages remaining
                if (inst->num_EX_stages_remaining <= 1) {
                        // Checks if memory stage already contains an instruction of type load or store (which matchs current instruction)
                        bool violates_in_order = (inst->instruction_type == 4 && mem_contains_load)|| (inst->instruction_type == 5 && mem_contains_store)|| MEM_stage.size() >= 2;
                        if (violates_in_order) {
                                next_EX.push_back(inst);
                                continue;
                        }
                        else {
                                // Checks for control hazards 
                                if (inst->instruction_type == 3) {
                                        halt_instruction_fetch = false; 
                                } 
                                // Update if an instruction of type load or store has been added to the memory stage
                                MEM_stage.push_back(inst);
                                if (inst->instruction_type == 4) {
                                        mem_contains_load = true;
                                }
                                if (inst->instruction_type == 5) {
                                        mem_contains_store = true;
                                }
                        }
                } else {
                        // Decrement the number of execution stages for the current instruction
                        inst->num_EX_stages_remaining -= 1;
                        next_EX.push_back(inst);
                        continue;
                }
        }
        EX_stage = next_EX;
}
 
// Memory Access (MEM) represents fourth stage of the processor pipeline. Simulates instruction reaching into memory.
// Maintains in-order execution and allows instruction to access memory enough times depending on depth value.
void Simulation::MemoryAccess(){
        // Create a vector to store instructions for next iteration of the memory access stage(if needed)
        std::vector<Instruction*> next_MEM;
        for (auto inst : MEM_stage) {
                // Checks if required number of memory access stages has been completed
                if (inst->num_MEM_stages_remaining <= 1) {
                        // Push instructions to WB stage if space is available
                        if(WB_stage.size() < 2 ){
                                WB_stage.push_back(inst);
                        }
                        else {
                                next_MEM.push_back(inst);
                        }
                } else {
                        // Decrement number of memory access stages remaining for the instruction
                        inst->num_MEM_stages_remaining -= 1;
                        next_MEM.push_back(inst);
                        continue;
                }
        }
        MEM_stage = next_MEM;       
}
// Memory Access (MEM) represents fourth stage of the processor pipeline. Simulates writing back data and retiring the instruction
void Simulation::WritebackResultsAndRetire(){
    int count = std::ranges::min(2, (int)WB_stage.size());
    for (int i = 0; i < count; i++) {
        Instruction* cur_instruction = WB_stage[i];
        // Determine that the type of the current instruction is and increment the count of number of observed instructions of that type
        switch(cur_instruction->instruction_type) {
                case 1:
                        num_integer_instructions += 1;
                        break;
                case 2:
	 		num_float_instructions += 1;
                        break;
                case 3:
			num_branch_instructions += 1;
                        break;
                case 4:
			num_load_instructions += 1;
                        break;
                case 5:
			num_store_instructions += 1;
                        break;
                default:
                        break;
        }
        num_retired_instructions +=1;
        // Delete instruction object once retired
        delete cur_instruction;
        }
        WB_stage.erase(WB_stage.begin(), WB_stage.begin() + count);
}



// This is the main simulator functiosn
// Should run until num_retired_instructions == inst_count
// Processor can run multiple pipelines in parallel (instead of fetching, decoding etc. 1 inst/cycle)
// Each cycle runs all stages of pipeline (IF, ID, EX, MEM, WB)
// Advances cycle_clock by 1 each iteration
// Updates statistics if needed
// Checks if instruction fetch needs to be halted 
// Don't print end of simulation statistics which will be printed from main()
void Simulation::RunSimulation(){
        cycle_clock = 0;
        num_integer_instructions = 0;
        num_float_instructions = 0;
	num_branch_instructions = 0;
        num_load_instructions = 0;
	num_store_instructions = 0;
        ReadTrace(trace_file_name);
        while(num_retired_instructions < inst_count) {
                cycle_clock += 1;
                WritebackResultsAndRetire();
                MemoryAccess();
                InstructionIssueAndExecute();
                InstructionDecodeAndReadOperands();
                InstructionFetch();
        }
}

// Program's main function
int main(int argc, char* argv[]){
        // input arguments trace_file_name start_inst inst_count D
        if(argc >= 5){
                std::string trace_file_name = argv[1];
                double start_inst = atof(argv[2]);
                double inst_count = atof(argv[3]);
                double D = atof(argv[4]);

        fflush(stdout);
                for (int i=2; i < argc-1; i++){
                        char* end;
                        errno = 0;
                        double val = strtod(argv[i], &end);
                        if (end == argv[i] || *end != '\0' || errno == ERANGE) {
                                printf("%s is invalid character.\n", argv[i]);
                                printf(" val is %f\n", val);
                                return 1;
                        }
                }
        fflush(stdout);
                if (start_inst < 0 || inst_count < 0 || D < 0) {
                        printf("Input Error. Terminating Simulation...\n");
                        return 1;
                }
                // If no input errors, generate simulation with simulated statistics
                Simulation* s = new Simulation(trace_file_name, start_inst, inst_count, D);
                // Start Simulation
                s->RunSimulation();
                s->PrintStatistics(0);
                delete s;
                return 0;
        }
        else {
                printf("Insufficient number of arguments provided!\n");
                return 1;
        }
}