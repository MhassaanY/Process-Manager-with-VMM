#include <sstream>
#include <string> 
#include <ctime>
#include<unistd.h>
#include <chrono>
#include <thread>
#include <queue>
#include <stdio.h>
#include <iostream>
#include <fstream> // Added for file I/O
#include <vector>
#include <unordered_map>
using namespace std
const int PAGE_SIZE = 1; // 1KB page size
const int NUM_PAGES = 200;   // Total number of pages in memory
const int INVERTED_PAGE_TABLE_SIZE = 100;
const int RAM = 20;

// Page table entry structure
struct InvertedPageTableEntry {
    bool modifiedBit;
    unsigned int pid;
    int pageNo;
    bool available;
    bool referenceBit;
    int accessTime;
    int frameNumber; // Updated to store physical frame number
};

struct History {
    unsigned int starting_time;
    int completion_time;
    int waiting_time;
    int nature_of_completion; // 0 for normal, 1 for abnormal
};

struct IO {
    unsigned int location; // on which burst instruction it came
    int time; // how long it will run
    unsigned int nature; // 0 for disk IO, 1 for printer IO, 2 for network IO
};

// Structure to store information about each process
struct PCB {
    unsigned int PID;
    unsigned int PPID;
    unsigned int size; // size of processes = 200 bytes
    time_t arrival_time;
    unsigned int totalCPUInstructions;
    unsigned int Reg[5];
    struct IO IOBurst[3]; // each process has 3 IO bursts
    struct History completion_history;
    string currentState;
    unsigned int Remaining_cpuburst;
    int pc; // next instruction to be executed
    PCB* next; // Pointer to the next PCB in the linked list
    unsigned int priority;
    
};
//setting first and last to null
PCB* first=nullptr;
PCB* last=nullptr;
 // Queues for waiting
    queue<PCB> diskIOQueue;
    queue<PCB> printerQueue;
    queue<PCB> internetIOQueue;
//queue for waiting 
queue<PCB> RQ0Wait;
queue<PCB> RQ1Wait;
queue<PCB> RQ2Wait
    // Ready queues
    queue<PCB> RQ0;
    queue<PCB> RQ1;
    queue<PCB> RQ2;
//making dynamic arrays
vector<PCB> PCBs;
vector<History> history;    
// Function display
void displayProcesses(){
    if(PCBs.empty()){
        cout<<"NO PROCESSES IN THE READY QUEUE. "<<endl;
    }
    else{
        cout << "the processses are: "<<endl;
        for (auto it = PCBs.begin(); it != PCBs.end(); ++it) {
            const auto& PCBs= *it;
             cout << "PID: " << PCBs.PID<< ", PPID: " << PCBs.PPID 
             <<  ", size: " << PCBs.size << ", CPUInstructionsleft: "
              << PCBs.totalCPUInstructions << "Current State: " << PCBs.currentState  <<endl;
        }
        //DISPLAY STATUS DALLLLLLOOOOOO
}

}
    void retrieveProcessesFromFile() {
    string filename = "project.txt";
//    cout << "Enter the name of the file containing processes: ";
//    cin >> filename;

    ifstream file(filename);
    if (!file.is_open()) {
        cout << "Error: Unable to open file." << endl;
        return;
    }

    // Read processes from file and add them to the processes vector
    PCB new_process;
    while (file >> new_process.PID >> new_process.PPID >> new_process.size >> new_process.totalCPUInstructions >> new_process.priority) {
        int num_io_instructions;
        for (int i = 0; i < 3; i++) {
            file >> new_process.IOBurst[0] >> new_process.IOBurst[1] >> new_process.IOBurst[2];
        }
        new_process.currentState = 1; // Initial state
        
        // Add new process to 'processes' vector
        PCBs.push_back(new_process);
    }

    file.close();

    // Display all retrieved processes
    cout << "Retrieved Processes:" << endl;
    for (const auto& PCB : PCBs) {
        cout << "PID: " << PCB.PID << ", PPID: " << PCB.PPID << ", Size: " << PCB.size << ", Total CPU Burst: " << PCB.totalCPUInstructions << ", Proiroty: " << PCB.priority << endl;

        for (int i = 0; i < 3; ++i) {
            if (PCB.[0] != -1 || PCB.IOBurst[1] != -1 || PCB.IOBurst[2] != -1) {
                cout << "IO Instruction " << i + 1 << ": Location=" << PCB.IOBurst[0] << ", Type=" << PCB.IOBurst[1] << ", Time=" << PCB.IOBurst[2] << endl;
            } else {
                cout << "IO Instruction " << i + 1 << " is empty or incomplete." << endl;
            }
        }
    }
}
//CREATE PROCESES

