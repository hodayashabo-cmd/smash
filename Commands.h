#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>

#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)

class Command {
// TODO: Add your data members

protected:
bool is_Stopped;
char *args[COMMAND_MAX_ARGS]; 
int pid; //TODO: need to check how to update this 
int num_of_args;


public:
 const char* cmd_line;

  Command(const char* cmd_line, int pid);
  virtual ~Command();
  virtual void execute() = 0;
  int get_pid();
  void set_pid(int new_pid);

};


class BuiltInCommand : public Command {
  
 public:
  BuiltInCommand(const char* cmd_line, int pid);
  virtual ~BuiltInCommand() {}
};

class ExternalCommand : public Command {
 public:
  bool is_BG;
  ExternalCommand(const char* cmd_line , int pid);
  virtual ~ExternalCommand() {}
  void execute() override;
};

class PipeCommand : public Command {
  // TODO: Add your data members
 public:
  PipeCommand(const char* cmd_line , int pid);
  virtual ~PipeCommand() {}
  void execute() override;
};

class RedirectionCommand : public Command {
 // TODO: Add your data members
 public:
 bool to_append;
  explicit RedirectionCommand(const char* cmd_line , int pid);
  virtual ~RedirectionCommand() {}
  void execute() override;
  //void prepare() override;
  //void cleanup() override;
};


class ChpromptCommand : public BuiltInCommand{
    public:
    ChpromptCommand(const char* cmd_line , int pid);
    virtual ~ChpromptCommand() {}
    void execute() override;

};

class ChangeDirCommand : public BuiltInCommand {
// TODO: Add your data members public:
  public:
  char* plastPwd;

  ChangeDirCommand(const char* cmd_line, char* plastPwd, int pid);
  virtual ~ChangeDirCommand() {}
  void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand {
 public:
  GetCurrDirCommand(const char* cmd_line, int pid);
  virtual ~GetCurrDirCommand() {}
  void execute() override;
};

class ShowPidCommand : public BuiltInCommand {
 public:
  ShowPidCommand(const char* cmd_line, int pid);
  virtual ~ShowPidCommand() {}
  void execute() override;
};

class JobsList;
class QuitCommand : public BuiltInCommand {
  public:
  JobsList* jobs;
// TODO: Add your data members public:
  QuitCommand(const char* cmd_line, JobsList* jobs, int pid);
  virtual ~QuitCommand() {}
  void execute() override;
};





class JobsList {
 public:
  class JobEntry {

   int job_id;
   int process_id; 
   
 
   
   public:
  JobEntry(int process_id, int job_id, Command* cmd, bool isStopped);
   int get_process_id();
   int get_job_id();
  Command* command;
  bool isStopped;
  };
 

 // TODO: Add your data members
 public:
  std::vector<JobsList::JobEntry>* jobs;\
  int max_job_id;

  JobsList();
  ~JobsList();
  void addJob(int pid, Command* cmd, bool isStopped = false); 
  void printJobsList();
  void killAllJobs();
  void removeFinishedJobs();
  JobEntry * getJobById(int jobId);
  void removeJobById(int jobId);
  JobEntry * getLastJob(int* lastJobId); 
  JobEntry *getLastStoppedJob(int *jobId);

  bool isJobExist(int jobId);
  void BackToFront(int jonId);
   int getJobListSize();
   void updateMax();
  JobEntry* getLastJob();


};

class JobsCommand : public BuiltInCommand {
 // TODO: Add your data members
 JobsList* jobs; 

 public:
  JobsCommand(const char* cmd_line, JobsList* jobs, int pid);
  virtual ~JobsCommand() {} 
  void execute() override;
};

class KillCommand : public BuiltInCommand {
  JobsList* jobs;
 // TODO: Add your data members
 public:
  KillCommand(const char* cmd_line, JobsList* jobs , int pid);
  virtual ~KillCommand() {}
  void execute() override;
};

class ForegroundCommand : public BuiltInCommand {
 // TODO: Add your data members
 JobsList* jobs;    

 public:
  ForegroundCommand(const char* cmd_line, JobsList* jobs , int pid);
  virtual ~ForegroundCommand() {}
  void execute() override;
};

class ChmodCommand : public BuiltInCommand {
 public:
  ChmodCommand(const char* cmd_line , int pid);
  virtual ~ChmodCommand() {}
  void execute() override;
};


class SmallShell {


  SmallShell();
protected:
std::string prompt;



 public:
 char* plastPwd;
 Command* fg_cmd; 
 int fg_pid; 
 JobsList* jobs_list;

  Command *CreateCommand(const char* cmd_line);
  SmallShell(SmallShell const&)      = delete; // disable copy ctor
  void operator=(SmallShell const&)  = delete; // disable = operator
  static SmallShell& getInstance() // make SmallShell singleton
  {
    static SmallShell instance; // Guaranteed to be destroyed.
    // Instantiated on first use.
    return instance;
  }
  ~SmallShell();
  void executeCommand(const char* cmd_line);
  // TODO: add extra methods as needed
  std::string get_prompt();
  void set_prompt(std::string new_prompt);
  
  //TODO: 
  char** getplastPwd();
  void setplastPwd(char * new_plastPwd);
  int getShellPid();
  JobsList* get_jobs_list();

};

#endif //SMASH_COMMAND_H_
