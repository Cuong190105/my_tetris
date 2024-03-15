import random
f = open("data_test.txt", "w")
for i in range(20):
    for j in range(10):
        rd = random.randint(0,7)
        f.write(str(rd) + " ")
    f.write("\n")
f.close()