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


void Simulation::ReadTrace(std::string filename){
    std::ifstream file(filename);

    file.seekg(std::ios::beg); // Start from the beginning of the file

    // Skip all lines(instructions) until start_inst
    for (int i = 0; i < start_inst - 1; ++i) {
        // Ignore characters up to the next newline character
        file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }


    // String to store each line of the file.
    std::string line;
    double current_line_num = start_inst;
    double target_line = start_inst + inst_count;


    if (file.is_open()) {
        while (current_line_num < target_line && getline(file, line)){
                //std::cout << line <<std::endl;
                current_line_num += 1;
                std::stringstream ss(line);
                std::string token;
                getline(ss, token, ',');
                std::string pc = token;
                getline(ss, token, ',');
                int type = stoi(token);
                std::vector<std::string> pc_dependencies;
                while (getline(ss, token, ',')) {
                        pc_dependencies.push_back(token);
                }
                Instruction* inst = new Instruction;
                inst->instruction_type = type;
                inst->instruction_pc = pc;

                inst->dependencies = pc_dependencies;
                inst->ex_done =false;
                inst->mem_done=false;
                inst->seq_num=next_seq_num++;

                if (D == 2 || D == 4) {
                        inst->num_EX_stages_remaining = 2;
                }
                else {
                        inst->num_EX_stages_remaining = 1;
                        inst->num_MEM_stages_remaining = 1;
                }
               
                if (D == 3 || D == 4) {
                        inst->num_MEM_stages_remaining = 3;
                }
                else {
                        inst->num_EX_stages_remaining = 1;
                        inst->num_MEM_stages_remaining = 1;
                }
                


                instructions_queue.push(inst);

        }
        file.close();
    }
    else {
        printf("file doesnt exist\n");
        exit(1);
    }

}

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

