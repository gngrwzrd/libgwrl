#!/usr/bin/python
import subprocess,argparse
parser = argparse.ArgumentParser("Generate the lcov html files.")
parser.add_argument("-g","--gcovbasedir",type=str,dest="gcovbasedir",help="Set the base directory where all gcov files are.")
parser.add_argument("-o","--output",type=str,dest="output",help="Set the output directory where html will be generated.")
args = parser.parse_args()
gcovbasedir = args.gcovbasedir
output = args.output
lcov_info = "%s/all.info" % output
html_dir = "%s/html" % output
subprocess.call(["geninfo","-o",lcov_info,gcovbasedir])
subprocess.call(["genhtml","-o",output,lcov_info])
