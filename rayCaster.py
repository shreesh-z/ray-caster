import numpy as np
import pygame as pg
import pygame.freetype
import math

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
        
    def draw(self, screen):
        """Draws on the passed screen surface the player's square,
        and a line showing the direction in which they are looking"""
        pg.draw.rect(screen,(255,255,0),self.playRect)
        pg.draw.aaline(screen, (255,0,0), (self.x, self.y), (int(self.x + self.playerDim*fastMath.cos(self.ang)), int(self.y - self.playerDim*fastMath.sin(self.ang))))
        
    
    def move(self, speed, angVel, keys, dt, bounds):
        """For player movement. Parameters accepted:
        speed  : walking speed of the player
        angVel : turning speed of the player
        keys   : string list of pressed keys
        dt     : time passed since the last time the in-game clock ticked; used to maintain framerate
        
        The keys do the following actions:
        up, down, left, right keys for walking
        a, s keys for turning left and right respectively
        space for halving the speed of walking and turning
        
        Returns the net movement vector as a tuple
        """
        
        #to hold pre-movement x, y positions
        x_, y_ = self.x, self.y
        
        #slow down motion if space is pressed
        if 'space' in keys:
            speed /= 2
            angVel /= 2
        
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
                    return 0
                
                #motion is constrained to screen dimensions
                self.x = (self.x + dx)%bounds[0]; self.y = (self.y + dy)%bounds[1]
                
        self.playRect.move_ip(int(round(self.x - x_)), int(round(self.y - y_)))
        return (self.x - x_, self.y - y_)
    
    def collision(self, gMap, motion):
        """Detects collisions with walls and corrects them.
        Parameters:
            gMap   : GameMap object
            motion : the displacement vector tuple
        """
        #get map array indices from x, y position
        left = int(self.x/gMap.blockDims[0])
        up = int(self.y/gMap.blockDims[1])
        
        #if player has moved into a block, reverse the motion
        if gMap.mapArr[up, left] != 0:
            self.x -= motion[0]
            self.y -= motion[1]
        

