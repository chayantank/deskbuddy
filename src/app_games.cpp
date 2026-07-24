// ============================================================================
// DeskBuddy — Games Menu Implementation
// ============================================================================
#include "app_games.h"
#include "game_dino.h"
#include "game_pong.h"
#include "game_simon.h"
#include "game_reaction.h"

AppGames::AppGames() : _dm(nullptr), _storage(nullptr), _face(nullptr), _selectedGame(GameID::NONE), _menuIndex(0) {
}

void AppGames::begin(DisplayManager* dm, Storage* storage, FaceRenderer* face) {
    _dm = dm;
    _storage = storage;
    _face = face;
}

AppGames::~AppGames() {
    _exitCurrentGame();
}

void AppGames::onEnter() {
    _selectedGame = GameID::NONE;
    _menuIndex = 0;
}

void AppGames::onExit() {
    _exitCurrentGame();
}

void AppGames::update() {
    switch (_selectedGame) {
        case GameID::DINO: if (_dino) _dino->update(); break;
        case GameID::PONG: if (_pong) _pong->update(); break;
        case GameID::SIMON: if (_simon) _simon->update(); break;
        case GameID::REACTION: if (_reaction) _reaction->update(); break;
        default: break;
    }
}

void AppGames::handleTouch(TouchEvent event) {
    if (_selectedGame == GameID::NONE) {
        // Menu navigation
        if (event == TouchEvent::TAP) {
            _menuIndex = (_menuIndex + 1) % (int)GameID::COUNT;
        } else if (event == TouchEvent::LONG_PRESS) {
            _startGame((GameID)_menuIndex);
        }
    } else {
        // Route to active game
        if (event == TouchEvent::DOUBLE_TAP) {
            // Exit game
            _exitCurrentGame();
            if (_face) _face->setExpression(Expression::NEUTRAL, 300);
        } else {
            switch (_selectedGame) {
                case GameID::DINO: if (_dino) _dino->handleTouch(event); break;
                case GameID::PONG: if (_pong) _pong->handleTouch(event); break;
                case GameID::SIMON: if (_simon) _simon->handleTouch(event); break;
                case GameID::REACTION: if (_reaction) _reaction->handleTouch(event); break;
                default: break;
            }
        }
    }
}

void AppGames::_startGame(GameID id) {
    _exitCurrentGame();
    _selectedGame = id;
    
    switch (id) {
        case GameID::DINO: 
            _dino = new GameDino(_dm, _storage, _face); 
            _dino->onEnter();
            break;
        case GameID::PONG: 
            _pong = new GamePong(_dm, _storage, _face); 
            _pong->onEnter();
            break;
        case GameID::SIMON: 
            _simon = new GameSimon(_dm, _storage, _face); 
            _simon->onEnter();
            break;
        case GameID::REACTION: 
            _reaction = new GameReaction(_dm, _storage, _face); 
            _reaction->onEnter();
            break;
        default: break;
    }
}

void AppGames::_exitCurrentGame() {
    if (_dino) { delete _dino; _dino = nullptr; }
    if (_pong) { delete _pong; _pong = nullptr; }
    if (_simon) { delete _simon; _simon = nullptr; }
    if (_reaction) { delete _reaction; _reaction = nullptr; }
    
    _selectedGame = GameID::NONE;
    
    // Ensure display isn't inverted by reaction game
    if (_dm) {
        _dm->display().invertDisplay(false);
    }
}

void AppGames::render() {
    if (!_dm) return;
    
    if (_selectedGame == GameID::NONE) {
        _renderMenu();
    } else {
        switch (_selectedGame) {
            case GameID::DINO: if (_dino) _dino->render(); break;
            case GameID::PONG: if (_pong) _pong->render(); break;
            case GameID::SIMON: if (_simon) _simon->render(); break;
            case GameID::REACTION: if (_reaction) _reaction->render(); break;
            default: break;
        }
    }
}

void AppGames::_renderMenu() {
    Adafruit_SSD1306& d = _dm->display();
    
    d.setTextSize(1);
    _dm->drawCenteredText("GAMES", 2, 1);
    d.drawLine(0, 12, SCREEN_W, 12, SSD1306_WHITE);
    
    const char* gameNames[] = {
        "DINO RUN",
        "SOLO PONG",
        "SIMON SAYS",
        "REACTION"
    };
    
    _dm->drawCenteredText(gameNames[_menuIndex], 30, 2);
    _dm->drawCenteredText("Long Press to Start", 50, 1);
    
    // Dots
    int startX = SCREEN_W / 2 - 12;
    for (int i = 0; i < (int)GameID::COUNT; i++) {
        int x = startX + i * 8;
        if (i == _menuIndex) {
            d.fillCircle(x, 60, 2, SSD1306_WHITE);
        } else {
            d.drawPixel(x, 60, SSD1306_WHITE);
        }
    }
}
