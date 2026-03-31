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


/*
// Create and add an event to the event queue and insert it in the correct priority order based on event time
void EventQueue::ScheduleEvent(double etime, int etype, ElementQueueNode* qnode) {
    EventQueueNode* event = CreateEvent(etime, etype, qnode);
        if (head == nullptr || tail == nullptr) {
                head = event;
                tail = event;
        }
        else {
                EventQueueNode* current = head;
                if (etime < current->event_time) {
                        event -> next = head;
                        head = event;
                        return;
                }
while (current->next != nullptr) {
        if (etime < current->next->event_time) {
                event -> next = current->next;
                current -> next = event;
                return;
        }
        if (etime == current->next->event_time) {
        //if Departure and arrival happen at same time
                if (event->event_type == 3 && current->next->event_type ==1) {
                        event -> next = current->next;
                        current -> next = event;
                        return;
                }
//if Service and arrival happen at same time
                if (event->event_type == 2 && current->next->event_type ==1) {
                        event -> next = current->next;
                        current -> next = event;
                        return;
                }
        }
        current=current->next;
        }
        if (current-> next == nullptr) {
                current->next = event;
                tail = event;
                event->next = nullptr;
        }
        }
}
*/


// This function is called from simulator if the next event is an arrival for evaluation with nurse
// Should update simulated statistics based on new arrival
// Should update system state
// Should schedule the arrival event for the next element in the Element Queue (except for last arrival)
// Should drop patient if the maximum capacity has been reached
// Should schedule a start service event if less than m1 nurses are busy
// Should place patient in wait list to see nurse for evaluation if all m1 nurses busy
// *arriving_patient points to queue node that arrived

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

void Simulation::InstructionFetch(int num_to_fetch) {
    // Retrieve 2 instructions from the queue where they are stored, they are currently in IF stage
    num_to_fetch = 2 - IF_stage.size();
    //printf("num to fetch %d\n", num_to_fetch);
    cycle_clock += 1;
    for (int i = 0; i< num_to_fetch; i++) {
        Instruction* cur_instruction = instructions_queue.front(); 
        instructions_queue.pop();
        IF_stage.push_back(cur_instruction);
    }
    if (ValidateStructuralHazards(IF_stage)) {
        for (int i = 0; i < num_to_fetch;i++) {
                ID_stage.push_back(IF_stage[i]);
        }
    }
    else {
        ID_stage.push_back(IF_stage[0]);
    }
    IF_stage.erase(IF_stage.begin(), IF_stage.begin() + num_to_fetch);
   // printf("size of IF after erase %ld\n", IF_stage.size());
    // move on to next stage
  //  InstructionDecodeAndReadOperands();

}
// This function represents the evaluation service once patient sees the nurse
void Simulation::InstructionDecodeAndReadOperands(){
    cycle_clock += 1;
    int count =std::ranges::min(2, (int)ID_stage.size());
    //printf("size of ID before erase %ld\n", ID_stage.size());
        if (ValidateStructuralHazards(ID_stage)) {
                for (int i = 0; i < count; i++) {
                Instruction* cur_instruction = ID_stage[i];
                EX_stage.push_back(cur_instruction);
                }
        }
        else {
                count = 1;
                EX_stage.push_back(ID_stage[0]);
        }
        ID_stage.erase(ID_stage.begin(), ID_stage.begin() + count);
        //printf("size of ID after erase %ld\n", ID_stage.size());

}     
void Simulation::InstructionIssueAndExecute(bool all_cycles_completed){
    //cycle_clock += 1;    
    //bool all_cycles_completed = true;
       // printf("size of EX before erase %ld\n", EX_stage.size());
        cycle_clock += 1;
        //int count = std::ranges::min(2, (int)EX_stage.size());
        std::vector<Instruction*> next_EX;
        std::vector<Instruction*> to_MEM;

                /*for (int j = 0; j< D;j++) {
                        cycle_clock += 1;    

                        if (ValidateStructuralHazards(ID_stage)) {

                                for (int i = 0; i < count; i++) {
                                        Instruction* cur_instruction = ID_stage[i];
                                        EX_stage.push_back(cur_instruction);
                                }
                        }
                        else {
                                count = 1;
                                EX_stage.push_back(ID_stage[0]);
                        }

                        if (j < D-1) {
                                int ex_size = (int)EX_stage.size();
                                EX_stage.erase(EX_stage.begin(), EX_stage.begin() + ex_size);
                        }

                }*/

        for (auto inst : EX_stage) {

                if (inst->num_EX_stages_remaining == 0) {
                        MEM_stage.push_back(inst);
                } else {
                        inst->num_EX_stages_remaining -= 1;
                        next_EX.push_back(inst);
                }
        }
       // EX_stage.erase(EX_stage.begin(), EX_stage.begin() + count);
        EX_stage = next_EX;
       // printf("size of EX after erase %ld\n", EX_stage.size());
}
    // check if we can advance to next stage or we have to repeat execution cycle
    /*if (all_cycles_completed) {
        int count = std::min(2, (int)ID_stage.size());

        for (int i = 0; i < count; i++) {
        ID_stage.pop_back();
        }

        MemoryAccess();
    }
    else {
        for (int i = 0;i < 2; i++) {
                printf(" here size ID %ld\n", ID_stage.size());

                EX_stage.pop_back();
        }
        InstructionIssueAndExecute();
    }
    */

