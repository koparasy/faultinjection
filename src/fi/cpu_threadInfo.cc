#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "fi/cpu_threadInfo.hh"
#include "cpu/o3/cpu.hh"
#include "fi/faultq.hh"
#include "fi/fi_system.hh"



//Set all the counters to the correct value
cpuExecutedTicks:: cpuExecutedTicks(std:: string name)
{
  setName(name);
  setInstrFetched(0);
  setInstrDecoded(0);
  setInstrExecuted(0);
  setTicks(0);
}


void cpuExecutedTicks:: dump(){
    if (DTRACE(FaultInjection)) {
    std::cout<<"================\t"<<"CpuExecutedTicks :"<<getName()<<" \t==========================\n"; 
    std::cout << "CpuFetchedInstr: " <<getInstrFetched() <<"\n";
    std::cout << "CpuDecodedInstr: " <<getInstrExecuted() <<  "\n";
    std::cout << "CpuExecutedInstr: "<<getInstrExecuted() <<  "\n";
    std::cout << "Ticks: "<<getTicks() << "\n";
    std::cout<<"================\t~CpuExecutedTicks~\t==========================\n";
  }
}

//a new thread has enabled fault injection get the id and set all the information
ThreadEnabledFault::ThreadEnabledFault(int threadId)
{
  setThreaId(threadId);
  setMyid();
  setMagicInstVirtualAddr(-1);
}





void ThreadEnabledFault::dump(){

  if (DTRACE(FaultInjection)) {
    std::cout<<"================\t"<<"ThreadEnabledFault "<<getMyId()<<" \t==========================\n"; 
    std::cout << "ThreadEnabledInfo  MagicInstVirtualAddr : "<<getMagicInstVirtualAddr()<<" ThreadId :"<<getThreaId() <<"\n";
    std::cout<<"================\t~ThreadEnabledFault~\t==========================\n";
  }
}

//This thread has executed one more cycle increase it

int ThreadEnabledFault:: increaseTicks(std:: string curCpu, uint64_t ticks)
{
 
  //find the core where i am being executed
  itcores=cores.find(curCpu);
  if(itcores == cores.end()){
      //First time I execute on this core add it to my map
      cores.insert(pair<string,cpuExecutedTicks*>(curCpu,new cpuExecutedTicks(curCpu)));
  }
  else{
      itcores->second->increaseTicks(ticks);
  }
  return 1;
}


//This thread has fethed one more instruction.


int ThreadEnabledFault:: increaseFetchedInstr(std:: string curCpu)
{
  //have I fetch instruction from this core before?
  itcores =cores.find(curCpu);
  if(itcores == cores.end()){
    //no
      cores.insert(pair<string,cpuExecutedTicks*>(curCpu,new cpuExecutedTicks(curCpu)));
  }
  else{
      //yes
      itcores->second->increaseFetchInstr();
  }
  return 1;
}

//This thread has decode one more instruction.

int ThreadEnabledFault:: increaseDecodedInstr(std:: string curCpu)
{
  //have I decoded instructions from this core before?
  itcores=cores.find(curCpu);
  if(itcores == cores.end()){
      cores.insert(pair<string,cpuExecutedTicks*>(curCpu,new cpuExecutedTicks(curCpu)));
  }
  else{
      itcores->second->increaseDecodeInstr();
  }
  return 1;
}

//This thread has executed one more instruction.

int ThreadEnabledFault:: increaseExecutedInstr(std:: string curCpu)
{
  itcores=cores.find(curCpu);
  //have I executed instructions from this core before?
  if(itcores == cores.end()){
      cores.insert(pair<string,cpuExecutedTicks*>(curCpu,new cpuExecutedTicks(curCpu)));
  }
  else{
      itcores->second->increaseExecInstr();
  }
  return 1;
  
}


//how many instruction has this thread fetched  untill now on this core or on all cores?

void ThreadEnabledFault:: CalculateFetchedTime(std::string curCpu , uint64_t *fetched_instr , uint64_t *fetched_time )
{

  *fetched_time = 0;
  *fetched_instr = 0;
  //Do i want to find the fetched instructions for this core or for all cores?
  if(curCpu.compare("all")==0){
    //ALL
    for(itcores = cores.begin(); itcores!=cores.end() ; ++itcores){
      *fetched_time  +=  itcores->second->getTicks();
      *fetched_instr +=  itcores->second->getInstrFetched();
    }
  }
  else{
    //SPECIFIC core
    itcores=cores.find(curCpu);
    if(itcores!=cores.end()){
      *fetched_time = itcores->second->getTicks();
      *fetched_instr = itcores->second->getInstrFetched();
      return;
    }
  }
}


void ThreadEnabledFault:: CalculateDecodedTime(std::string curCpu , uint64_t *decoded_instr , uint64_t *decoded_time )
{
  
  *decoded_time = 0;
  *decoded_instr = 0;
  if(curCpu.compare("all")==0){
    for(itcores = cores.begin(); itcores!=cores.end() ; ++itcores){
      *decoded_time  +=  itcores->second->getTicks();
      *decoded_instr +=  itcores->second->getInstrDecoded();
    }
  }
  else{
    itcores=cores.find(curCpu);
    if(itcores!=cores.end()){
      *decoded_time = itcores->second->getTicks();
      *decoded_instr = itcores->second->getInstrDecoded();
      return;
    }
  }
}

void ThreadEnabledFault:: CalculateExecutedTime(std::string curCpu  , uint64_t *exec_instr , uint64_t *exec_time)
{
  *exec_time = 0;
  *exec_instr = 0;
  if(curCpu.compare("all")==0){
    for(itcores = cores.begin(); itcores!=cores.end() ; ++itcores){
      *exec_time  +=  itcores->second->getTicks();
      *exec_instr +=  itcores->second->getInstrExecuted();
    }
  }
  else{
    itcores=cores.find(curCpu);
    if(itcores!=cores.end()){
      *exec_time = itcores->second->getTicks();
      *exec_instr = itcores->second->getInstrExecuted();
      return;
    }
  }
}


//It is good to know how many instructions i have executed on all cores.
//The more the info the better the results.
void ThreadEnabledFault:: print_time(){
  if (DTRACE(FaultInjection)){
    std::cout<<"THREAD ID: "<<getMyId()<<"\n";
    for(itcores = cores.begin(); itcores!=cores.end() ; ++itcores){
      std::cout<<"CORE:"<<itcores->second->getName()<<"\n";
      std::cout<<"Fetched Instr: "<< itcores->second->getInstrFetched() <<"\n";
      std::cout<<"Decoded Instr: "<< itcores->second->getInstrDecoded() <<"\n";
      std::cout<<"Executed Instr: "<< itcores->second->getInstrExecuted() <<"\n";
      std::cout<<"Ticks : "<< itcores->second->getTicks() <<"\n";
    }
    
  }
}






