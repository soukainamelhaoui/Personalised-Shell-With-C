#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <dirent.h>
#include "tubesem.h"

 /* initialiser le semaphore S */ 
 void Initsem(Semaphore S, int N) 
{ 
  int i ; 
  char c='a'; 
  pipe(S); 
  for(i=1;i<=N;i++)
    write(S[1],&c,1);
 } 
/* P sur le semaphore S, prendre un jeton 'a' */ 
void P (Semaphore S)
{
  char c;
  read(S[0],&c,1);
} 
/* V sur le semaphore S, remettre un jeton 'a' */
void V (Semaphore S)
{
  char c='a';
  write(S[1],&c,1);
}
/* Attendre un nombre aleatoire de secondes entre 0 et N-1*/
void attente(int N)
{
  sleep(rand() % N);
}
/* Ecrire un message s dans la i eme colonne, la premiere colonne a le numero 1 */
void message(int i, char* s, int n)
{
  #define colonne 20 
  int Nb, j ;
  Nb=(i-1)*colonne;
  for(j=0; j<Nb; j++) 
    putchar(' ');
  printf("%s %d\n" ,s, n);
  fflush(stdout);
}


//clears terminal and prints welcome to my shell
void init_shell(){
  system("clear");
  execlp("figlet", "figlet", "-c", "-f", "Poison.flf", "Welcome To My Shell", NULL);
  sleep(5);
  //if execlp returns an error
  printf("\nerror execlp 1..");
  exit(0);
}

//menu command
void menu()
{
  puts("\n ***WELCOME TO MY SHELL MENU***"
        "\n"
        "\n> %             :  handle pipes"
        "\n> Mypwd         :  print current directory"
        "\n> Myls          :  list files and directories"
        "\n> Myalarm       :  set alarm"
        "\n> Mycalculator  :  calculator"
        "\n> Mylinux       :  execute linux built-in cmd"
        "\n> Myhistory     :  print cmd history"
        "\n> Myps          :  print running processes"
        "\n> Mykillpid     :  kill a process"
        "\n> Myexit        :  exit shell"
        "\n> Ctl-C         :  set alarm"
        "\n> Ctl-Z         :  exit shell"
        "\n");
  
  return;
}

//prompt function
void prompt(){
  char *prompt = "Soukaina@MyShell> ";
  printf("\033[0;35m"); //color purple
  printf("%s", prompt);
  printf("\033[0;37m"); //main color
}

//handle user input
char* handleInput(){
  char *line = NULL;
  size_t bufsize = 0;
  getline(&line, &bufsize, stdin);
  return line;
}

void displayHistory(int count, char* cmdHistory[]){
  for(int i=0; i<count; i++){
    printf("%s\n", cmdHistory[i]);
  }
}

//checking for pipes
int parsePipe(char* str){
  char str1[50];
  char str2[50];
  int result;
  result = sscanf(str, "%s %% %s", str1, str2);
  if(result == 2){
    return 1;
  }
  return 0;
}

//executing pipes
void execPipe(char* handle){
  //split the input into commands using the pipe character "%"
  char* cmd1 = strtok(handle, "%");
  char* cmd2 = strtok(NULL, "%");
  int i = 0;
  //0 for read, 1 for write
  int fd[2];
  pid_t p;
  if(pipe(fd) == -1){
    printf("\nError pipe");
    return;
  }
  p = fork();
  if(p == 0){ //child process 1
    close(fd[0]); //close the read
    dup2(fd[1], STDOUT_FILENO);
    close(fd[1]);//close the write
    execlp(cmd1, cmd1, NULL); // execute cmd 1
    printf("Could not execlp command 1..%s\n", cmd1);
    exit(0);
  }else{ // parent process
  wait(NULL);
    close(fd[1]); //close the write
    p = fork();
    if(p == 0){ //child process 2
      dup2(fd[0], STDIN_FILENO);
      close(fd[0]);//close the read
      execlp(cmd2, cmd2, NULL); // execute cmd 2
      printf("\nCould not execlp command 2..%s\n", cmd2);
      exit(0);
    }else{
      wait(0);
    }// parent waiting for two children processes
    }
    close(fd[0]);
    int status;
    waitpid(p, &status, 0);
}

void psFunction() {
  system("ps -a");
}

void killProcess() {
    int pid;
    printf("Enter pid to kill: \n");
    scanf("%d", &pid);
    kill(pid, SIGTERM);
}

//print current directory.
void printDirectory()
{
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    printf("Dir: %s\n", cwd);
}

//list function
void list(const char* path){
  DIR* dir;
  struct dirent* entry;
  printf("%s", path);
  if(fork() == 0){
    //open directory
    dir = opendir(path);
    if (dir == NULL){
      perror("error opendir.\n");
      exit(EXIT_FAILURE);
    }
    //read dir entries
    while((entry = readdir(dir)) != NULL){
      printf("%s\n", entry->d_name);
    }
    //close directory
    closedir(dir);
    exit(0);
  }
  wait(0);
  return;
}

//alarm function
void alarmFunction(){
  if (fork() == 0){
    execlp("./alarm", "./alarm", NULL);
    printf("Erreur d'execution de l'alarme.\n");
    exit(0);
  }
  wait(0);
  return;
}

void builtIn(){
  printf("Enter linux built in command\n");
  char *cmd = NULL;
  size_t bufsize = 0;
  getline(&cmd, &bufsize, stdin);
  system(cmd);
}

