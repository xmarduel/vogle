
from sys   import *
from math  import *
import os

import vogle

USE_MOUSE = True

ESC =	27

TRANS		        = 0.06
POLYFILL         = 1

xyz = [
   [ 0.000000,  0.000000,  1.000000],
   [ 0.809017, -0.587785,  0.500000],
   [ 0.809017,  0.587785,  0.500000],
   [-0.309017,  0.951057,  0.500000],
   [-1.000000,  0.000000,  0.500000],
   [-0.309017, -0.951057,  0.500000],
   [ 1.000000,  0.000000, -0.500000],
   [ 0.309017,  0.951057, -0.500000],
   [-0.809017,  0.587785, -0.500000],
   [-0.809017, -0.587785, -0.500000],
   [ 0.309017, -0.951057, -0.500000],
   [ 0.000000,  0.000000, -1.000000]
]

ncon = [
   [ 1,  2,  3],
   [ 1,  3,  4],
   [ 1,  4,  5],
   [ 1,  5,  6],
   [ 1,  6,  2],
   [ 2,  7,  3],
   [ 3,  8,  4],
   [ 4,  9,  5],
   [ 5, 10,  6],
   [ 6, 11,  2],
   [ 7,  8,  3],
   [ 8,  9,  4],
   [ 9, 10,  5],
   [10, 11,  6],
   [11,  7,  2],
   [ 7, 12,  8],
   [ 8, 12,  9],
   [ 9, 12, 10],
   [10, 12, 11],
   [11, 12,  7]
]

#

def drawshape():
    '''
    '''
    for i in range(20):
        
        vogle.color(i+1)
      
        vogle.polyfill(POLYFILL)
        
        vogle.makepoly()
        vogle.move(xyz[ncon[i][0]-1][0], xyz[ncon[i][0]-1][1], xyz[ncon[i][0]-1][2])
        vogle.draw(xyz[ncon[i][1]-1][0], xyz[ncon[i][1]-1][1], xyz[ncon[i][1]-1][2])
        vogle.draw(xyz[ncon[i][2]-1][0], xyz[ncon[i][2]-1][1], xyz[ncon[i][2]-1][2])
        vogle.closepoly()

#
#

