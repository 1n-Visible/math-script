
int a=1
int b[]=[4, 1.125, 23i, -2];
str$ s="Hello, neonscript world!"
char newl=$'\t'

for (int x in b):
    if (x&1 or x>2)
        print(x)
    else print("!")

func do_thing(float16 val, str name="default") -> float16:
    print($"Operation on ${val}, ${name}!\n")
    return val*(val+1)/2;
