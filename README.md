# Pacman_SDL2

Pacman_SDL2 is a modern take on the classic Pacman game, developed as a semester project. This version includes custom ghost AI, multiple levels, and a newly designed UI. You can easily add new levels using Tiled.

## Features
- **Multiple Levels**: Create and add as many levels as you like.
- **New UI**: Fresh user interface for a modern gameplay experience.

## Setup Instructions

1. **Create a New Project in Visual Studio**:
   - Open Visual Studio.
   - Create a new project.

2. **Set Up Project Properties**:
   - Ensure all necessary properties for an SDL2 project are set up.

3. **Copy Project Files**:
   - Copy all the files from the `pacman_sdl2` directory to your new project directory (same directory as the solution file).

4. **Build and Run**:
   - Build the project and Run the game.

## Adding Levels

To add new levels to the game:

1. **Install Tiled**:
   - Download and install Tiled from [mapeditor.org](https://mapeditor.org).

2. **Create a New Map**:
   - Get some or make some tilemaps.
   - Design your level using Tiled.

3. **Integrate the New Map**:
   - Export as lua or any other format and convert it into a 2D C++ array.
   - Copy the new map file into the game.
   - Update the game configuration to include the new level.

## Dependencies

- **SDL2**: Make sure SDL2 is installed and configured in your development environment.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

Don't just copy everything from here for your semester project 👀.
