


import engine_draw

class GradientBackground:
    @classmethod
    @micropython.viper
    def render(cls,frame:int):
        buf = ptr16(engine_draw.back_fb_data())
        cols = ptr16(cls.colors)
        y7:int = 0
        yP:int = 0
        col1:int = 0
        col2:int = 0

        for py in range(0, 128):
            y7 = (py<<7)
            for px in range(0, 128):
                buf[px|y7] =cols[(((127-px))+frame-(py>>1))&511]

    @classmethod
    @micropython.native
    def hsv_to_rgb( cls, h, s, v) -> tuple:
        if s:
            if h == 1.0: h = 0.0
            i = int(h*6.0); f = h*6.0 - i
            
            w = v * (1.0 - s)
            q = v * (1.0 - s * f)
            t = v * (1.0 - s * (1.0 - f))
            
            if i==0: return (v, t, w, 1)
            if i==1: return (q, v, w, 1)
            if i==2: return (w, v, t, 1)
            if i==3: return (w, q, v, 1)
            if i==4: return (t, w, v, 1)
            if i==5: return (v, w, q, 1)
        else: return (v, v, v, 1)

        
    @classmethod
    @micropython.native
    def prepareColor(cls):
        colors = [0 for x in range(1024)]
        for x in range(0, 512):
            color = cls.hsv_to_rgb(x/512.0 ,0.3,1)
            c1= int(color[0]*31)
            c2= int(color[1]*63)
            c3= int(color[2]*31)
            colors[x*2+1] =  (c1<<3) | (c2>>3)
            colors[x*2] = c3 |  (c2<<5)
        cls.colors = bytearray(colors)