//HPC calculator
Semaphore sem, semCal;
void calculator(int num, int tabNumbers[]){
  //calculation time
  clock_t start, end;
  double execution_time;
  Initsem(semCal,1);
  P(semCal);
  int n1, n2, n3, n4;
  void operation(int, int, int*);
  printf("\n%-20s%-20s%-20s%-20s\n","OPERATION 1", "OPERATION 2"
  ,"OPERATION 3", "OPERATION 4");
  Initsem(sem,0);
  start = clock();
  // tubes
  int tubes[num][2];
  //shared memory to share result
  int Num =shmget(IPC_PRIVATE, sizeof(int), 0600 | IPC_CREAT);
  int* res = (int *) shmat(Num, NULL, 0);
  if(fork()==0){
    for (int i =0; i<num; i++){
      //creating pipes
      if(pipe(tubes[i]) == -1){perror("error pipe"); exit(1);}
      operation(i+1, tabNumbers[i], tubes[i]);
      P(sem);
      //reading from tube1
      close(tubes[i][1]);
      read(tubes[i][0], &tabNumbers[i], sizeof(int));
      *res += tabNumbers[i];
      fflush(stdout);
      close(tubes[i][0]);
    }
  }else{
  wait(0);
  end = clock();
  execution_time = ((double)(end - start))/(CLOCKS_PER_SEC/1000000);
  printf("Parallel calculator in %f nanoseconds: %d\n",execution_time, *res);
  fflush(stdout);
  V(semCal);}
}

//operation
void operation(int i, int n, int* tube)
{
  if(fork()==0) { 
    message(i,"2 * ", n);
    attente(2);
    n = n*2;
    close(tube[0]);
    write(tube[1], &n, sizeof(int));
    close(tube[1]);
    V(sem);
    exit(0); }
}

void simpleCalculator(int num, int tabNumbers[]){
  //calculation time
  clock_t start, end;
  double execution_time;
  start = clock();
  int res = 0;
  for (int i =0; i<num; i++){
      res += tabNumbers[i]*2;
    }
  end = clock();
  execution_time = ((double)(end - start))/(CLOCKS_PER_SEC/1000000);
  printf("Simple calculator in %f nanoseconds: %d\n",execution_time, res);
}

void signalHandler(int sig){
  if(sig == SIGINT){
    alarmFunction();
  }else{
    char input;
    printf(" Are you sure you want to exit? (y/n): \n");
    scanf("%c", &input);
    if(input == 'y' || input == 'Y'){
      printf("Exiting MyShell....\n");
      exit(0);
    }else{

    }
  }
}


//main program
int main(int ac, char **argv){
  const char *exits = "Myexit\n";
  const char *man = "Myman\n";
  const char *pwd = "Mypwd\n";
  const char *ls = "Myls\n";
  const char *alrm = "Myalarm\n";
  const char *cal = "Mycalculator\n";
  const char *lnx = "Mylinux\n";
  const char *history = "Myhistory\n";
  const char *ps = "Myps\n";
  const char *killpid = "Mykillpid\n";
  const char *empty = "\n";
  int count = 0;
  char* cmdHistory[256];
  //handling CTL+C and CTL+Z
  signal(SIGINT, signalHandler);
  signal(SIGTSTP, signalHandler);
  //forks to execute init_shell() without exiting the program
  pid_t childPid = fork();
  if (childPid == 0) {
    init_shell();
  }else{ //parent process
    wait(NULL);
  }
  printf("Enter Myman for manual\n");
  while (1){ //infinite loop to stay in MyShell
    prompt();
    char * handle = handleInput();
    //save cmd
    if(strcmp(handle, empty)!=0){
      count++;
      cmdHistory[count] = handle;
    }
    int isPipe = parsePipe(handle);
    if (strcmp(handle, exits)==0){
      printf("Exiting MyShell....\n");
      return -1;
    } else if(strcmp(handle, man)==0){
      menu();
    }else if(strcmp(handle, pwd)==0){
      printDirectory();
    }else if(strcmp(handle, alrm)==0){
      alarmFunction();
    }else if(strcmp(handle, ls)==0){
      char dir[256];
      printf("Enter directory path (use . for current directory): \n");
      scanf("%s", dir);
      list(dir);
    }else if(strcmp(handle, cal)==0){
      Initsem(semCal, 1);
      int i = 0;
      int num;
      int tabNumbers[4] = {0}; //stores user input
      printf("Enter number of operations (2-4): \n");
      scanf("%d", &num);
      while(i < num){
        printf("Enter value %d: \n", i+1);
        scanf("%d", &tabNumbers[i]);
        i++;
      }
      if (fork() == 0){
        calculator(num, tabNumbers);
        exit(0);
      }
      wait(0);
      simpleCalculator(num, tabNumbers);
    }else if(isPipe == 1){
      //handling pipes
      execPipe(handle);
    }else if(strcmp(handle, lnx)==0){
      //linux commands
      builtIn();
    }else if(strcmp(handle, history)==0){
      //history
      displayHistory(count, cmdHistory);
    }else if(strcmp(handle, ps)==0){
      psFunction();
    }else if(strcmp(handle, killpid)==0){
      killProcess();
    }else if(strcmp(handle, empty)==0){
      continue;
    }else{
      printf("err: command not found.\n");
    }
  }

}
