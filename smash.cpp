#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include "Commands.h"
#include "signals.h"
#include <fstream>

int main(int argc, char* argv[]) {
    
    if(signal(SIGINT , ctrlCHandler)==SIG_ERR) {
        perror("smash error: failed to set ctrl-C handler");
    }
    // if(signal(SIG_ALRM , alarmHandler)==SIG_ERR) {
    //     perror("smash error: failed to set alarm handler");
    // }

    //TODO: setup sig alarm handler
    // std::ifstream in("./tests/inputs/test_2020_jobs.txt"); // TO DELETE AFTER
    SmallShell& smash = SmallShell::getInstance();
    while(true) {
        std::cout << smash.get_prompt() + "> ";
        std::string cmd_line;
        std::getline(std::cin, cmd_line); // CHANGE LATER TO std::cin
        smash.executeCommand(cmd_line.c_str());
    }
    return 0;
}