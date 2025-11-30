import os
from concurrent.futures import ThreadPoolExecutor, as_completed

srcs_should_work = [
	"test/buffer.cpp",
	"test/memory.cpp",
	"test/memory_refwrapper.cpp",
]
srcs_shouldnt_work = []
srcs_shouldnt_compile = []

COLOR_RESET = "\033[0m"
COLOR_RED = "\033[0;91m"
COLOR_GREEN = "\033[0;32m"

cxx = ""
cflags = ""
ldflags = ""
tests_succeeded = 0
total_tests = 0

def main():
	global cxx, cflags, ldflags, srcs_should_work, srcs_shouldnt_work, srcs_shouldnt_compile

	if not os.path.exists("build.ninja"):
		raise Exception("please run configure.py first")

	# get the compiler + cflags + company
	with open("build.ninja", "r") as f:
		f.readline() # skip initial comment
		cxx = f.readline()[len("cxx = "):].replace("\n", "")
		cflags = f.readline()[len("cflags = "):].replace("\n", "")
		ldflags = f.readline()[len("ldflags = "):].replace("\n", "")

	assert os.system("ninja") == 0

	# some fuckery to run them in parallel
	# TODO this will take a while once we have 5 billion trillion tests
	max_workers = len(srcs_should_work) + len(srcs_shouldnt_work) + len(srcs_shouldnt_compile)
	with ThreadPoolExecutor(max_workers=max_workers) as executor:
		futures = []
		for s in srcs_should_work:
			futures.append(executor.submit(should_work, s))
		for s in srcs_shouldnt_work:
			futures.append(executor.submit(shouldnt_work, s))
		for s in srcs_shouldnt_compile:
			futures.append(executor.submit(shouldnt_compile, s))

		# wait for everything to finish
		for future in as_completed(futures):
			pass

	print(f"\n{tests_succeeded}/{total_tests} tests succeeded")

def should_work(src):
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

def shouldnt_work(src):
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