class GameMap:
    """A game map class that represents the grid map.
    Attributes:
        (vBlockCnt, hBlockCnt) : Map dimensions by number of blocks   (mapDims)
        (blockW, blockH)       : Block dimensions in pixels           (blockDims)
        (mapL, mapU)           : Position of the 2D map on the screen (mapPos)
        wallColorDict          : Dictionary for colors of walls
    
    Derived Attributes:
        mapArr    : numpy array of the map layout.
        mapHLines : numpy array of the horizontal walls
        mapVLines : numpy array of the vertical walls
        The mapping scheme used by the dictionary is:
            0    for no block
            non0 for a (colored) block
        
    """
    def __init__(self, vBlockCnt, hBlockCnt, blockW, blockH, mapL, mapU, wallColorDict):
        self.mapDims = (vBlockCnt, hBlockCnt)   #how many blocks per direction
        self.blockDims = (blockW, blockH)       #dimension of blocks by pixel
        self.mapPos = (mapL, mapU)              #position of map on screen by pixel
        
        self.mapArr = np.zeros(self.mapDims, 'int8')    #stores the map layout
        #-----customizing-of-the-map-----
        self.mapArr[0,:] = 1
        self.mapArr[-1,:] = 2
        self.mapArr[:,0] = 1
        self.mapArr[:,-1] = 3
        self.mapArr[1:7,1:7] = np.array([[0, 0, 1, 0, 0, 0],
                                           [0, 0, 1, 0, 0, 0],
                                           [0, 0, 0, 0, 2, 2],
                                           [0, 3, 0, 0, 0, 0],
                                           [0, 3, 0, 0, 0, 0],
                                           [0, 0, 0, 1, 0, 0]])
        self.mapArr[7:13,1:7] = self.mapArr[1:7,1:7] 
        self.mapArr[1:7,7:13] = np.roll(self.mapArr[1:7,1:7], 1, axis = 1)
        self.mapArr[7:13,7:13] = np.roll(np.transpose(self.mapArr[1:7,1:7]), 2, axis=0)
        
        #dont try this, you won't be able to move
        #self.mapArr[1:-1,1:-1] = np.random.randint(0,4,(self.mapDims[0]-2,self.mapDims[1]-2))
        #-----customizing-of-the-map-----
        
        self.mapHLines = np.zeros((vBlockCnt+1, hBlockCnt), 'int8') #stores horiz walls
        self.mapVLines = np.zeros((vBlockCnt, hBlockCnt+1), 'int8') #stores vert walls
        for i in np.arange(self.mapArr.shape[0]):
            for j in np.arange(self.mapArr.shape[1]):
                if self.mapArr[i,j] != 0:
                    self.mapHLines[i:i+2,j] = self.mapArr[i,j]
                    self.mapVLines[i,j:j+2] = self.mapArr[i,j]
        self.wallColorDict = wallColorDict
    
    def getMapSurf(self, backColor):
        """Returns a pygame.Surface object with the game map drawn on it.
        Takes the background color as an argument"""
        surf = pg.Surface((self.mapDims[1]*self.blockDims[0], self.mapDims[0]*self.blockDims[1]))
        surf.fill(backColor)
        
        for i in np.arange(self.mapDims[0]):
            for j in np.arange(self.mapDims[1]):
                col = self.wallColorDict.get(self.mapArr[i,j])
                pg.draw.rect(surf, col, (self.blockDims[0]*j, self.blockDims[1]*i, self.blockDims[0]-1, self.blockDims[1]-1))
                #blocks are drawn one pixel less than their dimension thick to add borders
        
        return surf
    
    def castRays(self, screen, player, spread, depth, wallHeight, wallColorRatio, drawrays = False):
        """The main part of the rendering process.
        Arguments:
            screen          : the screen pygame.Surface object
            player          : the Player object present on the map
            spread          : the angular spread of the field of view (in degrees)
            step            : discrete steps of angles in the field of view at which rays are cast (in degrees)
            depth           : depth of field in number of blocks
            scale           : amount of scaling done on the final 3D screen
            wallColorRatio  : ratio of brightness of vert walls over horiz walls, to achieve some lighting effects
            drawrays        : whether to draw the rays on the 2D map
        
        This ray caster works by casting two rays at a given angle. One which checks for 
        horizontal wall collisions, and another which checks for vertical wall collisions.
        
            While checking horiz walls, first the part ray from the player's (x,y) position to the intersection
            with the nearest grid line is taken. It is projected along the vertical line, whose end point is hRayY.
            Using tan(rAng) and hRayY, hRayX is found. (hRayX, hRayY) is the first point of intersection on the grid.
            To find new points of intersection, only an offset needs to be added to the first point,
            which is (-blockH*cot(rAng), blockH) (sign flips if player looking up). At each grid line intersection, 
            wall presence is checked, and rays are cast only up till the depth of field. Once wall presence is found,
            horiz. wall checking is done and the final distance up to which the ray travelled is stored.
            
            Checking vertical walls is similar to checking horiz. walls, but the first ray cast is projected onto the
            horizontal grid line, whose end point is vRayX. tan(rAng) and vRayX are used to find vRayY. The first point of
            intersection with the grid lines is (vRayX, vRayY). The offset is (blockW, -blockW*tan(rAng)). Rest of the
            procedure is the same as before.
            
        Once both the wall checks are done, the two distances are compared, and the ray that travelled the lower distance
        is chosen to render the corresponding wall section. 
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
        screenOffs = self.mapDims[1]*self.blockDims[0] if drawrays else 0
        
        #The width of the rectangle that is rendered by casting one ray
        rayWid = round((w - screenOffs) / rayCount)
        
        #The point on the horizon at which the wall section rendered by a ray is centered
        #Increases as a new ray is to be cast
        rayCenterx = round(screenOffs + rayWid/2)
        
        #casting all the rays
        for rayAng in rAngs:
        
            #finding the ray angle modulo 2pi
            rAng = rayAng%(2*math.pi)
            
            #since this value is used a lot
            tanAng = fastMath.tan(rAng)
            
            #if ray angles are perfectly vertical or horizontal, NaN errors can occur
            #Hence if such angles are encountered, the ray is not casted
            checkRay = True
            
            #The distance travelled by the ray when it hits a wall
            #finalDist = 0
            
            #The indices in mapHLines or mapVLines of the wall at which the ray hit
            #hitIndX, hitIndY = 0, 0
            
            #The indices in mapHLines (hmapX and hmapY) and mapVLines (vmapX and vmapY)
            #Used by the traveling ray to check if the vertical or horiz. wall it hit has a block
            #vmapX, vmapY, hmapX, hmapY = 0,0,0,0
            
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
                if drawrays:
                    #only draw the ray line if the 2D map has to be drawn
                    pg.draw.aaline(screen,(255,0,0),(x,y),(int(hRayX),int(hRayY)))
                
                finalDist = hDist
                hitIndX, hitIndY = hmapX, hmapY
                
                #from the wall color dictionary, get the corresponding wall color
                color = self.wallColorDict.get(self.mapHLines[hitIndY,hitIndX])
            else:
                if drawrays:
                    pg.draw.aaline(screen,(255,0,0),(x,y),(int(vRayX),int(vRayY)))
                    
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
                rayHig = (wallHeight * screenDist)/finalDist
            except ZeroDivisionError:
                continue
            
            #cap the wall height at the screen height
            rayHig = rayHig if rayHig < h else h
            
            #the rectangle that will be drawn by one ray 
            #round(numpy.float64) converts to numpy.float64
            #int(numpy.float64) converts to int
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
    
    scDims = (640,400)
    
    #initializing the player
    player = Player(85,85,math.radians(360-45),20)
    
    #setting the wall color dictionary for the map
    wallColors = {
        0: (255,255,255),
        1: (255,120,0),
        2: (0,200,255),
        3: (0,0,250)
    }
    #initializing the map
    gMap = GameMap(14,14,50,50,0,0,wallColors)
    
    screen = getScreen(scDims, True)
    #to run the game loop
    running = True
    
    #to maintain the frame rate
    clock = pg.time.Clock()
    
    #the surface on which the 2D map is drawn
    mapSurf = gMap.getMapSurf((50,50,50))
    
    #whether to draw the 2D map on the screen
    drawMap = False
    
    #drawing the intro screen
    gameText = pygame.freetype.SysFont("Courier", 18)
    screen.fill((0,0,0))
    
    textSurf1, textRect1 = gameText.render("A Simple Ray Casting Engine - written by Shreesh", (255,255,255))
    textSurf2, textRect2 = gameText.render("Use direction keys to move; a, s to turn", (255,255,255))
    textSurf3, textRect3 = gameText.render("Press space to slow down movement", (255,255,255))
    textSurf4, textRect4 = gameText.render("Press escape to exit", (255,255,255))
    
    textRect1.center = (scDims[0]//2, scDims[1]//2 - 60)
    textRect2.center = (scDims[0]//2, scDims[1]//2 - 20)
    textRect3.center = (scDims[0]//2, scDims[1]//2 + 20)
    textRect4.center = (scDims[0]//2, scDims[1]//2 + 60)
    
    screen.blit(textSurf1, textRect1)
    screen.blit(textSurf2, textRect2)
    screen.blit(textSurf3, textRect3)
    screen.blit(textSurf4, textRect4)
    
    pg.display.flip()
    
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
                
                #add any other key press to the buffer,
                #no duplicate key presses recorded
                if len(keys) < 4 and keyDown not in keys:
                    keys.append(keyDown)
            
            #if a pressed key was released
            elif event.type == pg.KEYUP:
                
                #keys are removed from the list buffer once released,
                #provided the buffer is not empty, to prevent underflow
                keyUp = pg.key.name(event.key)
                if keys:
                    keys.remove(keyUp)
            
            #if the window quit button is pressed
            elif event.type == pg.QUIT:
                running = False
                break
        
        #display is only refreshed with game logic if a key is pressed
        if keys:
            
            #the floor will be a dark gray in color
            screen.fill((50, 50, 50))
            #the "sky" will be white in color
            pg.draw.rect(screen,(255,255,255),(0,0,scDims[0],scDims[1]//2))
            
            #movement is tied to framerate
            motion = player.move(8/fps, 0.05/fps, keys, dt, ((gMap.mapDims[1]-1)*gMap.blockDims[0], (gMap.mapDims[0]-1)*gMap.blockDims[1]))
            
            #detect wall collisions and correct them
            player.collision(gMap, motion)
            
            if drawMap:
                #if 2D map is to be drawn
                screen.blit(mapSurf, gMap.mapPos)
                player.draw(screen)
            
            #cast rays
            gMap.castRays(screen, player, 30, 14, 50, 0.8, drawMap)
            
            #refresh the display
            pg.display.flip()
            
    pg.quit()
    
if __name__ == "__main__":
    main()
