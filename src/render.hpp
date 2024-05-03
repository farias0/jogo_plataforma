#ifndef _RENDER_H_INCLUDED_
#define _RENDER_H_INCLUDED_

#include "string"


#define SYS_MSG_BUFFER_SIZE 1000

#define LEVEL_TRANSITION_ANIMATION_DURATION 0.7 // in seconds

#define ORIGIN_GHOST_TRANSPARENCY 30


namespace Level { class Entity; }

namespace Render {


class IDrawable {

public:
    virtual void Draw() = 0;
    virtual void DrawMoveGhost() = 0; // Used when moving the entity around in the editor
};


void Initialize();

void Render();

void DrawLevelEntity(Level::Entity *entity);

void DrawLevelEntityOriginGhost(Level::Entity *entity);

void DrawLevelEntityMoveGhost(Level::Entity *entity);

// Prints a system message in the screen
void PrintSysMessage(const std::string &msg);

/*
    Starts the visual effect of transitioning from overworld to level or vice-versa.

    sceneFocusPont: The point in the scene at the center of the effect.
    isClose: If the effect will close on the sceneFocusPont, or, if false, if it will open from it.
*/
void LevelTransitionEffectStart(Vector2 sceneFocusPont, bool isClose);

// Toggles between full screen and windowed
void FullscreenToggle();


} // namespace


#endif // _RENDER_H_INCLUDED_
