#need to import os and sys for native code compilation
#ignore when running with py interpreter
import os
import sys
import numpy as np
import pygame as pg
import pygame.freetype
import math
import imageio

"""GLOBAL VARIABLES"""
BLOCK_WIDTH = 50
BLOCK_LENGTH = 50 #height is handled differently
BLOCK_HEIGHT = 50
BLANK_COLOR = (20,20,20)
DEPTH_OF_FIELD = 15


def resource_path(relative_path):
    """
    This function is used only for compiling to native code using pyinstaller.
    Ignore if running with the python interpreter.
    """
    try:
    # PyInstaller creates a temp folder and stores path in _MEIPASS
        base_path = sys._MEIPASS
    except Exception:
        base_path = os.path.abspath(".")

    return os.path.join(base_path, relative_path)

class FastMath:
    """Class for fast evaluation of trig functions.
    Takes a stepsize parameter for the lookup tables"""
    
    def __init__(self, step):
        self.step = step
        self.sineLookup = np.sin(np.arange(0,2*np.pi,self.step))
        self.cosLookup = np.cos(np.arange(0,2*np.pi,self.step))
        self.tanLookup = np.tan(np.arange(0,2*np.pi,self.step))
        
    def sin(self, ang):
        """Employs a numpy array as a lookup table.
        Accepts angle in radians.
        Returns a small angle approximation near zero angle"""
        
        ind = int(ang/self.step)
        if ind != 0:
            return self.sineLookup[ind]
        else:
            return ang
            
    def cos(self, ang):
        """Employs a numpy array as a lookup table.
        Accepts angle in radians.
        Returns a small angle approximation near zero angle"""
        
        ind = int(ang/self.step)
        if ind != 0:
            return self.cosLookup[ind]
        else:
            return 1 - (ang**2)/2
            
    def tan(self, ang):
        """Employs a numpy array as a lookup table.
        Accepts angle in radians.
        Returns a small angle approximation near zero angle"""
        
        ind = int(ang/self.step)
        if ind != 0:
            return self.tanLookup[ind]
        else:
            return ang
            

fastMath = FastMath(0.001)

