#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;
const string SHELL_PROMPT = "smash";
const std::string WHITESPACE = " \n\r\t\f\v";
#define STDIN 0
#define STDOUT 1
#define STDERR 2

#if 0
#define FUNC_ENTRY()  \
  cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

string _ltrim(const std::string& s)
{
  size_t start = s.find_first_not_of(WHITESPACE);
  return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string& s)
{
  size_t end = s.find_last_not_of(WHITESPACE);
  return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string& s)
{
  return _rtrim(_ltrim(s));
}

int _parseCommandLine(const char* cmd_line, char** args) {
  FUNC_ENTRY()
  int i = 0;
  std::istringstream iss(_trim(string(cmd_line)).c_str());
  for(std::string s; iss >> s; ) {
    args[i] = (char*)malloc(s.length()+1);
    memset(args[i], 0, s.length()+1);
    strcpy(args[i], s.c_str());
    args[++i] = NULL;
  }
  return i;

  FUNC_EXIT()
}

bool _isBackgroundCommand(const char* cmd_line) {
  const string str(cmd_line);
  return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char* cmd_line) {
  const string str(cmd_line);
  // find last character other than spaces
  unsigned int idx = str.find_last_not_of(WHITESPACE);
  // if all characters are spaces then return
  if (idx == string::npos) {
    return;
  }
  // if the command line does not end with & then return
  if (cmd_line[idx] != '&') {
    return;
  }
  // replace the & (background sign) with space and then remove all tailing spaces.
  cmd_line[idx] =' ';
  // truncate the command line string up to the last non-space character
  cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}

bool is_digits(const std::string &str){
    return str.find_first_not_of("-0123456789") == std::string::npos;
}


// TODO: Add your implementation for classes in Commands.h 




SmallShell::SmallShell() : prompt("smash"),plastPwd(nullptr) {

    fg_cmd = nullptr;
  jobs_list = new JobsList();

}

SmallShell::~SmallShell() {

}

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command * SmallShell::CreateCommand(const char* cmd_line ) {

    JobsList* jobs = this->jobs_list;
    string cmd_s = _trim(string(cmd_line));
    string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));

    if(firstWord.compare("") == 0) {
        return nullptr;
    }
    else if(cmd_s.find(">") != std::string::npos) {
        return new RedirectionCommand(cmd_line, -1);
    }
    else if (firstWord.compare("pwd") == 0 || firstWord.compare("pwd&") == 0) {
        return new GetCurrDirCommand(cmd_line, -1);
    }
    else if (firstWord.compare("chprompt") == 0 || firstWord.compare("chprompt&") == 0) {
        return new ChpromptCommand(cmd_line, -1);
    }
    else if (firstWord.compare("showpid") == 0 || firstWord.compare("showpid&") == 0) {
        return new ShowPidCommand(cmd_line, -1);
    }
    else if(firstWord.compare("quit") == 0 || firstWord.compare("quit&") == 0) {
        return new QuitCommand(cmd_line, jobs, -1);
    }
    else if (firstWord.compare("cd") == 0 || firstWord.compare("cd&") == 0) {
        return new ChangeDirCommand(cmd_line, plastPwd, -1);
    }
    else if (firstWord.compare("jobs") == 0 || firstWord.compare("jobs&") == 0) {
        return new JobsCommand(cmd_line, jobs, -1);
    }
    else if(firstWord.compare("kill") == 0 || firstWord.compare("&kill") == 0) {
        return new KillCommand(cmd_line, jobs, -1);
    }
    else if(firstWord.compare("fg") == 0 || firstWord.compare("fg&") == 0) {
        return new ForegroundCommand(cmd_line, jobs, -1);
    }
    else if(firstWord.compare("chmod") == 0 || firstWord.compare("chmod&") == 0) {
        return new ChmodCommand(cmd_line, -1);
    }

  
    else {
        return new ExternalCommand(cmd_line, -1);
    }
    return nullptr;

}

void SmallShell::executeCommand(const char *cmd_line) {
   Command* cmd = CreateCommand(cmd_line);
   if(cmd == nullptr){
    return;
   }
   cmd->execute();
  
}


