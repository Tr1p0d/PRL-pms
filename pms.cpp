#include <mpi.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <iterator>
#include <algorithm>
#include <queue>

#define TAG 0
#define SIZE 1
#define DONE -1

using namespace std;

int done = -1;

int toggleQueue(int, int);
int main(int argc, char** argv) {
  int numProc;
  int myRank;
  int neighRank;
  MPI_Status stat;            //struct- obsahuje kod- source, tag, error


  MPI_Init(&argc,&argv);                          // inicializace MPI 
  MPI_Comm_size(MPI_COMM_WORLD, &numProc);       // zjistíme, kolik procesů běží 
  MPI_Comm_rank(MPI_COMM_WORLD, &myRank);           // zjistíme id svého procesu 

  neighRank = myRank + 1;

  if(myRank == 0) {

    int number;
    char input[]= "numbers";                          //jmeno souboru    
    fstream fin;                                    //cteni ze souboru
	int nextQueueNumber = 0;
	int counter = 0;

    fin.open(input, ios::in); 	

    while(1){
      number= fin.get();
      if(!fin.good()) break;                      //nacte i eof, takze vyskocim
      cout<<number<<" ";                          //kdo dostane kere cislo
	nextQueueNumber = toggleQueue(1, counter++);
        //buffer,velikost,typ,rank prijemce,tag,komunikacni skupina 
	MPI_Send(&nextQueueNumber, SIZE, MPI_INT, neighRank, TAG, MPI_COMM_WORLD);  	  
        //buffer,velikost,typ,rank prijemce,tag,komunikacni skupina 
        MPI_Send(&number, SIZE, MPI_INT, neighRank, TAG, MPI_COMM_WORLD);      
    }//while

    //cout<<endl<<"process : 0 DONE"<<endl;
    fin.close(); 
    MPI_Send(&done, SIZE, MPI_INT, neighRank, TAG, MPI_COMM_WORLD);  
  }

  if(myRank == 1) {
    int queueNumber;
    int number;
    int maxIndex;
    int nextQueueNumber;
    int counter = 0;

    vector< queue<int>* > qv;
    qv.push_back(new queue<int> );
    qv.push_back(new queue<int> );

    while(1) {
      while ( !qv[0]->empty() && !qv[1]->empty() ) {
	maxIndex = (qv[0]->front() > qv[1]->front() ? 0 : 1);
	nextQueueNumber = toggleQueue(2, counter++);
        //buffer,velikost,typ,rank prijemce,tag,komunikacni skupina 
	MPI_Send(&nextQueueNumber, SIZE, MPI_INT, neighRank, TAG, MPI_COMM_WORLD);  
        //buffer,velikost,typ,rank prijemce,tag,komunikacni skupina 
	MPI_Send(&qv[maxIndex]->front(), SIZE, MPI_INT, neighRank, TAG, MPI_COMM_WORLD);  

	//cout<<"process : "<<myRank<<" sent : "<<qv[maxIndex]->front()<<endl;
	qv[maxIndex]->pop();
      } 

      //buffer,velikost,typ,rank odesilatele,tag, skupina, stat
      MPI_Recv(&queueNumber, SIZE, MPI_INT, 0, TAG, MPI_COMM_WORLD, &stat); 
      if( queueNumber == -1 ) {
	maxIndex = (qv[0]->empty() ? 1 : 0);
	nextQueueNumber = toggleQueue(2, counter++);
	MPI_Send(&nextQueueNumber, SIZE, MPI_INT, neighRank, TAG, MPI_COMM_WORLD);  
	MPI_Send(&qv[maxIndex]->front(), SIZE, MPI_INT, neighRank, TAG, MPI_COMM_WORLD);  

	//cout<<"process : "<<myRank<<" sent : "<<qv[maxIndex]->front()<<endl;
	MPI_Send(&done, SIZE, MPI_INT, neighRank, TAG, MPI_COMM_WORLD);  
	//cout<<"process : "<<myRank<<" DONE "<<endl;
	qv[maxIndex]->pop();
      }
      //buffer,velikost,typ,rank odesilatele,tag, skupina, stat
      MPI_Recv(&number, SIZE, MPI_INT, 0, TAG, MPI_COMM_WORLD, &stat); 
      qv[queueNumber]->push(number);

      //cout<<"process : "<<myRank<<" queue : "<<queueNumber<<" number : "<<number<<endl;
	  
    } // while
  }

  if(myRank == 2) {
    int queueNumber;
	int number;
	int maxIndex;

	vector< queue<int>* > qv;
    qv.push_back(new queue<int> );
    qv.push_back(new queue<int> );

    while(1) {
      while ( !qv[0]->empty() && !qv[1]->empty() ) {
	maxIndex = (qv[0]->front() > qv[1]->front() ? 0 : 1);
	cout<<"process : "<<myRank<<" result : "<<qv[maxIndex]->front()<<endl;
	qv[maxIndex]->pop();
      }
      MPI_Recv(&queueNumber, SIZE, MPI_INT, myRank-1, TAG, MPI_COMM_WORLD, &stat); 
      if( queueNumber == -1 ) {
	maxIndex = (qv[0]->empty() ? 1 : 0);
	cout<<"process : "<<myRank<<" result : "<<qv[maxIndex]->front()<<endl;
	//cout<<"process : "<<myRank<<" DONE "<<endl;
	qv[maxIndex]->pop();
      }
      MPI_Recv(&number, SIZE, MPI_INT, myRank-1, TAG, MPI_COMM_WORLD, &stat); 
      qv[queueNumber]->push(number);
      //cout<<"process : "<<myRank<<" queue : "<<queueNumber<<" number : "<<number<<endl;
    }
  }

  MPI_Finalize();

  return 0;
} 
    

int toggleQueue(int base, int counter) {
  return (counter / base) % 2;
}
    
    
   

