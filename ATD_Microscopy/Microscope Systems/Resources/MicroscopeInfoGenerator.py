import xmlrpclib
import datetime

proxy = xmlrpclib.ServerProxy("http://localhost:63782/")

print proxy.MicroscopeName()
print proxy.Uptime()

raw_input()