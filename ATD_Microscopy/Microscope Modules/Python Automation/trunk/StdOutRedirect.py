import sys, gci, Output

def WriteOutput(text):
    gci.ShowStatus("Python Module", text)

ostream = Output.OutputStream()
ostream.set_output_function(WriteOutput)

sys.stdout = ostream
sys.stderr = ostream