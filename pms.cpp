#include <mpi.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <iterator>
#include <algorithm>
#include <queue>
#include <cmath>

#define TAG 0
#define SIZE 1
#define DONE -1

typedef struct {
  int numProc;
  int myRank;
  int neighRank;
  int predRank;
  int base;
} t_SORTER;

using namespace std;

MPI_Status stat;            //struct- obsahuje kod- source, tag, error
int done = -1;

bool toggleLastNuber(int, int);
int toggleQueue(int, int);

int runFirstSorter(t_SORTER* s) {
  int number;
  char input[]= "numbers";                          //jmeno souboru    
  fstream fin;                                    //cteni ze souboru
  int nextQueueNumber = 0;
  int counter = 0;

  fin.open(input, ios::in); 	

  while(1){
    number= fin.get();
    if(!fin.good()) break;                      //nacte i eof, takze vyskocim
    cout<< number <<" ";
    nextQueueNumber = toggleQueue(s->base, counter++);
    //buffer,velikost,typ,rank prijemce,tag,komunikacni skupina 
    MPI_Send(&nextQueueNumber, SIZE, MPI_INT, s->neighRank, TAG, MPI_COMM_WORLD);  	  
    //buffer,velikost,typ,rank prijemce,tag,komunikacni skupina 
    MPI_Send(&number, SIZE, MPI_INT, s->neighRank, TAG, MPI_COMM_WORLD);      
  }//while

  cout<<endl;
  cout<<endl<<"process : 0 DONE"<<endl;
  fin.close(); 
  MPI_Send(&done, SIZE, MPI_INT, s->neighRank, TAG, MPI_COMM_WORLD);  
  return 0;
}

int runSorter(t_SORTER* s) {
  int queueNumber;
  int number;
  int maxIndex;
  int nextQueueNumber;
  int counter = 0;
  int lastNumber = 0;
 
  vector< queue<int>* > qv;
  qv.push_back(new queue<int> );
  qv.push_back(new queue<int> );
 
  while(1) {
    while ( !qv[0]->empty() && !qv[1]->empty() ) {
      if( toggleLastNuber(s->base, counter)) {
        //cout<<"process : "<<s->myRank<<"smaller : "<<qv[0]->front()<<" "<<maxIndex<<" "<<qv[1]->front()<<endl;
        if ( qv[0]->front() < lastNumber && qv[1]->front() < lastNumber ) {
          maxIndex = (qv[0]->front() > qv[1]->front() ? 0 : 1);
        } else if (qv[0]->front() < lastNumber && qv[1]->front() > lastNumber) {
          maxIndex = 0;
        } else {
          maxIndex = 1;
        }
      } else {
        maxIndex = (qv[0]->front() > qv[1]->front() ? 0 : 1);
      }
      nextQueueNumber = toggleQueue(s->base, counter++);
      //buffer,velikost,typ,rank prijemce,tag,komunikacni skupina 
      MPI_Send(&nextQueueNumber, SIZE, MPI_INT, s->neighRank, TAG, MPI_COMM_WORLD);  
      //buffer,velikost,typ,rank prijemce,tag,komunikacni skupina 
      MPI_Send(&qv[maxIndex]->front(), SIZE, MPI_INT, s->neighRank, TAG, MPI_COMM_WORLD);  
      lastNumber = qv[maxIndex]->front();
 
      cout<<"process : "<<s->myRank<<" sent : "<<qv[maxIndex]->front()<<endl;
      qv[maxIndex]->pop();
    } 
 
    //buffer,velikost,typ,rank odesilatele,tag, skupina, stat
    MPI_Recv(&queueNumber, SIZE, MPI_INT, s->predRank, TAG, MPI_COMM_WORLD, &stat); 
    if( queueNumber == -1 ) {
      maxIndex = (qv[0]->empty() ? 1 : 0);
      while ( !qv[maxIndex]->empty() ) {
        nextQueueNumber = toggleQueue(s->base, counter++);
        MPI_Send(&nextQueueNumber, SIZE, MPI_INT, s->neighRank, TAG, MPI_COMM_WORLD);  
        MPI_Send(&qv[maxIndex]->front(), SIZE, MPI_INT, s->neighRank, TAG, MPI_COMM_WORLD);  
 
        cout<<"process : "<<s->myRank<<" sent : "<<qv[maxIndex]->front()<<endl;
        qv[maxIndex]->pop();
      }
      MPI_Send(&done, SIZE, MPI_INT, s->neighRank, TAG, MPI_COMM_WORLD);  
      cout<<"process : "<<s->myRank<<" DONE "<<endl;
      return 0;
    }
    //buffer,velikost,typ,rank odesilatele,tag, skupina, stat
    MPI_Recv(&number, SIZE, MPI_INT, s->predRank, TAG, MPI_COMM_WORLD, &stat); 
    qv[queueNumber]->push(number);
 
    cout<<"process : "<<s->myRank<<" queue : "<<queueNumber<<" number : "<<number<<endl;
  } // while
}

int runLastSorter(t_SORTER* s) {
  int queueNumber = 0;
  int number = 0;
  int maxIndex;

  vector< queue<int>* > qv;
  qv.push_back(new queue<int> );
  qv.push_back(new queue<int> );

  while(1) {
    while ( !qv[0]->empty() && !qv[1]->empty() ) {
      maxIndex = (qv[0]->front() > qv[1]->front() ? 0 : 1);
      cout<<qv[maxIndex]->front()<<endl;
      qv[maxIndex]->pop();
    }
    MPI_Recv(&queueNumber, SIZE, MPI_INT, s->predRank, TAG, MPI_COMM_WORLD, &stat); 
    if( queueNumber == -1 ) {

      maxIndex = (qv[0]->empty() ? 1 : 0);
      while ( !qv[maxIndex]->empty() ) {
        cout<<qv[maxIndex]->front()<<endl;
        qv[maxIndex]->pop();
      }
      
      cout<<"process : "<<s->myRank<<" DONE "<<endl;
      return 0;
    }
    MPI_Recv(&number, SIZE, MPI_INT, s->predRank, TAG, MPI_COMM_WORLD, &stat); 
    qv[queueNumber]->push(number);
    cout<<"process : "<<s->myRank<<" queue : "<<queueNumber<<" number : "<<number<<endl;
//    cout<<"process : "<<s->myRank<<" queue : "<<queueNumber<<" pred : "<<s->predRank<<endl;
  }
}

int main(int argc, char** argv) {
  t_SORTER* s = new t_SORTER; 

  MPI_Init(&argc,&argv);                          
  MPI_Comm_size(MPI_COMM_WORLD, &s->numProc);
  MPI_Comm_rank(MPI_COMM_WORLD, &s->myRank);       

  s->predRank = s->myRank - 1;
  s->neighRank = s->myRank + 1;
  s->base = pow(2, s->myRank);
 
  if ( s->myRank == 0 ) {
    runFirstSorter(s);
  }

  for( int i = 1; i < (s->numProc - 1); i++) {
    if ( s->myRank == i ) {
      runSorter(s);
    }
  } 

  if ( s->myRank == (s->numProc - 1)) {
    runLastSorter(s);
  }

  MPI_Finalize();
  return 0;
} 
bool toggleLastNuber(int base, int counter) {
  return (counter % base) != 0;
}

int toggleQueue(int base, int counter) {
  return (counter / base) % 2;
}