//Command




Command::Command(const char* cmd_line, int pid) :  is_Stopped(false) , pid(pid){
   this->cmd_line = strdup(cmd_line); // Make a copy of the command line
   num_of_args = _parseCommandLine(cmd_line, args);
}

Command::~Command(){
    free(const_cast<char*>(cmd_line)); // Free the memory allocated for the command line
}


int Command::get_pid(){
    return pid;
}

void Command::set_pid(int new_pid){
    this->pid = new_pid;
}




//Built-in command 
BuiltInCommand::BuiltInCommand(const char* cmd_line1 , int pid) : Command(cmd_line1, pid){
 
 if( _isBackgroundCommand(cmd_line1)){
    char new_cmd_line[strlen(cmd_line1)];
    strcpy(new_cmd_line, cmd_line1);
    _removeBackgroundSign(new_cmd_line);
    num_of_args=_parseCommandLine(new_cmd_line, args);
 }


}


///////////////////chmod Command////////////////////////
ChmodCommand::ChmodCommand(const char *cmd_line, int pid) : BuiltInCommand(cmd_line, pid){}

void ChmodCommand::execute()
{
    if(num_of_args != 3)
    {
        std::cerr << "smash error: chmod: invalid arguments" << endl;
        return;
    }
     std::stoi(args[1]);
    mode_t mode =  std::stoi(args[1], 0 , 8); 
    const char* file_path = args[2];
    if(chmod(file_path,mode) == -1)
    {
        perror("smash error: chmod failed");
        return;
    }

}


ChpromptCommand::ChpromptCommand(const char* cmd_line, int pid) : BuiltInCommand(cmd_line , pid){}

void ChpromptCommand::execute(){
  SmallShell& smash = SmallShell::getInstance();
  if(num_of_args > 1 ){
    smash.set_prompt(args[1]);
  }else{
    smash.set_prompt(SHELL_PROMPT);
  }
}


  ShowPidCommand::ShowPidCommand(const char* cmd_line, int pid): BuiltInCommand(cmd_line , pid){ }

  void ShowPidCommand::execute(){
    //CHECK if needed endl
    std::cout << "smash pid is ";
    std::cout << getpid() << std::endl;
    
  }

  GetCurrDirCommand::GetCurrDirCommand(const char* cmd_line , int pid): BuiltInCommand(cmd_line, pid){ }

   void GetCurrDirCommand::execute(){
    string pwd = getcwd(NULL, 0);
    std::cout << pwd << std::endl;
  }



ChangeDirCommand::ChangeDirCommand(const char* cmd_line, char* plastPwd, int pid ): BuiltInCommand(cmd_line , pid), plastPwd(plastPwd){} 



  void ChangeDirCommand::execute(){

  //TODO : cd..
   SmallShell& smash = SmallShell::getInstance();
   if(num_of_args > 2){
    std::cerr <<"smash error: cd: too many arguments" << std::endl;
    return;
   }

   char *curr_pwd = getcwd(NULL,0);

   if(strcmp(args[1],"-") == 0 ){
      if( plastPwd == nullptr){
        std::cerr<<"smash error: cd: OLDPWD not set"<<std::endl;
        return;
      }else{
        if(chdir(plastPwd) == -1)
        {
          perror("smash error: chdir failed");
          return;
        }
        else{
             smash.setplastPwd(curr_pwd);  
        }


      }
   }else{
    if(chdir(args[1]) == -1){
      perror("smash error: chdir failed");
      return;
    }else{
      smash.plastPwd = curr_pwd; 
    }
   }

    return;
  }


JobsCommand::JobsCommand(const char* cmd_line, JobsList* jobs1 , int pid) :BuiltInCommand(cmd_line , pid), jobs(jobs1) {}

void JobsCommand::execute(){
  if(jobs->jobs->size() == 0){
    return;
  }else{
    jobs->removeFinishedJobs();
    jobs->printJobsList();
  }
  
  return;

}

ForegroundCommand::ForegroundCommand(const char* cmd_line, JobsList* jobs, int pid): BuiltInCommand(cmd_line , pid) , jobs(jobs){}