def main():
    '''
    '''
    global POLYFILL
    
    DRAG = 0
    EVENT_X = 0
    EVENT_Y = 0

    XROT = 0.0
    YROT = 0.0

    XTRANS = 0.0
    YTRANS = 0.0
    ZTRANS = 0.0

    SCALE_FACTOR = 1.0

    SIZE = 300
   
    vogle.prefposition(50, 50)
    vogle.prefsize(SIZE, SIZE)

    w1 = vogle.winopen("X11", "Licosa")
    nplanes = vogle.getdepth()
    vogle.backbuffer()

    vogle.window(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0)
    vogle.lookat(0.0, 0.0, 2.1, 0.0, 0.0, 0.0, 0.0)

    vogle.textsize(0.15, 0.3)

    vogle.backface(1)

    #
    # Green color ramp...
    #
    for i in range(1,21):
        vogle.mapcolor(i, 20, 20 + i * 10 , 20)
   
    if vogle.backbuffer() < 0:
        vogle.vexit()
        print "licosa: device doesn't support double buffering.\n"
        exit(0)


    while (1):
   
        if USE_MOUSE:
            
            psx  = vogle.new_doublep()
            psy  = vogle.new_doublep()
            
            but = vogle.slocator(psx, psy)

            sx = vogle.doublep_value(psx)
            sy = vogle.doublep_value(psy)
            
            if (but==1):
                vogle.rotate(90.0 * sx, 'x')
            if (but==2):
                vogle.rotate(190.0 * sx, 'y')
            if (but==4):
                vogle.rotate(190.0 * sx, 'z')
            
            vogle.pushmatrix()
            vogle.rotate(90 * sx, 'y')
            vogle.rotate(90 * sy, 'z')
   
            vogle.color(vogle.BLACK)
            vogle.clear()
            drawshape()
            vogle.popmatrix()
            vogle.swapbuffers()

            key = vogle.checkkey()
            
            if key in [ord('q'), ESC] :	# Stop the program
                vogle.vexit()
                exit(0)
            if key == ord('e'):	# Enlarge
                vogle.scale(1.1, 1.1, 1.1)
            if key == ord('r'):	# Reduce
                vogle.scale(0.9, 0.9, 0.9)
            if key == ord('w'):	# WireFrame
                print "WireFrame"
                POLYFILL = 0
                #vogle.polyfill(0)
            if key == ord('s'):	# Solid
                print "Solid"
                POLYFILL = 1
                 #vogle.polyfill(1)
            if key == ord('g'):
                # gif
                print "make licosa.gif"
                w_gif = vogle.winopen("gif", "licosa.gif")
                
                for i in range(1,21):
                    vogle.mapcolor(i, 20, 20 + i * 10 , 20)
                vogle.pushmatrix()

                vogle.rotate(90 * sx, 'y')
                vogle.rotate(90 * sy, 'z')

                vogle.scale(SCALE_FACTOR,SCALE_FACTOR,SCALE_FACTOR)
                vogle.translate(XTRANS,-YTRANS,ZTRANS)

                vogle.color(vogle.BLACK)
                vogle.clear()
      
                drawshape()
      
                # if (nplanes == 1):
                #     drawshape(0)
      
                vogle.popmatrix()
            
                vogle.swapbuffers()
                vogle.winclose(w_gif)
                vogle.winset(w1)
           
        else:

            vogle.pushmatrix()

            vogle.rotate( -XROT , 'x')
            vogle.rotate(  YROT , 'y')

            vogle.scale(SCALE_FACTOR,SCALE_FACTOR,SCALE_FACTOR)
            vogle.translate(XTRANS,-YTRANS,ZTRANS)

            vogle.color(BLACK)
            vogle.clear()
      
            drawshape()
      
            # if (nplanes == 1):
            #     drawshape(0)
      
            vogle.popmatrix()
            
            vogle.swapbuffers()


            vev = vogle.new_Vevent()
            
            w = vogle.vgetevent(vev, 1)

            #if vev.type == VMOTION:
            if vogle.Vevent_type_get(vev) == VMOTION:
      
                if DRAG == 1 :
                
                    if vogle.Vevent_data_get(vev) == 1:
                    
                        DELTA_X = vogle.Vevent_x_get(vev) - EVENT_X
                        DELTA_Y = vogle.Vevent_y_get(vev) - EVENT_Y

                        print " In VMOTION , (DELTA_X,DELTA_Y) = (%d,%d) )\n" % (DELTA_X, DELTA_Y)

                        EVENT_X = vogle.Vevent_x_get(vev)
                        EVENT_Y = vogle.Vevent_y_get(vev)

                        print " In VMOTION , (EVENT_X,EVENT_Y) = (%d,%d) )\n"  % (EVENT_X, EVENT_Y)

                        rx = 2 * DELTA_X / (float)(SIZE)
                        ry = 2 * DELTA_Y / (float)(SIZE)

                        print " In VMOTION , (rx,ry) = (%6.3lf , %6.3lf) \n"  % (rx,ry)

                        XROT  = vogle.fmod( XROT + 180*ry , 360 )
                        YROT  = vogle.fmod( YROT + 180*rx , 360 )

                        print " In VMOTION , (XROT,YROT) = (%6.3lf , %6.3lf) \n"  % (XROT, YROT)

                    if vogle.Vevent_data_get(vev) == 2:
                   
                        DELTA_X = vogle.Vevent_x_get(vev) - EVENT_X
                        DELTA_Y = Vvogle.event_y_get(vev) - EVENT_Y

                        print " In VMOTION , (DELTA_X,DELTA_Y) = (%d,%d) )\n"  %( DELTA_X,DELTA_Y)

                        EVENT_X = vogle.Vevent_x_get(vev)
                        EVENT_Y = vogle.Vevent_y_get(vev)

                        print " In VMOTION , (EVENT_X,EVENT_Y) = (%d,%d) )\n"  % (EVENT_X,EVENT_Y)

                        rx = 2000 * DELTA_X / (float)(SIZE)
                        ry = 2000 * DELTA_Y / (float)(SIZE)

                        print " In VMOTION , (rx,ry) = (%6.3lf , %6.3lf) \n" % (rx, ry)

                        XTRANS  -= rx
                        YTRANS  -= ry
                        ZTRANS  = 0.0
      
            if vogle.Vevent_type_get(vev) == vogle.VRESIZE:
      
                vogle.winset(w)
                vogle.calcviewport()
      
            if vogle.Vevent_type_get(vev) == vogle.VBUTTONPRESS:
         
                DRAG = 1
                #
                # print " In VBUTTONPRESS , (x.y) = (%d , %d)\n" %(vev.x,vev.y)
                #
                EVENT_X = Vevent_x_get(vev)
                EVENT_Y = Vevent_y_get(vev)
      
            if vogle.Vevent_type_get(vev) == vogle.VBUTTONRELEASE:
       
                DRAG = 0
      
      
            if vogle.Vevent_type_get(vev) == vogle.VKEYPRESS:
      
                if vogle.Vevent_data_get(vev) == ord('s'):
                    print "Sild2"
                    POLYFILL = 1
                    #vogle.polyfill(1)
                if vogle.Vevent_data_get(vev) == ord('w'):
                    print "WireFrame2"
                    POLYFILL = 0
                    pass
                    #vogle.polyfill(0)
                if vogle.Vevent_data_get(vev) in [ ESC, ord('q')]:
                   vogle.vexit()
   
    vogle.vexit()

    return(0)

#
#
if __name__ == '__main__':
    main()
#
