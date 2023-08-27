
# Table of Contents

1.  [Totui](#orgf9e33dc)
    1.  [Building](#org1eaee8c)
2.  [Usage guide](#org74c41f2)
3.  [Things to do (no pun intended)](#orga2ddb36)
    1.  [Core functionality](#orga7d1a27)
    2.  [Aesthetics](#org8317072)
    3.  [Perhaps in the future](#org97b5180)


<a id="orgf9e33dc"></a>

# Totui

Totui is a terminal todo list manager. The name *Totui* is from the words &ldquo;TOdo&rdquo; and &ldquo;TUI&rdquo; (terminal user interface).


<a id="org1eaee8c"></a>

## Building

You can

1.  Clone the repo: `git clone https://github.com/yaaama/totui`
2.  Go into the cloned folder: `cd totui`
3.  Make the file `make`
4.  Run the program: `./totui`


<a id="org74c41f2"></a>

# Usage guide

The keybindings are simple. They are inspired by **vim** keys.

-   Move up:
    -   <kbd> k </kbd>
-   Move down:
    -   <kbd> j </kbd>
-   Insert new todo:
    -   <kbd> a </kbd>
-   Delete the currently selected todo item:
    -   <kbd> d </kbd>
-   Quit the program:
    -   <kbd> q </kbd>
-   Toggle the todo status:
    -   <kbd> SPACE </kbd>


<a id="orga2ddb36"></a>

# Things to do (no pun intended)

Please note that I have basically done what I wanted to do and may stop updating this project as often as I have been.
I was mostly using this project as a reason for me to learn how `ncurses` works.


<a id="orga7d1a27"></a>

## Core functionality

-   [X] Can traverse through todo items displayed on the screen.
-   [X] User can add a new todo item.
-   [X] Todo items can be deleted.
-   [X] Toggle todo items
-   [X] Resize application as the window is resized.
-   [X] Echo bar on the bottom (displays information about user actions and program processes).
-   [ ] Help bar on the top (displays q to quit, etc).
-   [ ] Warnings for when the user may insert too long of a todo.


<a id="org8317072"></a>

## Aesthetics

-   [X] Colour lines based on status.
-   [ ] A line containing something longer than the terminal width should be truncated on the screen.
    -   Program should calculate how many lines the new item is occupying, and then move the cursor so that it is below the new item.
-   [ ] Lines that overflow the buffer should be rejected


<a id="org97b5180"></a>

## Perhaps in the future

-   [ ] User can specify the default file they want to use.
-   [ ] A split plane where the user can add more information about each todo (not limited by character counts).
-   [ ] Configuration file where the user can specify the file to store the todo items in.

