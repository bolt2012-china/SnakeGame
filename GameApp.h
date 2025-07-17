#pragma once

// Application state machine for Snake game
enum class AppState {
    StartMenu,   // Welcome screen with mode selection
    LevelMode,   // Current implemented level-based mode
    PortalMode,  // Portal mode (to be implemented)
    ScoreMode,   // Endless score‑attack mode (to be implemented)
    Exit,        // Terminate application
    EnergyTrackMode
};