void createProcess() {
  retrieveProcessesFromFile();
	    PCB new_process;

	     // Generate a unique PID
    static unordered_set<unsigned int> usedPIDs;
    unsigned int newPID;
    do {
        newPID = rand() % 1000 + 1; // Generate random PID between 1 and 1000
    } while (usedPIDs.count(newPID) > 0); // Ensure PID is unique
    usedPIDs.insert(newPID); // Insert the new PID into the set of used PIDs
	    // Ensure that the entered PID is unique and not equal to 1
	    new_process.PID = newPID;
	 //ensuring that ppid is not negative
     unsigned int ppid;
do {
    cout << "Enter PPID: ";
    cin >> ppid;

    // Check if PPID is negative
    if (ppid < 0) {
        cout << "Error: PPID cannot be negative. Please enter a non-negative value." << endl;
    }
} while (ppid < 0);

// Assign PPID after valid input
new_process.PPID = ppid;

	//now taking size of it   
	    cout << "Enter size (in bytes): ";
	    cin >> new_process.size;
       
	    cout << "Enter total CPU burst (in terms of number of instructions): ";
	    cin >> new_process.totalCPUInstructions;
	    cout << "Enter process Priority: ";
	    cin >> new_process.priority;
	    int max;
	    cout << "How many IO instructions should the process have?" << endl;
	    cin >> max;
	    //IOBurst with -1 provides a clear and consistent starting point, allowing for easier management and manipulation of IO instructions in the subsequent code.
	    for (int i = 0; i < 3; i++) {
	            new_process.IOBurst[i] = -1;
	    }
	    // Input for IO instructions
        //IT IS ACCESSING A LINKED LIST BUT AS AN ARRAY????
	    for (int i = 0; i < max; i++) {
	        cout << "Enter IO instruction " << i + 1 << " location (after how many CPU instructions): ";
	        cin >> new_process.IOBurst[0];
	        cout << "Enter IO instruction " << i + 1 << " nature (device requirement): ";
	        cin >> new_process.IOBurst[1];
	        cout << "Enter IO instruction " << i + 1 << " duration (in seconds): ";
	        cin >> new_process.IOBurst[2];
	    }
	    new_process.currentState = 1; // Initial state
	     // Write the new process to the file
	     
	    ofstream file("project.txt", ios::app);
	    
	    if (file.is_open()) {	    
		file << endl;
	    // Write the new process data to the file
	    file << new_process.PID << " " << new_process.PPID << " " << new_process.size << " " << new_process.totalCPUInstructions << " " << new_process.priority << " ";
	    for (int i = 0; i < 3; i++) {
	        file << new_process.IOBurst[0] << " " << new_process.IOBurst[1] << " " << new_process.IOBurst[2] << " ";
	    }
	    
	    file.close();
	    cout << "New process added and written to file successfully." << endl;
		} 	else {
	    cout << "Error: Unable to open file." << endl;
		}

	}
  

