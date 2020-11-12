#include "SrcMain.h"
#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <iostream>
#include <fstream>
#include <map>
#include <cstring>

extern int proccparse(); // NOLINT
struct yy_buffer_state; // NOLINT
extern void procc_flush_buffer(yy_buffer_state* b); // NOLINT
extern int procclex_destroy(); // NOLINT
extern FILE* proccin; // NOLINT

extern int gLineNumber;
extern NProgram* gProgram;
bool gSyntaxError = false;

// CHANGE ANYTHING ABOVE THIS LINE AT YOUR OWN RISK!!!!

void outputAssembly_virtual(CodeContext& c) {
    std::ofstream output;
    output.open("emit.txt");
    for (std::pair<std::string, std::vector<std::string> > line: c.ops) {
        output << line.first << " ";
        for (int i = 0; i < static_cast<int>(line.second.size() - 1); ++i) {
            output << line.second[i] << ",";
        }
        if (!line.second.empty()) {
            output << line.second[line.second.size() - 1];
        }
        output << "\n";
    }
    output.close();
}

void outputAssembly_real(CodeContext& c, const std::map<std::string, std::string, regComp>& regMap) {
    std::ofstream output;
    output.open("emit.txt");
    for (std::pair<std::string, std::vector<std::string> > line: c.ops) {
        output << line.first << " ";
        for (int i = 0; i < static_cast<int>(line.second.size() - 1); ++i) {
            // if the param is a virtual register, replace with real register
            if (line.second[i].find("%") != std::string::npos) {
                output << regMap.at(line.second[i]) << ",";
            }
            else {
                output << line.second[i] << ",";
            }
        }
        if (!line.second.empty()) {
            if (line.second[line.second.size() - 1].find("%") != std::string::npos) {
                output << regMap.at(line.second[line.second.size() - 1]);
            }
            else {
                output << line.second[line.second.size() - 1];
            }
        }
        output << "\n";
    }
    output.close();
}

void registerAllocation(CodeContext& c, std::map<std::string, std::string, regComp>& regMap) {
    std::map<std::string, bool> availability = { {"r1", true},
                                                 {"r2", true},
                                                 {"r3", true},
                                                 {"r4", true},
                                                 {"r5", true},
                                                 {"r6", true},
                                                 {"r7", true} }; // register availability, initialize all to true
    for (int i = 0; i < c.ops.size(); ++i) {
        for (auto it = c.regIntervals.begin(); it != c.regIntervals.end(); ++it) {
            if (i < it->second.first) {
                break; // break out of inner loop early because instruction # should always be greater than VR's first appearance
            }
            // if a virtual register just expired (at the last instruction for that VR), the assigned real register is now free
            if (it->second.second == i && it->second.first != i) {
                std::string realReg = regMap[it->first]; // the real register assigned to the expired VR
                availability[realReg] = true; // set the real register to available
            }
            // if VR is now active, assign first available real register
            if (it->second.first == i) {
                std::string firstAvailable;
                // find the first available register, assuming we always have enough
                for (auto availabilityIt = availability.begin(); availabilityIt != availability.end(); ++availabilityIt) {
                    if (availabilityIt->second) {
                        availabilityIt->second = false; // set availability to false
                        firstAvailable = availabilityIt->first;
                        break;
                    }
                }
                regMap[it->first] = firstAvailable;
            }
        }
    }
}

// output virtual register intervals and mapping from virtual to real registers
void outputReg(CodeContext& c, const std::map<std::string, std::string, regComp>& regMap) {
    std::ofstream output;
    output.open("reg.txt");
    output << "INTERVALS:\n";
    for (auto it = c.regIntervals.begin(); it != c.regIntervals.end(); ++it) {
        output << it->first << ":" << it->second.first << "," << it->second.second << "\n";
    }
    output << "ALLOCATION:\n";
    for (auto it = regMap.begin(); it != regMap.end(); ++it) {
        output << it->first << ":" << it->second << "\n";
    }
    output.close();
}

int ProcessCommandArgs(int argc, const char* argv[])
{
	gLineNumber = 1;
	gSyntaxError = false;
	if (argc < 2)
	{
		std::cout << "You must pass the input file as a command line parameter." << std::endl;
		return 1;
	}

	// Read input from the first command line parameter
	proccin = fopen(argv[1], "r");
	if (proccin == nullptr)
	{
		std::cout << "File not found: " << argv[1] << std::endl;
		return 1;
	}
	procc_flush_buffer(nullptr);

	// Start the parse
	proccparse();

	if (gProgram != nullptr && argc == 3)
	{
		// TODO: Add any needed code for Parts 2, 3, and 4!!!
        if (strcmp(argv[2], "ast") == 0) {
            std::ofstream output;
            output.open("ast.txt");
            gProgram->OutputAST(output, 0);
            output.close();
        }
        else if (strcmp(argv[2], "emit") == 0) {
            CodeContext c;
            gProgram->CodeGen(c);
            outputAssembly_virtual(c);
        }
        else if (strcmp(argv[2], "reg") == 0) {
            CodeContext c;
            gProgram->CodeGen(c);
            std::map<std::string, std::string, regComp> regMap;
            registerAllocation(c, regMap);
            outputReg(c, regMap);
            outputAssembly_real(c, regMap);
        }
	}
	else
	{
		// (Just a useful separator for debug cout statements in grammar)
		std::cout << "**********************************************\n";
	}

	// Close the file stream
	fclose(proccin);
	// Destroy lexer so it reinitializes
	procclex_destroy();
	// Return 1 if syntax error, 0 otherwise
	return static_cast<int>(gSyntaxError);
}

void proccerror(const char* s) // NOLINT
{
	std::cout << s << " on line " << gLineNumber << std::endl;
	gSyntaxError = true;
}
