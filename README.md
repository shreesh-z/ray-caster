# ray-caster
A simple ray casting engine I'm building using two languages:
* Python, with pygame and numpy
* C++, with SDL for windows API support (basically the backend of pygame)

The python engine is way ahead of the C++ engine, as I started work on it first.
I will only be working on the C++ engine from now on. I may work on the python engine on request.

Enemy agents are now added to the engine. As of right now they keep following the player unless hardcoded not to.
Once the enemy/s touch the player, the game is over.
Agent sprite rendering is also now supported. The sprites clip around walls.

Textured walls have finally been added. I realized very late about the horizontal distortion that was happening due to
a constant angular step size taken during ray casting. I've fixed it with an arctan calculation for each column of pixels
on the screen. The wall textures now look realistic, except when you go very close to them and they distort.

Some very basic game mechanics have been added to the game. As the player, you have to explore the map to find a 
fully white block. Several monsters are present in the map, and if they touch you, it's game over. If you find the white
block, you win.

As of right now, you can either play the one level loaded by me into the engine, or you can create your own and play.
This is how to create levels in paint:
* Work in 24bit bitmap image format (.bmp). Save it as such
* Place black pixels to create empty blocks
* Place a pixel of value RGB (1, 0, 255) where you want the player to be at the start
* Place a pixel of value RGB (0, 255, 1) where you want to place an enemy
* Textured walls have special pixel values: RGB (255, 0, 5) to (255, 0, 10). Six textures can be currently loaded, you can add more yourself
* Remember to place a white pixel so you can win the game

I'm also planning on adding GPU rendering in the next commit.
I want to keep the CPU and GPU rendering repos separate, so I'll be creating a different repo or a different branch
for it. Maybe fork it to a different repo altogether.

Thanks for reading this.