void terminateProcess(int PID) {
    for (auto it = PCBs.begin(); it != PCBs.end(); ++it) {
        if (it->PID == PID) {
            it->current_state = 4;
            it->nature = "abnormal";
            // Move the terminated process to completion history
            History terminated_process;
            terminated_process.PID = it->PID;
            terminated_process.starting_time = it->starting_time;
            terminated_process.ending_time = it->ending_time;
            terminated_process.waiting_time = it->waiting_time;
            terminated_process.nature = it->nature;
            completion_history.push_back(terminated_process);
            // Erase the terminated process from the processes vector
            processes.erase(it);
            cout << "Process " << PID << " terminated prematurely.\n";
            return;
        }
    }
    cout << "Process with PID " << PID << " not found.\n";
}


void endProcess(int PID) {
    for (auto it = PCBs.begin(); it != PCBs.end(); ++it) {
        if (it->PID == PID) {
            it->currentState = "TERMINATED";
            it->completion_history.nature_of_completion = 0; // Assuming 0 for normal completion
            it->completion_history.completion_time = time(nullptr);
            // Move the terminated process to completion history
            history.push_back(it->completion_history);
            // Erase the terminated process from the PCBs vector
            PCBs.erase(it);
            cout << "Process " << PID << " terminated normally.\n";
            return;
        }
    }
    cout << "Process with PID " << PID << " not found.\n";
}


void displayQueue(queue<PCB>& Q) {
    if (Q.empty()) {
        cout << "Queue is empty." << endl;
        return;
    }

    queue<PCB> temp = Q;
    while (!temp.empty()) {
        PCB p = temp.front();
        cout << "Process ID: " << p.PID << ", Priority: " << p.priority << ", State: " << p.currentState << endl;
        temp.pop();
    }
}
void displaySuspendedProcesses() {
    bool found = false;
    for (const auto& PCB : PCBs) {
        if (PCB.current_state == 3) {
            cout << "Process ID: " << PCB.PID << ", State: " << PCB.currentState << endl;
            found = true;
        }
    }
    if (!found) {
        cout << "No suspended processes." << endl;
    }
}


void displayCompletionHistory() {
    if (history.empty()) {
        cout << "Completion history is empty." << endl;
        return;
    }

    for (const auto& History : history) {
        cout << ", Start Time: " << history.starting_time << ", End Time: " << history.end_time << ", Waiting Time: " << history.waiting_time << ", Nature: " << history.nature_of_completion<< endl;
    }
}



// Function to translate virtual address to physical address using MMU
int translateAddress(InvertedPageTableEntry invertedPageTable[], int invertedPageTableSize, unsigned int pid, int pageNo, int virtualAddress) {
    // Calculate the offset from the virtual address and page size
    int offset = virtualAddress % PAGE_SIZE;

    // Search for the page table entry corresponding to the given PID and page number
    for (int i = 0; i < invertedPageTableSize; ++i) {
        if (!invertedPageTable[i].available && invertedPageTable[i].pid == pid && invertedPageTable[i].pageNo == pageNo) {
            // Calculate the physical address by combining the frame number and offset
            int physicalAddress = invertedPageTable[i].frameNumber * PAGE_SIZE + offset;
            return physicalAddress;
        }
    }
    // If the page table entry is not found, return -1 
    return -1;
}


void handlePageFault(InvertedPageTableEntry invertedPageTable[], int invertedPageTableSize, int pid, int pageNo) {
    // Search for an available entry in the inverted page table
    int index = -1;
    for (int i = 0; i < invertedPageTableSize; ++i) {
        if (invertedPageTable[i].available) {
            index = i;
            break;
        }
    }
    if (index != -1) {
        // Found an available entry
        invertedPageTable[index].pid = pid;
        invertedPageTable[index].pageNo = pageNo;
        invertedPageTable[index].available = false;
        invertedPageTable[index].modifiedBit = false; // Assuming not modified initially
        invertedPageTable[index].referenceBit = false; // Assuming not referenced initially
        cout << "Page fault handled. Page " << pageNo << " loaded into frame " << index << endl;
    } else {
        // No available entry in the inverted page table
        cout << "Error: No available entry in the inverted page table to handle page fault." << endl;
    }
}

