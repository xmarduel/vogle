
import math
import vogle


RADIUS = 10.0
SPHERE =	1

#
#  makesphere
# 
# 	make a sphere object
# 
def makesphere():
    '''
    '''
    vogle.makeobj(SPHERE)

    #
    # create the latitudinal rings
    #
    for i in range(0, 180, 20):
        vogle.pushmatrix()
        vogle.rotate(i, 'y')
        vogle.circle(0.0, 0.0, RADIUS)
        vogle.popmatrix()
		
    #
    # create the longitudinal rings
    #
    vogle.pushmatrix()
    vogle.rotate(90.0, 'x')
    for a in range(-90, 90, 20):
        r = RADIUS * math.cos(a * vogle.PI / 180.0)
        z = RADIUS * math.sin(a * vogle.PI / 180.0)
        vogle.pushmatrix()
        vogle.translate(0.0, 0.0, -z)
        vogle.circle(0.0, 0.0, r)
        vogle.popmatrix();

    vogle.popmatrix()

    vogle.closeobj()


#
# a demonstration of objects
#
def main():
    '''
    '''
    vogle.vinit("X11")

    vogle.vsetflush(1)

    #
    # set up our viewing transformation
    #
    vogle.perspective(90.0, 1.0, 0.001, 500.0)
    vogle.lookat(13.0, 13.0, 8.0, 0.0, 0.0, 0.0, 0.0)

    vogle.color(vogle.BLACK)
    vogle.clear()

    #
    # Call a routine to make the sphere object
    #
    makesphere()

    #
    # Now draw the sphere object scaled down. We use the pushmatrix
    # and the popmatrix to preserve the transformation matrix so
    # that only this sphere is drawn scaled.
    #
    vogle.color(vogle.CYAN)

    vogle.pushmatrix()
    vogle.scale(0.5, 0.5, 0.5)
    vogle.callobj(SPHERE)
    vogle.popmatrix()

    #
    # now we draw the same sphere translated, with a different
    # scale and color.
    #
    vogle.color(vogle.WHITE)

    vogle.pushmatrix()
    vogle.translate(0.0, -1.4 * RADIUS, 1.4 * RADIUS)
    vogle.scale(0.3, 0.3, 0.3)
    vogle.callobj(SPHERE)
    vogle.popmatrix()

    #
    # and maybe a few more times....
    #
    vogle.color(vogle.RED)

    vogle.pushmatrix()
    vogle.translate(0.0, RADIUS, 0.7 * RADIUS)
    vogle.scale(0.2, 0.2, 0.2)
    vogle.callobj(SPHERE)
    vogle.popmatrix()


    vogle.color(vogle.GREEN)

    vogle.pushmatrix()
    vogle.translate(0.0, 1.5 * RADIUS, -RADIUS)
    vogle.scale(0.15, 0.15, 0.15)
    vogle.callobj(SPHERE)
    vogle.popmatrix()


    vogle.color(vogle.YELLOW)

    vogle.pushmatrix()
    vogle.translate(0.0, -RADIUS, -RADIUS)
    vogle.scale(0.12, 0.12, 0.12)
    vogle.callobj(SPHERE)
    vogle.popmatrix()

    vogle.color(vogle.BLUE)

    vogle.pushmatrix()
    vogle.translate(0.0, -2.0*RADIUS, -RADIUS)
    vogle.scale(0.3, 0.3, 0.3)
    vogle.callobj(SPHERE)
    vogle.popmatrix()


    vogle.font("times.rb")
    vogle.ortho2(0.0, 1.0, 0.0, 1.0)
    vogle.centertext(1)
    vogle.textsize(0.08, 0.15)
    vogle.move2(0.8, 0.5)
    vogle.textang(-90.0)
    vogle.drawstr("I'm very ordinary!")

    vogle.getkey()

    vogle.vexit()


if __name__ == '__main__':
    main()

