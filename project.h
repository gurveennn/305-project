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
/*struct ComparePriority {
    bool operator()(const ElementQueueNode* a, const ElementQueueNode* b) const {
        // Returns true if 'a' has LOWER priority than 'b' (for a Max-Heap)
        return a->patient_priority < b->patient_priority;
    }
};*/
// Definition of a Queue Node in the Event Queue
struct InstructionEventQueueNode {
    //double event_time; // event start time
    int instruction_type;   // Event type. 1: Arrival; 2: Start Service; 3: Departure
    //ElementQueueNode* qnode;  // pointer to corresponding element in the Element Queue
    //truct EventQueueNode *next;  // pointer to next event
	
};

// Class including all elements in the simulation 
/*class ElementQueue {
	public:
		ElementQueue(int seed, double lambda, double mu_e, double mu_t, double mu_c) {
			current = 0; // current element is the first element in the element array
			InitializeQueue(seed, lambda, mu_e, mu_t, mu_c);  // create nodes in the queue based on arrival and service distributions
		};
		~ElementQueue() {
        };
		ElementQueueNode* GetCurrentElement() {
			return &ElementArray[current];
		};
		ElementQueueNode* GetElementAtIndex(uint64_t index) {
			if (index < size)
				return &ElementArray[index];
			else	
				return nullptr;
		};

		ElementQueueNode* AdvanceToNextElement() {
			if (current < (size - 1)) {
				current++;
				return &ElementArray[current];
			} 
			else {
				return nullptr; 
			}
		};
	private:
		void InitializeQueue(int seed, double lambda, double mu_e, double mu_t, double mu_c);
		uint64_t size;  // total size of element queue
		uint64_t current; // Point to the current node being processed for arrival event
		std::vector<ElementQueueNode> ElementArray;  // Array containing all elements, created at the beginning of simulation
	};
*/
// Event Queue for events that have been scheduled, implemented as a priority queue
/*class EventQueue {
	public:
		EventQueue() {
			head = nullptr;
			tail = nullptr; 
			num_events = 0;
		};
		~EventQueue() {
            while (head != nullptr) {
                EventQueueNode* tmp = head;
                head = head->next;  
                delete(tmp); 
            }
		}; 
		EventQueueNode* CreateEvent(double etime, int etype, ElementQueueNode* qnode) {
			EventQueueNode* node = new(EventQueueNode); 
  			node->event_time = etime;
  			node->event_type = etype;
  			node->qnode = qnode;
			node->next = nullptr;
			return node;
		};
		void ScheduleEvent(double etime, int etype, ElementQueueNode* qnode);
		EventQueueNode* GetNextEvent();
		EventQueueNode* GetHeadEvent();
		bool IsEmpty() {
			return (num_events == 0);
		};
		// Remove an event from memory, allow for removal of head of queue
		void RemoveEvent(EventQueueNode* node) {
			if (node == head) {
				head = head->next;
				if (head == nullptr) {
					tail = nullptr;
				}
				delete node;
				return;
			}
			else {
				delete node;
			}
		};
		void PrintEventQueue() {   // Use this for debugging purposes
			EventQueueNode* node;
  			printf("EventQ = ");
  			for (node = head; node; node = node->next)
    			printf("(%f,%d) ", node->event_time, node->event_type);
  			printf("\n");
		};

	private:
	    EventQueueNode* head;
    	EventQueueNode* tail;
		uint64_t num_events; 
};
*/
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

			num_integer_instructions = 0;
	 		num_float_instructions = 0;
			num_branch_instructions = 0;
			num_load_instructions = 0;
			num_store_instructions = 0;
			num_EX_stages_remaining = 0;
			num_MEM_stages_remaining= 0;
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
		int num_EX_stages_remaining;
		int	num_MEM_stages_remaining;

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