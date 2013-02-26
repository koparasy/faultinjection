#ifndef __FI_RELATIVE__
#define __FI_RELATIVE__


#include <iostream>

#include "fi/faultq.hh"
#include "fi/cpu_threadInfo.hh"
#include "fi/fi_system.hh"





class FI_Inst
{
public:
  void 
  fi_activate_inst(Addr PCBAddr, uint64_t instCnt)
  {  
    DPRINTF(FaultInjection, "===Fault Injection Activation Instruction===\n");
    DPRINTF(FaultInjection, "\tFetched Instructions: %llu\n", instCnt);
    DPRINTF(FaultInjection, "\tSimulation Ticks: %llu\n", curTick());
    
    
    DPRINTF(FaultInjection, "\t Process Control Block(PCB) Addressx: %llx\n", PCBAddr);
    
    fi_system->fi_activation_iter = fi_system->fi_activation.find(PCBAddr);
    if (fi_system->fi_activation_iter == fi_system->fi_activation.end()) { //insert new
      fi_system->fi_activation[PCBAddr] = true;
    } else { //flip old
      fi_system->fi_activation[PCBAddr] = !fi_system->fi_activation_iter->second;
    }
    

    
    
    DPRINTF(FaultInjection, "~==Fault Injection Activation Instruction===\n");
  }

  void
  fi_relative_inst(std::string name, Addr PCAddr, uint64_t instCnt, uint64_t ticks)
  {

    
    if (DTRACE(FaultInjection)) {
      std::cout << "===Fault Injection Relative Point Instruction===\n";
      std::cout << "~==Fault Injection Relative Point Instruction===\n";
    }
  }

};
extern FI_Inst fi_inst;

#endif //__FI_RELATIVE__
