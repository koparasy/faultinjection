#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>

#include "cpu/o3/cpu.hh"
#include "cpu/base.hh"

#include "fi/faultq.hh"
#include "fi/cpu_threadInfo.hh"
#include "fi/fi_system.hh"
#include "fi/o3cpu_injfault.hh"
#include "fi/cpu_injfault.hh"
#include "fi/genfetch_injfault.hh"
#include "fi/iew_injfault.hh"
#include "fi/mem_injfault.hh"
#include "fi/opcode_injfault.hh"
#include "fi/pc_injfault.hh"
#include "fi/regdec_injfault.hh"
#include "fi/reg_injfault.hh"



#include "config/the_isa.hh"
#include "base/types.hh"
#include "arch/types.hh"
#include "base/trace.hh"

#include "mem/mem_object.hh"


using namespace std;



Fi_System *fi_system;

Fi_System::Fi_System(Params *p)
  :MemObject(p)
{
  std:: stringstream s1;
  in_name = p->input_fi;
  setcheck(p->check_before_init);
  vectorpos = 0;
  
  fi_system = this;
  
  mainInjectedFaultQueue.setName("MainFaultQueue");
  mainInjectedFaultQueue.setHead(NULL);
  mainInjectedFaultQueue.setTail(NULL);
  fetchStageInjectedFaultQueue.setName("FetchStageFaultQueue");
  fetchStageInjectedFaultQueue.setHead(NULL);
  fetchStageInjectedFaultQueue.setTail(NULL);
  decodeStageInjectedFaultQueue.setName("DecodeStageFaultQueue");
  decodeStageInjectedFaultQueue.setHead(NULL);
  decodeStageInjectedFaultQueue.setTail(NULL);
  iewStageInjectedFaultQueue.setName("IEWStageFaultQueue");
  iewStageInjectedFaultQueue.setHead(NULL);
  iewStageInjectedFaultQueue.setTail(NULL);
  
  

  if(in_name.size() > 1){
    input.open (in_name.c_str(), ifstream::in);
    getFromFile(input);
    input.close();
  }

  
}
Fi_System::~Fi_System(){
  
}

void
Fi_System::init()
{
  if (DTRACE(FaultInjection)) {
    std::cout << "Fi_System:init()\n";
  }
}


void 
Fi_System:: dump(){
  InjectedFault *p;
  
  if (DTRACE(FaultInjection)) {
    std::cout <<"===Fi_System::dump()===\n";
    std::cout << "Input: " << in_name << "\n";
    
    for(fi_activation_iter = fi_activation.begin(); fi_activation_iter != fi_activation.end(); ++fi_activation_iter){
	    (*threadList[fi_activation_iter->second]).dump();
    }
    

    p=mainInjectedFaultQueue.head;
    while(p){

	    p->dump();
	    p=p->nxt;
    }
    
    p=fetchStageInjectedFaultQueue.head;
    while(p){
	    p->dump();
	    p=p->nxt;
    }
    
    p=decodeStageInjectedFaultQueue.head;
    while(p){
	    p->dump();
	    p=p->nxt;
    }
    
    p=iewStageInjectedFaultQueue.head;
    while(p){
	    p->dump();
	    p=p->nxt;
    }
   std::cout <<"~===Fi_System::dump()===\n"; 
  }
  
  
}


void
Fi_System::startup()
{
  if (DTRACE(FaultInjection)) {
    std::cout << "Fi_System:startup()\n";
  }
  dump();
}


Fi_System *
Fi_SystemParams::create()
{
  if (DTRACE(FaultInjection)) {
    std::cout << "Fi_System:create()\n";
  }
    return new Fi_System(this);
}

Port *
Fi_System::getPort(const string &if_name, int idx)
{
  std::cout << "Fi_System:getPort() " << "if_name: " << if_name << " idx: " << idx <<  "\n";
  panic("No Such Port\n");
}

//Initialize faults from a file
//Note that the conditions of how the faults are
//stored in a file are very strict.