void writeToDisk(int page) {
    // Open file for writing (replace "page_data.dat" with your desired filename)
    ofstream outfile("page_data.dat", ios::binary | ios::app);
    if (!outfile.is_open()) {
        cerr << "Error: Could not open file for writing page " << page << endl;
        return;
    }

    // Write dummy data for demonstration (replace with actual page data)
    char pageData[PAGE_SIZE] = "Sample data";
    outfile.write(reinterpret_cast<char*>(pageData), PAGE_SIZE);  // Write page data as bytes

    outfile.close();
}

void readFromDisk(int page) {
    // Open file for reading (replace "page_data.dat" with your desired filename)
    ifstream infile("page_data.dat", ios::binary);
    if (!infile.is_open()) {
        cerr << "Error: Could not open file for reading page " << page << endl;
        return;
    }

    // Read dummy data for demonstration (replace with actual page data processing)
    char pageData[PAGE_SIZE];
    infile.seekg(page * PAGE_SIZE, ios::beg); // Seek to the position of the page data
    infile.read(reinterpret_cast<char*>(pageData), PAGE_SIZE);  // Read page data as bytes
    cout << "Read data from disk for page " << page << ": " << pageData << endl;

    infile.close();
}

void handlePageReplacementLRU(InvertedPageTableEntry invertedPageTable[], int invertedPageTableSize, int pid, int pageNo, int currentTime) {
    // Find the least recently used page
    int lruIndex = 0;
    int minAccessTime = invertedPageTable[0].accessTime;
    for (int i = 1; i < invertedPageTableSize; ++i) {
        if (invertedPageTable[i].accessTime < minAccessTime) {
            lruIndex = i;
            minAccessTime = invertedPageTable[i].accessTime;
        }
    }

    // Replace the least recently used page with the new page
    invertedPageTable[lruIndex].pid = pid;
    invertedPageTable[lruIndex].pageNo = pageNo;
    invertedPageTable[lruIndex].modifiedBit = false; // Assuming not modified initially
    invertedPageTable[lruIndex].available = false;
    invertedPageTable[lruIndex].accessTime = currentTime; // Update access time
    cout << "Page " << invertedPageTable[lruIndex].pageNo << " replaced. New page " << pageNo << " loaded into frame " << lruIndex << endl;

    // Replace the least recently used page with the new page
    if (invertedPageTable[lruIndex].modifiedBit) {
        // If the page to be replaced is modified, write it back to disk
        writeToDisk(invertedPageTable[lruIndex].pageNo);
    }

    // Load the new page from disk
    readFromDisk(pageNo);

    // Update the inverted page table entry for the new page
    invertedPageTable[lruIndex].pid = pid;
    invertedPageTable[lruIndex].pageNo = pageNo;
    invertedPageTable[lruIndex].modifiedBit = false; // Assuming not modified initially
    invertedPageTable[lruIndex].available = false;
    invertedPageTable[lruIndex].accessTime = currentTime; // Update access time
    cout << "Page " << invertedPageTable[lruIndex].pageNo << " replaced. New page " << pageNo << " loaded into frame " << lruIndex << endl;
}


