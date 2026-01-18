cmake_minimum_required(VERSION 4.0)
project(S)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(S
        Bomb.cpp
        Bomb.h
        Door.h
        FileGame.cpp
        FileGame.h
        GameBase.cpp
        GameBase.h
        GameDefs.h
        Key.h
        KeyboardGame.cpp
        KeyboardGame.h
        Main.cpp
        Maps.h
        Obstacle.h
        Player.cpp
        Player.h
        Point.cpp
        Point.h
        Results.cpp
        Results.h
        Riddle.cpp
        Riddle.h
        Screen.cpp
        Screen.h
        Spring.cpp
        Spring.h
        Steps.cpp
        Steps.h
        Switch.h
        Templates.h
        Torch.h
        Utils.cpp
        Utils.h
        GameBase.cpp
        DoorElement.h
        BoardChars.h
        Legand.h)