void
Fi_System:: getFromFile(std::ifstream &os){
	string check;
	InjectedFault *k;
	
	while(os.good()){
		os>>check;		
		if(check.compare("CPUInjectedFault") ==0){
			k = new CPUInjectedFault(os);
			k->dump();
		}
		else if(check.compare("InjectedFault") ==0){
			k = new InjectedFault(os);
			k->dump();
		}
		else if(check.compare("GeneralFetchInjectedFault") ==0){
			k = new GeneralFetchInjectedFault(os);
			k->dump();
		}
		else if(check.compare("IEWStageInjectedFault") ==0){
			k = new IEWStageInjectedFault(os);
			k->dump();
		}
		else if(check.compare("MemoryInjectedFault") ==0){
			k = new MemoryInjectedFault(os);
			k->dump();
		}
		else if(check.compare("O3CPUInjectedFault") ==0){
			k = new O3CPUInjectedFault(os);
			k->dump();
		}
		else if(check.compare("OpCodeInjectedFault") ==0){
			k = new OpCodeInjectedFault(os);
			k->dump();
		}
		else if(check.compare("PCInjectedFault") ==0){
			k = new PCInjectedFault(os);
			k->dump();
		}
		else if(check.compare("RegisterInjectedFault") ==0){
			k = new RegisterInjectedFault(os);
			k->dump();
		}
		else if(check.compare("RegisterDecodingInjectedFault") ==0){
			k = new RegisterDecodingInjectedFault(os);
			k->dump();
		}
		else{
		  if (DTRACE(FaultInjection)) {
		    std::cout << "No such Object: "<<check<<"\n";
		  }
		}
	}
}




//delete all info and restart from the begining

void
Fi_System:: reset()
{

  std:: stringstream s1;

  vectorpos = 0;
  //remove faults from Queue
  while(!mainInjectedFaultQueue.empty())
    mainInjectedFaultQueue.remove(mainInjectedFaultQueue.head);
  
  while(!fetchStageInjectedFaultQueue.empty())
    fetchStageInjectedFaultQueue.remove(fetchStageInjectedFaultQueue.head);
  
  while(!decodeStageInjectedFaultQueue.empty())
    decodeStageInjectedFaultQueue.remove(decodeStageInjectedFaultQueue.head);
  
  while(!iewStageInjectedFaultQueue.empty())
    iewStageInjectedFaultQueue.remove(iewStageInjectedFaultQueue.head);
 
  
  mainInjectedFaultQueue.setName("MainFaultQueue");
  mainInjectedFaultQueue.setHead(NULL);
  mainInjectedFaultQueue.setTail(NULL);
  fetchStageInjectedFaultQueue.setName("FetchStageFaultQueue");
  fetchStageInjectedFaultQueue.setHead(NULL);
  fetchStageInjectedFaultQueue.setTail(NULL);
  decodeStageInjectedFaultQueue.setName("DecodeStageFaultQueue");
  decodeStageInjectedFaultQueue.setHead(NULL);
  decodeStageInjectedFaultQueue.setTail(NULL);
  iewStageInjectedFaultQueue.setName("IEWStageFaultQueue");
  iewStageInjectedFaultQueue.setHead(NULL);
  iewStageInjectedFaultQueue.setTail(NULL);
  
  threadList.erase(threadList.begin(),threadList.end());
  
  
  if(in_name.size() > 1){
    if (DTRACE(FaultInjection)) {
       std::cout << "Fi_System::Reading New Faults \n";
    }
    input.open (in_name.c_str(), ifstream::in);
    getFromFile(input);
    input.close();
    
    if (DTRACE(FaultInjection)) {
      std::cout << "~Fi_System::Reading New Faults \n";
    }
  }
  dump();
  
}



//Wrapper function
int
Fi_System:: increase_instr_fetched(std:: string curCpu , ThreadEnabledFault *curThread){
   
  
   if(curThread){
      curThread->increaseFetchedInstr(curCpu);
    }
    return (1);
}
//Wrapper function
int
Fi_System:: increase_instr_decoded(std:: string curCpu , ThreadEnabledFault *curThread){
     
    if(curThread)
      curThread->increaseDecodedInstr(curCpu);
    return (1);
}

//Wrapper function
int
Fi_System:: increase_instr_executed(std:: string curCpu , ThreadEnabledFault *curThread){

    if(curThread)
      curThread->increaseExecutedInstr(curCpu);
    return (1);
}


//Wrapper function
int
Fi_System:: increaseTicks(std :: string curCpu , ThreadEnabledFault *curThread, uint64_t ticks){
    

    if(curThread)
      curThread->increaseTicks(curCpu,ticks);
    
    
    return (1);
}

