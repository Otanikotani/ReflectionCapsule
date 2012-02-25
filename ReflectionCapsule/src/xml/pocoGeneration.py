
import commands
import os
import sys

POCO_TOOL = 'pocotools.jar'

def execute(cmd):
    print 'Doing', cmd
    (status, output) = commands.getstatusoutput(cmd)
    if status:    ## Error case, print the command's output to stderr and exit
        sys.stderr.write(output)
        sys.exit(1)
os.chdir(sys.path[0])        
execute('java -cp pocotools.jar com.pocomatic.tools.Project -r=scatte -o=../proxy.cpp setup.xml')
execute('java -cp pocotools.jar com.pocomatic.tools.Encode -tofile=../contextString.cpp -s=.c setup_test_release.xml')