void retrieveProcessesFromFile() {
    string filename = "project.txt";
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "Error: Unable to open file." << endl;
        return;
    }

    // Read processes from file and add them to the processes vector
    PCB new_process;
    while (file >> new_process.PID >> new_process.PPID >> new_process.size >> new_process.totalCPUInstructions >> new_process.priority) {
        for (int i = 0; i < 3; i++) {
            file >> new_process.IOBurst[i].location >> new_process.IOBurst[i].nature >> new_process.IOBurst[i].time;
        }
        new_process.currentState = "INIT"; // Initial state
        new_process.arrival_time = time(nullptr); // Record arrival time
        // Add new process to 'PCBs' vector
        PCBs.push_back(new_process);
    }

    file.close();

    // Display all retrieved processes
    cout << "Retrieved Processes:" << endl;
    for (const auto& process : PCBs) {
        cout << "PID: " << process.PID << ", PPID: " << process.PPID << ", Size: " << process.size << ", Total CPU Burst: " << process.totalCPUInstructions << ", Priority: " << process.priority << endl;

        for (int i = 0; i < 3; ++i) {
            cout << "IO Instruction " << i + 1 << ": Location=" << process.IOBurst[i].location << ", Nature=" << process.IOBurst[i].nature << ", Time=" << process.IOBurst[i].time << endl;
        }
    }
}
void displayStatus() {
    cout << "----------- Current Process Status -----------" << endl;

    cout << "Ready Queue 0 (Highest Priority):" << endl;
    displayQueue(RQ0);
    cout << endl;
    
    cout << "Ready Queue 1 (Medium Priority):" << endl;
    displayQueue(RQ1);
    cout << endl;

    cout << "Ready Queue 2 (Lowest Priority):" << endl;
    displayQueue(RQ2);
    cout << endl;

    cout << "Suspended Processes:" << endl;
    displaySuspendedProcesses();
    cout << endl;

    cout << "Completion History:" << endl;
    displayCompletionHistory();
    
    cout << "----------------------------------------------" << endl;
}

void manualScheduling() {
    cout << "Existing Processes:" << endl;
    for (const auto& process : PCBs) {
        cout << "PID: " << process.PID << ", PPID: " << process.PPID << endl;
    }

    // Ask user which process to execute
    int selected_PID;
    cout << "Enter PID of the process to execute: ";
    cin >> selected_PID;

    // Find the process with the selected PID
    bool process_found = false;
    for (auto& process : PCBs) {
        if (process.PID == selected_PID) {
            // Add the selected process to RQ0
            RQ0.push(process);
            cout << "Process with PID " << process.PID << " added to RQ0 for execution." << endl;
            scheduleRQ0();
            process_found = true;
            break;
        }
    }

    if (!process_found) {
        cout << "Process with PID " << selected_PID << " not found." << endl;
    }
}


void AutomaticScheduleRQ0() {
    if (!RQ0.empty()) {
        while (!RQ0.empty()) {
            PCB frontProcess = RQ0.front();
            if (frontProcess.totalCPUInstructions > 4) {
                frontProcess.totalCPUInstructions -= 4;
                RQ0.pop();
                RQ1.push(frontProcess);
                cout << "Process with PID " << frontProcess.PID << " moved from RQ0 to RQ1 due to quantum expiration.\n";
            } else {
                cout << "Process with PID " << frontProcess.PID << " remains in RQ0 for execution.\n";
                if (frontProcess.IOBurst[0].location == 0) {	
                    RQ0.pop();
                    cout << "Process with PID: " << frontProcess.PID << " terminated in RQ0" << endl;
                    endProcess(frontProcess.PID);
                } else {
                    int i = 0;
                    while (i < 3 && frontProcess.IOBurst[i].location != -1) {
                        switch (frontProcess.IOBurst[i].nature) {
                            case 0:
                                IODisk.push(frontProcess);
                                cout << "Process with PID: " << frontProcess.PID << " is in Disk IO" << endl;
                                cout << "Process with PID: " << frontProcess.PID << " is accessing Disk IO" << endl;
                                this_thread::sleep_for(chrono::seconds(1));
                                IODisk.pop();
                                cout << "Process with PID: " << frontProcess.PID << " is pop out from Disk IO" << endl;
                                break;
                            case 1:
                                IOPrinter.push(frontProcess);
                                cout << "Process with PID: " << frontProcess.PID << " is in Printer IO" << endl;
                                cout << "Process with PID: " << frontProcess.PID << " is accessing Printer IO" << endl;
                                this_thread::sleep_for(chrono::seconds(1));
                                IOPrinter.pop();
                                cout << "Process with PID: " << frontProcess.PID << " is pop out from Printer IO" << endl;
                                break;
                            case 2:
                                IOInternet.push(frontProcess);
                                cout << "Process with PID: " << frontProcess.PID << " is in Internet IO" << endl;
                                cout << "Process with PID: " << frontProcess.PID << " is accessing Internet IO" << endl;
                                this_thread::sleep_for(chrono::seconds(1));
                                IOInternet.pop();
                                cout << "Process with PID: " << frontProcess.PID << " is pop out from Internet IO" << endl;
                                break;
                            default:
                                break;
                        }
                        cout << "Waiting for 2 seconds to come into RQ.\n";
                        this_thread::sleep_for(chrono::seconds(2));
                        cout << "Returned to Ready Queue for termination after accessing IO" << endl;
                        i++;
                    }
                    cout << "Process with PID: " << frontProcess.PID << " terminated in RQ0" << endl;
                    endProcess(frontProcess.PID);
                    RQ0.pop();
                }
            }
        }
    } else {
        cout << "RQ0 is empty.\n";
    }
    AutomaticScheduleRQ1();
}