//TODO: resume the function
void ForegroundCommand::execute(){
    
    int job_id;

    SmallShell& smash = SmallShell::getInstance();
    if(num_of_args > 2 || (num_of_args > 1 && !(is_digits(string(args[1]))))){
        cerr << "smash error: fg: invalid arguments" << endl;
        return;
    }
    smash.jobs_list->removeFinishedJobs();

    if(num_of_args == 2){
        job_id = atoi(args[1]);
        if(smash.jobs_list->getJobById(job_id) == nullptr){
            cerr << "smash error: fg: job-id " << job_id << " does not exist" << endl;
            return;
        }
    }else{
      if(smash.jobs_list->jobs->size() == 0){
       cerr <<"smash error: fg: jobs list is empty"<< endl;
       return;
      }

      job_id = smash.jobs_list->getLastJob()->get_job_id();

    }
        JobsList::JobEntry* job1 = smash.jobs_list->getJobById(job_id);
        cout << job1->command->cmd_line << " : " << job1->get_process_id() << endl;
        smash.jobs_list->removeJobById(job_id);
        smash.fg_cmd = job1->command; //Added by Maayan
        waitpid(job1->get_process_id(), NULL, WUNTRACED);
        delete smash.fg_cmd;
        smash.fg_cmd = nullptr;
        
}


///////////////////Quit command////////////////////////

QuitCommand::QuitCommand(const char* cmd_line, JobsList* jobs , int pid ): BuiltInCommand(cmd_line, pid) , jobs(jobs){}

void QuitCommand::execute()
{
    SmallShell& smash = SmallShell::getInstance();
    if(num_of_args > 1 && strcmp(args[1], "kill") == 0)
    {
        smash.jobs_list->removeFinishedJobs();
        int jobs_list_size = smash.jobs_list->getJobListSize();
        std::cout << "smash: sending SIGKILL signal to " << jobs_list_size << " jobs:" << std::endl;
        smash.jobs_list->killAllJobs();
    }

    exit(0);

}



///////////////////Kill command////////////////////////

KillCommand::KillCommand(const char* cmd_line, JobsList* jobs, int pid ) : BuiltInCommand(cmd_line , pid), jobs(jobs) {}

void KillCommand::execute()
{
    SmallShell& smash = SmallShell::getInstance();
    if(num_of_args != 3)
    {
        std::cerr << "smash error: kill: invalid arguments" << endl;
        return;
    }

    //checking if the signum is valid
    int sig_num = atoi(args[1]); //atoi convert a string containing an integer representation in ASCII
    if(sig_num >= 0)
    {
        std::cerr << "smash error: kill: invalid arguments" << endl;
        return;
    }

    //checking if the job exist
    int job_id = atoi(args[2]);
    JobsList::JobEntry* job;
    job = smash.jobs_list->getJobById(job_id);
    if(job == nullptr)
    {
        cerr << "smash error: kill: job-id " << job_id << " does not exist" << endl;
        return;
    }

    sig_num = std::abs(sig_num);
    if (kill(job->get_process_id(), sig_num) == -1 )
    {
        perror("smash error: kill failed");
        return;
    }

    cout << "signal number " << sig_num << " was sent to pid " << job_id << endl;
    if(sig_num == 9)
    {
        smash.jobs_list->removeJobById(job_id);
    }


}






//Jobs List 


JobsList::JobEntry::JobEntry(int process_id, int job_id, Command* cmd, bool isStopped) : job_id(job_id), process_id(process_id), command(cmd), isStopped(isStopped) {}



JobsList::JobsList(): max_job_id(0){
  this->jobs = new vector<JobsList::JobEntry>;

}

JobsList::~JobsList(){
    delete(this->jobs);
}

void JobsList::addJob(int pid, Command* cmd, bool isStopped ){
  this->removeFinishedJobs();

    updateMax();
  int new_job_id = max_job_id+1;

  cmd->set_pid(pid);
    
  JobEntry new_job = JobEntry(pid, new_job_id, cmd, isStopped);

    if(jobs->size() == 0){
        jobs->push_back(new_job);

    }else{
        vector<JobsList::JobEntry>::iterator iter;
        iter = jobs->begin();
        while(iter->get_job_id() < new_job_id && iter != jobs->end()){
            iter++;
        }
        jobs->insert(iter, new_job);
    }

    updateMax();

}

