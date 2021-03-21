import random, sys
with open(sys.argv[2], "w") as f:
	f.write("\n".join([str(random.randint(1,1000)) for i in range(int(sys.argv[1]))]))
