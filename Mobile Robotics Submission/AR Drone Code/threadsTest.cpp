/* Code to Test Boost Libraries */

#include <iostream>  
#include <boost/thread.hpp>
#include <boost/date_time.hpp>
      
using namespace std;
      
void workerFunc(int id){
   boost::posix_time::seconds workTime(id); 
   
   while(1){
    boost::this_thread::sleep(workTime);
    
    cout << id << " sec Thread" <<endl;
   }
}
      
int main(int argc, char* argv[]){ 
    std::cout << "main: startup" << std::endl;          
    boost::thread workerThread(workerFunc,1);
    boost::thread workerThread2(workerFunc,5);
    boost::thread workerThread3(workerFunc,10);
    boost::thread workerThread4(workerFunc,20);
      
    std::cout << "main: waiting for thread" << std::endl;          
    workerThread.join();
    workerThread2.join();
    workerThread4.join();
    return 0;  
}
