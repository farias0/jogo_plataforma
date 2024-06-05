#include "npc.hpp"
#include "princess.hpp"
#include "../../render.hpp"
#include "../../editor.hpp"


#define NPC_FALL_RATE           7.0f


void INpc::CheckAndAdd(Vector2 pos, int interactionTags) {

    if (!(interactionTags & EDITOR_INTERACTION_CLICK)) return;
    
    Princess::CheckAndAdd(pos, interactionTags);
}

void INpc::Tick()
{

    if (Level::STATE->concludedAgo >= 0) return;

    if (isFalling) {
        if (hitbox.y + hitbox.height > FLOOR_DEATH_HEIGHT) {
            isFalling = false;
            Render::PrintSysMessage("NPC não encontrou geometria");
            TraceLog(LOG_WARNING, "Falling NPC didn't find the ground");
        }
        else if (auto groundBeneath = Level::GetGroundBeneath(this)) {
            hitbox.y = groundBeneath->hitbox.y - hitbox.height;
            isFalling = false;
        }
        else {
            hitbox.y += NPC_FALL_RATE;
        }
    }
}

void INpc::Reset() {

    Level::Entity::Reset();
    isFalling = true;
}

void INpc::PersistenceParse(const std::string &data) {

    Level::Entity::PersistenceParse(data);
    // int id = std::stoi(persistenceReadValue(data, "textId")); // TODO NPC ID
}

std::string INpc::PersistanceSerialize() {
    
    std::string data = Level::Entity::PersistanceSerialize();
    // TODO add NPC ID
    return data;
}
