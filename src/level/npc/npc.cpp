#include "npc.hpp"
#include "../../render.hpp"
#include "../../editor.hpp"


#define DEFAULT_NPC_SPRITE      PrincessDefault1

#define NPC_FALL_RATE           7.0f


Npc *Npc::Add() {
    return Add({ 0, 0 });
}

Npc *Npc::Add(Vector2 pos) {
    
    Npc *newNpc = new Npc();

    Sprite *sprite = &SPRITES->DEFAULT_NPC_SPRITE;
    Rectangle hitbox = SpriteHitboxFromEdge(sprite, pos);

    newNpc->tags = 0;
    newNpc->hitbox = hitbox;
    newNpc->origin = pos;
    newNpc->sprite = sprite;
    newNpc->layer = -1;
    newNpc->isFacingRight = true;

    newNpc->isFalling = true;

    LinkedList::AddNode(&Level::STATE->listHead, newNpc);

    TraceLog(LOG_TRACE, "Added npc to level (x=%.1f, y=%.1f)",
                newNpc->hitbox.x, newNpc->hitbox.y);

    return newNpc;
}

void Npc::CheckAndAdd(Vector2 pos, int interactionTags) {

    if (!(interactionTags & EDITOR_INTERACTION_CLICK)) return;

    // TODO check

    Add(pos);
}

void Npc::Tick() {

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

void Npc::Reset() {

    Level::Entity::Reset();
    isFalling = true;
}

void Npc::PersistenceParse(const std::string &data) {

    Level::Entity::PersistenceParse(data);
    // int id = std::stoi(persistenceReadValue(data, "textId")); // TODO NPC ID
}

std::string Npc::PersistanceSerialize() {
    
    std::string data = Level::Entity::PersistanceSerialize();
    // TODO add NPC ID
    return data;
}