Instruction* Simulation::LatestPC(Instruction* current_inst, const std::string& dependencies_pc){
        Instruction* latest = nullptr; 
        for (Instruction* inst : IF_stage){
                if(inst==nullptr){
                        continue;
                }
                else if(inst->instruction_pc==dependencies_pc && inst->seq_num<current_inst->seq_num){
                        if (latest==nullptr || inst->seq_num>latest->seq_num){
                                latest=inst;
                        }

                }
        }
        for (Instruction* inst : ID_stage){
                if(inst==nullptr){
                        continue;
                }
                else if(inst->instruction_pc==dependencies_pc && inst->seq_num<current_inst->seq_num){
                        if (latest==nullptr || inst->seq_num>latest->seq_num){
                                latest=inst;
                        }

                }
        }
        for (Instruction* inst : EX_stage){
                if(inst==nullptr){
                        continue;
                }
                else if(inst->instruction_pc==dependencies_pc && inst->seq_num<current_inst->seq_num){
                        if (latest==nullptr || inst->seq_num>latest->seq_num){
                                latest=inst;
                        }

                }
        }
        for (Instruction* inst : MEM_stage){
                if(inst==nullptr){
                        continue;
                }
                else if(inst->instruction_pc==dependencies_pc && inst->seq_num<current_inst->seq_num){
                        if (latest==nullptr || inst->seq_num>latest->seq_num){
                                latest=inst;
                        }

                }
        }
        for (Instruction* inst : WB_stage){
                if(inst==nullptr){
                        continue;
                }
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

bool Simulation::CheckDataHazard(Instruction* current_inst){
        for (const std::string& dependencies_pc: current_inst->dependencies){
                Instruction* latest = LatestPC(current_inst,dependencies_pc);
                if(latest==nullptr){
                        continue;
                }

                if(latest->instruction_type == 1 || latest->instruction_type == 2){
                        if (!latest->ex_done){
                                return false;
                        }

                }
                if(latest->instruction_type == 4 || latest->instruction_type == 5){
                        if (!latest->mem_done){
                                return false;
                        }

                }

        }
        
        return true;  
}

void Simulation::InstructionFetch(int num_to_fetch) {
    // Retrieve 2 instructions from the queue where they are stored, they are currently in IF stage
      while (IF_stage.size() < 2 && !instructions_queue.empty()) {
        IF_stage.push_back(instructions_queue.front());
        instructions_queue.pop();
    }

    // Step 2: move from IF into ID (up to 2 total in ID)
    // Only move if ID has room — stall otherwise
    while (!IF_stage.empty() && ID_stage.size() < 2) {
        ID_stage.push_back(IF_stage.front());
        IF_stage.erase(IF_stage.begin());
    }
}
// This function represents the evaluation service once patient sees the nurse
void Simulation::InstructionDecodeAndReadOperands(){
    //printf("size of ID before erase %ld\n", ID_stage.size());
    int count;
    if (ValidateStructuralHazards(ID_stage)) {
        count = std::ranges::min(2, (int)ID_stage.size());
    } else {
        count = 1; // structural hazard: only issue one
    }
                for (int i = 0; i < count; i++) {
                Instruction* cur_instruction = ID_stage[i];
                if(!CheckDataHazard(cur_instruction)){
                        count=i;
                        break;
                }
                if (cur_instruction->instruction_type == 3) {
                        halt_instruction_fetch = true;
                }
                EX_stage.push_back(cur_instruction);
                }
       
        ID_stage.erase(ID_stage.begin(), ID_stage.begin() + count);
        //printf("size of ID after erase %ld\n", ID_stage.size());

}     

void Simulation::InstructionIssueAndExecute(bool all_cycles_completed){
           if (EX_stage.empty()){

            return;
        }

        std::vector<Instruction*> next_EX;
        std::vector<Instruction*> to_MEM;
        bool mem_contains_load = false;
        bool mem_contains_store = false;

        for (auto inst : MEM_stage) {
                if (inst->instruction_type == 4) {
                        mem_contains_load = true;
                }
                if (inst -> instruction_type == 5) {
                        mem_contains_store = true;
                }
        }
        for (auto inst : EX_stage) {
                if (inst->num_EX_stages_remaining <= 1) {
                        if ((inst->instruction_type == 4 && mem_contains_load)  || (inst->instruction_type == 5 && mem_contains_store) || MEM_stage.size() >= 2 ) {
                               next_EX.push_back(inst);
 
                        }
                        else {
                                if (inst->instruction_type == 3) {
                                        halt_instruction_fetch = false; 
                                } 
                                MEM_stage.push_back(inst);
                        }
                } else {
                        inst->num_EX_stages_remaining -= 1;
                        next_EX.push_back(inst);
                        continue;
                }
        }
        EX_stage = next_EX;
}
 

// This function is called from simulator if the next event is an arrival for treatment for top priority patient from PQ
// Should update simulated statistics based on new arrival
// Should update system state
// Should schedule a start service event if the server is idle
// *arriving_patient points to priority queue patient that arrived

void Simulation::MemoryAccess(bool all_cycles_completed){
        std::vector<Instruction*> next_MEM;
        std::vector<Instruction*> to_WB;
        for (auto inst : MEM_stage) {
                if (inst->num_MEM_stages_remaining <= 1) {
                        if(WB_stage.size() < 2 ){
                                WB_stage.push_back(inst);
                        }
                        else {
                                next_MEM.push_back(inst);
                        }
                } else {
                        inst->num_MEM_stages_remaining -= 1;
                        next_MEM.push_back(inst);
                        continue;
                }
        }
        MEM_stage = next_MEM;       
}
// This function represents the treatment service once patient is placed in available room
void Simulation::WritebackResultsAndRetire(){
    int count = std::ranges::min(2, (int)WB_stage.size());
   
    for (int i = 0; i < count; i++) {
        Instruction* cur_instruction = WB_stage[i];
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
        delete cur_instruction;
        }
        WB_stage.erase(WB_stage.begin(), WB_stage.begin() + count);
}



// This is the main simulator functiosn
// Should run until departure_count == total_departures
// Needs to schedule the first arrival on the Event Queue
// Each iteration determines what the next event is from the Event Queue
// Calls appropriate function based on next event: ProcessArrival(), StartService(), ProcessDeparture()
// Advances cycle_clock to next event
// Updates queue statistics if needed
// Print statistics if departure_count is a multiple of print_period
// Don't print end of simulation statistics which will be printed from main()
void Simulation::RunSimulation(){
        cycle_clock = 0;
        num_integer_instructions = 0;
        num_float_instructions = 0;
	num_branch_instructions = 0;
        num_load_instructions = 0;
	num_store_instructions = 0;

        num_EX_stages_remaining = 1;
        num_MEM_stages_remaining = 1;
        if (D == 2 || D == 4) {
                num_EX_stages_remaining = 2;
                }
        else if (D == 3 || D == 4) {
                num_MEM_stages_remaining = 3;
        }

        ReadTrace(trace_file_name);
        printf("size queue %ld\n", instructions_queue.size());
        while(num_retired_instructions < inst_count) {
                printf("cycle %f | IF %ld ID %ld EX %ld MEM %ld WB %ld retired %d\n",
    cycle_clock,
    IF_stage.size(),
    ID_stage.size(),
    EX_stage.size(),
    MEM_stage.size(),
    WB_stage.size(),
    num_retired_instructions);
                cycle_clock += 1;
                WritebackResultsAndRetire();
                MemoryAccess(true);
                InstructionIssueAndExecute(true);
                InstructionDecodeAndReadOperands();
                InstructionFetch(2);


        }
        //The program should continue the simulation starting from instruction fetch
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
                // If no input errors, generate simulation with simulated statistics based on formulas from class
                Simulation* s = new Simulation(trace_file_name, start_inst, inst_count, D);
                // Start Simulation
                printf("Simulating traces ");//with lambda = %f, mu_e = %f, mu_t = %f, mu_c = %f, B = %f, R = %f, m1 = %d, m2 = %d, S = %d\n", lambda, mu_e, mu_t,mu_c, buffer, R, m1, m2, random_seed); 
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