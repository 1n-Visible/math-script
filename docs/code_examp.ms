a=1;
$b=[4, 1.125, 23i, -2];
s="Hello, mathscript world!"
newl=$'\t'

for (x in b):
    if (x%2 or x<3):
        print(x)
    else: print("!")

func do_thing(val, name="default"):
    print($"Operation on ${val}, ${name}!\n")
    return val*(val+1)/2;