void AutomaticScheduleRQ1() {
    if (!RQ1.empty()) {
        PCB shortestProcess = RQ1.front();
        for (size_t i = 1; i < RQ1.size(); ++i) {
            PCB p = RQ1.front();
            RQ1.pop();  // Remove the front element temporarily
            if (p.totalCPUInstructions < shortestProcess.totalCPUInstructions) {
                shortestProcess = p;
            }
            RQ1.push(p);  // Put the element back at the end of the queue
        }

        queue<PCB> tempQueue;
        while (!RQ1.empty()) {
            PCB p = RQ1.front();
            RQ1.pop();
            if (p.PID != shortestProcess.PID && p.totalCPUInstructions > shortestProcess.totalCPUInstructions) {
                RQ2.push(p);
                cout << "Moved process with PID " << p.PID << " from RQ1 to RQ2.\n";
            } else {
                tempQueue.push(p); 
            }
        }
        RQ1 = tempQueue;
        cout << "Executing process with PID: " << shortestProcess.PID << " from RQ1." << endl;
        if (shortestProcess.IOBurst[0].location != -1) {
            int i = 0;
            while (i < 3 && shortestProcess.IOBurst[i].location != -1) {
                switch (shortestProcess.IOBurst[i].nature) {
                    case 0:
                        IODisk.push(shortestProcess);
                        cout << "Process with PID: " << shortestProcess.PID << " is in Disk IO" << endl;
                        cout << "Process with PID: " << shortestProcess.PID << " is accessing Disk IO" << endl;
                        cout << "Process with PID: " << shortestProcess.PID << " will wait for 1 second.\n";
                        this_thread::sleep_for(chrono::seconds(1));
                        IODisk.pop();
                        cout << "Process with PID: " << shortestProcess.PID << " is pop out from Disk IO" << endl;
                        break;
                    case 1:
                        IOPrinter.push(shortestProcess);
                        cout << "Process with PID: " << shortestProcess.PID << " is in Printer IO" << endl;
                        cout << "Process with PID: " << shortestProcess.PID << " is accessing Printer IO" << endl;
                        cout << "Process with PID: " << shortestProcess.PID << " will wait for 1 second.\n";
                        this_thread::sleep_for(chrono::seconds(1));
                        IOPrinter.pop();
                        cout << "Process with PID: " << shortestProcess.PID << " is pop out from Printer IO" << endl;
                        break;
                    case 2:
                        IOInternet.push(shortestProcess);
                        cout << "Process with PID: " << shortestProcess.PID << " is in Internet IO" << endl;
                        cout << "Process with PID: " << shortestProcess.PID << " is accessing Internet IO" << endl;
                        cout << "Process with PID: " << shortestProcess.PID << " will wait for 1 second.\n";
                        this_thread::sleep_for(chrono::seconds(1));
                        IOInternet.pop();
                        cout << "Process with PID: " << shortestProcess.PID << " is pop out from Internet IO" << endl;
                        break;
                    default:
                        break;
                }
                cout << "Waiting for 2 seconds to come into RQ.\n";
                this_thread::sleep_for(chrono::seconds(2));
                cout << "Returned to Ready Queue for termination after accessing IO" << endl;
                i++;
            }
        }
        cout << "Process with PID: " << shortestProcess.PID << " terminated in RQ1" << endl;
        endProcess(shortestProcess.PID);
        RQ1.pop();
    } else {
        cout << "RQ1 is empty.\n";
    }
        AutomaticScheduleRQ2();
}

