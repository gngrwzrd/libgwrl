#!/usr/bin/python
import subprocess,argparse
parser = argparse.ArgumentParser(description="Run all tests gathering gcov data into seperate folders for each test.")
parser.add_argument("-x","--scriptsdir",dest="scriptsdir",type=str,help="Set the scripts directory path.")
parser.add_argument("-s","--searchdir",dest="searchdir",type=str,help="Set the base directory for gcov file searching.")
parser.add_argument("-g","--gcovdir",dest="gcovdir",type=str,help="Set the base directory to save gcov data in.")
parser.add_argument("-t","--testdir",dest="testdir",type=str,help="Set the test directory path.")
args = parser.parse_args()
scriptsdir = args.scriptsdir
searchdir = args.searchdir
gcovdir = args.gcovdir
testdir = args.testdir
tests = []

def add_test(testname):
	global tests,basedir,gcovdir
	tests.append([
		"%sruntest.py" % (scriptsdir),
		"-s","%s" % (searchdir),
		"-i","%ssrc/gwrl/CMakeFiles/gwrl_dyld.dir/" % (searchdir),
		"-g","%s%s/" % (gcovdir,testname),
		"-e","%s%s" % (testdir,testname),
	])

add_test("proactor_buf")
add_test("proactor_filters")
add_test("proactor_free1")
#add_test("proactor_thread_specific_nix")
add_test("proactor_wrq")
add_test("reactor_custom_gathering")
add_test("reactor_del_pers_timeouts")
add_test("reactor_del_pers_timeouts2")
add_test("reactor_del_pers_timeouts3")
add_test("reactor_events")
add_test("reactor_free1")
add_test("reactor_free2")
add_test("reactor_free3")
add_test("reactor_gatherfunc")
add_test("reactor_options")
add_test("reactor_src")
add_test("reactor_timeout_rearm")
add_test("reactor_wake_pipe")
add_test("time_fncs")

for i in range(0,len(tests)): subprocess.call(tests[i])