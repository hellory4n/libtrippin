import os

cxx = ""
cflags = ""
ldflags = ""

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

def should_run_and_work(src):
	global cxx, cflags, ldflags
	assert os.system(f"{cxx} {cflags} {ldflags} {src} -o {src.replace(".cpp", "")}") == 0
	assert os.system(f"./{src.replace(".cpp", "")}") == 0

def should_run_and_not_work(src):
	global cxx, cflags, ldflags
	assert os.system(f"{cxx} {cflags} {ldflags} {src} -o {src.replace(".cpp", "")}") == 0
	assert os.system(f"./{src.replace(".cpp", "")}") != 0

def shouldnt_compile(src):
	global cxx, cflags, ldflags
	assert os.system(f"{cxx} {cflags} {ldflags} {src} -o {src.replace(".cpp", "")}") != 0

if __name__ == "__main__":
	main()