void JobsList::updateMax() {
    vector<JobsList::JobEntry>::iterator iter;
    iter = jobs->begin();
    while(iter != jobs->end()){
        max_job_id = iter->get_job_id();
        iter++;
    }
}






  void JobsList::printJobsList(){

    vector<JobsList::JobEntry>::iterator iter;
    this->removeFinishedJobs();

    for(iter = jobs->begin();iter < jobs->end() ;iter++){
        cout << "[" << iter->get_job_id() << "] " << iter->command->cmd_line <<endl;
    }
  }

   void JobsList::killAllJobs(){

    this->removeFinishedJobs();
 
     for(auto& job : *(this->jobs))
    {
        cout << job.get_process_id() << ": " << job.command->cmd_line <<endl;

        if (kill(job.get_process_id(), SIGKILL))
        {
            perror("smash error: kill failed");
            return;
        }
    }
 }

  void JobsList::removeFinishedJobs(){


    if (jobs->size() == 0) {
        return;
    }
    vector<JobsList::JobEntry>::iterator iter = jobs->begin();
    for (; iter <= jobs->end();) {
        //we will check if wait gives us jobs that are done and not stopped
        if(iter==jobs->end()){
            vector<JobsList::JobEntry>::iterator iter;
             iter = jobs->begin();
            while(iter != jobs->end()){
               max_job_id = iter->get_job_id();
               iter++;
             }
            return;
        }
        int status;
        pid_t return_pid = waitpid(iter->get_process_id(), &status, WNOHANG);
        if (return_pid > 0 || return_pid == -1) {
            jobs->erase(iter);
            continue; //erase increases the iterator
        }
        iter++;
    }

    return;

  }



JobsList::JobEntry* JobsList::getJobById(int JobId){
    this->removeFinishedJobs();

    for (auto job = jobs->begin(); job != jobs->end();)
    {
        if(job->get_job_id() == JobId){
            return &(*job);
        }
        ++job;
    }
    return nullptr;
}


void JobsList::removeJobById(int JobId){
    for (auto job = jobs->begin(); job != jobs->end();)
    {
        if(job->get_job_id() == JobId){
            this->jobs->erase(job);
            return;
        }
        else{
            ++job;
        }
    }
}

JobsList::JobEntry* JobsList::getLastJob(int* lastJobId){
    if(jobs->size() == 0){
        return nullptr;
    }
    else{
        this->removeFinishedJobs();
        JobEntry* last_job = this->getJobById(max_job_id);
        return last_job;
    }
}

JobsList::JobEntry* JobsList::getLastStoppedJob(int *jobId){
    JobEntry* max_stopped_ptr = nullptr;
    for (auto job = jobs->begin(); job != jobs->end();)
    {
        if(job->isStopped){
            max_stopped_ptr = &(*job);
        }
        ++job;
    }
    if(max_stopped_ptr != nullptr && jobId != nullptr){
        *jobId = max_stopped_ptr->get_job_id();
    }
    return max_stopped_ptr;
}

JobsList::JobEntry* JobsList::getLastJob() {

    removeFinishedJobs();
    JobEntry* last_job = this->getJobById(max_job_id);
    return last_job;
}


int JobsList::getJobListSize()
{
    int size = jobs->size();
    return size;
}

int JobsList::JobEntry::get_process_id() {
    return process_id;
}

int JobsList::JobEntry::get_job_id() {
    return job_id;
}





//Small Shell 

void SmallShell::set_prompt(std::string new_prompt){
  prompt = new_prompt;
}

 std::string SmallShell::get_prompt(){
  return prompt;
 }

void SmallShell::setplastPwd(char * new_plastPwd){
    strcpy(plastPwd, new_plastPwd);
    
}

//////////////////External Command////////////////////////

