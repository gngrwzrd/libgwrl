#!/usr/bin/python
import subprocess,argparse
parser = argparse.ArgumentParser("Generate the lcov html files.")
parser.add_argument("-g","--gcovbasedir",type=str,dest="gcovbasedir",help="Set the base directory where all gcov files are.")
parser.add_argument("-o","--output",type=str,dest="output",help="Set the output directory where html will be generated.")
parser.add_argument("-r","--remove",type=str,dest="remove",action="append",help="Set one or more patters to remove from the generate lcov html.")
args = parser.parse_args()
gcovbasedir = args.gcovbasedir
output = args.output
remove = args.remove
lcov_info = "%s/lcov.info" % output
if remove:
	subprocess.call(["geninfo","-o",lcov_info,gcovbasedir])
	lcov_file_in = "lcov.info"
	for i in range(0,len(remove)):
		lcov_file_out = "lcov%i.info" % (i)
		subprocess.call(["lcov","-o","%s/%s"%(output,lcov_file_out),"-r","%s/%s"%(output,lcov_file_in),remove[i]])
		lcov_file_in = lcov_file_out
	subprocess.call(["genhtml","-o",output,"%s/%s"%(output,lcov_file_out)])
else:
	subprocess.call(["geninfo","-o",lcov_info,gcovbasedir])
	subprocess.call(["genhtml","-o",output,lcov_info])