// This function is called from simulator if the next event is an arrival for treatment for top priority patient from PQ
// Should update simulated statistics based on new arrival
// Should update system state
// Should schedule a start service event if the server is idle
// *arriving_patient points to priority queue patient that arrived

void Simulation::MemoryAccess(bool all_cycles_completed){
        /*int count = std::ranges::min(2, (int)EX_stage.size());

        for (int j = 0; j< D;j++) {
                cycle_clock += 1;    

                if (ValidateStructuralHazards(EX_stage)) {

                        for (int i = 0; i < count; i++) {
                                Instruction* cur_instruction = EX_stage[i];
                                MEM_stage.push_back(cur_instruction);
                        }
                }
                else {
                        count = 1;
                        MEM_stage.push_back(EX_stage[0]);
                }
                if (j < D-1) {
                        int mem_size = (int)MEM_stage.size();
                        MEM_stage.erase(MEM_stage.begin(), MEM_stage.begin() + mem_size);
                }

        }        
        EX_stage.erase(EX_stage.begin(), EX_stage.begin() + count);*/


        printf("size of MEM before erase %ld\n", MEM_stage.size());
        cycle_clock += 1;
        //int count = std::ranges::min(2, (int)MEM_stage.size());
        std::vector<Instruction*> next_MEM;

        for (auto inst : MEM_stage) {

                if (inst->num_MEM_stages_remaining) {
                        WB_stage.push_back(inst);
                } else {
                        inst->num_MEM_stages_remaining -= 1;
                        next_MEM.push_back(inst);
                }
        }
        //MEM_stage.erase(MEM_stage.begin(), MEM_stage.begin() + count);
        MEM_stage = next_MEM;
        printf("size of MEM after erase %ld\n", MEM_stage.size());        
}
// This function represents the treatment service once patient is placed in available room
void Simulation::WritebackResultsAndRetire(){
        printf("here \nß");
    cycle_clock += 1;    
    int count = std::ranges::min(2, (int)WB_stage.size());
   
    for (int i = 0; i < count; i++) {
        Instruction* cur_instruction = WB_stage[i];
        // Determine the type of the instruction that is retiring
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
        }
        //MEM_stage.erase(MEM_stage.begin(), MEM_stage.begin() + count);
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

        if (D == 2 || D == 4) {
                num_EX_stages_remaining = 2;
                }
        else if (D == 3 || D == 4) {
                num_MEM_stages_remaining = 3;
        }
        else {
                num_MEM_stages_remaining = 1;
                num_EX_stages_remaining = 1;
        }

        ReadTrace(trace_file_name);
        printf("size queue %ld\n", instructions_queue.size());
        //while(!instructions_queue.empty()) {
        while (!instructions_queue.empty() ||
                !IF_stage.empty() ||
                !ID_stage.empty() ||
                !EX_stage.empty() ||
                !MEM_stage.empty() ||
                !WB_stage.empty()) {
                InstructionFetch(2);
                InstructionDecodeAndReadOperands();

                if (D == 2 || D == 4) {
                        num_EX_stages_remaining = 2;
                }
                else {
                        num_EX_stages_remaining = 1;
                        num_MEM_stages_remaining = 1;
                }


                for (int i = 0;i < num_EX_stages_remaining;i++) {
                        InstructionIssueAndExecute(false);
  
                }                


                if (D == 3 || D == 4) {
                        num_MEM_stages_remaining = 3;
                }
                else {
                        num_MEM_stages_remaining = 1;
                }

                for (int i = 0; i<num_MEM_stages_remaining;i++) {
                        MemoryAccess(false);
                }
                all_MEM_cycles_completed = true;
                //Remove EX
                //if (num_MEM_stages_remaining == 0) {
              
               // printf("here 4\n");
                WritebackResultsAndRetire();
               // printf("here 5\n");
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