bool is_complex(const char* cmd_line)
{
    for(unsigned int i = 0; i < strlen(cmd_line); i++){
        if(cmd_line[i] == '*' || cmd_line[i] == '?'){
            return true;
        }
    }
    return false;
}

ExternalCommand::ExternalCommand(const char* cmd_line , int pid ) : Command(cmd_line , pid)
{
      char new_cmd_line[strlen(cmd_line)];
       strcpy(new_cmd_line, cmd_line);
       if(cmd_line){
       is_BG = _isBackgroundCommand(new_cmd_line);
       }
}


void ExternalCommand :: execute()
{
    if(!cmd_line){
        return;
    }
    SmallShell& smash = SmallShell::getInstance();
    char new_cmd_line[strlen(cmd_line) + 1];
    strcpy(new_cmd_line,cmd_line);
    _removeBackgroundSign(new_cmd_line);
    char* new_args[COMMAND_MAX_ARGS];
    _parseCommandLine(new_cmd_line, new_args);


    if(!is_BG){
        smash.fg_cmd = this;
    }


    pid_t pid = fork();
    if(pid == -1)
    {
        perror("smash error: fork failed");
        return;
    }
    if(pid == 0) //the son. execute the given command in a forked child
    {
        if (setpgrp() == -1)
        {
            perror("smash error: setpgrp failed");
            exit(1);
        }
        if(!(is_complex(new_cmd_line))) //simple external command
        {
            //use execvp to execute the command
            if (execvp(new_args[0], new_args) == -1)
            {
                perror("smash error: execvp failed");
                exit(1);
            }
        }else if(execl("/bin/bash", "/bin/bash", "-c", new_cmd_line, nullptr) == -1){ //complex external command. use execl to execute the command
            perror("smash error: execl failed");
            exit(1);
        }
    }
    else{ //the parent.
        if(!is_BG)
        {
            this->pid = pid;
            if(waitpid(pid, nullptr, WUNTRACED) == -1)
            {
                perror("smash error: waitpid failed");
                return;
            }
            smash.fg_cmd = nullptr;
        }else{ //command in background. need to add to jobs list
            Command* new_command = smash.CreateCommand(cmd_line);
            smash.jobs_list->addJob(pid, new_command,false); //check this
        }
    }
    return;
}




///////////////////Redirection command////////////////////////

RedirectionCommand::RedirectionCommand(const char *cmd_line, int pid) : Command(cmd_line,pid) {
    to_append = false;
}

void RedirectionCommand::execute()
{
    SmallShell& smash = SmallShell::getInstance();

    char* file = args[num_of_args - 1];
    std::string file_path(file);
    std::string command_name;
    //create a string. to work with string functions
    std::string command(cmd_line);
    if(command.find(">>") != std::string::npos){
            to_append = true;
    }
    
    int fd_stdout = dup(STDOUT); 
    if(fd_stdout == -1)
    {
        perror("smash error: dup failed");
        return;
    }
    if(close(STDOUT) == -1)
    {
        perror("smash error: close failed");
        return;
    }
    int open_fd;
    if(!to_append)
    {
        //find the position of the '<' symbol
        std::size_t pos = command.find(">");
        //extract the substring before '<' (excluding '<' itself)
         command_name = command.substr(0, pos);

        open_fd = open(file_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
    }
    else{ //we need to append
        //find the position of the '<<' symbol
        std::size_t pos = command.find(">>");
        //extract the substring before '<<' (excluding '<<' itself)
         command_name = command.substr(0, pos);

        open_fd = open(file_path.c_str(), O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
    }

    if(open_fd == -1)
    {
        perror("smash error: open failed");
        if(dup2(fd_stdout, 1) == -1)
        {
            perror("smash error: dup2 failed");
            return;
        }
        if(close(fd_stdout) == -1) {
            perror("smash error: close failed");
            return;
        }
        return;
    } 

    smash.executeCommand(command_name.c_str());
    if(dup2(fd_stdout, 1) == -1)
    {
        perror("smash error: dup2 failed");
        return;
    }
    if(close(fd_stdout) == -1)
    {
        perror("smash error: close failed");
        return;
    }


}
