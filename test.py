import os

COLOR_RESET = "\033[0m"
COLOR_RED = "\033[0;91m"
COLOR_GREEN = "\033[0;32m"

cxx = ""
cflags = ""
ldflags = ""
tests_succeeded = 0
total_tests = 0

def main():
	global cxx, cflags, ldflags

	if not os.path.exists("build.ninja"):
		raise Exception("please run configure.py first")

	# get the compiler + cflags + company
	with open("build.ninja", "r") as f:
		f.readline() # skip initial comment
		cxx = f.readline()[len("cxx = "):].replace("\n", "")
		cflags = f.readline()[len("cflags = "):].replace("\n", "")
		ldflags = f.readline()[len("ldflags = "):].replace("\n", "")

	assert os.system("ninja") == 0

	# testma<3
	should_run_and_work("test/memory.cpp")

	print(f"\n{tests_succeeded}/{total_tests} tests succeeded")

def should_run_and_work(src):
	global cxx, cflags, ldflags, tests_succeeded, total_tests
	total_tests += 1

	if os.system(f"{cxx} {cflags} {ldflags} {src} libtrippin.a -o {src.replace(".cpp", "")}") != 0:
		print(f"{COLOR_RED}* FAILED: {src}{COLOR_RESET}")
		return
	if os.system(f"./{src.replace(".cpp", "")}") != 0:
		print(f"{COLOR_RED}* FAILED: {src}{COLOR_RESET}")
		return

	print(f"{COLOR_GREEN}* OK:     {src}{COLOR_RESET}")
	tests_succeeded += 1

def should_run_and_not_work(src):
	global cxx, cflags, ldflags, tests_succeeded, total_tests
	total_tests += 1

	if os.system(f"{cxx} {cflags} {ldflags} {src} libtrippin.a -o {src.replace(".cpp", "")}") != 0:
		print(f"{COLOR_RED}* FAILED: {src}{COLOR_RESET}")
		return
	if os.system(f"./{src.replace(".cpp", "")}") == 0:
		print(f"{COLOR_RED}* FAILED: {src}{COLOR_RESET}")
		return

	print(f"{COLOR_GREEN}* OK:     {src}{COLOR_RESET}")
	tests_succeeded += 1

def shouldnt_compile(src):
	global cxx, cflags, ldflags, tests_succeeded, total_tests
	total_tests += 1

	if os.system(f"{cxx} {cflags} {ldflags} {src} libtrippin.a -o {src.replace(".cpp", "")}") == 0:
		print(f"* FAILED: {src}")
		return

	print(f"{COLOR_GREEN}* OK:     {src}{COLOR_RESET}")
	tests_succeeded += 1

if __name__ == "__main__":
	main()
