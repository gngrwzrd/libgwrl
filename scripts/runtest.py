#!/usr/bin/python
"""
This script is used to execute one test, then copy all gcno and gcda
files into a folder. The gcov files are seperated into specific folders
for lcov generation later. Because each test only covers a small amount
of code, all these tests need to be combined by lcov later.
"""
import glob, subprocess, sys, os, fnmatch, shutil, argparse, re
def recursive_glob(rootdir='.', pattern='*'):
	return [os.path.join(rootdir, filename)
		for rootdir, dirnames, filenames in os.walk(rootdir)
		for filename in filenames
		if fnmatch.fnmatch(filename, pattern)]
parser = argparse.ArgumentParser(description="Run a test executable and gather gcov (gcno,gcda) files afterwords.")
parser.add_argument("-s",'--searchdir',dest="searchdir",type=str,help="The directory to recursively search for gcno and gcda files.")
parser.add_argument("-i","--ignoredir",dest="ignoredirs",type=str,action="append",help="Directories with gcno or gcda files to ignore.")
parser.add_argument("-g",'--gcovdir',dest="gcovdir",type=str,help="The directory to store discovered gcov files.")
parser.add_argument("-e","--execfile",dest="execfile",type=str,help="The test executable to run.")
parser.add_argument("-a","--execargs",nargs="*",dest="execargs",type=str,help="The test executable arguments")
parser.add_argument("-d",dest="delgcov",type=bool,help="Delete discovered gcov files instead of copying.")
parser.add_argument("-p",dest="printlogs",type=bool,help="Print some debug logs for this script.")
args = parser.parse_args()
gcovdir = args.gcovdir
searchdir = args.searchdir
ignoredirs = args.ignoredirs or []
executable = args.execfile
execargs = args.execargs
delgcov = args.delgcov
printlogs = args.printlogs
if executable:
	cmd = [executable]
	if execargs: cmd.extend(execargs)
	print " ".join(cmd)
	subprocess.call(cmd)
if gcovdir:
	try: os.makedirs(gcovdir)
	except: pass
gcno = recursive_glob(rootdir=searchdir,pattern="*.gcno")
gcda = recursive_glob(rootdir=searchdir,pattern="*.gcda")
def copy_to_gcov(f):
	global delgcov, gcovdir, searchdir, printlogs, ignoredirs
	fl = f.split("/")[-1]
	for i in range(0,len(ignoredirs)):
		if(re.match(ignoredirs[i],f)):
			if printlogs: print "ignoring %s" % (f)
			return
	if printlogs: print f
	if delgcov:
		if printlogs: print "deleting: %s" % (f)
		os.remove(f)
	else:
		if os.path.exists("%s/%s" % (gcovdir,fl)):
			if printlogs:
				print f
				print "exists: %s/%s" % (gcovdir,fl)
				print ""
		try: shutil.copy2(f,gcovdir)
		except: pass
	if printlogs: print ""
for f in gcno: copy_to_gcov(f)
for f in gcda: copy_to_gcov(f)
