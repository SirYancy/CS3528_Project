import random

coords = list()
roads = ('Ave.', 'St.')

for i in range(100):
    coords.append((random.randint(-20,20), random.randint(-20,20)))

f = open("addresses.txt", 'w')

for t in coords:
    quadrant = ""
    if t[1] < 0:
        quadrant += "S"
    elif t[1] > 0:
        quadrant += "N"
    if t[0] < 0:
        quadrant += "W"
    elif t[0]>0:
        quadrant += "E" 
    
    r = random.randint(0,1)
    if r == 0:
        a = "" + str(abs(t[0] * 100 + random.randint(0,100))) + " " + str(abs(t[1])) + " " + roads[r] + " " + quadrant + ", Bemidji, MN, 56601\n"
        f.write(a)

f.close()
