import sys
 
class OutputStream:

  def set_output_function(self, fun):
    self.fun = fun

  def write(self, text): 
    self.fun(text)