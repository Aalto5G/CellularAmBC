#!/usr/bin/python3

from tkinter import *
#from tkinter import Canvas, Frame
import numpy as np

import matplotlib as matplotlib

from matplotlib.backends.backend_tkagg import (
    FigureCanvasTkAgg, NavigationToolbar2Tk)
from matplotlib.colors import Normalize
import matplotlib.cm as cm
import matplotlib.pyplot as plt
import numpy as np
from matplotlib import colors, colorbar

from imageio import imsave

# a subclass of Canvas for dealing with resizing of windows
class ResizingCanvas(Canvas):
  def __init__(self,parent,**kwargs):
    Canvas.__init__(self,parent,**kwargs)
    self.bind("<Configure>", self.on_resize)
    self.height = self.winfo_reqheight()
    self.width = self.winfo_reqwidth()

  def on_resize(self,event):
    # determine the ratio of old width/height to new width/height
    wscale = float(event.width)/self.width
    hscale = float(event.height)/self.height
    self.width = event.width
    self.height = event.height
    # resize the canvas 
    self.config(width=self.width, height=self.height)
    # rescale all the objects tagged with the "all" tag
    self.scale("all",0,0,wscale,hscale)

def rgb(r, g, b):
   return "#%s%s%s" % tuple([hex(c)[2:].rjust(2, "0")
      for c in (r, g, b)])


class main():
  def __init__(self):
  
    
    self.master = Tk()
    self.master.wm_title("space invader")
    
    self.f = Frame(self.master)
    self.f.pack(fill=BOTH, expand=YES)
    
    self.w = ResizingCanvas(self.f,width=1000, height=300, bg="white", highlightthickness=0)
    self.w.pack(fill=BOTH, expand=YES)
    
    self.f2 = Frame(self.master)
    self.f2.pack(fill=BOTH, expand=YES)
    self.w2 = ResizingCanvas(self.f2,width=1000, height=800, bg="white", highlightthickness=0)
    self.w2.pack(fill=BOTH, expand=YES)
    
    # Iterate through the color and fill the rectangle with colors(r,g,0)
    #for x in range(0, 256):
    #  r = x * 2 if x < 128 else 255
    #  g = 255 if x < 128 else 255 - (x - 128) * 2
    #  self.w.create_rectangle(x * 2, 350, x * 2 + 2, 400, fill=rgb(r, g, 0), outline=rgb(r, g, 0))

  
    
    

# 11*8
# -x------x-
# --xxxxxx--
# -xx-xxx-x-
# xx--xx--xx
# xxxxxxxxxx
# --x----x--
# -x------x-
# x--------x
 
  def draw_space_invader(self):
    
    #bar = np.linspace(-20,20,41).reshape(-1,1)
    #bar
    #bar = matplotlib.colors.Normalize(vmin=-20, vmax=20)
    #bar = bar.repeat(20,1)  
    
    #b = np.linspace(-20,20,41)
    #n = colors.BoundaryNorm(bar,len(bar))
    
    #c = cm.jet(bar)[:,:,:3]
    
    #c = cm.jet(bar)[:,:,:3]
    
    #imsave("colorbar.png", (colorbar * 255).astype(np.uint8))
    
    #fig,ax = plt.subplots(1,1) 
    
    #cb = colorbar.ColorbarBase(ax, cmap="jet", norm=n, spacing='proportional', ticks=None,
    #                               boundaries=b, format='%1i', orientation=u'horizontal')
    #cb = colorbar.ColorbarBase(ax, cmap="jet", norm=n, spacing='proportional', ticks=None,format='%1i', orientation=u'horizontal')
    #self.w.add_widget(colorbar)
    #plt.colorbar(c)
    
    #fig,ax = plt.subplots(111)
    #fig.colorbar(cm.ScalarMappable(norm=colors.Normalize(),cmap="jet"),ax=ax)
    #fig.show()
    
    
    # example for the space invader description in bits 10x8 bitds long vector
    inDataV = np.array([0,0,0,1,1,0,0,1,1,0,1,1,1,0,1,0,0,1,1,0,1,1,0,0,0,1,0,0,1,0,0,0,0,1,1,1,1,0,0,0,0,1,1,1,1,0,0,0,0,1,1,0,1,0,0,0,0,1,0,0,1,1,0,0,1,0,1,1,1,0,1,0,0,0,0,1,1,0,0,1])
    
    for inx1 in inDataV.reshape(8,int(len(inDataV)/8)):
       print(inx1)
       for inx2 in inx1:
         print(inx2)
    
    #p = ((1, 1))
    tuc = 0
    tur = 0
    tlc = 100
    tlr = 100

    shiftsVectorH = (0,1,2,3,4,5,6,7,8,9) 
    shiftsVector = (0,1, 2, 3, 4, 5, 6, 7) 
    valueVector = ((0,0,0,1,1,0,0,1),(1,0,1,1,1,0,1,0),(0,1,1,0,1,1,0,0),(0,1,0,0,1,0,0,0),(0,1,1,1,1,0,0,0),(0,1,1,1,1,0,0,0),(0,1,1,0,1,0,0,0),(0,1,0,0,1,1,0,0),(1,0,1,1,1,0,1,0),(0,0,0,1,1,0,0,1))
    
    for v,shiftH in zip(valueVector,shiftsVectorH):
      print(v)
      print(shiftH)
      for val, shiftV in zip(v,shiftsVector):
        print(val,shiftV)
        uc = (tuc+shiftV)*100
        lr = (tlr+shiftV)*100
        ur = (tur + shiftH)*100
        lc = (tlc + shiftH)*100
        if val==0:
          #print(val)
          self.w2.create_rectangle(ur, uc, lr, lc, fill="white",outline="white")
        if val==1:
          #print(val)
          self.w2.create_rectangle(ur, uc, lr, lc, fill="black",outline="black")
        
    # tag all of the drawn widgets
    self.w2.addtag_all("all")
    self.master.mainloop()
    
    
    #for shifts in shiftsVector:
      #uc = tuc
      #lr = tlr
      #ur = (tur + shifts)*100
      #lc = (tlc + shifts)*100
      # add some widgets to the canvas
      #self.w.create_line(0, 0, 200, 100)
      #self.w.create_line(0, 100, 200, 0, fill="red", dash=(4, 4))
      #self.w.create_rectangle(50, 25, 150, 75, fill="blue")
      #self.w.create_rectangle(uc, ur, lc, lr, fill="blue")
      #self.w.create_rectangle(ur, uc, lr, lc, fill="black")
       
      # tag all of the drawn widgets
      #self.w.addtag_all("all")
      #self.master.mainloop()
     


#master = tkinter.Tk()
# master = Tk()

def get_x_and_y(event):
  global last_x, last_y
  last_x,last_y = event.x, event.y
  
#def draw_space_invader(event):
#  global last_x, last_y 

#  ul = 0
#  ur = 0
#  ll = 100
#  lr = 100

#  w.create_rectangle(ul, ur, ll, lr, fill="blue", outline = 'blue')

#  last_x,last_y = event.x, event.y
  

    
#WidthSize = 250
#HeigthSize = 200
  
#w = tkinter.Canvas(master, width=WidthSize, height=HeigthSize)



#w.create_rectangle(0, 0, 100, 100, fill="blue", outline = 'blue')
# w.create_rectangle(50, 50, 100, 100, fill="red", outline = 'blue') 

#w.pack()

if __name__ == '__main__':

  m = main()
  
  m.draw_space_invader()
#    master.mainloop()


# master.mainloop()