void AutomaticScheduleRQ2() {
    if (!RQ2.empty()) {
        while (!RQ2.empty()) {
            // Find the process with the highest priority
            PCB highestPriorityProcess = RQ2.front();
            queue<PCB> tempQueue = RQ2;
            while (!tempQueue.empty()) {
                PCB p = tempQueue.front();
                tempQueue.pop();
                if (p.priority > highestPriorityProcess.priority) {
                    highestPriorityProcess = p;
                }
            }

            // Execute the process with the highest priority
            cout << "Executing process with PID: " << highestPriorityProcess.PID << " from RQ2." << endl;
            // Process I/O instructions
            for (int i = 0; i < 3 && highestPriorityProcess.IOBurst[i].location != -1; ++i) {
                switch (highestPriorityProcess.IOBurst[i].nature) {
                    case 0:
                        IODisk.push(highestPriorityProcess);
                        cout << "Process with PID: " << highestPriorityProcess.PID << " is in Disk IO" << endl;
                        cout << "Process with PID: " << highestPriorityProcess.PID << " is accessing Disk IO" << endl;
                        cout << "Process with PID: " << highestPriorityProcess.PID << " will wait for 1 second.\n";
                        this_thread::sleep_for(chrono::seconds(1));
                        IODisk.pop();
                        cout << "Process with PID: " << highestPriorityProcess.PID << " is pop out from Disk IO" << endl;
                        cout << "Waiting for 2 seconds to come into RQ.\n";
                        this_thread::sleep_for(chrono::seconds(2));
                        break;
                    case 1:
                        IOPrinter.push(highestPriorityProcess);
                        cout << "Process with PID: " << highestPriorityProcess.PID << " is in Printer IO" << endl;
                        cout << "Process with PID: " << highestPriorityProcess.PID << " is accessing Printer IO" << endl;
                        cout << "Process with PID: " << highestPriorityProcess.PID << " will wait for 1 second.\n";
                        this_thread::sleep_for(chrono::seconds(1));
                        IOPrinter.pop();
                        cout << "Process with PID: " << highestPriorityProcess.PID << " is pop out from Printer IO" << endl;
                        cout << "Waiting for 2 seconds to come into RQ.\n";
                        this_thread::sleep_for(chrono::seconds(2));
                        break;
                    case 2:
                        IOInternet.push(highestPriorityProcess);
                        cout << "Process with PID: " << highestPriorityProcess.PID << " is in Internet IO" << endl;
                        cout << "Process with PID: " << highestPriorityProcess.PID << " is accessing Internet IO" << endl;
                        cout << "Process with PID: " << highestPriorityProcess.PID << " will wait for 1 second.\n";
                        this_thread::sleep_for(chrono::seconds(1));
                        IOInternet.pop();
                        cout << "Process with PID: " << highestPriorityProcess.PID << " is pop out from Internet IO" << endl;
                        cout << "Waiting for 2 seconds to come into RQ.\n";
                        this_thread::sleep_for(chrono::seconds(2));
                        break;
                    default:
                        break;
                }
            }
            // Print termination message
            cout << "Process with PID: " << highestPriorityProcess.PID << " terminated in RQ2" << endl;
            endProcess(highestPriorityProcess.PID);

            // Remove the processed process from the queue
            while (!RQ2.empty() && RQ2.front().PID != highestPriorityProcess.PID) {
                RQ2.push(RQ2.front());
                RQ2.pop();
            }
            if (!RQ2.empty()) RQ2.pop(); // Remove the executed process from the queue
        }
    } else {
        cout << "RQ2 is empty.\n";
    }
}