class Player:
    """A Player class that has all functions regarding the player
    Attributes:
        x         : x position of the player (in pixels)
        y         : y position of the player (in pixels)
        ang       : Angle of the direction in which the player is looking, 0 at right, positive when anticlock
        playerDim : Dimensions of the player's square on the 2D map
    playRect is the derived attribute representing the rectangle of the player
    """
    def __init__(self, x, y, ang, playerDim):
        self.x = x
        self.y = y
        self.ang = ang
        self.playerDim = playerDim
        self.playRect = pg.Rect(x - playerDim//2, y - playerDim//2, playerDim, playerDim)
        
    def draw(self, screen, mapRect, gMap):
        """Draws on the passed screen surface the player's square,
        and a line showing the direction in which they are looking"""
        scaleX = mapRect.width/(gMap.blockDims[0]*gMap.mapDims[1])
        scaleY = mapRect.height/(gMap.blockDims[1]*gMap.mapDims[0])
        playerDim = gMap.blockDims[0]
        pg.draw.rect(screen,(250,255,0),
                    ( int(mapRect.left + (self.x - playerDim//2)*scaleX), 
                    int(mapRect.top + (self.y - playerDim//2)*scaleY), 
                    int(playerDim*scaleX), int(playerDim*scaleY) ) 
                    )
        playerDim*=2            
        pg.draw.aaline(screen, (255,0,0),
                    ( (mapRect.left + self.x*scaleX, 
                    mapRect.top + self.y*scaleY) ), 
                    ( int(mapRect.left + (self.x + playerDim*fastMath.cos(self.ang))*scaleX), 
                    int(mapRect.top + (self.y - playerDim*fastMath.sin(self.ang))*scaleY) )
                    )
        
    def move(self, speed, angVel, keys, dt, bounds):
        """
        For player movement. Parameters accepted:
        speed  : walking speed of the player
        angVel : turning speed of the player
        keys   : string list of pressed keys
        dt     : time passed since the last time the in-game clock ticked; used to maintain framerate
        
        The keys do the following actions:
        up, down, left, right keys for walking
        a, s keys for turning left and right respectively
        space for halving the speed of walking and turning
        
        Returns the net movement vector as a list
        """
        
        #to hold pre-movement x, y positions
        x_, y_ = self.x, self.y
        
        #double the motion speed if space is pressed
        if 'space' in keys:
            speed *= 2
            angVel *= 2
        
        for key in keys:
            #Angle is modulo 2*pi
            if key == 'a':
                self.ang = (self.ang + angVel*dt)%(2*math.pi)
            elif key == 's':
                self.ang = (self.ang - angVel*dt)%(2*math.pi)
            else:
                #x and y movements are projections of the net
                #motion along the direction of eyesight
                if key == 'right':
                    dx = fastMath.sin(self.ang)*speed*dt
                    dy = fastMath.cos(self.ang)*speed*dt
                elif key == 'left':
                    dx = -fastMath.sin(self.ang)*speed*dt
                    dy = -fastMath.cos(self.ang)*speed*dt
                elif key == 'up':
                    dx = fastMath.cos(self.ang)*speed*dt
                    dy = -fastMath.sin(self.ang)*speed*dt
                elif key == 'down':
                    dx = -fastMath.cos(self.ang)*speed*dt
                    dy = fastMath.sin(self.ang)*speed*dt
                elif key == 'space':
                    continue
                else:
                    return [0,0]
                
                #motion is constrained to screen dimensions
                self.x = (self.x + dx)%bounds[0]; self.y = (self.y + dy)%bounds[1]
                
        #self.playRect.move_ip(int(round(self.x - x_)), int(round(self.y - y_)))
        return [self.x - x_, self.y - y_]
    
    def tryMove(self, gMap):
        """
        Function is called only after an entity/player has moved.
        Checks if current position is valid, ie it doesn't intersect
        with walls or other entities in the gMap map.
        """
        #the bounds of the entity rectangle
        xl = int((self.x - self.playerDim)/gMap.blockDims[0])
        xh = int((self.x + self.playerDim)/gMap.blockDims[0])
        yl = int((self.y - self.playerDim)/gMap.blockDims[1])
        yh = int((self.y + self.playerDim)/gMap.blockDims[1])
        
        #checking if a wall is inside the rectangle
        for i in range(yl,yh+1):
            for j in range(xl,xh+1):
                if gMap.mapArr[i,j] != 0:
                    return False
        return True
        
    def checkCollision(self, gMap, motion):
        """
            Checks for collisions with other entities in the map.
            If collision is found, the motion is clipped in orthog
            directions and clipped until the motion is valid.
        """
        #check if full motion valid
        if self.tryMove(gMap):
            return
        
        #else check if only vertical motion valid
        self.x -= motion[0]
        if self.tryMove(gMap):
            return
        
        #else check if only horizontal motion valid
        self.y -= motion[1]
        self.x += motion[0]
        if self.tryMove(gMap):
            return
        
        #else this is a fully invalid move, roll back motion
        self.x -= motion[0]
    
class GameMap:
    """A game map class that represents the grid map.
    Attributes:
        mapDims       : Map dimensions by number of blocks
        blockDims     : Block dimensions in pixels
        mapPos        : Position of the 2D map on the screen
        wallColorDict : Dictionary for colors of walls
        mapArr        : numpy array of the map layout.
        mapHLines     : numpy array of the horizontal walls
        mapVLines     : numpy array of the vertical walls
        
        The mapping scheme used by the dictionary is:
            0    for no block
            non0 for a (colored) block
        
    """
    def __init__(self, mapImageName, blockW, blockH, mapL, mapU):
        """
        Parameters:
            mapImageName : name of the image file of the map
            blockW       : block width
            blockH       : block height
            mapL         : map's top-left corner's x position
            mapU         : map's top-left corner's y position
        """
        
        #reading the image into memory as a numpy array
        #the image HAS to be a 24-bit bit map file (.bmp)
        mapImage = imageio.imread(mapImageName)
        
        #setting tuple definitions
        self.blockDims = (blockW, blockH)       #dimension of blocks is by pixel
        self.mapPos = (mapL, mapU) 
        self.mapDims = mapImage.shape[0:2]
        
        #this is the color of blank blocks on the map
        #when wall distance exceeds draw distance, this
        #dark gray color will be drawn
        blankColor = BLANK_COLOR
        
        #this list holds all the colors encountered in the map.
        #Nothing encountered yet, so only filled with blank color
        colorList = [blankColor]
        
        #the wall color dictionary that holds
        #index-color_tuple key-value pairs
        #initial pair is for empty blocks
        self.wallColorDict = {0:blankColor}
        
        #the inverse of the wall color dict that holds
        #color_tuple-index key-value pairs
        indexDict = {blankColor:0}
        
        #next color to be added will have an index 1
        colorInd = 1
        
        #map array holding the indices of each block
        #zero index by default
        self.mapArr = np.zeros(self.mapDims, 'int8')
        
        #building the map array by reading the image
        for i in np.arange(mapImage.shape[0]):
            for j in np.arange(mapImage.shape[1]):
                if (mapImage[i,j,:] != 0).any(): #if pixel isn't black
                    
                    #the color tuple at the given pixel
                    pixelColor = tuple(mapImage[i,j,:])
                    
                    #if this color has not been encountered before
                    if pixelColor not in colorList:
                        
                        #adding newly encountered color into color list
                        colorList.append(pixelColor)
                        
                        #adding key-value pairs into corresp. dictionaries
                        self.wallColorDict[colorInd] = pixelColor
                        indexDict[pixelColor] = colorInd
                        
                        #setting map block to said index
                        self.mapArr[i,j] = colorInd
                        colorInd += 1                                              
                    else:
                        #if color already encountered, update the index at
                        #the given block
                        self.mapArr[i,j] = indexDict.get(pixelColor)
        
        #building the wall arrays using the map array
        self.mapHLines = np.zeros((self.mapDims[0]+1, self.mapDims[1]), 'int8') #stores horiz walls
        self.mapVLines = np.zeros((self.mapDims[0], self.mapDims[1]+1), 'int8') #stores vert walls
        
        for i in np.arange(self.mapArr.shape[0]):
            for j in np.arange(self.mapArr.shape[1]):
                if self.mapArr[i,j] != 0:
                    self.mapHLines[i:i+2,j] = self.mapArr[i,j]
                    self.mapVLines[i,j:j+2] = self.mapArr[i,j]
    
    
    def getMapSurf(self, backColor):
        """Returns a pygame.Surface object with the game map drawn on it.
        Takes the background color as an argument"""
        surf = pg.Surface((self.mapDims[1]*self.blockDims[0], self.mapDims[0]*self.blockDims[1]))
        surf.fill(backColor)
        
        for i in np.arange(self.mapDims[0]):
            for j in np.arange(self.mapDims[1]):
                col = self.wallColorDict.get(self.mapArr[i,j])
                pg.draw.rect(surf, col, (self.blockDims[0]*j, self.blockDims[1]*i, self.blockDims[0], self.blockDims[1]))
                #blocks are drawn one pixel less than their dimension thick to add borders
        
        return surf
    
    def castRays(self, screen, player, spread, depth, wallHeight, wallColorRatio):
        """The main part of the rendering process.
        Arguments:
            screen          : the screen pygame.Surface object
            player          : the Player object present on the map
            spread          : the angular spread of the field of view (in degrees)
            depth           : depth of field in number of blocks
            wallHeight      : height of the blocks in pixels
            wallColorRatio  : ratio of brightness of vert walls over horiz walls, to achieve some lighting effects 
        """
        x, y, ang = player.x, player.y, player.ang
        
        #The screen dimensions
        w, h = screen.get_size()
        
        #The numpy array represents all the rays cast by the player in the map
        castRange = math.radians(spread)
        rayCount = w        
        rAngs = np.linspace(ang+castRange,ang-castRange,rayCount)
        
        screenDist = w/(2*fastMath.tan(castRange))
        #This index is used along with rayCount to check if all rays have been drawn
        index = 0
        #rayCount = rAngs.shape[0]
        
        
        
        #The horizontal offset at which the 3D scene is drawn
        #If the map is to be drawn, rays lines are drawn on it and the 3D scene is rendered with an offset
        #No map is not to be drawn, the 3D scene is drawn without any offset
        
        #The width of the rectangle that is rendered by casting one ray
        rayWid = round(w / rayCount)
        
        #The point on the horizon at which the wall section rendered by a ray is centered
        #Increases as a new ray is to be cast
        rayCenterx = round(rayWid/2)
        
        #casting all the rays
        for rayAng in rAngs:
        
            #finding the ray angle modulo 2pi
            rAng = rayAng%(2*math.pi)
            
            #since this value is used a lot
            tanAng = fastMath.tan(rAng)
            
            #if ray angles are perfectly vertical or horizontal, NaN errors can occur.
            #Hence if such angles are encountered, the ray is not casted
            checkRay = True
            
            #first, checking for horizontal wall collisions            
            if rAng > math.pi:                 #looking down
                #projecting the ray onto the vertical grid line
                hRayY = y - y%self.blockDims[1] + self.blockDims[1]
                #calculating the point of intersection with nearest horiz. wall
                hRayX = x - (hRayY - y)/tanAng
                
                #calculating the offsets for further ray casting
                h_yOffs = self.blockDims[1]
                h_xOffs = -h_yOffs/tanAng
                
            elif rAng < math.pi and rAng != 0: #looking up
                hRayY = y - y%self.blockDims[1]
                hRayX = x + (y - hRayY)/tanAng
                h_yOffs = -self.blockDims[1]
                h_xOffs = -h_yOffs/tanAng
            else:
                #if the ray is perfectly left or right, dont' cast it,
                #as it will never meet a horizontal wall
                hRayX = x
                hRayY = y
                checkRay = False
            
            if checkRay:
                #casting until depth of field is reached
                for dof in range(depth):
                    #extracting the map indices from ray positions
                    hmapX, hmapY = int(hRayX/self.blockDims[0]), int(hRayY/self.blockDims[1])
                    
                    #if the ray hit a horiz. wall
                    if hmapX in range(0, self.mapDims[1]) and hmapY in range(0, self.mapDims[0]) and self.mapHLines[hmapY,hmapX] != 0:
                        break
                    #if not, keep going
                    else:
                        hRayX += h_xOffs
                        hRayY += h_yOffs
            
            #resetting checkray to check for vertical walls
            checkRay = True
            
            #then, checking for vertical wall collisions
            if rAng > math.pi/2 and rAng < (3*math.pi)/2:   #looking left
                #projecting the ray onto a horizontal grid line
                vRayX = x - x%self.blockDims[0]
                #calculating the point of intersection with the nearest vert. wall
                vRayY = y + (x - vRayX)*tanAng
                
                #calculating the offsets for further ray casting
                v_xOffs = -self.blockDims[0]
                v_yOffs = -v_xOffs*tanAng
                
            elif rAng > (3*math.pi)/2 or rAng < math.pi*2:  #looking right
                vRayX = x - x%self.blockDims[0] + self.blockDims[0]
                vRayY = y - (vRayX - x)*tanAng
                v_xOffs = self.blockDims[0]
                v_yOffs = -v_xOffs*tanAng
            else:
                #if the ray is perfectly up or down, don't cast it,
                #as it will never meet a vertical wall
                vRayX = x
                vRayY = y
                checkRay = False
                
            if checkRay:
                #casting until depth of field is reached
                for dof in range(depth):
                    #extracting the map indices from ray positions
                    vmapX, vmapY = int(vRayX/self.blockDims[0]), int(vRayY/self.blockDims[1])
                    
                    #if the ray hit a vert. wall
                    if vmapX in range(0, self.mapDims[1]) and vmapY in range(0, self.mapDims[0]) and self.mapVLines[vmapY,vmapX] != 0:
                        break
                    #if not, keep going
                    else:
                        vRayX += v_xOffs
                        vRayY += v_yOffs
            
            #finding which ray travelled the least distance
            #used to be done with pythagoras thm, now with faster cos call
            vDist = abs((x - vRayX)/fastMath.cos(rAng))
            hDist = abs((x - hRayX)/fastMath.cos(rAng))
            
            if vDist >= hDist:
                #i.e. the horiz. wall ray has to be considered
                
                #The distance travelled by the ray when it hits a wall
                finalDist = hDist
                
                #The indices of the wall at which the ray hit
                hitIndX, hitIndY = hmapX, hmapY
                
                #from the wall color dictionary, get the corresponding wall color
                color = self.wallColorDict.get(self.mapHLines[hitIndY,hitIndX])
            else:
                    
                finalDist = vDist
                hitIndX, hitIndY = vmapX, vmapY
                
                #from the wall color dictionary, get the corresponding wall color
                color = self.wallColorDict.get(self.mapVLines[hitIndY,hitIndX])
                #then scale the color down to achieve some lighting effects
                color = (int(color[0]*wallColorRatio),int(color[1]*wallColorRatio),int(color[2]*wallColorRatio))

            #To get rid of fish eye effect, scale down the distance according to the projection of
            #the casted ray onto the line of sight of the player
            finalDist = finalDist*fastMath.cos(rAng-ang)
            
            #drawing the 3D scene
            try:
                #scaling the wall slice acc. to the actual height 
                #and the projection on the screen
                rayHig = (wallHeight * screenDist)/finalDist
            except ZeroDivisionError:
                continue
            
            #cap the wall height at the screen height
            rayHig = rayHig if rayHig < h else h
            
            #the rectangle that will be drawn by one ray 
            #note: round(numpy.float64) converts to numpy.float64
            #      int(numpy.float64) converts to int
            rayRect = pg.Rect(0,0,int(round(rayWid)),int(round(rayHig)))
            
            #centering the rect
            rayRect.center = (rayCenterx, round(h/2))
            
            #drawing the rect
            pg.draw.rect(screen,color,rayRect)
            
            #if the last rect has been casted, draw a black rect
            #in the remaining area
            if index == rayCount-1 and rayCenterx < w:
                pg.draw.rect(screen,(0,0,0),(rayCenterx,0,w - rayCenterx, h))
            
            #incrementing
            rayCenterx = round(rayCenterx + rayWid)
            index += 1
    
def getScreen(scDims, fullSc = False):
    """Initializes the screen and fills it with a dark gray color.
    Parameters: 
        scDims : the dimensions of the screen as a tuple
        fullSc : if game should run in fullscreen mode
    """
    
    pg.display.set_caption("Ray Caster by Shreesh")
    
    if fullSc:
        screen = pg.display.set_mode(scDims, pg.FULLSCREEN)
    else:
        screen = pg.display.set_mode(scDims)
    
    return screen

def main():
    """the main function that runs the game"""
    #initializing pygame
    pg.init()
    
    #initializing key buffer list
    keys = []
    
    #setting FPS
    fps = 30
    
    #screen resolution
    scDims = (640,400)
    
    #initializing the player
    player = Player(85,85,math.radians(360-45),10)
    
    #initializing the map with the map image
    gMap = GameMap('./levelTrial.bmp', BLOCK_WIDTH, BLOCK_LENGTH, 0, 0)
    
    #initializing screen surface, True for fullSc
    screen = getScreen(scDims, True)
    
    #to maintain the frame rate
    clock = pg.time.Clock()
    
    #the 2D map surface centered at the middle of the screen
    mapSurf = gMap.getMapSurf((50,50,50))
    mapSurf = pg.transform.scale(mapSurf, (scDims[1],scDims[1]))
    mapRect = mapSurf.get_rect()
    mapRect.center = (scDims[0]//2, scDims[1]//2)
    
    #to draw the 3D scene or 2D map, triggered by tab button
    draw3D = True
    
    #drawing the intro screen
    gameText = pygame.freetype.SysFont("Courier", 18)
    screen.fill((0,0,0))
    
    textSurf1, textRect1 = gameText.render("A Simple Ray Casting Engine - written by Shreesh", (255,255,255))
    textSurf2, textRect2 = gameText.render("Use direction keys to move; a, s to turn", (255,255,255))
    textSurf3, textRect3 = gameText.render("Press space to double movement speed", (255,255,255))
    textSurf4, textRect4 = gameText.render("Press escape to exit", (255,255,255))
    textSurf5, textRect5 = gameText.render("Press Enter to continue", (255,255,255))
    
    textRect1.center = (scDims[0]//2, scDims[1]//2 - 60)
    textRect2.center = (scDims[0]//2, scDims[1]//2 - 20)
    textRect3.center = (scDims[0]//2, scDims[1]//2)
    textRect4.center = (scDims[0]//2, scDims[1]//2 + 20)
    textRect5.center = (scDims[0]//2, scDims[1]//2 + 60)
    
    screen.blit(textSurf1, textRect1)
    screen.blit(textSurf2, textRect2)
    screen.blit(textSurf3, textRect3)
    screen.blit(textSurf4, textRect4)
    screen.blit(textSurf5, textRect5)
    
    pg.display.flip()
    #intro screen drawn############################
    
    #to run the game loop
    running = False
    
    #to show intro screen until enter is pressed
    while not running:
        for event in pg.event.get():
            if event.type == pg.KEYDOWN:
                if pg.key.name(event.key) == 'return':
                    running = True
    
    while running:
        #the time since the clock last ticked at the given frequency
        dt = clock.tick(fps)
        
        #checking all events in the event queue
        for event in pg.event.get():
        
            #if a key was pressed down
            if event.type == pg.KEYDOWN:
                
                #reading the key name
                keyDown = pg.key.name(event.key)
                
                #exit if escape was pressed 
                if keyDown == 'escape':
                    running = False
                    break
                elif keyDown == 'tab':
                    draw3D = False
                else:
                    #add any other key press to the buffer,
                    #no duplicate key presses recorded
                    if len(keys) < 5 and keyDown not in keys:
                        keys.append(keyDown)
            
            #if a pressed key was released
            elif event.type == pg.KEYUP:
                
                #keys are removed from the list buffer once released,
                #provided the buffer is not empty, to prevent underflow
                keyUp = pg.key.name(event.key)
                if keyUp == 'tab':
                    draw3D = True
                elif keys:
                    keys.remove(keyUp)
            
            #if the window quit button is pressed
            elif event.type == pg.QUIT:
                running = False
                break
        
        #game logic is only refreshed if a key is pressed
        if keys:
            #movement is tied to framerate
            motion = player.move(8/fps, 0.05/fps, keys, dt, ((gMap.mapDims[1]-1)*gMap.blockDims[0], (gMap.mapDims[0]-1)*gMap.blockDims[1]))
            
            #detect wall collisions and correct them
            player.checkCollision(gMap, motion)
        
        #if tab is pressed, draw 3D scene
        if draw3D:
            #the floor will be a dark gray in color
            screen.fill((50, 50, 50))
            #the "sky" will be white in color
            pg.draw.rect(screen,(255,255,255),(0,0,scDims[0],scDims[1]//2))
            
            gMap.castRays(screen, player, 30, DEPTH_OF_FIELD, BLOCK_HEIGHT, 0.8)
        else:
            #if 2D map is to be drawn
            screen.fill((0, 0, 0))
            screen.blit(mapSurf, mapRect)
            player.draw(screen, mapRect, gMap)
            #cast rays
            
        #refresh the display
        pg.display.flip()
            
    pg.quit()


if __name__ == "__main__":
    main()