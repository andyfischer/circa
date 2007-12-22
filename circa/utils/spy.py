import types
import pdb

def modifyToPrint(originalMethod):
  def replacement(*args, **kwargs):
    output = "Call to: " + originalMethod.im_func.func_name
    arg_list = []
    arg_list += map(str, args)
    arg_list += ([str(key)+"="+str(kwargs[key]) for key in kwargs])
    if arg_list:
      output += " " + str(arg_list)
    print output

    return originalMethod(*args, **kwargs)
  return replacement

def printCall(target_object, method_name):
  """
  Modify the target object so that the method prints to output
  whenever it is called
  """

  attr = getattr(target_object, method_name)
  setattr(target_object, method_name, modifyToPrint(attr))
  
def printAllCalls(target_object):
  for attr_name in dir(target_object):
    attr = getattr(target_object, attr_name)
    if isinstance(attr, types.MethodType):
      setattr(target_object, attr_name, modifyToPrint(attr))

class SpyObject(object):
  def __init__(self, target):
    self.target = target

  def __getattr__(self, name):
    original_obj = getattr(self.target,name)

    # if this is a function then give them a dummy version
    if isinstance(original_obj, types.MethodType):
      return callPrinter(original_obj)
      
    return original_obj
