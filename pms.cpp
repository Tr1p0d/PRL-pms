#include <mpi.h>
//#include <assert.h>
#include <iostream>
#include <fstream>
#include <vector>
//#include <iterator>
#include <algorithm>
#include <queue>
#include <cmath>
//#include <chrono>

#define TAG 0
#define SIZE 1

long long int totalNumbers = 0;

typedef struct {
  int numProc;
  int myRank;
  int neighRank;
  int predRank;
  int base;
  int predBase;
  int neighBase;
} t_SORTER;

using namespace std;

MPI_Status stat;            //struct- obsahuje kod- source, tag, error

int toggleQueue(int, int);

int runFirstSorter(t_SORTER* s) {
  int number;
  char input[]= "numbers";                          //jmeno souboru    
  fstream fin;                                    //cteni ze souboru
  int nextQueueNumber = 0;
  int counter = 0;
  queue<int> inputQ;

  fin.open(input, ios::in); 	

  while(1){
    number= fin.get();
    if(!fin.good()) break;                      //nacte i eof, takze vyskocim
    inputQ.push(number);


  }//while
    fin.close(); 

	totalNumbers = inputQ.size();
    for(int i = 0; i < inputQ.size(); i++) {
    	number = inputQ.front();
    	inputQ.pop();
    	cout << number << " ";
    	inputQ.push(number); // znova vlozim do fronty
    }    
    cout<<endl;

  	MPI_Bcast(&totalNumbers, 1, MPI_INT, 0, MPI_COMM_WORLD);

	while ( inputQ.size() > 0 ) {
      number = inputQ.front(); 
      nextQueueNumber = toggleQueue(s->base, counter++);
      MPI_Send(&nextQueueNumber, SIZE, MPI_INT, s->neighRank, TAG, MPI_COMM_WORLD);  	  
      MPI_Send(&number, SIZE, MPI_INT, s->neighRank, TAG, MPI_COMM_WORLD);      
	  inputQ.pop();
	}
 // cout<<endl;
 // cout<<endl<<"process : 0 DONE"<<endl;
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

  MPI_Bcast(&totalNumbers, 1, MPI_INT, 0, MPI_COMM_WORLD);

  //cout<<"process : "<<s->myRank<<" total "<<totalNumbers<<endl;
  while ( counter < totalNumbers ) {
    //cout<<"process : "<<s->myRank<<" q0size : "<<qv[0]->size()<<endl;
    //cout<<"process : "<<s->myRank<<" q1size : "<<qv[1]->size()<<endl;
    if ( (qv[0]->size() >= s->predBase/*(1 << (s->myRank - 1)) predSorterBase(s->base)*/ ) && (qv[1]->size() >= 1) ) {
  //      cout<<"process : "<<s->myRank<<" entering cycle : "<<endl;

      for( int orderCount = 0, received = 0 ; orderCount < s->base; orderCount++ ) {
		if ( !qv[0]->empty() && !qv[1]->empty() ) 
          maxIndex = qv[0]->front() > qv[1]->front() ? 1 : 0; 
		else if ( qv[1]->empty() ) 
		  maxIndex = 0;
		else 
		  maxIndex = 1;

//        cout<<"process : "<<s->myRank<<" maxindex: "<< maxIndex<<endl;
        nextQueueNumber = toggleQueue(s->base, counter++);

        MPI_Send(&nextQueueNumber, SIZE, MPI_INT, s->neighRank, TAG, MPI_COMM_WORLD);  
        MPI_Send(&qv[maxIndex]->front(), SIZE, MPI_INT, s->neighRank, TAG, MPI_COMM_WORLD);  
 //       cout<<"process : "<<s->myRank<<" sent : "<<qv[maxIndex]->front()<<endl;
		qv[maxIndex]->pop();
		
        if ( received < s->predBase - 1 /*(1 << (s->myRank - 1)) predSorterBase(s->base) - 1*/) { 
          MPI_Recv(&queueNumber, SIZE, MPI_INT, s->predRank, TAG, MPI_COMM_WORLD, &stat); 
          MPI_Recv(&number, SIZE, MPI_INT, s->predRank, TAG, MPI_COMM_WORLD, &stat); 
          qv[queueNumber]->push(number);
 //         cout<<"process : "<<s->myRank<<" queue : "<<queueNumber<<" number : "<<number<<endl;
	      received++;
		}
	  }
   //     cout<<"process : "<<s->myRank<<" exiting cycle : "<<endl;
	} else {
      if ( received != totalNumbers) { 
      MPI_Recv(&queueNumber, SIZE, MPI_INT, s->predRank, TAG, MPI_COMM_WORLD, &stat); 
      MPI_Recv(&number, SIZE, MPI_INT, s->predRank, TAG, MPI_COMM_WORLD, &stat); 
      qv[queueNumber]->push(number);
   //   cout<<"process : "<<s->myRank<<" queue : "<<queueNumber<<" number : "<<number<<endl;
	  received++;
		}
	}
  }
//  cout<<"process : "<<s->myRank<<" DONE"<<endl;
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

  MPI_Bcast(&totalNumbers, 1, MPI_INT, 0, MPI_COMM_WORLD);

  while ( 1 ) {
    if ( qv[0]->size() >= s->predBase && qv[1]->size() >= 1 ) {
      while( counter < totalNumbers ) {
		if ( !qv[0]->empty() && !qv[1]->empty() )
          maxIndex = qv[0]->front() > qv[1]->front() ? 1 : 0; 
		else if ( qv[1]->empty() ) 
		  maxIndex = 0;
		else 
		  maxIndex = 1;

        cout<<qv[maxIndex]->front()<<endl;
		qv[maxIndex]->pop();
		counter ++;
          
        //cout<<"process : "<<s->myRank<<" ordercount : "<< counter <<endl;
        if ( received != totalNumbers) { 
          MPI_Recv(&queueNumber, SIZE, MPI_INT, s->predRank, TAG, MPI_COMM_WORLD, &stat); 
          MPI_Recv(&number, SIZE, MPI_INT, s->predRank, TAG, MPI_COMM_WORLD, &stat); 
          qv[queueNumber]->push(number);
//          cout<<"process : "<<s->myRank<<" queue : "<<queueNumber<<" number : "<<number<<endl;
		  received++;
		}
	  }
	  break;
	} else {
      MPI_Recv(&queueNumber, SIZE, MPI_INT, s->predRank, TAG, MPI_COMM_WORLD, &stat); 
      MPI_Recv(&number, SIZE, MPI_INT, s->predRank, TAG, MPI_COMM_WORLD, &stat); 
      qv[queueNumber]->push(number);
  //    cout<<"process : "<<s->myRank<<" queue : "<<queueNumber<<" number : "<<number<<endl;
	  received++;
	}
  }
 // cout<<"process : "<<s->myRank<<" DONE"<<endl;
  return 0;
}

int main(int argc, char** argv) {
  t_SORTER* s = new t_SORTER; 

//  std::chrono::time_point<std::chrono::system_clock> start;
//  std::chrono::time_point<std::chrono::system_clock> end;

  MPI_Init(&argc,&argv);                          
  MPI_Comm_size(MPI_COMM_WORLD, &s->numProc);
  MPI_Comm_rank(MPI_COMM_WORLD, &s->myRank);       

  s->predRank = s->myRank - 1;
  s->neighRank = s->myRank + 1;
  s->predBase = 1 << s->predRank;
  s->base = 1 << s->myRank;
  s->neighBase = 1 << s->neighRank;

//  if (s->myRank == (s->numProc - 1)) {
//    start = std::chrono::high_resolution_clock::now();
//  }

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
	//end = std::chrono::high_resolution_clock::now();
	//std::chrono::duration<double> diff = end-start;
//	cerr<<diff.count()<<endl;
  }

  MPI_Finalize();
  return 0;
} 
int toggleQueue(int base, int counter) {
  return (counter / base) % 2;
}
