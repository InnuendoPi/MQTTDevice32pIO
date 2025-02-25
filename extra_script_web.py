from shutil import copyfile
from subprocess import check_output, CalledProcessError
import sys
import os
import platform
import subprocess
# Import("env")

print("start reduce index")
subprocess.call("reduce_index.cmd", shell=True)