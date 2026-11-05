import math
import turtle

# Constants
SCREEN_W = 1900
SCREEN_H = 1000
MIN_CELL = 16

# Setip
t = turtle.Turtle()
t.speed(0) # 1:slowest, 3:slow, 5:normal, 10:fast, 0:fastest
screen = t.getscreen()
screen.setup(width=SCREEN_W, height=SCREEN_H)
t.penup()

# Utility functions
def teleport(x, y):
  t.setx(x - SCREEN_W / 2 + 30)
  t.sety(y)

def circle(centerx, centery, rad, steps=None):
  teleport(centerx, centery - rad)
  t.pendown()
  t.circle(rad, steps=int(steps) if steps else None)
  t.penup()

def line(startx, starty, angle, length):
  teleport(startx, starty)
  t.seth(angle)
  t.pendown()
  t.forward(length)
  t.penup()
  t.seth(0)

g_sample_count = 0

# Sampling rays
def ray(angle, cell_size, far, near):
    stepx = math.cos(math.radians(angle)) * MIN_CELL
    stepy = math.sin(math.radians(angle)) * MIN_CELL
    posx = stepx * far
    posy = stepy * far
    i = far
    while i >= near:
        circle(posx, posy, cell_size * MIN_CELL * 0.5, steps = cell_size * 8)
        global g_sample_count
        g_sample_count += 1
        posx -= stepx * cell_size
        posy -= stepy * cell_size
        i -= cell_size

# Grid
if False:
    t.pencolor(0.9, 0.9, 0.9)
    for y in range(-496, 497, MIN_CELL):
        line(0, y, 0, 1900)
    for x in range(0, 1900, MIN_CELL):
        line(x, -SCREEN_H / 2, 90, 1000)
    t.pencolor(0.0, 0.0, 0.0)

# Circular sector
line(0, 0, 22.5, 1280)
line(0, 0, -22.5, 1280)

NEW = True
if NEW:
    #                A,  B,  C,  D,  E,  F,  G
    RAY_CHILDREN = [ 2,  4,  8,  1, 16,  1,  0]
    RAY_CELL     = [ 1,  1,  1,  1,  2,  2,  4]
    RAY_FAR      = [ 1,  2,  5, 10, 21, 43, 78]
    RAY_NEAR     = [ 1,  2,  3,  6, 11, 22, 44]
else:
    RAY_CHILDREN = [ 2,  4,  8,  16, 32, 0]
    RAY_CELL     = [ 1,  1,  1,  1,  1,  1]
    RAY_FAR      = [ 1,  2,  5, 10, 20, 40]
    RAY_NEAR     = [ 1,  2,  3,  6, 11, 21]
RAY_MAX_RECURSION = len(RAY_CHILDREN)

def tree(depth, angle):
    if RAY_CHILDREN[depth] >= 2:
        tree(depth + 1, angle + 22.5 / RAY_CHILDREN[depth])
        tree(depth + 1, angle - 22.5 / RAY_CHILDREN[depth])
    elif RAY_CHILDREN[depth] == 1:
        tree(depth + 1, angle)
    ray(angle, RAY_CELL[depth], RAY_FAR[depth], RAY_NEAR[depth])

tree(depth=0, angle=0)

teleport(-50, 0)

print(f"Total samples: {g_sample_count}")

screen.exitonclick()