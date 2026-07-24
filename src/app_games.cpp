#include "app_games.h"
#include "game_dino.h"
#include "game_flappy.h"
#include "game_stack.h"
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
        case GameID::FLAPPY: if (_flappy) _flappy->update(); break;
        case GameID::STACK: if (_stack) _stack->update(); break;
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
        // Route to active game with universal exit
        if (event == TouchEvent::DOUBLE_TAP) {
            // Exit game back to game menu
            _exitCurrentGame();
            if (_face) _face->setExpression(Expression::NEUTRAL, 300);
        } else {
            switch (_selectedGame) {
                case GameID::DINO: if (_dino) _dino->handleTouch(event); break;
                case GameID::FLAPPY: if (_flappy) _flappy->handleTouch(event); break;
                case GameID::STACK: if (_stack) _stack->handleTouch(event); break;
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
        case GameID::FLAPPY: 
            _flappy = new GameFlappy(_dm, _storage, _face); 
            _flappy->onEnter();
            break;
        case GameID::STACK: 
            _stack = new GameStack(_dm, _storage, _face); 
            _stack->onEnter();
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
    if (_flappy) { delete _flappy; _flappy = nullptr; }
    if (_stack) { delete _stack; _stack = nullptr; }
    if (_reaction) { delete _reaction; _reaction = nullptr; }
    
    _selectedGame = GameID::NONE;
    
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
            case GameID::FLAPPY: if (_flappy) _flappy->render(); break;
            case GameID::STACK: if (_stack) _stack->render(); break;
            case GameID::REACTION: if (_reaction) _reaction->render(); break;
            default: break;
        }
    }
}

void AppGames::_renderMenu() {
    Adafruit_SSD1306& d = _dm->display();
    
    d.setTextSize(1);
    _dm->drawCenteredText("1-TAP GAMES", 2, 1);
    d.drawLine(0, 12, SCREEN_W, 12, SSD1306_WHITE);
    
    const char* gameNames[] = {
        "DINO RUN",
        "FLAPPY BIRD",
        "STACK TOWER",
        "REACTION TEST"
    };
    
    d.setTextSize(1);
    _dm->drawCenteredText(gameNames[_menuIndex], 30, 2);
    
    // Page Dots
    int startX = SCREEN_W / 2 - 12;
    for (int i = 0; i < (int)GameID::COUNT; i++) {
        int x = startX + i * 8;
        if (i == _menuIndex) {
            d.fillCircle(x, 58, 2, SSD1306_WHITE);
        } else {
            d.drawPixel(x, 58, SSD1306_WHITE);
        }
    }
}