void Autoschedule() {
    // Load processes from the file
    loadProcessesFromFile("project.txt");

    // Check if there are any processes in the processes vector
    if (PCBs.empty()) {
        cout << "No processes to schedule." << endl;
        return;
    }

    // Iterate through each process in the processes vector
    for (const auto& process : PCBs) {
        // Add the process to RQ0
        RQ0.push(process);
        cout << "Process with PID " << process.PID << " added to RQ0 for execution." << endl;
    }
    
    AutomaticScheduleRQ0();
}

int main() {
 // Create an array to hold the inverted page table entries
    InvertedPageTableEntry invertedPageTable[INVERTED_PAGE_TABLE_SIZE];

    // Initialize inverted page table entries
    for (int i = 0; i < INVERTED_PAGE_TABLE_SIZE; ++i) {
        invertedPageTable[i].modifiedBit = false;
        invertedPageTable[i].pid = -1;
        invertedPageTable[i].pageNo = -1;
        invertedPageTable[i].available = true;
        invertedPageTable[i].referenceBit = false;
        invertedPageTable[i].frameNumber = -1; // Initialize frame number
    }

    // Example usage of functions (for demonstration purposes)
    // Not part of the main program logic
    char choice;
    do {
        cout << "\nMenu:" << std::endl;
        cout << "1. Display Processes" << std::endl;
        cout << "2. Create Process" << std::endl;
        cout << "3. Terminate Process" << std::endl;
        cout << "4. Suspend a process" << std::endl;
         cout << "5. Resume a process "<< std::endl;
        cout << "6. AUTOMATIC SCEDULING"<< std::endl;
        cout<<"7.DISPLAY COMPLETION HISTORY"<< std::endl;
        cout << "8. write page to the disk "<< std::endl;
        cout << "9. read pages from disk"<< std::endl;
        cout << "10. handle page replacement "<< std::endl;
         cout << "11. Manual Sceduling"<< std::endl;
         cout << "11. exit"<< std::endl;
        cout << "Enter your choice: ";
        cin >> choice;

        switch (choice) {
            case '1':
                displayProcesses();
                break;
            case '2':
                createProcess();
                break;
            case '3':
                int PID;
                cout << "Enter the PID of the process to terminate: ";
                cin >> PID;
                terminateProcess(PID);
                break;
            case '4':
              displaySuspendedProcesses() 
                break;
            case '5':
            int resume_PID;
                cout << "Enter PID of the process to resume: ";
                cin >> resume_PID;
                resume(resume_PID);
                break;

                cout << "" << std::endl;
               void handlePageFault(InvertedPageTableEntry invertedPageTable[], int invertedPageTableSize, int pid, int pageNo);
                break;
                cout<<""
            case '6':
                     // Call autoschedule function
                 Autoschedule(); // Automatic scheduling option
                break;
            case '7':
            displayCompletionHistory();
            break;    
            case 8:
               void writeToDisk(int page);
              
                break; 
            case '9':
               void readFromDisk(int page);
                break;
            case '10':
               retrieveProcessesFromFile();
               break; 
             case '11':
               manualScheduling();
               break; 
            case '12':
                 cout << "Exiting...\n";
                break;
            default:
                cout << "Invalid choice. Please try again." << std::endl;
        }
    } while (choice != '12');

    return 0;
}
