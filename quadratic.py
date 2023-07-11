x1 = 10
y1 = 0
m1 = 1

x2 = 50
y2 = 20
m2 = 0

'''
y = ax^2 + bx + c
dy/dx = 2ax + b
for eq 1, dy/dx = m1 = 2*a*x1 + b
for eq 2, dy/dx = m2 = 2*a*x2 + b
we want to solve these equations simultaneously

solve by making eq 1 = eq 2:
0 = 2*a*x1 + b - m1
0 = 2*a*x2 + b - m2
-> 2*a*x1 + b - m1 = 2*a*x2 + b - m2

simplify:
(2*a*x1) - (2*a*x2) = -m2 + m1
x1, x2, m1, m2 are known so this is solveable
a(2*x1) - a(2*x2) = -m2 + m1
(2*x1 - 2*x2) * a = -m2 + m1
a = (-m2 + m1) / (2*x1 - 2*x2)

then to find b:
m1 = 2*a*x1 + b
b = m1 - 2*a*x1

and to find c:
c = y - (ax^2 + bx)
'''
a = (-m2 + m1) / (2*x1 - 2*x2)
b = m1 - 2*a*x1
c = y1 - ((a * (x1 * x1)) + (b * x1))

'''
now find given x2,y2 and not m2
y = ax^2 + bx + c
dy/dx = 2ax + b
m1 = 2a(x1) + b
b = m1 - 2a(x1)
y1 = a * (x1^2) + (m1 - 2a(x1)) * x1 + c
y1 = a(x1^2 - 2*x1) + m1*x1 + c
0 = a(x1^2 - 2*x1) + m1*x1 + c - y1

y2 = a * (x2^2) + (m1 - 2a(x1)) * x2 + c
y2 = a * (x2^2 - 2*x1*x2) + x2*m1 + c
0 = a * (x2^2 - 2*x1*x2) + x2*m1 + c - y2

a(x1^2 - 2*x1) + m1*x1 + c - y1 = a * (x2^2 - 2*x1*x2) + x2*m1 + c - y2
a((x1^2 - 2*x1) - (x2^2 - 2*x1*x2)) = x2*m1 - m1*x1 + c - c - y2 + y1
a = (x2*m1 - m1*x1 - y2 + y1) / ((x1^2 - 2*x1) - (x2^2 - 2*x1*x2))
b = m1 - 2a(x1)
y1 = a * (x1^2) + b * x1 + c
c = y1 - (a * (x1^2) + b * x1)
'''

a = (x2*m1 - m1*x1 - y2 + y1) / ((x1*x1 - 2*x1) - (x2*x2 - 2*x1*x2))
b = m1 - 2*a*x1
c = y1 - (a * (x1*x1) + b * x1)

print(f"a: {a} b: {b} c: {c}")