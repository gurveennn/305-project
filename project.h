#include<stdio.h>
#include<time.h>
#include<math.h>
#include<stdlib.h>
#include<unistd.h>
#include<assert.h>
#include<float.h>
#include <cstdint>
#include <vector>
#include <list>
#include <cerrno>
#include <limits.h>
#include <iostream>
#include <cmath>
#include <string>

#include <queue>
#ifndef PROJECT_H_
#define PROJECT_H_
#define MAX_SIZE 20000 

// Definition of a Queue Node including arrival and service time
struct Instruction {
    double arrival_time;  // customer arrival time, measured from time t=0, inter-arrival times exponential
	int instruction_type;
	std::string instruction_pc;
	std::vector<std::string> dependencies;
	int num_MEM_stages_remaining;
	int num_EX_stages_remaining;


};

class Simulation {
	public:
		Simulation(std::string trace_file_name_in, double start_inst_in, double inst_count_in, double D_in) {
            trace_file_name = trace_file_name_in;
        	start_inst = start_inst_in;
            inst_count = inst_count_in;
            D = D_in;

			std::queue<Instruction*> traced_instructions;
			std::vector<Instruction*> IF_stage;
			std::vector<Instruction*> ID_stage;
			std::vector<Instruction*> EX_stage;
			std::vector<Instruction*> MEM_stage;
			std::vector<Instruction*> WB_stage;


			//ElementQ = new ElementQueue(seed, lambda, mu_e, mu_t, mu_c);
			//E = new EventQueue();
			
			//P = new EventQueue();
			//C = new EventQueue();
			halt_instruction_fetch = false;
			num_integer_instructions = 0;
	 		num_float_instructions = 0;
			num_branch_instructions = 0;
			num_load_instructions = 0;
			num_store_instructions = 0;
			num_EX_stages_remaining = 0;
			num_MEM_stages_remaining= 0;
			num_retired_instructions = 0;
			all_EX_cycles_completed = false;
			all_MEM_cycles_completed = false;
			cycle_clock = 0;

			// Default statistics to 0
			simulated_stats[0] = 0; 
			simulated_stats[1] = 0; 
			simulated_stats[2] = 0; 
			simulated_stats[3] = 0;
			simulated_stats[4] = 0;	
		};
		~Simulation() {
			//delete ElementQ;
		//	delete E;
			//delete P;
			//delete C;
		};

		void ReadTrace(std::string filename);
		bool ValidateStructuralHazards(std::vector<Instruction*> cur_stage);
		void InstructionFetch(int num_to_fetch);
		void InstructionDecodeAndReadOperands();
		void InstructionIssueAndExecute(bool all_cycles_completed);
		void MemoryAccess(bool all_cycles_completed);
		void WritebackResultsAndRetire();
		void RunSimulation();

		// This function should be called to print periodic and/or end-of-simulation statistics
		void PrintStatistics(double lambda) {
			simulated_stats[0] = (num_integer_instructions / inst_count) * 100;  // simulated mean number
			simulated_stats[1] = (num_float_instructions / inst_count) * 100;  // simulated mean response time
			simulated_stats[2] = (num_branch_instructions / inst_count) * 100;  // simulated mean waiting time for evaluation
			simulated_stats[3] = (num_load_instructions / inst_count) * 100;  // simulated mean waiting time for treatment
			simulated_stats[4] = (num_store_instructions / inst_count) * 100;  // simulated clean time
		int total =
			num_integer_instructions +
			num_float_instructions +
			num_branch_instructions +
			num_load_instructions +
			num_store_instructions;

		printf("Total retired = %d\n", total);
		printf("Expected = %f\n", inst_count);		


			printf("number of cycles completed %f\n", cycle_clock);
			printf("percentage integer instructions = %f\n", simulated_stats[0]);
			printf("percentage float instructions  = %f\n",simulated_stats[1]); 
			printf("percentage branch instructions = %f\n", simulated_stats[2]);
			printf("percentage load instructions= %f\n", simulated_stats[3]);
			printf("percentage storage instructions  = %f\n", simulated_stats[4]);

		};
	private:
	    std::string trace_file_name;
        double start_inst;
        double inst_count;
        double D;

		// Histogram stats
		double num_integer_instructions;
		double num_float_instructions;
		double num_branch_instructions;
		double num_load_instructions;
		double num_store_instructions;
	    bool all_EX_cycles_completed;
		bool all_MEM_cycles_completed;
		bool halt_instruction_fetch;
		int num_EX_stages_remaining;
		int	num_MEM_stages_remaining;
		int num_retired_instructions;
		//ElementQueue* ElementQ;     // Element Queue for all elements used in simulation
		//EventQueue* E;              // Event Queue for patients arriving to emergency dept and waiting for evaluation	
		///EventQueue* P;              // Event Queue to model treatment process
		//∂EventQueue* C;              // Event Queue to model cleaning process

		std::queue<Instruction*> instructions_queue; //this contains just a list of instructions in order of the input file
		std::vector<Instruction*> IF_stage;
		std::vector<Instruction*> ID_stage;
		std::vector<Instruction*> EX_stage;
		std::vector<Instruction*> MEM_stage;
		std::vector<Instruction*> WB_stage;		


		double simulated_stats[5];  // Store simulated statistics
		double cycle_clock;        // current time during simulation
};

#endif