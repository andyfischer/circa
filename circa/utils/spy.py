import types

def modifyToPrint(originalMethod):
  """
  Return a version of originalMethod that prints when it is called
  """
  def replacement(*args, **kwargs):
    output = "Call to: " + originalMethod.im_func.func_name

    # put the arguments into a string
    arg_list = []
    arg_list += map(str, args)
    arg_list += ([str(key)+"="+str(kwargs[key]) for key in kwargs])
    if arg_list:
      output += "(" + ",".join(arg_list) + ")"
    else:
      output += "()"
    print output

    # call the original method and return its result
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

