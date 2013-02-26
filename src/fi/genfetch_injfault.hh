#ifndef __GENERAL_FETCH_INJECTED_FAULT_HH__
#define __GENERAL_FETCH_INJECTED_FAULT_HH__

#include "config/the_isa.hh"
#include "base/types.hh"
#include "fi/faultq.hh"
#include "fi/o3cpu_injfault.hh"
#include "cpu/o3/cpu.hh"

/*
 * Inject a fault into the fetch stage (instruction)
 */


class GeneralFetchInjectedFault : public O3CPUInjectedFault
{

public:

  GeneralFetchInjectedFault(std::ifstream &os);
  ~GeneralFetchInjectedFault();

  virtual const char *description() const;
  
  void dump() const;

  TheISA::MachInst process(TheISA::MachInst inst); // The MachInst is the pure binary form 
						    //of the instruction so insert the 
						    //fault just before fetching the instruction

	
};

#endif // __GENERAL_FETCH_INJECTED_FAULT_HH__