//this function calculates all the instructions that all threads 
// have fetched in a specific core or even on all cores
int
Fi_System:: get_core_fetched_time(std::string Cpu,uint64_t *instr,uint64_t *time){
  uint64_t temp_time;
  uint64_t temp_instr;
  for(fi_activation_iter = fi_activation.begin(); fi_activation_iter != fi_activation.end(); ++fi_activation_iter){
    if(fi_activation_iter->second != -1 ){
      (*threadList[fi_activation_iter->second]).CalculateFetchedTime(Cpu, &temp_instr , &temp_time );
      *time +=temp_time;
      *instr +=temp_instr;
    }
  }
  
  return 1;
}

//this function calculates all the instructions that all threads 
// have decoded in a specific core or even on all cores
int
Fi_System:: get_core_decoded_time(std::string Cpu,uint64_t *instr,uint64_t *time){
  uint64_t temp_time;
  uint64_t temp_instr;
  for(fi_activation_iter = fi_activation.begin(); fi_activation_iter != fi_activation.end(); ++fi_activation_iter){
    if(fi_activation_iter->second != -1 ){
      (*threadList[fi_activation_iter->second]).CalculateDecodedTime(Cpu,&temp_instr ,&temp_time );
      *time +=temp_time;
      *instr +=temp_instr;
    }
  }
  return 1;
}

//this function calculates all the instructions that all threads 
// have executed in a specific core or even on all cores

int
Fi_System:: get_core_executed_time(std::string Cpu,uint64_t *instr,uint64_t *time){
  uint64_t temp_time;
  uint64_t temp_instr;
  for(fi_activation_iter = fi_activation.begin(); fi_activation_iter != fi_activation.end(); ++fi_activation_iter){
    if(fi_activation_iter->second != -1 ){
      (*threadList[fi_activation_iter->second]).CalculateExecutedTime(Cpu , &temp_instr , &temp_time);
      *time +=temp_time;
      *instr +=temp_instr;
    }
  }
  return 1;
}



int 
Fi_System:: get_fi_fetch_counters( InjectedFault *p , ThreadEnabledFault &thread,std::string curCpu , uint64_t *fetch_instr , uint64_t *fetch_time ){
  
  *fetch_time=0;
  *fetch_instr=0;
  // Case :: specific cpu ---- specific thread
  if((p->getWhere().compare(curCpu))==0 && (p->getThread()).compare("all") != 0 && thread.getThreaId() == atoi( (p->getThread()).c_str() ) ){ // case thread_id - cpu_id
      thread.CalculateFetchedTime(curCpu,fetch_instr,fetch_time);
  }//Case :: ALL cores --- specific Thread
  else if((p->getWhere().compare("all") == 0) && (p->getThread()).compare("all") != 0 && thread.getThreaId() == atoi( (p->getThread()).c_str() )){// case thread_id - all
      thread.CalculateFetchedTime("all",fetch_instr,fetch_time);
  }//Case :: Specific Cpu --- All threads
  else if( ( (p->getWhere().compare(curCpu)) == 0  ) && (((p->getThread()).compare("all")) == 0)  ){ //case cpu_id - all
    get_core_fetched_time(curCpu,fetch_instr,fetch_time);
  }//Case:: All cores --- All threads
  else if( ((p->getThread()).compare("all") == 0) && ((p->getWhere().compare("all")) == 0) ){ //case all - all
    get_core_fetched_time("all",fetch_instr,fetch_time);
  }
  if(*fetch_time|*fetch_instr){
    if(p->getFaultType() == p->RegisterInjectedFault || p->getFaultType() == p->PCInjectedFault || p->getFaultType() == p->MemoryInjectedFault  ){
	  p->setCPU(reinterpret_cast<BaseCPU *>(find(curCpu.c_str())));// I may manifest during this cycle so se the core.
	  return 1;
    }
    else if(p->getFaultType() == p->GeneralFetchInjectedFault || p->getFaultType() == p->OpCodeInjectedFault ||
	    p->getFaultType() == p->RegisterDecodingInjectedFault || p->getFaultType() == p->ExecutionInjectedFault){
	O3CPUInjectedFault *k = reinterpret_cast<O3CPUInjectedFault*> (p);
	k->setCPU(reinterpret_cast<BaseO3CPU *>(find(curCpu.c_str())));// I may manifest during this cycle so se the core.
	return 2;
    }
  }
  return 0;
}



