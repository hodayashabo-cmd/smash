#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;

// void ctrlCHandler(int sig_num) {
//   // TODO: Add your implementation
//   exit(0);
// }

void ctrlCHandler(int sig_num) {
  cout << "smash: got ctrl-C" << endl;
  
  SmallShell& smash = SmallShell::getInstance();
  if(smash.fg_cmd == nullptr ){
    return;
  }else if(smash.fg_cmd->get_pid() == -1 ){
    return;
  }  else{
    if (kill(smash.fg_cmd->get_pid(), SIGKILL) == -1 )
      {
        perror("smash error: kill failed");
        return;
      }
      cout << "smash: process " << smash.fg_cmd->get_pid() << " was killed" << endl;
      smash.fg_cmd = nullptr;
  }
}
  




