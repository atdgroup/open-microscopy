import os, re

def rmgeneric(path, __func__):

    try:
        __func__(path)
        print 'Removed ', path
    except OSError, (errno, strerror):
        print "Remove Error ", strerror

            
def removeall(path):

    if not os.path.isdir(path):
        return

    files=os.listdir(path)

    for x in files:
        
        fullpath=os.path.join(path, x)

        if os.path.isfile(fullpath):

            f=os.remove
            rmgeneric(fullpath, f)
            
        elif os.path.isdir(fullpath):
            
            removeall(fullpath)
            f=os.rmdir
            rmgeneric(fullpath, f)

    os.rmdir(path)
      

remove_file_list = [".*\.exe?",
                    ".*\.cdb?",
                    ".*\.niobj?",
                    ".*\.nidobj?",
                    ".*\.cab",
                    ".*\.msi"
                    ".*\.ncb",
                    ".*\.pdb",
                    ".*\.ilk",
                    ".*\.idb"]
      
def remove_crud(dirname):
      
    head, tail = os.path.split(dirname)

    if re.compile("cvibuild*").match(tail, 0) != None:   
        removeall(dirname)
    elif re.compile("cvidistkit*").match(tail, 0) != None: 
        removeall(dirname)
    else:
        
        files=os.listdir(dirname)

        for filename in files:
        
            if filename == '.svn':
                continue
        
            fullpath=os.path.join(dirname, filename)
        
            for fi in remove_file_list:
                if re.compile(fi).match(filename, 0) != None and os.path.isfile(fullpath):
                    os.remove(fullpath)   

                    
cwd = os.getcwd() 

for p, dirs, files in os.walk(cwd):
    
    # Skip .svn directory
    try:
        dirs.remove(".svn")
    except ValueError:
        pass

    remove_crud(p)