#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/types.h> //pid_t
#include <sys/wait.h> //waitpid
#include <sys/ipc.h>
#include <sys/shm.h>
#include <limits.h>
#include <assert.h>
#include <stdbool.h>


int *NE;      //počet skřítků  0<NE<1000•
int *NR;      //počet sobů. 0<NR< 20•
int *TE;      //Maximální doba v milisekundách, po kterou skřítek pracuje samostatně. 0<=TE<=1000.•
int *TR;      //Maximální doba v milisekundách, po které se sob vrací z dovolené domů. 0<=TR<=1000.
int *elfID;
int *rdID;
FILE *output = NULL;

int *citac;
int *elfqueuecounter;
bool *emptyworkshop;
bool *closed;
bool *closedworkshop;
int *helpedelves;
int *retdeerscount;
int *hitcheddeerscounter;
int *elfleavecounter;

sem_t *s_print;    //only one process at a time can increment counter and write to output file
sem_t *s_elfidinc;
sem_t *s_rdidinc;
sem_t *s_santasleep;
sem_t *s_elfqueue;
sem_t *s_elfqueuecounter;
sem_t *s_elfhelpcounter;
sem_t *s_santahelping;
sem_t *s_retdeerscount;
sem_t *s_deerwait;
sem_t *s_santadoigdeers;
sem_t *s_hitcheddeerscounter;
sem_t *s_santastartedhelping;
sem_t *s_elfleavecounter;
sem_t *s_emptyworkshopswitch;
sem_t *s_freeelves;
//open file for output, allocate memories, initialize semaphores and set starting variable values
void init(){
  int errorcode = 0;
  if ((output = fopen("proj2.out","w")) == NULL){
        fprintf(stderr,"Error while trying to open/create proj2.out file\n");
        exit(1);
    }
  setlinebuf(output); //make file write with every new line (\n) basically with every fprintf in my case


        if ((s_print= mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED) errorcode = 1;
        if ((s_elfidinc= mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED) errorcode = 1;
        if ((s_rdidinc= mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED) errorcode = 1;
        if ((s_santasleep= mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED) errorcode = 1;
        if ((s_elfqueue= mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED) errorcode = 1;
        if ((s_elfqueuecounter= mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED) errorcode = 1;
        if ((s_elfhelpcounter= mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED) errorcode = 1;
        if ((s_santahelping= mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED) errorcode = 1;
        if ((s_retdeerscount= mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED) errorcode = 1;
        if ((s_deerwait= mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED) errorcode = 1;
        if ((s_santadoigdeers= mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED) errorcode = 1;
        if ((s_hitcheddeerscounter= mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED) errorcode = 1;
        if ((s_santastartedhelping= mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED) errorcode = 1;
        if ((s_emptyworkshopswitch= mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED) errorcode = 1;
        if ((s_freeelves= mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED) errorcode = 1;

        if ((s_elfleavecounter= mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED) errorcode = 1;

        if (errorcode != 0){
          fprintf(stderr,"Error: Failed allocate memory!\n");
          exit(1);
        }

        if (sem_init(s_print, 1, 1) == -1) errorcode = 1;
        if (sem_init(s_elfidinc, 1, 1) == -1) errorcode = 1;
        if (sem_init(s_rdidinc, 1, 1) == -1) errorcode = 1;
        if (sem_init(s_santasleep, 1, 0) == -1) errorcode = 1;
        if (sem_init(s_elfqueue, 1, 0) == -1) errorcode = 1;
        if (sem_init(s_elfqueuecounter, 1, 1) == -1) errorcode = 1;
        if (sem_init(s_elfhelpcounter, 1, 1) == -1) errorcode = 1;
        if (sem_init(s_santahelping, 1, 0) == -1) errorcode = 1;
        if (sem_init(s_retdeerscount, 1, 1) == -1) errorcode = 1;
        if (sem_init(s_deerwait, 1, 0) == -1) errorcode = 1;
        if (sem_init(s_santadoigdeers, 1, 0) == -1) errorcode = 1;
        if (sem_init(s_hitcheddeerscounter, 1, 1) == -1) errorcode = 1;
        if (sem_init(s_santastartedhelping, 1, 0) == -1) errorcode = 1;
        if (sem_init(s_emptyworkshopswitch, 1, 1) == -1) errorcode = 1;
        if (sem_init(s_freeelves, 1, 0) == -1) errorcode = 1;
        if (sem_init(s_elfleavecounter, 1, 1) == -1) errorcode = 1;


        if (errorcode != 0){
          fprintf(stderr,"Error: Failed initialize semaphors!\n");
          exit(1);
        }
        if ((NE= mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED) errorcode = 1;
        if ((NR= mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED) errorcode = 1;
        if ((TE= mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED) errorcode = 1;
        if ((TR= mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED) errorcode = 1;
        if ((citac= mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED) errorcode = 1;
        if ((elfID= mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED) errorcode = 1;
        if ((rdID= mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED) errorcode = 1;
        if ((elfqueuecounter= mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED) errorcode = 1;
        if ((emptyworkshop= mmap(NULL, sizeof(bool), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED) errorcode = 1;
        if ((closed= mmap(NULL, sizeof(bool), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED) errorcode = 1;
        if ((helpedelves= mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED) errorcode = 1;
        if ((retdeerscount= mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED) errorcode = 1;
        if ((hitcheddeerscounter= mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED) errorcode = 1;
        if ((closedworkshop= mmap(NULL, sizeof(bool), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED) errorcode = 1;
        if ((elfleavecounter= mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED) errorcode = 1;
        if (errorcode != 0){
          fprintf(stderr,"Error: Failed allocate memory!\n");
          exit(1);
        }
        *elfID = 1;
        *citac = 1;
        *rdID = 1;
        *elfqueuecounter = 0;
        *closed = false;
        *emptyworkshop = true;
        *helpedelves = 0;
        *retdeerscount = 0;
        *hitcheddeerscounter=0;
        *closedworkshop = false;
        *elfleavecounter=0;


    }
//close file, unallocate memomries, destroy semaphores
void clean(){
    fclose(output);
    int errorcode = 0;

    if (sem_destroy(s_print) == -1) errorcode = 1;
    if (sem_destroy(s_elfidinc) == -1) errorcode = 1;
    if (sem_destroy(s_rdidinc) == -1) errorcode = 1;
    if (sem_destroy(s_santasleep) == -1) errorcode = 1;
    if (sem_destroy(s_elfqueue) == -1) errorcode = 1;
    if (sem_destroy(s_elfqueuecounter) == -1) errorcode = 1;
    if (sem_destroy(s_elfhelpcounter) == -1) errorcode = 1;
    if (sem_destroy(s_santahelping) == -1) errorcode = 1;
    if (sem_destroy(s_retdeerscount) == -1) errorcode = 1;
    if (sem_destroy(s_deerwait) == -1) errorcode = 1;
    if (sem_destroy(s_santadoigdeers) == -1) errorcode = 1;
    if (sem_destroy(s_hitcheddeerscounter) == -1) errorcode = 1;
    if (sem_destroy(s_santastartedhelping) == -1) errorcode = 1;
    if (sem_destroy(s_emptyworkshopswitch) == -1) errorcode = 1;
    if (sem_destroy(s_freeelves) == -1) errorcode = 1;
    if (sem_destroy(s_elfleavecounter) == -1) errorcode = 1;

    if ( errorcode == 1 ){
      fprintf(stderr, "An error occured while destroying semaphores!\n");
      exit(1);
    }
    munmap(s_print,sizeof(sem_t));
    munmap(s_elfidinc,sizeof(sem_t));
    munmap(s_rdidinc,sizeof(sem_t));
    munmap(s_santasleep,sizeof(sem_t));
    munmap(s_elfqueue,sizeof(sem_t));
    munmap(s_elfqueuecounter,sizeof(sem_t));
    munmap(s_elfhelpcounter,sizeof(sem_t));
    munmap(s_santahelping,sizeof(sem_t));
    munmap(s_retdeerscount,sizeof(sem_t));
    munmap(s_deerwait,sizeof(sem_t));
    munmap(s_santadoigdeers,sizeof(sem_t));
    munmap(s_hitcheddeerscounter,sizeof(sem_t));
    munmap(s_santastartedhelping,sizeof(sem_t));
    munmap(s_emptyworkshopswitch,sizeof(sem_t));
    munmap(s_elfleavecounter,sizeof(sem_t));
    munmap(s_freeelves,sizeof(sem_t));

    munmap(NE,sizeof(int));
    munmap(NR,sizeof(int));
    munmap(TE,sizeof(int));
    munmap(TR,sizeof(int));
    munmap(citac,sizeof(int));
    munmap(elfID,sizeof(int));
    munmap(rdID,sizeof(int));
    munmap(elfqueuecounter,sizeof(int));
    munmap(emptyworkshop,sizeof(bool));
    munmap(closed,sizeof(bool));
    munmap(helpedelves,sizeof(int));
    munmap(retdeerscount,sizeof(int));
    munmap(hitcheddeerscounter,sizeof(int));
    munmap(elfleavecounter,sizeof(int));
    munmap(closedworkshop,sizeof(bool));

}

//my function for file writing, is=1=elf is=2=deer is=0=Santa
void print(char* x, int is, int nID){
  sem_wait(s_print);
  if(is == 1){
    fprintf(output,"%d: Elf %d: %s\n",*citac,nID, x);
    *citac = *citac +1;
  }else if(is == 2){
    fprintf(output,"%d: RD %d: %s\n",*citac,nID, x);
    *citac = *citac +1;
  }else{
    fprintf(output,"%d: %s\n",*citac, x);
    *citac = *citac+1;
  }
  fflush(output);
  sem_post(s_print);

}


void reindeer(){   //function for reindeers
  srand(getpid());
  sem_wait(s_rdidinc);
  int myRdID = *rdID;
  *rdID = *rdID +1;
  sem_post(s_rdidinc);
  print("rstarted", 2, myRdID);
  if (*TR != 0) {
    usleep(((rand() % (*TR+1 - *TR/2)) + *TR/2)* 1000);
  }

  print("return home", 2, myRdID);
  sem_wait(s_retdeerscount);
  *retdeerscount = *retdeerscount +1;
  if(*retdeerscount == *NR){
    *closed = true;
    sem_post(s_santasleep);
  }
  sem_post(s_retdeerscount);

  sem_wait(s_deerwait);  //here are waiting all deers till santa comes for them
  sem_wait(s_hitcheddeerscounter);
  *hitcheddeerscounter = *hitcheddeerscounter +1;
  print("get hitched", 2, myRdID);
  if(*hitcheddeerscounter == *NR){
    sem_post(s_santadoigdeers);
  }
  sem_post(s_hitcheddeerscounter);
  exit(0);
}


void elf(){         //function for elfs (almost gave up there)
  srand(getpid());
  sem_wait(s_elfidinc);
  int myelfID = *elfID;
  *elfID = *elfID +1;
  sem_post(s_elfidinc);
  print("started", 1, myelfID);
  bool gothelpedthisloop = false;   //if elf got helped last loop he haas to loop agan to at least ask for help even if workshop is closed
  do{
  gothelpedthisloop = false;
  if (*TE != 0) {                       //set sleep
    usleep((rand() % (*TE)+1) * 1000);
  }


  print("need help", 1, myelfID);

  if(*closedworkshop == false){

    sem_wait(s_elfqueuecounter);
    *elfqueuecounter = *elfqueuecounter +1;

    if(*elfqueuecounter == 3 && *emptyworkshop == true && *closedworkshop == false){
      *elfqueuecounter = *elfqueuecounter -3;
      sem_wait(s_emptyworkshopswitch);
      *emptyworkshop = false;
      sem_post(s_emptyworkshopswitch);
      sem_post(s_santasleep);  //if there are at least 3 elves waiting, let them in santas workshop
      sem_post(s_elfqueue);
      sem_post(s_elfqueue);
      sem_post(s_elfqueue);
    }
    sem_post(s_elfqueuecounter);
    sem_wait(s_elfqueue);
    sem_wait(s_santastartedhelping);

    if(*closedworkshop == false){

      print("get help", 1, myelfID);
      gothelpedthisloop=true;
      sem_wait(s_elfleavecounter);
      *elfleavecounter = *elfleavecounter +1;
      if(*elfleavecounter >= 3){
        *elfleavecounter = *elfleavecounter -3;
        sem_wait(s_emptyworkshopswitch);
        *emptyworkshop = true;
        sem_post(s_emptyworkshopswitch);
        sem_post(s_santahelping);
        if(*elfqueuecounter >= 3 && *closed==false){
          sem_wait(s_elfqueuecounter);
          *elfqueuecounter = *elfqueuecounter -3;
          sem_wait(s_emptyworkshopswitch);
          *emptyworkshop = false;
          sem_post(s_emptyworkshopswitch);
          sem_post(s_elfqueuecounter);
          sem_post(s_santasleep);  //if there are at least 3 elves waiting, let them in santas workshop
          sem_post(s_elfqueue);
          sem_post(s_elfqueue);
          sem_post(s_elfqueue);
        }
      }
      sem_post(s_elfleavecounter);

    }

  }

}while(*closedworkshop == false || gothelpedthisloop == true);
  sem_wait(s_freeelves);
  print("taking holidays", 1, myelfID);

  exit(0);
}
//function for santa, santa goes to sleep and is helping elves in loop till all sobs are home, also if he helped elves this loop he has to go another one to at least write going to sleep status,
//after he passes the loop he will close workshop and free all waiting alves and deers and after he does all the deers he prints christmas strted and ends himself
void Santa(){
bool helped;
  do{
  helped=false;
  print("Santa: going to sleep", 0, 0);
  if(*closed == false)
  {
    sem_wait(s_santasleep);
  }
  if(*closed == false){

    if(*emptyworkshop == false){
      helped = true;
      print("Santa: helping elves", 0, 0);
      sem_post(s_santastartedhelping);
      sem_post(s_santastartedhelping);
      sem_post(s_santastartedhelping);
      sem_wait(s_santahelping);
    }

  }


}while(*closed == false ||  helped == true);

  *closedworkshop = true;
  print("Santa: closing workshop", 0, 0);

  for(int j = 0; j<*NE; ++j){
    sem_post(s_elfqueue);
    sem_post(s_santastartedhelping);
    sem_post(s_freeelves);
  }

  for(int i = 0; i<*NR; ++i){
    sem_post(s_deerwait);
  }

  sem_wait(s_santadoigdeers);
  print("Santa: Christmas started", 0, 0);
  exit(0);
}

int toNumber(char* strnum){
  //makes argument string into number if possible
    char* endptr = NULL;
    long converted = strtol(strnum, &endptr, 10);
    if (*endptr != '\0' || errno != 0){
        fprintf(stderr,"%s","Wrong paramether!\n");
        exit(1);
    }
    if(converted <= 2147483647){  //checing if long is in range of int to convert

    return (int)converted;
  }else{
    fprintf(stderr, "Too big paramether, couldnot convert to Integer value!\n");
    exit(1);
  }
}

int main(int argc, char* argv[]) {

if(argc != 5){
  fprintf(stderr, "Wrong number of arguments! (./proj2 NE NR TE TR)\n");
  exit(1);
}

  init();

  *NE=toNumber(argv[1]);

  if(!(*NE > 0 && *NE < 1000)){
    fprintf(stderr, "Argument NE isnt in range: (0, 1000)!\n");
    exit(1);
  }


  *NR=toNumber(argv[2]);
  if(!(*NR > 0 && *NR < 20)){
    fprintf(stderr, "Argument NR isnt in range: (0, 20)!\n");
    exit(1);
  }

  *TE=toNumber(argv[3]);
  if(!(*TE >= 0 && *TE <= 1000)){
    fprintf(stderr, "Argument TE isnt in range: <0, 1000>!\n");
    exit(1);
  }

  *TR=toNumber(argv[4]);
  if(!(*TR >= 0 && *TR <= 1000)){
    fprintf(stderr, "Argument TR isnt in range: <0, 1000>!\n");
    exit(1);
  }

//after getting arguments and setting up counter start creating processes

int isSanta = fork();  //create santa process and send it to santa() function
if(isSanta == 0){
  Santa();
}


for(int i =0; i<*NE; ++i){  //create NE elf processes and send them to elf() function
  int id = fork();
  if(id == 0){  //if id is 0 then its child process so send it to workshop
    elf();

  }
}

for(int i =0; i<*NR; ++i){  //creating NR deer processes
  int id = fork();
  if(id == 0){  //if id is 0 then its child process so send it to its function
    reindeer();

  }
}


//wait for all child processes to end, waits in wait(null) when child process ends then it goes through loop again if errno != ECHILD
    while(wait(NULL) != -1 || errno != ECHILD) ;
    clean();

    return 0;
}
