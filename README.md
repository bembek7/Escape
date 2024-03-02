# Escape

Developed with Unreal Engine 5

Short video showing how the gameplay looks like:

[![Alt text](https://img.youtube.com/vi/v8iWWGkLW_0/0.jpg)](https://youtu.be/v8iWWGkLW_0)


There is a packaged build for Windows in PackedBuild/Windows

### Description

A small parkour game where using abilities like dash or grapple you have to get through as many floors as you can. The faster you are the better. Slide, jump, run on the walls to get as fast as you can to the end of the floor. Every floor is randomly generated from the array of possible rooms so you can play infinitely and every run will be different.


### Code Structure
Thera are 3 main classes:

<h5>PlayerBase</h5>
Player Character class where abilities and movemnt mechanics are implemented.

<h5>PlayerControllerBase</h5>
Player Controller class mostly responsible for managing the widgets.

<h5>GMBase</h5> 

Game Mode class responsible for spawning the level and counting the score.


The project was firstly done in blueprints and then coverted to C++. I was doing it all alone so there is no sound or animation. Some of the abilities are used from my another project ASCEND, but they're also made by me.
