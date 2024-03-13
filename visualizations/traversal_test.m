width = 15;
height = 5;

half_width = width * 0.5;
half_height = height * 0.5;

#rotation = pi/3;
rotation = 0;

x_traversel_cos = cos(rotation) * half_width;
x_traversel_sin = sin(rotation) * half_width;

y_traversel_cos = cos(rotation + (pi/2)) * half_height;
y_traversel_sin = sin(rotation + (pi/2)) * half_height;

# top-left
x0 = -x_traversel_cos + y_traversel_cos
y0 =  x_traversel_sin - y_traversel_sin

# top-right
x1 =  x_traversel_cos + y_traversel_cos
y1 = -x_traversel_sin - y_traversel_sin

# bottom-right
x2 =  x_traversel_cos - y_traversel_cos
y2 = -x_traversel_sin + y_traversel_sin

# bottom-left
x3 = -x_traversel_cos - y_traversel_cos
y3 =  x_traversel_sin + y_traversel_sin


x_zero = 0
y_zero = 0


x_axis = [x_zero, x0, x1, x2, x3];
y_axis = [y_zero, y0, y1, y2, y3];

scatter(x_axis, y_axis);
axis equal;
axis([-10 10 -10 10]);
set (gca (), "ydir", "reverse")
