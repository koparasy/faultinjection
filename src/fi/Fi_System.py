from m5.params import *
from MemObject import MemObject

class Fi_System(MemObject):
  type='Fi_System'
  input_fi=Param.String("","Input File Name")
  check_before_init=Param.Bool(False, "create CheckPoint before initialize of fault injection system")
  
  