int 
Fi_System:: get_fi_decode_counters( InjectedFault *p , ThreadEnabledFault &thread,std::string curCpu , uint64_t *decode_instr , uint64_t *decode_time ){

  // case thread_id - cpu_id
  *decode_time=0;
  *decode_instr=0;
  if((p->getWhere().compare(curCpu))==0 && (p->getThread()).compare("all") != 0 && thread.getThreaId() == atoi( (p->getThread()).c_str() ) ){ 
      thread.CalculateFetchedTime(curCpu,decode_instr,decode_time);
  }// case thread_id - all
  else if((p->getWhere().compare("all") == 0) && (p->getThread()).compare("all") != 0 && thread.getThreaId() == atoi( (p->getThread()).c_str() )){
      thread.CalculateFetchedTime("all",decode_instr,decode_time);
 }//case cpu_id - all
  else if( ( (p->getWhere().compare(curCpu)) == 0  ) && (((p->getThread()).compare("all")) == 0)  ){ 
    get_core_decoded_time(curCpu,decode_instr,decode_time);
  } //case all - all
  else if( ((p->getThread()).compare("all") == 0) && ((p->getWhere().compare("all")) == 0) ){
    get_core_decoded_time("all",decode_instr,decode_time);
  }
  
  if(*decode_time | *decode_instr){
    if(p->getFaultType() == p->RegisterInjectedFault || p->getFaultType() == p->PCInjectedFault || p->getFaultType() == p->MemoryInjectedFault ) {
	p->setCPU(reinterpret_cast<BaseCPU *>(find(curCpu.c_str())));// I may manifest during this cycle so se the core.
	return 1;
    }
    else if(p->getFaultType() == p->GeneralFetchInjectedFault || p->getFaultType() == p->OpCodeInjectedFault ||
	  p->getFaultType() == p->RegisterDecodingInjectedFault || p->getFaultType() == p->ExecutionInjectedFault){
	O3CPUInjectedFault *k = reinterpret_cast<O3CPUInjectedFault*> (p);
	BaseO3CPU *v = reinterpret_cast<BaseO3CPU *>(find(curCpu.c_str())); // I may manifest during this cycle so se the core.
	k->setCPU(v);
      return 2;
    }
  }
   
  return 0;
}




int 
Fi_System:: get_fi_exec_counters( InjectedFault *p , ThreadEnabledFault &thread,std::string curCpu , uint64_t *exec_instr, uint64_t *exec_time  ){
  
  *exec_time=0;
  *exec_instr=0;
  if((p->getWhere().compare(curCpu))==0 && (p->getThread()).compare("all") != 0 && thread.getThreaId() == atoi( (p->getThread()).c_str() ) ){ // case thread_id - cpu_id
      thread.CalculateExecutedTime(curCpu,exec_instr,exec_time);
  }
  else if((p->getWhere().compare("all") == 0) && (p->getThread()).compare("all") != 0 && thread.getThreaId() == atoi( (p->getThread()).c_str() )){// case thread_id - all
      thread.CalculateExecutedTime("all",exec_instr,exec_time);
  }
  else if( ( (p->getWhere().compare(curCpu)) == 0  ) && (((p->getThread()).compare("all")) == 0)  ){ //case cpu_id - all
    get_core_executed_time(curCpu,exec_instr,exec_time);
  }
  else if( ((p->getThread()).compare("all") == 0) && ((p->getWhere().compare("all")) == 0) ){ //case all - all
    get_core_executed_time("all",exec_instr,exec_time);
  }
  if(*exec_time | *exec_instr){
    if(p->getFaultType() == p->RegisterInjectedFault || p->getFaultType() == p->PCInjectedFault || p->getFaultType() == p->MemoryInjectedFault){
	p->setCPU(reinterpret_cast<BaseCPU *>(find(curCpu.c_str()))); // I may manifest during this cycle so se the core.
	return 1;
    }
    else if(p->getFaultType() == p->GeneralFetchInjectedFault || p->getFaultType() == p->OpCodeInjectedFault ||
	    p->getFaultType() == p->RegisterDecodingInjectedFault || p->getFaultType() == p->ExecutionInjectedFault){
	O3CPUInjectedFault *k = reinterpret_cast<O3CPUInjectedFault*> (p);
	BaseO3CPU *v = reinterpret_cast<BaseO3CPU *>(find(curCpu.c_str())); // I may manifest during this cycle so se the core.
	k->setCPU(v);
      return 2;
    }
  }
  return 0;
}





