
# Pin ball game just using C++

## Why did I decided to do it?

That's an interesting question, well that the past two or three projects I decided to come back to C to practice and keeping my skills on point, being more focus in coding learning and implementing my knowledge  on visual things like games or UI coding the backend and the frontend of the app (I'm a lot better in backend).

## SDL not OpenGL?

My motto is just "Use the tool that matches with the project" in this case I just doing a simple game so I just needed a simple library to do it, thinking and researching how to draw a circle that's a theory that I had but I just wanted to implemented in this project, also the physics like the collisions were interesting to think and research on.

## How do I build the project?

That's pretty simple use g++ the typical way to build a C++ file, in my case I enabled the warnings to make sure what was wrong in case I did one mistake.
One important thing is install SDL on your system just research on internet how to do it depending of your system

***These are the flags that you have to add when you compile the program***
```
`sdl2-config --cflags --libs`
```
***Or simply copy the following command***
```
g++ -Wall -Wextra main.cpp -o demo `sdl2-config --cflags --libs`
```

## Installing SDL on your system

### Mac
```
brew install sdl2
```
### Debian/ Ubuntu/ Mint 
```
sudo apt install libsdl2-dev
```
## Versions

- V1.2

![ScreenRecording2026-01-27at17 55 51-ezgif com-video-to-gif-converter](https://github.com/user-attachments/assets/357cfe2e-655c-41ba-8f8a-500c0e8c05ef)

- V1.2.1
To pouse the game you have to push scape key on your keyboard, to resume it you have to press space bar

