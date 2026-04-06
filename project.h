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

// Definition of Instruction struct
struct Instruction {
	// Contains information about instruction type, its PC, any PC dependencies, number of EX and MEM stages remaining
	// if ex and mem instructions are completed and sequence of numbers
	int instruction_type;
	std::string instruction_pc;
	std::vector<std::string> dependencies;
	int num_MEM_stages_remaining;
	int num_EX_stages_remaining;
	bool ex_done;
	bool mem_done;
	long long seq_num;

};

// main simulations class 
class Simulation {
	public:
		Simulation(std::string trace_file_name_in, double start_inst_in, double inst_count_in, double D_in) {
            trace_file_name = trace_file_name_in;
        	start_inst = start_inst_in;
            inst_count = inst_count_in;
            D = D_in;
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
		};

		void ReadTrace(std::string filename);
		bool ValidateStructuralHazards(std::vector<Instruction*> cur_stage);
		bool CheckDataHazard(Instruction* current_inst);
		Instruction* LatestPC(Instruction* current_inst, const std::string& dependencies_pc);
		void InstructionFetch();
		void InstructionDecodeAndReadOperands();
		void InstructionIssueAndExecute();
		void MemoryAccess();
		void WritebackResultsAndRetire();
		void RunSimulation();

		// get the fequency based on the depth D
		double get_frequency() {
			if (D == 1) {
				return 1.0;

			} else if (D == 2) {
				return 1.2;

			} else if (D == 3) {
				return 1.7;

			} else if (D == 4) {
				return 1.8;
			}

			return 1.0; 
		}

		// This function should be called to print periodic and/or end-of-simulation statistics
		void PrintStatistics(double lambda) {
			printf("\n---Simulation Results---\n");
			simulated_stats[0] = (num_integer_instructions / inst_count) * 100;  // percentage of total integer instructions
			simulated_stats[1] = (num_float_instructions / inst_count) * 100;  // percentage of total floating point instructions
			simulated_stats[2] = (num_branch_instructions / inst_count) * 100;  // percentage of total branch instructions
			simulated_stats[3] = (num_load_instructions / inst_count) * 100;  // percentage of total load instructions
			simulated_stats[4] = (num_store_instructions / inst_count) * 100;  // percentage of total store instructions
		int total =
			num_integer_instructions +
			num_float_instructions +
			num_branch_instructions +
			num_load_instructions +
			num_store_instructions;

		// get the frequency and compute the executiion time 
		double freq_ghz = get_frequency();
		double exec_time = cycle_clock / (freq_ghz * 1e6);

		printf("Total retired = %d\n", total);
		printf("Execution time = %.6f\n", exec_time);	
		printf("Number of cycles completed = %f\n\n", cycle_clock);

		printf("Percentage integer instructions = %f\n", simulated_stats[0]);
		printf("Percentage float instructions  = %f\n",simulated_stats[1]); 
		printf("Percentage branch instructions = %f\n", simulated_stats[2]);
		printf("Percentage load instructions = %f\n", simulated_stats[3]);
		printf("Percentage storage instructions  = %f\n", simulated_stats[4]);

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
		long long next_seq_num;

		std::queue<Instruction*> instructions_queue; //this contains just a list of instructions in order of the input file
		std::vector<Instruction*> IF_stage;
		std::vector<Instruction*> ID_stage;
		std::vector<Instruction*> EX_stage;
		std::vector<Instruction*> MEM_stage;
		std::vector<Instruction*> WB_stage;		
		std::vector<Instruction*> retired_instructions;;	

		double simulated_stats[5];  // Store simulated statistics
		double cycle_clock;        // current time during simulation
};

#endif