SCREEN_WIDTH = 128;
SCREEN_HEIGHT = 128;

view_distance = 50.0;
step_scale = 1.0;

dz = 1.0;
z = 0.5;

cam_pos_x = 32;
cam_pos_y = 64;


angle = 0
fov = pi/2
sinphi = sin(angle)
cosphi = cos(angle)

x_coords = [];
y_coords = [];

heightmap_x = -32;
heightmap_y = 0;
heightmap_width = 128;
heightmap_height = 128;


while (z < view_distance)
    hypot = z / cos(angle-fov/2)
    pleft_x = hypot * cos(angle-fov/2);
    pleft_y = hypot * sin(angle-fov/2);

    pright_x = hypot * cos(angle+fov/2);
    pright_y = hypot * sin(angle+fov/2);

    dx = (pright_x - pleft_x) / SCREEN_WIDTH;
    dy = (pright_y - pleft_y) / SCREEN_WIDTH;

    pleft_x = pleft_x + cam_pos_x;
    pleft_y = pleft_y + cam_pos_y;

    for i = 1:SCREEN_WIDTH
        x = fix(pleft_x);
        y = fix(pleft_y);

        if ((x >= heightmap_x && x < heightmap_x+heightmap_width) && (y >= heightmap_y && y < heightmap_y+heightmap_height))
            x_coords(end+1) = x;
            y_coords(end+1) = y;
        endif

        pleft_x = pleft_x + dx;
        pleft_y = pleft_y + dy;
    endfor

    z = z + dz;
    dz = dz + 0.05;
endwhile

scatter(x_coords, y_coords);
rectangle('Position', [heightmap_x heightmap_y heightmap_width heightmap_height])
axis equal;
axis([-60 150 -60 150]);
