import pysvn

client = pysvn.Client()

#check out revision 11 of the pysvn project
#client.checkout('http://localhost/example/trunk', './examples/pysvn-11', revision=pysvn.Revision(pysvn.opt_revision_kind.number, 11))

#check out the current version of the pysvn project
print "Creating Libraries Folder"
print "Checking out UIModule"
client.checkout('svn://svnserver/C Libraries/UIModule', './Libraries/UIModule')
print "Checking out GCIUtils"
client.checkout('svn://svnserver/C Libraries/GCI Utils/trunk', './Libraries/GCIUtils')
print "Checking out GCIRegistry"
client.checkout('svn://svnserver/C Libraries/GCI Registry/trunk', './Libraries/GCIRegistry')
print "Checking out Status"
client.checkout('svn://svnserver/C Libraries/Status/trunk', './Libraries/Status')
print "Checking out StringUtils"
client.checkout('svn://svnserver/C Libraries/String Utils/trunk', './Libraries/StringUtils')
print "Checking out Signals"
client.checkout('svn://svnserver/C Libraries/Signals/trunk', './Libraries/Signals')

