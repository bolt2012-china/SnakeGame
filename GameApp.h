#pragma once

// Application state machine for Snake game
enum class AppState {
    StartMenu,   
    LevelMode, 
    EnergyTrackMode,
    PortalMode,  
    ScoreMode,   
    Exit    
};
