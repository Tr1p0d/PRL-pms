#include <mpi.h>
#include <assert.h>
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

int totalNumbers = 16;

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
double predSorterBase(int);

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

  //cout<<endl;
  cout<<endl<<"process : 0 DONE"<<endl;
  fin.close(); 
  MPI_Send(&done, SIZE, MPI_INT, s->neighRank, TAG, MPI_COMM_WORLD);  
  return 0;
}

int runSorter(t_SORTER* s) {
  int queueNumber;
  int number;
  int maxIndex;
  int received = 0;
  int nextQueueNumber;
  int counter = 0;
 
  vector< queue<int>* > qv;
  qv.push_back(new queue<int> );
  qv.push_back(new queue<int> );

  while ( counter < totalNumbers ) {
    //cout<<"process : "<<s->myRank<<" q0size : "<<qv[0]->size()<<endl;
    //cout<<"process : "<<s->myRank<<" q1size : "<<qv[1]->size()<<endl;
    if ( (qv[0]->size() >= predSorterBase(s->base) ) && (qv[1]->size() >= 1) ) {
        cout<<"process : "<<s->myRank<<" entering cycle : "<<endl;

      for( int orderCount = 0, received = 0 ; orderCount < s->base; orderCount++ ) {
        maxIndex = qv[0]->front() < qv[1]->front() ? 1 : 0; 
        cout<<"process : "<<s->myRank<<" maxindex: "<< maxIndex<<endl;
        nextQueueNumber = toggleQueue(s->base, counter++);

        MPI_Send(&nextQueueNumber, SIZE, MPI_INT, s->neighRank, TAG, MPI_COMM_WORLD);  
        MPI_Send(&qv[maxIndex]->front(), SIZE, MPI_INT, s->neighRank, TAG, MPI_COMM_WORLD);  
        cout<<"process : "<<s->myRank<<" sent : "<<qv[maxIndex]->front()<<endl;
		qv[maxIndex]->pop();
		
        if ( received < predSorterBase(s->base) - 1) { 
          MPI_Recv(&queueNumber, SIZE, MPI_INT, s->predRank, TAG, MPI_COMM_WORLD, &stat); 
          MPI_Recv(&number, SIZE, MPI_INT, s->predRank, TAG, MPI_COMM_WORLD, &stat); 
          qv[queueNumber]->push(number);
          cout<<"process : "<<s->myRank<<" queue : "<<queueNumber<<" number : "<<number<<endl;
	      received++;
		}
	  }
	} else {
      if ( received != totalNumbers) { 
      MPI_Recv(&queueNumber, SIZE, MPI_INT, s->predRank, TAG, MPI_COMM_WORLD, &stat); 
      MPI_Recv(&number, SIZE, MPI_INT, s->predRank, TAG, MPI_COMM_WORLD, &stat); 
      qv[queueNumber]->push(number);
      cout<<"process : "<<s->myRank<<" queue : "<<queueNumber<<" number : "<<number<<endl;
	  received++;
		}
	}
  }
  cout<<"process : "<<s->myRank<<" DONE"<<endl;
  return 0;
} 

int runLastSorter(t_SORTER* s) {
  int queueNumber = 0;
  int number = 0;
  int received = 0;
  int counter = 0;
  int maxIndex;

  vector< queue<int>* > qv;
  qv.push_back(new queue<int> );
  qv.push_back(new queue<int> );

  while ( 1 ) {
    if ( qv[0]->size() >= predSorterBase(s->base) && qv[1]->size() >= 1 ) {
      while( counter < totalNumbers ) {
        maxIndex = qv[0]->front() < qv[1]->front() ? 1 : 0; 
        cout<<qv[maxIndex]->front()<<endl;
		qv[maxIndex]->pop();
		counter ++;
          
        //cout<<"process : "<<s->myRank<<" ordercount : "<< counter <<endl;
        if ( received != totalNumbers) { 
          MPI_Recv(&queueNumber, SIZE, MPI_INT, s->predRank, TAG, MPI_COMM_WORLD, &stat); 
          MPI_Recv(&number, SIZE, MPI_INT, s->predRank, TAG, MPI_COMM_WORLD, &stat); 
          qv[queueNumber]->push(number);
          cout<<"process : "<<s->myRank<<" queue : "<<queueNumber<<" number : "<<number<<endl;
		  received++;
		}
	  }
	  break;
	} else {
      MPI_Recv(&queueNumber, SIZE, MPI_INT, s->predRank, TAG, MPI_COMM_WORLD, &stat); 
      MPI_Recv(&number, SIZE, MPI_INT, s->predRank, TAG, MPI_COMM_WORLD, &stat); 
      qv[queueNumber]->push(number);
      cout<<"process : "<<s->myRank<<" queue : "<<queueNumber<<" number : "<<number<<endl;
	  received++;
	}
  }
  cout<<"process : "<<s->myRank<<" DONE"<<endl;
  return 0;
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
double predSorterBase(int base) {
  pow(2,(log2(base)-1));
}

int toggleQueue(int base, int counter) {
  return (counter / base) % 2;
}
